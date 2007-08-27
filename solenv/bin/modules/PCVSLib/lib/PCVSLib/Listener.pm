#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: Listener.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: vg $ $Date: 2007-08-27 13:34:42 $
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


#
# Listener.pm - package Listener provides the internal listeners for PCVSLib Responses
#


package PCVSLib::Listener;

use Carp;

use PCVSLib::Directory;
use PCVSLib::Event;
use PCVSLib::ModeTime;

use strict;
use warnings;

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    $self->{message}    = undef;
    $self->{is_success} = 0;
    bless ($self, $class);
    return $self;
}

#### instance accessors #####

for my $datum qw(is_success message) {
    no strict "refs";
    *$datum = sub {
        my $self = shift;
        return $self->{$datum};
    }
}

#### public methods ####

sub notify
{
    my $self  = shift;
    my $event = shift;

    if ( $event->isa('PCVSLib::ErrorMessageEvent') ) {
        $self->{message} = $event->get_message();
    }
    if ( $event->isa('PCVSLib::TerminatedEvent') ) {
        $self->{is_success} = $event->is_success();
    }
}


# Used for all internal commands
package PCVSLib::InternalListener;
use vars('@ISA');

use Carp;
use File::Basename;

@ISA=('PCVSLib::Listener');

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    $self->{module_expansion}  = undef;
    $self->{root} = undef
    # private members
    $self->{last_mode_} = undef;
    $self->{touched_dirs_} = ();
    $self->{needs_directories_update_} = 0;
    $self->{last_modification_time_}   = 0;
    bless ($self, $class);
    return $self;
}

for my $datum qw(module_expansion) {
    no strict "refs";
    *$datum = sub {
        my $self = shift;
        return $self->{$datum};
    }
}

for my $datum qw(root) {
    no strict "refs";
    *$datum = sub {
        my $self = shift;
        $self->{$datum} = shift if @_;
        return $self->{$datum};
    }
}

sub notify
{
    my $self  = shift;
    my $event = shift;

    $self->SUPER::notify($event);

    if ( $event->isa('PCVSLib::ModuleExpansionEvent') ) {
        $self->{module_expansion}  = $event->get_module_expansion();
    }
    if ( $event->isa('PCVSLib::ModTimeEvent') ) {
        $self->{last_modification_time_}  = $event->get_modification_time();
    }
    if ( $event->isa('PCVSLib::SetStickyEvent') ) {
        my $dir = $event->get_directory();
        my $repository = $event->get_repository();
        my $directory = $self->maybe_create_directory($dir, $repository);
        my $tag = $event->get_tag();
        $directory->tag($tag);
    }
    if ( $event->isa('PCVSLib::ClearStickyEvent') ) {
        my $dir = $event->get_directory();
        my $repository = $event->get_repository();
        my $directory = $self->maybe_create_directory($dir, $repository);
        $directory->tag('');
    }
    if ( $event->isa('PCVSLib::CreatedEvent') ) {
        my $file  = $event->get_file();
        my $dir = $file->path_name()->local_directory();
        my $repository = $file->path_name()->repository();
        my $directory = $self->maybe_create_directory($dir, $repository);
        $file->receive_and_save();

        my $entry = $file->entry();
        if ( $self->{last_modification_time_} ) {
            my $mode_time = PCVSLib::ModeTime->new($file->path_name()->local_path());
            $mode_time->touch($self->{last_modification_time_});
            $entry->timestamp($self->{last_modification_time_});
            $self->{last_modification_time_} = 0;
        }
        else {
            croak("PCVSLIB::Listener::notify(): 'Created' response was not preceded by 'Mod-time' response");
        }
        $directory->set_entry($entry->name(), $entry);
        $self->{needs_directories_update_} = 1;
    }
    if ( $event->isa('PCVSLib::UpdateExistingEvent') ) {
        my $file  = $event->get_file();
        $file->receive_and_save();
        my $entry = $file->entry();

        # File time stamp and time_stamp() in entry line
        # need to refer to exact the same time
        my $mode_time = PCVSLib::ModeTime->new($file->path_name()->local_path());

        my $time = time();
        $mode_time->touch($time);
        $entry->timestamp($time);

        # update administrative files
        my $dir = $file->path_name()->local_directory();
        my $repository = $file->path_name()->repository();
        my $directory = $self->maybe_create_directory($dir, $repository);
        $directory->set_entry($entry->name(), $entry);
        $self->{needs_directories_update_} = 1;
    }
    if ( $event->isa('PCVSLib::ModeEvent') ) {
        $self->{last_mode_}  = $event->get_mode();
    }
    if ( $event->isa('PCVSLib::CheckedInEvent') ) {
        my $path_name = $event->get_path_name();
        my $dir = $path_name->local_directory();
        my $path = $path_name->local_path();
        my $repository = $path_name->repository();
        my $directory = $self->maybe_create_directory($dir, $repository);
        my $entry     = PCVSLib::Entry->new($event->get_entry());

        # TODO: keyword expansion -> needs to be done here
        my $mode_time = PCVSLib::ModeTime->new($path);
        if ( $self->{last_mode_} ) {
            $mode_time->set_mode($self->{last_mode_});
            $self->{last_mode_} = undef;
        }

        if ( $entry->version() =~ /^-/ ) {
            $entry->timestamp(0);
        }
        else {
            # File time stamp and time_stamp() in entry line
            # need to refer to exact the same time
            my $time = time();
            $mode_time->touch($time);
            $entry->timestamp($time);
        }

        $directory->set_entry($entry->name(), $entry);
           $self->{needs_directories_update_} = 1;
    }
    if ( $event->isa('PCVSLib::CopyFileEvent') ) {
        my $path_name = $event->get_path_name();
        my $path      = $path_name->local_path();
        my $dir       = $path_name->local_directory();
        my $new_name  = "$dir/" . $event->get_new_name();
        # CVS protocoll document: this can optionally be implemented as a 'rename'
        if ( -e $new_name ) {
            if ( !unlink($new_name) ) {
                croak("PCVSLIB::UpdateListener::notify(): can't unlink '$new_name': $!");
            }
        }
        if ( !rename($path, $new_name) ) {
            croak("PCVSLIB::UpdateListener::notify(): can't unlink '$new_name': $!");
        }
    }
    if ( $event->isa('PCVSLib::MergedEvent') ) {
        my $file  = $event->get_file();
        $file->receive_and_save();
        my $entry = $file->entry();

        # File time stamp and time_stamp() in entry line
        # need to refer to exact the same time in case of a conflict
        my $mode_time = PCVSLib::ModeTime->new($file->path_name()->local_path());

        my $time = time();
        $mode_time->touch($time);
        $entry->timestamp($time);
        $entry->is_merge(1); # can't be guessed from the entry line for clean merges

        # update administrative files
        my $dir = $file->path_name()->local_directory();
        my $repository = $file->path_name()->repository();
        my $directory = $self->maybe_create_directory($dir, $repository);
        $directory->set_entry($entry->name(), $entry);
        $self->{needs_directories_update_} = 1;
    }
    if ( $event->isa('PCVSLib::ClearStaticDirectoryEvent') ) {
        my $dir = $event->get_directory();
        my $repository = $event->get_repository();
        my $directory = $self->maybe_create_directory($dir, $repository);
        $directory->is_static(0);
        $self->{needs_directories_update_} = 1;
    }
    if ( $event->isa('PCVSLib::SetStaticDirectoryEvent') ) {
        my $dir = $event->get_directory();
        my $repository = $event->get_repository();
        my $directory = $self->maybe_create_directory($dir, $repository);
        $directory->is_static(1);
        $self->{needs_directories_update_} = 1;
    }
    if ( $event->isa('PCVSLib::RemoveEntryEvent') ) {
        my $path_name = $event->get_path_name();
        my $name = $path_name->name();
        my $dir = $path_name->local_directory();
        my $repository = $path_name->repository();
        my $directory = $self->maybe_create_directory($dir, $repository);
        $directory->remove_entry($name);
        $self->{needs_directories_update_} = 1;
    }
    if ( $event->isa('PCVSLib::TerminatedEvent') ) {
        if ( $self->{needs_directories_update_} ) {
            foreach ( keys %{$self->{touched_dirs_}} ) {
                $self->{touched_dirs_}->{$_}->update();
            }
            $self->{needs_directories_update_} = 0;
        }
    }
}

sub maybe_create_directory
{
    my $self       = shift;
    my $dir        = shift;
    my $repository = shift;

    my $directory;
    if ( !exists($self->{touched_dirs_}->{$dir}) ) {
        if ( -d $dir ) {
            $directory = PCVSLib::Directory->new($dir);
        }
        else {
            $directory = $self->create_directory($dir, $repository);
        }
        $self->{touched_dirs_}->{$dir} = $directory;
    }
    else {
        $directory = $self->{touched_dirs_}->{$dir};
    }
}

sub create_directory
{
    my $self            = shift;
    my $dir             = shift;
    my $repository      = shift;

    if ( !mkdir($dir) ) {
        croak("PCVSLIB::Listener::notify(): can't create directory '$dir': $!");
    }
    # add directory entry to parent
    my ($name, $parent) = fileparse($dir);
    chop($parent); # remove trailing /
    if ( exists($self->{touched_dirs_}->{$parent}) ) {
        my $entry = PCVSLib::Entry->new();
        $entry->name($name);
        $entry->is_directory(1);
        $self->{touched_dirs_}->{$parent}->set_entry($entry->name(), $entry);
    }

    my $root_dir = $self->{root}->root_dir();
    my $directory = PCVSLib::Directory->new($dir);
    $directory->root($self->{root});
    $directory->repository($repository);
    return $directory;
}

1;
# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:
