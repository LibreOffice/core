#*************************************************************************
#
#   $RCSfile: pre2par.pl,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: hjs $ $Date: 2004-06-25 16:09:26 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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

my $parfile = pre2par::work::convert($prefile);

pre2par::work::formatter($parfile);

my $ulffilename = pre2par::work::getulffilename($pre2par::globals::prefilename);

if ( pre2par::work::fileexists($ulffilename) )
{
    my $ulffile = pre2par::files::read_file($ulffilename);
    pre2par::language::localize($parfile, $ulffile);
}

pre2par::files::save_file($pre2par::globals::parfilename, $parfile);

####################################
# End main program
####################################
