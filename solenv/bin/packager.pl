#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: packager.pl,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 22:12:25 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
