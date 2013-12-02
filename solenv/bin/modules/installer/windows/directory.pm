#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************



package installer::windows::directory;

use installer::exiter;
use installer::files;
use installer::globals;
use installer::pathanalyzer;
use installer::windows::idtglobal;
use installer::windows::msiglobal;

use strict;

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




=head2 make_short_dir_version($longstring)

    Transform the given string into one that is at most 70 characters long.
    That is done in two steps:
    - Cut all parts separated by '_' or '-' down to a length of 5.
    - Cut down the result to a length of 60 and fill it up to length 70
      with the MD5 checksum.

    This transform always returns the same result for the same string.
    There is no counter and reference to a global set of names to make the string unique.

=cut
sub make_short_dir_version ($)
{
    my ($longstring) = @_;

    my $shortstring = "";
    my $cutlength = 60;
    my $length = 5; # So the directory can still be recognized
    my $longstring_save = $longstring;

    # Splitting the string at each "underline" and allowing only $length characters per directory name.
    # Checking also uniqueness and length.

    my @outer_parts = split(/_/, $longstring);
    foreach my $onestring (@outer_parts)
    {
        my $partstring = "";

        if ( $onestring =~ /\-/ )
        {
            my @inner_parts = split(/-/, $onestring);
            @inner_parts = map {substr($_,0,$length)} @inner_parts;
            $partstring = join("-", @inner_parts);
            $partstring =~ s/^\s*\-//;
        }
        else
        {
            $partstring = substr($onestring, 0, $length);
        }

        $shortstring .= "_" . $partstring;
    }

    $shortstring =~ s/^\s*\_//;

    # Setting unique ID to each directory
    # No counter allowed, process must be absolute reproducable due to patch creation process.

    my $subid = installer::windows::msiglobal::calculate_id($longstring_save, 9); # taking only the first 9 digits
    $shortstring = substr($shortstring, 0, $cutlength) . "_" . $subid;

    return $shortstring;
}

##############################################################
# Adding unique directory names to the directory collection
##############################################################

sub create_unique_directorynames
{
    my ($directoryref, $allvariables) = @_;

    $installer::globals::officeinstalldirectoryset = 0;

    my %completedirhashstep1 = ();
    my %shortdirhash = ();
    my %shortdirhashreverse = ();
    my $infoline = "";
    my $errorcount = 0;

    for ( my $i = 0; $i <= $#{$directoryref}; $i++ )
    {
        my $onedir = ${$directoryref}[$i];
        my $uniquename = $onedir->{'HostName'};

        my $styles = "";
        if ( $onedir->{'Styles'} ) { $styles = $onedir->{'Styles'}; }

        $uniquename =~ s/^\s*//g;               # removing beginning white spaces
        $uniquename =~ s/\s*$//g;               # removing ending white spaces
        $uniquename =~ s/\s//g;                 # removing white spaces
        $uniquename =~ s/\_//g;                 # removing existing underlines
        $uniquename =~ s/\.//g;                 # removing dots in directoryname
        $uniquename =~ s/OpenOffice/OO/g;

        $uniquename =~ s/\Q$installer::globals::separator\E/\_/g;   # replacing slash and backslash with underline

        $uniquename =~ s/_registry/_rgy/g;
        $uniquename =~ s/_registration/_rgn/g;
        $uniquename =~ s/_extension/_ext/g;
        $uniquename =~ s/_frame/_frm/g;
        $uniquename =~ s/_table/_tbl/g;
        $uniquename =~ s/_chart/_crt/g;

        # The names after this small changes must still be unique!
        if ( exists($completedirhashstep1{$uniquename}) ) { installer::exiter::exit_program("ERROR: Error in packaging process. Unallowed modification of directory name, not unique (step 1): \"$uniquename\".", "create_unique_directorynames"); }
        $completedirhashstep1{$uniquename} = 1;

        # Starting to make unique name for the parent and its directory
        my $originaluniquename = $uniquename;

        $uniquename = make_short_dir_version($uniquename);

        # Checking if the same directory already exists, but has another short version.
        if (( exists($shortdirhash{$originaluniquename}) ) && ( $shortdirhash{$originaluniquename} ne $uniquename )) { installer::exiter::exit_program("ERROR: Error in packaging process. Unallowed modification of directory name, not unique (step 2A): \"$uniquename\".", "create_unique_directorynames"); }

        # Also checking vice versa
        # Checking if the same short directory already exists, but has another long version.
        if (( exists($shortdirhashreverse{$uniquename}) ) && ( $shortdirhashreverse{$uniquename} ne $originaluniquename )) { installer::exiter::exit_program("ERROR: Error in packaging process. Unallowed modification of directory name, not unique (step 2B): \"$uniquename\".", "create_unique_directorynames"); }

        # Creating assignment from long to short directory names
        $shortdirhash{$originaluniquename} = $uniquename;
        $shortdirhashreverse{$uniquename} = $originaluniquename;

        # Important: The unique parent is generated from the string $originaluniquename (with the use of underlines).

        my $uniqueparentname = $originaluniquename;
        my $keepparent = 1;

        if ( $uniqueparentname =~ /^\s*(.*)\_(.*?)\s*$/ )   # the underline is now the separator
        {
            $uniqueparentname = $1;
            $keepparent = 0;
        }
        else
        {
            $uniqueparentname = $installer::globals::programfilesfolder;
            $keepparent = 1;
        }

        if ( $styles =~ /\bPROGRAMFILESFOLDER\b/ )
        {
            $uniqueparentname = $installer::globals::programfilesfolder;
            $keepparent = 1;
        }
        if ( $styles =~ /\bCOMMONFILESFOLDER\b/ )
        {
            $uniqueparentname = $installer::globals::commonfilesfolder;
            $keepparent = 1;
        }
        if ( $styles =~ /\bCOMMONAPPDATAFOLDER\b/ )
        {
            $uniqueparentname = $installer::globals::commonappdatafolder;
            $keepparent = 1;
        }
        if ( $styles =~ /\bLOCALAPPDATAFOLDER\b/ )
        {
            $uniqueparentname = $installer::globals::localappdatafolder;
            $keepparent = 1;
        }

        if ( $styles =~ /\bSHAREPOINTPATH\b/ )
        {
            $uniqueparentname = "SHAREPOINTPATH";
            $installer::globals::usesharepointpath = 1;
            $keepparent = 1;
        }

        # also setting short directory name for the parent

        my $originaluniqueparentname = $uniqueparentname;

        if ( ! $keepparent )
        {
            $uniqueparentname = make_short_dir_version($uniqueparentname);
        }

        # Again checking if the same directory already exists, but has another short version.
        if (( exists($shortdirhash{$originaluniqueparentname}) ) && ( $shortdirhash{$originaluniqueparentname} ne $uniqueparentname )) { installer::exiter::exit_program("ERROR: Error in packaging process. Unallowed modification of directory name, not unique (step 3A): \"$uniqueparentname\".", "create_unique_directorynames"); }

        # Also checking vice versa
        # Checking if the same short directory already exists, but has another long version.
        if (( exists($shortdirhashreverse{$uniqueparentname}) ) && ( $shortdirhashreverse{$uniqueparentname} ne $originaluniqueparentname )) { installer::exiter::exit_program("ERROR: Error in packaging process. Unallowed modification of directory name, not unique (step 3B): \"$uniqueparentname\".", "create_unique_directorynames"); }

        $shortdirhash{$originaluniqueparentname} = $uniqueparentname;
        $shortdirhashreverse{$uniqueparentname} = $originaluniqueparentname;

        # Hyphen not allowed in database
        $uniquename =~ s/\-/\_/g;           # making "-" to "_"
        $uniqueparentname =~ s/\-/\_/g;     # making "-" to "_"

        # And finally setting the values for the directories
        $onedir->{'uniquename'} = $uniquename;
        $onedir->{'uniqueparentname'} = $uniqueparentname;

        # setting the installlocation directory
        if ( $styles =~ /\bISINSTALLLOCATION\b/ )
        {
            if ( $installer::globals::installlocationdirectoryset ) { installer::exiter::exit_program("ERROR: Directory with flag ISINSTALLLOCATION alread set: \"$installer::globals::installlocationdirectory\".", "create_unique_directorynames"); }
            $installer::globals::installlocationdirectory = $uniquename;
            $installer::globals::installlocationdirectoryset = 1;
        }

        # setting the sundirectory
        if ( $styles =~ /\bSUNDIRECTORY\b/ )
        {
            if ( $installer::globals::vendordirectoryset ) { installer::exiter::exit_program("ERROR: Directory with flag SUNDIRECTORY alread set: \"$installer::globals::vendordirectory\".", "create_unique_directorynames"); }
            $installer::globals::vendordirectory = $uniquename;
            $installer::globals::vendordirectoryset = 1;
        }
    }
}

#####################################################
# Adding ":." to selected default directory names
#####################################################

sub check_sourcedir_addon
{
    my ( $onedir, $allvariableshashref ) = @_;

    if (($installer::globals::addchildprojects) ||
        ($installer::globals::patch) ||
        ($installer::globals::languagepack) ||
        ($allvariableshashref->{'CHANGETARGETDIR'}))
    {
        my $sourcediraddon = "\:\.";
        $onedir->{'defaultdir'} = $onedir->{'defaultdir'} . $sourcediraddon;
    }

}

#####################################################
# The directory with the style ISINSTALLLOCATION
# will be replaced by INSTALLLOCATION
#####################################################

sub set_installlocation_directory
{
    my ( $directoryref, $allvariableshashref ) = @_;

    if ( ! $installer::globals::installlocationdirectoryset ) { installer::exiter::exit_program("ERROR: Directory with flag ISINSTALLLOCATION not set!", "set_installlocation_directory"); }

    for ( my $i = 0; $i <= $#{$directoryref}; $i++ )
    {
        my $onedir = ${$directoryref}[$i];

        if ( $onedir->{'uniquename'} eq $installer::globals::installlocationdirectory )
        {
            $onedir->{'uniquename'} = "INSTALLLOCATION";
            check_sourcedir_addon($onedir, $allvariableshashref);
        }

        if ( $onedir->{'uniquename'} eq $installer::globals::vendordirectory )
        {
            check_sourcedir_addon($onedir, $allvariableshashref);
        }

        if ( $onedir->{'uniqueparentname'} eq $installer::globals::installlocationdirectory )
        {
            $onedir->{'uniqueparentname'} = "INSTALLLOCATION";
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

sub create_defaultdir_directorynames ($)
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

#   my $sourcediraddon = "";
#   if (($installer::globals::addchildprojects) ||
#       ($installer::globals::patch) ||
#       ($installer::globals::languagepack) ||
#       ($allvariableshashref->{'CHANGETARGETDIR'}))
#   {
#       $sourcediraddon = "\:\.";
#   }

    my $oneline = "";

    if (( ! $installer::globals::patch ) && ( ! $installer::globals::languagepack ) && ( ! $allvariableshashref->{'DONTUSESTARTMENUFOLDER'} ))
    {
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
        if ( $allvariableshashref->{'NOSPACEINDIRECTORYNAME'} )
        {
            $productkey =~ s/\ /\_/g;
            $realproductkey =~ s/\ /\_/g;
        }

        my $shortproductkey = installer::windows::idtglobal::make_eight_three_conform($productkey, "dir");      # third parameter not used
        $shortproductkey =~ s/\s/\_/g;                                  # changing empty space to underline

        $oneline = "$installer::globals::officemenufolder\t$installer::globals::programmenufolder\t$shortproductkey|$realproductkey\n";
        push(@{$directorytableref}, $oneline);
    }

    $oneline = "TARGETDIR\t\tSourceDir\n";
    push(@{$directorytableref}, $oneline);

    $oneline = "$installer::globals::programfilesfolder\tTARGETDIR\t.\n";
    push(@{$directorytableref}, $oneline);

    $oneline = "$installer::globals::programmenufolder\tTARGETDIR\t.\n";
    push(@{$directorytableref}, $oneline);

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

    if ( $installer::globals::usesharepointpath )
    {
        $oneline = "SHAREPOINTPATH\tTARGETDIR\t.\n";
        push(@{$directorytableref}, $oneline);
    }

    $oneline = "$installer::globals::systemfolder\tTARGETDIR\t.\n";
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

###############################################
# Creating the file Director.idt dynamically
###############################################

sub create_directory_table ($$$$)
{
    my ($directoryref, $basedir, $allvariableshashref, $loggingdir) = @_;

    # Structure of the directory table:
    # Directory Directory_Parent DefaultDir
    # Directory is a unique identifier
    # Directory_Parent is the unique identifier of the parent
    # DefaultDir is .:APPLIC~1|Application Data with
    # Before ":" : [sourcedir]:[destdir] (not programmed yet)
    # After ":" : 8+3 and not 8+3 the destination directory name

    $installer::logger::Lang->add_timestamp("Performance Info: Directory Table start");

    my @directorytable = ();
    my $infoline;

    overwrite_programfilesfolder($allvariableshashref);
    create_unique_directorynames($directoryref, $allvariableshashref);
    create_defaultdir_directorynames($directoryref);    # only destdir!
    set_installlocation_directory($directoryref, $allvariableshashref);
    installer::windows::idtglobal::write_idt_header(\@directorytable, "directory");
    add_root_directories(\@directorytable, $allvariableshashref);
    create_directorytable_from_collection(\@directorytable, $directoryref);

    # Saving the file

    my $directorytablename = $basedir . $installer::globals::separator . "Director.idt";
    installer::files::save_file($directorytablename ,\@directorytable);
    $installer::logger::Lang->printf("Created idt file: %s\n", $directorytablename);

    $installer::logger::Lang->add_timestamp("Performance Info: Directory Table end");
}

1;
