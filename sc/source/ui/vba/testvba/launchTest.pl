#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************



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
