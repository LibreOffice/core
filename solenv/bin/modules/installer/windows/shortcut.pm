#*************************************************************************
#
#   $RCSfile: shortcut.pm,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kz $ $Date: 2004-06-11 18:20:35 $
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

package installer::windows::shortcut;

use installer::existence;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::windows::idtglobal;

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

    if ($directory eq "") { $directory = $installer::globals::officefolder; }       # Shortcuts in the root directory

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

    return "";
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

###################################################################
# Returning the directory for a folderitem for shortcut table.
###################################################################

sub get_folderitem_directory
{
    my ($shortcut) = @_;

    my $directory = "$installer::globals::programmenufolder";    # default

    # The value $installer::globals::programmenufolder is not correct for the
    # PREDEFINED folders, like PREDEFINED_AUTOSTART

    if ( $shortcut->{'FolderID'} eq "PREDEFINED_AUTOSTART" )
    {
        $directory = $installer::globals::startupfolder;
    }

    # saving the directory in the folderitems collector

    $shortcut->{'directory'} = $directory;

    return $directory;
}

########################################################################
# Returning the target (feature) for a folderitem for shortcut table.
########################################################################

sub get_folderitem_target
{
    my ($shortcut, $filesref) = @_;

    my $onefile;
    my $target = "";
    my $found = 0;
    my $shortcut_fileid = $shortcut->{'FileID'};

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
    my ($filesref, $linksref, $folderref, $folderitemsref, $dirref, $basedir, $languagesarrayref, $iconfilecollector) = @_;

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
                $shortcut{'WkDir'} = get_shortcut_wkdir($onelink);

                my $oneline = $shortcut{'Shortcut'} . "\t" . $shortcut{'Directory_'} . "\t" . $shortcut{'Name'} . "\t"
                            . $shortcut{'Component_'} . "\t" . $shortcut{'Target'} . "\t" . $shortcut{'Arguments'} . "\t"
                            . $shortcut{'Description'} . "\t" . $shortcut{'Hotkey'} . "\t" . $shortcut{'Icon_'} . "\t"
                            . $shortcut{'IconIndex'} . "\t" . $shortcut{'ShowCmd'} . "\t" . $shortcut{'WkDir'} . "\n";

                push(@shortcuttable, $oneline);
            }
        }

        # Saving the file

        my $shortcuttablename = $basedir . $installer::globals::separator . "Shortcut.idt" . "." . $onelanguage;
        installer::files::save_file($shortcuttablename ,\@shortcuttable);
        my $infoline = "Created idt file: $shortcuttablename\n";
        push(@installer::globals::logfileinfo, $infoline);
    }
}


1;