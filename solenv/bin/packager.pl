#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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
