#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: LogHandle.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: vg $ $Date: 2007-08-27 13:34:52 $
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
# LogHandle.pm - package for logging CVS protocol traffic
#

package PCVSLib::LogHandle;
use base(IO::Handle);

use strict;
use warnings;

#### ctor ####

sub new
{
    my $class = shift;
    my $self = { };
    $self->{handle}         = shift;    # file handle to be wrapped
    $self->{logfile}        = shift;    # logfile
    bless ($self, $class);
    return $self;
}

#### instance accessors #####

for my $datum qw(handle logfile) {
    no strict "refs";
    *$datum = sub {
        my $self = shift;
        $self->{$datum} = shift if @_;
        return $self->{$datum};
    }
}

#### public methods ####

sub print
{
    my $self = shift;
    if ( $self->logfile() ) {
        $self->logfile()->print("C: ");
        $self->logfile()->print(@_);
    }
    $self->handle()->print(@_);
}

sub getline
{
    my $self = shift;

    my $line = $self->handle()->getline();
    if ( $self->logfile() ) {
        $self->logfile()->print("S: ");
        $self->logfile()->print($line);
    }
    return $line;
}

sub read
{
    my $self   = shift;

    my $nbytes = $self->handle()->read(@_);
    if ( $self->logfile() ) {
        $self->logfile()->print("S: send file:\n");
        if ( $nbytes > 512 ) {
            $self->logfile()->print("... $nbytes of data ...\n");
        }
        else {
            $self->logfile()->print($_[0]);
        }
    }
    return $nbytes;
}

sub write
{
    my $self = shift;

    if ( $self->logfile() ) {
        $self->logfile()->print("C: send file\n");
        if ($_[1] > 512) {
            $self->logfile()->print("... $_[1] of data ...\n");
        }
        else {
            $self->logfile()->print($_[0]);
        }
    }
    return $self->handle()->write(@_);
}

sub shutdown
{
    my $self = shift;
    if ( $self->logfile() ) {
        $self->logfile()->print("C: shutdown server socket\n");
    }

    return $self->handle()->shutdown(@_);
}

1;
# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:
