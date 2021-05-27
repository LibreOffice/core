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


package pre2par::systemactions;

use File::Copy;
use pre2par::exiter;
use pre2par::globals;

######################################################
# Creating a new directory
######################################################

sub create_directory
{
    my ($directory) = @_;

    my $returnvalue = 1;
    my $infoline = "";

    if ($directory eq "" )
    {
        return 0;
    }

    if (!(-d $directory))
    {
        $returnvalue = mkdir($directory, 0775);

        if ($returnvalue)
        {
            $infoline = "Created directory: $directory\n";
            push(@pre2par::globals::logfileinfo, $infoline);

            if ($pre2par::globals::isunix)
            {
                my $localcall = "chmod 775 $directory \>\/dev\/null 2\>\&1";
                system($localcall);
            }
        }
        else
        {
            # New solution in parallel packing: It is possible, that the directory now exists, although it
            # was not created in this process. There is only an important error, if the directory does not
            # exist now.

            if (!(-d $directory))
            {
                pre2par::exiter::exit_program("Error: Could not create directory: $directory", "create_directory");
            }
            else
            {
                $infoline = "\nAnother process created this directory in exactly this moment :-) : $directory\n";
                push(@pre2par::globals::logfileinfo, $infoline);
            }
        }
    }
    else
    {
        $infoline = "\nAlready existing directory, did not create: $directory\n";
        push(@pre2par::globals::logfileinfo, $infoline);
    }
}

1;
