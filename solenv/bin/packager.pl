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



use lib ("$ENV{SOLARENV}/bin/modules");

use Cwd;
use packager::check;
use packager::files;
use packager::globals;
use packager::work;

####################################
# Main program
####################################

packager::check::check_environment();
packager::check::check_packlist();
packager::check::check_parameter();

packager::work::set_global_variable();

my $packagelist = packager::files::read_file($packager::globals::packlistname);

my $targets = packager::work::create_package_todos($packagelist);

if ( $ENV{'BSCLIENT'} ) { packager::work::start_build_server($targets); }
else { packager::work::execute_system_calls($targets); }

if ( $packager::globals::logging )
{
    packager::files::save_file($packager::globals::logfilename, \@packager::globals::logfileinfo);
    print "Log file written: $packager::globals::logfilename\n";
}

####################################
# End main program
####################################
