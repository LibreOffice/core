#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: directory.pm,v $
#
#   $Revision: 1.28 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:03:18 $
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

package installer::windows::directory;

use installer::exiter;
use installer::files;
use installer::globals;
use installer::pathanalyzer;
use installer::windows::idtglobal;

##############################################################
# Collecting all directory trees in global hash
##############################################################

sub collectdirectorytrees
{
    my ( $directoryref ) = @_;

    for ( my $i = 0; $i <= $#{$directoryref}; $i++ )
    {
        my $onedir = ${$directoryref}[$i];
        my $styles = "";
        if ( $onedir->{'Styles'} ) { $styles = $onedir->{'Styles'}; }

        if ( $styles ne "" )
        {
            foreach my $treestyle ( keys %installer::globals::treestyles )
            {
                if ( $styles =~ /\b$treestyle\b/ )
                {
                    my $hostname = $onedir->{'HostName'};
                    # -> hostname is the key, the style the value!
                    $installer::globals::hostnametreestyles{$hostname} = $treestyle;
                }
            }
        }
    }
}

##############################################################
# Overwriting global programfilesfolder, if required
##############################################################

sub overwrite_programfilesfolder
{
    my ( $allvariables ) = @_;

    if ( $allvariables->{'PROGRAMFILESFOLDERNAME'} )
    {
        $installer::globals::programfilesfolder = $allvariables->{'PROGRAMFILESFOLDERNAME'};
    }
}

##############################################################
# Adding unique directory names to the directory collection
##############################################################

sub create_unique_directorynames
{
    my ($directoryref) = @_;

    $installer::globals::officeinstalldirectoryset = 0;

    for ( my $i = 0; $i <= $#{$directoryref}; $i++ )
    {
        my $onedir = ${$directoryref}[$i];
        my $uniquename = $onedir->{'HostName'};
        my $styles = "";
        if ( $onedir->{'Styles'} ) { $styles = $onedir->{'Styles'}; }
        # get_path_from_fullqualifiedname(\$uniqueparentname);
        # making /registry/schema/org/openoffice/VCL.xcs to VCL.xcs

        $uniquename =~ s/^\s*//g;               # removing beginning white spaces
        $uniquename =~ s/\s*$//g;               # removing ending white spaces
        $uniquename =~ s/\s//g;                 # removing white spaces
        $uniquename =~ s/\_//g;                 # removing existing underlines
        $uniquename =~ s/\.//g;                 # removing dots in directoryname
        $uniquename =~ s/\Q$installer::globals::separator\E/\_/g;   # replacing slash and backslash with underline

        my $uniqueparentname = $uniquename;

        if ( $uniqueparentname =~ /^\s*(.*)\_(.*?)\s*$/ )   # the underline is now the separator
        {
            $uniqueparentname = $1;
        }
        else
        {
            $uniqueparentname = "INSTALLLOCATION";
        }

        if ( $styles =~ /\bPROGRAMFILESFOLDER\b/ ) { $uniqueparentname = $installer::globals::programfilesfolder; }
        if ( $styles =~ /\bCOMMONFILESFOLDER\b/ ) { $uniqueparentname = $installer::globals::commonfilesfolder; }
        if ( $styles =~ /\bCOMMONAPPDATAFOLDER\b/ ) { $uniqueparentname = $installer::globals::commonappdatafolder; }
        if ( $styles =~ /\bLOCALAPPDATAFOLDER\b/ ) { $uniqueparentname = $installer::globals::localappdatafolder; }


        $uniquename =~ s/\-/\_/g;           # making "-" to "_"
        $uniqueparentname =~ s/\-/\_/g;     # making "-" to "_"

        $onedir->{'uniquename'} = $uniquename;
        $onedir->{'uniqueparentname'} = $uniqueparentname;

        # setting the office installation directory
        if ( $styles =~ /\bOFFICEDIRECTORY\b/ )
        {
            if ( $installer::globals::officeinstalldirectoryset ) { installer::exiter::exit_program("ERROR: Directory with flag OFFICEDIRECTORY alread set: \"$installer::globals::officeinstalldirectory\".", "create_unique_directorynames"); }
            $installer::globals::officeinstalldirectory = $uniquename;
            $installer::globals::officeinstalldirectoryset = 1;
        }

        # setting the bais installation directory
        if ( $styles =~ /\bBASISDIRECTORY\b/ )
        {
            if ( $installer::globals::basisinstalldirectoryset ) { installer::exiter::exit_program("ERROR: Directory with flag BASISDIRECTORY alread set: \"$installer::globals::basisinstalldirectory\".", "create_unique_directorynames"); }
            $installer::globals::basisinstalldirectory = $uniquename;
            $installer::globals::basisinstalldirectoryset = 1;
        }

        # setting the ure installation directory
        if ( $styles =~ /\bUREDIRECTORY\b/ )
        {
            if ( $installer::globals::ureinstalldirectoryset ) { installer::exiter::exit_program("ERROR: Directory with flag UREDIRECTORY alread set: \"$installer::globals::ureinstalldirectory\".", "create_unique_directorynames"); }
            $installer::globals::ureinstalldirectory = $uniquename;
            $installer::globals::ureinstalldirectoryset = 1;
        }
    }
}

#####################################################
# Getting the name of the top level directory. This
# can have only one letter
#####################################################

sub get_last_directory_name
{
    my ($completepathref) = @_;

    if ( $$completepathref =~ /^.*[\/\\](.+?)\s*$/ )
    {
        $$completepathref = $1;
    }
}

#####################################################
# Creating the defaultdir for the file Director.idt
#####################################################

sub create_defaultdir_directorynames
{
    my ($directoryref) = @_;

    my @shortnames = ();
    if ( $installer::globals::prepare_winpatch ) { @shortnames = values(%installer::globals::saved83dirmapping); }

    for ( my $i = 0; $i <= $#{$directoryref}; $i++ )
    {
        my $onedir = ${$directoryref}[$i];
        my $hostname = $onedir->{'HostName'};

        $hostname =~ s/\Q$installer::globals::separator\E\s*$//;
        get_last_directory_name(\$hostname);
        # installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$hostname); # making program/classes to classes
        my $uniquename = $onedir->{'uniquename'};
        my $shortstring;
        if (( $installer::globals::prepare_winpatch ) && ( exists($installer::globals::saved83dirmapping{$uniquename}) ))
        {
            $shortstring = $installer::globals::saved83dirmapping{$uniquename};
        }
        else
        {
            $shortstring = installer::windows::idtglobal::make_eight_three_conform($hostname, "dir", \@shortnames);
        }

        my $defaultdir;

        if ( $shortstring eq $hostname )
        {
            $defaultdir = $hostname;
        }
        else
        {
            $defaultdir = $shortstring . "|" . $hostname;
        }

        $onedir->{'defaultdir'} = $defaultdir;

        my $fontdir = "";
        if ( $onedir->{'Dir'} ) { $fontdir = $onedir->{'Dir'}; }

        my $fontdefaultdir = "";
        if ( $onedir->{'defaultdir'} ) { $fontdefaultdir = $onedir->{'defaultdir'}; }

        if (( $fontdir eq "PREDEFINED_OSSYSTEMFONTDIR" ) && ( $fontdefaultdir eq $installer::globals::fontsdirhostname ))
        {
            $installer::globals::fontsdirname = $onedir->{'defaultdir'};
            $installer::globals::fontsdirparent = $onedir->{'uniqueparentname'};
        }
    }
}

###############################################
# Fill content into the directory table
###############################################

sub create_directorytable_from_collection
{
    my ($directorytableref, $directoryref) = @_;

    for ( my $i = 0; $i <= $#{$directoryref}; $i++ )
    {
        my $onedir = ${$directoryref}[$i];
        my $hostname = $onedir->{'HostName'};
        my $dir = "";

        if ( $onedir->{'Dir'} ) { $dir = $onedir->{'Dir'}; }

        if (( $dir eq "PREDEFINED_PROGDIR" ) && ( $hostname eq "" )) { next; }  # removing files from root directory

        my $oneline = $onedir->{'uniquename'} . "\t" . $onedir->{'uniqueparentname'} . "\t" . $onedir->{'defaultdir'} . "\n";

        push(@{$directorytableref}, $oneline);
    }
}

###############################################
# Defining the root installation structure
###############################################

sub add_root_directories
{
    my ($directorytableref, $allvariableshashref) = @_;

    my $oneline = "TARGETDIR\t\tSourceDir\n";
    push(@{$directorytableref}, $oneline);

    my $sourcediraddon = "";
    if (($installer::globals::addchildprojects) ||
        ($installer::globals::patch) ||
        ($installer::globals::languagepack) ||
        ($allvariableshashref->{'CHANGETARGETDIR'}))
    {
        $sourcediraddon = "\:\.";
    }

    if (!($installer::globals::product =~ /ada/i )) # the following directories not for ada products
    {
        $oneline = "$installer::globals::programfilesfolder\tTARGETDIR\t.\n";
        push(@{$directorytableref}, $oneline);

        # my $manufacturer = $installer::globals::manufacturer;
        # my $shortmanufacturer = installer::windows::idtglobal::make_eight_three_conform($manufacturer, "dir");    # third parameter not used
        # $shortmanufacturer =~ s/\s/\_/g;                                  # changing empty space to underline

        my $productname = $allvariableshashref->{'PRODUCTNAME'};
        my $productversion = $allvariableshashref->{'PRODUCTVERSION'};
        my $baseproductversion = $productversion;

        if (( $installer::globals::prepare_winpatch ) && ( $allvariableshashref->{'BASEPRODUCTVERSION'} ))
        {
            $baseproductversion = $allvariableshashref->{'BASEPRODUCTVERSION'};  # for example "2.0" for OOo
        }

        my $realproductkey = $productname . " " . $productversion;
        my $productkey = $productname . " " . $baseproductversion;

        if (( $allvariableshashref->{'POSTVERSIONEXTENSION'} ) && ( ! $allvariableshashref->{'DONTUSEEXTENSIONINDEFAULTDIR'} ))
        {
            $productkey = $productkey . " " . $allvariableshashref->{'POSTVERSIONEXTENSION'};
            $realproductkey = $realproductkey . " " . $allvariableshashref->{'POSTVERSIONEXTENSION'};
        }
        if ( $allvariableshashref->{'NOVERSIONINDIRNAME'} )
        {
            $productkey = $productname;
            $realproductkey = $realproductname;
        }
        if ( $allvariableshashref->{'NOSPACEINDIRECTORYNAME'} )
        {
            $productkey =~ s/\ /\_/g;
            $realproductkey =~ s/\ /\_/g;
        }

        my $shortproductkey = installer::windows::idtglobal::make_eight_three_conform($productkey, "dir");      # third parameter not used
        $shortproductkey =~ s/\s/\_/g;                                  # changing empty space to underline

        if ( $allvariableshashref->{'SUNDIR'} )
        {
            $oneline = "sundirectory\t$installer::globals::programfilesfolder\t$installer::globals::sundirname$sourcediraddon\n";
            push(@{$directorytableref}, $oneline);

            $oneline = "INSTALLLOCATION\tsundirectory\t$shortproductkey|$productkey$sourcediraddon\n";
            push(@{$directorytableref}, $oneline);
        }
        else
        {
            if ( $allvariableshashref->{'PROGRAMFILESROOT'} )
            {
                $oneline = "INSTALLLOCATION\t$installer::globals::programfilesfolder\t.\n";
            }
            else
            {
                $oneline = "INSTALLLOCATION\t$installer::globals::programfilesfolder\t$shortproductkey|$productkey$sourcediraddon\n";
            }

            push(@{$directorytableref}, $oneline);
        }

        $oneline = "$installer::globals::programmenufolder\tTARGETDIR\t.\n";
        push(@{$directorytableref}, $oneline);

        if (( ! $installer::globals::patch ) && ( ! $installer::globals::languagepack ) && ( ! $allvariableshashref->{'DONTUSESTARTMENUFOLDER'} ))
        {
            $oneline = "$installer::globals::officemenufolder\t$installer::globals::programmenufolder\t$shortproductkey|$realproductkey\n";
            push(@{$directorytableref}, $oneline);
        }

        $oneline = "$installer::globals::startupfolder\tTARGETDIR\t.\n";
        push(@{$directorytableref}, $oneline);

        $oneline = "$installer::globals::desktopfolder\tTARGETDIR\t.\n";
        push(@{$directorytableref}, $oneline);

        $oneline = "$installer::globals::startmenufolder\tTARGETDIR\t.\n";
        push(@{$directorytableref}, $oneline);

        $oneline = "$installer::globals::commonfilesfolder\tTARGETDIR\t.\n";
        push(@{$directorytableref}, $oneline);

        $oneline = "$installer::globals::commonappdatafolder\tTARGETDIR\t.\n";
        push(@{$directorytableref}, $oneline);

        $oneline = "$installer::globals::localappdatafolder\tTARGETDIR\t.\n";
        push(@{$directorytableref}, $oneline);

        my $localtemplatefoldername = $installer::globals::templatefoldername;
        my $directorytableentry = $localtemplatefoldername;
        my $shorttemplatefoldername = installer::windows::idtglobal::make_eight_three_conform($localtemplatefoldername, "dir");
        if ( $shorttemplatefoldername ne $localtemplatefoldername ) { $directorytableentry = "$shorttemplatefoldername|$localtemplatefoldername"; }
        $oneline = "$installer::globals::templatefolder\tTARGETDIR\t$directorytableentry\n";
        push(@{$directorytableref}, $oneline);

        if ( $installer::globals::fontsdirname )
        {
            $oneline = "$installer::globals::fontsfolder\t$installer::globals::fontsdirparent\t$installer::globals::fontsfoldername\:$installer::globals::fontsdirname\n";
        }
        else
        {
            $oneline = "$installer::globals::fontsfolder\tTARGETDIR\t$installer::globals::fontsfoldername\n";
        }

        push(@{$directorytableref}, $oneline);
    }

}

###############################################
# Creating the file Director.idt dynamically
###############################################

sub create_directory_table
{
    my ($directoryref, $basedir, $allvariableshashref) = @_;

    # Structure of the directory table:
    # Directory Directory_Parent DefaultDir
    # Directory is a unique identifier
    # Directory_Parent is the unique identifier of the parent
    # DefaultDir is .:APPLIC~1|Application Data with
    # Before ":" : [sourcedir]:[destdir] (not programmed yet)
    # After ":" : 8+3 and not 8+3 the destination directory name

    my @directorytable = ();
    my $infoline;

    overwrite_programfilesfolder($allvariableshashref);
    create_unique_directorynames($directoryref);
    create_defaultdir_directorynames($directoryref);    # only destdir!
    installer::windows::idtglobal::write_idt_header(\@directorytable, "directory");
    add_root_directories(\@directorytable, $allvariableshashref);
    create_directorytable_from_collection(\@directorytable, $directoryref);

    # Saving the file

    my $directorytablename = $basedir . $installer::globals::separator . "Director.idt";
    installer::files::save_file($directorytablename ,\@directorytable);
    $infoline = "Created idt file: $directorytablename\n";
    push(@installer::globals::logfileinfo, $infoline);

}

1;
