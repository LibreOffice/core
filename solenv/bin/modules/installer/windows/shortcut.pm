#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: shortcut.pm,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:05:07 $
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

package installer::windows::shortcut;

use installer::existence;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::windows::idtglobal;

##############################################################
# Returning the file object for the msiassembly table.
##############################################################

sub get_file_by_name
{
    my ( $filesref, $filename ) = @_;

    my $foundfile = 0;
    my $onefile;

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        $onefile = ${$filesref}[$i];
        my $name = $onefile->{'Name'};

        if ( $name eq $filename )
        {
            $foundfile = 1;
            last;
        }
    }

    if (! $foundfile ) { $onefile  = ""; }

    return $onefile;
}

##############################################################
# Returning identifier for shortcut table.
##############################################################

sub get_shortcut_identifier
{
    my ($shortcut) = @_;

    my $identifier = $shortcut->{'gid'};

    return $identifier;
}

##############################################################
# Returning directory for shortcut table.
##############################################################

sub get_shortcut_directory
{
    my ($shortcut, $dirref) = @_;

    # For shortcuts it is easy to convert the gid_Dir_Abc into the unique name in
    # the directory table, for instance help_en_simpressidx.
    # For files (components) this is not so easy, because files can be included
    # in zip files with subdirectories that are not defined in scp.

    my $onedir;
    my $shortcutdirectory = $shortcut->{'Dir'};
    my $directory = "";
    my $found = 0;

    for ( my $i = 0; $i <= $#{$dirref}; $i++ )
    {
        $onedir = ${$dirref}[$i];
        my $directorygid = $onedir->{'Dir'};

        if ( $directorygid eq $shortcutdirectory )
        {
            $found = 1;
            last;
        }
    }

    if (!($found))
    {
        installer::exiter::exit_program("ERROR: Did not find DirectoryID $shortcutdirectory in directory collection for shortcut", "get_shortcut_directory");
    }

    $directory = $onedir->{'uniquename'};

    if ($directory eq "") { $directory = "OFFICEINSTALLLOCATION"; }     # Shortcuts in the root directory

    return $directory;
}

##############################################################
# Returning name for shortcut table.
##############################################################

sub get_shortcut_name
{
    my ($shortcut, $shortnamesref, $onelanguage) = @_;

    my $returnstring;

    my $name = $shortcut->{'Name'};

    my $shortstring = installer::windows::idtglobal::make_eight_three_conform($name, "shortcut", $shortnamesref);
    $shortstring =~ s/\s/\_/g;  # replacing white spaces with underline

    if ( $shortstring eq $name ) { $returnstring = $name; } # nothing changed
    else {$returnstring = $shortstring . "\|" . $name; }

    return $returnstring;
}

##############################################################
# Returning component for shortcut table.
##############################################################

sub get_shortcut_component
{
    my ($shortcut, $filesref) = @_;

    my $onefile;
    my $component = "";
    my $found = 0;
    my $shortcut_fileid = $shortcut->{'FileID'};

    my $absolute_filename = 0;
    if ( $shortcut->{'Styles'} ) { $styles = $shortcut->{'Styles'}; }
    if ( $styles =~ /\bABSOLUTE_FILENAME\b/ ) { $absolute_filename = 1; }   # FileID contains an absolute filename
    if ( $styles =~ /\bUSE_HELPER_FILENAME\b/ ) { $absolute_filename = 1; } # ComponentIDFile contains id of a helper file

    # if the FileID contains an absolute filename, therefore the entry for "ComponentIDFile" has to be used.
    if ( $absolute_filename ) { $shortcut_fileid = $shortcut->{'ComponentIDFile'}; }

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        $onefile = ${$filesref}[$i];
        my $filegid = $onefile->{'gid'};

        if ( $filegid eq $shortcut_fileid )
        {
            $found = 1;
            last;
        }
    }

    if (!($found))
    {
        installer::exiter::exit_program("ERROR: Did not find FileID $shortcut_fileid in file collection for shortcut", "get_shortcut_component");
    }

    $component = $onefile->{'componentname'};

    # finally saving the componentname in the folderitem collector

    $shortcut->{'component'} = $component;

    return $component;
}

##############################################################
# Returning target for shortcut table.
##############################################################

sub get_shortcut_target
{
    my ($shortcut, $filesref) = @_;

    my $target = "";
    my $found = 0;
    my $shortcut_fileid = $shortcut->{'FileID'};
    my $onefile;

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        $onefile = ${$filesref}[$i];
        my $filegid = $onefile->{'gid'};

        if ( $filegid eq $shortcut_fileid )
        {
            $found = 1;
            last;
        }
    }

    if (!($found))
    {
        installer::exiter::exit_program("ERROR: Did not find FileID $shortcut_fileid in file collection for shortcut", "get_shortcut_target");
    }

    if ( $onefile->{'Name'} )
    {
        $target = $onefile->{'Name'};
    }

    $target = "\[\#" . $target . "\]";  # format for Non-Advertised shortcuts

    return $target;
}

##############################################################
# Returning arguments for shortcut table.
##############################################################

sub get_shortcut_arguments
{
    my ($shortcut) = @_;

    return "";
}

##############################################################
# Returning the localized description for shortcut table.
##############################################################

sub get_shortcut_description
{
    my ($shortcut, $onelanguage) = @_;

    my $description = "";
    if ( $shortcut->{'Tooltip'} ) { $description = $shortcut->{'Tooltip'}; }

    return $description;
}

##############################################################
# Returning hotkey for shortcut table.
##############################################################

sub get_shortcut_hotkey
{
    my ($shortcut) = @_;

    return "";
}

##############################################################
# Returning icon for shortcut table.
##############################################################

sub get_shortcut_icon
{
    my ($shortcut) = @_;

    return "";
}

##############################################################
# Returning iconindex for shortcut table.
##############################################################

sub get_shortcut_iconindex
{
    my ($shortcut) = @_;

    return "";
}

##############################################################
# Returning show command for shortcut table.
##############################################################

sub get_shortcut_showcmd
{
    my ($shortcut) = @_;

    return "";
}

##############################################################
# Returning working directory for shortcut table.
##############################################################

sub get_shortcut_wkdir
{
    my ($shortcut) = @_;

    return "";
}

####################################################################
# Returning working directory for shortcut table for FolderItems.
####################################################################

sub get_folderitem_wkdir
{
    my ($onelink, $dirref) = @_;

    # For shortcuts it is easy to convert the gid_Dir_Abc into the unique name in
    # the directory table, for instance help_en_simpressidx.

    my $onedir;
    my $workingdirectory = "";
    if ( $onelink->{'WkDir'} ) { $workingdirectory = $onelink->{'WkDir'}; }
    my $directory = "";

    if ( $workingdirectory )
    {
        my $found = 0;

        for ( my $i = 0; $i <= $#{$dirref}; $i++ )
        {
            $onedir = ${$dirref}[$i];
            my $directorygid = $onedir->{'Dir'};

            if ( $directorygid eq $workingdirectory )
            {
                $found = 1;
                last;
            }
        }

        if (!($found))
        {
            installer::exiter::exit_program("ERROR: Did not find DirectoryID $workingdirectory in directory collection for FolderItem", "get_folderitem_wkdir");
        }

        $directory = $onedir->{'uniquename'};

        if ($directory eq "") { $directory = "OFFICEINSTALLLOCATION"; }
    }

    return $directory;
}

###################################################################
# Returning the directory for a folderitem for shortcut table.
###################################################################

sub get_folderitem_directory
{
    my ($shortcut) = @_;

    # my $directory = "$installer::globals::programmenufolder";  # default
    my $directory = "$installer::globals::officemenufolder";     # default

    # The value $installer::globals::programmenufolder is not correct for the
    # PREDEFINED folders, like PREDEFINED_AUTOSTART

    if ( $shortcut->{'FolderID'} eq "PREDEFINED_AUTOSTART" )
    {
        $directory = $installer::globals::startupfolder;
    }

    if ( $shortcut->{'FolderID'} eq "PREDEFINED_DESKTOP" )
    {
        $directory = $installer::globals::desktopfolder;
        $installer::globals::desktoplinkexists = 1;
    }

    if ( $shortcut->{'FolderID'} eq "PREDEFINED_STARTMENU" )
    {
        $directory = $installer::globals::startmenufolder;
    }

    # saving the directory in the folderitems collector

    $shortcut->{'directory'} = $directory;

    return $directory;
}

########################################################################
# Returning the target (feature) for a folderitem for shortcut table.
# For non-advertised shortcuts this is a formatted string.
########################################################################

sub get_folderitem_target
{
    my ($shortcut, $filesref) = @_;

    my $onefile;
    my $target = "";
    my $found = 0;
    my $shortcut_fileid = $shortcut->{'FileID'};

    my $styles = "";
    my $nonadvertised = 0;
    my $absolute_filename = 0;
    if ( $shortcut->{'Styles'} ) { $styles = $shortcut->{'Styles'}; }
    if ( $styles =~ /\bNON_ADVERTISED\b/ ) { $nonadvertised = 1; }  # this is a non-advertised shortcut
    if ( $styles =~ /\bABSOLUTE_FILENAME\b/ ) { $absolute_filename = 1; }   # FileID contains an absolute filename

    # if the FileID contains an absolute filename this can simply be returned as target for the shortcut table.
    if ( $absolute_filename )
    {
        $shortcut->{'target'} = $shortcut_fileid;
        return $shortcut_fileid;
    }

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        $onefile = ${$filesref}[$i];
        my $filegid = $onefile->{'gid'};

        if ( $filegid eq $shortcut_fileid )
        {
            $found = 1;
            last;
        }
    }

    if (!($found))
    {
        installer::exiter::exit_program("ERROR: Did not find FileID $shortcut_fileid in file collection for folderitem", "get_folderitem_target");
    }

    # Non advertised shortcuts do not return the feature, but the path to the file
    if ( $nonadvertised )
    {
        $target = "\[" . $onefile->{'uniquedirname'} . "\]" . "\\" . $onefile->{'Name'};
        $shortcut->{'target'} = $target;
        return $target;
    }

    # the rest only for advertised shortcuts, which contain the feature in the shortcut table.

    if ( $onefile->{'modules'} ) { $target = $onefile->{'modules'}; }

    # If modules contains a list of modules, only taking the first one.
    # But this should never be needed

    if ( $target =~ /^\s*(.*?)\,/ ) { $target = $1; }

    # Attention: Maximum feature length is 38!
    installer::windows::idtglobal::shorten_feature_gid(\$target);

    # and finally saving the target in the folderitems collector

    $shortcut->{'target'} = $target;

    return $target;
}

########################################################################
# Returning the arguments for a folderitem for shortcut table.
########################################################################

sub get_folderitem_arguments
{
    my ($shortcut) = @_;

    my $parameter = "";

    if ( $shortcut->{'Parameter'} ) { $parameter = $shortcut->{'Parameter'}; }

    return $parameter;
}

########################################################################
# Returning the icon for a folderitem for shortcut table.
# The returned value has to be defined in the icon table.
########################################################################

sub get_folderitem_icon
{
    my ($shortcut, $filesref, $iconfilecollector) = @_;

    my $styles = "";
    if ( $shortcut->{'Styles'} ) { $styles = $shortcut->{'Styles'}; }
    if ( $styles =~ /\bNON_ADVERTISED\b/ ) { return ""; }   # no icon for non-advertised shortcuts

    my $iconfilegid = "";

    if ( $shortcut->{'IconFile'} ) { $iconfilegid = $shortcut->{'IconFile'}; }
    else { $iconfilegid = $shortcut->{'FileID'}; }

    my $onefile;
    my $found = 0;

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        $onefile = ${$filesref}[$i];
        my $filegid = $onefile->{'gid'};

        if ( $filegid eq $iconfilegid )
        {
            $found = 1;
            last;
        }
    }

    if (!($found))
    {
        installer::exiter::exit_program("ERROR: Did not find FileID $iconfilegid in file collection", "get_folderitem_icon");
    }

    $iconfile = $onefile->{'Name'};

    # collecting all icon files to copy them into the icon directory

    my $sourcepath = $onefile->{'sourcepath'};

    if (! installer::existence::exists_in_array($sourcepath, $iconfilecollector))
    {
        push(@{$iconfilecollector}, $sourcepath);
    }

    return $iconfile;
}

########################################################################
# Returning the iconindex for a folderitem for shortcut table.
########################################################################

sub get_folderitem_iconindex
{
    my ($shortcut) = @_;

    my $styles = "";
    if ( $shortcut->{'Styles'} ) { $styles = $shortcut->{'Styles'}; }
    if ( $styles =~ /\bNON_ADVERTISED\b/ ) { return ""; }   # no iconindex for non-advertised shortcuts

    my $iconid = 0;

    if ( $shortcut->{'IconID'} ) { $iconid = $shortcut->{'IconID'}; }

    return $iconid;
}

########################################################################
# Returning the show command for a folderitem for shortcut table.
########################################################################

sub get_folderitem_showcmd
{
    my ($shortcut) = @_;

    return "1";
}

###########################################################################################################
# Creating the file Shortcut.idt dynamically
# Content:
# Shortcut Directory_ Name Component_ Target Arguments Description Hotkey Icon_ IconIndex ShowCmd WkDir
###########################################################################################################

sub create_shortcut_table
{
    my ($filesref, $linksref, $folderref, $folderitemsref, $dirref, $basedir, $languagesarrayref, $includepatharrayref, $iconfilecollector) = @_;

    for ( my $m = 0; $m <= $#{$languagesarrayref}; $m++ )
    {
        my $onelanguage = ${$languagesarrayref}[$m];

        my @shortcuttable = ();

        my @shortnames = ();    # to collect all short names

        installer::windows::idtglobal::write_idt_header(\@shortcuttable, "shortcut");

        # First the links, defined in scp as ShortCut

        for ( my $i = 0; $i <= $#{$linksref}; $i++ )
        {
            my $onelink = ${$linksref}[$i];

            # Controlling the language!
            # Only language independent folderitems or folderitems with the correct language
            # will be included into the table

            if (! (!(( $onelink->{'ismultilingual'} )) || ( $onelink->{'specificlanguage'} eq $onelanguage )) )  { next; }

            my %shortcut = ();

            $shortcut{'Shortcut'} = get_shortcut_identifier($onelink);
            $shortcut{'Directory_'} = get_shortcut_directory($onelink, $dirref);
            $shortcut{'Name'} = get_shortcut_name($onelink, \@shortnames, $onelanguage);    # localized name
            $shortcut{'Component_'} = get_shortcut_component($onelink, $filesref);
            $shortcut{'Target'} = get_shortcut_target($onelink, $filesref);
            $shortcut{'Arguments'} = get_shortcut_arguments($onelink);
            $shortcut{'Description'} = get_shortcut_description($onelink, $onelanguage);    # localized description
            $shortcut{'Hotkey'} = get_shortcut_hotkey($onelink);
            $shortcut{'Icon_'} = get_shortcut_icon($onelink);
            $shortcut{'IconIndex'} = get_shortcut_iconindex($onelink);
            $shortcut{'ShowCmd'} = get_shortcut_showcmd($onelink);
            $shortcut{'WkDir'} = get_shortcut_wkdir($onelink);

            my $oneline = $shortcut{'Shortcut'} . "\t" . $shortcut{'Directory_'} . "\t" . $shortcut{'Name'} . "\t"
                        . $shortcut{'Component_'} . "\t" . $shortcut{'Target'} . "\t" . $shortcut{'Arguments'} . "\t"
                        . $shortcut{'Description'} . "\t" . $shortcut{'Hotkey'} . "\t" . $shortcut{'Icon_'} . "\t"
                        . $shortcut{'IconIndex'} . "\t" . $shortcut{'ShowCmd'} . "\t" . $shortcut{'WkDir'} . "\n";

            push(@shortcuttable, $oneline);
        }

        # Second the entries into the start menu, defined in scp as Folder and Folderitem
        # These shortcuts will fill the icons table.

        for ( my $i = 0; $i <= $#{$folderref}; $i++ )
        {
            my $foldergid = ${$folderref}[$i]->{'gid'};

            # iterating over all folderitems for this folder

            for ( my $j = 0; $j <= $#{$folderitemsref}; $j++ )
            {
                my $onelink = ${$folderitemsref}[$j];

                # Controlling the language!
                # Only language independent folderitems or folderitems with the correct language
                # will be included into the table

                if (! (!(( $onelink->{'ismultilingual'} )) || ( $onelink->{'specificlanguage'} eq $onelanguage )) )  { next; }

                # controlling the folder

                my $localused = 0;

                if ( $onelink->{'used'} ) { $localused = $onelink->{'used'}; }

                if (!($localused == 1)) { $onelink->{'used'} = "0"; }       # no resetting

                if (!( $onelink->{'FolderID'} eq $foldergid )) { next; }

                $onelink->{'used'} = "1";

                my %shortcut = ();

                $shortcut{'Shortcut'} = get_shortcut_identifier($onelink);
                $shortcut{'Directory_'} = get_folderitem_directory($onelink);
                $shortcut{'Name'} = get_shortcut_name($onelink, \@shortnames, $onelanguage);    # localized name
                $shortcut{'Component_'} = get_shortcut_component($onelink, $filesref);
                $shortcut{'Target'} = get_folderitem_target($onelink, $filesref);
                $shortcut{'Arguments'} = get_folderitem_arguments($onelink);
                $shortcut{'Description'} = get_shortcut_description($onelink, $onelanguage);    # localized description
                $shortcut{'Hotkey'} = get_shortcut_hotkey($onelink);
                $shortcut{'Icon_'} = get_folderitem_icon($onelink, $filesref, $iconfilecollector);
                $shortcut{'IconIndex'} = get_folderitem_iconindex($onelink);
                $shortcut{'ShowCmd'} = get_folderitem_showcmd($onelink);
                $shortcut{'WkDir'} = get_folderitem_wkdir($onelink, $dirref);

                my $oneline = $shortcut{'Shortcut'} . "\t" . $shortcut{'Directory_'} . "\t" . $shortcut{'Name'} . "\t"
                            . $shortcut{'Component_'} . "\t" . $shortcut{'Target'} . "\t" . $shortcut{'Arguments'} . "\t"
                            . $shortcut{'Description'} . "\t" . $shortcut{'Hotkey'} . "\t" . $shortcut{'Icon_'} . "\t"
                            . $shortcut{'IconIndex'} . "\t" . $shortcut{'ShowCmd'} . "\t" . $shortcut{'WkDir'} . "\n";

                push(@shortcuttable, $oneline);
            }
        }

        # if it is part of the product, the soffice.exe has to be included into the icon table
        # as icon for the ARP applet

        my $sofficefile = "soffice.exe";
        my $onefile = get_file_by_name($filesref, $sofficefile);

        if ( $onefile ne "" )
        {
            my $sourcepath = $onefile->{'sourcepath'};
            if (! installer::existence::exists_in_array($sourcepath, $iconfilecollector))
            {
                unshift(@{$iconfilecollector}, $sourcepath);
                $installer::globals::sofficeiconadded = 1;
            }
        }

        # For language packs and patches the soffice.exe has to be included, even if it is not part of the product.
        # Also as part of the ARP applet (no substitution needed for ProductName, because the file is not installed!)

        if (( $onefile eq "" ) && (( $installer::globals::languagepack ) || ( $installer::globals::patch )))
        {
            my $sourcepathref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$sofficefile, $includepatharrayref, 1);
            if ($$sourcepathref eq "") { installer::exiter::exit_program("ERROR: Could not find $sofficefile as icon in language pack!", "create_shortcut_table"); }

            if (! installer::existence::exists_in_array($$sourcepathref, $iconfilecollector))
            {
                unshift(@{$iconfilecollector}, $$sourcepathref);
                $installer::globals::sofficeiconadded = 1;
            }

            my $localinfoline = "Added icon file $$sourcepathref for language pack into icon file collector.\n";
            push(@installer::globals::logfileinfo, $localinfoline);
        }

        # Saving the file

        my $shortcuttablename = $basedir . $installer::globals::separator . "Shortcut.idt" . "." . $onelanguage;
        installer::files::save_file($shortcuttablename ,\@shortcuttable);
        my $infoline = "Created idt file: $shortcuttablename\n";
        push(@installer::globals::logfileinfo, $infoline);
    }
}


1;