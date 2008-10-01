#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: launchTest.pl,v $
#
# $Revision: 1.3.138.1 $
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

use File::Basename;

my $test_class = shift || die 'must provide a ext name';
my $TESTDOCUMENT = shift || die 'must provide a path to testdocument dirs';
my $OUTPUTDIR = shift || die 'must provide an output path to deposit logs in';

die "can't access TestClass $test_class/TestVBA.class" unless -f "$test_class/TestVBA.class";
die "can't access officepath env variable \$OFFICEPATH" unless -d $ENV{OFFICEPATH};
die "can't access testdocuments" unless -d $TESTDOCUMENT;
die "testdocument not of the correct structure $TESTDOCUMENT/logs/excel" unless -d "$TESTDOCUMENT/logs/excel";
die "can't access output dir" unless -d $OUTPUTDIR;


my $officeclasspath = "$ENV{OFFICEPATH}/program/classes/";
my $classpath = "$officeclasspath/jurt.jar:$officeclasspath/unoil.jar:$officeclasspath/juh.jar:$officeclasspath/java_uno.jar:$officeclasspath/ridl.jar:$test_class:$ENV{CLASSPATH}";
$ENV{CLASSPATH}=$classpath;
print "classpath $ENV{CLASSPATH}\n";
my $status = system("java -classpath $ENV{CLASSPATH} TestVBA $TESTDOCUMENT $OUTPUTDIR" );
