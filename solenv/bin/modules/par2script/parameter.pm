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


package par2script::parameter;

use Cwd;
use par2script::files;
use par2script::globals;
use par2script::systemactions;

############################################
# Parameter Operations
############################################

###############################################################################
# Usage:
# perl par2script.pl -i ..\wntmsci8.pro\par,o:\SRX645\wntmsci8.pro\par.m24
#                   @@C:\DOCUMEN~1\is\LOCALS~1\Temp\mk6pd
#                   -o ..\wntmsci8.pro\bin\osl\setup_osl.inf
###############################################################################

sub usage
{
    print <<End;

--------------------------------------------------------------
$par2script::globals::prog
The following parameter are needed:
-i: include paths, comma separated list
-o: setup script file name
-v: writing logfile.txt (optional)
\@\@list: list of all par files

Example:
 perl par2script.pl -i ..\\wntmsci8\\par\,o\:\\SRX645\\wntmsci8\\par.m24
         \@\@C\:\\DOCUMEN\~1\\is\\LOCALS\~1\\Temp\\mk6pd
         -o ..\\wntmsci8.pro\\bin\\osl\\setup_osl.inf \[-v\]

--------------------------------------------------------------
End
    exit(-1);
}

#####################################
# Reading parameter
#####################################

sub getparameter
{
    while ( $#ARGV >= 0 )
    {
        my $param = shift(@ARGV);

        if ($param eq "-o") { $par2script::globals::scriptname = shift(@ARGV); }
        elsif ($param eq "-q") { $par2script::globals::verbose = 0; }
        elsif ($param eq "-v") { $par2script::globals::logging = 1; }
        elsif ($param =~ /\@\@/) { $par2script::globals::parfilelistorig = $param; }
        elsif ($param eq "-i") { $par2script::globals::includepathlist = shift(@ARGV); }
        elsif (($param =~ /\//) || ($param =~ /\\/))    # another include parameter!
        {
            $par2script::globals::includepathlist = $par2script::globals::includepathlist . "," . $param;
        }
        else
        {
            print("\n*************************************\n");
            print("Sorry, unknown parameter: $param");
            print("\n*************************************\n");
            usage();
            exit(-1);
        }
    }
}

############################################
# Controlling  the fundamental parameter
# (required for every process)
############################################

sub control_parameter
{
    if ($par2script::globals::includepathlist eq "")
    {
        print "\n************************************************\n";
        print "Error: Include paths not set not set (-i)!";
        print "\n************************************************\n";
        usage();
        exit(-1);
    }

    if ($par2script::globals::scriptname eq "")
    {
        print "\n************************************************\n";
        print "Error: Name of the setup script not set (-o)!";
        print "\n************************************************\n";
        usage();
        exit(-1);
    }

    if ($par2script::globals::parfilelistorig eq "")
    {
        print "\n************************************************\n";
        print "Error: List of par files not set!";
        print "\n************************************************\n";
        usage();
        exit(-1);
    }

    # The par file list has to exist

    $par2script::globals::parfilelist = $par2script::globals::parfilelistorig;
    $par2script::globals::parfilelist =~ s/\@\@//;
    par2script::files::check_file($par2script::globals::parfilelist);
}

#####################################
# Writing parameter to shell
#####################################

sub outputparameter
{
    my $outputline = "\n$par2script::globals::prog -i $par2script::globals::includepathlist $par2script::globals::parfilelistorig -o $par2script::globals::scriptname";

    if ($par2script::globals::logging) { $outputline .= " -v"; }

    $outputline .= "\n";

    print $outputline;
}

1;
