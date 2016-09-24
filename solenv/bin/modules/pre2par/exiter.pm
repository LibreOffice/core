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

package pre2par::exiter;

use pre2par::files;
use pre2par::globals;

############################################
# Exiting the program with an error
# This function is used instead of "die"
############################################

sub exit_program
{
    my ($message, $function) = @_;

    my $infoline;

    $infoline = "\n***************************************************************\n";
    push(@pre2par::globals::logfileinfo, $infoline);
    print("$infoline");

    $infoline = "$message\n";
    push(@pre2par::globals::logfileinfo, $infoline);
    print("$infoline");

    $infoline = "in function: $function\n";
    push(@pre2par::globals::logfileinfo, $infoline);
    print("$infoline");

    $infoline = "***************************************************************\n";
    push(@pre2par::globals::logfileinfo, $infoline);

    if ($pre2par::globals::logging)
    {
        pre2par::files::save_file($pre2par::globals::logfilename ,\@pre2par::globals::logfileinfo);
        print("Saved logfile: $pre2par::globals::logfilename\n");
    }

    print("$infoline");

    exit(-1);
}

1;
