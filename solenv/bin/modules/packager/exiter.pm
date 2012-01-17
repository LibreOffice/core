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




package packager::exiter;

use packager::files;
use packager::globals;

############################################
# Exiting the program with an error
# This function is used instead of "die"
############################################

sub exit_program
{
    my ($message, $function) = @_;

    my $infoline;

    $infoline = "\n***************************************************************\n";
    push(@packager::globals::logfileinfo, $infoline);
    print("$infoline");

    $infoline = "$message\n";
    push(@packager::globals::logfileinfo, $infoline);
    print("$infoline");

    $infoline = "in function: $function\n";
    push(@packager::globals::logfileinfo, $infoline);
    print("$infoline");

    $infoline = "***************************************************************\n";
    push(@packager::globals::logfileinfo, $infoline);

    if ( $packager::globals::logging )
    {
        packager::files::save_file($packager::globals::logfilename ,\@packager::globals::logfileinfo);
        print("Saved logfile: $packager::globals::logfilename\n");
    }

    print("$infoline");

    exit(-1);
}

1;
