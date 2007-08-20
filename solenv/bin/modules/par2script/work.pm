#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: work.pm,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: ihi $ $Date: 2007-08-20 15:29:46 $
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

    foreach ( @{$filecontent} ) { $parfilesref = analyze_comma_separated_list($_, $parfilesref); }

    return $parfilesref;
}

############################################
# finding the correct include path
# for the par files
############################################

sub make_complete_pathes_for_parfiles
{
    my ($parfiles, $includes) = @_;

    my $oneparfile;

    foreach $oneparfile ( @{$parfiles} )
    {
        my $foundparfile = 0;
        my $includepath;

        foreach $includepath ( @{$includes} )
        {
            my $parfile = "$includepath/$oneparfile";

            if ( -f $parfile )
            {
                $foundparfile = 1;
                $oneparfile = $parfile;
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
# collecting one special item in the par files and
# including it into the "definitions" hash
######################################################

sub collect_definitions
{
    my ($parfilecontent) = @_;

    my $multidefinitionerror = 0;
    my @multidefinitiongids = ();


    foreach $oneitem ( @par2script::globals::allitems )
    {
        my $docollect = 0;
        my $gid = "";
        my %allitemhash = ();

        for ( my $i = 0; $i <= $#{$parfilecontent}; $i++ )
        {
            my $line = ${$parfilecontent}[$i];

            if ( $line =~ /^\s*$oneitem\s+(\w+)\s*$/ )
            {
                $gid = $1;
                $docollect = 1;
            }
            else
            {
                $docollect = 0;
            }

            if ( $docollect )
            {
                my $currentline = $i;
                my %oneitemhash;

                while (! ( ${$parfilecontent}[$currentline] =~ /^\s*End\s*$/i ) )
                {
                    if ( ${$parfilecontent}[$currentline] =~ /^\s*(.+?)\s*\=\s*(.+?)\s*\;\s*$/ )    # only oneliner!
                    {
                        $itemkey = $1;
                        $itemvalue = $2;

                        if ( $oneitem eq "Directory" ) { if ( $itemkey =~ "DosName" ) { $itemkey =~ s/DosName/HostName/; } }
                        if (( $oneitem eq "Directory" ) || ( $oneitem eq "File" ) || ( $oneitem eq "Unixlink" )) { if ( $itemvalue eq "PD_PROGDIR" ) { $itemvalue = "PREDEFINED_PROGDIR"; }}
                        if (( $itemkey eq "Styles" ) && ( $itemvalue =~ /^\s*(\w+)(\s*\;\s*)$/ )) { $itemvalue = "($1)$2"; }

                        $oneitemhash{$itemkey} = $itemvalue;
                    }

                    $currentline++;
                }

                # no hyphen allowed in gids -> cannot happen here because (\w+) is required for gids
                if ( $gid =~ /-/ ) { par2script::exiter::exit_program("ERROR: No hyphen allowed in global id: $gid", "test_of_hyphen"); }

                # test of uniqueness
                if ( exists($allitemhash{$gid}) )
                {
                    $multidefinitionerror = 1;
                    push(@multidefinitiongids, $gid);
                }

                $allitemhash{$gid} = \%oneitemhash;
            }
        }

        $par2script::globals::definitions{$oneitem} = \%allitemhash;
    }

    if ( $multidefinitionerror ) {  par2script::exiter::multidefinitionerror(\@multidefinitiongids); }

    # foreach $key (keys %par2script::globals::definitions)
    # {
    #   print "Key: $key \n";
    #
    #   foreach $key (keys %{$par2script::globals::definitions{$key}})
    #   {
    #       print "\t$key \n";
    #   }
    # }
}

######################################################
# Filling content into the script
######################################################

sub put_oneitem_into_script
{
    my ( $script, $item, $itemhash, $itemkey ) = @_;

    push(@{$script}, "$item $itemkey\n" );
    my $content = "";
    foreach $content (sort keys %{$itemhash->{$itemkey}}) { push(@{$script}, "\t$content = $itemhash->{$itemkey}->{$content};\n" ); }
    push(@{$script}, "End\n" );
    push(@{$script}, "\n" );
}

######################################################
# Creating the script
######################################################

sub create_script
{
    my @script = ();
    my $oneitem;

    foreach $oneitem ( @par2script::globals::allitems )
    {
        if ( exists($par2script::globals::definitions{$oneitem}) )
        {
            if ( $oneitem eq "Shortcut" ) { next; } # "Shortcuts" after "Files"

            if (( $oneitem eq "Module" ) || ( $oneitem eq "Directory" )) { write_sorted_items(\@script, $oneitem); }
            else { write_unsorted_items(\@script, $oneitem); }
        }
    }

    return \@script;
}

######################################################
# Adding script content for the unsorted items
######################################################

sub write_unsorted_items
{
    my ( $script, $oneitem ) = @_;

    my $itemhash = $par2script::globals::definitions{$oneitem};

    my $itemkey = "";
    foreach $itemkey (sort keys %{$itemhash})
    {
        put_oneitem_into_script($script, $oneitem, $itemhash, $itemkey);

        # special handling for Shortcuts after Files
        if (( $oneitem eq "File" ) && ( exists($par2script::globals::definitions{"Shortcut"}) ))
        {
            my $shortcutkey;
            foreach $shortcutkey ( keys %{$par2script::globals::definitions{"Shortcut"}} )
            {
                if ( $par2script::globals::definitions{"Shortcut"}->{$shortcutkey}->{'FileID'} eq $itemkey )
                {
                    put_oneitem_into_script($script, "Shortcut", $par2script::globals::definitions{"Shortcut"}, $shortcutkey);

                    # and Shortcut to Shortcut also
                    my $internshortcutkey;
                    foreach $internshortcutkey ( keys %{$par2script::globals::definitions{"Shortcut"}} )
                    {
                        if ( $par2script::globals::definitions{"Shortcut"}->{$internshortcutkey}->{'ShortcutID'} eq $shortcutkey )
                        {
                            put_oneitem_into_script($script, "Shortcut", $par2script::globals::definitions{"Shortcut"}, $internshortcutkey);
                        }
                    }
                }
            }
        }
    }
}

######################################################
# Collecting all children of a specified parent
######################################################

sub collect_children
{
    my ( $itemhash, $parent, $order ) = @_;

    my $item;
    foreach $item ( keys %{$itemhash} )
    {
        if ( $itemhash->{$item}->{'ParentID'} eq $parent )
        {
            push(@{$order}, $item);
            my $newparent = $item;
            collect_children($itemhash, $newparent, $order);
        }
    }
}

######################################################
# Adding script content for the sorted items
######################################################

sub write_sorted_items
{
    my ( $script, $oneitem ) = @_;

    my $itemhash = $par2script::globals::definitions{$oneitem};

    my @itemorder = ();
    my @startparents = ();

    if ( $oneitem eq "Module" ) { push(@startparents, ""); }
    elsif ( $oneitem eq "Directory" ) { push(@startparents, "PREDEFINED_PROGDIR"); }
    else { die "ERROR: No root parent defined for item type $oneitem !\n"; }

    # supporting more than one toplevel item
    my $parent;
    foreach $parent ( @startparents ) { collect_children($itemhash, $parent, \@itemorder); }

    my $itemkey;
    foreach $itemkey ( @itemorder ) { put_oneitem_into_script($script, $oneitem, $itemhash, $itemkey); }
}

#######################################################################
# Collecting all assigned gids of the type "item" from the modules
# in the par files. Using a hash!
#######################################################################

sub collect_assigned_gids
{
    my $allmodules = $par2script::globals::definitions{'Module'};

    my $item;
    foreach $item ( @par2script::globals::items_assigned_at_modules )
    {
        if ( ! exists($par2script::globals::searchkeys{$item}) ) { par2script::exiter::exit_program("ERROR: Unknown type \"$item\" at modules.", "collect_assigned_gids"); }

        my $searchkey = $par2script::globals::searchkeys{$item};

        my %assignitems = ();
        my $modulegid = "";

        foreach $modulegid (keys %{$allmodules} )
        {
            # print "Module $modulegid\n";
            # my $content = "";
            # foreach $content (sort keys %{$allmodules->{$modulegid}}) { print "\t$content = $allmodules->{$modulegid}->{$content};\n"; }
            # print "End\n";
            # print "\n";

            if ( exists($allmodules->{$modulegid}->{$searchkey}) )
            {
                my $list = $allmodules->{$modulegid}->{$searchkey};
                if ( $list =~ /^\s*\((.*?)\)\s*(.*?)\s*$/ ) { $list = $1; }
                else { par2script::exiter::exit_program("ERROR: Invalid module list: $list", "collect_assigned_gids"); }
                my $allassigneditems = par2script::converter::convert_stringlist_into_array_2($list, ",");

                my $gid;
                foreach $gid ( @{$allassigneditems} )
                {
                    if ( exists($assignitems{$gid}) ) { $assignitems{$gid} = $assignitems{$gid} + 1; }
                    else { $assignitems{$gid} = 1; }
                }
            }
        }

        $par2script::globals::assignedgids{$item} = \%assignitems;
    }
}

##################################################
# Collecting the content of all par files.
# Then the files do not need to be opened twice.
##################################################

sub read_all_parfiles
{
    my ($parfiles) = @_;

    my @parfilecontent = ();
    my $parfilename;

    foreach $parfilename ( @{$parfiles} )
    {
        my $parfile = par2script::files::read_file($parfilename);
        foreach ( @{$parfile} ) { push(@parfilecontent, $_); }
        push(@parfilecontent, "\n");
    }

    return \@parfilecontent;
}

1;
