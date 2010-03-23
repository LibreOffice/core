#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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
