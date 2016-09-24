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

package par2script::module;

use par2script::converter;
use par2script::exiter;

###########################################
# Removing undefined gids
# from modules
###########################################

sub remove_from_modules
{
    my ($gid, $item) = @_;

    my $counter = 0;

    if ( ! exists($par2script::globals::searchkeys{$item}) ) { par2script::exiter::exit_program("ERROR: Unknown type \"$item\" at modules.", "remove_from_modules"); }
    my $searchkey = $par2script::globals::searchkeys{$item};

    my $allmodules = $par2script::globals::definitions{'Module'};

    my $onemodule;
    foreach $onemodule (keys %{$allmodules})
    {
        if (( exists($allmodules->{$onemodule}->{$searchkey}) ) && ( $allmodules->{$onemodule}->{$searchkey} =~ /\b$gid\b/ ))
        {
            my $infoline = "WARNING: Removing $gid because of missing definition\n";
            # print $infoline;
            push(@par2script::globals::logfileinfo, $infoline);

            $allmodules->{$onemodule}->{$searchkey} =~ s/\b$gid\b//;
            $allmodules->{$onemodule}->{$searchkey} =~ s/\,\s*\,/\,/;
            $allmodules->{$onemodule}->{$searchkey} =~ s/\(\s*\,\s*/\(/;
            $allmodules->{$onemodule}->{$searchkey} =~ s/\s*\,\s*\)/\)/;

            if (( $allmodules->{$onemodule}->{$searchkey} =~ /\(\s*\,\s*\)/ ) ||
                ( $allmodules->{$onemodule}->{$searchkey} =~ /\(\s*\)/ ))
            {
                delete($allmodules->{$onemodule}->{$searchkey});
            }

            $counter++;
        }
    }

    return $counter;
}

###########################################
# Removing undefined gids automatically
# from modules
###########################################

sub remove_undefined_gids_from_modules
{
    # If assigned gids for "File", "Directory" or "Unixlink" are not defined,
    # they are automatically removed from the module

    foreach $item ( @par2script::globals::items_assigned_at_modules )
    {
        my $assignedgids = $par2script::globals::assignedgids{$item};
        my $definedgids = $par2script::globals::definitions{$item};

        my $gid;
        foreach $gid ( keys %{$assignedgids} )
        {
            if ( ! exists( $definedgids->{$gid} ))
            {
                # deleting entry in module definition
                my $number_of_removals = remove_from_modules($gid, $item);
                # decreasing counter in assignments
                if ( $assignedgids->{$gid} > $number_of_removals ) { $assignedgids->{$gid} = $assignedgids->{$gid} - $number_of_removals; }
                else { delete($assignedgids->{$gid}); }
            }
        }
    }
}

############################################
# Getting the gid of the root module. The
# root module has no ParentID or an empty
# ParentID.
############################################

sub get_rootmodule_gid
{
    my $rootgid = "";
    my $foundroot = 0;

    my $allmodules = $par2script::globals::definitions{'Module'};

    my $modulegid = "";
    foreach $modulegid (keys %{$allmodules} )
    {
        # print "Module $modulegid\n";
        # my $content = "";
        # foreach $content (sort keys %{$allmodules->{$modulegid}}) { print "\t$content = $allmodules->{$modulegid}->{$content};\n"; }
        # print "End\n";
        # print "\n";

        if (( ! exists($allmodules->{$modulegid}->{'ParentID'})) || ( $allmodules->{$modulegid}->{'ParentID'} eq "" ))
        {
            if ( $foundroot ) { par2script::exiter::exit_program("ERROR: More than one Root module. Only one module without ParentID or with empty ParentID allowed ($rootgid and $modulegid).", "get_rootmodule_gid"); }
            $rootgid = $modulegid;
            $foundroot = 1;
        }
    }

    if ( ! $foundroot ) { par2script::exiter::exit_program("ERROR: Could not find Root module. Did not find module without ParentID or with empty ParentID.", "get_rootmodule_gid"); }

    return $rootgid;
}

####################################
# Adding defined items without
# assignment to the root module.
####################################

sub add_to_root_module
{
    # If defined gids for "File", "Directory" or "Unixlink" are not assigned,
    # they are automatically assigned to the root module

    my $rootmodulegid = get_rootmodule_gid();

    my $item;
    foreach $item ( @par2script::globals::items_assigned_at_modules )
    {
        my $assignedgids = $par2script::globals::assignedgids{$item};
        my $definedgids = $par2script::globals::definitions{$item};

        my $gidstring = "";

        # Perhaps there are already items assigned to the root
        if ( ! exists($par2script::globals::searchkeys{$item}) ) { par2script::exiter::exit_program("ERROR: Unknown type \"$item\" at modules.", "remove_from_modules"); }
        my $modulekey = $par2script::globals::searchkeys{$item};
        if ( exists($par2script::globals::definitions{'Module'}->{$rootmodulegid}->{$modulekey}) )
        {
            $gidstring = $par2script::globals::definitions{'Module'}->{$rootmodulegid}->{$modulekey};
            $gidstring =~ s/\(//;
            $gidstring =~ s/\)//;
        }

        my $gid;
        foreach $gid ( keys %{$definedgids} )
        {
            if ( ! exists( $assignedgids->{$gid} ))
            {
                if ( $gidstring eq "" )
                {
                    $gidstring = $gid;
                }
                else
                {
                    $gidstring = "$gidstring,$gid";
                }

                $assignedgids->{$gid} = 1;
            }
        }

        if ( $gidstring ne "" )
        {
            $gidstring = "\($gidstring\)";
            $par2script::globals::definitions{'Module'}->{$rootmodulegid}->{$modulekey} = $gidstring;
        }
    }
}

###################################################
# Including \n in a very long string
###################################################

sub include_linebreaks
{
    my ($allgidstring) = @_;

    my $newline = "";
    my $newlength = 0;

    $allgidstring =~ s/\(//;
    $allgidstring =~ s/\)//;

    my $allgids = par2script::converter::convert_stringlist_into_array_2($allgidstring, ",");

    if ( $#{$allgids} > -1 )
    {
        my $onegid;
        foreach $onegid ( @{$allgids} )
        {
            $newline = "$newline$onegid,";
            $newlength = $newlength + length($onegid) + 1; # +1 for the comma

            if ( $newlength > 80 )
            {
                $newline = $newline . "\n\t\t\t\t";
                $newlength = 0;
            }
        }
    }

    $newline =~ s/,\s*$//;
    $newline = "($newline)";

    return $newline;
}

###################################################
# Shorten the lines that belong to modules, if
# the length of the line is greater 100
###################################################

sub shorten_lines_at_modules
{
    my $item;
    foreach $item ( @par2script::globals::items_assigned_at_modules )
    {
        if ( ! exists($par2script::globals::searchkeys{$item}) ) { par2script::exiter::exit_program("ERROR: Unknown type \"$item\" at modules.", "shorten_lines_at_modules"); }
        my $searchkey = $par2script::globals::searchkeys{$item};

        my $allmodules = $par2script::globals::definitions{'Module'};

        my $onemodule;
        foreach $onemodule (keys %{$allmodules})
        {
            if (( exists($allmodules->{$onemodule}->{$searchkey}) ) &&
                ( length($allmodules->{$onemodule}->{$searchkey}) > 100 ))
            {
                # including "\n\t\t\t\t"
                my $newstring = include_linebreaks($allmodules->{$onemodule}->{$searchkey});
                $allmodules->{$onemodule}->{$searchkey} = $newstring;
            }
        }
    }
}

1;
