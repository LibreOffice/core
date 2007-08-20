#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: par2script.pl,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: ihi $ $Date: 2007-08-20 15:25:05 $
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
use par2script::check;
use par2script::files;
use par2script::globals;
use par2script::parameter;
use par2script::module;
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

print "Reading par files\n";
my $parfilecontent = par2script::work::read_all_parfiles($parfiles);

print "Collecting items\n";
par2script::work::collect_definitions($parfilecontent);

print "Collecting assigned items\n";
par2script::work::collect_assigned_gids();

# print "First control of multiple assignments\n";
# par2script::check::check_multiple_assignments();

print "Searching for Undefinitions\n";
par2script::undefine::undefine_gids($parfilecontent);
par2script::undefine::remove_complete_item("Directory", $parfilecontent);
par2script::undefine::remove_complete_item("Profile", $parfilecontent);

print "Removing assigned GIDs without definitions\n";
par2script::module::remove_undefined_gids_from_modules();

print "Adding definitions without assignment to the root\n";
par2script::module::add_to_root_module();

print "Control of multiple assignments\n";
par2script::check::check_multiple_assignments();

print "Control of definitions with missing assignments\n";
par2script::check::check_missing_assignments();

# checking the setup script
print "Checking directory definitions ...\n";
par2script::check::check_needed_directories();
par2script::check::check_directories_in_item_definitions();
print "Checking module definitions ...\n";
par2script::check::check_module_existence();
print "Checking module assignments ...\n";
par2script::check::check_moduleid_at_items();
print "Checking StarRegistry ...\n";
par2script::check::check_registry_at_files();
print "Checking Root Module ...";
par2script::check::check_rootmodule();
print "Checking Shortcut assignments ...\n";
par2script::check::check_shortcut_assignments();
print "Checking missing parents ...\n";
par2script::check::check_missing_parents();

print "Shorten lines at modules\n";
par2script::module::shorten_lines_at_modules();

# Now the script can be created
print "Creating setup script\n";
my $setupscript = par2script::work::create_script();

print "Saving script\n";
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
