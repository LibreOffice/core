#*************************************************************************
#
#   $RCSfile: ziplist.pm,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2004-07-30 16:37:25 $
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

package installer::ziplist;

use installer::existence;
use installer::exiter;
use installer::globals;
use installer::logger;
use installer::parameter;
use installer::remover;

#################################################
# Getting data from path file and zip list file
#################################################

sub getproductblock
{
    my ($fileref, $search) = @_;

    my @searchblock = ();
    my $searchexists = 0;
    my $record = 0;
    my $count = 0;
    my $line;

    for ( my $i = 0; $i <= $#{$fileref}; $i++ )
    {
        $line = ${$fileref}[$i];

        if ( $line =~ /^\s*\Q$search\E\s*$/i )      # case insensitive
        {
            $record = 1;
            $searchexists = 1;
        }

        if ($record)
        {
            push(@searchblock, $line);
        }

        if ( ($record) && ($line =~ /\{/) )
        {
            $count++;
        }

        if ( ($record) && ($line =~ /\}/) )
        {
            $count--;
        }

        if ( ($record) && ($line =~ /\}/) && ( $count == 0 ) )
        {
            $record = 0;
        }
    }

    if ( ! $searchexists )
    {
        if ($search eq $installer::globals::product )
        {
            installer::exiter::exit_program("ERROR: Product $installer::globals::product not defined in $installer::globals::ziplistname", "getproductblock");
        }
        elsif ($search eq $installer::globals::compiler )
        {
            installer::exiter::exit_program("ERROR: Compiler $installer::globals::compiler not defined in $installer::globals::pathfilename", "getproductblock");
        }
        else    # this is not possible
        {
            installer::exiter::exit_program("ERROR: Unknown value for $search in getproductblock()", "getproductblock");
        }
    }

    return \@searchblock;
}

###############################################
# Analyzing the settings in the zip list file
###############################################

sub analyze_settings_block
{
    my ($blockref) = @_;

    my @newsettingsblock = ();
    my $compilerstring = "";
    my $record = 1;
    my $counter = 0;

    # Allowed values in settings block:
    # "Settings", "Variables", "unix" (for destination path and logfile)
    # Furthermore allowed values are $installer::globals::build (srx645) and $installer::globals::compiler (pro and nonpro (unxsols4.pro))

    # Comment line in settings block begin with "#" or ";"

    if ( $installer::globals::pro )
    {
        $compilerstring = $installer::globals::compiler . ".pro";
    }
    else
    {
        $compilerstring = $installer::globals::compiler;
    }

    for ( my $i = 0; $i <= $#{$blockref}; $i++ )
    {
        my $line = ${$blockref}[$i];
        my $nextline = "";

        if ( ${$blockref}[$i+1] ) { $nextline = ${$blockref}[$i+1]; }

        # removing comment lines

        if (($line =~ /^\s*\#/) || ($line =~ /^\s*\;/))
        {
            next;
        }

        # complete blocks of unknows strings are not recorded

        if ((!($line =~ /^\s*\Q$compilerstring\E\s*$/i)) &&
            (!($line =~ /^\s*\Q$installer::globals::build\E\s*$/i)) &&
            (!($line =~ /^\s*\bSettings\b\s*$/i)) &&
            (!($line =~ /^\s*\bVariables\b\s*$/i)) &&
            (!($line =~ /^\s*\bunix\b\s*$/i)) &&
            ($nextline =~ /^\s*\{\s*$/i))
        {
            $record = 0;
            next;           # continue with next $i
        }

        if (!( $record ))
        {
            if ($line =~ /^\s*\{\s*$/i)
            {
                $counter++;
            }

            if ($line =~ /^\s*\}\s*$/i)
            {
                $counter--;
            }

            if ($counter == 0)
            {
                $record = 1;
                next;   # continue with next $i
            }
        }

        if ($record)
        {
            push(@newsettingsblock, $line);
        }
    }

    return \@newsettingsblock;
}

########################################
# Settings in zip list file
########################################

sub get_settings_from_ziplist
{
    my ($blockref) = @_;

    my @allsettings = ();
    my $isvariables = 0;
    my $counter = 0;
    my $variablescounter = 0;

    # Take all settings from the settings block
    # Do not take the variables from the settings block
    # If a setting is defined more than once, take the
    # setting with the largest counter (open brackets)

    for ( my $i = 0; $i <= $#{$blockref}; $i++ )
    {
        my $line = ${$blockref}[$i];
        my $nextline = "";

        if ( ${$blockref}[$i+1] ) { $nextline = ${$blockref}[$i+1]; }

        if (($line =~ /^\s*\S+\s*$/i) &&
            ($nextline =~ /^\s*\{\s*$/i) &&
            (!($line =~ /^\s*Variables\s*$/i)))
        {
            next;
        }

        if ($line =~ /^\s*Variables\s*$/i)
        {
            # This is a block of variables

            $isvariables = 1;
            next;
        }

        if ($line =~ /^\s*\{\s*$/i)
        {
            if ($isvariables)
            {
                $variablescounter++;
            }
            else
            {
                $counter++;
            }

            next;
        }

        if ($line =~ /^\s*\}\s*$/i)
        {
            if ($isvariables)
            {
                $variablescounter--;

                if ($variablescounter == 0)
                {
                    $isvariables = 0;
                }
            }
            else
            {
                $counter--;
            }

            next;
        }

        if ($isvariables)
        {
            next;
        }

        installer::remover::remove_leading_and_ending_whitespaces(\$line);

        $line .= "\t##$counter##\n";

        push(@allsettings, $line);
    }

    return \@allsettings;
}

#######################################
# Variables from zip list file
#######################################

sub get_variables_from_ziplist
{
    my ($blockref) = @_;

    my @allvariables = ();
    my $isvariables = 0;
    my $counter = 0;
    my $variablescounter = 0;
    my $countersum = 0;

    # Take all variables from the settings block
    # Do not take the other settings from the settings block
    # If a variable is defined more than once, take the
    # variable with the largest counter (open brackets)

    for ( my $i = 0; $i <= $#{$blockref}; $i++ )
    {
        my $line = ${$blockref}[$i];
        my $nextline = ${$blockref}[$i+1];

        if ($line =~ /^\s*Variables\s*$/i)
        {
            # This is a block of variables

            $isvariables = 1;
            next;
        }

        if ($line =~ /^\s*\{\s*$/i)
        {
            if ($isvariables)
            {
                $variablescounter++;
            }
            else
            {
                $counter++;
            }

            next;
        }

        if ($line =~ /^\s*\}\s*$/i)
        {
            if ($isvariables)
            {
                $variablescounter--;

                if ($variablescounter == 0)
                {
                    $isvariables = 0;
                }
            }
            else
            {
                $counter--;
            }

            next;
        }

        if (!($isvariables))
        {
            next;
        }

        $countersum = $counter + $variablescounter;

        installer::remover::remove_leading_and_ending_whitespaces(\$line);

        $line .= "\t##$countersum##\n";

        push(@allvariables, $line);
    }

    return \@allvariables;
}

#######################################################################
# Removing multiple variables and settings, defined in zip list file
#######################################################################

sub remove_multiples_from_ziplist
{
    my ($blockref) = @_;

    # remove all definitions of settings and variables
    # that occur more than once in the zip list file.
    # Take the one with the most open brackets. This
    # number is stored at the end of the string.

    my @newarray = ();
    my @itemarray = ();
    my ($line, $itemname, $itemnumber);

    # first collecting all variables and settings names

    for ( my $i = 0; $i <= $#{$blockref}; $i++ )
    {
        $line = ${$blockref}[$i];

        if ($line =~ /^\s*\b(\S*)\b\s+.*\#\#\d+\#\#\s*$/i)
        {
            $itemname = $1;
        }

        if (! installer::existence::exists_in_array($itemname, \@itemarray))
        {
            push(@itemarray, $itemname);
        }
    }

    # and now all $items can be selected with the highest number

    for ( my $i = 0; $i <= $#itemarray; $i++ )
    {
        $itemname = $itemarray[$i];

        my $itemnumbermax = 0;
        my $printline = "";

        for ( my $j = 0; $j <= $#{$blockref}; $j++ )
        {
            $line = ${$blockref}[$j];

            if ($line =~ /^\s*\Q$itemname\E\s+.*\#\#(\d+)\#\#\s*$/)
            {
                $itemnumber = $1;

                if ($itemnumber >= $itemnumbermax)
                {
                    $printline = $line;
                    $itemnumbermax = $itemnumber;
                }
            }
        }

        # removing the ending number from the printline
        # and putting it into the array

        $printline =~ s/\#\#\d+\#\#//;
        installer::remover::remove_leading_and_ending_whitespaces(\$line);
        push(@newarray, $printline);
    }

    return \@newarray;
}

#########################################################
# Reading one variable defined in the zip list file
#########################################################

sub getinfofromziplist
{
    my ($blockref, $variable) = @_;

    my $searchstring = "";
    my $line;

    for ( my $i = 0; $i <= $#{$blockref}; $i++ )
    {
        $line = ${$blockref}[$i];

        if ( $line =~ /^\s*\Q$variable\E\s+(.+?)\s*$/ ) # "?" for minimal matching
        {
            $searchstring = $1;
            last;
        }
    }

    return \$searchstring;
}

#######################################################
# Set zip list file name, if not defined as parameter
#######################################################

sub set_ziplist_name
{
    my ( $pathvariableshashref ) = @_;

    my $solarenvpath = $pathvariableshashref->{'solarenvpath'};

    $solarenvpath =~ s/solenv\Q$installer::globals::separator\Einst//;

    $installer::globals::ziplistname = $solarenvpath . "b_server" . $installer::globals::separator . "zip" . $installer::globals::separator . "zip.lst";

}

####################################################
# Replacing variables in include path
####################################################

sub replace_all_variables_in_pathes
{
    my ( $patharrayref, $variableshashref ) = @_;

    for ( my $i = 0; $i <= $#{$patharrayref}; $i++ )
    {
        my $line = ${$patharrayref}[$i];

        my $key;

        foreach $key (keys %{$variableshashref})
        {
            my $value = $variableshashref->{$key};

            if (( $line =~ /\{$key\}/ ) && ( $value eq "" )) { $line = ".\n"; }

            $line =~ s/\{\Q$key\E\}/$value/g;
        }

        ${$patharrayref}[$i] = $line;
    }
}

####################################################
# Replacing minor in include path
####################################################

sub replace_minor_in_pathes
{
    my ( $patharrayref ) = @_;

    for ( my $i = 0; $i <= $#{$patharrayref}; $i++ )
    {
        my $line = ${$patharrayref}[$i];

        if ( $installer::globals::minor )
        {
            $line =~ s/\{minor\}/$installer::globals::minor/g;
            # no difference for minor and minornonpre (ToDo ?)
            $line =~ s/\{minornonpre\}/$installer::globals::minor/g;
        }
        else    # building without a minor
        {
            $line =~ s/\.\{minor\}//g;
            $line =~ s/\.\{minornonpre\}//g;
        }

        ${$patharrayref}[$i] = $line;
    }
}

####################################################
# Removing ending separators in pathes
####################################################

sub remove_ending_separator
{
    my ( $patharrayref ) = @_;

    for ( my $i = 0; $i <= $#{$patharrayref}; $i++ )
    {
        my $line = ${$patharrayref}[$i];

        installer::remover::remove_ending_pathseparator(\$line);

        $line =~ s/\s*$//;
        $line = $line . "\n";

        ${$patharrayref}[$i] = $line;
    }
}

####################################################
# Replacing languages in include path
####################################################

sub replace_languages_in_pathes
{
    my ( $patharrayref, $languagesref ) = @_;

    my @patharray = ();

    for ( my $i = 0; $i <= $#{$patharrayref}; $i++ )
    {
        my $line = ${$patharrayref}[$i];

        if ( $line =~ /\$\(LANG\)/ )
        {
            my $originalline = $line;

            for ( my $j = 0; $j <= $#{$languagesref}; $j++ )
            {
                my $language = ${$languagesref}[$j];
                $line =~ s/\$\(LANG\)/$language/g;
                push(@patharray ,$line);
                $line = $originalline;
            }
        }
        else        # not language dependent include path
        {
            push(@patharray ,$line);
        }
    }

    return \@patharray;
}

#####################################################
# Collecting all files from all include paths
#####################################################

sub collect_all_files_from_include_path
{
    my ( $patharrayref) = @_;

    my @filesarray = ();

    installer::logger::include_header_into_logfile("Include pathes:");

    for ( my $i = 0; $i <= $#{$patharrayref}; $i++ )
    {
        my $count = 0;

        my $path = ${$patharrayref}[$i];

        installer::remover::remove_leading_and_ending_whitespaces(\$path);

        opendir(DIR, $path);

        my $direntry;

        foreach $direntry (readdir(DIR))
        {
            my $completefile = $path . $installer::globals::separator . $direntry;

            if ( -f $completefile )
            {
                $completefile .= "\n";
                push(@filesarray, $completefile);
                $count++;
            }
        }

        closedir(DIR);

        my $infoline = "$path\t$count files\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    # one empty line after directory section
    $infoline = "\n";
    push( @installer::globals::logfileinfo, $infoline);

    return \@filesarray;
}

#####################################################
# Collecting all files from all include paths
#####################################################

sub set_manufacturer
{
    my ($allvariables) = @_;

    my $openofficeproductname = "OpenOffice.org";
    my $sunname = "Sun Microsystems";

    if ( $allvariables->{'PRODUCTNAME'} eq $openofficeproductname )
    {
        $installer::globals::isopensourceproduct = 1;
        $installer::globals::manufacturer = $openofficeproductname;
    }
    else
    {
        $installer::globals::isopensourceproduct = 0;
        $installer::globals::manufacturer = $sunname;
    }
}

##############################################################
# A ProductVersion has to be defined. If it is not set in
# zip.lst, it is set now to "1"
##############################################################

sub set_default_productversion_if_required
{
    my ($allvariables) = @_;

    if (!($allvariables->{'PRODUCTVERSION'}))
    {
        $allvariables->{'PRODUCTVERSION'} = 1;  # FAKE
    }
}

####################################################
# Removing .. in pathes
####################################################

sub simplify_path
{
    my ( $pathref ) = @_;

    my $oldpath = $$pathref;

    my $change = 0;

    while ( $oldpath =~ /(^.*)(\Q$installer::globals::separator\E.*\w+?)(\Q$installer::globals::separator\E\.\.)(\Q$installer::globals::separator\E.*$)/ )
    {
        my $part1 = $1;
        my $part2 = $4;
        $oldpath = $part1 . $part2;
        $change = 1;
    }

    if ( $change ) { $$pathref = $oldpath . "\n"; }
}

####################################################
# Removing ending separators in pathes
####################################################

sub resolve_relative_pathes
{
    my ( $patharrayref ) = @_;

    for ( my $i = 0; $i <= $#{$patharrayref}; $i++ )
    {
        installer::parameter::make_path_absolute(\${$patharrayref}[$i]);
        simplify_path(\${$patharrayref}[$i]);
    }
}

####################################################
# Replacing variables inside zip list variables
# Example: {milestone} to be replaced by
# $installer::globals::lastminor
####################################################

sub replace_variables_in_ziplist_variables
{
    my ($blockref) = @_;

    my $milestonevariable = $installer::globals::lastminor;
    $milestonevariable =~ s/m//;
    $milestonevariable =~ s/s/\./;

    for ( my $i = 0; $i <= $#{$blockref}; $i++ )
    {
        if ($installer::globals::lastminor) { ${$blockref}[$i] =~ s/\{milestone\}/$milestonevariable/; }
    }
}

1;
