#*************************************************************************
#
#   $RCSfile: parameter.pm,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: svesik $ $Date: 2004-04-20 12:35:41 $
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
    print <<Ende;

--------------------------------------------------------------
$par2script::globals::prog V1.0 (c) Ingo Schmidt 2003
The following parameter are needed:
-i: include pathes, comma separated list
-o: setup script file name
-v: writing logfile.txt (optional)
\@\@list: list of all par files

Example:
 perl par2script.pl -i ..\\wntmsci8\\par\,o\:\\SRX645\\wntmsci8\\par.m24
         \@\@C\:\\DOCUMEN\~1\\is\\LOCALS\~1\\Temp\\mk6pd
         -o ..\\wntmsci8.pro\\bin\\osl\\setup_osl.inf \[-v\]

--------------------------------------------------------------
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

        if ($param eq "-o") { $par2script::globals::scriptname = shift(@ARGV); }
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
        print "Error: Include pathes not set not set (-i)!";
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

##########################################################
# The path parameters can be relative or absolute.
# This function creates absolute pathes.
##########################################################

sub make_path_absolute
{
    my ($pathref) = @_;

    if ( $par2script::globals::isunix )
    {
        if (!($$pathref =~ /^\s*\//))   # this is a relative unix path
        {
            $$pathref = cwd() . $par2script::globals::separator . $$pathref;
        }
    }

    if ( $par2script::globals::iswin )
    {
        if (!($$pathref =~ /^\s*\w\:/)) # this is a relative windows path
        {
            $$pathref = cwd() . $par2script::globals::separator . $$pathref;
            $$pathref =~ s/\//\\/g;
        }
    }

    $$pathref =~ s/\Q$par2script::globals::separator\E\s*$//;   # removing ending slashes
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
