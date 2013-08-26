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

package installer::windows::feature;

use installer::exiter;
use installer::files;
use installer::globals;
use installer::worker;
use installer::windows::idtglobal;
use installer::windows::language;

##############################################################
# Returning the gid for a feature.
# Attention: Maximum length
##############################################################

sub get_feature_gid
{
    my ($onefeature) = @_;

    my $gid = "";

    if ( $onefeature->{'gid'} ) { $gid = $onefeature->{'gid'}; }

    # Attention: Maximum feature length is 38!
    installer::windows::idtglobal::shorten_feature_gid(\$gid);

    return $gid
}

##############################################################
# Returning the gid of the parent.
# Attention: Maximum length
##############################################################

sub get_feature_parent
{
    my ($onefeature) = @_;

    my $parentgid = "";

    if ( $onefeature->{'ParentID'} ) { $parentgid = $onefeature->{'ParentID'}; }

    # The modules, hanging directly below the root, have to be root modules.
    # Only then it is possible to make the "real" root module invisible by
    # setting the display to "0".

    if ( $parentgid eq $installer::globals::rootmodulegid ) { $parentgid = ""; }

    # Attention: Maximum feature length is 38!
    installer::windows::idtglobal::shorten_feature_gid(\$parentgid);

    return $parentgid
}

##############################################################
# Returning the display for a feature.
# 0: Feature is not shown
# odd: subfeatures are shown
# even:  subfeatures are not shown
##############################################################

sub get_feature_display
{
    my ($onefeature) = @_;

    my $display;
    my $parentid = "";

    if ( $onefeature->{'ParentID'} ) { $parentid = $onefeature->{'ParentID'}; }

    if ( $parentid eq "" )
    {
        $display = "0";                                 # root module is not visible
    }
    elsif ( $onefeature->{'gid'} eq "gid_Module_Prg")   # program module shows subfeatures
    {
        $display = "1";                                 # root module shows subfeatures
    }
    else
    {
        $display = "2";                                 # all other modules do not show subfeatures
    }

    # special case: Feature has flag "HIDDEN_ROOT" -> $display is 0
    my $styles = "";
    if ( $onefeature->{'Styles'} ) { $styles = $onefeature->{'Styles'}; }
    if ( $styles =~ /\bHIDDEN_ROOT\b/ ) { $display = "0"; }

    # Special handling for language modules. Only visible in multilingual installation set
    if (( $styles =~ /\bSHOW_MULTILINGUAL_ONLY\b/ ) && ( ! $installer::globals::ismultilingual )) { $display = "0"; }

    # Special handling for c05office. No program module visible.
    if (( $onefeature->{'gid'} eq "gid_Module_Prg" ) && ( $installer::globals::product =~ /c05office/i )) { $display = "0"; }

    # making all feature invisible in Language packs and in Help packs!
    if ( $installer::globals::languagepack || $installer::globals::helppack ) { $display = "0"; }

    return $display
}

##############################################################
# Returning the level for a feature.
##############################################################

sub get_feature_level
{
    my ($onefeature) = @_;

    my $level = "20";   # the default

    my $localdefault = "";

    if ( $onefeature->{'Default'} ) { $localdefault = $onefeature->{'Default'}; }

    if ( $localdefault eq "NO" )    # explicitly set Default = "NO"
    {
        $level = "200";             # deselected in default installation, base is 100
    }

    # special handling for Java and Ada
    if ( $onefeature->{'Name'} )
    {
        if ( $onefeature->{'Name'} =~ /java/i ) { $level = $level + 40; }
    }

    # if FeatureLevel is defined in scp, this will be used

    if ( $onefeature->{'FeatureLevel'} ) { $level = $onefeature->{'FeatureLevel'}; }

    return $level
}

##############################################################
# Returning the directory for a feature.
##############################################################

sub get_feature_directory
{
    my ($onefeature) = @_;

    my $directory;

    $directory = "INSTALLLOCATION";

    return $directory
}

##############################################################
# Returning the directory for a feature.
##############################################################

sub get_feature_attributes
{
    my ($onefeature) = @_;

    my $attributes;

    # No advertising of features and no leaving on network.
    # Feature without parent must not have the "2"

    my $parentgid = "";
    if ( $onefeature->{'ParentID'} ) { $parentgid = $onefeature->{'ParentID'}; }

    if (( $parentgid eq "" ) || ( $parentgid eq $installer::globals::rootmodulegid )) { $attributes = "8"; }
    else { $attributes = "10"; }

    return $attributes
}

#################################################################################
# Replacing one variable in one files
#################################################################################

sub replace_one_variable
{
    my ($translationfile, $variable, $searchstring) = @_;

    for ( my $i = 0; $i <= $#{$translationfile}; $i++ )
    {
        ${$translationfile}[$i] =~ s/\%$searchstring/$variable/g;
    }
}

#################################################################################
# Replacing the variables in the feature names and descriptions
#################################################################################

sub replace_variables
{
    my ($translationfile, $variableshashref) = @_;

    # we want to substitute FOO_BR before FOO to avoid floating _BR suffixes
    foreach $key (sort { length ($b) <=> length ($a) } keys %{$variableshashref})
    {
        my $value = $variableshashref->{$key};
        replace_one_variable($translationfile, $value, $key);
    }
}

#################################################################################
# Collecting the feature recursively.
#################################################################################

sub collect_modules_recursive
{
    my ($modulesref, $parentid, $feature, $directaccess, $directgid, $directparent, $directsortkey, $sorted) = @_;

    my @allchildren = ();
    my $childrenexist = 0;

    # Collecting children from Module $parentid

    my $modulegid;
    foreach $modulegid ( keys %{$directparent})
    {
        if ( $directparent->{$modulegid} eq $parentid )
        {
            push @allchildren, [ $directsortkey->{$modulegid}, $modulegid ];
            $childrenexist = 1;
        }
    }

    # Sorting children

    if ( $childrenexist )
    {
        # Sort children
        @allchildren = map { $_->[1] }
                       sort { $a->[0] <=> $b->[0] }
                       @allchildren;

        # Adding children to new array
        foreach my $gid ( @allchildren )
        {
            # Saving all lines, that have this 'gid'

            my $unique;
            foreach $unique ( keys %{$directgid} )
            {
                if ( $directgid->{$unique} eq $gid )
                {
                    push(@{$feature}, ${$modulesref}[$directaccess->{$unique}]);
                    if ( $sorted->{$unique} == 1 ) { installer::exiter::exit_program("ERROR: Sorting feature failed! \"$unique\" already sorted.", "sort_feature"); }
                    $sorted->{$unique} = 1;
                }
            }

            collect_modules_recursive($modulesref, $gid, $feature, $directaccess, $directgid, $directparent, $directsortkey, $sorted);
        }
    }
}

#################################################################################
# Sorting the feature in specified order. Evaluated is the key "Sortkey", that
# is set in scp2 projects.
# The display order of modules in Windows Installer is dependent from the order
# in the idt file. Therefore the order of the modules array has to be adapted
# to the Sortkey order, before the idt file is created.
#################################################################################

sub sort_feature
{
    my ($modulesref) = @_;

    my @feature = ();

    my %directaccess = ();
    my %directparent = ();
    my %directgid = ();
    my %directsortkey = ();
    my %sorted = ();

    for ( my $i = 0; $i <= $#{$modulesref}; $i++ )
    {
        my $onefeature = ${$modulesref}[$i];

        my $uniquekey = $onefeature->{'uniquekey'};
        my $modulegid = $onefeature->{'gid'};

        $directaccess{$uniquekey} = $i;

        $directgid{$uniquekey} = $onefeature->{'gid'};

        # ParentID and Sortkey are not saved for the 'uniquekey', but only for the 'gid'

        if ( $onefeature->{'ParentID'} ) { $directparent{$modulegid} = $onefeature->{'ParentID'}; }
        else { $directparent{$modulegid} = ""; }

        if ( $onefeature->{'Sortkey'} ) { $directsortkey{$modulegid} = $onefeature->{'Sortkey'}; }
        else { $directsortkey{$modulegid} = "9999"; }

        # Bookkeeping:
        $sorted{$uniquekey} = 0;
    }

    # Searching all feature recursively, beginning with ParentID = ""
    my $parentid = "";
    collect_modules_recursive($modulesref, $parentid, \@feature, \%directaccess, \%directgid, \%directparent, \%directsortkey, \%sorted);

    # Bookkeeping
    my $modulekey;
    foreach $modulekey ( keys %sorted )
    {
        if ( $sorted{$modulekey} == 0 )
        {
            my $infoline = "Warning: Module \"$modulekey\" could not be sorted. Added to the end of the module array.\n";
            push(@installer::globals::logfileinfo, $infoline);
            push(@feature, ${$modulesref}[$directaccess{$modulekey}]);
        }
    }

    return \@feature;
}

#################################################################################
# Adding a unique key to the modules array. The gid is not unique for
# multilingual modules. Only the combination from gid and specific language
# is unique. Uniqueness is required for sorting mechanism.
#################################################################################

sub add_uniquekey
{
    my ( $modulesref ) = @_;

    for ( my $i = 0; $i <= $#{$modulesref}; $i++ )
    {
        my $uniquekey = ${$modulesref}[$i]->{'gid'};
        if ( ${$modulesref}[$i]->{'specificlanguage'} ) { $uniquekey = $uniquekey . "_" . ${$modulesref}[$i]->{'specificlanguage'}; }
        ${$modulesref}[$i]->{'uniquekey'} = $uniquekey;
    }
}

#################################################################################
# Creating the file Feature.idt dynamically
# Content:
# Feature Feature_Parent Title Description Display Level Directory_ Attributes
#################################################################################

sub create_feature_table
{
    my ($modulesref, $basedir, $languagesarrayref, $allvariableshashref) = @_;

    for ( my $m = 0; $m <= $#{$languagesarrayref}; $m++ )
    {
        my $onelanguage = ${$languagesarrayref}[$m];

        my $infoline;

        my @featuretable = ();

        installer::windows::idtglobal::write_idt_header(\@featuretable, "feature");

        for ( my $i = 0; $i <= $#{$modulesref}; $i++ )
        {
            my $onefeature = ${$modulesref}[$i];

            # Java and Ada only, if the correct settings are set
            my $styles = "";
            if ( $onefeature->{'Styles'} ) { $styles = $onefeature->{'Styles'}; }

            # Controlling the language!
            # Only language independent feature or feature with the correct language will be included into the table
            # But help packs are different. They have en-US added as setup language.

            if (! (!(( $onefeature->{'ismultilingual'} )) || ( $onefeature->{'specificlanguage'} eq $onelanguage ) || $installer::globals::helppack ) )  { next; }

            my %feature = ();

            $feature{'feature'} = get_feature_gid($onefeature);
            $feature{'feature_parent'} = get_feature_parent($onefeature);
            $feature{'Title'} = $onefeature->{'Name'};
            $feature{'Description'} = $onefeature->{'Description'};
            $feature{'Description'} =~ s/\\\"/\"/g;  # no more masquerading of '"'
            $feature{'Display'} = get_feature_display($onefeature);
            $feature{'Level'} = get_feature_level($onefeature);
            $feature{'Directory_'} = get_feature_directory($onefeature);
            $feature{'Attributes'} = get_feature_attributes($onefeature);

            my $oneline = $feature{'feature'} . "\t" . $feature{'feature_parent'} . "\t" . $feature{'Title'} . "\t"
                    . $feature{'Description'} . "\t" . $feature{'Display'} . "\t" . $feature{'Level'} . "\t"
                    . $feature{'Directory_'} . "\t" . $feature{'Attributes'} . "\n";

            push(@featuretable, $oneline);

            # collecting all feature in global feature collector (so that properties can be set in property table)
            if ( ! grep {$_ eq $feature{'feature'}} @installer::globals::featurecollector )
            {
                push(@installer::globals::featurecollector, $feature{'feature'});
            }

            # collecting all language feature in feature collector for check of language selection
            if (( $styles =~ /\bSHOW_MULTILINGUAL_ONLY\b/ ) && ( $onefeature->{'ParentID'} ne $installer::globals::rootmodulegid ))
            {
                $installer::globals::multilingual_only_modules{$feature{'feature'}} = 1;
            }

            # collecting all application feature in global feature collector for check of application selection
            if ( $styles =~ /\bAPPLICATIONMODULE\b/ )
            {
                $installer::globals::application_modules{$feature{'feature'}} = 1;
            }
        }

        # Saving the file

        my $featuretablename = $basedir . $installer::globals::separator . "Feature.idt" . "." . $onelanguage;
        installer::files::save_file($featuretablename ,\@featuretable);
        $infoline = "Created idt file: $featuretablename\n";
        push(@installer::globals::logfileinfo, $infoline);
    }

}

1;
