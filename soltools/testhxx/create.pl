#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: create.pl,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: hr $ $Date: 2006-06-20 05:09:44 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
} elsif ($ENV{OS} eq 'WNT') {
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
} else {
    print STDOUT 'echo \'no testhxx on this platform\'', "\n";
}
