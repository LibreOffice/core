#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: pre2par.pl,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: kz $ $Date: 2006-07-05 21:03:28 $
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
pre2par::parameter::outputparameter();

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
