#*************************************************************************
#
#   $RCSfile: par2script.pl,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: obo $ $Date: 2004-11-18 08:33:08 $
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
use par2script::check;
use par2script::files;
use par2script::globals;
use par2script::parameter;
use par2script::module;
use par2script::shortcut;
use par2script::undefine;
use par2script::work;

####################################
# Main program
####################################

par2script::parameter::getparameter();
par2script::parameter::control_parameter();
par2script::parameter::outputparameter();

my $includes = par2script::work::setincludes($par2script::globals::includepathlist);
my $parfiles = par2script::work::setparfiles($par2script::globals::parfilelist);

par2script::work::make_complete_pathes_for_parfiles($parfiles, $includes);

my $parfilecontent = par2script::work::read_all_parfiles($parfiles);

my $setupscript = par2script::work::collect_all_items($parfilecontent);

par2script::undefine::undefine_gids($setupscript, $parfilecontent);

par2script::shortcut::shift_shortcut_positions($setupscript);
par2script::module::remove_from_modules($setupscript);
par2script::module::add_to_root_module($setupscript);

# checking the setup script

par2script::check::check_needed_directories($setupscript);
par2script::check::check_directories_in_item_definitions($setupscript, "File");
par2script::check::check_directories_in_item_definitions($setupscript, "Shortcut");
par2script::check::check_directories_in_item_definitions($setupscript, "Profile");
par2script::check::check_module_existence($setupscript);
# par2script::check::check_registry_at_files($setupscript);
par2script::check::check_moduleid_at_items($setupscript);

# saving the script

par2script::files::save_file($par2script::globals::scriptname, $setupscript);

# logging, if set

if ($par2script::globals::logging)
{
    par2script::files::save_file($par2script::globals::logfilename, \@par2script::globals::logfileinfo);
    print "Log file written: $par2script::globals::logfilename\n";
}

####################################
# End main program
####################################
