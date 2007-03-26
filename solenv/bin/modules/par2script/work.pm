#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: work.pm,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: ihi $ $Date: 2007-03-26 12:46:22 $
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


package par2script::work;

use par2script::existence;
use par2script::globals;
use par2script::remover;

############################################
# par2script working module
############################################

sub analyze_comma_separated_list
{
    my ($list, $listref) = @_;  # second parameter is optional

    my @list = ();
    my $locallistref;

    if (!( $listref )) { $locallistref = \@list; }
    else { $locallistref = $listref; }

    par2script::remover::remove_leading_and_ending_comma(\$list);
    par2script::remover::remove_leading_and_ending_whitespaces(\$list);

    while ( $list =~ /^\s*(.*?)\s*\,\s*(.*)\s*$/ )
    {
        my $oneentry = $1;
        $list = $2;
        par2script::remover::remove_leading_and_ending_whitespaces(\$oneentry);
        push(@{$locallistref}, $oneentry);
    }

    # the last entry

    par2script::remover::remove_leading_and_ending_whitespaces(\$list);
    push(@{$locallistref}, $list);

    return $locallistref;
}

############################################
# setting list of include pathes
############################################

sub setincludes
{
    my ($list) = @_;

    # input is the comma separated list of include pathes

    my $includes = analyze_comma_separated_list($list);

    return $includes;
}

############################################
# setting list of all par files
############################################

sub setparfiles
{
    my ($filename) = @_;

    # input is the name of the list file

    $filename =~ s/\@//;    # removing the leading \@

    my $filecontent = par2script::files::read_file($filename);

    my @parfiles = ();
    my $parfilesref = \@parfiles;

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        my $oneline = ${$filecontent}[$i];
        my $parfilesref = analyze_comma_separated_list($oneline, $parfilesref);
    }

    return \@parfiles;
}

############################################
# finding the correct include path
# for the par files
############################################

sub make_complete_pathes_for_parfiles
{
    my ($parfiles, $includes) = @_;

    for ( my $i = 0; $i <= $#{$parfiles}; $i++ )
    {
        my $foundparfile = 0;

        for ( my $j = 0; $j <= $#{$includes}; $j++ )
        {
            my $parfile = ${$includes}[$j] . $par2script::globals::separator . ${$parfiles}[$i];

            if ( -f $parfile )
            {
                $foundparfile = 1;
                ${$parfiles}[$i] = $parfile;
                last;
            }
        }

        if ( ! $foundparfile )
        {
            die "ERROR: Could not find parfile ${$parfiles}[$i] in includes pathes: $par2script::globals::includepathlist !\n";
        }
    }
}

######################################################
# collecting one special item in all par files and
# including it into the script file
######################################################

sub put_item_into_collector
{
    my ( $item, $parfile, $collector ) = @_;

    my $include = 0;

    for ( my $i = 0; $i <= $#{$parfile}; $i++ )
    {
        if ( ${$parfile}[$i] =~ /^\s*$item\s*\w+\s*$/ )
        {
            $include = 1;
        }

        if ( $include )
        {
            push(@{$collector}, ${$parfile}[$i]);
        }

        if (( $include ) && ( ${$parfile}[$i] =~ /^\s*End\s*$/i ))
        {
            $include = 0;
            push(@{$collector}, "\n");  # empty line at the end
        }
    }
}

######################################################
# putting all collected items of one type
# into the script file
######################################################

sub put_item_into_script
{
    my ($script, $itemcollector) = @_;

    for ( my $i = 0; $i <= $#{$itemcollector}; $i++ )
    {
        push(@{$script}, ${$itemcollector}[$i]);
    }
}

#######################################################################
# Collecting all gids of the type "searchitem" from the setup script
#######################################################################

sub get_all_gids_from_script
{
    my ($itemcollector, $oneitem) = @_;

    my @allgidarray = ();

    for ( my $i = 0; $i <= $#{$itemcollector}; $i++ )
    {
        if ( ${$itemcollector}[$i] =~ /^\s*\Q$oneitem\E\s+(\S+)\s*$/ )
        {
            my $gid = $1;
            push(@allgidarray, $gid);
        }
    }

    return \@allgidarray;
}

#######################################################################
# Collecting all items of the type "searchitem" from the setup script
#######################################################################

sub get_all_items_from_script
{
    my ($scriptref, $searchitem) = @_;

    my @allitemarray = ();

    my ($line, $gid, $counter, $itemkey, $itemvalue, $valuecounter);

    for ( my $i = 0; $i <= $#{$scriptref}; $i++ )
    {
        $line = ${$scriptref}[$i];

        if ( $line =~ /^\s*\Q$searchitem\E\s+(\S+)\s*$/ )
        {
            $gid = $1;
            $counter = $i + 1;

            my %oneitemhash = ();
            $oneitemhash{'gid'} = $gid;

            while  (!( $line =~ /^\s*End\s*$/ ))
            {
                $line = ${$scriptref}[$counter];
                $counter++;

                if ( $line =~ /^\s*(.+?)\s*\=\s*(.+?)\s*\;\s*$/ )   # only oneliner!
                {
                    $itemkey = $1;
                    $itemvalue = $2;

                    par2script::remover::remove_leading_and_ending_quotationmarks(\$itemvalue);

                    $oneitemhash{$itemkey} = $itemvalue;

                }
            }

            push(@allitemarray, \%oneitemhash);
        }
    }

    return \@allitemarray;
}

########################################################
# Recursively defined procedure to order
# modules and directories
########################################################

sub get_children
{
    my ($allitems, $startparent, $newitemorder) = @_;

    for ( my $i = 0; $i <= $#{$allitems}; $i++ )
    {
        my $gid = ${$allitems}[$i]->{'gid'};
        my $parent = "";
        if ( ${$allitems}[$i]->{'ParentID'} ) { $parent = ${$allitems}[$i]->{'ParentID'}; }

        if ( $parent eq $startparent )
        {
            push(@{$newitemorder}, $gid);
            my $parent = $gid;
            get_children($allitems, $parent, $newitemorder);    # recursive!
        }
    }
}

########################################################
# Module and Directory have to be in the correct order
# in the setup script. This is an requirement by the
# old setup
########################################################

sub create_treestructure
{
    my ($item, $allitems) = @_;

    my @itemorder = ();

    my @startparents = ();

    if ( $item eq "Module" ) { push(@startparents, ""); }
    if ( $item eq "Directory" )
    {
        push(@startparents, "PREDEFINED_PROGDIR");
        push(@startparents, "PREDEFINED_KDEHOME");
        push(@startparents, "PREDEFINED_HOMEDIR");
    }

    for ( my $i = 0; $i <= $#startparents; $i++ )   # if there is more than one toplevel item
    {
        get_children($allitems, $startparents[$i], \@itemorder);
    }

    return \@itemorder;
}

########################################################
# Creating the item collector for Module and
# Directory in the new sorted order
########################################################

sub create_sorted_itemcollector
{
    my ($newitemorder, $collector, $oneitem) = @_;

    @newitemcollector = ();

    for ( my $i = 0; $i <= $#{$newitemorder}; $i++ )
    {
        my $gid = ${$newitemorder}[$i];

        for ( my $j = 0; $j <= $#{$collector}; $j++ )
        {
            my $line = ${$collector}[$j];

            if ( $line =~ /^\s*$oneitem\s+$gid\s*$/ )
            {
                $include = 1;
            }

            if ( $include )
            {
                push(@newitemcollector, $line);
            }

            if (($include) && ( $line =~ /^\s*End\s*$/i ))
            {
                $include = 0;
                push(@newitemcollector, "\n");  # empty line at the end
                last;
            }
        }
    }

    return \@newitemcollector;
}

############################################
# Every gid has to defined only once
# in the par files
############################################

sub test_of_gid_uniqueness
{
    my ($allitems) = @_;

    my @allgids = ();

    for ( my $i = 0; $i <= $#{$allitems}; $i++ )
    {
        my $gid = ${$allitems}[$i];

        if (! par2script::existence::exists_in_array($gid, \@allgids))
        {
            push(@allgids, $gid);
        }
        else
        {
            $par2script::globals::multidefinitionerror = 1;
            push(@par2script::globals::multidefinitiongids, $gid);
        }
    }
}

############################################
# gids must not cotain hyphens
############################################

sub test_of_hyphen
{
    my ($allitems) = @_;

    for ( my $i = 0; $i <= $#{$allitems}; $i++ )
    {
        my $gid = ${$allitems}[$i];

        if ( $gid =~ /\-/ )
        {
            par2script::exiter::exit_program("ERROR: No hyphen allowed in global id: $gid", "test_of_hyphen");
        }
    }
}

######################################################
# This function exists for compatibility reasons:
# In scp the string "DosName" is used, in the
# created script this is "HostName"
######################################################

sub convert_dosname_to_hostname
{
    my ($collector) = @_;

    for ( my $i = 0; $i <= $#{$collector}; $i++ )
    {
        ${$collector}[$i] =~ s/\bDosName\b/HostName/;
    }
}

###########################################################
# This function exists for compatibility reasons:
# In scp the string "PD_PROGDIR" is often used, in the
# created script this is "PREDEFINED_PROGDIR"
###########################################################

sub convert_pdprogdir_to_predefinedprogdir
{
    my ($collector) = @_;

    for ( my $i = 0; $i <= $#{$collector}; $i++ )
    {
        ${$collector}[$i] =~ s/\bPD_PROGDIR\b/PREDEFINED_PROGDIR/;
    }
}

######################################################
# Single styles are in scp sometimes defined as:
# "Styles = cfg_string;". This has to be replaced
# in the script to
# "Styles = (cfg_string);"
######################################################

sub setting_brackets_around_single_styles
{
    my ($collector) = @_;

    for ( my $i = 0; $i <= $#{$collector}; $i++ )
    {
        if ( ${$collector}[$i] =~ /^(\s*styles\s*\=\s*)(\w+)(\s*\;\s*)$/i )
        {
            my $start = $1;
            my $styles = $2;
            my $end = $3;

            my $newline = $start . "\(" . $styles . "\)" . $end;
            ${$collector}[$i] = $newline;
        }
    }
}

######################################################
# The scpzip and the setup require a script version
# in the Installation object. This has to be included
# for compatibility reasons. It will always be:
# "ScriptVersion = 100;"
######################################################

sub set_scriptversion_into_installation_object
{
    my ($collector) = @_;

    my $newline = "\tScriptVersion = 100\;\n";

    # determining the last line

    my $lastline;

    for ( my $i = 0; $i <= $#{$collector}; $i++ )
    {
        if ( ${$collector}[$i] =~ /^\s*End\s*$/i )
        {
            $lastline = $i;
            last;
        }
    }

    splice(@{$collector}, $lastline, 0, $newline);

}

############################################
# transferring the par file content
# into the script file
############################################

sub collect_all_items
{
    my ($parfile) = @_;

    my @setupscript = ();
    my $setupscript = \@setupscript;

    for ( my $i = 0; $i <= $#par2script::globals::allitems; $i++ )
    {
        my $oneitem = $par2script::globals::allitems[$i];

        my @itemcollector = ();
        my $itemcollector = \@itemcollector;

        put_item_into_collector($oneitem, $parfile, $itemcollector);

        # testing uniqueness of each gid

        my $allgids = get_all_gids_from_script($itemcollector, $oneitem);

        test_of_gid_uniqueness($allgids);

        test_of_hyphen($allgids);

        # renaming at directories "DosName" to "HostName" and "PD_PROGDIR" to "PREDEFINED_PROGDIR" (only for compatibility reasons)
        if ( $oneitem eq "Directory" ) { convert_dosname_to_hostname($itemcollector); }
        if (( $oneitem eq "Directory" ) || ( $oneitem eq "File" )) { convert_pdprogdir_to_predefinedprogdir($itemcollector); }

        # sorting directories and modules (modules are also oneliner after pre2par!)

        if (( $oneitem eq "Module" ) || ( $oneitem eq "Directory" ))
        {
            my $allitems = get_all_items_from_script($itemcollector, $oneitem);
            $newitemorder = create_treestructure($oneitem, $allitems);
            $itemcollector = create_sorted_itemcollector($newitemorder, $itemcollector, $oneitem);
        }

        # setting brackets around single styles: "styles = cfg_string;" -> "styles = (cfg_string);"

        setting_brackets_around_single_styles($itemcollector);

        # Installation objects need to get the script version (only for compatibility reasons)

        if ( $oneitem eq "Installation" ) { set_scriptversion_into_installation_object($itemcollector); }

        # putting the collector content into the setup script

        put_item_into_script($setupscript, $itemcollector);

    }

    if ( $par2script::globals::multidefinitionerror ) { par2script::exiter::multidefinitionerror(); }

    return $setupscript;
}

############################################
# Returning a complete definition block
# from the script
############################################

sub get_definitionblock_from_script
{
    my ($script, $gid) = @_;

    my @codeblock = ();
    my $startline = -1;

    for ( my $i = 0; $i <= $#{$script}; $i++ )
    {
        if ( ${$script}[$i] =~ /^\s*\w+\s+$gid\s*$/ )
        {
            $startline = $i;
            last;
        }
    }

    if ( $startline != -1 )
    {
        while (! ( ${$script}[$startline] =~ /^\s*End\s*$/i ) )
        {
            push(@codeblock, ${$script}[$startline]);
            $startline++;
        }

        push(@codeblock, ${$script}[$startline]);
    }

    return \@codeblock;
}

############################################
# Adding a complete definition block
# into the script
############################################

sub add_definitionblock_into_script
{
    my ($script, $newblock, $gid) = @_;

    # adding the new block behind the block defined by $gid

    my $insertline = -1;
    my $count = 0;

    for ( my $i = 0; $i <= $#{$script}; $i++ )
    {
        if ( ${$script}[$i] =~ /^\s*\w+\s+$gid\s*$/ )
        {
            $count = 1;
        }

        if (( $count ) && ( ${$script}[$i] =~ /^\s*End\s*$/i ))
        {
            $insertline = $i;
            last;
        }
    }

    if ( $insertline != -1 )
    {
        $insertline = $insertline + 2;
        # inserting an empty line at the end of the block if required
        if (!(${$newblock}[$#{$newblock}] =~ /^\s*$/)) { push(@{$newblock}, "\n"); }
        # inserting the new block
        splice( @{$script}, $insertline, 0, @{$newblock} );
    }
    else
    {
        die "ERROR: Could not include definition block. Found no definition of $gid!\n";
    }

}

############################################
# Removing a complete definition block
# from the script
############################################

sub remove_definitionblock_from_script
{
    my ($script, $gid) = @_;

    my $startline = -1;

    for ( my $i = 0; $i <= $#{$script}; $i++ )
    {
        if ( ${$script}[$i] =~ /^\s*\w+\s+$gid\s*$/i )
        {
            $startline = $i;
            last;
        }
    }

    if ( $startline != -1 )
    {
        my $endline = $startline;

        while (! ( ${$script}[$endline] =~ /^\s*End\s*$/i ) )
        {
            $endline++;
        }

        my $blocklength = $endline - $startline + 2;    # "+2" because of endline and emptyline
        splice(@{$script}, $startline, $blocklength);
    }
}

############################################
# Returning the value for a given key
# from a definition block
############################################

sub get_value_from_definitionblock
{
    my ($block, $key) = @_;

    my $value = "";

    for ( my $i = 0; $i <= $#{$block}; $i++ )
    {
        if ( ${$block}[$i] =~ /^\s*$key\s*\=\s*(.*?)\s*$/ )
        {
            $value = $1;
            last;
        }
    }

    par2script::remover::remove_leading_and_ending_whitespaces(\$value);
    $value =~ s/\;\s*$//;   # removing ending semicolons

    return $value;
}

##################################################
# Collecting the content of all par files.
# Then the files do not need to be opened twice.
##################################################

sub read_all_parfiles
{
    my ($parfiles) = @_;

    my @parfilecontent = ();

    for ( my $i = 0; $i <= $#{$parfiles}; $i++ )
    {
        my $parfile = par2script::files::read_file(${$parfiles}[$i]);
        add_array_into_array(\@parfilecontent, $parfile);
        push(@parfilecontent, "\n");
    }

    return \@parfilecontent;
}

##########################################################
# Add the content of an array to another array
##########################################################

sub add_array_into_array
{
    my ($basearray, $newarray) = @_;

    for ( my $i = 0; $i <= $#{$newarray}; $i++ )
    {
        push(@{$basearray}, ${$newarray}[$i]);
    }
}

##########################################################
# Collecting all subdirectories of a specified directory
##########################################################

sub collect_subdirectories
{
    my ($parfile, $directorygid, $collector) = @_;

    my $isdirectory = 0;
    my $currentgid = "";

    for ( my $i = 0; $i <= $#{$parfile}; $i++ )
    {
        my $oneline = ${$parfile}[$i];

        if ( $oneline =~ /^\s*Directory\s+(\w+)\s*$/ )
        {
            $currentgid = $1;
            $isdirectory = 1;
        }

        if (( $isdirectory ) && ( $oneline =~ /^\s*End\s*$/ )) { $isdirectory = 0; }

        if ( $isdirectory )
        {
            if ( $oneline =~ /^\s*ParentID\s*=\s*(\w+)\s*\;\s*$/ )
            {
                my $parentgid = $1;

                if ( $parentgid eq $directorygid )
                {
                    # Found a child of the directory, that shall be removed

                    my %removeitem = ();
                    my $item = "Directory";
                    $removeitem{'gid'} = $currentgid;
                    $removeitem{'item'} = $item;

                    push(@{$collector}, \%removeitem);

                    # recursively checking additional children
                    collect_subdirectories($parfile, $currentgid, $collector);
                }
            }
        }
    }
}

##########################################################
# Collecting all items (Files and Shortcuts), that
# are located in the list of directories.
##########################################################

sub get_all_items_in_directories
{
    my ($parfile, $directorygid, $item, $collector) = @_;

    my $isitem = 0;
    my $currentgid = "";

    for ( my $i = 0; $i <= $#{$parfile}; $i++ )
    {
        my $oneline = ${$parfile}[$i];

        if ( $oneline =~ /^\s*\Q$item\E\s+(\w+)\s*$/ )
        {
            $currentgid = $1;
            $isitem = 1;
        }

        if (( $isitem ) && ( $oneline =~ /^\s*End\s*$/ )) { $isitem = 0; }

        if ( $isitem )
        {
            if ( $oneline =~ /^\s*Dir\s*=\s*(\w+)\s*\;\s*$/ )
            {
                my $installdir = $1;

                if ( $installdir eq $directorygid )
                {
                    # Found an item, that shall be installed in the specific directory

                    my %removeitem = ();
                    $removeitem{'gid'} = $currentgid;
                    $removeitem{'item'} = $item;

                    push(@{$collector}, \%removeitem);
                }
            }
        }
    }
}

##########################################################
# Collecting all items (ProfileItems), that
# are located in the list of Profiles.
##########################################################

sub get_all_items_in_profile
{
    my ($parfile, $profilegid, $item, $collector) = @_;

    my $isitem = 0;
    my $currentgid = "";

    for ( my $i = 0; $i <= $#{$parfile}; $i++ )
    {
        my $oneline = ${$parfile}[$i];

        if ( $oneline =~ /^\s*\Q$item\E\s+(\w+)\s*$/ )
        {
            $currentgid = $1;
            $isitem = 1;
        }

        if (( $isitem ) && ( $oneline =~ /^\s*End\s*$/ )) { $isitem = 0; }

        if ( $isitem )
        {
            if ( $oneline =~ /^\s*ProfileID\s*=\s*(\w+)\s*\;\s*$/ )
            {
                my $profilename = $1;

                if ( $profilename eq $profilegid )
                {
                    # Found an item, that shall be installed in the specific directory

                    my %removeitem = ();
                    $removeitem{'gid'} = $currentgid;
                    $removeitem{'item'} = $item;

                    push(@{$collector}, \%removeitem);
                }
            }
        }
    }
}

1;
