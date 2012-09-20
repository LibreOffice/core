#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

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
# The window size for LZX compression
# CompressionMemory=15 | 16 | ... | 21
# Reference: http://msdn.microsoft.com/en-us/library/bb417343.aspx
    $oneline = ".Set CompressionMemory=$installer::globals::cabfilecompressionlevel\n";
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
    push(@installer::globals::logfileinfo, $infoline);
}

##########################################################################
# Lines in ddf files must not be longer than 256 characters.
# Therefore it can be useful to use relative paths. Then it is
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
    $sourcepath =~ s/^[\\\/]//;

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

    installer::logger::include_timestamp_into_logfile("Performance Info: ddf file generation start");

    if ( $^O =~ /cygwin/i ) { installer::worker::generate_cygwin_paths($filesref); }

    if ( $installer::globals::use_packages_for_cabs )
    {
        my $sequenceorder = get_sequenceorder($filesref);

        my $counter = 1;
        my $currentcabfile = "";

        while ( ( exists($sequenceorder->{$counter}) ) || ( exists($installer::globals::allmergemodulefilesequences{$counter}) ) ) # Taking care of files from merge modules
        {
            if ( exists($installer::globals::allmergemodulefilesequences{$counter}) )
            {
                # Skipping this sequence, it is not included in $filesref, because it is assigned to a file from a merge module.\n";
                $counter++;
                next;
            }

            # Files with increasing sequencerorder are included in one cab file
            my $onefile = ${$filesref}[$sequenceorder->{$counter}];
            my $cabinetfile = $onefile->{'assignedcabinetfile'};
            my $sourcepath =  $onefile->{'sourcepath'};
            if ( $^O =~ /cygwin/i ) { $sourcepath = $onefile->{'cyg_sourcepath'}; }
            my $uniquename =  $onefile->{'uniquename'};

            my $styles = "";
            my $doinclude = 1;
            if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; };
            if ( $styles =~ /\bDONT_PACK\b/ ) { $doinclude = 0; }

            # to avoid lines with more than 256 characters, it can be useful to use relative paths
            $sourcepath = make_relative_ddf_path($sourcepath);

            # all files with the same cabinetfile have increasing sequencenumbers

            my @ddffile = ();

            write_ddf_file_header(\@ddffile, $cabinetfile, $installdir);

            my $ddfline = "\"" . $sourcepath . "\" \"" . $uniquename . "\"\n";
            if ( $doinclude ) { push(@ddffile, $ddfline); }

            $counter++; # increasing the counter
            my $nextfile = "";
            my $nextcabinetfile = "";
            if ( exists($sequenceorder->{$counter}) ) { $nextfile = ${$filesref}[$sequenceorder->{$counter}]; }
            if ( $nextfile->{'assignedcabinetfile'} ) { $nextcabinetfile = $nextfile->{'assignedcabinetfile'}; }

            while ( $nextcabinetfile eq $cabinetfile )
            {
                $sourcepath =  $nextfile->{'sourcepath'};
                if ( $^O =~ /cygwin/i ) { $sourcepath = $nextfile->{'cyg_sourcepath'}; }
                # to avoid lines with more than 256 characters, it can be useful to use relative paths
                $sourcepath = make_relative_ddf_path($sourcepath);
                $uniquename =  $nextfile->{'uniquename'};
                my $localdoinclude = 1;
                my $nextfilestyles = "";
                if ( $nextfile->{'Styles'} ) { $nextfilestyles = $nextfile->{'Styles'}; }
                if ( $nextfilestyles =~ /\bDONT_PACK\b/ ) { $localdoinclude = 0; }
                $ddfline = "\"" . $sourcepath . "\" \"" . $uniquename . "\"\n";
                if ( $localdoinclude ) { push(@ddffile, $ddfline); }

                $counter++; # increasing the counter!
                $nextcabinetfile = "_lastfile_";
                if ( exists($sequenceorder->{$counter}) )
                {
                    $nextfile = ${$filesref}[$sequenceorder->{$counter}];
                    $nextcabinetfile = $nextfile->{'assignedcabinetfile'};
                }
            }

            # creating the DDF file

            my $ddffilename = $cabinetfile;
            $ddffilename =~ s/.cab/.ddf/;
            $ddfdir =~ s/\Q$installer::globals::separator\E\s*$//;
            $ddffilename = $ddfdir . $installer::globals::separator . $ddffilename;

            installer::files::save_file($ddffilename ,\@ddffile);
            my $infoline = "Created ddf file: $ddffilename\n";
            push(@installer::globals::logfileinfo, $infoline);

            # lines in ddf files must not be longer than 256 characters
            check_ddf_file(\@ddffile, $ddffilename);

            # Writing the makecab system call

            my $oneline = "makecab.exe /V3 /F " . $ddffilename . " 2\>\&1 |" . "\n";
            if ( $installer::globals::isunix )
            {
                $oneline = "$ENV{'OUTDIR_FOR_BUILD'}/bin/makecab.exe /V3 /F " . $ddffilename . " 2\>\&1 |" . "\n";
            }

            push(@cabfilelist, $oneline);

            # collecting all ddf files
            push(@installer::globals::allddffiles, $ddffilename);
        }
    }
    elsif ((( $installer::globals::cab_file_per_component ) || ( $installer::globals::fix_number_of_cab_files )) && ( $installer::globals::updatedatabase ))
    {
        my $sequenceorder = get_sequenceorder($filesref);

        my $counter = 1;
        my $currentcabfile = "";

        while ( ( exists($sequenceorder->{$counter}) ) || ( exists($installer::globals::allmergemodulefilesequences{$counter}) ) ) # Taking care of files from merge modules
        {
#           if ( exists($installer::globals::allmergemodulefilesequences{$counter}) )
#           {
#               # Skipping this sequence, it is not included in $filesref, because it is assigned to a file from a merge module.\n";
#               $counter++;
#               next;
#           }

            my $onefile = ${$filesref}[$sequenceorder->{$counter}];
            $counter++;

            my $cabinetfile = $onefile->{'cabinet'};
            my $sourcepath =  $onefile->{'sourcepath'};
            if ( $^O =~ /cygwin/i ) { $sourcepath = $onefile->{'cyg_sourcepath'}; }
            my $uniquename =  $onefile->{'uniquename'};

            my $styles = "";
            my $doinclude = 1;
            if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; };
            if ( $styles =~ /\bDONT_PACK\b/ ) { $doinclude = 0; }

            # to avoid lines with more than 256 characters, it can be useful to use relative paths
            $sourcepath = make_relative_ddf_path($sourcepath);

            my @ddffile = ();

            write_ddf_file_header(\@ddffile, $cabinetfile, $installdir);

            my $ddfline = "\"" . $sourcepath . "\" \"" . $uniquename . "\"\n";
            if ( $doinclude ) { push(@ddffile, $ddfline); }

            my $nextfile = "";
            if ( ${$filesref}[$sequenceorder->{$counter}] ) { $nextfile = ${$filesref}[$sequenceorder->{$counter}]; }

            my $nextcabinetfile = "";

            if ( $nextfile->{'cabinet'} ) { $nextcabinetfile = $nextfile->{'cabinet'}; }

            while ( $nextcabinetfile eq $cabinetfile )
            {
                $sourcepath =  $nextfile->{'sourcepath'};
                if ( $^O =~ /cygwin/i ) { $sourcepath = $nextfile->{'cyg_sourcepath'}; }
                # to avoid lines with more than 256 characters, it can be useful to use relative paths
                $sourcepath = make_relative_ddf_path($sourcepath);
                $uniquename =  $nextfile->{'uniquename'};
                my $localdoinclude = 1;
                my $nextfilestyles = "";
                if ( $nextfile->{'Styles'} ) { $nextfilestyles = $nextfile->{'Styles'}; }
                if ( $nextfilestyles =~ /\bDONT_PACK\b/ ) { $localdoinclude = 0; }
                $ddfline = "\"" . $sourcepath . "\" \"" . $uniquename . "\"\n";
                if ( $localdoinclude ) { push(@ddffile, $ddfline); }
                $counter++;
                $nextfile = "";
                $nextcabinetfile = "_lastfile_";
                if (( exists($sequenceorder->{$counter}) ) && ( ${$filesref}[$sequenceorder->{$counter}] ))
                {
                    $nextfile = ${$filesref}[$sequenceorder->{$counter}];
                    $nextcabinetfile = $nextfile->{'cabinet'};
                }
            }

            # creating the DDF file

            my $ddffilename = $cabinetfile;
            $ddffilename =~ s/.cab/.ddf/;
            $ddfdir =~ s/\Q$installer::globals::separator\E\s*$//;
            $ddffilename = $ddfdir . $installer::globals::separator . $ddffilename;

            installer::files::save_file($ddffilename ,\@ddffile);
            my $infoline = "Created ddf file: $ddffilename\n";
            push(@installer::globals::logfileinfo, $infoline);

            # lines in ddf files must not be longer than 256 characters
            check_ddf_file(\@ddffile, $ddffilename);

            # Writing the makecab system call

            my $oneline = "makecab.exe /V3 /F " . $ddffilename . " 2\>\&1 |" . "\n";
            if ( $installer::globals::isunix )
            {
                $oneline = "$ENV{'OUTDIR_FOR_BUILD'}/bin/makecab.exe /V3 /F " . $ddffilename . " 2\>\&1 |" . "\n";
            }

            push(@cabfilelist, $oneline);

            # collecting all ddf files
            push(@installer::globals::allddffiles, $ddffilename);
        }
    }
    elsif (( $installer::globals::cab_file_per_component ) || ( $installer::globals::fix_number_of_cab_files ))
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


            # to avoid lines with more than 256 characters, it can be useful to use relative paths
            $sourcepath = make_relative_ddf_path($sourcepath);

            # all files with the same cabinetfile are directly behind each other in the files collector

            my @ddffile = ();

            write_ddf_file_header(\@ddffile, $cabinetfile, $installdir);

            my $ddfline = "\"" . $sourcepath . "\" \"" . $uniquename . "\"\n";
            if ( $doinclude ) { push(@ddffile, $ddfline); }

            my $nextfile = ${$filesref}[$i+1];
            my $nextcabinetfile = "";

            if ( $nextfile->{'cabinet'} ) { $nextcabinetfile = $nextfile->{'cabinet'}; }

            while ( $nextcabinetfile eq $cabinetfile )
            {
                $sourcepath =  $nextfile->{'sourcepath'};
                if ( $^O =~ /cygwin/i ) { $sourcepath = $nextfile->{'cyg_sourcepath'}; }
                # to avoid lines with more than 256 characters, it can be useful to use relative paths
                $sourcepath = make_relative_ddf_path($sourcepath);
                $uniquename =  $nextfile->{'uniquename'};
                my $localdoinclude = 1;
                my $nextfilestyles = "";
                if ( $nextfile->{'Styles'} ) { $nextfilestyles = $nextfile->{'Styles'}; }
                if ( $nextfilestyles =~ /\bDONT_PACK\b/ ) { $localdoinclude = 0; }
                $ddfline = "\"" . $sourcepath . "\" \"" . $uniquename . "\"\n";
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
            push(@installer::globals::logfileinfo, $infoline);

            # lines in ddf files must not be longer than 256 characters
            check_ddf_file(\@ddffile, $ddffilename);

            # Writing the makecab system call

            my $oneline = "makecab.exe /V3 /F " . $ddffilename . " 2\>\&1 |" . "\n";
            if ( $installer::globals::isunix )
            {
                $oneline = "$ENV{'OUTDIR_FOR_BUILD'}/bin/makecab.exe /V3 /F " . $ddffilename . " 2\>\&1 |" . "\n";
            }

            push(@cabfilelist, $oneline);

            # collecting all ddf files
            push(@installer::globals::allddffiles, $ddffilename);
        }
    }
    elsif (( $installer::globals::one_cab_file ) && ( $installer::globals::updatedatabase ))
    {
        my $sequenceorder = get_sequenceorder($filesref);

        my $counter = 1;
        my $currentcabfile = "";

        while ( ( exists($sequenceorder->{$counter}) ) || ( exists($installer::globals::allmergemodulefilesequences{$counter}) ) ) # Taking care of files from merge modules
        {
            if ( exists($installer::globals::allmergemodulefilesequences{$counter}) )
            {
                # Skipping this sequence, it is not included in $filesref, because it is assigned to a file from a merge module.\n";
                $counter++;
                next;
            }

            my $onefile = ${$filesref}[$sequenceorder->{$counter}];

            $cabinetfile = $onefile->{'cabinet'};
            my $sourcepath =  $onefile->{'sourcepath'};
            if ( $^O =~ /cygwin/i ) { $sourcepath = $onefile->{'cyg_sourcepath'}; }
            my $uniquename =  $onefile->{'uniquename'};

            # to avoid lines with more than 256 characters, it can be useful to use relative paths
            $sourcepath = make_relative_ddf_path($sourcepath);

            if ( $counter == 1 ) { write_ddf_file_header(\@ddffile, $cabinetfile, $installdir); }

            my $styles = "";
            my $doinclude = 1;
            if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; };
            if ( $styles =~ /\bDONT_PACK\b/ ) { $doinclude = 0; }

            my $ddfline = "\"" . $sourcepath . "\" \"" . $uniquename . "\"\n";
            if ( $doinclude ) { push(@ddffile, $ddfline); }

            $counter++; # increasing the counter
        }

        # creating the DDF file

        my $ddffilename = $cabinetfile;
        $ddffilename =~ s/.cab/.ddf/;
        $ddfdir =~ s/[\/\\]\s*$//;
        $ddffilename = $ddfdir . $installer::globals::separator . $ddffilename;

        installer::files::save_file($ddffilename ,\@ddffile);
        my $infoline = "Created ddf file: $ddffilename\n";
        push(@installer::globals::logfileinfo, $infoline);

        # lines in ddf files must not be longer than 256 characters
        check_ddf_file(\@ddffile, $ddffilename);

        # Writing the makecab system call

        # my $oneline = "makecab.exe /F " . $ddffilename . "\n";
        my $oneline = "makecab.exe /V3 /F " . $ddffilename . " 2\>\&1 |" . "\n";
        if ( $installer::globals::isunix )
        {
            $oneline = "$ENV{'OUTDIR_FOR_BUILD'}/bin/makecab.exe /V3 /F " . $ddffilename . " 2\>\&1 |" . "\n";
        }

        push(@cabfilelist, $oneline);

        # collecting all ddf files
        push(@installer::globals::allddffiles, $ddffilename);
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

            # to avoid lines with more than 256 characters, it can be useful to use relative paths
            $sourcepath = make_relative_ddf_path($sourcepath);

            if ( $i == 0 ) { write_ddf_file_header(\@ddffile, $cabinetfile, $installdir); }

            my $styles = "";
            my $doinclude = 1;
            if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; };
            if ( $styles =~ /\bDONT_PACK\b/ ) { $doinclude = 0; }

            my $ddfline = "\"" . $sourcepath . "\" \"" . $uniquename . "\"\n";
            if ( $doinclude ) { push(@ddffile, $ddfline); }
        }

        # creating the DDF file

        my $ddffilename = $cabinetfile;
        $ddffilename =~ s/.cab/.ddf/;
        $ddfdir =~ s/[\/\\]\s*$//;
        $ddffilename = $ddfdir . $installer::globals::separator . $ddffilename;

        installer::files::save_file($ddffilename ,\@ddffile);
        my $infoline = "Created ddf file: $ddffilename\n";
        push(@installer::globals::logfileinfo, $infoline);

        # lines in ddf files must not be longer than 256 characters
        check_ddf_file(\@ddffile, $ddffilename);

        # Writing the makecab system call

        my $oneline = "makecab.exe /F " . $ddffilename . "\n";
        if ( $installer::globals::isunix )
        {
            $oneline = "$ENV{'OUTDIR_FOR_BUILD'}/bin/makecab.exe /F " . $ddffilename . "\n";
        }

        push(@cabfilelist, $oneline);

        # collecting all ddf files
        push(@installer::globals::allddffiles, $ddffilename);
    }
    else
    {
        installer::exiter::exit_program("ERROR: No cab file specification in globals.pm !", "generate_cab_file_list");
    }

    installer::logger::include_timestamp_into_logfile("Performance Info: ddf file generation end");

    return \@cabfilelist;   # contains all system calls for packaging process
}

########################################################################
# For update and patch reasons the pack order needs to be saved.
# The pack order is saved in the ddf files; the names and locations
# of the ddf files are saved in @installer::globals::allddffiles.
# The outputfile "packorder.txt" can be saved in
# $installer::globals::infodirectory .
########################################################################

sub save_packorder
{
    installer::logger::include_header_into_logfile("Saving pack order");

    installer::logger::include_timestamp_into_logfile("Performance Info: saving pack order start");

    my $packorderfilename = "packorder.txt";
    $packorderfilename = $installer::globals::infodirectory . $installer::globals::separator . $packorderfilename;

    my @packorder = ();

    my $headerline = "\# Syntax\: Filetable_Sequence Cabinetfilename Physical_FileName Unique_FileName\n\n";
    push(@packorder, $headerline);

    for ( my $i = 0; $i <= $#installer::globals::allddffiles; $i++ )
    {
        my $ddffilename = $installer::globals::allddffiles[$i];
        my $ddffile = installer::files::read_file($ddffilename);
        my $cabinetfile = "";

        for ( my $j = 0; $j <= $#{$ddffile}; $j++ )
        {
            my $oneline = ${$ddffile}[$j];

            # Getting the Cabinet file name

            if ( $oneline =~ /^\s*\.Set\s+CabinetName.*\=(.*?)\s*$/ ) { $cabinetfile = $1; }
            if ( $oneline =~ /^\s*\.Set\s+/ ) { next; }

            if ( $oneline =~ /^\s*\"(.*?)\"\s+\"(.*?)\"\s*$/ )
            {
                my $sourcefile = $1;
                my $uniquefilename = $2;

                installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$sourcefile);

                # Using the hash created in create_files_table for performance reasons to get the sequence number
                my $filesequence = "";
                if ( exists($installer::globals::uniquefilenamesequence{$uniquefilename}) ) { $filesequence = $installer::globals::uniquefilenamesequence{$uniquefilename}; }
                else { installer::exiter::exit_program("ERROR: No sequence number value for $uniquefilename !", "save_packorder"); }

                my $line = $filesequence . "\t" . $cabinetfile . "\t" . $sourcefile . "\t" . $uniquefilename . "\n";
                push(@packorder, $line);
            }
        }
    }

    installer::files::save_file($packorderfilename ,\@packorder);

    installer::logger::include_timestamp_into_logfile("Performance Info: saving pack order end");
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
    if ( $installer::globals::isunix )
    {
        $msidb = "$ENV{'OUTDIR_FOR_BUILD'}/bin/msidb.exe";
    }
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
    if ( $^O =~ /linux/i ) {
        $extraslash = "\\";
    }
    my $systemcall = $msidb . " -f " . $idtdirbase . " -d " . $msifilename . " -c " . "-i " . $extraslash . "*";

    my $returnvalue = system($systemcall);

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute $msidb!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Success: Executed $msidb successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
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

    my $guidref = get_guid_list(1, 1);  # only one GUID shall be generated

    ${$guidref}[0] =~ s/\s*$//;     # removing ending spaces

    my $guid = "\{" . ${$guidref}[0] . "\}";

    my $infoline = "PackageCode: $guid\n";
    push( @installer::globals::logfileinfo, $infoline);

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

    # -g : required msi version
    # -c : codepage
    # -p : template

    installer::logger::include_header_into_logfile("Writing summary information stream");

    my $msiinfo = "msiinfo.exe";    # Has to be in the path
    if ( $installer::globals::isunix )
    {
        $msiinfo = "$ENV{'OUTDIR_FOR_BUILD'}/bin/msiinfo.exe";
    }

    my $sislanguage = "en-US";  # title, comment, keyword, and appname are always in English

    my $msiversion = get_msiversion_for_sis();
    my $codepage = get_codepage_for_sis($language);
    my $template = get_template_for_sis($language, $allvariableshashref);
    my $guid = get_packagecode_for_sis();
    my $title = get_title_for_sis($sislanguage,$languagefile, "OOO_SIS_TITLE");
    my $author = get_author_for_sis();
    my $subject = get_subject_for_sis($allvariableshashref);
    my $comment = get_comment_for_sis($sislanguage,$languagefile, "OOO_SIS_COMMENT");
    $comment =~ s/\[ProductName\]/$allvariableshashref->{'PRODUCTNAME'}/;
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
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute $systemcall (return $returnvalue)\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Success: Executed $msiinfo successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
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

    my $cscript = "cscript.exe";    # Has to be in the path
    my $msitran = "msitran.exe";    # Has to be in the path
    my $msidb = "msidb.exe";    # Has to be in the path
    if ( $installer::globals::isunix )
    {
        $infoline = "ERROR: We cannot create transformations yet (we cannot use cscript.exe when cross-compiling)\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    my $tmpdir = $ENV{TMPDIR};    # Variable %TEMP% will be set to it for WiLangId.vbs to work
    my $wilangid = $ENV{WINDOWS_SDK_HOME} . "/Samples/SysMgmt/Msi/scripts/WiLangId.vbs";

    my $from = cwd();

    my $templatevalue = "1033";

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

        my $windowslanguage = installer::windows::language::get_windows_language($onelanguage);
        my $transformfile = $installdir . $installer::globals::separator .  $windowslanguage;

        my $systemcall = $msitran . " " . " -g " . $basedbname . " " . $referencedbname . " " . $transformfile . " " . $errorhandling;

        my $returnvalue = system($systemcall);

        my $infoline = "Systemcall: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);

        # Problem: msitran.exe in version 4.0 always returns "1", even if no failure occurred.
        # Therefore it has to be checked, if this is version 4.0. If yes, if the mst file
        # exists and if it is larger than 0 bytes. If this is true, then no error occurred.
        # File Version of msitran.exe: 4.0.6000.16384 has checksum: "b66190a70145a57773ec769e16777b29".
        # Same for msitran.exe from wntmsci12: "aa25d3445b94ffde8ef0c1efb77a56b8"

        if ($returnvalue)
        {
            $infoline = "WARNING: Returnvalue of $msitran is not 0. Checking version of $msitran!\n";
            push( @installer::globals::logfileinfo, $infoline);

            open(FILE, "<$installer::globals::msitranpath") or die "ERROR: Can't open $installer::globals::msitranpath for creating file hash";
            binmode(FILE);
            my $digest = Digest::MD5->new->addfile(*FILE)->hexdigest;
            close(FILE);

            my @problemchecksums = ("b66190a70145a57773ec769e16777b29", "aa25d3445b94ffde8ef0c1efb77a56b8", "748206e54fc93efe6a1aaa9d491f3ad1");
            my $isproblemchecksum = 0;

            foreach my $problemchecksum ( @problemchecksums )
            {
                $infoline = "Checksum of problematic MsiTran.exe: $problemchecksum\n";
                push( @installer::globals::logfileinfo, $infoline);
                $infoline = "Checksum of used MsiTran.exe: $digest\n";
                push( @installer::globals::logfileinfo, $infoline);
                if ( $digest eq $problemchecksum ) { $isproblemchecksum = 1; }
            }

            if ( $isproblemchecksum )
            {
                # Check existence of mst
                if ( -f $transformfile )
                {
                    $infoline = "File $transformfile exists.\n";
                    push( @installer::globals::logfileinfo, $infoline);
                    my $filesize = ( -s $transformfile );
                    $infoline = "Size of $transformfile: $filesize\n";
                    push( @installer::globals::logfileinfo, $infoline);

                    if ( $filesize > 0 )
                    {
                        $infoline = "Info: Returnvalue $returnvalue of $msitran is no problem :-) .\n";
                        push( @installer::globals::logfileinfo, $infoline);
                        $returnvalue = 0; # reset the error
                    }
                    else
                    {
                        $infoline = "Filesize indicates that an error occurred.\n";
                        push( @installer::globals::logfileinfo, $infoline);
                    }
                }
                else
                {
                    $infoline = "File $transformfile does not exist -> An error occurred.\n";
                    push( @installer::globals::logfileinfo, $infoline);
                }
            }
            else
            {
                $infoline = "This is not a problematic version of msitran.exe. Therefore the error is not caused by problematic msitran.exe.\n";
                push( @installer::globals::logfileinfo, $infoline);
            }
        }

        if ($returnvalue)
        {
            $infoline = "ERROR: Could not execute $msitran!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
        else
        {
            $infoline = "Success: Executed $msitran successfully!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }

        # The reference database can be deleted

        my $result = unlink($referencedbname);
        # $result contains the number of deleted files

        if ( $result == 0 )
        {
            $infoline = "ERROR: Could not remove file $$referencedbname !\n";
            push( @installer::globals::logfileinfo, $infoline);
            installer::exiter::exit_program($infoline, "create_transforms");
        }

        chdir($installdir);
        $systemcall = $msidb . " " . " -d " . $basedbname . " -r " . $windowslanguage;
        system($systemcall);
        # fdo#46181 - zh-HK and zh-MO should have fallen back to zh-TW not to zh-CN
        # we need to hack zh-HK and zh-MO LCIDs directly into the MSI
        if($windowslanguage eq '1028')
        {
            rename 1028,3076;
            $systemcall = $msidb . " " . " -d " . $basedbname . " -r " . 3076;
            system($systemcall);
            rename 3076,5124;
            $systemcall = $msidb . " " . " -d " . $basedbname . " -r " . 5124;
            system($systemcall);
            $templatevalue = $templatevalue . "," . 3076 . "," . 5124;
            rename 5124,1028;
        }
        chdir($from);
        unlink($transformfile);

        $infoline = "Systemcall: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);

        if ( $windowslanguage ne '1033')
        {
            $templatevalue = $templatevalue . "," . $windowslanguage;
        }
    }

    $systemcall = "TEMP=" . $tmpdir . " " . $cscript . " " . $wilangid . " " . $basedbname . " Package " . $templatevalue;

    $returnvalue = system($systemcall);

    $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute WiLangId.vbs!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Success: Executed WiLangId.vbs successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
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

#################################################################
# Copying MergeModules for the Windows installer into the
# installation set. The list of MergeModules is located
# in %installer::globals::copy_msm_files
#################################################################

sub copy_merge_modules_into_installset
{
    my ($installdir) = @_;

    installer::logger::include_header_into_logfile("Copying Merge files into installation set");

    my $cabfile;
    foreach $cabfile ( keys  %installer::globals::copy_msm_files )
    {
        my $sourcefile  = $installer::globals::copy_msm_files{$cabfile};
        my $destfile = $installdir . $installer::globals::separator . $cabfile;

        installer::systemactions::copy_one_file($sourcefile, $destfile);
    }
}

#################################################################
# Getting a list of GUID using uuidgen.exe.
# This works only on Windows
#################################################################

sub get_guid_list
{
    my ($number, $log) = @_;

    if ( $log ) { installer::logger::include_header_into_logfile("Generating $number GUID"); }

    my $uuidgen = $ENV{'UUIDGEN'};        # Has to be in the path

    # "-c" for uppercase output

    my $systemcall = "$uuidgen -n$number |";
    if ( $installer::globals::isunix )
    {
        # -n is not present in the non-windows uuidgen
        $systemcall = "for I in `seq 1 $number` ; do uuidgen ; done |";
    }
    open (UUIDGEN, "$systemcall" ) or die("uuidgen is missing.");
    my @uuidlist = <UUIDGEN>;
    close (UUIDGEN);

    my $infoline = "Systemcall: $systemcall\n";
    if ( $log ) { push( @installer::globals::logfileinfo, $infoline); }

    my $comparenumber = $#uuidlist + 1;

    if ( $comparenumber == $number )
    {
        $infoline = "Success: Executed $uuidgen successfully!\n";
        if ( $log ) { push( @installer::globals::logfileinfo, $infoline); }
    }
    else
    {
        $infoline = "ERROR: Could not execute $uuidgen successfully!\n";
        if ( $log ) { push( @installer::globals::logfileinfo, $infoline); }
    }

    # uppercase, no longer "-c", because this is only supported in uuidgen.exe v.1.01
    for ( my $i = 0; $i <= $#uuidlist; $i++ ) { $uuidlist[$i] = uc($uuidlist[$i]); }

    return \@uuidlist;
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

    my ($first, $second, $third, $fourth, $fifth) = unpack ('A8 A4 A4 A4 A12', $digest);
    $guid = "$first-$second-$third-$fourth-$fifth";

    return $guid;
}

#################################################################
# Filling real component GUID into the component table.
# This works only on Windows
#################################################################

sub set_uuid_into_component_table
{
    my ($idtdirbase, $allvariables) = @_;

    my $componenttablename  = $idtdirbase . $installer::globals::separator . "Componen.idt";

    my $componenttable = installer::files::read_file($componenttablename);

    # For update and patch reasons (small update) the GUID of an existing component must not change!
    # The collection of component GUIDs is saved in the directory $installer::globals::idttemplatepath in the file "components.txt"

    my $infoline = "";
    my $counter = 0;

    for ( my $i = 3; $i <= $#{$componenttable}; $i++ )  # ignoring the first three lines
    {
        my $oneline = ${$componenttable}[$i];
        my $componentname = "";
        if ( $oneline =~ /^\s*(\S+?)\t/ ) { $componentname = $1; }

        my $uuid = "";

            if ( exists($installer::globals::calculated_component_guids{$componentname}))
            {
                $uuid = $installer::globals::calculated_component_guids{$componentname};
            }
            else
            {
                # Calculating new GUID with the help of the component name.
                my $useproductversion = 1;
                if ( exists($installer::globals::base_independent_components{$componentname})) { $useproductversion = 0; }
                my $sourcestring = $componentname;

                if ( $useproductversion )
                {
                    if ( ! exists($allvariables->{'PRODUCTVERSION'}) ) { installer::exiter::exit_program("ERROR: Could not find variable \"PRODUCTVERSION\" (required value for GUID creation)!", "set_uuid_into_component_table"); }
                    $sourcestring = $sourcestring . "_" . $allvariables->{'PRODUCTVERSION'};
                }
                $uuid = calculate_guid($sourcestring);
                $counter++;

                # checking, if there is a conflict with an already created guid
                if ( exists($installer::globals::allcalculated_guids{$uuid}) ) { installer::exiter::exit_program("ERROR: \"$uuid\" was already created before!", "set_uuid_into_component_table"); }
                $installer::globals::allcalculated_guids{$uuid} = 1;
                $installer::globals::calculated_component_guids{$componentname} = $uuid;
            }

        ${$componenttable}[$i] =~ s/COMPONENTGUID/$uuid/;
    }

    installer::files::save_file($componenttablename, $componenttable);
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
                push(@installer::globals::logfileinfo, $infoline);
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
                $filecontent = installer::files::read_file($fullfilename);

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
                    push(@installer::globals::logfileinfo, $infoline);
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
    push( @installer::globals::logfileinfo, $infoline);

    my $msidb = "msidb.exe";    # Has to be in the path
    if ( $installer::globals::isunix )
    {
        $msidb = "$ENV{'OUTDIR_FOR_BUILD'}/bin/msidb.exe";
    }
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
        push( @installer::globals::logfileinfo, $infoline);

        if ($returnvalue)
        {
            $infoline = "ERROR: Could not execute $systemcall !\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
        else
        {
            $infoline = "Success: Executed $systemcall successfully!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }

        # deleting the cab file

        unlink(${$allcabfiles}[$i]);

        $infoline = "Deleted cab file: ${$allcabfiles}[$i]\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    $infoline = "Changing back into directory: $from";
    push( @installer::globals::logfileinfo, $infoline);

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

    installer::logger::include_timestamp_into_logfile("Performance Info: Execute packaging start");

    my $infoline = "";
    my $from = cwd();
    my $to = $loggingdir;

    chdir($to);
    $infoline = "chdir: $to \n";
    push( @installer::globals::logfileinfo, $infoline);

    # the ddf file contains relative paths, it is necessary to change into the temp directory
    $to = $installer::globals::temppath;
    chdir($to);
    $infoline = "chdir: $to \n";
    push( @installer::globals::logfileinfo, $infoline);

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

        installer::logger::print_message( "... makecab.exe ($callscounter/$allmakecabcalls) ... \n" );

        for ( my $n = 1; $n <= $maxmakecabcalls; $n++ )
        {
            my @ddfoutput = ();

            $infoline = "Systemcall: $systemcall";
            push( @installer::globals::logfileinfo, $infoline);

            open (DDF, "$systemcall");
            while (<DDF>) {push(@ddfoutput, $_); }
            close (DDF);

            my $returnvalue = $?;   # $? contains the return value of the systemcall

            if ($returnvalue)
            {
                if ( $n < $maxmakecabcalls )
                {
                    installer::logger::print_message( "makecab_error (Try $n): Trying again \n" );
                    $infoline = "makecab_error (Try $n): $systemcall !";
                }
                else
                {
                    installer::logger::print_message( "ERROR (Try $n): Abort packing \n" );
                    $infoline = "ERROR (Try $n): $systemcall !";
                }

                push( @installer::globals::logfileinfo, $infoline);

                for ( my $m = 0; $m <= $#ddfoutput; $m++ )
                {
                    if ( $ddfoutput[$m] =~ /(ERROR\:.*?)\s*$/ )
                    {
                        $infoline = $1 . "\n";
                        if ( $n < $maxmakecabcalls ) { $infoline =~ s/ERROR\:/makecab_error\:/i; }
                        installer::logger::print_message( $infoline );
                        push( @installer::globals::logfileinfo, $infoline);
                    }
                }

                if ( $n == $maxmakecabcalls ) { installer::exiter::exit_program("ERROR: \"$systemcall\"!", "execute_packaging"); }
            }
            else
            {
                $infoline = "Success (Try $n): $systemcall";
                push( @installer::globals::logfileinfo, $infoline);
                last;
            }
        }
    }

    installer::logger::include_timestamp_into_logfile("Performance Info: Execute packaging end");

    # setting back to the original tmp directory
    $ENV{'TMP'} = $origtemppath;

    chdir($from);
    $infoline = "chdir: $from \n";
    push( @installer::globals::logfileinfo, $infoline);
}

###############################################################
# Setting the global variables ProductCode and the UpgradeCode
###############################################################

sub set_global_code_variables
{
    my ( $languagesref, $languagestringref, $allvariableshashref, $alloldproperties ) = @_;

    # In the msi template directory a files "codes.txt" has to exist, in which the ProductCode
    # and the UpgradeCode for the product are defined.
    # The name "codes.txt" can be overwritten in Product definition with CODEFILENAME .
    # Default $installer::globals::codefilename is defined in parameter.pm.

    if ( $allvariableshashref->{'CODEFILENAME'} )
    {
        $installer::globals::codefilename = $installer::globals::idttemplatepath  . $installer::globals::separator . $allvariableshashref->{'CODEFILENAME'};
        installer::files::check_file($installer::globals::codefilename);
    }

    my $infoline = "Using Codes file: $installer::globals::codefilename \n";
    push( @installer::globals::logfileinfo, $infoline);

    my $codefile = installer::files::read_file($installer::globals::codefilename);

    my $onelanguage = "";

    if ( $#{$languagesref} > 0 )    # more than one language
    {
        if (( $installer::globals::added_english ) && ( $#{$languagesref} == 1 )) # only multilingual because of added English
        {
            $onelanguage = ${$languagesref}[1];  # setting the first language, that is not english
        }
        else
        {
            if (( ${$languagesref}[1] =~ /jp/ ) ||
                ( ${$languagesref}[1] =~ /ko/ ) ||
                ( ${$languagesref}[1] =~ /zh/ ))
            {
                $onelanguage = "multiasia";
            }
            else
            {
                $onelanguage = "multiwestern";
            }
        }
    }
    else    # only one language
    {
        $onelanguage = ${$languagesref}[0];
    }

    # ProductCode must not change, if Windows patches shall be applied
    if ( $installer::globals::updatedatabase )
    {
        $installer::globals::productcode = $alloldproperties->{'ProductCode'};
    }
    elsif ( $installer::globals::prepare_winpatch )
    {
        # ProductCode has to be specified in each language
        my $searchstring = "PRODUCTCODE";
        my $codeblock = installer::windows::idtglobal::get_language_block_from_language_file($searchstring, $codefile);
        $installer::globals::productcode = installer::windows::idtglobal::get_code_from_code_block($codeblock, $onelanguage);
    } else {
        my $guidref = get_guid_list(1, 1);  # only one GUID shall be generated
        ${$guidref}[0] =~ s/\s*$//;     # removing ending spaces
        $installer::globals::productcode = "\{" . ${$guidref}[0] . "\}";
    }

    if ( $installer::globals::patch ) # patch upgrade codes are defined in soffice.lst
    {
        if ( $allvariableshashref->{'PATCHUPGRADECODE'} ) { $installer::globals::upgradecode = $allvariableshashref->{'PATCHUPGRADECODE'}; }
        else { installer::exiter::exit_program("ERROR: PATCHUPGRADECODE not defined in list file!", "set_global_code_variables"); }
    }
    else
    {
        # UpgradeCode can take english as default, if not defined in specified language

        $searchstring = "UPGRADECODE";  # searching in the codes.txt file
        $codeblock = installer::windows::idtglobal::get_language_block_from_language_file($searchstring, $codefile);
        $installer::globals::upgradecode = installer::windows::idtglobal::get_language_string_from_language_block($codeblock, $onelanguage, "");
    }

    if ( $installer::globals::upgradecode eq "" ) { installer::exiter::exit_program("ERROR: UpgradeCode not defined in $installer::globals::codefilename !", "set_global_code_variables"); }

    $infoline = "Setting ProductCode to: $installer::globals::productcode \n";
    push( @installer::globals::logfileinfo, $infoline);
    $infoline = "Setting UpgradeCode to: $installer::globals::upgradecode \n";
    push( @installer::globals::logfileinfo, $infoline);

    # Adding both variables into the variables array

    $allvariableshashref->{'PRODUCTCODE'} = $installer::globals::productcode;
    $allvariableshashref->{'UPGRADECODE'} = $installer::globals::upgradecode;

    $infoline = "Defined variable PRODUCTCODE: $installer::globals::productcode \n";
    push( @installer::globals::logfileinfo, $infoline);

    $infoline = "Defined variable UPGRADECODE: $installer::globals::upgradecode \n";
    push( @installer::globals::logfileinfo, $infoline);

}

###############################################################
# Setting the product version used in property table and
# upgrade table. Saving in global variable $msiproductversion
###############################################################

sub set_msiproductversion
{
    my ( $allvariables ) = @_;

    my $productversion = $allvariables->{'PACKAGEVERSION'};

    if ( $productversion =~ /^\s*(\d+)\.(\d+)\.(\d+)\s*$/ )
    {
        $productversion = $1 . "\." . $2 . "\." . $3 . "\." . $installer::globals::buildid;
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

        if ( $onefile->{'gid'} eq "gid_Brand_Profile_Version_Ini" )
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
            push(@installer::globals::logfileinfo, $infoline);
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
        push(@installer::globals::logfileinfo, $infoline);
    }
}

##########################################################################
# Reading saved mappings in Files.idt and Director.idt.
# This is required, if installation sets shall be created,
# that can be used for creation of msp files.
##########################################################################

sub read_saved_mappings
{
    installer::logger::include_header_into_logfile("Reading saved mappings from older installation sets:");

    installer::logger::include_timestamp_into_logfile("Performance Info: Reading saved mappings start");

    if ( $installer::globals::previous_idt_dir )
    {
        my @errorlines = ();
        my $errorstring = "";
        my $error_occurred = 0;
        my $file_error_occurred = 0;
        my $dir_error = 0;

        my $idtdir = $installer::globals::previous_idt_dir;
        $idtdir =~ s/\Q$installer::globals::separator\E\s*$//;

        # Reading File.idt

        my $idtfile = $idtdir . $installer::globals::separator . "File.idt";
        push( @installer::globals::globallogfileinfo, "\nAnalyzing file: $idtfile\n" );
        if ( ! -f $idtfile ) { push( @installer::globals::globallogfileinfo, "Warning: File $idtfile does not exist!\n" ); }

        my $n = 0;
        open (F, "<$idtfile") || installer::exiter::exit_program("ERROR: Cannot open file $idtfile for reading", "read_saved_mappings");
        <F>; <F>; <F>;
        while (<F>)
        {
            m/^([^\t]+)\t([^\t]+)\t((.*)\|)?([^\t]*)/;
            print "OUT1: \$1: $1, \$2: $2, \$3: $3, \$4: $4, \$5: $5\n";
            next if ("$1" eq "$5") && (!defined($3));
            my $lc1 = lc($1);

            if ( exists($installer::globals::savedmapping{"$2/$5"}))
            {
                if ( ! $file_error_occurred )
                {
                    $errorstring = "\nErrors in $idtfile: \n";
                    push(@errorlines, $errorstring);
                }
                $errorstring = "Duplicate savedmapping{" . "$2/$5}\n";
                push(@errorlines, $errorstring);
                $error_occurred = 1;
                $file_error_occurred = 1;
            }

            if ( exists($installer::globals::savedrevmapping{$lc1}))
            {
                if ( ! $file_error_occurred )
                {
                    $errorstring = "\nErrors in $idtfile: \n";
                    push(@errorlines, $errorstring);
                }
                $errorstring = "Duplicate savedrevmapping{" . "$lc1}\n";
                push(@errorlines, $errorstring);
                $error_occurred = 1;
                $file_error_occurred = 1;
            }

            my $shortname = $4 || '';

            # Don't reuse illegal 8.3 mappings that we used to generate in 2.0.4
            if (index($shortname, '.') > 8 ||
                (index($shortname, '.') == -1 && length($shortname) > 8))
            {
                $shortname = '';
            }

            if (( $shortname ne '' ) && ( index($shortname, '~') > 0 ) && ( exists($installer::globals::savedrev83mapping{$shortname}) ))
            {
                if ( ! $file_error_occurred )
                {
                    $errorstring = "\nErrors in $idtfile: \n";
                    push(@errorlines, $errorstring);
                }
                $errorstring = "Duplicate savedrev83mapping{" . "$shortname}\n";
                push(@errorlines, $errorstring);
                $error_occurred = 1;
                $file_error_occurred = 1;
            }

            $installer::globals::savedmapping{"$2/$5"} = "$1;$shortname";
            $installer::globals::savedrevmapping{lc($1)} = "$2/$5";
            $installer::globals::savedrev83mapping{$shortname} = "$2/$5" if $shortname ne '';
            $n++;
        }

        close (F);

        push( @installer::globals::globallogfileinfo, "Read $n old file table key or 8.3 name mappings from $idtfile\n" );

        # Reading Director.idt

        $idtfile = $idtdir . $installer::globals::separator . "Director.idt";
        push( @installer::globals::globallogfileinfo, "\nAnalyzing file $idtfile\n" );
        if ( ! -f $idtfile ) { push( @installer::globals::globallogfileinfo, "Warning: File $idtfile does not exist!\n" ); }

        $n = 0;
        open (F, "<$idtfile") || installer::exiter::exit_program("ERROR: Cannot open file $idtfile for reading", "read_saved_mappings");
        <F>; <F>; <F>;
        while (<F>)
        {
            m/^([^\t]+)\t([^\t]+)\t(([^~]+~\d.*)\|)?([^\t]*)/;
            next if (!defined($3));
            my $lc1 = lc($1);

            print "OUT2: \$1: $1, \$2: $2, \$3: $3\n";

            if ( exists($installer::globals::saved83dirmapping{$1}) )
            {
                if ( ! $dir_error_occurred )
                {
                    $errorstring = "\nErrors in $idtfile: \n";
                    push(@errorlines, $errorstring);
                }
                $errorstring = "Duplicate saved83dirmapping{" . "$1}\n";
                push(@errorlines, $errorstring);
                $error_occurred = 1;
                $dir_error_occurred = 1;
            }

            $installer::globals::saved83dirmapping{$1} = $4;
            $n++;
        }
        close (F);

        push( @installer::globals::globallogfileinfo, "Read $n old directory 8.3 name mappings from $idtfile\n" );

        # Analyzing errors

        if ( $error_occurred )
        {
            for ( my $i = 0; $i <= $#errorlines; $i++ )
            {
                print "$errorlines[$i]";
                push( @installer::globals::globallogfileinfo, "$errorlines[$i]");
            }
            installer::exiter::exit_program("ERROR: Duplicate entries in saved mappings!", "read_saved_mappings");
        }
    } else {
        installer::exiter::exit_program("ERROR: Windows patch shall be prepared, but environment variable PREVIOUS_IDT_DIR is not set!", "read_saved_mappings");
    }

    installer::logger::include_timestamp_into_logfile("Performance Info: Reading saved mappings end");
}

1;

# vim:set shiftwidth=4 softtabstop=4 expandtab:
