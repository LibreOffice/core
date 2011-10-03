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

packager::work::execute_system_calls($targets);

if ( $packager::globals::logging )
{
    packager::files::save_file($packager::globals::logfilename, \@packager::globals::logfileinfo);
    print "Log file written: $packager::globals::logfilename\n";
}

####################################
# End main program
####################################
