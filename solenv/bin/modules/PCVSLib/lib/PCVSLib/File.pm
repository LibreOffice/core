#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: File.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: vg $ $Date: 2007-08-27 13:34:33 $
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
# File.pm - package for encapsulating file data
#

package PCVSLib::File;

use Carp;

use PCVSLib::ModeTime;

use strict;
use warnings;

#### ctor ####

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $path_name_string = shift;
    my $self = {};

    $self->{path_name}       = undef;
    $self->{entry}           = undef;
    $self->{mode}            = undef;
    $self->{size}            = undef;
    $self->{io_handle}       = undef;

    bless ($self, $class);

    if ( defined($path_name_string) ) {
        $self->{path_name} = $path_name_string;
    }
    return $self;
}

#### instance accessors #####

for my $datum qw(path_name entry mode size io_handle) {
    no strict "refs";
    *$datum = sub {
        my $self = shift;
        $self->{$datum} = shift if @_;
        return $self->{$datum};
    }
}

#### public methods ####

sub read_and_send
{
    my $self = shift;

    my $path = $self->{path_name}->local_path();
    my @sb = stat($path);
    if ( !@sb ) {
        croak("PCVSLib::File::read(): can't stat file '$path': $!");
    }

    my $mode = PCVSLib::ModeTime->new($path);
    $self->{mode} = $mode->get_mode();

    # transfer mode
    $self->{io_handle}->print($self->{mode} ."\n");

    # Textfiles: copy to temporary file and apply any needed conversion. We
    # do this for Unix, too, to catch and remove any spurios CR characters. After
    # that send the temporary file to CVS server

    # The tempfile is created in the tmp file standard directory, which is supposed
    # to be fast. Additional advantage: no need for writing rights in
    # $self->{path_name}->local_dir()

    my $inh;
    if ( $self->is_binary() ) {
        $self->{size} = $sb[7];
        my $inh = IO::File->new("<$path");
        if ( !defined($inh) ) {
            croak("PCVSLib::File::read(): can't open file '$path' for reading: $!");
        }
        binmode($inh);
    }
    else {
        $inh = IO::File->new_tmpfile();
        if ( !defined($inh) ) {
            croak("PCVSLib::File::read(): can't create temporary file: $!");
        }
        binmode($inh);
        my $fh = IO::File->new("<$path");
        if ( !defined($fh) ) {
            croak("PCVSLib::File::read(): can't open file '$path' for reading: $!");
        }

        my $line;
        while($line = $fh->getline()) {
            $line =~ s/\r//;
            $inh->print($line);
        }

        $fh->close();
        $inh->flush();
        if ( !$inh->seek(0, 0) ) { # rewind
            croak("PCVSLib::File::read(): can't rewind temporary file: $!");
        }

        my @sb = $inh->stat();
        if ( !@sb ) {
            croak("PCVSLib::File::read(): can't stat temporary file: $!");
        }
        $self->{size} = $sb[7];
    }

    # transfer size
    $self->{io_handle}->print($self->{size} ."\n");

    my $nbytes = $self->{size};
    my $buffer;
    my $nwant;
    my $nread;
    while ( $nbytes > 0 ) {
        $nwant = ($nbytes < 32768) ? $nbytes : 32768;
        $nread = $inh->read($buffer, $nwant);
        if ( !defined ($nread) ) {
            $inh->close();
            croak("PCVSLib::File::read(): reading temporary file failed: $!");
        }
        if ( $nread < $nwant ) {
            $inh->close();
            croak("PCVSLib::File::read(): short read from temporary file");
        }
        if ( !$self->{io_handle}->write($buffer, $nread) ) {
            $inh->close();
            croak("PCVSLib::File::save(): can't write to CVS server: $!");
        }
        $nbytes -= $nread;
    }
    $inh->close();
}

sub receive_and_save
{
    my $self = shift;

    # get the mode of the transferred file
    $self->{mode} = $self->io_handle->getline();
    chomp($self->{mode});


    # get the size of the transferred file
    $self->{size} = $self->io_handle->getline();
    chomp($self->{size});

    my $localdir = $self->{path_name}->local_directory();
    my $name = $self->{path_name}->name();

    my $path = $localdir . '/' . $name;

    if ( -e $path ) {
        if ( !unlink($path) ) {
            croak("PCVSLib::File::save(): can't unlink '$path': $!");
        }
    }

    # Textfiles: save first in a temporary file, apply any conversion if needed and
    # rename/copy the file to $path.

    # The temporary file is created in the destination directory,
    # this is for the most cases (Unix, binary files) the most efficent way - only
    # an additional rename needed - and writing rights in directory are required anyway.

    my $tmp_file = $localdir . '/' . ".#$name.tmp";

    my $tmph = IO::File->new(">$tmp_file");
    if ( !defined($tmph) ) {
        croak("PCVSLib::File::save(): can't open file '$tmp_file' for writing: $!");
    }
    binmode($tmph);

    my $buffer;
    my $nbytes = $self->{size};
    my $nwant;
    my $nread;
    while ( $nbytes > 0 ) {
        $nwant = ($nbytes < 32768) ? $nbytes : 32768;
        $nread = $self->{io_handle}->read($buffer, $nwant);
        if ( !defined ($nread) ) {
            croak("PCVSLib::File::save(): reading from CVS server socket failed: $!");
        }
        if ( $nread < $nwant ) {
            croak("PCVSLib::File::save(): short read from CVS server socket");
        }
        if ( !$tmph->write($buffer, $nread) ) {
            $tmph->close();
            unlink($tmp_file);
            croak("PCVSLib::File::save(): can't write to file '$tmp_file': $!");
        }
        $nbytes -= $nread;
    }
    $tmph->close();

    # copy with conversion for text files on Windows
    if ($^O eq 'MSWin32' && !$self->is_binary()) {
        carp("WARNING: Windows file handling is (still) completly untested, check carefully");
        my $inh = IO::File->new("<$tmp_file");
        if ( !defined($inh) ) {
            croak("PCVSLib::File::save(): can't open file '$tmp_file' for reading: $!");
        }
        my $outh = IO::File->new(">$path");
        if ( !defined($inh) ) {
            croak("PCVSLib::File::save(): can't open file '$tmp_file' for reading: $!");
        }
        while($inh->getline()){
            $outh->print($_);
        }
        $inh->close();
        $outh->close();
        if ( !unlink($tmp_file) ) {
            croak("PCVSLib::File::save(): can't unlink '$tmp_file': $!");
        }
    }
    else {
        if ( !rename($tmp_file, $path) ) {
            croak("PCVSLib::File::save(): can't rename '$tmp_file' to '$path': $!");
        }
    }

    # finally set the right permissions
    my $mode = PCVSLib::ModeTime->new($path);
    $mode->set_mode($self->{mode});
}

#### private methods ####

sub is_binary
{
    my $self = shift;

    if ( !defined $self->{entry} ) {
        croak("PCVSLib::File::is:binary(): internal error: entry line not set");
    }

    return $self->{entry}->is_binary();
}

1;
# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:
