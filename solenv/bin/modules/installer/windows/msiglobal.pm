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



package installer::windows::msiglobal;

use Cwd;
use Digest::MD5;
use installer::converter;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::logger;
use installer::pathanalyzer;
use installer::remover;
use installer::scriptitems;
use installer::systemactions;
use installer::worker;
use installer::windows::idtglobal;
use installer::windows::language;
use installer::patch::ReleasesList;

use strict;

###########################################################################
# Generating the header of the ddf file.
# The usage of ddf files is needed, because makecab.exe can only include
# one sourcefile into a cab file
###########################################################################

sub write_ddf_file_header
{
    my ($ddffileref, $cabinetfile, $installdir) = @_;

    my $oneline;

    $oneline = ".Set CabinetName1=" . $cabinetfile . "\n";
    push(@{$ddffileref} ,$oneline);
    $oneline = ".Set ReservePerCabinetSize=128\n";  # This reserves space for a digital signature.
    push(@{$ddffileref} ,$oneline);
    $oneline = ".Set MaxDiskSize=2147483648\n";     # This allows the .cab file to get a size of 2 GB.
    push(@{$ddffileref} ,$oneline);
    $oneline = ".Set CompressionType=LZX\n";
    push(@{$ddffileref} ,$oneline);
    $oneline = ".Set Compress=ON\n";
    push(@{$ddffileref} ,$oneline);
    $oneline = ".Set CompressionLevel=$installer::globals::cabfilecompressionlevel\n";
    push(@{$ddffileref} ,$oneline);
    $oneline = ".Set Cabinet=ON\n";
    push(@{$ddffileref} ,$oneline);
    $oneline = ".Set DiskDirectoryTemplate=" . $installdir . "\n";
    push(@{$ddffileref} ,$oneline);
}

##########################################################################
# Lines in ddf files must not contain more than 256 characters
##########################################################################

sub check_ddf_file
{
    my ( $ddffile, $ddffilename ) = @_;

    my $maxlength = 0;
    my $maxline = 0;
    my $linelength = 0;
    my $linenumber = 0;

    for ( my $i = 0; $i <= $#{$ddffile}; $i++ )
    {
        my $oneline = ${$ddffile}[$i];

        $linelength = length($oneline);
        $linenumber = $i + 1;

        if ( $linelength > 256 )
        {
            installer::exiter::exit_program("ERROR \"$ddffilename\" line $linenumber: Lines in ddf files must not contain more than 256 characters!", "check_ddf_file");
        }

        if ( $linelength > $maxlength )
        {
            $maxlength = $linelength;
            $maxline = $linenumber;
        }
    }

    my $infoline = "Check of ddf file \"$ddffilename\": Maximum length \"$maxlength\" in line \"$maxline\" (allowed line length: 256 characters)\n";
    $installer::logger::Lang->print($infoline);
}

##########################################################################
# Lines in ddf files must not be longer than 256 characters.
# Therefore it can be useful to use relative pathes. Then it is
# necessary to change into temp directory before calling
# makecab.exe.
##########################################################################

sub make_relative_ddf_path
{
    my ( $sourcepath ) = @_;

    my $windowstemppath = $installer::globals::temppath;

    if ( $^O =~ /cygwin/i )
    {
        $windowstemppath = $installer::globals::cyg_temppath;
    }

    $sourcepath =~ s/\Q$windowstemppath\E//;
    $sourcepath =~ s/^\\//;

    return $sourcepath;
}

##########################################################################
# Returning the order of the sequences in the files array.
##########################################################################

sub get_sequenceorder
{
    my ($filesref) = @_;

    my %order = ();

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];
        if ( ! $onefile->{'assignedsequencenumber'} ) { installer::exiter::exit_program("ERROR: No sequence number assigned to $onefile->{'gid'} ($onefile->{'uniquename'})!", "get_sequenceorder"); }
        $order{$onefile->{'assignedsequencenumber'}} = $i;
    }

    return \%order;
}

##########################################################################
# Generation the list, in which the source of the files is connected
# with the cabinet destination file. Because more than one file needs
# to be included into a cab file, this has to be done via ddf files.
##########################################################################

sub generate_cab_file_list
{
    my ($filesref, $installdir, $ddfdir, $allvariables) = @_;

    my @cabfilelist = ();

    installer::logger::include_header_into_logfile("Generating ddf files");

    $installer::logger::Lang->add_timestamp("Performance Info: ddf file generation start");

    if ( $^O =~ /cygwin/i ) { installer::worker::generate_cygwin_pathes($filesref); }

    if ( $installer::globals::fix_number_of_cab_files )
    {
        for ( my $i = 0; $i <= $#{$filesref}; $i++ )
        {
            my $onefile = ${$filesref}[$i];
            my $cabinetfile = $onefile->{'cabinet'};
            my $sourcepath =  $onefile->{'sourcepath'};
            if ( $^O =~ /cygwin/i ) { $sourcepath = $onefile->{'cyg_sourcepath'}; }
            my $uniquename =  $onefile->{'uniquename'};

            my $styles = "";
            my $doinclude = 1;
            if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; };
            if ( $styles =~ /\bDONT_PACK\b/ ) { $doinclude = 0; }


            # to avoid lines with more than 256 characters, it can be useful to use relative pathes
            if ( $allvariables->{'RELATIVE_PATHES_IN_DDF'} ) { $sourcepath = make_relative_ddf_path($sourcepath); }

            # all files with the same cabinetfile are directly behind each other in the files collector

            my @ddffile = ();

            write_ddf_file_header(\@ddffile, $cabinetfile, $installdir);

            my $ddfline = "\"" . $sourcepath . "\"" . " " . $uniquename . "\n";
            if ( $doinclude ) { push(@ddffile, $ddfline); }

            my $nextfile = ${$filesref}[$i+1];
            my $nextcabinetfile = "";

            if ( $nextfile->{'cabinet'} ) { $nextcabinetfile = $nextfile->{'cabinet'}; }

            while ( $nextcabinetfile eq $cabinetfile )
            {
                $sourcepath =  $nextfile->{'sourcepath'};
                if ( $^O =~ /cygwin/i ) { $sourcepath = $nextfile->{'cyg_sourcepath'}; }
                # to avoid lines with more than 256 characters, it can be useful to use relative pathes
                if ( $allvariables->{'RELATIVE_PATHES_IN_DDF'} ) { $sourcepath = make_relative_ddf_path($sourcepath); }
                $uniquename =  $nextfile->{'uniquename'};
                my $localdoinclude = 1;
                my $nextfilestyles = "";
                if ( $nextfile->{'Styles'} ) { $nextfilestyles = $nextfile->{'Styles'}; }
                if ( $nextfilestyles =~ /\bDONT_PACK\b/ ) { $localdoinclude = 0; }
                $ddfline = "\"" . $sourcepath . "\"" . " " . $uniquename . "\n";
                if ( $localdoinclude ) { push(@ddffile, $ddfline); }
                $i++;                                           # increasing the counter!
                $nextfile = ${$filesref}[$i+1];
                if ( $nextfile ) { $nextcabinetfile = $nextfile->{'cabinet'}; }
                else { $nextcabinetfile = "_lastfile_"; }
            }

            # creating the DDF file

            my $ddffilename = $cabinetfile;
            $ddffilename =~ s/.cab/.ddf/;
            $ddfdir =~ s/\Q$installer::globals::separator\E\s*$//;
            $ddffilename = $ddfdir . $installer::globals::separator . $ddffilename;

            installer::files::save_file($ddffilename ,\@ddffile);
            my $infoline = "Created ddf file: $ddffilename\n";
            $installer::logger::Lang->print($infoline);

            # lines in ddf files must not be longer than 256 characters
            check_ddf_file(\@ddffile, $ddffilename);

            # Writing the makecab system call

            my $oneline = "makecab.exe /V3 /F " . $ddffilename . " 2\>\&1 |" . "\n";

            push(@cabfilelist, $oneline);

            # collecting all ddf files
            push(@installer::globals::allddffiles, $ddffilename);
        }
    }
    elsif ( $installer::globals::one_cab_file )
    {
        my @ddffile = ();

        my $cabinetfile = "";

        for ( my $i = 0; $i <= $#{$filesref}; $i++ )
        {
            my $onefile = ${$filesref}[$i];
            $cabinetfile = $onefile->{'cabinet'};
            my $sourcepath =  $onefile->{'sourcepath'};
            if ( $^O =~ /cygwin/i ) { $sourcepath = $onefile->{'cyg_sourcepath'}; }
            my $uniquename =  $onefile->{'uniquename'};

            # to avoid lines with more than 256 characters, it can be useful to use relative pathes
            if ( $allvariables->{'RELATIVE_PATHES_IN_DDF'} ) { $sourcepath = make_relative_ddf_path($sourcepath); }

            if ( $i == 0 ) { write_ddf_file_header(\@ddffile, $cabinetfile, $installdir); }

            my $styles = "";
            my $doinclude = 1;
            if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; };
            if ( $styles =~ /\bDONT_PACK\b/ ) { $doinclude = 0; }

            my $ddfline = "\"" . $sourcepath . "\"" . " " . $uniquename . "\n";
            if ( $doinclude ) { push(@ddffile, $ddfline); }
        }

        # creating the DDF file

        my $ddffilename = $cabinetfile;
        $ddffilename =~ s/.cab/.ddf/;
        $ddfdir =~ s/[\/\\]\s*$//;
        $ddffilename = $ddfdir . $installer::globals::separator . $ddffilename;

        installer::files::save_file($ddffilename ,\@ddffile);
        my $infoline = "Created ddf file: $ddffilename\n";
        $installer::logger::Lang->print($infoline);

        # lines in ddf files must not be longer than 256 characters
        check_ddf_file(\@ddffile, $ddffilename);

        # Writing the makecab system call

        my $oneline = "makecab.exe /F " . $ddffilename . "\n";

        push(@cabfilelist, $oneline);

        # collecting all ddf files
        push(@installer::globals::allddffiles, $ddffilename);
    }
    else
    {
        installer::exiter::exit_program("ERROR: No cab file specification in globals.pm !", "create_media_table");
    }

    $installer::logger::Lang->add_timestamp("Performance Info: ddf file generation end");

    return \@cabfilelist;   # contains all system calls for packaging process
}

########################################################################
# Returning the file sequence of a specified file.
########################################################################

sub get_file_sequence
{
    my ($filesref, $uniquefilename) = @_;

    my $sequence = "";
    my $found_sequence = 0;

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];
        my $uniquename = $onefile->{'uniquename'};

        if ( $uniquename eq $uniquefilename )
        {
            $sequence = $onefile->{'sequencenumber'};
            $found_sequence = 1;
            last;
        }
    }

    if ( ! $found_sequence ) { installer::exiter::exit_program("ERROR: No sequence found for $uniquefilename !", "get_file_sequence"); }

    return $sequence;
}


#################################################################
# Returning the name of the msi database
#################################################################

sub get_msidatabasename
{
    my ($allvariableshashref, $language) = @_;

    my $databasename = $allvariableshashref->{'PRODUCTNAME'} . $allvariableshashref->{'PRODUCTVERSION'};
    $databasename = lc($databasename);
    $databasename =~ s/\.//g;
    $databasename =~ s/\-//g;
    $databasename =~ s/\s//g;

    # possibility to overwrite the name with variable DATABASENAME
    if ( $allvariableshashref->{'DATABASENAME'} )
    {
        $databasename = $allvariableshashref->{'DATABASENAME'};
    }

    if ( $language )
    {
        if (!($language eq ""))
        {
            $databasename .= "_$language";
        }
    }

    $databasename .= ".msi";

    return $databasename;
}

#################################################################
# Creating the msi database
# This works only on Windows
#################################################################

sub create_msi_database
{
    my ($idtdirbase ,$msifilename) = @_;

    # -f : path containing the idt files
    # -d : msi database, including path
    # -c : create database
    # -i : include the following tables ("*" includes all available tables)

    my $msidb = "msidb.exe";    # Has to be in the path
    my $extraslash = "";        # Has to be set for non-ActiveState perl

    installer::logger::include_header_into_logfile("Creating msi database");

    $idtdirbase = installer::converter::make_path_conform($idtdirbase);

    $msifilename = installer::converter::make_path_conform($msifilename);

    if ( $^O =~ /cygwin/i ) {
        # msidb.exe really wants backslashes. (And double escaping because system() expands the string.)
        $idtdirbase =~ s/\//\\\\/g;
        $msifilename =~ s/\//\\\\/g;
        $extraslash = "\\";
    }
    my $systemcall = $msidb . " -f " . $idtdirbase . " -d " . $msifilename . " -c " . "-i " . $extraslash . "*";

    my $returnvalue = system($systemcall);

    my $infoline = "Systemcall: $systemcall\n";
    $installer::logger::Lang->print($infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute $msidb!\n";
        $installer::logger::Lang->print($infoline);
    }
    else
    {
        $infoline = "Success: Executed $msidb successfully!\n";
        $installer::logger::Lang->print($infoline);
    }
}

#####################################################################
# Returning the value from sis.mlf for Summary Information Stream
#####################################################################

sub get_value_from_sis_lng
{
    my ($language, $languagefile, $searchstring) = @_;

    my $language_block = installer::windows::idtglobal::get_language_block_from_language_file($searchstring, $languagefile);
    my $newstring = installer::windows::idtglobal::get_language_string_from_language_block($language_block, $language, $searchstring);
    $newstring = "\"" . $newstring . "\"";

    return $newstring;
}

#################################################################
# Returning the msi version for the Summary Information Stream
#################################################################

sub get_msiversion_for_sis
{
    my $msiversion = "200";
    return $msiversion;
}

#################################################################
# Returning the word count for the Summary Information Stream
#################################################################

sub get_wordcount_for_sis
{
    my $wordcount = "0";
    return $wordcount;
}

#################################################################
# Returning the codepage for the Summary Information Stream
#################################################################

sub get_codepage_for_sis
{
    my ( $language ) = @_;

    my $codepage = installer::windows::language::get_windows_encoding($language);

    # Codepage 65001 does not work in Summary Information Stream
    if ( $codepage == 65001 ) { $codepage = 0; }

    # my $codepage = "1252";    # determine dynamically in a function
    # my $codepage = "65001";       # UTF-8
    return $codepage;
}

#################################################################
# Returning the template for the Summary Information Stream
#################################################################

sub get_template_for_sis
{
    my ( $language, $allvariables ) = @_;

    my $windowslanguage = installer::windows::language::get_windows_language($language);

    my $architecture = "Intel";

    # Adding 256, if this is a 64 bit installation set.
    if (( $allvariables->{'64BITPRODUCT'} ) && ( $allvariables->{'64BITPRODUCT'} == 1 )) { $architecture = "x64"; }

    my $value = "\"" . $architecture . ";" . $windowslanguage;  # adding the Windows language

    $value = $value . "\"";                     # adding ending '"'

    return $value ;
}

#################################################################
# Returning the PackageCode for the Summary Information Stream
#################################################################

sub get_packagecode_for_sis
{
    # always generating a new package code for each package

    my $guid = "\{" . create_guid() . "\}";

    my $infoline = "PackageCode: $guid\n";
    $installer::logger::Lang->print($infoline);

    return $guid;
}

#################################################################
# Returning the title for the Summary Information Stream
#################################################################

sub get_title_for_sis
{
    my ( $language, $languagefile, $searchstring ) = @_;

    my $title = get_value_from_sis_lng($language, $languagefile, $searchstring );

    return $title;
}

#################################################################
# Returning the author for the Summary Information Stream
#################################################################

sub get_author_for_sis
{
    my $author = $installer::globals::longmanufacturer;

    $author = "\"" . $author . "\"";

    return $author;
}

#################################################################
# Returning the subject for the Summary Information Stream
#################################################################

sub get_subject_for_sis
{
    my ( $allvariableshashref ) = @_;

    my $subject = $allvariableshashref->{'PRODUCTNAME'} . " " . $allvariableshashref->{'PRODUCTVERSION'};

    $subject = "\"" . $subject . "\"";

    return $subject;
}

#################################################################
# Returning the comment for the Summary Information Stream
#################################################################

sub get_comment_for_sis
{
    my ( $language, $languagefile, $searchstring ) = @_;

    my $comment = get_value_from_sis_lng($language, $languagefile, $searchstring );

    return $comment;
}

#################################################################
# Returning the keywords for the Summary Information Stream
#################################################################

sub get_keywords_for_sis
{
    my ( $language, $languagefile, $searchstring ) = @_;

    my $keywords = get_value_from_sis_lng($language, $languagefile, $searchstring );

    return $keywords;
}

######################################################################
# Returning the application name for the Summary Information Stream
######################################################################

sub get_appname_for_sis
{
    my ( $language, $languagefile, $searchstring ) = @_;

    my $appname = get_value_from_sis_lng($language, $languagefile, $searchstring );

    return $appname;
}

######################################################################
# Returning the security for the Summary Information Stream
######################################################################

sub get_security_for_sis
{
    my $security = "0";
    return $security;
}

#################################################################
# Writing the Summary information stream into the msi database
# This works only on Windows
#################################################################

sub write_summary_into_msi_database
{
    my ($msifilename, $language, $languagefile, $allvariableshashref) = @_;

    # -g : requrired msi version
    # -c : codepage
    # -p : template

    installer::logger::include_header_into_logfile("Writing summary information stream");

    my $msiinfo = "msiinfo.exe";    # Has to be in the path

    my $sislanguage = "en-US";  # title, comment, keyword and appname alway in english

    my $msiversion = get_msiversion_for_sis();
    my $codepage = get_codepage_for_sis($language);
    my $template = get_template_for_sis($language, $allvariableshashref);
    my $guid = get_packagecode_for_sis();
    my $title = get_title_for_sis($sislanguage,$languagefile, "OOO_SIS_TITLE");
    my $author = get_author_for_sis();
    my $subject = get_subject_for_sis($allvariableshashref);
    my $comment = get_comment_for_sis($sislanguage,$languagefile, "OOO_SIS_COMMENT");
    my $keywords = get_keywords_for_sis($sislanguage,$languagefile, "OOO_SIS_KEYWORDS");
    my $appname = get_appname_for_sis($sislanguage,$languagefile, "OOO_SIS_APPNAME");
    my $security = get_security_for_sis();
    my $wordcount = get_wordcount_for_sis();

    $msifilename = installer::converter::make_path_conform($msifilename);

    my $systemcall = $msiinfo . " " . $msifilename . " -g " . $msiversion . " -c " . $codepage
                    . " -p " . $template . " -v " . $guid . " -t " . $title . " -a " . $author
                    . " -j " . $subject . " -o " . $comment . " -k " . $keywords . " -n " . $appname
                    . " -u " . $security . " -w " . $wordcount;

    my $returnvalue = system($systemcall);

    my $infoline = "Systemcall: $systemcall\n";
    $installer::logger::Lang->print($infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute $msiinfo!\n";
        $installer::logger::Lang->print($infoline);
    }
    else
    {
        $infoline = "Success: Executed $msiinfo successfully!\n";
        $installer::logger::Lang->print($infoline);
    }
}

#########################################################################
# For more than one language in the installation set:
# Use one database and create Transformations for all other languages
#########################################################################

sub create_transforms
{
    my ($languagesarray, $defaultlanguage, $installdir, $allvariableshashref) = @_;

    installer::logger::include_header_into_logfile("Creating Transforms");

    my $msitran = "msitran.exe";    # Has to be in the path

    $installdir = installer::converter::make_path_conform($installdir);

    # Syntax for creating a transformation
    # msitran.exe -g <baseDB> <referenceDB> <transformfile> [<errorhandling>}

    my $basedbname = get_msidatabasename($allvariableshashref, $defaultlanguage);
    $basedbname = $installdir . $installer::globals::separator . $basedbname;

    my $errorhandling = "f";    # Suppress "change codepage" error

    # Iterating over all files

    foreach ( @{$languagesarray} )
    {
        my $onelanguage = $_;

        if ( $onelanguage eq $defaultlanguage ) { next; }

        my $referencedbname = get_msidatabasename($allvariableshashref, $onelanguage);
        $referencedbname = $installdir . $installer::globals::separator . $referencedbname;

        my $transformfile = $installdir . $installer::globals::separator . "trans_" . $onelanguage . ".mst";

        my $systemcall = $msitran . " " . " -g " . $basedbname . " " . $referencedbname . " " . $transformfile . " " . $errorhandling;

        my $returnvalue = system($systemcall);

        my $infoline = "Systemcall: $systemcall\n";
        $installer::logger::Lang->print($infoline);

        # Problem: msitran.exe in version 4.0 always returns "1", even if no failure occured.
        # Therefore it has to be checked, if this is version 4.0. If yes, if the mst file
        # exists and if it is larger than 0 bytes. If this is true, then no error occured.
        # File Version of msitran.exe: 4.0.6000.16384 has checksum: "b66190a70145a57773ec769e16777b29".
        # Same for msitran.exe from wntmsci12: "aa25d3445b94ffde8ef0c1efb77a56b8"

        if ($returnvalue)
        {
            $infoline = "WARNING: Returnvalue of $msitran is not 0. Checking version of $msitran!\n";
            $installer::logger::Lang->print($infoline);

            open(FILE, "<$installer::globals::msitranpath") or die "ERROR: Can't open $installer::globals::msitranpath for creating file hash";
            binmode(FILE);
            my $digest = Digest::MD5->new->addfile(*FILE)->hexdigest;
            close(FILE);

            my @problemchecksums = ("b66190a70145a57773ec769e16777b29", "aa25d3445b94ffde8ef0c1efb77a56b8");
            my $isproblemchecksum = 0;

            foreach my $problemchecksum ( @problemchecksums )
            {
                $infoline = "Checksum of problematic MsiTran.exe: $problemchecksum\n";
                $installer::logger::Lang->print($infoline);
                $infoline = "Checksum of used MsiTran.exe: $digest\n";
                $installer::logger::Lang->print($infoline);
                if ( $digest eq $problemchecksum ) { $isproblemchecksum = 1; }
            }

            if ( $isproblemchecksum )
            {
                # Check existence of mst
                if ( -f $transformfile )
                {
                    $infoline = "File $transformfile exists.\n";
                    $installer::logger::Lang->print($infoline);
                    my $filesize = ( -s $transformfile );
                    $infoline = "Size of $transformfile: $filesize\n";
                    $installer::logger::Lang->print($infoline);

                    if ( $filesize > 0 )
                    {
                        $infoline = "Info: Returnvalue $returnvalue of $msitran is no problem :-) .\n";
                        $installer::logger::Lang->print($infoline);
                        $returnvalue = 0; # reset the error
                    }
                    else
                    {
                        $infoline = "Filesize indicates that an error occured.\n";
                        $installer::logger::Lang->print($infoline);
                    }
                }
                else
                {
                    $infoline = "File $transformfile does not exist -> An error occured.\n";
                    $installer::logger::Lang->print($infoline);
                }
            }
            else
            {
                $infoline = "This is not a problematic version of msitran.exe. Therefore the error is not caused by problematic msitran.exe.\n";
                $installer::logger::Lang->print($infoline);
            }
        }

        if ($returnvalue)
        {
            $infoline = "ERROR: Could not execute $msitran!\n";
            $installer::logger::Lang->print($infoline);
        }
        else
        {
            $infoline = "Success: Executed $msitran successfully!\n";
            $installer::logger::Lang->print($infoline);
        }

        # The reference database can be deleted

        my $result = unlink($referencedbname);
        # $result contains the number of deleted files

        if ( $result == 0 )
        {
            $infoline = "ERROR: Could not remove file $$referencedbname !\n";
            $installer::logger::Lang->print($infoline);
            installer::exiter::exit_program($infoline, "create_transforms");
        }
    }
}

#########################################################################
# The default language msi database does not need to contain
# the language in the database name. Therefore the file
# is renamed. Example: "openofficeorg20_01.msi" to "openofficeorg20.msi"
#########################################################################

sub rename_msi_database_in_installset
{
    my ($defaultlanguage, $installdir, $allvariableshashref) = @_;

    installer::logger::include_header_into_logfile("Renaming msi database");

    my $olddatabasename = get_msidatabasename($allvariableshashref, $defaultlanguage);
    $olddatabasename = $installdir . $installer::globals::separator . $olddatabasename;

    my $newdatabasename = get_msidatabasename($allvariableshashref);

    $installer::globals::shortmsidatabasename = $newdatabasename;

    $newdatabasename = $installdir . $installer::globals::separator . $newdatabasename;

    installer::systemactions::rename_one_file($olddatabasename, $newdatabasename);

    $installer::globals::msidatabasename = $newdatabasename;
}

#########################################################################
# Adding the language to the name of the msi databasename,
# if this is required (ADDLANGUAGEINDATABASENAME)
#########################################################################

sub add_language_to_msi_database
{
    my ($defaultlanguage, $installdir, $allvariables) = @_;

    my $languagestring = $defaultlanguage;
    if ( $allvariables->{'USELANGUAGECODE'} ) { $languagestring = installer::windows::language::get_windows_language($defaultlanguage); }
    my $newdatabasename = $installer::globals::shortmsidatabasename;
    $newdatabasename =~ s/\.msi\s*$/_$languagestring\.msi/;
    $installer::globals::shortmsidatabasename = $newdatabasename;
    $newdatabasename = $installdir . $installer::globals::separator . $newdatabasename;

    my $olddatabasename = $installer::globals::msidatabasename;

    installer::systemactions::rename_one_file($olddatabasename, $newdatabasename);

    $installer::globals::msidatabasename = $newdatabasename;
}

##########################################################################
# Writing the databasename into the setup.ini.
##########################################################################

sub put_databasename_into_setupini
{
    my ($setupinifile, $allvariableshashref) = @_;

    my $databasename = get_msidatabasename($allvariableshashref);
    my $line = "database=" . $databasename . "\n";

    push(@{$setupinifile}, $line);
}

##########################################################################
# Writing the required msi version into setup.ini
##########################################################################

sub put_msiversion_into_setupini
{
    my ($setupinifile) = @_;

    my $msiversion = "2.0";
    my $line = "msiversion=" . $msiversion . "\n";

    push(@{$setupinifile}, $line);
}

##########################################################################
# Writing the productname into setup.ini
##########################################################################

sub put_productname_into_setupini
{
    my ($setupinifile, $allvariableshashref) = @_;

    my $productname = $allvariableshashref->{'PRODUCTNAME'};
    my $line = "productname=" . $productname . "\n";

    push(@{$setupinifile}, $line);
}

##########################################################################
# Writing the productcode into setup.ini
##########################################################################

sub put_productcode_into_setupini
{
    my ($setupinifile) = @_;

    my $productcode = $installer::globals::productcode;
    my $line = "productcode=" . $productcode . "\n";

    push(@{$setupinifile}, $line);
}

##########################################################################
# Writing the ProductVersion from Property table into setup.ini
##########################################################################

sub put_productversion_into_setupini
{
    my ($setupinifile) = @_;

    my $line = "productversion=" . $installer::globals::msiproductversion . "\n";
    push(@{$setupinifile}, $line);
}

##########################################################################
# Writing the key for Minor Upgrades into setup.ini
##########################################################################

sub put_upgradekey_into_setupini
{
    my ($setupinifile) = @_;

    if ( $installer::globals::minorupgradekey ne "" )
    {
        my $line = "upgradekey=" . $installer::globals::minorupgradekey . "\n";
        push(@{$setupinifile}, $line);
    }
}

##########################################################################
# Writing the number of languages into setup.ini
##########################################################################

sub put_languagecount_into_setupini
{
    my ($setupinifile, $languagesarray) = @_;

    my $languagecount = $#{$languagesarray} + 1;
    my $line = "count=" . $languagecount . "\n";

    push(@{$setupinifile}, $line);
}

##########################################################################
# Writing the defaultlanguage into setup.ini
##########################################################################

sub put_defaultlanguage_into_setupini
{
    my ($setupinifile, $defaultlanguage) = @_;

    my $windowslanguage = installer::windows::language::get_windows_language($defaultlanguage);
    my $line = "default=" . $windowslanguage . "\n";
    push(@{$setupinifile}, $line);
}

##########################################################################
# Writing the information about transformations into setup.ini
##########################################################################

sub put_transforms_into_setupini
{
    my ($setupinifile, $onelanguage, $counter) = @_;

    my $windowslanguage = installer::windows::language::get_windows_language($onelanguage);
    my $transformfilename = "trans_" . $onelanguage . ".mst";

    my $line = "lang" . $counter . "=" . $windowslanguage . "," . $transformfilename . "\n";

    push(@{$setupinifile}, $line);
}

###################################################
# Including Windows line ends in ini files
# Profiles on Windows shall have \r\n line ends
###################################################

sub include_windows_lineends
{
    my ($onefile) = @_;

    for ( my $i = 0; $i <= $#{$onefile}; $i++ )
    {
        ${$onefile}[$i] =~ s/\r?\n$/\r\n/;
    }
}

##########################################################################
# Generation the file setup.ini, that is used by the loader setup.exe.
##########################################################################

sub create_setup_ini
{
    my ($languagesarray, $defaultlanguage, $installdir, $allvariableshashref) = @_;

    installer::logger::include_header_into_logfile("Creating setup.ini");

    my $setupinifilename = $installdir . $installer::globals::separator . "setup.ini";

    my @setupinifile = ();
    my $setupinifile = \@setupinifile;

    my $line = "\[setup\]\n";
    push(@setupinifile, $line);

    put_databasename_into_setupini($setupinifile, $allvariableshashref);
    put_msiversion_into_setupini($setupinifile);
    put_productname_into_setupini($setupinifile, $allvariableshashref);
    put_productcode_into_setupini($setupinifile);
    put_productversion_into_setupini($setupinifile);
    put_upgradekey_into_setupini($setupinifile);

    $line = "\[languages\]\n";
    push(@setupinifile, $line);

    put_languagecount_into_setupini($setupinifile, $languagesarray);
    put_defaultlanguage_into_setupini($setupinifile, $defaultlanguage);

    if ( $#{$languagesarray} > 0 )  # writing the transforms information
    {
        my $counter = 1;

        for ( my $i = 0; $i <= $#{$languagesarray}; $i++ )
        {
            if ( ${$languagesarray}[$i] eq $defaultlanguage ) { next; }

            put_transforms_into_setupini($setupinifile, ${$languagesarray}[$i], $counter);
            $counter++;
        }
    }

    if ( $installer::globals::iswin && $installer::globals::plat =~ /cygwin/i)      # Windows line ends only for Cygwin
    {
        include_windows_lineends($setupinifile);
    }

    installer::files::save_file($setupinifilename, $setupinifile);

    $installer::logger::Lang->printf("Generated file %s\n", $setupinifilename);
}

#################################################################
# Copying the files defined as ScpActions into the
# installation set.
#################################################################

sub copy_scpactions_into_installset
{
    my ($defaultlanguage, $installdir, $allscpactions) = @_;

    installer::logger::include_header_into_logfile("Copying ScpAction files into installation set");

    for ( my $i = 0; $i <= $#{$allscpactions}; $i++ )
    {
        my $onescpaction = ${$allscpactions}[$i];

        if ( $onescpaction->{'Name'} eq "loader.exe" ) { next; }    # do not copy this ScpAction loader

        # only copying language independent files or files with the correct language (the defaultlanguage)

        my $filelanguage = $onescpaction->{'specificlanguage'};

        if ( ($filelanguage eq $defaultlanguage) || ($filelanguage eq "") )
        {
            my $sourcefile = $onescpaction->{'sourcepath'};
            my $destfile = $installdir . $installer::globals::separator . $onescpaction->{'DestinationName'};

            installer::systemactions::copy_one_file($sourcefile, $destfile);
        }
    }
}

#################################################################
# Copying the files for the Windows installer into the
# installation set (setup.exe).
#################################################################

sub copy_windows_installer_files_into_installset
{
    my ($installdir, $includepatharrayref, $allvariables) = @_;

    installer::logger::include_header_into_logfile("Copying Windows installer files into installation set");

    my @copyfile = ();
    push(@copyfile, "loader2.exe");

    if ( $allvariables->{'NOLOADERREQUIRED'} ) { @copyfile = (); }

    for ( my $i = 0; $i <= $#copyfile; $i++ )
    {
        my $filename = $copyfile[$i];
        my $sourcefileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$filename, $includepatharrayref, 1);

        if ( ! -f $$sourcefileref ) { installer::exiter::exit_program("ERROR: msi file not found: $$sourcefileref !", "copy_windows_installer_files_into_installset"); }

        my $destfile;
        if ( $copyfile[$i] eq "loader2.exe" ) { $destfile = "setup.exe"; }  # renaming the loader
        else { $destfile = $copyfile[$i]; }

        $destfile = $installdir . $installer::globals::separator . $destfile;

        installer::systemactions::copy_one_file($$sourcefileref, $destfile);
    }
}

#################################################################
# Copying the child projects into the
# installation set
#################################################################

sub copy_child_projects_into_installset
{
    my ($installdir, $allvariables) = @_;

    my $sourcefile = "";
    my $destdir = "";

    # adding Java

    if ( $allvariables->{'JAVAPRODUCT'} )
    {
        $sourcefile = $installer::globals::javafile->{'sourcepath'};
        $destdir = $installdir . $installer::globals::separator . $installer::globals::javafile->{'Subdir'};
        if ( ! -d $destdir) { installer::systemactions::create_directory($destdir); }
        installer::systemactions::copy_one_file($sourcefile, $destdir);
    }

    if ( $allvariables->{'UREPRODUCT'} )
    {
        $sourcefile = $installer::globals::urefile->{'sourcepath'};
        $destdir = $installdir . $installer::globals::separator . $installer::globals::urefile->{'Subdir'};
        if ( ! -d $destdir) { installer::systemactions::create_directory($destdir); }
        installer::systemactions::copy_one_file($sourcefile, $destdir);
    }
}



=head2 create_guid ()

    Create a single UUID aka GUID via calling the external executable 'uuidgen'.
    There are Perl modules for that, but do they exist on the build bots?

=cut
sub create_guid ()
{
    my $uuid = qx("uuidgen");
    $uuid =~ s/\s*$//;
    return uc($uuid);
}

#################################################################
# Calculating a GUID with a string using md5.
#################################################################

sub calculate_guid
{
    my ( $string ) = @_;

    my $guid = "";

    my $md5 = Digest::MD5->new;
    $md5->add($string);
    my $digest = $md5->hexdigest;
    $digest = uc($digest);

    # my $id = pack("A32", $digest);
    my ($first, $second, $third, $fourth, $fifth) = unpack ('A8 A4 A4 A4 A12', $digest);
    $guid = "$first-$second-$third-$fourth-$fifth";

    $installer::logger::Lang->printf("guid for '%s' is %s\n",
        $string, $guid);

    return $guid;
}

#################################################################
# Calculating a ID with a string using md5 (very fast).
#################################################################

sub calculate_id
{
    my ( $string, $length ) = @_;

    my $id = "";

    my $md5 = Digest::MD5->new;
    $md5->add($string);
    my $digest = lc($md5->hexdigest);
    $id = substr($digest, 0, $length);

    return $id;
}

#################################################################
# Filling the component hash with the values of the
# component file.
#################################################################

sub fill_component_hash
{
    my ($componentfile) = @_;

    my %components = ();

    for ( my $i = 0; $i <= $#{$componentfile}; $i++ )
    {
        my $line = ${$componentfile}[$i];

        if ( $line =~ /^\s*(.*?)\t(.*?)\s*$/ )
        {
            my $key = $1;
            my $value = $2;

            $components{$key} = $value;
        }
    }

    return \%components;
}

#################################################################
# Creating a new component file, if new guids were generated.
#################################################################

sub create_new_component_file
{
    my ($componenthash) = @_;

    my @componentfile = ();

    my $key;

    foreach $key (keys %{$componenthash})
    {
        my $value = $componenthash->{$key};
        my $input = "$key\t$value\n";
        push(@componentfile ,$input);
    }

    return \@componentfile;
}

#################################################################
# Filling real component GUID into the component table.
# This works only on Windows
#################################################################

sub __set_uuid_into_component_table
{
    my ($idtdirbase, $allvariables) = @_;

    my $componenttablename  = $idtdirbase . $installer::globals::separator . "Componen.idt";

    my $componenttable = installer::files::read_file($componenttablename);

    # For update and patch reasons (small update) the GUID of an existing component must not change!
    # The collection of component GUIDs is saved in the directory $installer::globals::idttemplatepath in the file "components.txt"

    my $infoline = "";
    my $counter = 0;
    # my $componentfile = installer::files::read_file($installer::globals::componentfilename);
    # my $componenthash = fill_component_hash($componentfile);

    for ( my $i = 3; $i <= $#{$componenttable}; $i++ )  # ignoring the first three lines
    {
        my $oneline = ${$componenttable}[$i];
        my $componentname = "";
        if ( $oneline =~ /^\s*(\S+?)\t/ ) { $componentname = $1; }

        my $uuid = "";

    #   if ( $componenthash->{$componentname} )
    #   {
    #       $uuid = $componenthash->{$componentname};
    #   }
    #   else
    #   {

            if ( exists($installer::globals::calculated_component_guids{$componentname}))
            {
                $uuid = $installer::globals::calculated_component_guids{$componentname};
            }
            else
            {
                # Calculating new GUID with the help of the component name.
                my $useooobaseversion = 1;
                if ( exists($installer::globals::base_independent_components{$componentname}))
                {
                    $useooobaseversion = 0;
                }
                my $sourcestring = $componentname;

                if ( $useooobaseversion )
                {
                    if ( ! exists($allvariables->{'OOOBASEVERSION'}) )
                    {
                        installer::exiter::exit_program(
                            "ERROR: Could not find variable \"OOOBASEVERSION\" (required value for GUID creation)!",
                            "set_uuid_into_component_table");
                    }
                    $sourcestring = $sourcestring . "_" . $allvariables->{'OOOBASEVERSION'};
                }
                $uuid = calculate_guid($sourcestring);
                $counter++;

                # checking, if there is a conflict with an already created guid
                if ( exists($installer::globals::allcalculated_guids{$uuid}) )
                {
                    installer::exiter::exit_program(
                        "ERROR: \"$uuid\" was already created before!",
                        "set_uuid_into_component_table");
                }
                $installer::globals::allcalculated_guids{$uuid} = 1;
                $installer::globals::calculated_component_guids{$componentname} = $uuid;

                # Setting new uuid
                # $componenthash->{$componentname} = $uuid;

                # Setting flag
                # $installer::globals::created_new_component_guid = 1;  # this is very important!
            }
    #   }

        ${$componenttable}[$i] =~ s/COMPONENTGUID/$uuid/;
    }

    installer::files::save_file($componenttablename, $componenttable);

#   if ( $installer::globals::created_new_component_guid )
#   {
#       # create new component file!
#       $componentfile = create_new_component_file($componenthash);
#       installer::worker::sort_array($componentfile);
#
#       # To avoid conflict the components file cannot be saved at the same place
#       # All important data have to be saved in the directory: $installer::globals::infodirectory
#       my $localcomponentfilename = $installer::globals::componentfilename;
#       installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$localcomponentfilename);
#       $localcomponentfilename = $installer::globals::infodirectory . $installer::globals::separator . $localcomponentfilename;
#       installer::files::save_file($localcomponentfilename, $componentfile);
#
#       # installer::files::save_file($installer::globals::componentfilename, $componentfile);  # version using new file in solver
#
#       $infoline = "COMPONENTCODES: Created $counter new GUIDs for components ! \n";
#       $installer::logger::Lang->print($infoline);
#   }
#   else
#   {
#       $infoline = "SUCCESS COMPONENTCODES: All component codes exist! \n";
#       $installer::logger::Lang->print($infoline);
#   }

}

#########################################################################
# Adding final 64 properties into msi database, if required.
# RegLocator : +16 in type column to search in 64 bit registry.
# All conditions: "VersionNT" -> "VersionNT64" (several tables).
# Already done: "+256" in Attributes column of table "Component".
# Still following: Setting "x64" instead of "Intel" in Summary
# Information Stream of msi database in "get_template_for_sis".
#########################################################################

sub prepare_64bit_database
{
    my ($basedir, $allvariables) = @_;

    my $infoline = "";

    if (( $allvariables->{'64BITPRODUCT'} ) && ( $allvariables->{'64BITPRODUCT'} == 1 ))
    {
        # 1. Beginning with table "RegLocat.idt". Adding "16" to the type.

        my $reglocatfile = "";
        my $reglocatfilename = $basedir . $installer::globals::separator . "RegLocat.idt";

        if ( -f $reglocatfilename )
        {
            my $saving_required = 0;
            $reglocatfile = installer::files::read_file($reglocatfilename);

            for ( my $i = 3; $i <= $#{$reglocatfile}; $i++ )    # ignoring the first three lines
            {
                my $oneline = ${$reglocatfile}[$i];

                if ( $oneline =~ /^\s*\#/ ) { next; }   # this is a comment line
                if ( $oneline =~ /^\s*$/ ) { next; }

                if ( $oneline =~ /^\s*(.*?)\t(.*?)\t(.*?)\t(.*?)\t(\d+)\s*$/ )
                {
                    # Syntax: Signature_ Root Key Name Type
                    my $sig = $1;
                    my $root = $2;
                    my $key = $3;
                    my $name = $4;
                    my $type = $5;

                    $type = $type + 16;

                    my $newline = $sig . "\t" . $root . "\t" . $key . "\t" . $name . "\t" . $type . "\n";
                    ${$reglocatfile}[$i] = $newline;

                    $saving_required = 1;
                }
            }

            if ( $saving_required )
            {
                # Saving the files
                installer::files::save_file($reglocatfilename ,$reglocatfile);
                $infoline = "Making idt file 64 bit conform: $reglocatfilename\n";
                $installer::logger::Lang->print($infoline);
            }
        }

        # 2. Replacing all occurences of "VersionNT" by "VersionNT64"

        my @versionnt_files = ("Componen.idt", "InstallE.idt", "InstallU.idt", "LaunchCo.idt");

        foreach my $onefile ( @versionnt_files )
        {
            my $fullfilename = $basedir . $installer::globals::separator . $onefile;

            if ( -f $fullfilename )
            {
                my $saving_required = 0;
                my $filecontent = installer::files::read_file($fullfilename);

                for ( my $i = 3; $i <= $#{$filecontent}; $i++ )     # ignoring the first three lines
                {
                    my $oneline = ${$filecontent}[$i];

                    if ( $oneline =~ /\bVersionNT\b/ )
                    {
                        ${$filecontent}[$i] =~ s/\bVersionNT\b/VersionNT64/g;
                        $saving_required = 1;
                    }
                }

                if ( $saving_required )
                {
                    # Saving the files
                    installer::files::save_file($fullfilename ,$filecontent);
                    $infoline = "Making idt file 64 bit conform: $fullfilename\n";
                    $installer::logger::Lang->print($infoline);
                }
            }
        }
    }

}

#################################################################
# Include all cab files into the msi database.
# This works only on Windows
#################################################################

sub include_cabs_into_msi
{
    my ($installdir) = @_;

    installer::logger::include_header_into_logfile("Including cabs into msi database");

    my $from = cwd();
    my $to = $installdir;

    chdir($to);

    my $infoline = "Changing into directory: $to";
    $installer::logger::Lang->print($infoline);

    my $msidb = "msidb.exe";    # Has to be in the path
    my $extraslash = "";        # Has to be set for non-ActiveState perl

    my $msifilename = $installer::globals::msidatabasename;

    $msifilename = installer::converter::make_path_conform($msifilename);

    # msidb.exe really wants backslashes. (And double escaping because system() expands the string.)
    $msifilename =~ s/\//\\\\/g;
    $extraslash = "\\";

    my $allcabfiles = installer::systemactions::find_file_with_file_extension("cab", $installdir);

    for ( my $i = 0; $i <= $#{$allcabfiles}; $i++ )
    {
        my $systemcall = $msidb . " -d " . $msifilename . " -a " . ${$allcabfiles}[$i];

        my $returnvalue = system($systemcall);

        $infoline = "Systemcall: $systemcall\n";
        $installer::logger::Lang->print($infoline);

        if ($returnvalue)
        {
            $infoline = "ERROR: Could not execute $systemcall !\n";
            $installer::logger::Lang->print($infoline);
        }
        else
        {
            $infoline = "Success: Executed $systemcall successfully!\n";
            $installer::logger::Lang->print($infoline);
        }

        # deleting the cab file

        unlink(${$allcabfiles}[$i]);

        $infoline = "Deleted cab file: ${$allcabfiles}[$i]\n";
        $installer::logger::Lang->print($infoline);
    }

    $infoline = "Changing back into directory: $from";
    $installer::logger::Lang->print($infoline);

    chdir($from);
}

#################################################################
# Executing the created batch file to pack all files.
# This works only on Windows
#################################################################

sub execute_packaging
{
    my ($localpackjobref, $loggingdir, $allvariables) = @_;

    installer::logger::include_header_into_logfile("Packaging process");

    $installer::logger::Lang->add_timestamp("Performance Info: Execute packaging start");

    my $infoline = "";
    my $from = cwd();
    my $to = $loggingdir;

    chdir($to);
    $infoline = "chdir: $to \n";
    $installer::logger::Lang->print($infoline);

    # if the ddf file contains relative pathes, it is necessary to change into the temp directory
    if ( $allvariables->{'RELATIVE_PATHES_IN_DDF'} )
    {
        $to = $installer::globals::temppath;
        chdir($to);
        $infoline = "chdir: $to \n";
        $installer::logger::Lang->print($infoline);
    }

    # changing the tmp directory, because makecab.exe generates temporary cab files
    my $origtemppath = "";
    if ( $ENV{'TMP'} ) { $origtemppath = $ENV{'TMP'}; }
    $ENV{'TMP'} = $installer::globals::temppath;    # setting TMP to the new unique directory!

    my $maxmakecabcalls = 3;
    my $allmakecabcalls = $#{$localpackjobref} + 1;

    for ( my $i = 0; $i <= $#{$localpackjobref}; $i++ )
    {
        my $systemcall = ${$localpackjobref}[$i];

        my $callscounter = $i + 1;

        $installer::logger::Info->printf("... makecab.exe (%s/%s) ... \n", $callscounter, $allmakecabcalls);

        # my $returnvalue = system($systemcall);

        for ( my $n = 1; $n <= $maxmakecabcalls; $n++ )
        {
            my @ddfoutput = ();

            $infoline = "Systemcall: $systemcall";
            $installer::logger::Lang->print($infoline);

            open (DDF, "$systemcall");
            while (<DDF>) {push(@ddfoutput, $_); }
            close (DDF);

            my $returnvalue = $?;   # $? contains the return value of the systemcall

            if ($returnvalue)
            {
                if ( $n < $maxmakecabcalls )
                {
                    $installer::logger::Info->printf("makecab_error (Try %s): Trying again\n", $n);
                    $installer::logger::Lang->printf("makecab_error (Try %s): Trying again\n", $n);
                }
                else
                {
                    $installer::logger::Info->printf("ERROR (Try %s): Abort packing \n", $n);
                    $installer::logger::Lang->printf("ERROR (Try %s): Abort packing \n", $n);
                }

                for ( my $m = 0; $m <= $#ddfoutput; $m++ )
                {
                    if ( $ddfoutput[$m] =~ /(ERROR\:.*?)\s*$/ )
                    {
                        $infoline = $1 . "\n";
                        if ( $n < $maxmakecabcalls )
                        {
                            $infoline =~ s/ERROR\:/makecab_error\:/i;
                        }
                        $installer::logger::Info->print($infoline);
                        $installer::logger::Lang->print($infoline);
                    }
                }

                if ( $n == $maxmakecabcalls ) { installer::exiter::exit_program("ERROR: \"$systemcall\"!", "execute_packaging"); }
            }
            else
            {
                $infoline = "Success (Try $n): $systemcall";
                $installer::logger::Lang->print($infoline);
                last;
            }
        }
    }

    $installer::logger::Lang->add_timestamp("Performance Info: Execute packaging end");

    # setting back to the original tmp directory
    $ENV{'TMP'} = $origtemppath;

    chdir($from);
    $infoline = "chdir: $from \n";
    $installer::logger::Lang->print($infoline);
}


=head2 get_source_codes($languagesref)

    Return product code and upgrade code from the source version.
    When no source version is defined then return undef for both.

=cut
sub get_source_codes ($)
{
    my ($languagesref) = @_;

    if ( ! defined $installer::globals::source_version)
    {
        return;
    }

    my $onelanguage = installer::languages::get_key_language($languagesref);

    my $release_data = installer::patch::ReleasesList::Instance()
        ->{$installer::globals::source_version}
        ->{$installer::globals::packageformat};
    if (defined $release_data)
    {
        my $language_data = $release_data->{$onelanguage};
        if (defined $language_data)
        {
            $installer::logger::Lang->printf("source product code is %s\n", $language_data->{'product-code'});
            $installer::logger::Lang->printf("source upgrade code is %s\n", $release_data->{'upgrade-code'});

            return (
                $language_data->{'product-code'},
                $release_data->{'upgrade-code'}
                );
        }
        else
        {
            $installer::logger::Info->printf(
                "Warning: can not access information about previous version %s and language %s\n",
                $installer::globals::source_version,
                $onelanguage);
            return (undef,undef);
        }
    }
    else
    {
        $installer::logger::Info->printf("Warning: can not access information about previous version %s\n",
            $installer::globals::source_version);
        return (undef,undef);
    }
}




=head2 set_global_code_variables ($languagesref, $allvariableshashref)

    Determine values for the product code and upgrade code of the target version.

    As perparation for building a Windows patch, certain conditions have to be fullfilled.
     - The upgrade code changes from old to new version
     - The product code remains the same
     In order to inforce that we have to access information about the source version.

    The resulting values are stored as global variables
        $installer::globals::productcode
        $installer::globals::upgradecode
    and as variables in the given hash
        $allvariableshashref->{'PRODUCTCODE'}
        $allvariableshashref->{'UPGRADECODE'}

=cut
sub set_global_code_variables ($$)
{
    my ($languagesref, $allvariableshashref) = @_;

    my ($source_product_code, $source_upgrade_code) = get_source_codes($languagesref);
    my ($target_product_code, $target_upgrade_code) = (undef, undef);

    if (defined $source_product_code && defined $source_upgrade_code)
    {
        if ($installer::globals::is_major_release)
        {
            # For a major release we have to change the product code.
            $target_product_code = "{" . create_guid() . "}";
            $installer::logger::Lang->printf("building a major release, created new product code %s\n",
                $target_product_code);

            # Let's do a paranoia check that the new and the old guids are
            # different.  In reality the new guid really has to be
            # different from all other guids used for * codes, components,
            # etc.
            if ($target_product_code eq $source_product_code)
            {
                installer::logger::PrintError(
                    "new GUID for product code is the same as the old product code but should be different.");
            }
        }
        else
        {
            # For minor or micro releases we have to keeep the old product code.
            $target_product_code = "{" . $source_product_code . "}";
            $installer::logger::Lang->printf("building a minor or micro release, reusing product code %s\n",
                $target_product_code);
        }

        $target_upgrade_code = "{" . create_guid() . "}";
        # Again, just one test for paranoia.
        if ($target_upgrade_code eq $source_upgrade_code)
        {
            installer::logger::PrintError(
                "new GUID for upgrade code is the same as the old upgrade code but should be different.");
        }
    }
    else
    {
        # There is no previous version with which to compare the product code.
        # Just create two new uuids.
        $target_product_code = "{" . create_guid() . "}";
        $target_upgrade_code = "{" . create_guid() . "}";
    }

    $installer::globals::productcode = $target_product_code;
    $installer::globals::upgradecode = $target_upgrade_code;
    $allvariableshashref->{'PRODUCTCODE'} = $target_product_code;
    $allvariableshashref->{'UPGRADECODE'} = $target_upgrade_code;

    $installer::logger::Lang->printf("target product code is %s\n", $target_product_code);
    $installer::logger::Lang->printf("target upgrade code is %s\n", $target_upgrade_code);
}




###############################################################
# Setting the product version used in property table and
# upgrade table. Saving in global variable $msiproductversion
###############################################################

sub set_msiproductversion
{
    my ( $allvariables ) = @_;

    my $productversion = $allvariables->{'PRODUCTVERSION'};

    if (( $productversion =~ /^\s*\d+\s*$/ ) && ( $productversion > 255 )) { $productversion = $productversion%256; }

    if ( $productversion =~ /^\s*(\d+)\.(\d+)\.(\d+)\s*$/ )
    {
        $productversion = $1 . "\." . $2 . $3 . "\." . $installer::globals::buildid;
    }
    elsif  ( $productversion =~ /^\s*(\d+)\.(\d+)\s*$/ )
    {
        $productversion = $1 . "\." . $2 . "\." . $installer::globals::buildid;
    }
    else
    {
        my $productminor = "00";
        if (( $allvariables->{'PACKAGEVERSION'} ) && ( $allvariables->{'PACKAGEVERSION'} ne "" ))
        {
            if ( $allvariables->{'PACKAGEVERSION'} =~ /^\s*(\d+)\.(\d+)\.(\d+)\s*$/ ) { $productminor = $2; }
        }

        $productversion = $productversion . "\." . $productminor . "\." . $installer::globals::buildid;
    }

    $installer::globals::msiproductversion = $productversion;

    # Setting $installer::globals::msimajorproductversion, to differ between old version in upgrade table

    if ( $installer::globals::msiproductversion =~ /^\s*(\d+)\./ )
    {
        my $major = $1;
        $installer::globals::msimajorproductversion = $major . "\.0\.0";
    }
}

#################################################################################
# Including the msi product version into the bootstrap.ini, Windows only
#################################################################################

sub put_msiproductversion_into_bootstrapfile
{
    my ($filesref) = @_;

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];

        if ( $onefile->{'gid'} eq "gid_Profile_Version_Ini" )
        {
            my $file = installer::files::read_file($onefile->{'sourcepath'});

            for ( my $j = 0; $j <= $#{$file}; $j++ )
            {
                ${$file}[$j] =~ s/\<msiproductversion\>/$installer::globals::msiproductversion/;
            }

            installer::files::save_file($onefile->{'sourcepath'}, $file);

            last;
        }
    }
}

####################################################################################
# Updating the file Property.idt dynamically
# Content:
# Property Value
####################################################################################

sub update_reglocat_table
{
    my ($basedir, $allvariables) = @_;

    my $reglocatfilename = $basedir . $installer::globals::separator . "RegLocat.idt";

    # Only do something, if this file exists

    if ( -f $reglocatfilename )
    {
        my $reglocatfile = installer::files::read_file($reglocatfilename);

        my $layername = "";
        if ( $allvariables->{'REGISTRYLAYERNAME'} )
        {
            $layername = $allvariables->{'REGISTRYLAYERNAME'};
        }
        else
        {
            for ( my $i = 0; $i <= $#{$reglocatfile}; $i++ )
            {
                if ( ${$reglocatfile}[$i] =~ /\bLAYERNAMETEMPLATE\b/ )
                {
                    installer::exiter::exit_program("ERROR: Variable \"REGISTRYLAYERNAME\" has to be defined", "update_reglocat_table");
                }
            }
        }

        if ( $layername ne "" )
        {
            # Updating the layername in

            for ( my $i = 0; $i <= $#{$reglocatfile}; $i++ )
            {
                ${$reglocatfile}[$i] =~ s/\bLAYERNAMETEMPLATE\b/$layername/;
            }

            # Saving the file
            installer::files::save_file($reglocatfilename ,$reglocatfile);
            my $infoline = "Updated idt file: $reglocatfilename\n";
            $installer::logger::Lang->print($infoline);
        }
    }
}



####################################################################################
# Updating the file RemoveRe.idt dynamically (RemoveRegistry.idt)
# The name of the component has to be replaced.
####################################################################################

sub update_removere_table
{
    my ($basedir) = @_;

    my $removeregistryfilename = $basedir . $installer::globals::separator . "RemoveRe.idt";

    # Only do something, if this file exists

    if ( -f $removeregistryfilename )
    {
        my $removeregistryfile = installer::files::read_file($removeregistryfilename);

        for ( my $i = 0; $i <= $#{$removeregistryfile}; $i++ )
        {
            for ( my $i = 0; $i <= $#{$removeregistryfile}; $i++ )
            {
                ${$removeregistryfile}[$i] =~ s/\bREGISTRYROOTCOMPONENT\b/$installer::globals::registryrootcomponent/;
            }
        }

        # Saving the file
        installer::files::save_file($removeregistryfilename ,$removeregistryfile);
        my $infoline = "Updated idt file: $removeregistryfilename \n";
        $installer::logger::Lang->print($infoline);
    }
}


1;

