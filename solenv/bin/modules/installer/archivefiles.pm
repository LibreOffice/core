#*************************************************************************
#
#   $RCSfile: archivefiles.pm,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2004-07-09 13:55:15 $
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

package installer::archivefiles;

use installer::converter;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::logger;
use installer::pathanalyzer;
use installer::systemactions;

#################################################################
# Analyzing files with flag ARCHIVE
#################################################################

sub resolving_archive_flag
{
    my ($filesarrayref, $additionalpathsref, $languagestringref, $loggingdir) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::archivefiles::resolving_archive_flag : $#{$filesarrayref} : $#{$additionalpathsref} : $$languagestringref : $loggingdir"); }

    my @newallfilesarray = ();

    my ($systemcall, $returnvalue, $infoline);

    my $unziplistfile = $loggingdir . "unziplist_" . $installer::globals::build . "_" . $installer::globals::compiler . "_" . $$languagestringref . ".txt";

    my $platformunzipdirbase = installer::systemactions::create_directories("zipfiles", $languagestringref);
    push(@installer::globals::removedirs, $platformunzipdirbase);

    installer::logger::include_header_into_logfile("Files with flag ARCHIVE:");

    my $repeat_unzip = 0;
    my $maxcounter = 0;

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        if ( $repeat_unzip ) { $i--; }  # decreasing the counter

        my $onefile = ${$filesarrayref}[$i];
        my $styles = "";

        if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }

        if ( $styles =~ /\bARCHIVE\b/ )     # copying, unzipping and changing the file list
        {
            my $iscommonfile = 0;
            my $sourcepath = $onefile->{'sourcepath'};

            if ( $sourcepath =~ /\Q$installer::globals::separator\E\bcommon$installer::globals::productextension\Q$installer::globals::separator\E/ )   # /common/ or /common.pro/
            {
                $iscommonfile = 1;
            }

            # creating directories

            my $onelanguage = $onefile->{'specificlanguage'};

            # files without language into directory "00"

            if ($onelanguage eq "") { $onelanguage = "00"; }

            my $unzipdir;

            # if ($iscommonfile) { $unzipdir = $commonunzipdirbase . $installer::globals::separator . $onelanguage . $installer::globals::separator; }
            # else { $unzipdir = $platformunzipdirbase . $installer::globals::separator . $onelanguage . $installer::globals::separator; }

            $unzipdir = $platformunzipdirbase . $installer::globals::separator . $onelanguage . $installer::globals::separator;

            installer::systemactions::create_directory($unzipdir);  # creating language specific subdirectories

            my $onefilename = $onefile->{'Name'};
            $onefilename =~ s/\./\_/g;      # creating new directory name
            $onefilename =~ s/\//\_/g;      # only because of /letter/fontunxpsprint.zip, the only zip file with path
            $unzipdir = $unzipdir . $onefilename . $installer::globals::separator;

            if ( $installer::globals::dounzip ) { installer::systemactions::create_directory($unzipdir); }  # creating subdirectories with the names of the zipfiles

            $systemcall = "$installer::globals::unzippath -l $sourcepath |";
            open (UNZIP, "$systemcall");
            my @zipfile = <UNZIP>;
            close (UNZIP);

            $infoline = "Systemcall: $systemcall\n";
            push( @installer::globals::logfileinfo, $infoline);

            if (!( $#zipfile > -1 ))    # the zipfile is empty
            {
                $infoline = "ERROR: Could not unzip $sourcepath\n";
                push( @installer::globals::logfileinfo, $infoline);

            }
            else
            {
                # now really unpacking the files
                # Parameter -o : overwrite files without prompting
                # Parameter -q : quiet mode

                if ( $installer::globals::dounzip )         # really unpacking the files
                {
                    $returnvalue = 1;
                    $systemcall = "$installer::globals::unzippath -o -q $sourcepath -d $unzipdir";
                    $returnvalue = system($systemcall);

                    $infoline = "Systemcall: $systemcall\n";
                    push( @installer::globals::logfileinfo, $infoline);

                    if ($returnvalue) { installer::exiter::exit_program("ERROR: $infoline", "resolving_archive_flag"); }

                    if ( ! $installer::globals::iswindowsbuild )
                    {
                        # Setting unix rights to "775" for all created directories inside the package

                        $systemcall = "cd $unzipdir; find . -type d -exec chmod 775 \{\} \\\;";
                        $returnvalue = system($systemcall);
                        $infoline = "Systemcall: $systemcall\n";
                        push( @installer::globals::logfileinfo, $infoline);

                        if ($returnvalue)
                        {
                            $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
                            push( @installer::globals::logfileinfo, $infoline);
                        }
                    }
                }

                my $zipfileref = \@zipfile;
                my $unziperror = 0;

                # Format: Length, Date, Time, Name
                # This includes new destination directories!

                for ( my $j = 0; $j <= $#{$zipfileref}; $j++ )
                {
                    my $line = ${$zipfileref}[$j];

                    # Format:
                    #    0 07-25-03  18:21   dir1/
                    # 1241 07-25-03  18:21   dir1/so7drawing.desktop

                    if ( $line =~ /^\s*(\d+)\s+(\S+)\s+(\S+)\s+(\S+.*\S+?)\s*$/ )
                    {
                        my $zipsize = $1;
                        my $zipdate = $2;
                        my $ziptime = $3;
                        my $zipname = $4;

                        # some directories and files (from the help) start with "./simpress.idx"

                        $zipname =~ s/^\s*\.\///;

                        if ($installer::globals::iswin) { $zipname =~ s/\//\\/g; }

                        # if ( $zipsize == 0 )  # also files can have a size of 0
                        if ( $zipname =~ /\Q$installer::globals::separator\E\s*$/ ) # slash or backslash at the end characterizes a directory
                        {
                            $zipname = $zipname . "\n";
                            push(@{$additionalpathsref}, $zipname);

                            # Also needed here:
                            # Name
                            # Language
                            # ismultilingual
                            # Basedirectory

                            # This is not needed, because the list of all directories for the
                            # epm list file is generated from the destination directories of the
                            # files included in the product!
                        }
                        else
                        {
                            my %newfile = ();
                            %newfile = %{$onefile};
                            $newfile{'Name'} = $zipname;
                            my $destination = $onefile->{'destination'};
                            installer::pathanalyzer::get_path_from_fullqualifiedname(\$destination);
                            $newfile{'destination'} = $destination . $zipname;
                            $newfile{'sourcepath'} = $unzipdir . $zipname;

                            my $sourcefiletest = $unzipdir . $zipname;
                            if ( ! -f $sourcefiletest )
                            {
                                $infoline = "ATTENTION: Unzip failed for $sourcefiletest!\n";
                                push( @installer::globals::logfileinfo, $infoline);
                                $unziperror = 1;
                            }

                            # only adding the new line into the files array, if not in repeat modus

                            if ( ! $repeat_unzip ) { push(@newallfilesarray, \%newfile); }
                        }
                    }
                }

                if ( $unziperror )
                {
                    print "WARNING: Repeating to unpack $sourcepath! \n";
                    $infoline = "ATTENTION: Repeating to unpack $sourcepath !\n";
                    push( @installer::globals::logfileinfo, $infoline);
                    $repeat_unzip = 1;
                    $maxcounter++;

                    if ( $maxcounter == 5 ) # exiting the program
                    {
                        installer::exiter::exit_program("ERROR: Failed to unzip $sourcepath !", "resolving_archive_flag");
                    }
                }
                else
                {
                    $infoline = "Info: $sourcepath unpacked without problems !\n";
                    push( @installer::globals::logfileinfo, $infoline);
                    $repeat_unzip = 0;
                    $maxcounter = 0;
                }
            }
        }
        else        # nothing to do here, no zipped file (no ARCHIVE flag)
        {
            push(@newallfilesarray, $onefile);
        }
    }

    $infoline = "\n";
    push( @installer::globals::logfileinfo, $infoline);

    return \@newallfilesarray;
}


1;
