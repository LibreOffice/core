#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: feature.pm,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: kz $ $Date: 2005-11-11 14:17:49 $
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

package installer::windows::feature;

use installer::files;
use installer::globals;
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

    if ( $parentgid eq "gid_Module_Root" ) { $parentgid = ""; }

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

    # special case: Feature is below root module and has flag "HIDDEN_ROOT" -> $display is 0

    my $styles = "";
    if ( $onefeature->{'Styles'} ) { $styles = $onefeature->{'Styles'}; }

    if (( $parentid eq "gid_Module_Root" ) && ( $styles =~ /\bHIDDEN_ROOT\b/ ))
    {
        $display = "0";
    }

    # Special handling for c05office. No program module visible.
    if (( $onefeature->{'gid'} eq "gid_Module_Prg" ) && ( $installer::globals::product =~ /c05office/i )) { $display = "0"; }

    if ( $installer::globals::languagepack ) { $display = "0"; }     # making all feature invisible!

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

    if ( $localdefault eq "NO" )    # explicitely set Default = "NO"
    {
        $level = "200";             # deselected in default installation, base is 100
    }

    # special handling for Java and Ada

    if ( $onefeature->{'Name'} =~ /java/i || $onefeature->{'Name'} =~ /adabas/i ) { $level = $level + 40; }

    return $level
}

##############################################################
# Returning the directory for a feature.
##############################################################

sub get_feature_directory
{
    my ($onefeature) = @_;

    my $directory;

    # my $parentid = "";

    # if ( $onefeature->{'ParentID'} ) { $parentid = $onefeature->{'ParentID'}; }

    # if (( $parentid eq "" ) || ( $parentid eq "gid_Module_Root" ))    # This is the root module and the modules directly below the root
    # {
        $directory = "INSTALLLOCATION";
    # }
    # else
    # {
    #   $directory = "";
    # }

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

    if (( $parentgid eq "" ) || ( $parentgid eq "gid_Module_Root" )) { $attributes = "8"; }
    else { $attributes = "10"; }

    return $attributes
}

##############################################################
# Searching for Name and Description of language pack
# modules in Langpack.mlf
##############################################################

sub get_localized_string
{
    my ($featuregid, $onelanguage, $translationfile, $type) = @_;

    my $locallanguage = "";
    if ( $featuregid =~ /^\s*(\w+)_(.+?)\s*$/ ) { $locallanguage = $2; }

    my $windowslanguage = "";
    if ( $locallanguage =~ /top/i ) { $windowslanguage = $locallanguage; }
    else { $windowslanguage = installer::windows::language::get_windows_language($locallanguage); }

    my $searchstring = "";
    if ( $type eq "Name" ) { $searchstring = "OOO_LANGPACK_NAME_" . $windowslanguage; }
    else { $searchstring = "OOO_LANGPACK_DESC_" . $windowslanguage; }

    my $language_block = installer::windows::idtglobal::get_language_block_from_language_file($searchstring, $translationfile);
    my $newstring = installer::windows::idtglobal::get_language_string_from_language_block($language_block, $onelanguage, $searchstring);

    return $newstring;
}

#####################################################################
# For multilingual installation sets, the language dependent files
# are advised to the language feature.
#####################################################################

sub change_modules_in_filescollector
{
    my ($filesref) = @_;

    my $feature = "gid_Module_Dynamic_Language_TOP";
    push(@installer::globals::multilanguagemodules, $feature);

    my $onefile;

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        $onefile =  ${$filesref}[$i];

        # Searching for multilingual files

        if ( $onefile->{'ismultilingual'} )
        {
            my $officelanguage = $onefile->{'specificlanguage'};
            $feature = "gid_Module_Dynamic_Language_" . $officelanguage;     # gid_Module_Dynamic_Language_de
            $onefile->{'modules'} = $feature;   # assigning the new language feature !

            # Collecting all new feature
            if ( ! installer::existence::exists_in_array($feature, \@installer::globals::multilanguagemodules) )
            {
                push(@installer::globals::multilanguagemodules, $feature);
            }
        }
    }
}

##############################################################
# Creating the feature for the language packs.
##############################################################

sub add_language_pack_feature
{
    my ($featuretableref, $translationfile, $onelanguage) = @_;

    for ( my $i = 0; $i <= $#installer::globals::languagepackfeature; $i++ )
    {
        my %feature = ();

        my $gid = $installer::globals::languagepackfeature[$i];
        # Attention: Maximum feature length is 38!
        installer::windows::idtglobal::shorten_feature_gid(\$gid);

        $feature{'feature'} = $gid;     # "gm_Langpack_de"
        $feature{'feature_parent'} = "";
        $feature{'Title'} = get_localized_string($gid, $onelanguage, $translationfile, "Name");
        $feature{'Description'} = get_localized_string($gid, $onelanguage, $translationfile, "Description");
        if ( $installer::globals::ismultilingual ) { $feature{'Display'} = "1"; }
        else { $feature{'Display'} = "1"; }
        $feature{'Level'} =  "20";
        $feature{'Directory_'} =  "INSTALLLOCATION";
        $feature{'Attributes'} =  "8";

        my $oneline = $feature{'feature'} . "\t" . $feature{'feature_parent'} . "\t" . $feature{'Title'} . "\t"
                    . $feature{'Description'} . "\t" . $feature{'Display'} . "\t" . $feature{'Level'} . "\t"
                    . $feature{'Directory_'} . "\t" . $feature{'Attributes'} . "\n";

        push(@{$featuretableref}, $oneline);

        # $onelanguage = "de"
        # $gid = "gm_Langpack_de"
        # -> only include if this is the language specific language name
        if ( $gid =~ /\_$onelanguage\s*$/) { push(@{$installer::globals::languagenames}, $feature{'Title'}); }  # saving all titles in the global variable!
    }

}

##################################################################
# Creating the feature for the multilingual installation sets.
##################################################################

sub add_multilingual_features
{
    my ($featuretableref, $translationfile, $onelanguage) = @_;

    # Adding the new dynamic language modules, collected in change_modules_in_filescollector

     my $topmodulegid = $installer::globals::multilanguagemodules[0];
    installer::windows::idtglobal::shorten_feature_gid(\$topmodulegid);

    for ( my $i = 0; $i <= $#installer::globals::multilanguagemodules; $i++ )
    {
        my %feature = ();

        my $gid = $installer::globals::multilanguagemodules[$i];
        # Attention: Maximum feature length is 38!
        installer::windows::idtglobal::shorten_feature_gid(\$gid);

        my $deselectable = 1;   # feature can be deselected
        if (( $i == 0 ) || ( $i == 1 )) { $deselectable = 0; } # Toplevel module and english cannot be deselected

        $feature{'feature'} = $gid;
        if ( $i == 0 ) { $feature{'feature_parent'} = ""; }
        else { $feature{'feature_parent'} = $topmodulegid; }
        $feature{'Title'} = get_localized_string($gid, $onelanguage, $translationfile, "Name");
        $feature{'Description'} = get_localized_string($gid, $onelanguage, $translationfile, "Description");
        $feature{'Display'} = "4";  # determines order of feature and visibility.
        $feature{'Level'} =  "20";
        $feature{'Directory_'} =  "INSTALLLOCATION";
        if ( $i == 0 ) { $feature{'Attributes'} = "8"; }
        else { $feature{'Attributes'} =  "10"; }

        if ( $deselectable == 0 ) { $feature{'Attributes'} = $feature{'Attributes'} + 16; } # making feature not deselectable

        $oneline = $feature{'feature'} . "\t" . $feature{'feature_parent'} . "\t" . $feature{'Title'} . "\t"
                    . $feature{'Description'} . "\t" . $feature{'Display'} . "\t" . $feature{'Level'} . "\t"
                    . $feature{'Directory_'} . "\t" . $feature{'Attributes'} . "\n";

        push(@{$featuretableref}, $oneline);
    }
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

    foreach $key (keys %{$variableshashref})
    {
        my $value = $variableshashref->{$key};
        replace_one_variable($translationfile, $value, $key);
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

    my $translationfile = "";
    if (( $installer::globals::languagepack ) || ( $installer::globals::ismultilingual ))
    {
        $translationfile = installer::files::read_file($installer::globals::idtlanguagepath . $installer::globals::separator . $installer::globals::langpackfilename);
        replace_variables($translationfile, $allvariableshashref);
    }

    for ( my $m = 0; $m <= $#{$languagesarrayref}; $m++ )
    {
        my $onelanguage = ${$languagesarrayref}[$m];

        my $infoline;

        my @featuretable = ();

        installer::windows::idtglobal::write_idt_header(\@featuretable, "feature");

        for ( my $i = 0; $i <= $#{$modulesref}; $i++ )
        {
            my $onefeature = ${$modulesref}[$i];

            # Controlling the language!
            # Only language independent feature or feature with the correct language will be included into the table

            if (! (!(( $onefeature->{'ismultilingual'} )) || ( $onefeature->{'specificlanguage'} eq $onelanguage )) )  { next; }

            my %feature = ();

            $feature{'feature'} = get_feature_gid($onefeature);
            $feature{'feature_parent'} = get_feature_parent($onefeature);
            # if ( $onefeature->{'ParentID'} eq "" ) { $feature{'feature_parent'} = ""; }   # Root has no parent
            $feature{'Title'} = $onefeature->{'Name'};
            $feature{'Description'} = $onefeature->{'Description'};
            $feature{'Display'} = get_feature_display($onefeature);
            $feature{'Level'} =  get_feature_level($onefeature);
            $feature{'Directory_'} =  get_feature_directory($onefeature);
            $feature{'Attributes'} =  get_feature_attributes($onefeature);

            my $oneline = $feature{'feature'} . "\t" . $feature{'feature_parent'} . "\t" . $feature{'Title'} . "\t"
                    . $feature{'Description'} . "\t" . $feature{'Display'} . "\t" . $feature{'Level'} . "\t"
                    . $feature{'Directory_'} . "\t" . $feature{'Attributes'} . "\n";

            push(@featuretable, $oneline);
        }

        if ( $installer::globals::languagepack ) { add_language_pack_feature(\@featuretable, $translationfile, $onelanguage); }

        if (( $installer::globals::ismultilingual ) && ( ! $installer::globals::languagepack )) { add_multilingual_features(\@featuretable, $translationfile, $onelanguage); }

        # Saving the file

        my $featuretablename = $basedir . $installer::globals::separator . "Feature.idt" . "." . $onelanguage;
        installer::files::save_file($featuretablename ,\@featuretable);
        $infoline = "Created idt file: $featuretablename\n";
        push(@installer::globals::logfileinfo, $infoline);
    }

}

1;