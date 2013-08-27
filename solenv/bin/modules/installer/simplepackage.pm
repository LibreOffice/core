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

package installer::simplepackage;

use Cwd;
use File::Copy;
use installer::download;
use installer::exiter;
use installer::globals;
use installer::logger;
use installer::strip qw(strip_libraries);
use installer::systemactions;
use installer::worker;

####################################################
# Checking if the simple packager is required.
# This can be achieved by setting the global
# variable SIMPLE_PACKAGE in *.lst file or by
# setting the environment variable SIMPLE_PACKAGE.
####################################################

sub check_simple_packager_project
{
    my ( $allvariables ) = @_;

    if (( $installer::globals::packageformat eq "installed" ) ||
        ( $installer::globals::packageformat eq "archive" ))
    {
        $installer::globals::is_simple_packager_project = 1;
        $installer::globals::patch_user_dir = 1;
    }
    elsif( $installer::globals::packageformat eq "dmg" )
    {
        $installer::globals::is_simple_packager_project = 1;
    }
}

####################################################
# Detecting the directory with extensions
####################################################

sub get_extensions_dir
{
    my ( $subfolderdir ) = @_;

    my $extensiondir = $subfolderdir . $installer::globals::separator;
    if ( $installer::globals::officedirhostname ne "" ) { $extensiondir = $extensiondir . $installer::globals::officedirhostname . $installer::globals::separator; }
    my $extensionsdir = $extensiondir . "share" . $installer::globals::separator . "extensions";

    return $extensionsdir;
}

##################################################################
# Collecting all identifier from ulf file
##################################################################

sub get_identifier
{
    my ( $translationfile ) = @_;

    my @identifier = ();

    for ( my $i = 0; $i <= $#{$translationfile}; $i++ )
    {
        my $oneline = ${$translationfile}[$i];

        if ( $oneline =~ /^\s*\[(.+)\]\s*$/ )
        {
            my $identifier = $1;
            push(@identifier, $identifier);
        }
    }

    return \@identifier;
}

##############################################################
# Returning the complete block in all languages
# for a specified string
##############################################################

sub get_language_block_from_language_file
{
    my ($searchstring, $languagefile) = @_;

    my @language_block = ();

    for ( my $i = 0; $i <= $#{$languagefile}; $i++ )
    {
        if ( ${$languagefile}[$i] =~ /^\s*\[\s*$searchstring\s*\]\s*$/ )
        {
            my $counter = $i;

            push(@language_block, ${$languagefile}[$counter]);
            $counter++;

            while (( $counter <= $#{$languagefile} ) && (!( ${$languagefile}[$counter] =~ /^\s*\[/ )))
            {
                push(@language_block, ${$languagefile}[$counter]);
                $counter++;
            }

            last;
        }
    }

    return \@language_block;
}

##############################################################
# Returning a specific language string from the block
# of all translations
##############################################################

sub get_language_string_from_language_block
{
    my ($language_block, $language) = @_;

    my $newstring = "";

    for ( my $i = 0; $i <= $#{$language_block}; $i++ )
    {
        if ( ${$language_block}[$i] =~ /^\s*$language\s*\=\s*\"(.*)\"\s*$/ )
        {
            $newstring = $1;
            last;
        }
    }

    if ( $newstring eq "" )
    {
        $language = "en-US";    # defaulting to english

        for ( my $i = 0; $i <= $#{$language_block}; $i++ )
        {
            if ( ${$language_block}[$i] =~ /^\s*$language\s*\=\s*\"(.*)\"\s*$/ )
            {
                $newstring = $1;
                last;
            }
        }
    }

    return $newstring;
}

########################################################################
# Localizing the script for the Mac Language Pack installer
########################################################################

sub localize_scriptfile
{
    my ($scriptfile, $translationfile, $languagestringref) = @_;

    my $onelanguage = $$languagestringref;
    if ( $onelanguage =~ /^\s*(.*?)_/ ) { $onelanguage = $1; }

    # Analyzing the ulf file, collecting all Identifier
    my $allidentifier = get_identifier($translationfile);

    for ( my $i = 0; $i <= $#{$allidentifier}; $i++ )
    {
        my $identifier = ${$allidentifier}[$i];
        my $language_block = get_language_block_from_language_file($identifier, $translationfile);
        my $newstring = get_language_string_from_language_block($language_block, $onelanguage);

        # removing mask
        $newstring =~ s/\\\'/\'/g;

        replace_one_variable_in_shellscript($scriptfile, $newstring, $identifier);
    }
}

#################################################################################
# Replacing one variable in Mac shell script
#################################################################################

sub replace_one_variable_in_shellscript
{
    my ($scriptfile, $variable, $searchstring) = @_;

    for ( my $i = 0; $i <= $#{$scriptfile}; $i++ )
    {
        ${$scriptfile}[$i] =~ s/\[$searchstring\]/$variable/g;
    }
}

#############################################
# Replacing variables in Mac shell script
#############################################

sub replace_variables_in_scriptfile
{
    my ($scriptfile, $volume_name, $volume_name_app, $allvariables) = @_;

    replace_one_variable_in_shellscript($scriptfile, $volume_name, "FULLPRODUCTNAME" );
    replace_one_variable_in_shellscript($scriptfile, $volume_name_app, "FULLAPPPRODUCTNAME" );
    replace_one_variable_in_shellscript($scriptfile, $allvariables->{'PRODUCTNAME'}, "PRODUCTNAME" );
    replace_one_variable_in_shellscript($scriptfile, $allvariables->{'PRODUCTVERSION'}, "PRODUCTVERSION" );

    my $scriptname = $allvariables->{'BUNDLEIDENTIFIER'};

    replace_one_variable_in_shellscript($scriptfile, $scriptname, "SEARCHSCRIPTNAME" );
}

#############################################
# Creating the "simple" package.
# "zip" for Windows
# "tar.gz" for all other platforms
# additionally "dmg" on Mac OS X
#############################################

sub create_package
{
    my ( $installdir, $archivedir, $packagename, $allvariables, $includepatharrayref, $languagestringref, $format ) = @_;

    installer::logger::print_message( "... creating $installer::globals::packageformat file ...\n" );
    installer::logger::include_header_into_logfile("Creating $installer::globals::packageformat file:");

    # moving dir into temporary directory
    my $pid = $$; # process id
    my $tempdir = $installdir . "_temp" . "." . $pid;
    my $systemcall = "";
    my $from = "";
    my $makesystemcall = 1;
    my $return_to_start = 0;
    installer::systemactions::rename_directory($installdir, $tempdir);

    # creating new directory with original name
    installer::systemactions::create_directory($archivedir);

    my $archive = $archivedir . $installer::globals::separator . $packagename . $format;

    if ( $archive =~ /zip$/ )
    {
        $from = cwd();
        $return_to_start = 1;
        chdir($tempdir);
        $systemcall = "$installer::globals::zippath -qr $archive .";

        # Using Archive::Zip fails because of very long path names below "share/uno_packages/cache"
        # my $packzip = Archive::Zip->new();
        # $packzip->addTree(".");   # after changing into $tempdir
        # $packzip->writeToFileNamed($archive);
        # $makesystemcall = 0;
    }
     elsif ( $archive =~ /dmg$/ )
    {
        my $folder = (( -l "$tempdir/$packagename/Applications" ) or ( -l "$tempdir/$packagename/opt" )) ? $packagename : "\.";

        if ( $allvariables->{'PACK_INSTALLED'} ) {
            $folder = $packagename;
        }

        my $volume_name = $allvariables->{'PRODUCTNAME'};
        my $volume_name_classic = $allvariables->{'PRODUCTNAME'} . ' ' . $allvariables->{'PRODUCTVERSION'};
        my $volume_name_classic_app = $volume_name;  # "app" should not contain version number
        if ( $allvariables->{'DMG_VOLUMEEXTENSION'} ) {
            $volume_name = $volume_name . ' ' . $allvariables->{'DMG_VOLUMEEXTENSION'};
            $volume_name_classic = $volume_name_classic . ' ' . $allvariables->{'DMG_VOLUMEEXTENSION'};
            $volume_name_classic_app = $volume_name_classic_app . ' ' . $allvariables->{'DMG_VOLUMEEXTENSION'};
        }

        my $sla = 'sla.r';
        my $ref = "";

        if ( ! $allvariables->{'HIDELICENSEDIALOG'} )
        {
            installer::scriptitems::get_sourcepath_from_filename_and_includepath( \$sla, $includepatharrayref, 0);
        }

        my $localtempdir = $tempdir;

        if (( $installer::globals::languagepack ) || ( $installer::globals::helppack ))
        {
            $localtempdir = "$tempdir/$packagename";
            if ( $installer::globals::helppack ) { $volume_name = "$volume_name Help Pack"; }
            if ( $installer::globals::languagepack )
            {
                $volume_name = "$volume_name Language Pack";
                $volume_name_classic = "$volume_name_classic Language Pack";
                $volume_name_classic_app = "$volume_name_classic_app Language Pack";
            }

            # Create tar ball named tarball.tar.bz2
            # my $appfolder = $localtempdir . "/" . $volume_name . "\.app";
            my $appfolder = $localtempdir . "/" . $volume_name_classic_app . "\.app";
            my $contentsfolder = $appfolder . "/Contents";
            my $tarballname = "tarball.tar.bz2";

            my $localfrom = cwd();
            chdir $appfolder;

            $systemcall = "tar -cjf $tarballname Contents/";

            print "... $systemcall ...\n";
            my $localreturnvalue = system($systemcall);
            $infoline = "Systemcall: $systemcall\n";
            push( @installer::globals::logfileinfo, $infoline);

            if ($localreturnvalue)
            {
                $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
                push( @installer::globals::logfileinfo, $infoline);
            }
            else
            {
                $infoline = "Success: Executed \"$systemcall\" successfully!\n";
                push( @installer::globals::logfileinfo, $infoline);
            }

            my $sourcefile = $appfolder . "/" . $tarballname;
            my $destfile = $contentsfolder . "/" . $tarballname;

            installer::systemactions::remove_complete_directory($contentsfolder);
            installer::systemactions::create_directory($contentsfolder);

            installer::systemactions::copy_one_file($sourcefile, $destfile);
            unlink($sourcefile);

            # Copy two files into installation set next to the tar ball
            # 1. "osx_install.applescript"
            # 2 "OpenOffice.org Languagepack"

            my $scriptrealfilename = "osx_install.applescript";
            my $scriptfilename = "";
            if ( $installer::globals::languagepack ) { $scriptfilename = "osx_install_languagepack.applescript"; }
            if ( $installer::globals::helppack ) { $scriptfilename = "osx_install_helppack.applescript"; }
            my $scripthelpersolverfilename = "mac_install.script";
            # my $scripthelperrealfilename = $volume_name;
            my $scripthelperrealfilename = $volume_name_classic_app;
            my $translationfilename = $installer::globals::macinstallfilename;

            # Finding both files in solver

            my $scriptref = installer::scriptitems::get_sourcepath_from_filename_and_includepath( \$scriptfilename, $includepatharrayref, 0);
            if ($$scriptref eq "") { installer::exiter::exit_program("ERROR: Could not find Apple script $scriptfilename!", "create_package"); }
            my $scripthelperref = installer::scriptitems::get_sourcepath_from_filename_and_includepath( \$scripthelpersolverfilename, $includepatharrayref, 0);
            if ($$scripthelperref eq "") { installer::exiter::exit_program("ERROR: Could not find Apple script $scripthelpersolverfilename!", "create_package"); }
            my $translationfileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath( \$translationfilename, $includepatharrayref, 0);
            if ($$translationfileref eq "") { installer::exiter::exit_program("ERROR: Could not find Apple script translation file $translationfilename ( includepatharrayref = $includepatharrayref )", "create_package"); }

            $scriptfilename = $contentsfolder . "/" . $scriptrealfilename;
            $scripthelperrealfilename = $contentsfolder . "/" . $scripthelperrealfilename;

            installer::systemactions::copy_one_file($$scriptref, $scriptfilename);
            installer::systemactions::copy_one_file($$scripthelperref, $scripthelperrealfilename);

            # Replacing variables in script $scriptfilename
            # Localizing script $scriptfilename
            my $scriptfilecontent = installer::files::read_file($scriptfilename);
            my $translationfilecontent = installer::files::read_file($$translationfileref);
            localize_scriptfile($scriptfilecontent, $translationfilecontent, $languagestringref);
            # replace_variables_in_scriptfile($scriptfilecontent, $volume_name, $allvariables);
            replace_variables_in_scriptfile($scriptfilecontent, $volume_name_classic, $volume_name_classic_app, $allvariables);
            installer::files::save_file($scriptfilename, $scriptfilecontent);

            chmod 0775, $scriptfilename;
            chmod 0775, $scripthelperrealfilename;

            # Copy also Info.plist and icon file
            # Finding both files in solver
            my $iconfile = "ooo3_installer.icns";
            my $iconfileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath( \$iconfile, $includepatharrayref, 0);
            if ($$iconfileref eq "") { installer::exiter::exit_program("ERROR: Could not find Apple script icon file $iconfile!", "create_package"); }
            my $subdir = $contentsfolder . "/" . "Resources";
            if ( ! -d $subdir ) { installer::systemactions::create_directory($subdir); }
            $destfile = $subdir . "/" . $iconfile;
            installer::systemactions::copy_one_file($$iconfileref, $destfile);

            my $infoplistfile = "Info.plist.langpack";
            my $installname = "Info.plist";
            my $infoplistfileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath( \$infoplistfile, $includepatharrayref, 0);
            if ($$infoplistfileref eq "") { installer::exiter::exit_program("ERROR: Could not find Apple script Info.plist: $infoplistfile!", "create_package"); }
            $destfile = $contentsfolder . "/" . $installname;
            installer::systemactions::copy_one_file($$infoplistfileref, $destfile);

            # Replacing variables in Info.plist
            $scriptfilecontent = installer::files::read_file($destfile);
            # replace_one_variable_in_shellscript($scriptfilecontent, $volume_name, "FULLPRODUCTNAME" );
            replace_one_variable_in_shellscript($scriptfilecontent, $volume_name_classic_app, "FULLAPPPRODUCTNAME" ); # OpenOffice.org Language Pack
            installer::files::save_file($destfile, $scriptfilecontent);

            chdir $localfrom;
        }
        else
        {
            if (($volume_name_classic_app eq 'LibreOffice' || $volume_name_classic_app eq 'LibreOfficeDev') &&
                defined($ENV{'MACOSX_CODESIGNING_IDENTITY'}) && $ENV{'MACOSX_CODESIGNING_IDENTITY'} ne "" )
            {
                $systemcall = "$ENV{'SRCDIR'}/solenv/bin/macosx-codesign-app-bundle $localtempdir/$folder/$volume_name_classic_app.app";
                print "... $systemcall ...\n";
                my $returnvalue = system($systemcall);
                $infoline = "Systemcall: $systemcall\n";
                push( @installer::globals::logfileinfo, $infoline);

                if ($returnvalue)
                {
                    $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
                    push( @installer::globals::logfileinfo, $infoline);
                }
                else
                {
                    $infoline = "Success: Executed \"$systemcall\" successfully!\n";
                    push( @installer::globals::logfileinfo, $infoline);
                }
            }
        }

        $systemcall = "cd $localtempdir && hdiutil makehybrid -hfs -hfs-openfolder $folder $folder -hfs-volume-name \"$volume_name\" -ov -o $installdir/tmp && hdiutil convert -ov -format UDBZ $installdir/tmp.dmg -o $archive && ";
        if (( $ref ne "" ) && ( $$ref ne "" )) {
            $systemcall .= "hdiutil unflatten $archive && Rez -a $$ref -o $archive && hdiutil flatten $archive &&";
        }
        $systemcall .= "rm -f $installdir/tmp.dmg";
    }
    else
    {
        # getting the path of the getuid.so (only required for Solaris and Linux)
        my $getuidlibrary = "";
        my $ldpreloadstring = "";
        if (( $installer::globals::issolarisbuild ) || ( $installer::globals::islinuxbuild ))
        {
            $getuidlibrary = installer::download::get_path_for_library($includepatharrayref);
            if ( $getuidlibrary ne "" ) { $ldpreloadstring = "LD_PRELOAD=" . $getuidlibrary; }
        }

        $systemcall = "cd $tempdir; $ldpreloadstring tar -cf - . | gzip > $archive";
    }

    if ( $makesystemcall )
    {
        print "... $systemcall ...\n";
        my $returnvalue = system($systemcall);
        my $infoline = "Systemcall: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);

        if ($returnvalue)
        {
            $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
        else
        {
            $infoline = "Success: Executed \"$systemcall\" successfully!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }

    if ( $return_to_start ) { chdir($from); }

    print "... removing $tempdir ...\n";
    installer::systemactions::remove_complete_directory($tempdir);
}

####################################################
# Main method for creating the simple package
# installation sets
####################################################

sub create_simple_package
{
    my ( $filesref, $dirsref, $scpactionsref, $linksref, $unixlinksref, $loggingdir, $languagestringref, $shipinstalldir, $allsettingsarrayref, $allvariables, $includepatharrayref ) = @_;

    # Creating directories

    my $current_install_number = "";
    my $infoline = "";

    installer::logger::print_message( "... creating installation directory ...\n" );
    installer::logger::include_header_into_logfile("Creating installation directory");

    $installer::globals::csp_installdir = installer::worker::create_installation_directory($shipinstalldir, $languagestringref, \$current_install_number);
    $installer::globals::csp_installlogdir = installer::systemactions::create_directory_next_to_directory($installer::globals::csp_installdir, "log");

    my $installdir = $installer::globals::csp_installdir;
    my $installlogdir = $installer::globals::csp_installlogdir;

    # Setting package name (similar to the download name)
    my $packagename = "";

    if ( $installer::globals::packageformat eq "archive"  ||
        $installer::globals::packageformat eq "dmg" )
    {
        $installer::globals::csp_languagestring = $$languagestringref;

        my $locallanguage = $installer::globals::csp_languagestring;

        if ( $allvariables->{'OOODOWNLOADNAME'} )
        {
            $packagename = installer::download::set_download_filename(\$locallanguage, $allvariables);
        }
        else
        {
            $downloadname = installer::ziplist::getinfofromziplist($allsettingsarrayref, "downloadname");
            if ( $installer::globals::languagepack ) { $downloadname = installer::ziplist::getinfofromziplist($allsettingsarrayref, "langpackdownloadname"); }
            if ( $installer::globals::helppack ) { $downloadname = installer::ziplist::getinfofromziplist($allsettingsarrayref, "helppackdownloadname"); }
            $packagename = installer::download::resolve_variables_in_downloadname($allvariables, $$downloadname, \$locallanguage);
        }
    }

    # Work around Windows problems with long pathnames (see issue 50885) by
    # putting the to-be-archived installation tree into the temp directory
    # instead of the module output tree (unless LOCALINSTALLDIR dictates
    # otherwise, anyway); can be removed once issue 50885 is fixed:
    my $tempinstalldir = $installdir;
    if ( $installer::globals::iswindowsbuild &&
         $installer::globals::packageformat eq "archive" &&
         !$installer::globals::localinstalldirset )
    {
        $tempinstalldir = File::Temp::tempdir;
    }

    # Creating subfolder in installdir, which shall become the root of package or zip file
    my $subfolderdir = "";
    if ( $packagename ne "" ) { $subfolderdir = $tempinstalldir . $installer::globals::separator . $packagename; }
    else { $subfolderdir = $tempinstalldir; }

    if ( ! -d $subfolderdir ) { installer::systemactions::create_directory($subfolderdir); }

    # Create directories, copy files and ScpActions

    installer::logger::print_message( "... creating directories ...\n" );
    installer::logger::include_header_into_logfile("Creating directories:");

    for ( my $i = 0; $i <= $#{$dirsref}; $i++ )
    {
        my $onedir = ${$dirsref}[$i];

        if ( $onedir->{'HostName'} )
        {
            my $destdir = $subfolderdir . $installer::globals::separator . $onedir->{'HostName'};

            if ( ! -d $destdir )
            {
                if ( $^O =~ /cygwin/i ) # Cygwin performance check
                {
                    $infoline = "Try to create directory $destdir\n";
                    push(@installer::globals::logfileinfo, $infoline);
                    # Directories in $dirsref are sorted and all parents were added -> "mkdir" works without parent creation!
                    if ( ! ( -d $destdir )) { mkdir($destdir, 0775); }
                }
                else
                {
                    installer::systemactions::create_directory_structure($destdir);
                }
            }
        }
    }

    # stripping files ?!
    if (( $installer::globals::strip ) && ( ! $installer::globals::iswindowsbuild )) { strip_libraries($filesref, $languagestringref); }

    # copy Files
    installer::logger::print_message( "... copying files ...\n" );
    installer::logger::include_header_into_logfile("Copying files:");

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];

        if (( $onefile->{'Styles'} ) && ( $onefile->{'Styles'} =~ /\bBINARYTABLE_ONLY\b/ )) { next; }

        my $source = $onefile->{'sourcepath'};
        my $destination = $onefile->{'destination'};
        $destination = $subfolderdir . $installer::globals::separator . $destination;

        # Replacing $$ by $ is necessary to install files with $ in its name (back-masquerading)
        # Otherwise, the following shell command does not work and the file list is not correct
        $source =~ s/\$\$/\$/;
        $destination =~ s/\$\$/\$/;

        if ( $^O =~ /cygwin/i ) # Cygwin performance, do not use copy_one_file. "chmod -R" at the end
        {
            my $copyreturn = copy($source, $destination);

            if ($copyreturn)
            {
                $infoline = "Copy: $source to $destination\n";
                $returnvalue = 1;
            }
            else
            {
                $infoline = "ERROR: Could not copy $source to $destination $!\n";
                $returnvalue = 0;
            }

            push(@installer::globals::logfileinfo, $infoline);
        }
        else
        {
            installer::systemactions::copy_one_file($source, $destination);

            if ( ! $installer::globals::iswindowsbuild )
            {
                # see issue 102274
                if ( $onefile->{'UnixRights'} )
                {
                    chmod oct($onefile->{'UnixRights'}), $destination;
                }
            }
        }
    }

    # creating Links

    installer::logger::print_message( "... creating links ...\n" );
    installer::logger::include_header_into_logfile("Creating links:");

    for ( my $i = 0; $i <= $#{$linksref}; $i++ )
    {
        my $onelink = ${$linksref}[$i];

        my $destination = $onelink->{'destination'};
        $destination = $subfolderdir . $installer::globals::separator . $destination;
        my $destinationfile = $onelink->{'destinationfile'};

        my $localcall = "ln -sf \'$destinationfile\' \'$destination\' \>\/dev\/null 2\>\&1";
        system($localcall);

        $infoline = "Creating link: \"ln -sf $destinationfile $destination\"\n";
        push(@installer::globals::logfileinfo, $infoline);
    }

    for ( my $i = 0; $i <= $#{$unixlinksref}; $i++ )
    {
        my $onelink = ${$unixlinksref}[$i];

        my $target = $onelink->{'Target'};
        my $destination = $subfolderdir . $installer::globals::separator . $onelink->{'destination'};

        my $localcall = "ln -sf \'$target\' \'$destination\' \>\/dev\/null 2\>\&1";
        system($localcall);

        $infoline = "Creating Unix link: \"ln -sf $target $destination\"\n";
        push(@installer::globals::logfileinfo, $infoline);
    }

    # Setting privileges for cygwin globally

    if ( $^O =~ /cygwin/i )
    {
        installer::logger::print_message( "... changing privileges in $subfolderdir ...\n" );
        installer::logger::include_header_into_logfile("Changing privileges in $subfolderdir:");

        my $localcall = "chmod -R 755 " . "\"" . $subfolderdir . "\"";
        system($localcall);
    }

    installer::logger::print_message( "... removing superfluous directories ...\n" );
    installer::logger::include_header_into_logfile("Removing superfluous directories:");

    my $extensionfolder = get_extensions_dir($subfolderdir);
    installer::systemactions::remove_empty_dirs_in_folder($extensionfolder);

    if ( $installer::globals::compiler =~ /^unxmacx/ )
    {
        installer::worker::put_scpactions_into_installset("$installdir/$packagename");
    }

    # Creating archive file
    if ( $installer::globals::packageformat eq "archive" )
    {
        create_package($tempinstalldir, $installdir, $packagename, $allvariables, $includepatharrayref, $languagestringref, $installer::globals::archiveformat);
    }
    elsif ( $installer::globals::packageformat eq "dmg" )
    {
        create_package($installdir, $installdir, $packagename, $allvariables, $includepatharrayref, $languagestringref, ".dmg");
    }

    # Analyzing the log file

    installer::worker::clean_output_tree(); # removing directories created in the output tree
    installer::worker::analyze_and_save_logfile($loggingdir, $installdir, $installlogdir, $allsettingsarrayref, $languagestringref, $current_install_number);
}

1;
