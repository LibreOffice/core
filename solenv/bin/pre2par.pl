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
use pre2par::directory;
use pre2par::files;
use pre2par::globals;
use pre2par::language;
use pre2par::parameter;
use pre2par::work;

####################################
# Main program
####################################

pre2par::parameter::getparameter();
pre2par::parameter::control_parameter();

pre2par::directory::check_directory($pre2par::globals::parfilename);

my $prefile = pre2par::files::read_file($pre2par::globals::prefilename);

pre2par::work::check_content($prefile, $pre2par::globals::prefilename);

my $parfile = pre2par::work::convert($prefile);

pre2par::work::formatter($parfile);

my $langfilename = pre2par::work::getlangfilename();
my $ulffilename = pre2par::work::getulffilename($pre2par::globals::prefilename);

my $dolocalization = pre2par::work::check_existence_of_langfiles($langfilename, $ulffilename);

if ( $dolocalization )
{
    my $langfile = pre2par::files::read_file($langfilename);
    pre2par::language::localize($parfile, $langfile);
}

pre2par::files::save_file($pre2par::globals::parfilename, $parfile);

# checking of par file was written correctly
my $parfilecomp = pre2par::files::read_file($pre2par::globals::parfilename);
pre2par::work::diff_content($parfile, $parfilecomp, $pre2par::globals::parfilename);

####################################
# End main program
####################################
