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



package macosxotoolhelper;
require Exporter;
our @ISA = Exporter;
our @EXPORT = otoolD;

use File::Basename;
$::CC_PATH=(fileparse( $ENV{"CC"}))[1];

sub otoolD($) {
    my ($file) = @_;
    my $call = "otool -D $file";
    open(IN, "-|", $call) or die "cannot $call";
    my $line = <IN>;
    if( $line !~ /^\Q$file\E:\n$/ ) {
        die "unexpected otool -D output (\"$line\", expecting \"$file:\")";
    }
    $line = <IN>;
    <IN> == undef or die "unexpected otool -D output";
    close(IN);
    return $line;
}

