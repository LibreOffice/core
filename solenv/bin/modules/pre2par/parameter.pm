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


package pre2par::parameter;

use Cwd;
use pre2par::files;
use pre2par::globals;
use pre2par::systemactions;

############################################
# Parameter Operations
############################################

sub usage
{
    print <<End;
---------------------------------------------------------
$pre2par::globals::prog
The following parameter are needed:
-s: path to the pre file
-o: path to the par file
-l: path to the ulf file (mlf or jlf file)
-v: log  process (optional)

Example:

perl pre2par.pl -l test.mlf -s readme.pre -o readme.par -v

---------------------------------------------------------
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

        if ($param eq "-s") { $pre2par::globals::prefilename = shift(@ARGV); }
        elsif ($param eq "-o") { $pre2par::globals::parfilename = shift(@ARGV); }
        elsif ($param eq "-l") { $pre2par::globals::langfilename = shift(@ARGV); }
        elsif ($param eq "-v") { $pre2par::globals::logging = 1; }
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
    if ($pre2par::globals::prefilename eq "")
    {
        print "\n************************************************\n";
        print "Error: Name of the input file not set (-s)!";
        print "\n************************************************\n";
        usage();
        exit(-1);
    }

    if ($pre2par::globals::parfilename eq "")
    {
        print "\n************************************************\n";
        print "Error: Name of the output file not set (-o)!";
        print "\n************************************************\n";
        usage();
        exit(-1);
    }

    if (!($pre2par::globals::prefilename =~ /\.pre\s*$/))
    {
        print "\n************************************************\n";
        print "Error: Input file is no .pre file!";
        print "\n************************************************\n";
        usage();
        exit(-1);
    }

    if (!($pre2par::globals::parfilename =~ /\.par\s*$/))
    {
        print "\n************************************************\n";
        print "Error: Output file is no .par file!";
        print "\n************************************************\n";
        usage();
        exit(-1);
    }

    # The input file has to exist

    pre2par::files::check_file($pre2par::globals::prefilename);
}

#####################################
# Writing parameter to shell
#####################################

sub outputparameter
{
    $pre2par::globals::logging ? ($logoption = " -v") : ($logoption = "");
    print "\n$pre2par::globals::prog -l $pre2par::globals::langfilename -s $pre2par::globals::prefilename -o $pre2par::globals::parfilename$logoption\n";

#   print "\n********************************************************\n";
#   print "This is $pre2par::globals::prog, version 1.0\n";
#   print "Input file: $pre2par::globals::prefilename\n";
#   print "Output file: $pre2par::globals::parfilename\n";
#   print "********************************************************\n";
}

1;
