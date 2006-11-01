#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: parameter.pm,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: vg $ $Date: 2006-11-01 13:49:04 $
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
    print <<Ende;
---------------------------------------------------------
$pre2par::globals::prog V1.0 (c) Ingo Schmidt 2003
The following parameter are needed:
-s: path to the pre file
-o: path to the par file
-l: path to the ulf file (mlf or jlf file)
-v: log  process (optional)

Example:

perl pre2par.pl -l test.mlf -s readme.pre -o readme.par -v

---------------------------------------------------------
Ende
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

##########################################################
# The path parameters can be relative or absolute.
# This function creates absolute pathes.
##########################################################

sub make_path_absolute
{
    my ($pathref) = @_;

    if ( $pre2par::globals::isunix )
    {
        if (!($$pathref =~ /^\s*\//))   # this is a relative unix path
        {
            $$pathref = cwd() . $pre2par::globals::separator . $$pathref;
        }
    }

    if ( $pre2par::globals::iswin )
    {
        if (!($$pathref =~ /^\s*\w\:/)) # this is a relative windows path
        {
            $$pathref = cwd() . $pre2par::globals::separator . $$pathref;
            $$pathref =~ s/\//\\/g;
        }
    }

    $$pathref =~ s/\Q$pre2par::globals::separator\E\s*$//;  # removing ending slashes
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
