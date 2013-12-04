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
use installer::scriptitems;

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




=head2 get_unique_name ($hostname, $unique_map, $shortdirhash, $shortdirhashreverse)

    Return a long and a short unique name for the given $hostname.
    Despite the function name and unlike the generation of unique
    names for files, the returned names are not really unique.  Quite
    the opposite.  The returned names are quaranteed to return the
    same result for the same input.

    The returned short name has at most length 70.

=cut
sub get_unique_name ($$)
{
    my ($hostname, $hostnamehash) = @_;

    # Make sure that we where not called for this hostname before.  Otherwise the other test would be triggered.
    if (defined $hostnamehash->{$hostname})
    {
        installer::exiter::exit_program(
            "ERROR: get_unique_name was already called for hostname ".$hostname,
            "get_unique_name");
    }
    $hostnamehash->{$hostname} = 1;

    my $uniquename = $hostname;

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

    my $short_uniquename = make_short_dir_version($uniquename);

    return ($uniquename, $short_uniquename);
}




=head2 check_unique_directorynames($directories)

    The one really important check is made in get_unique_name().  It
    checks that get_unique_name() is not called twice for the same
    directory host name.  The tests in this function contain the
    legacy tests that basically only check if there where a collision
    of the partial MD5 sum that is used to make the short unique names
    unique.

    The maps $unique_map, $shortdirhash, $shortdirhashreverse are used
    only to check that _different_ input names are mapped to different
    results.  They are not used to influence the result.  That assumes
    that this function is called only once for every directory
    hostname.
=cut
sub check_unique_directorynames ($)
{
    my ($directories) = @_;

    my %completedirhashstep1 = ();
    my %shortdirhash = ();
    my %shortdirhashreverse = ();

    # Check unique name of directories.
    foreach my $directory (@$directories)
    {
        my ($long_uniquename, $short_uniquename) = ($directory->{'long_uniquename'}, $directory->{'uniquename'});

        # The names after this small changes must still be unique!
        if (exists($completedirhashstep1{$long_uniquename}))
        {
            installer::exiter::exit_program(
                sprintf("ERROR: Unallowed modification of directory name, not unique (step 1): \"%s\".",
                    $short_uniquename),
                "check_unique_directorynames");
        }
        $completedirhashstep1{$long_uniquename} = 1;


        # Checking if the same directory already exists, but has another short version.
        if (exists($shortdirhash{$long_uniquename})
            && ( $shortdirhash{$long_uniquename} ne $short_uniquename ))
        {
            installer::exiter::exit_program(
                sprintf(
                    "ERROR: Unallowed modification of directory name, not unique (step 2A): \"%s\".",
                    $short_uniquename),
                "check_unique_directorynames");
        }
        $shortdirhash{$long_uniquename} = $short_uniquename;

        # Also checking vice versa
        # Checking if the same short directory already exists, but has another long version.
        if (exists($shortdirhashreverse{$short_uniquename})
            && ( $shortdirhashreverse{$short_uniquename} ne $long_uniquename ))
        {
            installer::exiter::exit_program(
                sprintf(
                    "ERROR: Unallowed modification of directory name, not unique (step 2B): \"%s\".",
                    $short_uniquename),
                "check_unique_directorynames");
        }
        $shortdirhashreverse{$short_uniquename} = $long_uniquename;
    }

    # Check unique name of parents
    foreach my $directory (@$directories)
    {
        my ($long_uniquename, $short_uniquename)
            = ($directory->{'long_uniqueparentname'}, $directory->{'uniqueparentname'});

        # Again checking if the same directory already exists, but has another short version.
        if (exists($shortdirhash{$long_uniquename})
            && ( $shortdirhash{$long_uniquename} ne $short_uniquename ))
        {
            installer::exiter::exit_program(
                sprintf(
                    "ERROR: Unallowed modification of directory name, not unique (step 3A): \"%s\".",
                    $short_uniquename),
                "check_unique_directorynames");
        }
        $shortdirhash{$long_uniquename} = $short_uniquename;

        # Also checking vice versa
        # Checking if the same short directory already exists, but has another long version.
        if (exists($shortdirhashreverse{$short_uniquename})
            && ( $shortdirhashreverse{$short_uniquename} ne $long_uniquename ))
        {
            installer::exiter::exit_program(
                sprintf(
                    "ERROR: Unallowed modification of directory name, not unique (step 3B): \"%s\".",
                    $short_uniquename),
                "check_unique_directorynames");
        }
        $shortdirhashreverse{$short_uniquename} = $long_uniquename;
    }
}




sub get_unique_parent_name ($$)
{
    my ($uniqueparentname, $styles) = @_;

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

    return ($originaluniqueparentname, $uniqueparentname);
}




##############################################################
# Adding unique directory names to the directory collection
##############################################################

sub create_unique_directorynames ($)
{
    my ($directories) = @_;

    $installer::globals::officeinstalldirectoryset = 0;

    my %hostnamehash = ();
    my $infoline = "";
    my $errorcount = 0;

    foreach my $directory (@$directories)
    {
        next if defined $directory->{'uniquename'};

        my $styles = $directory->{'Styles'} // "";

        my ($originaluniquename, $uniquename) = get_unique_name(
            $directory->{'HostName'},
            \%hostnamehash);

        my ($originaluniqueparentname, $uniqueparentname) = get_unique_parent_name(
            $originaluniquename,
            $styles);


        # Hyphen not allowed in database
        $uniquename =~ s/\-/\_/g;           # making "-" to "_"
        $uniqueparentname =~ s/\-/\_/g;     # making "-" to "_"

        # And finally setting the values for the directories
        $directory->{'uniquename'} = $uniquename;
        $directory->{'uniqueparentname'} = $uniqueparentname;
        $directory->{'long_uniquename'} = $originaluniquename;
        $directory->{'long_uniqueparentname'} = $originaluniqueparentname;
    }

    # Find the installation directory.
    foreach my $directory (@$directories)
    {
        next unless defined $directory->{'Styles'};

        # setting the installlocation directory
        next unless $directory->{'Styles'} =~ /\bISINSTALLLOCATION\b/;

        if ( $installer::globals::installlocationdirectoryset )
        {
            installer::exiter::exit_program(
                sprintf(
                    "ERROR: Directory with flag ISINSTALLLOCATION alread set: \"%s\".",
                    $installer::globals::installlocationdirectory),
                "create_unique_directorynames");
        }

        $installer::globals::installlocationdirectory = $directory->{'uniquename'};
        $installer::globals::installlocationdirectoryset = 1;
    }
}




#####################################################
# Adding ":." to selected default directory names
#####################################################

sub update_defaultdir ($$)
{
    my ( $onedir, $allvariableshashref ) = @_;

    if ($installer::globals::addchildprojects
        || $installer::globals::patch
        || $installer::globals::languagepack
        || $allvariableshashref->{'CHANGETARGETDIR'})
    {
        my $sourcediraddon = "\:\.";
        return $onedir->{'defaultdir'} . $sourcediraddon;
    }
    else
    {
        return $onedir->{'defaultdir'};
    }
}

#####################################################
# The directory with the style ISINSTALLLOCATION
# will be replaced by INSTALLLOCATION
#####################################################

sub set_installlocation_directory
{
    my ( $directoryref, $allvariableshashref ) = @_;

    if ( ! $installer::globals::installlocationdirectoryset )
    {
        installer::exiter::exit_program(
            "ERROR: Directory with flag ISINSTALLLOCATION not set!",
            "set_installlocation_directory");
    }

    for ( my $i = 0; $i <= $#{$directoryref}; $i++ )
    {
        my $onedir = ${$directoryref}[$i];

        if ( $onedir->{'uniquename'} eq $installer::globals::installlocationdirectory )
        {
            $onedir->{'uniquename'} = "INSTALLLOCATION";
            $onedir->{'defaultdir'} = update_defaultdir($onedir, $allvariableshashref);
        }

        if ( $onedir->{'uniquename'} eq $installer::globals::vendordirectory )
        {
            $onedir->{'defaultdir'} = update_defaultdir($onedir, $allvariableshashref);
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

sub setup_global_font_directory_name ($)
{
    my ($directories) = @_;

    foreach my $directory (@$directories)
    {
        next unless defined $directory->{'Dir'};
        next unless defined $directory->{'defaultdir'};

        next if $directory->{'Dir'} ne "PREDEFINED_OSSYSTEMFONTDIR";
        next if $directory->{'defaultdir'} ne $installer::globals::fontsdirhostname;

        $installer::globals::fontsdirname = $installer::globals::fontsdirhostname;
        $installer::globals::fontsdirparent = $directory->{'uniqueparentname'};

        $installer::logger::Info->printf("%s, fdhn %s, dd %s, ipn %s, HN %s\n",
            "PREDEFINED_OSSYSTEMFONTDIR",
            $installer::globals::fontsdirhostname,
            $directory->{'defaultdir'},
            $directory->{'uniqueparentname'},
            $directory->{'HostName'});
        installer::scriptitems::print_script_item($directory);
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
    }
}

###############################################
# Fill content into the directory table
###############################################

sub create_directorytable_from_collection ($$)
{
    my ($directorytableref, $directoryref) = @_;

    foreach my $onedir (@$directoryref)
    {
        # Remove entries for special directories.
        if (defined $onedir->{'HostName'}
            && $onedir->{'HostName'} eq ""
            && defined $onedir->{'Dir'}
            && $onedir->{'Dir'} eq "PREDEFINED_PROGDIR")
        {
            next;
        }

        my $oneline = sprintf(
            "%s\t%s\t%s\n",
            $onedir->{'uniquename'},
            $onedir->{'uniqueparentname'},
            $onedir->{'defaultdir'});

        push @{$directorytableref}, $oneline;
    }
}

###############################################
# Defining the root installation structure
###############################################

sub process_root_directories ($$)
{
    my ($allvariableshashref, $functor) = @_;

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

        my $shortproductkey = installer::windows::idtglobal::make_eight_three_conform($productkey, "dir", undef);
        $shortproductkey =~ s/\s/\_/g;                                  # changing empty space to underline

        &$functor(
            $installer::globals::officemenufolder,
            $installer::globals::programmenufolder,
            $shortproductkey . "|". $realproductkey);
    }

    &$functor("TARGETDIR", "", "SourceDir");
    &$functor($installer::globals::programfilesfolder, "TARGETDIR", ".");
    &$functor($installer::globals::programmenufolder, "TARGETDIR", ".");
    &$functor($installer::globals::startupfolder, "TARGETDIR", ".");
    &$functor($installer::globals::desktopfolder, "TARGETDIR", ".");
    &$functor($installer::globals::startmenufolder, "TARGETDIR", ".");
    &$functor($installer::globals::commonfilesfolder, "TARGETDIR", ".");
    &$functor($installer::globals::commonappdatafolder, "TARGETDIR", ".");
    &$functor($installer::globals::localappdatafolder, "TARGETDIR", ".");

    if ( $installer::globals::usesharepointpath )
    {
        &$functor("SHAREPOINTPATH", "TARGETDIR", ".");
    }

    &$functor($installer::globals::systemfolder, "TARGETDIR", ".");

    my $localtemplatefoldername = $installer::globals::templatefoldername;
    my $directorytableentry = $localtemplatefoldername;
    my $shorttemplatefoldername = installer::windows::idtglobal::make_eight_three_conform($localtemplatefoldername, "dir");
    if ( $shorttemplatefoldername ne $localtemplatefoldername )
    {
        $directorytableentry = $shorttemplatefoldername . "|" . $localtemplatefoldername;
    }
    &$functor($installer::globals::templatefolder, "TARGETDIR", $directorytableentry);

    if ( $installer::globals::fontsdirname )
    {
        &$functor(
             $installer::globals::fontsfolder,
             $installer::globals::fontsdirparent,
             $installer::globals::fontsfoldername . ":" . $installer::globals::fontsdirname);
    }
    else
    {
        &$functor(
             $installer::globals::fontsfolder,
             "TARGETDIR",
             $installer::globals::fontsfoldername);
    }
}




sub find_missing_directories ($$)
{
    my ($directories, $allvariableshashref) = @_;

    # Set up the list of target directories.
    my %target_directories = map {$_->{'uniquename'} => 1} @$directories;
    # Add special directories.
    process_root_directories(
        $allvariableshashref,
        sub($$$){
            my ($uniquename, $parentname, $defaultdir) = @_;
            $target_directories{$uniquename} = 1;
        }
    );

    # Set up the list of source directories.
    my $source_directory_map = $installer::globals::source_msi->GetDirectoryMap();
    my $source_file_map = $installer::globals::source_msi->GetFileMap();
    my %source_directories = map {$_->{'unique_name'} => $_} values %$source_directory_map;

    # Find the missing source directories.
    my @missing_directories = ();
    foreach my $source_uniquename (keys %source_directories)
    {
        if ( ! $target_directories{$source_uniquename})
        {
            push @missing_directories, $source_directories{$source_uniquename};
        }
    }

    # Report the missing directories.
    $installer::logger::Info->printf("found %d missing directories\n", scalar @missing_directories);
    my $index = 0;
    foreach my $directory_item (@missing_directories)
    {
        # Print information about the directory.
        $installer::logger::Info->printf("missing directory %d: %s\n",
            ++$index,
            $directory_item->{'full_target_long_name'});
        while (my($key,$value) = each %$directory_item)
        {
            $installer::logger::Info->printf("    %s -> %s\n", $key, $value);
        }

        # Print the referencing files.
        my @filenames = ();
        while (my ($key,$value) = each %$source_file_map)
        {
            if ($value->{'directory'}->{'unique_name'} eq $directory_item->{'unique_name'})
            {
                push @filenames, $key;
            }
        }
        $installer::logger::Info->printf("  referencing files are %s\n", join(", ", @filenames));
    }

    foreach my $directory (@$directories)
    {
        $installer::logger::Lang->printf("target directory %s -> HN %s\n",
            $directory->{'uniquename'},
            $directory->{'HostName'});
        installer::scriptitems::print_script_item($directory);
    }

    # Setup a map of directory uniquenames to verify that the new
    # entries don't use unique names that are already in use.
    my %unique_names = map {$_->{'uniquename'} => $_} @$directories;

    # Create script items for the missing directories.
    my @new_source_directories = ();
    foreach my $source_directory_item (@missing_directories)
    {
        my $new_directory_item = {
            'uniquename' => $source_directory_item->{'unique_name'},
            'uniqueparentname' => $source_directory_item->{'parent'},
            'defaultdir' => $source_directory_item->{'default_dir'},
            'HostName' => $source_directory_item->{'full_target_long_name'},
            'componentname' => $source_directory_item->{'component_name'},
        };

        if (defined $unique_names{$new_directory_item->{'uniquename'}})
        {
            installer::logger::PrintError("newly created directory entry collides with existing directory");
            last;
        }

        push @new_source_directories, $new_directory_item;
    }

    return @new_source_directories;
}




sub prepare_directory_table_creation ($$)
{
    my ($directories, $allvariableshashref) = @_;

    foreach my $directory (@$directories)
    {
        delete $directory->{'uniquename'};
    }

    overwrite_programfilesfolder($allvariableshashref);
    create_unique_directorynames($directories);
    check_unique_directorynames($directories);
    create_defaultdir_directorynames($directories); # only destdir!
    setup_global_font_directory_name($directories);
    set_installlocation_directory($directories, $allvariableshashref);

    if ($installer::globals::is_release)
    {
        my @new_directories = find_missing_directories($directories, $allvariableshashref);
        push @$directories, @new_directories;
    }
}




###############################################
# Creating the file Director.idt dynamically
###############################################

sub create_directory_table ($$$)
{
    my ($directoryref, $basedir, $allvariableshashref) = @_;

    # Structure of the directory table:
    # Directory Directory_Parent DefaultDir
    # Directory is a unique identifier
    # Directory_Parent is the unique identifier of the parent
    # DefaultDir is .:APPLIC~1|Application Data with
    # Before ":" : [sourcedir]:[destdir] (not programmed yet)
    # After ":" : 8+3 and not 8+3 the destination directory name

    $installer::logger::Lang->add_timestamp("Performance Info: Directory Table start");

    my @directorytable = ();
    installer::windows::idtglobal::write_idt_header(\@directorytable, "directory");

    # Add entries for the root directories (and a few special directories like that for fonts).
    process_root_directories(
        $allvariableshashref,
        sub($$$){
            push(@directorytable, join("\t", @_)."\n");
        }
    );

    # Add entries for the non-root directories.
    create_directorytable_from_collection(\@directorytable, $directoryref);

    # Saving the file

    my $directorytablename = $basedir . $installer::globals::separator . "Director.idt";
    installer::files::save_file($directorytablename ,\@directorytable);
    $installer::logger::Lang->printf("Created idt file: %s\n", $directorytablename);

    $installer::logger::Lang->add_timestamp("Performance Info: Directory Table end");
}


1;
