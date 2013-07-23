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

package installer::windows::msishortcutproperty;

use installer::exiter;
use installer::files;
use installer::globals;
use installer::windows::idtglobal;

##############################################################
# Returning identifier for msishortcutproperty table.
##############################################################

sub get_msishortcutproperty_identifier
{
    my ($msishortcutproperty) = @_;

    my $identifier = $msishortcutproperty->{'gid'};

    return $identifier;
}

##############################################################
# Returning shortcut for msishortcutproperty table.
##############################################################

sub get_msishorcutproperty_shortcut
{
    my ($msishortcutproperty, $filesref) = @_;

    my $onefile;
    my $shortcut = "";
    my $found = 0;
    my $msishortcutproperty_shortcutid = $msishortcutproperty->{'ShortcutID'};

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        $onefile = ${$filesref}[$i];
        my $filegid = $onefile->{'gid'};

        if ( $filegid eq $msishortcutproperty_shortcutid )
        {
            $found = 1;
            last;
        }
    }

    if (!($found))
    {
        installer::exiter::exit_program("ERROR: Did not find ShortcutID $msishortcutproperty_shortcutid in file collection for shortcut", "get_msishorcutproperty_shortcut");
    }

    $shortcut = $onefile->{'gid'};

    return $shortcut;
}

##############################################################
# Returning the propertykey for msishortcutproperty table.
##############################################################

sub get_msishortcutproperty_propertykey
{
    my ($msishortcutproperty, $onelanguage) = @_;

    my $propertykey = "";
    if ( $msishortcutproperty->{'PropertyKey'} ) { $propertykey = $msishortcutproperty->{'PropertyKey'}; }

    return $propertykey;
}

################################################################
# Returning the propvariantvalue for msishortcutproperty table.
################################################################

sub get_msishortcutproperty_propvariantvalue
{
    my ($msishortcutproperty, $onelanguage) = @_;

    my $propvariantvalue = "";
    if ( $msishortcutproperty->{'PropVariantValue'} ) { $propvariantvalue = $msishortcutproperty->{'PropVariantValue'}; }

    return $propvariantvalue;
}

###################################################################
# Creating the file MsiShortcutProperty.idt dynamically
# Content:
# MsiShortcutProperty Shortcut_ PropertyKey PropVariantValue
###################################################################

sub create_msishortcutproperty_table
{
    my ($folderitempropertiesref, $folderitemsref, $basedir) = @_;

    for ( my $m = 0; $m <= $#{$languagesarrayref}; $m++ )
    {
        my @msishortcutpropertytable = ();

        installer::windows::idtglobal::write_idt_header(\@msishortcutpropertytable, "msishortcutproperty");

        # The entries defined in scp as FolderItemProperties
        # These shortcuts will fill the icons table.

        for ( my $j = 0; $j <= $#{$folderitempropertiesref}; $j++ )
        {
            my $onelink = ${$folderitempropertiesref}[$j];

            my %msishortcutproperty = ();

            $msishortcutproperty{'MsiShortcutProperty'} = get_msishortcutproperty_identifier($onelink);
            $msishortcutproperty{'Shortcut_'} = get_msishorcutproperty_shortcut($onelink, $folderitemsref);
            $msishortcutproperty{'PropertyKey'} = get_msishortcutproperty_propertykey($onelink);
            $msishortcutproperty{'PropVariantValue'} = get_msishortcutproperty_propvariantvalue($onelink);

            my $oneline = $msishortcutproperty{'MsiShortcutProperty'} . "\t" . $msishortcutproperty{'Shortcut_'} . "\t"
                        . $msishortcutproperty{'PropertyKey'} . "\t" . $msishortcutproperty{'PropVariantValue'} . "\n";

            push(@msishortcutpropertytable, $oneline);
        }

        # Saving the file

        my $msishortcutpropertytablename = $basedir . $installer::globals::separator . "MsiShortcutProperty.idt" . "." . $onelanguage;
        installer::files::save_file($msishortcutpropertytablename ,\@msishortcutpropertytable);
        my $infoline = "Created idt file: $msishortcutpropertytablename\n";
        push(@installer::globals::logfileinfo, $infoline);
    }
}
