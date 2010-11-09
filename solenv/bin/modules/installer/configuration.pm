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

package installer::configuration;

use Cwd;
use installer::converter;
use installer::existence;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::logger;
use installer::remover;
use installer::systemactions;

################################################################################
# Getting package from configurationitem (for instance: org.openoffice.Office)
# Getting name from configurationitem (for instance: Common)
################################################################################

sub analyze_path_of_configurationitem
{
    my ($configurationitemsref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::configuration::analyze_path_of_configurationitem : $#{$configurationitemsref}"); }

    my ($startpath, $nodes, $name, $packagename, $onenode, $first, $second, $third, $bracketnode);

    for ( my $i = 0; $i <= $#{$configurationitemsref}; $i++ )
    {
        my $oneconfig = ${$configurationitemsref}[$i];
        my $path = $oneconfig->{'Path'};

        installer::remover::remove_leading_and_ending_slashes(\$path);  # in scp are some path beginning with "/"

        if ( $path =~ /^\s*(.*?)\/(.*)\s*$/ )
        {
            $startpath = $1;
            $nodes = $2;
        }
        else
        {
            installer::exiter::exit_program("ERROR: Unknown format of ConfigurationItem path: $path", "analyze_path_of_configurationitem");
        }

        # Startpath is now: org.openoffice.Setup
        # Nodes is now: Office/Factories/com.sun.star.chart.ChartDocument

        # Dividing the startpath into package (org.openoffic) and name (Setup).

        $oneconfig->{'startpath'} = $startpath; # saving the startpath into the hash

        if ( $startpath =~ /^\s*(\S*)\.(\S*?)\s*$/ )
        {
            $packagename = $1;
            $name = $2;
            $oneconfig->{'name'} = $name;
            $oneconfig->{'packagename'} = $packagename;
        }
        else
        {
            installer::exiter::exit_program("ERROR: Unknown format of ConfigurationItem startpath: $startpath", "analyze_path_of_configurationitem");
        }

        # Collecting all nodes

        installer::remover::remove_leading_and_ending_slashes(\$nodes);

        my $counter = 1;

        # Attention: Do not trust the slash
        # Filters/Filter['StarWriter 5.0 Vorlage/Template']
        # Menus/New/*['m10']/Title

        if ( $nodes =~ /^(.*\[\')(.*\/.*)(\'\].*)$/ )
        {
            $first = $1;
            $second = $2;
            $third = $3;

            $second =~ s/\//SUBSTITUTEDSLASH/g; # substituting "/" to "SUBSTITUTEDSLASH"
            $nodes = $first . $second . $third;
        }

        while ( $nodes =~ /\// )
        {
            if ($nodes =~ /^\s*(.*?)\/(.*)\s*$/ )
            {
                $onenode = $1;
                $nodes = $2;
                $nodename = "node". $counter;

                # Special handling for filters. Difference between:
                # Filter['StarWriter 5.0 Vorlage/Template'] without oor:op="replace"
                # *['m10'] with oor:op="replace"

                if ( $onenode =~ /^\s*Filter\[\'(.*)\'\].*$/ ) { $oneconfig->{'isfilter'} = 1; }

                # Changing the nodes with brackets:
                # Filter['StarWriter 5.0 Vorlage/Template']
                # *['m10']

                if ( $onenode =~ /^.*\[\'(.*)\'\].*$/ )
                {
                    $onenode = $1;
                    $bracketnode = "bracket_" . $nodename;
                    $oneconfig->{$bracketnode} = 1;
                }

                $onenode =~ s/SUBSTITUTEDSLASH/\//g;    # substituting "SUBSTITUTEDSLASH" to "/"
                $oneconfig->{$nodename} = $onenode;

                # special handling for nodes "Factories"

                if ( $onenode eq "Factories" ) { $oneconfig->{'factoriesnode'} = $counter; }
                else { $oneconfig->{'factoriesnode'} = -99; }
            }

            $counter++
        }

        # and the final node

        if ( $nodes =~ /^\s*Filter\[\'(.*)\'\].*$/ ) { $oneconfig->{'isfilter'} = 1; }

        $nodename = "node". $counter;

        if ( $nodes =~ /^.*\[\'(.*)\'\].*$/ )
        {
            $nodes = $1;
            $bracketnode = "bracket_" . $nodename;
            $oneconfig->{$bracketnode} = 1;
        }

        $nodes =~ s/SUBSTITUTEDSLASH/\//g;  # substituting "SUBSTITUTEDSLASH" to "/"

        if (($nodes eq "Name") || ($nodes eq "Title"))  # isocodes below "Name" or "Title"
        {
            # if the last node $nodes is "Name" or "Title", it is a Property, not a name! See Common.xcu

            $oneconfig->{'isisocode'} = 1;

            if ( $nodes eq "Name" ) { $oneconfig->{'isname'} = 1; }
            if ( $nodes eq "Title" ) { $oneconfig->{'istitle'} = 1; }
            $counter--;     # decreasing the counter, because "Name" and "Title" are no nodes
        }
        else
        {
            $oneconfig->{$nodename} = $nodes;
            $oneconfig->{'isisocode'} = 0;
        }

        # special handling for nodes "Factories"

        if ( $onenode eq "Factories" ) { $oneconfig->{'factoriesnode'} = $counter; }
        else { $oneconfig->{'factoriesnode'} = -99; }

        # saving the number of nodes

        $oneconfig->{'nodenumber'} = $counter;
    }
}

####################################################################
# Inserting the start block into a configuration file
####################################################################

sub insert_start_block_into_configfile
{
    my ($configfileref, $oneconfig) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::configuration::insert_start_block_into_configfile : $#{$configfileref} : $oneconfig->{'name'}"); }

    my $line = '<?xml version="1.0" encoding="UTF-8"?>' . "\n";
    push( @{$configfileref}, $line);

    $line = '<oor:component-data xmlns:oor="http://openoffice.org/2001/registry" xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:install="http://openoffice.org/2004/installation" oor:name="FILENAME" oor:package="PACKAGENAME">' . "\n";
    my $packagename = $oneconfig->{'packagename'};
    my $name = $oneconfig->{'name'};
    $line =~ s/PACKAGENAME/$packagename/g;
    $line =~ s/FILENAME/$name/g;
    push( @{$configfileref}, $line);

    $line = "\n";
    push( @{$configfileref}, $line);
}

####################################################################
# Inserting the end block into a configuration file
####################################################################

sub insert_end_block_into_configfile
{
    my ($configfileref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::configuration::insert_end_block_into_configfile : $#{$configfileref}"); }

    my $line = "\n" . '</oor:component-data>' . "\n";
    push( @{$configfileref}, $line);
}

##############################################################
# Getting the content of a node
##############################################################

sub get_node_content
{
    my ($nodeline) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::configuration::get_node_content : $nodeline"); }

    my $content = "";

    if ( $nodeline =~ /name\=\"(.*?)\"/ )
    {
        $content = $1;
    }
    else
    {
        installer::exiter::exit_program("ERROR: Impossible error in function get_node_content!", "get_node_content");
    }

    return \$content;
}

##############################################################
# Getting the line number of an existing node
# Return "-1" if node does not exist
##############################################################

sub get_node_line_number
{
    my ($nodecount, $oneconfig, $oneconfigfileref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::configuration::get_node_line_number : $nodecount : $oneconfig->{'name'} : $#{$oneconfigfileref}"); }

    my $linenumber = -1;    # the node does not exists, if returnvalue is "-1"

    # Attention: Take care of the two title nodes:
    # Path=org.openoffice.Office.Common/Menus/Wizard/*['m14']/Title
    # Path=org.openoffice.Office.Common/Menus/Wizard/*['m15']/Title
    # -> every subnode has to be identical

    # creating the allnodes string from $oneconfig

    my $allnodes = "";

    for ( my $i = 1; $i <= $nodecount; $i++ )
    {
        my $nodename = "node" . $i;
        $allnodes .= $oneconfig->{$nodename} . "/";
    }

    installer::remover::remove_leading_and_ending_slashes(\$allnodes);  # exactly this string has to be found  in the following iteration

    # Iterating over the already built configuration file

    my @allnodes = ();

    for ( my $i = 0; $i <= $#{$oneconfigfileref}; $i++ )
    {
        my $line = ${$oneconfigfileref}[$i];
        installer::remover::remove_leading_and_ending_whitespaces(\$line);
        my $nodechanged = 0;

        if ( $line =~ /^\s*\<node/ )    # opening node
        {
            $nodechanged = 1;
            my $nodecontentref = get_node_content($line);
            push(@allnodes, $$nodecontentref);  # collecting all nodes in an array
        }

        if ( $line =~ /^\s*\<\/node/ )  # ending node
        {
            $nodechanged = 1;
            pop(@allnodes);     # removing the last node from the array
        }

        if (( $nodechanged ) && ($#allnodes > -1))  # a node was found and the node array is not empty
        {
            # creating the string to compare with the string $allnodes

            my $nodestring = "";

            for ( my $j = 0; $j <= $#allnodes; $j++ )
            {
                $nodestring .= $allnodes[$j] . "/";
            }

            installer::remover::remove_leading_and_ending_slashes(\$nodestring);

            if ( $nodestring eq $allnodes )
            {
                # that is exactly the same node

                $linenumber = $i;
                $linenumber++;  # increasing the linenumber
                last;

            }
        }
    }

    return $linenumber;
}

##############################################################
# Inserting one configurationitem into the configurationfile
##############################################################

sub insert_into_config_file
{
    my ($oneconfig, $oneconfigfileref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::configuration::insert_into_config_file : $oneconfig->{'nodenumber'} : $#{$oneconfigfileref}"); }

    my ($nodename, $nodecontent, $newnodeline, $bracketkey, $line, $insertline);

    # interpreting the nodes, keys and values

    my $nodeline = '<node oor:name="NODECONTENT" REPLACEPART >' . "\n";
    my $propline = '<prop oor:name="KEYCONTENT" REPLACEPART TYPEPART>' . "\n";
    my $valueline = '<value SEPARATORPART>VALUECONTENT</value>' . "\n";
    my $langvalueline = '<value xml:lang="SAVEDLANGUAGE">VALUECONTENT</value>' . "\n";
    my $propendline = '</prop>' . "\n";
    my $nodeendline = '</node>' . "\n";

    my $replacepart = 'oor:op="replace"';
    my $typepart = 'oor:type="xs:VALUETYPE"';   # VALUETYPE can be "string", "boolean", ...

    my $nodecount = $oneconfig->{'nodenumber'};
    my $styles = $oneconfig->{'Styles'};

    for ( my $i = 1; $i <= $nodecount; $i++ )
    {
        $insertline = get_node_line_number($i, $oneconfig, $oneconfigfileref);

        if ( $insertline == -1) # if true, the node does not exist
        {
            $nodename = "node" . $i;
            $nodecontent = $oneconfig->{$nodename};
            $newnodeline = $nodeline;

            $newnodeline =~ s/NODECONTENT/$nodecontent/g;

            # Case1:
            # Nodes with brackets, need the replacepart 'oor:op="replace"'
            # Bracket node is set for each node with: bracket_node1=1, bracket_node2=1, ...
            # Case a: <node oor:name="m0" oor:op="replace">     (Common.xcu needs oor:op="replace")
            # Case b: <node oor:name="Ami Pro 1.x-3.1 (W4W)">   (TypeDetection.xcu does not need oor:op="replace")
            # For case b introducting a special case for Filters

            $bracketkey = "bracket_" . $nodename;

            my $localbracketkey = 0;

            if ( $oneconfig->{$bracketkey} ) { $localbracketkey = $oneconfig->{$bracketkey}; }

            if ( $localbracketkey == 1 )    # 'oor:op="replace"' is needed
            {
                my $isfilter = 0;
                if ( $oneconfig->{'isfilter'} ) { $isfilter = $oneconfig->{'isfilter'}; }

                if ( $isfilter == 1 )   # this is a filter
                {
                    $newnodeline =~ s/REPLACEPART//;
                }
                else
                {
                    $newnodeline =~ s/REPLACEPART/$replacepart/;
                }

                $newnodeline =~ s/\s*\>/\>/;    # removing resulting whitespaces
            }

            # Case2:
            # Nodes below a Node "Factories", also need the replacepart 'oor:op="replace"'
            # This is saved in $oneconfig->{'factoriesnode'}. If not set, the value is "-99"

            if ( $i == $oneconfig->{'factoriesnode'} )
            {
                $newnodeline =~ s/REPLACEPART/$replacepart/;
                $newnodeline =~ s/\s*\>/\>/;    # removing resulting whitespaces
            }

            # Case3:
            # In all other cases, REPLACEPART in nodes can be removed

            $newnodeline =~ s/REPLACEPART//;
            $newnodeline =~ s/\s*\>/\>/;    # removing resulting whitespaces

            # Finding the correct place for inserting the node

            if ( $i == 1 )  # this is a toplevel node
            {
                push(@{$oneconfigfileref}, $newnodeline);
                push(@{$oneconfigfileref}, $nodeendline);
            }
            else
            {
                # searching for the parent node

                my $parentnumber = $i-1;
                $insertline = get_node_line_number($parentnumber, $oneconfig, $oneconfigfileref);
                splice(@{$oneconfigfileref}, $insertline, 0, ($newnodeline, $nodeendline));
            }
        }
    }

    # Setting variables $isbracketnode and $isfactorynode for the properties


    my $isbracketnode = 0;
    my $isfactorynode = 0;

    for ( my $i = 1; $i <= $nodecount; $i++ )
    {
        $nodename = "node" . $i;
        $bracketkey = "bracket_" . $nodename;

        my $localbracketkey = 0;
        if ( $oneconfig->{$bracketkey} ) { $localbracketkey = $oneconfig->{$bracketkey}; }

        if ( $localbracketkey == 1 ) { $isbracketnode = 1;  }
        if ( $i == $oneconfig->{'factoriesnode'} ) { $isfactorynode = 1; }
    }

    # now all nodes exist, and the key and value can be inserted into the configfile
    # the next line contains the key, for instance: <prop oor:name="UseDefaultMailer" oor:type="xs:boolean">
    # my $propline = '<prop oor:name="KEYCONTENT" REPLACEPART TYPEPART>' . "\n";
    # The type is only needed, if a replace is set.

    my $newpropline = $propline;

    # Replacement of KEYCONTENT, REPLACEPART and TYPEPART

    # Case 1:
    # Properties with oor:name="Name" (Common.xcu) are simply <prop oor:name="Name">
    # The information about such a property is stored in $oneconfig->{'isisocode'}

    if ( $oneconfig->{'isisocode'} )
    {
        if ( $oneconfig->{'isname'} ) { $newpropline =~ s/KEYCONTENT/Name/; }       # The property name is always "Name"
        if ( $oneconfig->{'istitle'} ) { $newpropline =~ s/KEYCONTENT/Title/; }     # The property name is always "Title"
        $newpropline =~ s/REPLACEPART//;
        $newpropline =~ s/TYPEPART//;
        $newpropline =~ s/\s*\>/\>/;    # removing resulting whitespaces
    }

    # Begin of all other cases

    my $key = $oneconfig->{'Key'};
    $newpropline =~ s/KEYCONTENT/$key/;

    my $valuetype;

    if ( $styles =~ /CFG_STRING\b/ ) { $valuetype = "string"; }
    elsif ( $styles =~ /CFG_NUMERIC/ ) { $valuetype = "int"; }
    elsif ( $styles =~ /CFG_BOOLEAN/ ) { $valuetype = "boolean"; }
    elsif ( $styles =~ /CFG_STRINGLIST/ ) { $valuetype = "string-list"; }
    else
    {
        installer::exiter::exit_program("ERROR: Unknown configuration value type: $styles", "insert_into_config_file");
    }

    # Case 2:
    # Properties below a node "Factories" do not need a 'oor:op="replace"' and a 'oor:type="xs:VALUETYPE"'

    if ( $isfactorynode )
    {
        $newpropline =~ s/REPLACEPART//;
        $newpropline =~ s/TYPEPART//;
        $newpropline =~ s/\s*\>/\>/;    # removing resulting whitespaces
    }

    # Case 3:
    # Properties below a "bracket" node do not need a 'oor:op="replace"', except they are iso-codes
    # Assumption here: They are multilingual

    if ( $isbracketnode )
    {
        my $localtypepart = $typepart;
        $localtypepart =~ s/VALUETYPE/$valuetype/;
        $newpropline =~ s/TYPEPART/$localtypepart/;

        if ( $oneconfig->{'ismultilingual'} )   # This is solved by "Name" and "Title"
        {
            $newpropline =~ s/REPLACEPART/$replacepart/;
        }
        else
        {
            $newpropline =~ s/REPLACEPART//;
        }

        $newpropline =~ s/\s*\>/\>/;    # removing resulting whitespaces
    }

    # Case 4:
    # if the flag CREATE is set, the properties get 'oor:op="replace"' and 'oor:type="xs:VALUETYPE"'

    if (( $styles =~ /\bCREATE\b/ ) && (!($isbracketnode)))
    {
        my $localtypepart = $typepart;
        $localtypepart =~ s/VALUETYPE/$valuetype/;

        $newpropline =~ s/TYPEPART/$localtypepart/;
        $newpropline =~ s/REPLACEPART/$replacepart/;
        $newpropline =~ s/\s*\>/\>/;    # removing resulting whitespaces
    }

    # Case 5:
    # all other ConfigurationItems do not need 'oor:op="replace"' and 'oor:type="xs:VALUETYPE"'

    $newpropline =~ s/REPLACEPART//;
    $newpropline =~ s/TYPEPART//;
    $newpropline =~ s/\s*\>/\>/;    # removing resulting whitespaces

    # finally the value can be set

    my $value = $oneconfig->{'Value'};

    # Some values in setup script are written during installation process by the setup. These
    # have values like "<title>". This will lead to an error, because of the brackets. Therefore the
    # brackets have to be removed.

    # ToDo: Substituting the setup replace variables

    $value =~ s/^\s*\<//;
    $value =~ s/\>\s*$//;

    # Deal with list separators
    my $separatorpart = '';
    if ( ($valuetype eq "string-list") && ($value =~ /\|/) )
    {
        $separatorpart = 'oor:separator="|"';
    }

    # Fake: substituting german umlauts

    $value =~ s/\ä/ae/;
    $value =~ s/\ö/oe/;
    $value =~ s/\ü/ue/;
    $value =~ s/\Ä/AE/;
    $value =~ s/\Ö/OE/;
    $value =~ s/\Ü/UE/;

    my $newvalueline;

    if (!($oneconfig->{'isisocode'} ))  # this is the simpe case
    {
        $newvalueline = $valueline;
        $newvalueline =~ s/VALUECONTENT/$value/g;
        $newvalueline =~ s/SEPARATORPART/$separatorpart/;
    }
    else
    {
        $newvalueline = $langvalueline;
        $newvalueline =~ s/VALUECONTENT/$value/;
        my $savedlanguage = $oneconfig->{'Key'};
        $newvalueline =~ s/SAVEDLANGUAGE/$savedlanguage/;
    }

    # For language dependent values, it is possible, that the property already exist.
    # In this case the prop must not be created again and only the value has to be included:
    # <prop oor:name="Name">
    #  <value xml:lang="de">OpenOffice.org 2.0 Diagramm</value>
    #  <value xml:lang="en-US">OpenOffice.org 2.0 Chart</value>
    # </prop>

    # The key has to be written after the line, containing the complete node

    $insertline = get_node_line_number($nodecount, $oneconfig, $oneconfigfileref);

    if ( $oneconfig->{'ismultilingual'} )
    {
        if ( $newpropline eq ${$oneconfigfileref}[$insertline] )
        {
            if (!($newvalueline eq ${$oneconfigfileref}[$insertline+1]))    # only include, if the value not already exists (example: value="FALSE" for many languages)
            {
                splice(@{$oneconfigfileref}, $insertline+1, 0, ($newvalueline));    # only the value needs to be added
            }
        }
        else
        {
            splice(@{$oneconfigfileref}, $insertline, 0, ($newpropline, $newvalueline, $propendline));
        }
    }
    else
    {
        splice(@{$oneconfigfileref}, $insertline, 0, ($newpropline, $newvalueline, $propendline));
    }

    return $oneconfigfileref;
}

##########################################################
# Inserting tabs for better view into configuration file
##########################################################

sub insert_tabs_into_configfile
{
    my ($configfileref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::configuration::insert_tabs_into_configfile : $#{$configfileref}"); }

    my $counter = 0;

    for ( my $i = 0; $i <= $#{$configfileref}; $i++ )
    {
        my $line = ${$configfileref}[$i];
        my $linebefore = ${$configfileref}[$i-1];

        if (( $line =~ /^\s*\<node/ ) || ( $line =~ /^\s*\<prop/ ))
        {
            if ((!( $linebefore =~ /^\s*\<\/node/ )) && (!( $linebefore =~ /^\s*\<\/prop/ )))       # do not increase after "</node" and after "</prop"
            {
                $counter++;
            }
        }

        if ( $line =~ /^\s*\<value/ )
        {
            if (!($linebefore =~ /^\s*\<value/ ))       # do not increase counter with "<value>" after "<value>" (multilingual configitems)
            {
                $counter++;
            }
        }

        if (( $line =~ /^\s*\<\/node\>/ ) || ( $line =~ /^\s*\<\/prop\>/ ))
        {
            if ((!( $linebefore =~ /^\s*\<node/ )) && (!( $linebefore =~ /^\s*\<prop/ )))       # do not decrease after "<node" and after "<prop"
            {
                $counter--;
            }
        }

        if ($counter > 0)
        {
            for ( my $j = 0; $j < $counter; $j++ )
            {
                $line = "\t" . $line;
            }
        }

        ${$configfileref}[$i] = $line;
    }
}

######################################################################
# Collecting all different configuration items (Files and Modules)
######################################################################

sub collect_all_configuration_items
{
    my ($configurationitemsref, $item) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::configuration::collect_all_configuration_items : $#{$configurationitemsref} : $item"); }

    my @allitems = ();

    for ( my $i = 0; $i <= $#{$configurationitemsref}; $i++ )
    {
        my $oneconfig = ${$configurationitemsref}[$i];

        if (! installer::existence::exists_in_array($oneconfig->{$item}, \@allitems))
        {
            push(@allitems, $oneconfig->{$item});
        }
    }

    return \@allitems;
}

######################################################################
# Collecting all module specific configuration items
######################################################################

sub get_all_configitems_at_module
{
    my ($moduleid, $configurationitemsref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::configuration::get_all_configitems_at_module : $moduleid : $#{$configurationitemsref}"); }

    my @moduleconfigurationitems = ();

    for ( my $i = 0; $i <= $#{$configurationitemsref}; $i++ )
    {
        my $oneconfig = ${$configurationitemsref}[$i];

        if ( $oneconfig->{'ModuleID'} eq $moduleid )
        {
            push(@moduleconfigurationitems, $oneconfig);
        }
    }

    return \@moduleconfigurationitems;
}

#######################################################
# Saving and zipping the created configurationfile
#######################################################

sub save_and_zip_configfile
{
    my ($oneconfigfileref, $onefile, $onemodule, $configdir) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::configuration::save_and_zip_configfile : $#{$oneconfigfileref} : $onefile : $onemodule : $configdir"); }

    my $savefilename = $onefile;
    $savefilename =~ s/\./\_/g;
    $savefilename = $savefilename . "_" . $onemodule;
    $savefilename = $savefilename . ".xcu";
    my $shortsavefilename = $savefilename;

    $savefilename = $configdir . $installer::globals::separator . $savefilename;

    installer::files::save_file($savefilename, $oneconfigfileref);

    # zipping the configfile

    my $returnvalue = 1;

    my $zipfilename = $shortsavefilename;
    $zipfilename =~ s/\.xcu/\.zip/;

    my $currentdir = cwd();
    if ( $installer::globals::iswin ) { $currentdir =~ s/\//\\/g; }

    chdir($configdir);

     my $systemcall = "$installer::globals::zippath -q -m $zipfilename $shortsavefilename";
    $returnvalue = system($systemcall);

    chdir($currentdir);

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not zip $savefilename to $zipfilename\n";
    }
    else
    {
        $infoline = "SUCCESS: Zipped file $savefilename to $zipfilename\n";
    }
    push( @installer::globals::logfileinfo, $infoline);

    return $zipfilename;
}

#####################################################################
# Adding the newly created configuration file into the file list
#####################################################################

sub add_zipfile_into_filelist
{
    my ($zipfilename, $configdir, $filesarrayref, $onemodule) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::configuration::add_zipfile_into_filelist: $zipfilename : $configdir : $#{$filesarrayref} : $onemodule"); }

    my $longzipfilename = $configdir . $installer::globals::separator . $zipfilename;
    my $gid = "gid_" . $zipfilename;
    $gid =~ s/\./\_/g;

    my %configfile = ();

    # Taking the base data from the "gid_File_Lib_Vcl"

    my $vclgid = "gid_File_Lib_Vcl";
    my $vclfile = installer::existence::get_specified_file($filesarrayref, $vclgid);

    # copying all base data
    installer::converter::copy_item_object($vclfile, \%configfile);

    # and overriding all new data
    $configfile{'ismultilingual'} = 0;
    $configfile{'sourcepath'} = $longzipfilename;
    $configfile{'Name'} = $zipfilename;
    $configfile{'UnixRights'} = "644";
    $configfile{'gid'} = $gid;
    $configfile{'Dir'} = "gid_Dir_Share_Uno_Packages";
    $configfile{'destination'} = "share" . $installer::globals::separator . "uno_packages" . $installer::globals::separator . $zipfilename;
    $configfile{'modules'} = $onemodule;    # assigning the file to the correct module!

    push(@{$filesarrayref}, \%configfile);
}

#######################################################
# Creating configuration files from configurationitems
#######################################################

sub create_configuration_files
{
    my ($configurationitemsref, $filesarrayref, $languagestringref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::configuration::create_configuration_files: $#{$configurationitemsref} : $#{$filesarrayref} : $$languagestringref"); }

    installer::logger::include_header_into_logfile("Creating configuration files:");

    # creating the directory

    my $configdir = installer::systemactions::create_directories("configfiles", $languagestringref);

    $configdir = installer::converter::make_path_conform($configdir);

    # collecting an array of all modules
    my $allmodules = collect_all_configuration_items($configurationitemsref, "ModuleID");

    # iterating over all modules

    for ( my $i = 0; $i <= $#{$allmodules}; $i++ )
    {
        my $onemodule = ${$allmodules}[$i];

        my $moduleconfigitems = get_all_configitems_at_module($onemodule, $configurationitemsref);

        # collecting an array of all "startpath". This are all different files (org.openoffice.Office.Common).
        my $allfiles = collect_all_configuration_items($moduleconfigitems, "startpath");

        # iteration over all files

        for ( my $j = 0; $j <= $#{$allfiles}; $j++ )
        {
            my $onefile = ${$allfiles}[$j];

            my @oneconfigfile = ();
            my $oneconfigfileref = \@oneconfigfile;

            my $startblockwritten = 0;

            for ( my $k = 0; $k <= $#{$moduleconfigitems}; $k++ )
            {
                my $oneconfig = ${$moduleconfigitems}[$k];

                my $startpath = $oneconfig->{'startpath'};

                if ($startpath eq $onefile)
                {
                    if (!($startblockwritten))  # writing some global lines into the xcu file
                    {
                        insert_start_block_into_configfile($oneconfigfileref, $oneconfig);
                        $startblockwritten = 1;
                    }

                    $oneconfigfileref = insert_into_config_file($oneconfig, $oneconfigfileref);
                }
            }

            insert_end_block_into_configfile($oneconfigfileref);

            # inserting tabs for nice appearance
            insert_tabs_into_configfile($oneconfigfileref);

            # saving the configfile
            my $zipfilename = save_and_zip_configfile($oneconfigfileref, $onefile, $onemodule, $configdir);

            # adding the zipped configfile to the list of installed files
            # Some data are set now, others are taken from the file "soffice.exe" ("soffice.bin")

            add_zipfile_into_filelist($zipfilename, $configdir, $filesarrayref, $onemodule);
        }
    }

    my $infoline = "\n";
    push( @installer::globals::logfileinfo, $infoline);

}

1;
