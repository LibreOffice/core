#*************************************************************************
#
#   $RCSfile: module.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: kz $ $Date: 2004-01-29 11:44:31 $
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


package par2script::module;

use par2script::converter;
use par2script::work;

###########################################
# Removing undefined gids automatically
# from modules
###########################################

sub remove_from_modules
{
    my ($script) = @_;

    # if these 4 gids are not defined, they are automatically removed from the module

    my $allfilegids = par2script::work::get_all_gids_from_script($script, "File");
    my $allproceduregids = par2script::work::get_all_gids_from_script($script, "Procedure");
    my $allcustomgids = par2script::work::get_all_gids_from_script($script, "Custom");
    my $alldirectorygids = par2script::work::get_all_gids_from_script($script, "Directory");

    for ( my $i = 0; $i <= $#{$script}; $i++ )
    {
        my $oneline = ${$script}[$i];

        my $modulename;

        if ( $oneline =~ /^\s*Module\s+(\w+)\s*$/ )
        {
            $modulegid = $1;
            next;
        }

        if ( $oneline =~ /^\s*(\w+?)\s*\=\s*\((.*?)\)\s*\;\s*$/ )
        {
            my $key = $1;
            my $allassigneditemsstring = $2;
            my $defineditemgids;

            if ( $key eq "Files" ) { $defineditemgids = $allfilegids; }
            elsif ( $key eq "Dirs" ) { $defineditemgids = $alldirectorygids; }
            elsif ( $key eq "Procedures" ) { $defineditemgids = $allproceduregids; }
            elsif ( $key eq "Customs" ) { $defineditemgids = $allcustomgids; }
            else { next; }  # for instance "Styles"

            my $allassigneditems = par2script::converter::convert_stringlist_into_array(\$allassigneditemsstring, ",");

            for ( my $j = 0; $j <= $#{$allassigneditems}; $j++ )
            {
                my $oneassigneditem = ${$allassigneditems}[$j];

                # is there a definition for this assigned item?

                my $itemisdefined = 0;

                for ( my $k = 0; $k <= $#{$defineditemgids}; $k++ )
                {
                    if ( $oneassigneditem eq ${$defineditemgids}[$k] )
                    {
                        $itemisdefined = 1;
                        last;
                    }
                }

                if (! $itemisdefined)
                {
                    my $infoline = "WARNING: Removing $oneassigneditem from Module $modulegid\n";
                    # print $infoline;
                    push(@par2script::globals::logfileinfo, $infoline);

                    ${$script}[$i] =~ s/$oneassigneditem//;
                    ${$script}[$i] =~ s/\,\s*\,/\,/;
                    ${$script}[$i] =~ s/\(\s*\,\s*/\(/;
                    ${$script}[$i] =~ s/\s*\,\s*\)/\)/;

                    if (( ${$script}[$i] =~ /\(\s*\,\s*\)/ ) || ( ${$script}[$i] =~ /\(\s*\)/ ))
                    {
                        # this line can be removed
                        splice(@{$script}, $i, 1);
                        $i--;       # decreasing the counter!
                    }
                }
            }
        }
    }
}

###########################################
# Adding the collectors
# to the root module
###########################################

sub create_rootmodule
{
    my ($rootmodule, $allitemgids, $itemname) = @_;

    if ( $#{$allitemgids} > -1 )
    {
        my $oneline = "\t$itemname \= \(";

        for ( my $i = 0; $i <= $#{$allitemgids}; $i++ )
        {
            my $onegid = ${$allitemgids}[$i];

            if ($oneline eq "") { $oneline = "\t\t\t\t"; }

            $oneline .= $onegid;

            if ( $i == $#{$allitemgids} ) { $oneline .= "\)\;"; }
            else { $oneline .= "\,"; }

            if ( length($oneline) > 100 )
            {
                $oneline .= "\n";
                push(@{$rootmodule}, $oneline);
                $oneline = "";
            }

        }

        if (! $oneline =~ /^\s*$/ )
        {
            $oneline .= "\n";
            push(@{$rootmodule}, $oneline);
        }
    }
}

###########################################
# Adding defined gids automatically
# to the root module
###########################################

sub add_to_root_module
{
    my ($script) = @_;

    my $rootmodulestartline = "";

    # if these 4 gids are defined and not added to another module, they are automatically added to the root module

    my $allfilegids = par2script::work::get_all_gids_from_script($script, "File");
    my $allproceduregids = par2script::work::get_all_gids_from_script($script, "Procedure");
    my $allcustomgids = par2script::work::get_all_gids_from_script($script, "Custom");
    my $alldirectorygids = par2script::work::get_all_gids_from_script($script, "Directory");

    for ( my $i = 0; $i <= $#{$script}; $i++ )
    {
        my $oneline = ${$script}[$i];

        if (( $oneline =~ /^\s*Module\s+\w+\s*$/ ) && ( $rootmodulestartline eq "" ))   # the first module is the root module
        {
            $rootmodulestartline = $i;
        }

        if ( $oneline =~ /^\s*(\w+?)\s*\=\s*\((.*?)\)\s*\;\s*$/ )
        {
            my $key = $1;
            my $allassigneditemsstring = $2;
            my $defineditemgids;

            if ( $key eq "Files" ) { $defineditemgids = $allfilegids; }
            elsif ( $key eq "Dirs" ) { $defineditemgids = $alldirectorygids; }
            elsif ( $key eq "Procedures" ) { $defineditemgids = $allproceduregids; }
            elsif ( $key eq "Customs" ) { $defineditemgids = $allcustomgids; }
            else { next; }  # for instance "Styles"

            my $allassigneditems = par2script::converter::convert_stringlist_into_array(\$allassigneditemsstring, ",");

            for ( my $j = 0; $j <= $#{$allassigneditems}; $j++ )
            {
                my $oneassigneditem = ${$allassigneditems}[$j];

                # this can be removed for the list for the root module

                for ( my $k = 0; $k <= $#{$defineditemgids}; $k++ )
                {
                    if ( $oneassigneditem eq ${$defineditemgids}[$k] )
                    {
                        splice(@{$defineditemgids}, $k, 1);
                        last;
                    }
                }
            }
        }
    }

    # Now the four collectors contain only gids, that have to be added to the root module
    # The module begins at $rootmodulestartline

    for ( my $i = $rootmodulestartline; $i <= $#{$script}; $i++ )
    {
        my $oneline = ${$script}[$i];

        if ( $oneline =~ /^\s*End\s*$/ ) { last; }

        if ( $oneline =~ /^\s*(\w+)\s+\=\s+\((.*)\)\s*\;\s*$/ )
        {
            my $key = $1;
            my $allassigneditemsstring = $2;
            my $defineditemgids;

            if ( $key eq "Files" ) { $defineditemgids = $allfilegids; }
            elsif ( $key eq "Dirs" ) { $defineditemgids = $alldirectorygids; }
            elsif ( $key eq "Procedures" ) { $defineditemgids = $allproceduregids; }
            elsif ( $key eq "Customs" ) { $defineditemgids = $allcustomgids; }
            else { next; }  # for instance "Styles"

            my $allassigneditems = par2script::converter::convert_stringlist_into_array(\$allassigneditemsstring, "\,");

            # adding the root module items to the collectors

            for ( my $j = 0; $j <= $#{$allassigneditems}; $j++ )
            {
                push(@{$defineditemgids}, ${$allassigneditems}[$j]);
            }

            # then the existing lines can be removed

            splice(@{$script}, $i, 1);
            $i--;       # decreasing the counter!
        }
    }

    # creation of the new block

    my @rootmodule = ();
    my $rootmoduleref = \@rootmodule;

    create_rootmodule($rootmoduleref, $alldirectorygids, "Dirs");
    create_rootmodule($rootmoduleref, $allfilegids, "Files");
    create_rootmodule($rootmoduleref, $allproceduregids, "Procedures");
    create_rootmodule($rootmoduleref, $allcustomgids, "Customs");

    # and finally the new blocks can be inserted into the root module

    my $insertline;

    for ( my $i = $rootmodulestartline; $i <= $#{$script}; $i++ )
    {
        if ( ${$script}[$i] =~ /^\s*End\s*$/i )
        {
            $insertline = $i;
            last;
        }
    }

    splice(@{$script}, $insertline, 0, @{$rootmoduleref});

}

1;
