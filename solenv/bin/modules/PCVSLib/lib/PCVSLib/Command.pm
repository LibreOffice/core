#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: Command.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: vg $ $Date: 2007-08-27 13:33:23 $
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
# Command.pm - package Command encapsulates the process of sending a command
#              to the CVS server
#

package PCVSLib::Command;

use Carp;
use File::Basename;
use IO::Dir;

use strict;
use warnings;

use PCVSLib::Request;
use PCVSLib::Response;
use PCVSLib::Listener;
use PCVSLib::PathName;
use PCVSLib::File;

#### ctor ####

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};

    $self->{event_handler} = shift;
    $self->{io_handle}     = undef;
    $self->{file_list}     = undef;
    $self->{tag}           = '';
    $self->{branch}        = '';
    $self->{rev1}          = '';
    $self->{rev2}          = '';
    $self->{merge1}        = '';
    $self->{merge2}        = '';
    $self->{options}       = undef;
    $self->{comment}       = undef;
    $self->{recursive}     = 1;
    $self->{first_command} = 0;       # is this a first command
    $self->{root}          = undef;
    # private members

    bless ($self, $class);

    return $self;
}

#### instance accessors #####

for my $datum qw(event_handler io_handle file_list tag branch rev1 rev2 merge1 merge2 options comment recursive first_command root) {
    no strict "refs";
    *$datum = sub {
        my $self = shift;
        $self->{$datum} = shift if @_;
        return $self->{$datum};
    }
}

#### public methods ####

sub execute
{
    my $self = shift;

    if ( $self->{first_command} ) {
        if ( !$self->{root} ) {
            croak("PCVSLIB::Command::execute(): root not set");
        }
        my $request = PCVSLib::RootRequest->new($self->{root}->root_dir());
        my $request_string = $request->to_string();
        $self->{io_handle}->print($request_string);
        $request = PCVSLib::ValidResponsesRequest->new();
        $request_string = $request->to_string();
        $self->{io_handle}->print($request_string);
        $request = PCVSLib::UseUnchangedRequest->new();
        $request_string = $request->to_string();
        $self->{io_handle}->print($request_string);
    }
    $self->execute_command_();
}

#### private methods ####

sub process_responses_
{
    my $self = shift;
    while (1) {
        my $response
            = PCVSLib::Response->create_response($self->{event_handler}, $self->{io_handle});
        $response->process();
        last if $response->is_last();
    }
}

# default implememtation
sub execute_command_
{
    return;
}

# default implememtation
sub build_request_list_
{
    return \();
}

sub recurse_directories_
{
    my $self      = shift;
    my $directory = shift;

    my @dir_entries = ();

    if ( ! -d "$directory/CVS" ) {
        return (); # directory not managed, not interesting
    }

    my $dir = IO::Dir->new($directory);
    if ( defined($dir) ) {
        while ( $_ = $dir->read() ) {
            next if /^\.$/;
            next if /^\.\.$/;
            next if ( /^CVS$/ && -d "$directory/CVS" );
            push(@dir_entries, "$directory/$_");
        }
        $dir->close();
    }
    else {
        croak("PCVSLIB::Command::recurse_directories_(): failed to open directory '$directory': $!");
    }
    my @subdir_entries = ();
    my @all_entries = ();
    foreach ( @dir_entries ) {
        if ( -d $_ ) {
            if ( ! -d "$_/CVS" ) {
                push(@all_entries, $_); # unmanaged subdirs are going to get a ? mark
                                        # for being questionable
            }
            else {
                if ( $self->{recursive} ) {
                    push(@subdir_entries, @{$self->recurse_directories_($_)});
                }
            }
        }
        else {
            push(@all_entries, $_);
        }
    }
    push(@all_entries, @subdir_entries);
    return \@all_entries;
}

sub gather_request_files_
{
    # Returns a reference on LoL. The third element of each sub list item
    # indicates that the sub list item needs to be an explicit argument.
    # The fourth element determines if a Modified, Is-modified or UnChanged
    # request should be send
    my $self = shift;

    my @request_files_list = ();
    my %directories = ();

    foreach ( @{$self->{file_list}} ) {
        my ($name, $dir) = fileparse($_);
        chop($dir); # remove trailing slash
        if ( !-e $_ && -e "$dir/CVS" ) {
            # File could be either locally removed, obsolete on this branch
            # or just be non existant
            if ( !exists $directories{$dir} ) {
                $directories{$dir} = PCVSLib::Directory->new($dir);
            }
            push(@request_files_list, [$name, $directories{$dir}, 1, 0]);
        }
        elsif ( -f _ ) {
            if ( !exists $directories{$dir} ) {
                $directories{$dir} = PCVSLib::Directory->new($dir);
            }
            my $entry = $directories{$dir}->get_entry($name);
            push(@request_files_list, [$name, $directories{$dir}, 1, 1]);
        }
        elsif ( -d _ ) {
            foreach ( @{$self->recurse_directories_($_)} ) {
                my ($name, $dir) = fileparse($_);
                chop($dir); # remove trailing slash
                if ( !exists $directories{$dir} ) {
                    $directories{$dir} = PCVSLib::Directory->new($dir);
                }
                push(@request_files_list, [$name, $directories{$dir}, 0, 1]);
            }
            if ( !exists $directories{$dir} ) {
                $directories{$dir} = PCVSLib::Directory->new($dir);
            }
            push(@request_files_list, [$name, $directories{$dir}, 1, 0]);
        }
        else {
            croak("PCVSLIB::Command::gather_request_files_(): internal error");
       }
    }

    return \@request_files_list;
}

sub build_files_request_list_
{
    my $self = shift;
    my $send_modified = shift;

    my @request_list = ();

    my $request_files = $self->gather_request_files_();
    my $current_dir = '';

    my @arguments = ();

    foreach ( @{$request_files} ) {
        my $name               = $_->[0];
        my $dir                = $_->[1];
        my $is_argument        = $_->[2];
        my $needs_change_check = $_->[3];
        my $dir_name = $dir->directory();
        if ( $dir_name ne $current_dir ) {
            $dir_name =~ s/^\.\///; # if we got a leading ./ remove it
            my $repository = $dir->repository();
            push(@request_list, PCVSLib::DirectoryRequest->new($dir_name, $repository));
            if ( $dir->tag() ) {
                push(@request_list, PCVSLib::StickyRequest->new($dir->tag()));
            }
            if ( $dir->is_static() ) {
                push(@request_list, PCVSLib::StaticDirectoryRequest->new());
            }
            $current_dir = $dir_name;
        }
        my $entry = $dir->get_entry($name);

        if ( $entry ) {
            $entry->to_server(1); # we intend to send the entry line to the server
            if ( $entry->is_conflict() ) {
                if ($dir->is_resolved($name) ) {
                    $entry->is_resolved(1);
                }
            }
            push(@request_list, PCVSLib::EntryRequest->new($entry));
            if ( $needs_change_check ) {
                if ( $dir->is_modified($name) ) {
                    if ( $send_modified ) {
                        my $path_name = PCVSLib::PathName->new();
                        $path_name->local_directory($dir_name);
                        $path_name->name($name);
                        my $file = PCVSLib::File->new($path_name);
                        $file->entry($entry);
                        push(@request_list, PCVSLib::ModifiedRequest->new($name, undef, $file));
                    }
                    else {
                        push(@request_list, PCVSLib::IsModifiedRequest->new($name));
                    }
                }
                else {
                    push(@request_list, PCVSLib::UnchangedRequest->new($name));
                }
            }
        }
        else {
            if ( $name ne '.' ) {
                push(@request_list, PCVSLib::QuestionableRequest->new($name));
            }
        }

        if ( $is_argument ) {
            push(@arguments, $dir_name eq '.' ? $name : "$dir_name/$name");
        }

    }
    # The last DirectoryRequest must be for the working dir. If current_dir is
    # not the working dir, push working dir on @request_list (possibly for the
    # second time)
    if ( $current_dir ne '.' ) {
        my $working_dir = PCVSLib::Directory->new('.');
        my $repository = $working_dir->repository();
        push(@request_list, PCVSLib::DirectoryRequest->new('.', $repository));
        if ( $working_dir->tag() ) {
            push(@request_list, PCVSLib::StickyRequest->new($working_dir->tag()));
        }
    }
    # Finally push file arguments on @request_list
    foreach ( @arguments ) {
        push(@request_list, PCVSLib::ArgumentRequest->new($_));
    }
    return \@request_list;
}


sub send_request_
{
    my $self    = shift;
    my $request = shift;

    my $request_string = $request->to_string();
    $self->{io_handle}->print($request_string);

    if ( $request->additional_data_attached() ) {
        my $additional_data_string = $request->additional_data_to_string();
        $self->{io_handle}->print($additional_data_string);
    }

    if ( $request->file_attached() ) {
        my $file = $request->file();
        $file->io_handle($self->{io_handle});
        $file->read_and_send();
    }

    if ( $request->response_expected() ) {
        $self->process_responses_();
    }
}


#### derived classes ####

package PCVSLib::VersionCommand;
use vars('@ISA');
@ISA=('PCVSLib::Command');

sub execute_command_ {
    my $self = shift;

    my $request = PCVSLib::VersionRequest->new();
    $self->send_request_($request);
}

package PCVSLib::ValidRequestsCommand;
use vars('@ISA');
@ISA=('PCVSLib::Command');

sub execute_command_ {
    my $self = shift;

    my $request = PCVSLib::ValidRequestsRequest->new();
    $self->send_request_($request);
}

package PCVSLib::ValidResponsesCommand;
use vars('@ISA');
@ISA=('PCVSLib::Command');

sub execute_command_ {
    my $self = shift;

    my $request = PCVSLib::ValidResponsesRequest->new();
    $self->send_request_($request);
}

package PCVSLib::StatusCommand;
use vars('@ISA');
@ISA=('PCVSLib::Command');

sub execute_command_ {
    my $self = shift;

    my $send_modified = 1; # we are going to play safe, send Modified insetad of Is-Modified

    my @request_list = ();
    foreach ( @{$self->{options}} ) {
        push(@request_list, PCVSLib::ArgumentRequest->new($_));
    }
    my $request_list = $self->build_files_request_list_($send_modified);
    foreach ( @{$request_list} ) {
        $self->send_request_($_);
    }
    my $request = PCVSLib::StatusRequest->new();
    $self->send_request_($request);
}

package PCVSLib::RDiffCommand;
use vars('@ISA');
@ISA=('PCVSLib::Command');

sub execute_command_ {
    my $self = shift;

    my $request_list = $self->build_request_list_();
    foreach ( @{$request_list} ) {
        $self->send_request_($_);
    }
    my $request = PCVSLib::RDiffRequest->new();
    $self->send_request_($request);
}

sub build_request_list_
{
    my $self = shift;

    if ( !$self->{rev1} ) {
        croak("PCVSLIB::RDiffCommand::build_request_list_(): revsion rev1 not set");
    }
    if ( !$self->{rev2} ) {
        croak("PCVSLIB::RDiffCommand::build_request_list_(): revsion rev2 not set");
    }
    if ( !@{$self->{file_list}} ) {
        croak("PCVSLIB::RDiffCommand::build_request_list_(): no module specfied");
    }

    my @request_list = ();
    foreach ( @{$self->{options}} ) {
        push(@request_list, PCVSLib::ArgumentRequest->new($_));
    }
    push(@request_list, PCVSLib::ArgumentRequest->new('-r'));
    push(@request_list, PCVSLib::ArgumentRequest->new($self->{rev1}));
    push(@request_list, PCVSLib::ArgumentRequest->new('-r'));
    push(@request_list, PCVSLib::ArgumentRequest->new($self->{rev2}));
    foreach ( @{$self->{file_list}} ) {
        push(@request_list, PCVSLib::ArgumentRequest->new($_));
    }
    return \@request_list;
}

package PCVSLib::RLogCommand;
use vars('@ISA');
@ISA=('PCVSLib::Command');

sub execute_command_ {
    my $self = shift;

    my $request_list = $self->build_request_list_();
    foreach ( @{$request_list} ) {
        $self->send_request_($_);
    }
    my $request = PCVSLib::RLogRequest->new();
    $self->send_request_($request);
}

sub build_request_list_
{
    my $self = shift;

    if ( !@{$self->{file_list}} ) {
        croak("PCVSLIB::RLogCommand::build_request_list_(): no module specfied");
    }

    my @request_list = ();
    foreach ( @{$self->{options}} ) {
        push(@request_list, PCVSLib::ArgumentRequest->new($_));
    }
    foreach ( @{$self->{file_list}} ) {
        push(@request_list, PCVSLib::ArgumentRequest->new($_));
    }
    return \@request_list;
}

package PCVSLib::LogCommand;
use vars('@ISA');
@ISA=('PCVSLib::Command');

sub execute_command_ {
    my $self = shift;

    my $send_modified = 0; # go easy, use  Is-Modified
    my $request_list = $self->build_files_request_list_($send_modified);
    foreach ( @{$request_list} ) {
        $self->send_request_($_);
    }
    my $request = PCVSLib::LogRequest->new();
    $self->send_request_($request);
}

package PCVSLib::CheckoutCommand;
use vars('@ISA');
@ISA=('PCVSLib::Command');

use Carp;

sub execute_command_ {
    my $self = shift;

    my $update_listener = PCVSLib::InternalListener->new();
    $update_listener->root($self->{root});
    $self->{event_handler}->add_listener($update_listener);


    my $request_list = $self->build_request_list_();
    foreach ( @{$request_list} ) {
        $self->send_request_($_);
    }
    $self->{event_handler}->remove_listener($update_listener);
}

sub build_request_list_
{
    my $self = shift;

    if ( $self->{tag} && $self->{branch} ) {
        croak("PCVSLIB::CheckoutCommand::build_request_list_(): either tag or branch label can be set, not both");
    }

    my @request_list = ();

    if ( $self->{tag} || $self->{branch} ) {
        push(@request_list, PCVSLib::ArgumentRequest->new('-P')); # prune
        push(@request_list, PCVSLib::ArgumentRequest->new('-r'));
        push(@request_list, PCVSLib::ArgumentRequest->new($self->{tag} ? $self->{tag} : $self->{branch} ));
        push(@request_list, PCVSLib::ArgumentRequest->new('--'));
    }
    foreach ( @{$self->{file_list}} ) {
        if ( -e $_ ) {
            # Implement with ExpandModuleRequest etc
            croak("PCVSLIB::CheckoutCommand::build_checkout_request_list_(): checkout of already exiting modules/directories/files not yet supported");
        }
        push(@request_list, PCVSLib::ArgumentRequest->new($_));
    }

    push(@request_list, PCVSLib::DirectoryRequest->new('.', $self->{root}->root_dir() ));
    push(@request_list, PCVSLib::CheckoutRequest->new());

    return \@request_list;
}

package PCVSLib::TagCommand;
use vars('@ISA');
@ISA=('PCVSLib::Command');

use Carp;

sub execute_command_ {
    my $self = shift;

    my $request_list = $self->build_request_list_();
    foreach ( @{$request_list} ) {
        $self->send_request_($_);
    }
}

sub build_request_list_
{
    my $self = shift;

    my @request_list = ();

    if ( $self->{tag} && $self->{branch} ) {
        croak("PCVSLIB::TagCommand::build_request_list_(): either tag or branch label must be set, not both");
    }

    if ( !$self->{tag} && !$self->{branch} ) {
        croak("PCVSLIB::TagCommand::build_request_list_(): tag or branch label not set");
    }

    foreach ( @{$self->{options}} ) {
        push(@request_list, PCVSLib::ArgumentRequest->new($_));
    }
    if ( !$self->{recursive} ) {
        push(@request_list, PCVSLib::ArgumentRequest->new('-l'));
    }

    if ( $self->{branch} ) {
        push(@request_list, PCVSLib::ArgumentRequest->new('-b'));
        push(@request_list, PCVSLib::ArgumentRequest->new('--'));
        push(@request_list, PCVSLib::ArgumentRequest->new($self->{branch}));
    }
    else {
        push(@request_list, PCVSLib::ArgumentRequest->new($self->{tag}));
    }
    my $send_modified = 0; # Using Is-modified is good enough as long the '-c' option is not used
    for ( @{$self->{options}} ) {
        $send_modified++ if /-c/;
    }
    my $file_request_list = $self->build_files_request_list_($send_modified);
    push(@request_list, @{$file_request_list});
    push(@request_list,  PCVSLib::TagRequest->new());

    return \@request_list;
}

package PCVSLib::CommitCommand;
use vars('@ISA');
@ISA=('PCVSLib::Command');

use Carp;

sub execute_command_ {
    my $self = shift;

    my $ci_listener = PCVSLib::InternalListener->new();
    $self->{event_handler}->add_listener($ci_listener);

    my $request_list = $self->build_request_list_();
    foreach ( @{$request_list} ) {
        $self->send_request_($_);
    }
    $self->{event_handler}->remove_listener($ci_listener);
}

sub build_request_list_
{
    my $self = shift;

    my $send_modified = 1;
    my @request_list = ();

    foreach ( @{$self->{options}} ) {
        push(@request_list, PCVSLib::ArgumentRequest->new($_));
    }

    if ( !$self->{recursive} ) {
        push(@request_list, PCVSLib::ArgumentRequest->new('-l'));
    }

    if ( !$self->{comment} ) {
        croak("PCVSLIB::CommitCommand::build_request_list_(): commit comment missing");
    }
    else {
        push(@request_list, PCVSLib::ArgumentRequest->new('-m'));
        my $line = shift @{$self->{comment}};
        push(@request_list, PCVSLib::ArgumentRequest->new($line));
        foreach (@{$self->{comment}}) {
            push(@request_list, PCVSLib::ArgumentxRequest->new($_));
        }
    }

    my $file_request_list = $self->build_files_request_list_($send_modified);
    push(@request_list, @{$file_request_list});
    # TODO: check for conflict markers
    push(@request_list,  PCVSLib::CiRequest->new());

    return \@request_list;
}

package PCVSLib::UpdateCommand;
use vars('@ISA');
@ISA=('PCVSLib::Command');

use Carp;

sub execute_command_ {
    my $self = shift;

    my $update_listener = PCVSLib::InternalListener->new();
    $self->{event_handler}->add_listener($update_listener);

    my $request_list = $self->build_request_list_();
    foreach ( @{$request_list} ) {
        $self->send_request_($_);
    }
    $self->{event_handler}->remove_listener($update_listener);
}

sub build_request_list_
{
    my $self = shift;

    my $send_modified = 1;
    my @request_list = ();

    foreach ( @{$self->{options}} ) {
        push(@request_list, PCVSLib::ArgumentRequest->new($_));
    }

    if ( !$self->{recursive} ) {
        push(@request_list, PCVSLib::ArgumentRequest->new('-l'));
    }

    # sanity checks
    if ( $self->{tag} && $self->{branch} ) {
        croak("PCVSLIB::UpdateCommand::build_request_list_(): can update to either tag or branch, but not both");
    }

    if ( $self->{tag} && ($self->{merge1} || $self->{merge2}) ) {
        croak("PCVSLIB::UpdateCommand::build_request_list_(): can update to either tag or merge but not both");
    }

    if ( $self->{branch} && ($self->{merge1} || $self->{merge2}) ) {
        croak("PCVSLIB::UpdateCommand::build_request_list_(): can update to either branch or merge but not both");
    }

    if ( $self->{merge2} && !$self->{merge1} ) {
        croak("PCVSLIB::UpdateCommand::build_request_list_(): inconsistent specification of merge revisisons");
    }

    # revision arguments for update and merge
    if ( $self->{tag} ) {
        push(@request_list, PCVSLib::ArgumentRequest->new('-r'));
        push(@request_list, PCVSLib::ArgumentRequest->new($self->{tag}));
    }

    if ( $self->{branch} ) {
        push(@request_list, PCVSLib::ArgumentRequest->new('-r'));
        push(@request_list, PCVSLib::ArgumentRequest->new($self->{branch}));
    }

    if ( $self->{merge1} ) {
        push(@request_list, PCVSLib::ArgumentRequest->new('-j'));
        push(@request_list, PCVSLib::ArgumentRequest->new($self->{merge1}));
    }

    if ( $self->{merge2} ) {
        push(@request_list, PCVSLib::ArgumentRequest->new('-j'));
        push(@request_list, PCVSLib::ArgumentRequest->new($self->{merge2}));
    }

    my $file_request_list = $self->build_files_request_list_($send_modified);
    push(@request_list, @{$file_request_list});
    push(@request_list,  PCVSLib::UpdateRequest->new());

    return \@request_list;
}

package PCVSLib::RemoveCommand;
use vars('@ISA');
@ISA=('PCVSLib::Command');

use Carp;

sub execute_command_ {
    my $self = shift;

    my $remove_listener = PCVSLib::InternalListener->new();
    $self->{event_handler}->add_listener($remove_listener);

    my $request_list = $self->build_request_list_();
    foreach ( @{$request_list} ) {
        $self->send_request_($_);
    }
    $self->{event_handler}->remove_listener($remove_listener);
}

sub build_request_list_
{
    my $self = shift;

    my $send_modified = 0;
    my @request_list = ();

    my $force_removal = 0;

    foreach ( @{$self->{options}} ) {
        if ( /-f/ ) {
            $force_removal++;
        }
    }

    if ( $force_removal && $self->{recursive} ) {
        croak("PCVSLIB::RemoveCommand::build_request_list_(): for removal: option -f and recursive operation are mutually exclusive");
    }

    if ( $force_removal ) {
        foreach ( @{$self->{file_list}} ) {
            if ( !unlink($_) ) {
                croak("PCVSLIB::RemoveCommand::build_request_list_(): can't unlink file '$_': $!");
            }
        }
    }

    my $file_request_list = $self->build_files_request_list_($send_modified);
    push(@request_list, @{$file_request_list});
    # TODO: check for conflict markers
    push(@request_list,  PCVSLib::RemoveRequest->new());

    return \@request_list;
}

1;
# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:
