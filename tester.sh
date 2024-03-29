#!/bin/sh
#     dctrl-tools - Debian control file inspection tools
#     Copyright (C) 2007, 2010, 2011 Antti-Juhani Kaijanaho
#
#     This program is free software; you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation; either version 2 of the License, or
#     (at your option) any later version.
#
#     This program is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
#
#     You should have received a copy of the GNU General Public License along
#     with this program; if not, write to the Free Software Foundation, Inc.,
#     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

set -e

trap "rm .testout .diffout .testerr .differr 2>/dev/null" \
    EXIT ABRT BUS FPE HUP ILL QUIT SEGV TERM

GREP_DCTRL=../grep-dctrl/grep-dctrl
SORT_DCTRL=../sort-dctrl/sort-dctrl
TBL_DCTRL=../tbl-dctrl/tbl-dctrl
JOIN_DCTRL=../join-dctrl/join-dctrl
export GREP_DCTRL SORT_DCTRL TBL_DCTRL JOIN_DCTRL

cd tests

if [ $# -ge 1 ] ; then
    tests=$@
else
    tests=`ls -1 *.sh | sort`
fi

rv=0

for tst in $tests ; do
    tst_base=`basename $tst .sh`
    tst_in=$tst_base.in
    tst_out=$tst_base.out
    tst_ero=$tst_base.err
    tst_err=$tst_base.fails
    if [ ! -r $tst_in ] ; then
        tst_in=/dev/null
    fi
    if [ ! -r $tst_ero ] ; then
        tst_ero=/dev/null
    fi
    if [ ! -r $tst_out ] && [ ! -r $tst_err ] ; then
        echo 1>&2 "neither $tst_out nor $tst_err exists"
        exit 1
    fi
    if [ -r $tst_err ]; then
        echo -n "$0: Test case $tst_base (expecting failure)..."
    else
        echo -n "$0: Test case $tst_base (expecting success)..."
    fi
    ok=true
    if sh $tst < $tst_in > .testout 2> .testerr ; then
        if diff -au $tst_out .testout > .diffout ; then
            :
        else
           ok=false
        fi
    else
        if [ -r $tst_err ] ; then
            :
        else
            ok=false
        fi
    fi
    if [ -r $tst_ero ] ; then
        if diff -au $tst_ero .testerr > .differr ; then
            :
        else
            ok=false
        fi
    fi
    if $ok ; then
        echo "ok."
    else
        echo "FAILED."
        echo "stdout diff:"
        cat .diffout
        echo "stderr diff:"
        cat .differr
        rv=1
    fi
done

exit $rv
