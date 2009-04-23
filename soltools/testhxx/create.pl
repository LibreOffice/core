#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: create.pl,v $
#
# $Revision: 1.4 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$solarversion = $ENV{SOLARVERSION};
$solarversion =~ s![^0-9A-Za-z]!\\$&!g;
$in = <> || die 'no input';
chomp $in;
if ($ENV{OS} eq 'LINUX') {
    1 while $in =~ s!\s+-I\s*[^/]\S*\s*! !g; # discard relative includes
    $in =~ s!(\s+-I\s*)$solarversion(\S*)!$1\${SOLARVERSION}$2!og;
        # macrofy includes to solver
    $in =~ s!\s+-o\s*\S+! -o /dev/null! || die 'bad input: no -o';
    $in =~ s!\S+/testhxx.cxx!-x c++ /proc/self/fd/0!
        || die 'bad input: no source file';
    print STDOUT '#!/bin/bash', "\n";
    print STDOUT $in,
        ' <<<"#include \\"`echo $(if [ ${1%/*} != $1 ];then cd ${1%/*};fi;',
        '/bin/pwd)/${1##*/}`\\""', "\n";
} elsif ($ENV{OS} eq 'SOLARIS') {
    1 while $in =~ s!\s+-I\s*[^/]\S*\s*! !g; # discard relative includes
    $in =~ s!(\s+-I\s*)$solarversion(\S*)!$1\${SOLARVERSION}$2!og;
        # macrofy includes to solver
    $in =~ s!\s+-o\s*\S+! -o /dev/null! || die 'bad input: no -o';
    $in =~ s!\S+/testhxx.cxx!\${my_tmp}!
        || die 'bad input: no source file';
    print STDOUT '#!/bin/sh', "\n";
    print STDOUT
        'my_tmp=${TMPDIR:-/tmp}/`/usr/xpg4/bin/id -u`_$$_include.cc', "\n";
    print STDOUT 'my_pat=`dirname $1`', "\n";
    print STDOUT 'my_fil=`basename $1`', "\n";
    print STDOUT 'my_org=${PWD}', "\n";
    print STDOUT 'cd $my_pat || exit 1', "\n";
    print STDOUT 'my_pat=`pwd`', "\n";
    print STDOUT 'cd $my_org || exit 1', "\n";
    print STDOUT
        'echo "#include \\"${my_pat}/${my_fil}\\"" > ${my_tmp} || exit 1', "\n";
    print STDOUT $in, ' > ${my_tmp}.out 2>&1', "\n";
    print STDOUT 'my_ret=$?', "\n";
    print STDOUT
        'if [ ${my_ret} -ne 0 ] ; then echo $1 >&2 ; cat ${my_tmp}.out >&2 ;',
        ' fi', "\n";
    print STDOUT 'unlink ${my_tmp} || exit 1', "\n";
    print STDOUT 'unlink ${my_tmp}.out || exit 1', "\n";
    print STDOUT 'exit ${my_ret}', "\n";
} elsif ($ENV{OS} eq 'WNT' and $ENV{USE_SHELL} eq '4nt') {
    1 while $in =~ s!\s+-I\s*\.\S*\s*! !g; # discard relative includes
    $in =~ s!(\s+-I\s*)(?i:$solarversion)(\S*)!$1%SOLARVERSION%$2!og;
        # macrofy includes to solver
    $in =~ s!\s+-Fo\s*\S+! -Fo%my_tmp%.obj! || die 'bad input: no -Fo';
    $in =~ s!\s+-Zi\s! !;
    $in =~ s!\s+-Fd\s*\S+!!;
    $in =~ s!\S+\\testhxx.cxx!%my_tmp%!
        || die 'bad input: no source file';
    print STDOUT '@ECHO OFF', "\n";
    print STDOUT 'SET my_tmp=%TMP%\%_PID%_include.cc', "\n";
    print STDOUT 'ECHO #include "%@REPLACE[\\,/,%@FULL[%1]]" > %my_tmp%', "\n";
    print STDOUT $in, ' > %my_tmp%.out', "\n";
    print STDOUT 'SET my_ret=%ERRORLEVEL%', "\n";
    print STDOUT 'IF %my_ret% != 0',
        ' sed -e s!%@FILENAME[%my_tmp%]!%@REPLACE[\\,\\\\,%1]! %my_tmp%.out',
        ' 1>&2', "\n";
    print STDOUT 'DEL /EQ %my_tmp% %my_tmp%.obj %my_tmp%.pdb %my_tmp%.out',
        "\n";
    print STDOUT 'QUIT %my_ret%', "\n";
} elsif ($ENV{OS} eq 'WNT' and $ENV{USE_SHELL} ne '4nt') {
    1 while $in =~ s!\s+-I\s*\.\S*\s*! !g; # discard relative includes
    $in =~ s!(\s+-I\s*)(?i:$solarversion)(\S*)!$1\${SOLARVERSION}$2!og;
        # macrofy includes to solver
    $in =~ s!\s+-Fo\s*\S+! -Fo$[my_tmp}obj! || die 'bad input: no -Fo';
    $in =~ s!\s+-Zi\s! !;
    $in =~ s!\s+-Fd\s*\S+!!;
    print STDOUT '#!/bin/sh', "\n";
    print STDOUT
        'my_tmp=${TMPDIR:-/tmp}/`id -u`_$$_include.cc', "\n";
    print STDOUT 'my_pat=`dirname $1`', "\n";
    print STDOUT 'my_fil=`basename $1`', "\n";
    print STDOUT 'my_org=${PWD}', "\n";
    print STDOUT 'cd $my_pat || exit 1', "\n";
    print STDOUT 'my_pat=`pwd`', "\n";
    print STDOUT 'cd $my_org || exit 1', "\n";
    print STDOUT
        'echo "#include \\"${my_pat}/${my_fil}\\"" > ${my_tmp} || exit 1', "\n";
    print STDOUT $in, ' > ${my_tmp}.out 2>&1', "\n";
    print STDOUT 'my_ret=$?', "\n";
    print STDOUT
        'if [ ${my_ret} -ne 0 ] ; then echo $1 >&2 ; cat ${my_tmp}.out >&2 ;',
        ' fi', "\n";
    print STDOUT 'unlink ${my_tmp} || exit 1', "\n";
    print STDOUT 'unlink ${my_tmp}.out || exit 1', "\n";
    print STDOUT 'exit ${my_ret}', "\n";
} else {
    print STDOUT 'echo \'no testhxx on this platform\'', "\n";
}
