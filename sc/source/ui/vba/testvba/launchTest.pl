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
