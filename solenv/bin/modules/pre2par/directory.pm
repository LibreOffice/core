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



package pre2par::directory;

use pre2par::pathanalyzer;
use pre2par::systemactions;

############################################
# Checking, whether the output directories
# exist. If not, they are created.
############################################

sub check_directory
{
    my ($parfilename) = @_;

    my $productdirectory = $parfilename;
    pre2par::pathanalyzer::get_path_from_fullqualifiedname(\$productdirectory);
    $productdirectory =~ s/\Q$pre2par::globals::separator\E\s*$//;

    my $pardirectory = $productdirectory;
    pre2par::pathanalyzer::get_path_from_fullqualifiedname(\$pardirectory);
    $pardirectory =~ s/\Q$pre2par::globals::separator\E\s*$//;

    if ( ! -d $pardirectory ) { pre2par::systemactions::create_directory($pardirectory); }
    if ( ! -d $productdirectory ) { pre2par::systemactions::create_directory($productdirectory); }
}

1;