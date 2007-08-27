#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: Response.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: vg $ $Date: 2007-08-27 13:35:31 $
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
# Response.pm - packages for receiving and processing Responses
#

package PCVSLib::Response;

use Carp;
use File::Basename;

use strict;
use warnings;

use PCVSLib::Event;
use PCVSLib::File;
use PCVSLib::PathName;

# Response factory (class method)
sub create_response
{
    my $self = shift; # ignore
    my $event_handler = shift;
    my $io_handle     = shift;

    my $response_string = $io_handle->getline();

    if ( $response_string =~ /^M (.*)$/ ) {
        my $message = $1;
        my $response = PCVSLib::MessageResponse->new($event_handler, $message);
        return $response;
    }
    if ( $response_string =~ /^MT (.*)$/ ) {
        my $message = $1;
        my $response = PCVSLib::MTResponse->new($event_handler, $message);
        return $response;
    }
    elsif ( $response_string =~ /^E (.*)$/ ) {
        my $error_message = $1;
        my $response = PCVSLib::ErrorMessageResponse->new($event_handler, $error_message);
        return $response;
    }
    elsif ( $response_string =~ /^F$/ ) {
        my $response = PCVSLib::FlushResponse->new($event_handler);
        return $response;
    }
    elsif ( $response_string =~ /^error (.*)$/ ) {
        my $error_message = $1;
        my $response = PCVSLib::ErrorResponse->new($event_handler, $error_message);
        return $response;
    }
    elsif ( $response_string =~ /^ok/ ) {
        my $response = PCVSLib::SuccessResponse->new($event_handler);
        return $response;
    }
    elsif ( $response_string =~ /^Valid-requests (.*)$/ ) {
        my $request_list_str = $1;
        my $response = PCVSLib::ValidRequestsResponse->new($event_handler, $request_list_str);
        return $response;
    }
    elsif ( $response_string =~ /^Clear-sticky (.*)$/ ) {
        my $pathname = $self->get_path_name($1, $io_handle);
        my $response = PCVSLib::ClearStickyResponse->new($event_handler, $pathname);
        return $response;
    }
    elsif ( $response_string =~ /^Set-sticky (.*)$/ ) {
        my $pathname = $self->get_path_name($1, $io_handle);
        my $tag = $io_handle->getline();
        chomp($tag);
        my $response = PCVSLib::SetStickyResponse->new($event_handler, [$pathname, $tag]);
        return $response;
    }
    elsif ( $response_string =~ /^Clear-static-directory (.*)$/ ) {
        my $pathname = $self->get_path_name($1, $io_handle);
        my $response = PCVSLib::ClearStaticDirectoryResponse->new($event_handler, $pathname);
        return $response;
    }
    elsif ( $response_string =~ /^Set-static-directory (.*)$/ ) {
        my $pathname = $self->get_path_name($1, $io_handle);
        my $response = PCVSLib::SetStaticDirectoryResponse->new($event_handler, $pathname);
        return $response;
    }
    elsif ( $response_string =~ /^Module-expansion (.*)$/ ) {
        my $module_expansion = $1;
        my $response = PCVSLib::ModuleExpansionResponse->new($event_handler, $module_expansion);
        return $response;
    }
    elsif ( $response_string =~ /^Updated (.*)$/ ) {
        # We support 'Created' and 'Update-existing' rseponses, no server should ever
        # send am 'Updated' response.
        carp("PCVSLIB::Response::create_response(): unexpected response 'Updated'");
        my $path_name = $self->get_path_name($1, $io_handle);
        my $file = $self->get_transmitted_file_($path_name, $io_handle);
        my $response = PCVSLib::UpdatedResponse->new($event_handler, $file);
        return $response;
    }
    elsif ( $response_string =~ /^Merged (.*)$/ ) {
        my $path_name = $self->get_path_name($1, $io_handle);
        my $file = $self->get_transmitted_file_($path_name, $io_handle);
        my $response = PCVSLib::MergedResponse->new($event_handler, $file);
        return $response;
    }
    elsif ( $response_string =~ /^Update-existing (.*)$/ ) {
        my $path_name = $self->get_path_name($1, $io_handle);
        my $file = $self->get_transmitted_file_($path_name, $io_handle);
        my $response = PCVSLib::UpdateExistingResponse->new($event_handler, $file);
        return $response;
    }
    elsif ( $response_string =~ /^Created (.*)$/ ) {
        my $path_name = $self->get_path_name($1, $io_handle);
        my $file = $self->get_transmitted_file_($path_name, $io_handle);
        my $response = PCVSLib::CreatedResponse->new($event_handler, $file);
        return $response;
    }
    elsif ( $response_string =~ /^Mod-time (.*)$/ ) {
        my $mod_time = $1;
        my $response = PCVSLib::ModTimeResponse->new($event_handler, $mod_time);
        return $response;
    }
    elsif ( $response_string =~ /^Mode (.*)$/ ) {
        my $mode = $1;
        my $response = PCVSLib::ModeResponse->new($event_handler, $mode);
        return $response;
    }
    elsif ( $response_string =~ /^Checked-in (.*)$/ ) {
        my $pathname = $self->get_path_name($1, $io_handle);
        my $entry = $io_handle->getline();
        chomp($entry);
        my $response = PCVSLib::CheckedInResponse->new($event_handler, [$pathname, $entry]);
        return $response;
    }
    elsif ( $response_string =~ /^Remove-entry (.*)$/ ) {
        my $pathname = $self->get_path_name($1, $io_handle);
        my $response = PCVSLib::RemoveEntryResponse->new($event_handler, $pathname);
        return $response;
    }
    elsif ( $response_string =~ /^Copy-file (.*)$/ ) {
        my $pathname = $self->get_path_name($1, $io_handle);
        my $new_name = $io_handle->getline();
        chomp($new_name);
        my $response = PCVSLib::CopyFileResponse->new($event_handler, [$pathname, $new_name]);
        return $response;
    }
    else {
        croak("PCVSLIB::Response::create_response(): internal error: unexpected response '$response_string'");
    }
}

my %ResponseClassData = (
    valid_responses => []
);

# class data accessor methods
for my $datum (qw(valid_responses)) {
    no strict "refs";
    *$datum = sub {
        shift; # ignore calling class/object
        return $ResponseClassData{$datum};
    }
}

#### ctor ####

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    $self->{event_handler}   = shift;
    $self->{response_data}   = shift;
    $self->{io_handle}       = shift;
    # private members

    bless ($self, $class);

    return $self;
}

#### instance accessors #####

for my $datum qw(event_handler response_data io_handle) {
    no strict "refs";
    *$datum = sub {
        my $self = shift;
        $self->{$datum} = shift if @_;
        return $self->{$datum};
    }
}

#### public methods ####

sub is_last
{
    return 0;
}

# process Response, the default is to do nothing
sub process
{
    return;
}

#### private methods ####

sub get_transmitted_file_
{
    my $self = shift; # ignore
    my $path_name = shift;
    my $io_handle = shift;

    # get entry line
    my $entry_str = $io_handle->getline();
    chomp($entry_str);
    my $entry = PCVSLib::Entry->new($entry_str);

    my $file = PCVSLib::File->new($path_name);
    $file->entry($entry);
    # transfer the file handle to the PCVSLib file handle,
    # the content of the file is saved to disk in a checkout or
    # update Listener
    $file->io_handle($io_handle);

    return $file;
}

sub get_path_name
{
    my $self      = shift; #ignore
    my $local_dir = shift;
    my $io_handle = shift;

    my $pathname = PCVSLib::PathName->new();
    chop($local_dir); # remove trailing /
    $pathname->local_directory($local_dir);
    my $path = $io_handle->getline();
    chop($path); # remove trailing new line
    my ($name, $repository) = fileparse($path);
    chop($repository); # remove trailing /

    $pathname->repository($repository);
    $pathname->name($name);

    return $pathname;
}


#### derived classes ####

package PCVSLib::SuccessResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "ok");

sub is_last
{
    my $self          = shift;

    my $event = PCVSLib::TerminatedEvent->new(1);
    $self->{event_handler}->send_event($event);
    return 1;
}

package PCVSLib::ErrorResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "error");

sub is_last
{
    my $self          = shift;

    my $event = PCVSLib::TerminatedEvent->new(0);
    $self->{event_handler}->send_event($event);
    return 1;
}

sub process
{
    my $self          = shift;

    if ( $self->response_data =~ /\S/ ) {
        my $event = PCVSLib::ErrorMessageEvent->new($self->response_data);
        $self->{event_handler}->send_event($event);
    }
}

package PCVSLib::ValidRequestsResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "Valid-requests");

sub process
{
    my $self          = shift;

    my $event = PCVSLib::ValidRequestsEvent->new($self->response_data);
    $self->{event_handler}->send_event($event);
}

package PCVSLib::CheckedInResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "Checked-in");

sub process
{
    my $self          = shift;

    my $event = PCVSLib::CheckedInEvent->new($self->response_data);
    $self->{event_handler}->send_event($event);
}

package PCVSLib::NewEntryResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "New-entry");

use Carp;

sub process
{
    my $self          = shift;

    carp("Not yet implemented");
}

package PCVSLib::CheckSumResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "Checksum");

use Carp;

sub process
{
    my $self          = shift;

    carp("Not yet implemented");

}

package PCVSLib::CopyFileResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "Copy-file");

sub process
{
    my $self          = shift;

    my $event = PCVSLib::CopyFileEvent->new($self->response_data);
    $self->{event_handler}->send_event($event);
}

package PCVSLib::UpdatedResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "Updated");

sub process
{
    my $self          = shift;

    my $event = PCVSLib::UpdatedEvent->new($self->response_data);
    $self->{event_handler}->send_event($event);
}

package PCVSLib::CreatedResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "Created");

sub process
{
    my $self          = shift;

    my $event = PCVSLib::CreatedEvent->new($self->response_data);
    $self->{event_handler}->send_event($event);
}

package PCVSLib::UpdateExistingResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "Update-existing");

sub process
{
    my $self          = shift;

    my $event = PCVSLib::UpdateExistingEvent->new($self->response_data);
    $self->{event_handler}->send_event($event);
}

package PCVSLib::MergedResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "Merged");

sub process
{
    my $self          = shift;

    my $event = PCVSLib::MergedEvent->new($self->response_data);
    $self->{event_handler}->send_event($event);
}

use Carp;

package PCVSLib::PatchedResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "Patched");

sub process
{
    my $self          = shift;

    carp("Not yet implemented");

}

use Carp;

package PCVSLib::RcsDiffResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "Rcs-diff");

sub process
{
    my $self          = shift;

    carp("Not yet implemented");

}

package PCVSLib::ModeResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "Mode");

sub process
{
    my $self          = shift;

    my $event = PCVSLib::ModeEvent->new($self->response_data);
    $self->{event_handler}->send_event($event);

}

package PCVSLib::ModTimeResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "Mod-time");

sub process
{
    my $self          = shift;

    my $event = PCVSLib::ModTimeEvent->new($self->response_data);
    $self->{event_handler}->send_event($event);
}

package PCVSLib::RemovedResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "Removed");

use Carp;

sub process
{
    my $self          = shift;

    carp("Not yet implemented");

}

package PCVSLib::RemoveEntryResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "Remove-entry");

sub process
{
    my $self          = shift;

    my $event = PCVSLib::RemoveEntryEvent->new($self->response_data);
    $self->{event_handler}->send_event($event);
}

package PCVSLib::SetStaticDirectoryResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "Set-static-directory");

sub process
{
    my $self          = shift;

    my $event = PCVSLib::SetStaticDirectoryEvent->new($self->response_data);
    $self->{event_handler}->send_event($event);
}

package PCVSLib::ClearStaticDirectoryResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "Clear-static-directory");

sub process
{
    my $self          = shift;

    my $event = PCVSLib::ClearStaticDirectoryEvent->new($self->response_data);
    $self->{event_handler}->send_event($event);
}

package PCVSLib::SetStickyResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "Set-sticky");

sub process
{
    my $self          = shift;

    my $event = PCVSLib::SetStickyEvent->new($self->response_data);
    $self->{event_handler}->send_event($event);
}

package PCVSLib::ClearStickyResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "Clear-sticky");

sub process
{
    my $self          = shift;

    my $event = PCVSLib::ClearStickyEvent->new($self->response_data);
    $self->{event_handler}->send_event($event);
}

#package PCVSLib::TemplateResponse;
#use vars('@ISA');
#@ISA=('PCVSLib::Response');
#push(@{$ResponseClassData{valid_responses}}, "Template");

#use Carp;

#sub process
#{
#    my $self          = shift;
#
#    carp("Not yet implemented");
#
#}

package PCVSLib::NotifiedResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "Notified");

use Carp;

sub process
{
    my $self          = shift;

    carp("Not yet implemented");

}

package PCVSLib::ModuleExpansionResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "Module-expansion");

sub process
{
    my $self          = shift;

    my $event = PCVSLib::ModuleExpansionEvent->new($self->response_data);
    $self->{event_handler}->send_event($event);
}

package PCVSLib::WrapperRcsOptionsResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "Wrapper-rcsOption");

use Carp;

sub process
{
    my $self          = shift;

    carp("Not yet implemented");

}

package PCVSLib::MessageResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "M");

sub process
{
    my $self          = shift;

    my $event = PCVSLib::MessageEvent->new($self->response_data);
    $self->{event_handler}->send_event($event);
}

package PCVSLib::MbinaryResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "Mbinary");

use Carp;

sub process
{
    my $self          = shift;

    carp("Not yet implemented");

}

package PCVSLib::ErrorMessageResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "E");

sub process
{
    my $self          = shift;

    my $event = PCVSLib::ErrorMessageEvent->new($self->response_data);
    $self->{event_handler}->send_event($event);
}

package PCVSLib::FlushResponse;
use vars('@ISA');
@ISA=('PCVSLib::Response');
push(@{$ResponseClassData{valid_responses}}, "F");

use Carp;

sub process
{
    my $self          = shift;

    # Ignore flush responses.

}
1;
# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:
