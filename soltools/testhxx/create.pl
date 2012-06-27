#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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
    print STDOUT '#!/bin/sh', "\n";
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
  if ($ENV{COM} eq 'GCC') {
    1 while $in =~ s!\s+-I\s*\.\S*\s*! !g; # discard relative includes
    $in =~ s!(\s+-I\s*)(?i:$solarversion)(\S*)!$1\${SOLARVERSION}$2!og;
        # macrofy includes to solver
    $in =~ s!\s+-o\s*\S+! -o /dev/null! || die 'bad input: no -o';
    $in =~ s!\S+/testhxx.cxx!\${my_tmp}!
        || die 'bad input: no source file';
    print STDOUT '#!/bin/sh', "\n";
    print STDOUT
        'my_tmp=${TMPDIR:-/tmp}/`id -u`_$$_include.cc', "\n";
    print STDOUT 'my_pat=`dirname $1`', "\n";
    print STDOUT 'my_fil=`basename $1`', "\n";
    print STDOUT 'my_org=${PWD}', "\n";
    print STDOUT 'cd $my_pat || exit 1', "\n";
    print STDOUT 'my_pat=`cygpath -m \`pwd\``', "\n";
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
  }
} else {
    print STDOUT 'echo \'no testhxx on this platform\'', "\n";
}
