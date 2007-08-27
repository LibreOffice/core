#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: Directory.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: vg $ $Date: 2007-08-27 13:33:54 $
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
# Directory.pm - package representing the concept of a CVS directory
#

package PCVSLib::Directory;

use Carp;

use PCVSLib::Entry;
use PCVSLib::Root;

use strict;
use warnings;

#### ctor ####

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    $self->{directory}  = shift;

    $self->{entries}    = ();
    $self->{repository} = undef;
    $self->{root}       = undef;
    $self->{tag}        = '';
    $self->{is_static}  = 0;
    $self->{is_leaf}    = undef;

    # private members
    $self->{entries_changed_}    = 0;
    $self->{repository_changed_} = 0;
    $self->{root_changed_}       = 0;
    $self->{tag_changed_}        = 0;
    $self->{is_static_changed_}  = 0;

    $self->{parsed_from_dir_}    = 0;
    bless ($self, $class);

    if ( !defined($self->{directory}) ) {
        croak("PCVSLIB::Directory::new(): directory not set")
    }

    if ( -d "$self->{directory}/CVS" ) {
        $self->parse_admin_files_();
    }

    return $self;
}

#### instance accessors #####

for my $datum qw(directory is_leaf) {
    no strict "refs";
    *$datum = sub {
        my $self = shift;
        $self->{$datum} = shift if @_;
        return $self->{$datum};
    }
}

for my $datum qw(entries repository root tag is_static) {
    no strict "refs";
    *$datum = sub {
        my $self = shift;
        if ( @_ ) {
            $self->{$datum} = shift;
            $self->{$datum . '_changed_'} = 1;
        }
        return $self->{$datum};
    }
}

#### public methods ####

sub get_entry
{
    my $self = shift;
    my $name = shift;

    return $self->{entries}->{$name};
}

sub set_entry
{
    my $self  = shift;
    my $name  = shift;
    my $entry = shift;

    $self->{entries}->{$name} = $entry;
    $self->{entries_changed_} = 1;
}

sub remove_entry
{
    my $self  = shift;
    my $name  = shift;

    if ( delete($self->{entries}->{$name}) ) {
        $self->{entries_changed_} = 1;
    }
}

sub is_questionable
{
    my $self = shift;
    my $file = shift;

    if ( exists $self->{entries}->{$file} ) {
        return 0;
    }

    my $path = $self->{directory} . "/$file";
    if ( ! -e $path ) {
        croak("PCVSLIB::Directory::is_questionable_(): nothing known about '$path'");
    }
    return 1;
}

sub is_resolved
{
    my $self = shift;
    my $file = shift;

    my $entry = $self->{entries}->{$file};

    if ( !defined($entry) ) {
        croak("PCVSLIB::Directory::is_resolved(): '$file' is not a managed file");
    }

    my $path = $self->{directory} . "/$file";
    if ( ! -e $path ) {
        croak("PCVSLIB::Directory::is_resolved(): '$path' has been lost");
    }
    my @stat = stat(_);

    # Only files that have been merged with conflicts can be "resolved"
    if ( $entry->is_conflict() ) {
        if ( $entry->timestamp() < $stat[9] ) {
            return 1;
        }
    }
    return 0;
}


sub is_modified
{
    my $self = shift;
    my $file = shift;

    my $entry = $self->{entries}->{$file};

    if ( !defined($entry) ) {
        croak("PCVSLIB::Directory::is_modified_(): '$file' is not a managed file");
    }

    my $path = $self->{directory} . "/$file";
    if ( ! -e $path ) {
        croak("PCVSLIB::Directory::is_modified_(): '$path' has been lost");
    }
    my @stat = stat(_);

    if ( $entry->is_merge() ) {
        # Note: this includes the conflict case
        return 1;
    }

    if ( $entry->timestamp() == $stat[9] ) {
        return 0;
    }

    return 1;
}

sub update
{
    my $self = shift;


    if ( !defined($self->{directory}) ) {
        croak("PCVSLIB::Directory::update(): directory not set");
    }

    my $cvs_dir = $self->{directory} . "/CVS";
    my $new = 0;
    if ( !-d $cvs_dir ) {
        if ( !mkdir($cvs_dir) ) {
            croak("PCVSLIB::Directory::update(): can't create directory '$cvs_dir': $!");
        }
        $new++;
    }

    if ( $new || $self->{entries_changed_} ) {
        $self->write_entries_file();
        $self->{entries_changed_} = 0;
    }

    if ( $new || $self->{root_changed_} ) {
        $self->write_root_file();
        $self->{root_changed_} = 0;
    }

    if ( $new || $self->{repository_changed_} ) {
        $self->write_repository_file();
        $self->{repository_changed_} = 0;
    }

    if ( $new || $self->{tag_changed_} ) {
        $self->write_tag_file();
        $self->{tag_changed_} = 0;
    }

    if ( $new || $self->{is_static_changed_} ) {
        if ( $self->{is_static} ) {
            $self->touch_file_('Entries.Static');
        }
        else {
            $self->unlink_file_('Entries.Static');
        }
        $self->{is_static_changed_} = 0;
    }
}


#### private methods ####

sub read_entries_file_ {
    my $self = shift;

    my $entries_fh = $self->open_admin_file_("Entries", 0);
    $self->{is_leaf} = 0;
    while (my $line = <$entries_fh>) {
        chomp($line);
        if ( $line =~ /^D$/ ) {
            $self->{is_leaf} = 1;
            next;
        }
        my $entry = PCVSLib::Entry->new($line);
        my $name = $entry->name();
        $self->{entries}->{$name} = $entry;
    }
    $entries_fh->close();
}

sub read_root_file_ {
    my $self = shift;

    my $root_fh = $self->open_admin_file_("Root", 0);
    my $root = <$root_fh>;
    chomp($root);
    $self->{root} = PCVSLib::Root->new($root);
    $root_fh->close();

}

sub read_repository_file_ {
    my $self = shift;

    my $repository_fh = $self->open_admin_file_("Repository", 0);
    my $line = <$repository_fh>;
    chomp($line);
    $self->{repository} = $line;
    $repository_fh->close();
}

sub read_tag_file_ {
    my $self = shift;

    my $tag_path = $self->{directory} . "/CVS/Tag";
    if ( -e $tag_path ) {
        my $tag_fh = $self->open_admin_file_("Tag", 0);
        $self->{tag} = <$tag_fh>;
        chomp($self->{tag});
        $tag_fh->close();
    }
    else {
        $self->{tag} = 0;
    }
}

sub write_entries_file {
    my $self = shift;

    my @dirs;
    my @files;
    $self->{is_leaf} = 1;
    foreach ( keys %{$self->{entries}} ) {
        if ( $self->{entries}->{$_}->is_directory() ) {
            push(@dirs, $self->{entries}->{$_}->to_string());
            $self->{is_leaf} = 0;
        }
        else {
            push(@files, $self->{entries}->{$_}->to_string());
        }
    }

    my $entries_fh = $self->open_admin_file_("Entries.tmp", 1);
    foreach ( sort(@dirs), sort(@files) ) {
        $entries_fh->print("$_\n");
    }
    if ( $self->{is_leaf} ) {
        $entries_fh->print("D\n");
    }
    $entries_fh->close();
    $self->rename_tmp("Entries.tmp", "Entries");
}

sub write_root_file {
    my $self = shift;

    my $root_fh = $self->open_admin_file_("Root.tmp", 1);
    if ( $self->{root}->port() != 2401 ) {
        $root_fh->print($self->{root}->to_string_with_port() . "\n");
    }
    else {
        $root_fh->print($self->{root}->to_string() . "\n");
    }
    $root_fh->close();
    $self->rename_tmp("Root.tmp", "Root");
}

sub write_repository_file {
    my $self = shift;

    my $repository_fh = $self->open_admin_file_("Repository.tmp", 1);
    $repository_fh->print($self->{repository} . "\n");
    $repository_fh->close();
    $self->rename_tmp("Repository.tmp", "Repository");
}

sub write_tag_file {
    my $self = shift;

    if ( $self->{tag} ) {
        my $tag_fh = $self->open_admin_file_("Tag.tmp", 1);
        $tag_fh->print($self->{tag} . "\n");
        $tag_fh->close();
        $self->rename_tmp("Tag.tmp", "Tag");
    }
    else {
        $self->unlink_file_("Tag");
    }
}


sub parse_admin_files_
{
    my $self = shift;

    if ( !defined($self->directory) ) {
        croak("PCVSLIB::Directory::parse_admin_files_(): directory not set");
    }

    $self->read_entries_file_();
    $self->read_root_file_();
    $self->read_repository_file_();
    $self->read_tag_file_();

    if ( -e $self->{directory} . '/CVS/Entries.Static' ) {
        $self->{is_static} = 1;
    }

    $self->{parsed_from_dir_}++;
}

sub open_admin_file_
{
    my $self  = shift;
    my $file  = shift;
    my $write = shift;

    my $path = $self->{directory} . "/CVS/" . $file;
    # read Entries file
    my $fh = IO::File->new( $write ? ">$path" : "<$path");
    if ( !defined($fh) ) {
        my $mode = $write ? 'writing' : 'reading';
        croak("PCVSLIB::Directory::open_admin_file_(): can't open file for $mode: '$path': $!");
    }
    return $fh;
}

sub rename_tmp
{
    my $self = shift;
    my $old  = $self->{directory} . "/CVS/" . shift;
    my $new  = $self->{directory} . "/CVS/" . shift;

    if ( -e $new ) {
        if ( !unlink($new) ) {
            croak("PCVSLIB::Directory::rename_tmp(): can't unlink file: '$new': $!");
        }
    }

    if ( !rename($old, $new) ) {
        croak("PCVSLIB::Directory::rename_tmp(): can't rename file: '$old' -> '$new': $!");
    }
}

sub unlink_file_
{
    my $self = shift;

    my $path  = $self->{directory} . "/CVS/" . shift;

    if ( -e $path ) {
        if ( !unlink($path) ) {
            croak("PCVSLIB::Directory::unlink_file_(): can't unlink '$path': $!");
        }
    }
}

sub touch_file_
{
    my $self = shift;

    my $path  = $self->{directory} . "/CVS/" . shift;

    my $fh = IO::File->new( ">$path");
    if ( !defined($fh) ) {
        croak("PCVSLIB::Directory::touch_file_(): can't write to file '$path': $!");
    }
    $fh->close();
}

1;
# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:
