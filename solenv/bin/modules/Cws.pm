#*************************************************************************
#
#   $RCSfile: Cws.pm,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2004-08-09 13:34:24 $
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


#
# Cws.pm - package for accessing/manipulating child workspaces
#

# TODO: needs some cleanup

package Cws;
use strict;

use Eis;
use CwsConfig;
use Carp;
use URI::Escape;

my $config = CwsConfig::get_config();

##### class data #####

my %CwsClassData = (
    # EIS database connectivity
    EIS_URI        => 'urn:ChildWorkspaceDataService',
    EIS_PROXY_LIST => $config->cws_db_url_list_ref(),
    NET_PROXY      => $config->net_proxy(),
    EIS            => undef
);

##### ctor #####

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    # instance data
    # initialize CWS name from environment
    $self->{CHILD}       = undef;    # name of child workspace
    $self->{MASTER}      = undef;    # name of master workspace
    $self->{EIS_ID}      = undef;    # id of child workspace in EIS
    $self->{FILES}       = undef;    # list of files registered with child
                                     # any file can be registered multiple times
    $self->{PATCH_FILES} = undef     # list of product patch files registered with
                                     # child, each file can be added only once
    $self->{MILESTONE}   = undef;    # master milestone to which child is related
    $self->{MODULES}     = undef;    # list of modules belonging to child
    $self->{TASKIDS}     = undef;    # list of tasks registered with child
    bless($self, $class);
    return $self;
}

#### methods to access instance data ####

# Get the EIS ID for child workspace,
# return value: undef => not yet asked EIS for ID
#                        or connection failed
#               0     => queried EIS but didn't find such
#                        a child workspace for this master
# silently ignore any parameter, only the EIS database,
# hands out EIS IDs.
sub eis_id
{
    my $self = shift;
    if ( !defined($self->{EIS_ID} ) ) {
            $self->{EIS_ID} = $self->get_eis_id();
    }
    return $self->{EIS_ID};
}

# Generate remaining instance data accessor methods;
# if this looks strange see 'perldoc perltootc'

# Accessor methods for single value instance data
for my $datum (qw(master milestone)) {
    no strict "refs";
    *$datum = sub {
        my $self = shift;
        my $ucdatum = uc($datum);
        if ( @_ ) {
            # set item in database
            my $item = shift;
            # if we already have a valid EIS registered CWS then reset EIS value
            # otherwise just set member to the given value
            if ( !$self->{uc($datum)} # keep order of evaluation
                || !$self->eis_id()
                || $self->set_item_in_eis($datum, $item) )
            {
               $self->{uc($datum)} = $item;

            }
        }
        else {
            if ( !defined($self->{$ucdatum} ) ) {
                # fetch item from database
                $self->{$ucdatum} = $self->fetch_item_from_eis($datum);
            }
        }
        return $self->{uc($datum)};
    }
}

# Accessor methods for instance data consisting of item lists
# like modules and taskids
for my $datum (qw(files patch_files modules taskids)) {
    no strict "refs";
    *$datum = sub {
        # get current item list
        # fetch list from EIS database if called the first time
        my $self = shift;
        my $ucdatum = uc($datum);
        if ( !defined($self->{$ucdatum}) ) {
            # fetch item list from databse
            $self->{$ucdatum} = $self->fetch_items_from_eis($datum);
            return undef if !defined($self->{$ucdatum});
        }
        return wantarray ? @{$self->{$ucdatum}} : $self->{$ucdatum}
    }
}

for my $datum (qw(child)) {
    no strict "refs";
    *$datum = sub {
        my $self = shift;
        $self->{uc($datum)} = shift if @_;
        return $self->{uc($datum)};
    }
}


#### additional public methods ####

# Query if CWS name is still available. Does not yet register
# anything with EIS.
sub is_cws_name_available
{
    my $self     = shift;

    my $is_available = $self->is_cws_name_available_in_eis();
    return $is_available;
}

# Register new child workspace with the EIS database.
sub register
{
    my $self     = shift;
    my $vcsid    = shift;
    my $location = shift;

    my $child_id = $self->register_child_with_eis($vcsid, $location);
    return $child_id;
}

# Promote a child workspace with status 'planned' to a full CWS
sub promote
{
    my $self     = shift;
    my $vcsid    = shift;
    my $location = shift;

    my $rc = $self->promote_child_in_eis($vcsid, $location);
    return $rc;
}

# New style add_module method. Takes an additional bool indicating if
# a module is public or private. Obsoletes add_modules()
sub add_module
{
    my $self   = shift;
    my $module = shift;
    my $public = shift;

    my $items_ref =  $self->add_items('modules', $public, $module);
    if (defined ($items_ref->[0]) &&  ($items_ref->[0] eq $module)) {
        return 1;  # module has been added
    }
    elsif ( defined($items_ref) ) {
        return 0;  # module was already add
    }
    return undef;  # something went wrong
}

# Add module to modules list.
sub add_modules
{
    my $self   = shift;

    my $items_ref =  $self->add_items('modules', undef, @_);
    return undef unless defined($items_ref);
    return wantarray ? @{$items_ref} : $items_ref;
}

# Add tasksids to taskids list.
sub add_taskids
{
    my $self   = shift;
    my $vcsid  = shift;

    my $items_ref = $self->add_items('taskids', $vcsid, @_);
    return undef unless defined($items_ref);
    return wantarray ? @{$items_ref} : $items_ref;
}

# Add a file to the files list.
sub add_file
{
    my $self         = shift;
    my $module       = shift;
    my $file         = shift;
    my $revision     = shift;
    my $authors_ref  = shift;
    my $taskids_ref  = shift;
    my $archive_path = shift;

    my $files_ref = $self->files();

    if ( $self->add_file_to_eis($module, $file, $revision,
            $authors_ref, $taskids_ref, $archive_path) )
    {
        push(@{$files_ref}, $file);
        return 1;
    }
    return 0;
}

# Add a file to the patch file list.
sub add_patch_file
{
    my $self         = shift;
    my $file         = shift;

    my $patch_files_ref = $self->patch_files();

    foreach (@{$patch_files_ref}) {
        return 0 if $file eq $_;
    }

    if ( $self->add_patch_file_to_eis($file) )
    {
        push(@{$patch_files_ref}, $file);
        return 1;
    }
    return 0;
}

#
# Procedure retrieves the stand which
# is based on cvs head (not branch)
#
sub get_cvs_head {
    my $eis = Cws::eis();
    my $result;
    eval { $result = $eis->getCVSHead() };
    if ( $@ ) {
        carp("ERROR: get_eis_id(): EIS database transaction failed. Reason:\n$@\n");
    }
    return $result;
};

sub get_master_tag {
    my ($self, $master, $milestone) = @_;
    $master = $self->master() if (!defined $master);
    $milestone = $self->milestone() if (!defined $milestone);
    return uc($master) . '_' . lc($milestone);
};

# Returns the branch and root tags for child workspace.
sub get_tags
{
    my $self = shift;

    # check if child workspace is valid
    my $id = $self->eis_id();
    if ( !$id ) {
        carp("ERROR: Childworkspace not (yet) registered with EIS.\n");
        return undef;
    }

    # check in environment if master is on the the HEAD branch
    my $cvs_head = get_cvs_head();
    my $current_master = $self->master();
    my $creation_master = $self->get_creation_master();
    if ( !$creation_master ) {
        carp("ERROR: Can't determine creation MWS.\n");
        return undef;
    }
    my $milestone = $self->milestone();

    my $master_branch_tag
        = (lc($current_master) eq lc($cvs_head)) ? '' : 'mws_' . lc($current_master);
    my $cws_branch_tag = 'cws_' . lc($creation_master) . '_' . lc($self->child());
    my $cws_root_tag   = uc($cws_branch_tag) . "_ANCHOR";
    my $master_milestone_tag = uc($current_master) . "_" . $milestone;

    return ($master_branch_tag, $cws_branch_tag, $cws_root_tag, $master_milestone_tag );
}

# Get child workspace approval status,
# return values can be:
# 'planned', 'new', 'nominated', 'integrated'
# and undef in case of error.
sub get_approval
{
    my $self = shift;

    return $self->get_status_from_eis();
}

# Set child workspace approval status
# to 'integrated'. Return true if successful
# or undef in case of error
sub set_integrated
{
    my $self = shift;

    return $self->set_status_in_eis();
}

# Set child workspace integration milestone
# Return true if successful or undef in case of error
sub set_integration_milestone
{
    my $self      = shift;
    my $milestone = shift;
    my $buildid   = shift;

    return $self->set_integration_milestone_in_eis($milestone, $buildid);
}

# Get the MWS on which a CWS was created
sub get_creation_master
{
    my $self = shift;

    return $self->get_creation_master_from_eis();
}

# Get the 'public' flag indicating whether a CWS is visible on OOo
sub get_public_flag
{
    my $self = shift;

    return $self->get_public_flag_from_eis();
}

#### public class methods ####

# Query master milestone combination for being used by an
# active CWS
sub is_milestone_used
{
    my $self      = shift;
    my $master    = shift;
    my $milestone = shift;

    return $self->get_is_milestone_used_from_eis($master, $milestone);
}

# Set current milestone for MWS.
sub set_current_milestone
{
    my $self      = shift;
    my $master    = shift;
    my $milestone = shift;

    return $self->set_current_milestone_in_eis($master, $milestone);
}

# Get current milestone for MWS.
sub get_current_milestone
{
    my $self      = shift;
    my $master    = shift;

    return $self->get_current_milestone_from_eis($master);
}

# Get all child workspaces which have been integrated on a
# given master and milestone.
sub get_integrated_cws
{
    my $self      = shift;
    my $master    = shift;
    my $milestone = shift;

    return wantarray ? @{$self->get_childworkspaces_for_milestone($master, $milestone)}
                     :   $self->get_childworkspaces_for_milestone($master, $milestone);
}

sub set_log_entry
{
    my $self        = shift;
    my $commandline = shift;
    my $vcsid       = shift;
    my $start       = shift;
    my $stop        = shift;
    my $comment     = shift;
    return $self->set_log_entry_in_eis($commandline, $vcsid, $start, $stop, $comment);
}

sub set_log_entry_extended
{
    my $self        = shift;
    my $commandname = shift;
    my $parameter   = shift;
    my $vcsid       = shift;
    my $start       = shift;
    my $stop        = shift;
    my $comment     = shift;
    my $mastername  = shift;
    my $childname   = shift;
#set_log_entry_extended_in_eis($commandname, $parameter, $vcsid, $start, $stop, $comment, $mastername, $childname);
    return $self->set_log_entry_extended_in_eis($commandname, $parameter, $vcsid, $start, $stop, $comment, $mastername, $childname);
}


#### private ####

# class data accessor methods
sub eis
{
    shift; # ignore calling class/object
    $CwsClassData{EIS} = shift if @_;
    if ( !defined($CwsClassData{EIS}) ) {
        $CwsClassData{EIS} = init_eis_connector();
    }
    return $CwsClassData{EIS};
}

# generate remaining class data accessor methods
# if this looks strange see 'perldoc perltootc'
for my $datum (qw(eis_uri eis_proxy_list net_proxy)) {
    no strict "refs";
    *$datum = sub {
        shift; # ignore calling class/object
        return $CwsClassData{uc($datum)};
    }
}

#### helper methods ####

# instance methods

# Add item to items list,
# update eis database,
# returns a list of newly added items,
# specifying an existing item is not an
# error, but it want appear in the return list.
sub add_items
{
    my $self          = shift;
    my $type          = shift;
    my $optional_data = shift;

    my $items_ref;
    if ( $type eq 'modules' ) {
        $items_ref   = $self->modules();
    }
    elsif ( $type eq 'taskids' ) {
        $items_ref   = $self->taskids();
    }
    else {
        # fall through, can't happen
        carp("ERROR: wrong item type\n");
        return undef;
    }

    my $item;
    my @new_items = ();
    return undef if !defined($items_ref);
    # find which items which are not already in items list
    ITEM: while ( $item = shift ) {
        foreach ( @{$items_ref} ) {
            next ITEM if $_ eq $item;
        }
        push(@new_items, $item);
    }
    if ( $#new_items > -1 ) {
        # add items to database
        if ( $self->add_items_to_eis($type, $optional_data, \@new_items) ) {
            push(@{$items_ref}, @new_items);
        }
        else {
            # something went wrong
            return undef;
        }
    }
    return \@new_items;
}

# Get EIS id for workspace from EIS database
sub get_eis_id
{
    my $self = shift;
    my $eis = Cws::eis();

    # It's not an error if one of these is unset, so don't carp().
    if ( !$self->master() || !$self->child() ) {
        return undef;
    }

    my $master = Eis::to_string($self->master());
    my $child  = Eis::to_string($self->child());

    my $result;
    eval { $result = $eis->getChildWorkspaceId($master, $child) };
    if ( $@ ) {
        carp("ERROR: get_eis_id(): EIS database transaction failed. Reason:\n$@\n");
    }
    return $result;
}

sub fetch_item_from_eis
{
    my $self = shift;
    my $type = shift;

    my $eis = Cws::eis();
    my $id = $self->eis_id();

    if ( !$id ) {
        carp("ERROR: Childworkspace not (yet) registered with EIS.\n");
        return undef;
    }

    my $result;
    if ( $type eq 'milestone' ) {
        eval { $result = $eis->getMilestone($id) };
    }
    elsif ( $type eq 'master' ) {
        # master can't be queried from the EIS database,
        # just return what already in member
        return $self->{MASTER}
    }
    else {
        # fall through, can't happen
        carp("ERROR: wrong item type\n");
        return undef;
    }
    if ( $@ ) {
        carp("ERROR: fetch_item(): EIS database transaction failed. Reason:\n$@\n");
    }
    return $result;
}

sub set_item_in_eis
{
    my $self     = shift;
    my $type     = shift;
    my $item     = shift;

    my $eis = Cws::eis();
    my $id = $self->eis_id();

    if ( !$id ) {
        carp("ERROR: Childworkspace not (yet) registered with EIS.\n");
        return undef;
    }

    # make certain that the item is a string, otherwise
    # autotyping will occasionally choose the wrong type
    $item = Eis::to_string($item);

    my $result;
    if ( $type eq 'milestone' ) {
        eval { $result = $eis->setMilestone($id, $item) };
    }
    elsif ( $type eq 'master' ) {
        eval { $result = $eis->setMasterWorkspace($id, $item) };
    }
    else {
        # fall through, can't happen
        carp("ERROR: wrong item type\n");
        return 0;
    }

    if ( $@ ) {
        carp("ERROR: set_item(): EIS database transaction failed. Reason:\n$@\n");
        return undef;
    }
    return 1 if $result;
    return 0;
}

sub fetch_items_from_eis
{
    my $self = shift;
    my $type = shift;

    my $eis = Cws::eis();
    my $id = $self->eis_id();

    if ( !$id ) {
        carp("ERROR: Childworkspace not (yet) registered with EIS.\n");
        return undef;
    }

    my $result;
    if ( $type eq 'modules' ) {
        eval { $result = $eis->getModules($id) };
    }
    elsif ( $type eq 'taskids' ) {
        eval { $result = $eis->getTaskIds($id) };
    }
    elsif ( $type eq 'files' ) {
        eval { $result = $eis->getFiles($id) };
    }
    elsif ( $type eq 'patch_files' ) {
        eval { $result = $eis->getOutputFiles($id) };
    }
    else {
        # fall through, can't happen
        carp("ERROR: wrong item type\n");
        return undef;
    }
    if ( $@ ) {
        carp("ERROR: fetch_item(): EIS database transaction failed. Reason:\n$@\n");
    }
    return $result;
}

sub add_items_to_eis
{
    my $self          = shift;
    my $type          = shift;
    my $optional_data = shift;
    my $item_ref      = shift;

    my $eis = Cws::eis();
    my $id = $self->eis_id();

    if ( !$id ) {
        carp("ERROR: Childworkspace not (yet) registered with EIS.\n");
        return undef;
    }

    # make certain that all items are strings, otherwise
    # autotyping will occasionally choose the wrong type
    my @items = ();
    foreach ( @{$item_ref} ) {
        push(@items, Eis::to_string($_));
    }

    my $result;
    if ( $type eq 'modules' ) {
        if ( defined($optional_data) ) {
            # add a module new style, with public attribute
            eval { $result = $eis->addModule($id, $items[0], $optional_data) };
        }
        else {
            # old style, add a list of modules
            eval { $result = $eis->addModules($id, \@items) };
        }
    }
    elsif ( $type eq 'taskids' ) {
        eval { $result = $eis->addTaskIds($id, \@items, $optional_data) };
    }
    else {
        # fall through, can't happen
        carp("ERROR: wrong item type\n");
        return 0;
    }

    if ( $@ ) {
        carp("ERROR: add_item(): EIS database transaction failed. Reason:\n$@\n");
        return undef;
    }
    return 1 if $result;
    return 0;
}

sub add_file_to_eis
{
    my $self         = shift;
    my $module       = shift;
    my $file         = shift;
    my $revision     = shift;
    my $authors_ref  = shift;
    my $taskids_ref  = shift;
    my $archive_path = shift;


    my $eis = Cws::eis();
    my $id = $self->eis_id();

    if ( !$id ) {
        carp("ERROR: Childworkspace not (yet) registered with EIS.\n");
        return undef;
    }

    # make certain that all task_ids are strings, otherwise
    # autotyping will choose the wrong type
    # Note: I think typing just the first element should suffice, but ...
    my @taskids = ();
    foreach ( @{$taskids_ref} ) {
        push(@taskids, Eis::to_string($_));
    }
    # HACK Its possible that we get no valid taskid.
    # Autotyping will fail for a list without elements;
    if ( !@taskids ) {
        push(@taskids, Eis::to_string(''));
    }

    # same for revision
    $revision = Eis::to_string($revision);

    if ( !$archive_path ) {
        $archive_path = Eis::to_string('');
    }

    my $result;
    eval {
        $result = $eis->addFile($id, $module, $file, $archive_path,
                                $revision, $authors_ref, \@taskids)
    };
    if ( $@ ) {
        carp("ERROR: add_file(): EIS database transaction failed. Reason:\n$@\n");
        return undef;
    }
    return 1 if $result;
    return 0;
}

sub add_patch_file_to_eis
{
    my $self         = shift;
    my $file         = shift;

    my $eis = Cws::eis();
    my $id = $self->eis_id();

    if ( !$id ) {
        carp("ERROR: Childworkspace not (yet) registered with EIS.\n");
        return undef;
    }

    my $result;
    eval { $result = $eis->addOutputFile($id, $file) };
    if ( $@ ) {
        carp("ERROR: add_patch_file(): EIS database transaction failed. Reason:\n$@\n");
        return undef;
    }
    return $1;# appOutputFile has void as return value ...
}

sub is_cws_name_available_in_eis
{
    my $self     = shift;

    if ( !$self->master() ) {
        carp("ERROR: master workspace name not set\n");
        return undef;
    }

    if ( !$self->child() ) {
        carp("ERROR: child workspace name not set\n");
        return undef;
    }

    my $eis = Cws::eis();
    my $master    = Eis::to_string($self->master());
    my $child     = Eis::to_string($self->child());

    my $result;
    eval { $result = $eis->isChildWorkspaceUnique($master, $child) };
    if ( $@ ) {
        carp("ERROR: is_cws_name_available(): EIS database transaction failed. Reason:\n$@\n");
    }
    return $result;
}

sub register_child_with_eis
{
    my $self     = shift;
    my $vcsid    = shift;
    my $location = shift;

    if ( !$self->master() ) {
        carp("ERROR: master workspace name not set\n");
        return undef;
    }

    if ( !$self->milestone() ) {
        carp("ERROR: master milestone not set\n");
        return undef;
    }

    if ( !$self->child() ) {
        carp("ERROR: child workspace name not set\n");
        return undef;
    }

    $vcsid    = '' unless $vcsid;
    $location = '' unless $location;

    my $eis = Cws::eis();
    my $master    = Eis::to_string($self->master());
    my $milestone = Eis::to_string($self->milestone());
    my $child     = Eis::to_string($self->child());

    $vcsid        = Eis::to_string($vcsid);
    $location     = Eis::to_string($location);

    my $result;
    eval {
        $result = $eis->createChildWorkspace($master, $milestone, $child,
                                                 $vcsid, $location)
    };

    if ( $@ ) {
        carp("ERROR: create_child_wortkspace(): EIS database transaction failed. Reason:\n$@\n");
        return undef;
    }
    # set EIS_ID directly, since $self->eis_id() is not
    # supposed to take parameters.
    $self->{EIS_ID} = $result;
    return $result;
}

sub promote_child_in_eis
{
    my $self     = shift;
    my $vcsid    = shift;
    my $location = shift;

    my $eis = Cws::eis();
    my $id = $self->eis_id();

    if ( !$id ) {
        carp("ERROR: Childworkspace not (yet) registered with EIS.\n");
        return undef;
    }

    if ( !$self->milestone() ) {
        carp("ERROR: master milestone not set\n");
        return undef;
    }

    my $milestone = Eis::to_string($self->milestone());

    $vcsid    = '' unless $vcsid;
    $location = '' unless $location;

    $vcsid        = Eis::to_string($vcsid);
    $location     = Eis::to_string($location);

    my $result;
    eval {
        $result = $eis->initializeChildWorkspace($id, $milestone, $vcsid, $location)
    };

    eval { $result = $eis->getStatus($id) };
    if ( $@ ) {
        carp("ERROR: promote(): EIS database transaction failed. Reason:\n$@\n");
        return 0;
    }
    return 1;
}

# Get child workspace approval status from EIS,
# return undef in case of error.
sub get_status_from_eis
{
    my $self = shift;

    # check if child workspace is valid
    my $id = $self->eis_id();
    if ( !$id ) {
        carp("ERROR: Childworkspace not (yet) registered with EIS.\n");
        return undef;
    }

    my $eis = Cws::eis();
    my $result;
    eval { $result = $eis->getStatus($id) };
    if ( $@ ) {
        carp("ERROR: get_status(): EIS database transaction failed. Reason:\n$@\n");
    }
    return $result;
}

# Get child workspace approval status from EIS,
# return undef in case of error.
sub set_status_in_eis
{
    my $self = shift;
    my $status = shift;
    my $method = 'set';
    $method .= (defined $status) ? $status : 'Integrated';

    # check if child workspace is valid
    my $id = $self->eis_id();
    if ( !$id ) {
        carp("ERROR: Childworkspace not (yet) registered with EIS.\n");
        return undef;
    }
    my $eis = Cws::eis();
    my $result;
    if (defined $status) {
        eval { $result = $eis->setFixedOnMaster($id) };
    } else {
        eval { $result = $eis->setIntegrated($id) };
    }
    if ( $@ ) {
        carp("ERROR: $method(): EIS database transaction failed. Reason:\n$@\n");
    }
    return $result;
}

# Get child workspace approval status from EIS,
# return undef in case of error.
sub set_integration_milestone_in_eis
{
    my $self      = shift;
    my $milestone = shift;
    my $buildid   = shift;

    # check if child workspace is valid
    my $id = $self->eis_id();
    if ( !$id ) {
        carp("ERROR: Childworkspace not (yet) registered with EIS.\n");
        return undef;
    }

    my $eis = Cws::eis();

    # just in case ...
    if ( !defined($milestone) ) {
        $milestone = Eis::to_string('');
    }
    # $buildid must be transfered as string
    if ( !defined($buildid) ) {
        $buildid = Eis::to_string('');
    }
    else {
        $buildid = Eis::to_string($buildid);
    }

    my $result;
    eval { $result = $eis->setIntegrationMilestone($id, $milestone, $buildid) };
    if ( $@ ) {
        carp("ERROR: set_integration_milestone(): EIS database transaction failed. Reason:\n$@\n");
    }
    return $result;
}

sub set_current_milestone_in_eis
{
    my $self      = shift;
    my $master    = shift;
    my $milestone = shift;

    $master    = Eis::to_string($master);
    $milestone = Eis::to_string($milestone);

    my $eis = Cws::eis();
    my $result;
    eval { $result = $eis->setCurrentMilestone( $master, $milestone ) };
    if ( $@ ) {
        carp("ERROR: set_current_milestone(): EIS database transaction failed. Reason:\n$@\n");
    }
    return $result;
}

sub get_current_milestone_from_eis
{
    my $self      = shift;
    my $master    = shift;

    $master    = Eis::to_string($master);

    my $eis = Cws::eis();
    my $result;
    eval { $result = $eis->getCurrentMilestone( $master ) };
    if ( $@ ) {
        carp("ERROR: get_current_milestone(): EIS database transaction failed. Reason:\n$@\n");
    }
    return $result;
}

sub get_is_milestone_used_from_eis
{
    my $self      = shift;
    my $master    = shift;
    my $milestone = shift;

    $master    = Eis::to_string($master);
    $milestone = Eis::to_string($milestone);

    my $eis = Cws::eis();
    my $result;
    eval { $result = $eis->isMilestoneInUse($master, $milestone) };
    if ( $@ ) {
        carp("ERROR: is_milestone(): EIS database transaction failed. Reason:\n$@\n");
    }
    return $result;
}

sub get_childworkspaces_for_milestone
{
    my $self      = shift;
    my $master    = shift;
    my $milestone = shift;

    $master    = Eis::to_string($master);
    $milestone = Eis::to_string($milestone);

    my $eis = Cws::eis();
    my $result;
    eval { $result = $eis->searchChildWorkspacesForMilestone($master, $milestone) };
    if ( $@ ) {
        carp("ERROR: get_childworkspaces_for_milestone(): EIS database transaction failed. Reason:\n$@\n");
    }
    return $result;
}

sub get_creation_master_from_eis
{
    my $self      = shift;

    # check if child workspace is valid
    my $id = $self->eis_id();
    if ( !$id ) {
        carp("ERROR: Childworkspace not (yet) registered with EIS.\n");
        return undef;
    }

    my $eis = Cws::eis();
    my $result;
    eval { $result = $eis->getCreationMasterWorkspace($id) };
    if ( $@ ) {
        carp("ERROR: get_creation_master(): EIS database transaction failed. Reason:\n$@\n");
    }
    return $result;

}

sub get_public_flag_from_eis
{
    my $self      = shift;

    # check if child workspace is valid
    my $id = $self->eis_id();
    if ( !$id ) {
        carp("ERROR: Childworkspace not (yet) registered with EIS.\n");
        return undef;
    }

    my $eis = Cws::eis();
    my $result;
    eval { $result = $eis->isPublic($id) };
    if ( $@ ) {
        carp("ERROR: get_public_flag(): EIS database transaction failed. Reason:\n$@\n");
    }
    return $result;
}

#logging
sub set_log_entry_in_eis
{
    my $self        = shift;
    my $commandline = shift;
    my $vcsid       = shift;
    my $start       = shift;
    my $end         = shift;
    my $comment     = shift;

    $commandline    = SOAP::Data->type(string => $commandline);
    $comment        = SOAP::Data->type(string => $comment);

    # *format* for  $start and $end = "2003-05-28 12:34:59";

#=====================================================
    #TO DO:
    #experimenell für saubere schnittstelle
    #$start = SOAP::Data->type(dateTime => $start);
    #$end = SOAP::Data->type(dateTime => $end);
#=====================================================

    my $eis = Cws::eis();
    my $result;
    eval { $result = $eis->storeCommandLogEntry( $commandline, $vcsid, $start, $end, $comment ) };
    if ( $@ ) {
        carp("ERROR: set_log_entry(): Logging failed. Reason:\n$@\n");
    }
    return $result;
}

#set_log_entry_extended_in_eis($commandname, $parameter, $vcsid, $start, $stop, $comment, $mastername, $childname);
sub set_log_entry_extended_in_eis
{
    my $self        = shift;
    my $commandname = shift;
    my $parameter   = shift;
    my $vcsid       = shift;
    my $start       = shift;
    my $end         = shift;
    my $comment     = shift;
    my $mastername  = shift;
    my $childname   = shift;

    $commandname    = SOAP::Data->type(string => $commandname);
    $parameter      = SOAP::Data->type(string => $parameter);
    $comment        = SOAP::Data->type(string => $comment);
    $mastername     = SOAP::Data->type(string => $mastername);
    $childname      = SOAP::Data->type(string => $childname);

    # *format* for  $start and $end = "2003-05-28 12:34:59";

#=====================================================
    #TO DO:
    #experimenell für saubere schnittstelle
    #$start = SOAP::Data->type(dateTime => $start);
    #$end = SOAP::Data->type(dateTime => $end);
#=====================================================

    my $eis = Cws::eis();
    my $result;
    eval { $result = $eis->storeCommandLogEntry($commandname, $parameter, $vcsid, $start, $end, $comment, $mastername, $childname) };
    if ( $@ ) {
        carp("ERROR: set_log_entry_extended(): Logging failed. Reason:\n$@\n");
    }
    return $result;
}


#### class methods ####

sub init_eis_connector
{
    my $eis = Eis->new( uri => Cws::eis_uri(),
                        proxy_list => Cws::eis_proxy_list(),
                        net_proxy => Cws::net_proxy()
                      );
    return $eis;
}

####

1; # needed by "use" or "require"
