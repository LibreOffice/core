#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: CwsCvsOps.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: vg $ $Date: 2007-08-27 13:32:05 $
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
# CwsCvsOps.pm - CVS operations on CWSs implemented in terms of PCVSLib
#

package CwsCvsOps;
use strict;

use Carp;
use File::Find;
use File::Path;
use Cwd;
use IO::File;
use PCVSLib;
use CwsConfig;

##### ctor ####

sub new
{
    my $invocant      = shift;
    my $config        = shift;
    my $server_type   = shift; # 'remote' (OOo), 'local', 'directory'
    my $module_or_dir = shift;
    my $log_handle    = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    # private
    $self->{mclient_}       = undef; # Mirror client if available
    $self->{client_}        = undef;
    $self->{is_old_client_} = undef;
    $self->{server_type_}   = undef;
    $self->{module_or_dir_} = undef;
    $self->{config_}        = undef;
    $self->{event_handler_} = PCVSLib::EventHandler->new();
    $self->{log_handle_}    = undef;
    bless ($self, $class);

    if ( defined($server_type) ) {
        $self->{server_type_} = $server_type
    }
    else {
        croak("CwsCvsOps::new(): internal error: server type not set");
    }
    if ( defined($config) ) {
        $self->{config_} = $config;
    }
    else {
        croak("CwsCvsOps::new(): internal error: config not set");
    }
    if ( defined($module_or_dir) ) {
        $self->{module_or_dir_} = $module_or_dir;
    }
    if ( defined($log_handle) ) {
        $self->{log_handle_} = $log_handle;
    }
    return $self;
}

# Find all changed files in a module vs. a specfic tag
# return a LoL: [name of file, rev_old, rev_new].
# It's also possible to specify a single file
sub get_changed_files
{
    my $self           = shift;
    my $module_or_file = shift;
    my $rev1           = shift;
    my $rev2           = shift;

    if ( !defined($self->{client_}) ) {
        $self->{client_} = $self->create_client_('client');
    }

    my $is_file = 0;
    if ( $module_or_file =~ /\// ) {
        $is_file = 1;
    }

    my $listener = CwsCvsRdiffListener->new();
    my $event_handler = $self->{event_handler_};
    $event_handler->add_listener($listener);
    my $rdiff_command = PCVSLib::RDiffCommand->new($event_handler);
    $rdiff_command->options(['-s']);
    $rdiff_command->rev1($rev1);
    $rdiff_command->rev2($rev2);
    $rdiff_command->file_list([$module_or_file]);
    STDOUT->autoflush(1);
    $self->{client_}->execute_command($rdiff_command);
    STDOUT->autoflush(0);
    print "\n";

    if ( !$listener->is_success() ) {
        $listener->print_log();
        if ( $is_file ) {
            croak("CwsCvsOps::get_changed_files(): file '$module_or_file': rdiff -r$rev1 -r$rev2 operation failed");
        }
        else {
            croak("CwsCvsOps::get_changed_files(): module '$module_or_file': rdiff -r$rev1 -r$rev2 operation failed");
        }

    }
    my @changed_files;
    my $diff_ref = $listener->get_diff();
    my $module_prefix = $listener->get_module_prefix();
    if ( !$is_file && !$module_prefix ) {
        croak("CwsCvsOps::get_changed_files(): internal error, undefined module prefix");
    }
    foreach (@{$diff_ref}) {
        if ( /^File (.+?) / ) {
            my $file_name = $1;
            my ($rev_old, $rev_new);
            if ( /changed from revision ([\d\.]+) to ([\d\.]+)/ ) {
                $rev_old = $1;
                $rev_new = $2;
            }
            elsif ( /is new; current revision ([\d\.]+)/ ) {
                $rev_new = $1;
                $rev_old = undef;
            }
            # since cvs-1.12
            elsif ( /is new; \w+ revision ([\d\.]+)/ ) {
                $rev_new = $1;
                $rev_old = undef;
            }
            elsif ( /is removed; not included in release tag/ ) {
                $rev_new = undef;
                $rev_old = undef;
            }
            # since cvs-1.12
            elsif ( /is removed; \w+ revision ([\d\.]+)/ ) {
                $rev_new = undef;
                $rev_old = undef;
            }
            else {
                croak("ERROR: CwsCvsOps::get_changed_files(): unexpected output from rdiff command");
            }
            if ( !$is_file ) {
                $file_name =~ s/^$module_prefix\///;
            }
            push(@changed_files, [$file_name, $rev_old, $rev_new]);
        }
    }
    $event_handler->remove_listener($listener);

    return wantarray ? @changed_files : \@changed_files;
}

sub checkout
{
    my $self         = shift;
    my $dir          = shift;
    my $module       = shift;
    my $filelist_ref = shift;
    my $tag          = shift;

    my $from_mirror = 0;

    # set working directory to checkout directory
    my $savedir = cwd();
    if ( !chdir($dir) ) {
        croak("ERROR: CwsCvsOps::checkout(): can't chdir() into directory '$dir'");
    }
    if ( !defined($module) ) {
        croak("ERROR: CwsCvsOps::checkout(): module not set");
    }
    if ( defined($filelist_ref) && !@{$filelist_ref} ) {
        croak("ERROR: CwsCvsOps::checkout(): empty file list");
    }
    if ( !defined($self->{client_}) ) {
        $self->{client_} = $self->create_client_('client');
    }
    if ( $self->{server_type_} eq 'remote' && $self->{config_}->cvs_mirror_root() ) {
        $from_mirror = 1;
        if ( !defined($self->{mclient_}) ) {
            $self->{mclient_} = $self->create_client_('mclient');
        }
    }
    if ( $from_mirror ) {
        my $count = $self->do_checkout($self->{mclient_}, $module, $filelist_ref, $tag);
        if ( !$count ) {
            # If no file is returned from the mirror it might be due to
            # the fact that the tag has not yet propagated to the mirror.
            # Retry from remote server

            # Clean up first
            if ( !rmtree($module) ) {
                # now this shouldn't really ever happen ...
                croak("ERROR: CwsCvsOps::checkout(): can't remove directory '$module': $!");
            }
            $count = $self->do_checkout($self->{client_}, $module, $filelist_ref, $tag);
        }
        else {
            my $mirror = $self->{config_}->cvs_mirror_root();
            $mirror =~ /^.*\@(.*):/;
            $mirror = $1;
            my $server = $self->{config_}->cvs_server_root();
            $server =~ /^.*\@(.*):/;
            $server = $1;
            $self->patch_admin_files($module, $mirror, $server);
            my $codir = cwd();
            if ( !chdir($module) ) {
                croak("ERROR: CwsCvsOps::checkout(): can't chdir() into directory '$savedir/$module'");
            }
            $self->do_update($self->{client_}, '.', $tag);
            if ( !chdir($codir) ) {
                croak("ERROR: CwsCvsOps::checkout(): can't chdir() into directory '$savedir'");
            }

        }
    }
    else {
        my $count = $self->do_checkout($self->{client_}, $module, $filelist_ref, $tag);
    }
    # reset working directory
    if ( !chdir($savedir) ) {
        croak("ERROR: CwsCvsOps::checkout(): can't chdir() into directory '$savedir'");
    }
}

sub remove_files
{
    my $self         = shift;
    my $filelist_ref = shift;

    if ( ! @{$filelist_ref} ) {
        croak("CwsCvsOps::remove_files(): empty file list");
    }
    if ( !defined($self->{client_}) ) {
        $self->{client_} = $self->create_client_('client');
    }
    my $listener = CwsCvsSimpleListener->new();
    my $event_handler = $self->{event_handler_};
    $event_handler->add_listener($listener);
    my $remove_command = PCVSLib::RemoveCommand->new($event_handler);
    $remove_command->file_list($filelist_ref);
    $self->{client_}->execute_command($remove_command);

    if ( !$listener->is_success() ) {
        $listener->print_log();
        my $file_str = join(",", @{$filelist_ref});
        croak("CwsCvsOps::remove_files(): removing file(s) '$file_str' failed");
    }
    $event_handler->remove_listener($listener);
}

sub stati
{
    my $self         = shift;
    my $filelist_ref = shift;

    if ( !defined($self->{client_}) ) {
        $self->{client_} = $self->create_client_('client');
    }
    my $listener = CwsCvsStatusListener->new();
    my $event_handler = $self->{event_handler_};
    $event_handler->add_listener($listener);
    my $status_command = PCVSLib::StatusCommand->new($event_handler);
    $status_command->file_list($filelist_ref);
    $self->{client_}->execute_command($status_command);

    if ( !$listener->is_success() ) {
        $listener->print_log();
        my $file_str = join(",", @{$filelist_ref});
        croak("CwsCvsOps::stati(): fetching status for file(s) '$file_str' failed");
    }
    $event_handler->remove_listener($listener);
    return $listener->get_stati();
}

sub tag_files
{
    my $self         = shift;
    my $filelist_ref = shift;
    my $tag          = shift;
    my $is_branch    = shift;
    my $revision     = shift;

    if ( !defined($self->{client_}) ) {
        $self->{client_} = $self->create_client_('client');
    }
    my $listener = CwsCvsTagListener->new();
    my $event_handler = $self->{event_handler_};
    $event_handler->add_listener($listener);
    my $tag_command = PCVSLib::TagCommand->new($event_handler);
    $tag_command->tag($tag);
    my @options;
    push(@options, '-F');
    if ( $is_branch ) {
        if ( !$self->{is_old_client_} ) {
            push(@options, '-B');
        }
        push(@options, '-b');
    }
    if ( defined($revision) ) {
        push(@options, "-r$revision");
    }
    $tag_command->options(\@options);
    $tag_command->file_list($filelist_ref);
    STDOUT->autoflush(1);
    $self->{client_}->execute_command($tag_command);
    STDOUT->autoflush(0);
    print "\n";

    if ( !$listener->is_success() ) {
        $listener->print_log();
        my $file_str = join(",", @{$filelist_ref});
        croak("CwsCvsOps::tag_files(): tag file(s) '$file_str' with tag '$tag' failed");
    }
    $event_handler->remove_listener($listener);
    return $listener->get_tagged_files();
}

sub commit_files
{
    my $self         = shift;
    my $filelist_ref = shift;
    my $comment_ref  = shift;

    if ( !defined($self->{client_}) ) {
        $self->{client_} = $self->create_client_('client');
    }
    my $listener = CwsCvsSimpleListener->new();
    my $event_handler = $self->{event_handler_};
    $event_handler->add_listener($listener);
    my $commit_command = PCVSLib::CommitCommand->new($event_handler);
    $commit_command->comment($comment_ref);
    $commit_command->file_list($filelist_ref);
    STDOUT->autoflush(1);
    $self->{client_}->execute_command($commit_command);
    STDOUT->autoflush(0);

    # The nice thing about commits is they either succeed or fail completely.
    # We can't check counts here, because a 'null' commit does not generate
    # a response, but it's no error either.
    if ( !$listener->is_success() ) {
        $listener->print_log();
        my $file_str = join(",", @{$filelist_ref});
        croak("CwsCvsOps::commit_files(): commit file(s) '$file_str' failed");
    }
    $event_handler->remove_listener($listener);
    return;
}

sub get_collected_logs
{
    my $self         = shift;
    my $filelist_ref = shift;
    my $branch       = shift;

    if ( !defined($self->{client_}) ) {
        $self->{client_} = $self->create_client_('client');
    }
    my $listener = CwsCvsLogListener->new();
    my $event_handler = $self->{event_handler_};
    $event_handler->add_listener($listener);
    my $log_command = PCVSLib::LogCommand->new($event_handler);
    $log_command->file_list($filelist_ref);
    my @options;
    $log_command->options(['-N',"-r$branch"]);
    $self->{client_}->execute_command($log_command);

    if ( !$listener->is_success() ) {
        $listener->print_log();
        my $file_str = join(",", @{$filelist_ref});
        croak("CwsCvsOps::logs(): fetching logs for file(s) '$file_str' failed");
    }
    $event_handler->remove_listener($listener);
    return $listener->get_collected_logs();
}

sub do_checkout
{
    my $self         = shift;
    my $client       = shift;
    my $module       = shift;
    my $filelist_ref = shift;
    my $tag          = shift;

    my $listener = CwsCvsUpdateListener->new();
    my $event_handler = $self->{event_handler_};
    $event_handler->add_listener($listener);
    my $checkout_command = PCVSLib::CheckoutCommand->new($event_handler);
    $checkout_command->tag($tag);
    my @files;
    if ( defined($filelist_ref) ) {
        foreach (@{$filelist_ref}) {
            push(@files, "$module/$_");
        }
    }
    else {
        @files = ($module);
    }
    $checkout_command->file_list([@files]);
    STDOUT->autoflush(1);
    $client->execute_command($checkout_command);
    STDOUT->autoflush(0);
    if ( !$listener->is_success() ) {
        $listener->print_log();
        my $root = $client->connection()->root();
        croak("CwsCvsOps::do_checkout(): checkout (module '$module, tag $tag) from CVS '$root' failed");
    }
    my $count = $listener->get_count();
    if ( $count ) {
        print "\n";
    }
    $event_handler->remove_listener($listener);
    return $count;
}

sub do_update
{
    my $self         = shift;
    my $client       = shift;
    my $module       = shift;
    my $tag          = shift;

    my $listener = CwsCvsUpdateListener->new();
    my $event_handler = $self->{event_handler_};
    $event_handler->add_listener($listener);
    my $update_command = PCVSLib::UpdateCommand->new($event_handler);
    $update_command->tag($tag);
    $update_command->file_list([$module]);
    STDOUT->autoflush(1);
    $client->execute_command($update_command);
    STDOUT->autoflush(0);
    print "\n";
    if ( !$listener->is_success() ) {
        $listener->print_log();
        my $root = $client->connection()->root();
        croak("CwsCvsOps::do_update(): update (module '$module, tag $tag) from CVS '$root' failed");
    }
    $event_handler->remove_listener($listener);
}

sub merge_files
{
    my $self         = shift;
    my $filelist_ref = shift;
    my $merge1       = shift;
    my $merge2       = shift;

    if ( !defined($self->{client_}) ) {
        $self->{client_} = $self->create_client_('client');
    }
    my $listener = CwsCvsMergeListener->new();
    my $event_handler = $self->{event_handler_};
    $event_handler->add_listener($listener);
    my $merge_command = PCVSLib::UpdateCommand->new($event_handler);
    $merge_command->merge1($merge1);
    $merge_command->merge2($merge2);
    $merge_command->options(['-kk']);
    $merge_command->file_list($filelist_ref);
    STDOUT->autoflush(1);
    $self->{client_}->execute_command($merge_command);
    STDOUT->autoflush(0);
    print "\n";
    if ( !$listener->is_success() ) {
        $listener->print_log();
        my $file_str = join(",", @{$filelist_ref});
        croak("CwsCvsOps::merge_files(): merge for file(s) '$file_str' failed");
    }
    $event_handler->remove_listener($listener);
    # sanity check
    my $n_files = @{$filelist_ref};
    if ( $listener->get_count() != $n_files ) {
        croak("CwsCvsOps::merge_files(): INTERNAL ERROR: merge count differing");
    }

    return ($listener->get_conflicts(), $listener->get_already_merged());
}

sub get_relative_path
{
    my $self      = shift;
    my $directory = shift;

    if ( !defined($self->{client_}) ) {
        $self->{client_} = $self->create_client_('client');
    }

    if ( $self->{server_type_} ne 'directory' ) {
        croak("CwsCvsOps::merge_files(): INTERNAL ERROR: method can only be used with server_type 'directory'");
    }

    my $dir = PCVSLib::Directory->new($directory);

    my $root = $dir->root()->root_dir();
    my $repository = $dir->repository();

    $repository =~ s/$root\///;

    return $repository;
}


sub patch_admin_files
{
    my $self   = shift;
    my $module = shift;
    my $old    = shift;
    my $new    = shift;

    my @cvs_dirs;
    find sub { push @cvs_dirs, $File::Find::name if -d _ && /CVS$/}, $module;
    foreach my $cvs_dir (@cvs_dirs) {
        my $root_file = "$cvs_dir/Root";
        next if (!-f $root_file);
        my $ih = IO::File->new( "<$cvs_dir/Root");
        if ( !defined($ih) ) {
            croak("ERROR: CwsCvsOps::patch_admin_files: can't read file '$root_file'");
        }
        my $line = <$ih>;
        $ih->close();

        # patch root
        if ( !($line =~ s/$old/$new/) ) {
            croak ("ERROR: CwsCvsOps::patch_admin_files(): file '$root_file' has wrong format");
        }
        my $oh = IO::File->new( ">$cvs_dir/Root");
        if ( !defined($oh)  ) {
            croak("ERROR: CwsCvsOps::patch_admin_files: can't write file '$root_file'");
        }
        $oh->print($line);
        $oh->close();
    }
}

sub shutdown
{
    my $self = shift;

    if ( defined($self->{mclient_}) ) {
        $self->{mclient_}->connection()->close();
    }
    if ( defined($self->{client_}) ) {
        $self->{client_}->connection()->close();
    }
}

# private methods

sub create_client_
{
    my $self        = shift;
    my $client_type = shift;

    my $config = $self->{config_};
    my $server_type = $self->{server_type_};
    my $server;
    # This is of course highly specific to the OOo setup.
    if ( $server_type eq 'remote' && $client_type eq 'mclient') {
        $server = $config->cvs_mirror_root();
    }
    elsif ( $server_type eq 'remote' && $client_type eq 'client') {
        $server = $config->cvs_server_root();
    }
    elsif ( $server_type eq 'local' ) {
        $server = $config->cvs_local_root();
    }
    elsif ( $server_type eq 'directory' ) {
        my $root_file = "CVS/Root";
        my $ih = IO::File->new("<$root_file");
        if ( !defined($ih) ) {
            croak("ERROR: CwsCvsOps::create_client: can't read file '$root_file'");
        }
        my $line = <$ih>;
        $ih->close();
        chomp($line);
        $server = $line;
    }
    else {
        croak("CwsCvsOps::create_client_(): internal error: invalid server/client combination");
    }

    # Sometimes several developers work one physical CWS representation. This is difficult
    # with CVS because the "user id" is part of the Root and is saved in the administration
    # files. If the root comes from the config file it can happen that it is just
    # a placeholder for a number of developers. We override this part of the root.
    my $vcsid = $config->vcsid();
    $server =~ s/^:pserver:\w+([\@\%])/:pserver:$vcsid$1/; # allow % for cvs proxy configurations

    my $root = PCVSLib::Root->new($server);
    my $credentials = PCVSLib::Credentials->new();
    my $scrambled_password = $credentials->get_password($root);

    my $connection = PCVSLib::Connection->new($root, $scrambled_password);

    my $io_handle = $connection->open();

    if ( defined($self->{log_handle_}) ) {
        $connection->io_handle(PCVSLib::LogHandle->new($io_handle, $self->{log_handle_}));
    }

    my $client = PCVSLib::Client->new($connection);
    # Find out the abilities of the server, this is important for moving tag branches
    # Since old server may not understand a 'version' request, we ask vor the ValidRequests
    # list. If 'version' is a valid request we know that this server is old and can't cope
    # with the -B switch for maoving branch tags
    if ( $server_type eq 'directory') {
        my $listener = CwsValidRequestsListener->new();
        my $event_handler = $self->{event_handler_};
        $event_handler->add_listener($listener);
        my $valid_requests_command = PCVSLib::ValidRequestsCommand->new($event_handler);
        $client->execute_command($valid_requests_command);

        if ( !$listener->is_success() ) {
            $listener->print_log();
            croak("CwsCvsOps::create_client_(): fetching VaildRequestsLists for server '$server' failed");
        }
        $event_handler->remove_listener($listener);
        $self->{is_old_client_} = 1;
        foreach (@{$listener->get_valid_requests()}) {
            if ( /version/ ) {
                $self->{is_old_client_} = 0;
            }
        }
    }

    return $client;
}

sub DESTROY
{
    my $self = shift;
    $self->shutdown();
}

package CwsCvsListener;

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    $self->{error_buffer_}  = ();
    $self->{is_success_}    = 0;
    bless ($self, $class);
    return $self;
}

sub is_success
{
    my $self = shift;
    return $self->{is_success_};
}

# Simple minded ring buffer for keeping the last lines of error messages
{

    my $nlog_size  = 5;
    my $nindex     = 0;
    sub append_to_log
    {
        my $self = shift;
        my $line = shift;

        $nindex++;
        if ( $nindex >= $nlog_size ) {
            $nindex = 0;
        }
        $self->{error_buffer}->[$nindex] = $line;
    }

    sub print_log
    {
        my $self = shift;

        my $first = $nindex+1;
        if ( $first >= $nlog_size ) {
            $first = $first - $nlog_size;
        }
        print STDERR "===== CVS Error: last error messages from CVS server: begin =====\n";
        my $log = "";
        for (my $i = 0; $i < $nlog_size; $i++) {
            my $n = $first + $i;
            if ( $n >= $nlog_size ) {
                $n = $n - $nlog_size;
            }
            if ( $self->{error_buffer}->[$n] ) {
                print STDERR $self->{error_buffer}->[$n] ."\n";
            }
        }
        print STDERR "===== CVS Error: last error messages from CVS server: end =====\n";
    }

    sub clear_log
    {
        my $self = shift;

        $self->{error_buffer} = ();
    }
}

package CwsCvsSimpleListener;
use vars('@ISA');
@ISA=('CwsCvsListener');

sub notify
{
    my $self  = shift;
    my $event = shift;

    if ( $event->isa('PCVSLib::ErrorMessageEvent') ) {
        my $message = $event->get_message();
        $self->append_to_log($message);
    }
    if ( $event->isa('PCVSLib::TerminatedEvent') ) {
        my $is_success = $event->is_success();
        if ( $is_success ) {
            $self->clear_log();
        }
        $self->{is_success_} = $is_success;
    }
}

package CwsValidRequestsListener;
use vars('@ISA');
@ISA=('CwsCvsListener');

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    $self->{valid_requests_} = undef;
    bless ($self, $class);
    return $self;
}

sub get_valid_requests {
    my $self = shift;
    return $self->{valid_requests_};
}

sub notify
{
    my $self  = shift;
    my $event = shift;

    if ( $event->isa('PCVSLib::ValidRequestsEvent') ) {
        my $valid_requests_ref = $event->get_valid_requests();

        $self->append_to_log(join(" ", @{$valid_requests_ref}));
        $self->{valid_requests_} = $valid_requests_ref;
    }
    if ( $event->isa('PCVSLib::TerminatedEvent') ) {
        my $is_success = $event->is_success();
        if ( $is_success ) {
            $self->clear_log();
        }
        $self->{is_success_} = $is_success;
    }
}

package CwsCvsRdiffListener;
use vars('@ISA');
@ISA=('CwsCvsListener');

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    $self->{diff} = ();
    $self->{module_prefix} = undef;
    bless ($self, $class);
    return $self;
}

sub get_diff
{
    my $self = shift;
    return $self->{diff};
}

sub get_module_prefix
{
    my $self = shift;
    return $self->{module_prefix};
}

sub notify
{
    my $self  = shift;
    my $event = shift;

    if ( $event->isa('PCVSLib::ErrorMessageEvent') ) {
        my $message = $event->get_message();
        $self->append_to_log($message);
        if ( !defined $self->{module_prefix} ) {
            # Needed are file names relative to the module.
            # The very first "Diffing ... " message contains the much needed path segment
            # which needs to be stripped from the front of the file names in the rdiff
            # message events. It's not possible to just strip the module name, because the
            # file name may be prefixed with the meta project name ie ('solenv' -> 'tools/solenv').
            if ( $message =~ /cvs (?:rdiff|server): Diffing (.*)/ ) {
                $self->{module_prefix} = $1;
            }
        }
        print '.';
    }
    if ( $event->isa('PCVSLib::MessageEvent') ) {
        push(@{$self->{diff}}, $event->get_message());
    }
    if ( $event->isa('PCVSLib::TerminatedEvent') ) {
        my $is_success = $event->is_success();
        if ( $is_success ) {
            $self->clear_log();
        }
        $self->{is_success_} = $is_success;
    }
}

package CwsCvsUpdateListener;
use vars('@ISA');
@ISA=('CwsCvsListener');

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    $self->{count} = 0;
    bless ($self, $class);
    return $self;
}

sub get_count
{
    my $self = shift;
    return $self->{count};
}

sub notify
{
    my $self  = shift;
    my $event = shift;

    if ( $event->isa('PCVSLib::ErrorMessageEvent') ) {
        my $message = $event->get_message();
        $self->append_to_log($message);
    }
    if ( $event->isa('PCVSLib::MessageEvent') ) {
        print '.';
        $self->{count}++;
    }
    if ( $event->isa('PCVSLib::TerminatedEvent') ) {
        my $is_success = $event->is_success();
        if ( $is_success ) {
            $self->clear_log();
        }
        $self->{is_success_} = $is_success;
    }
}

package CwsCvsTagListener;
use vars('@ISA');
@ISA=('CwsCvsListener');

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    $self->{tagged_files} = ();
    bless ($self, $class);
    return $self;
}

sub get_tagged_files
{
    my $self = shift;
    return $self->{tagged_files} ? $self->{tagged_files} : [];
}

sub notify
{
    my $self  = shift;
    my $event = shift;

    if ( $event->isa('PCVSLib::ErrorMessageEvent') ) {
        my $message = $event->get_message();
        $self->append_to_log($message);
    }
    if ( $event->isa('PCVSLib::MessageEvent') ) {
        my $message = $event->get_message();
        if ( $message =~ /^T (\S+)$/ ) {
            push(@{$self->{tagged_files}}, $1);
            print '.';
        }
    }
    if ( $event->isa('PCVSLib::TerminatedEvent') ) {
        my $is_success = $event->is_success();
        if ( $is_success ) {
            $self->clear_log();
        }
        $self->{is_success_} = $is_success;
    }
}

package CwsCvsMergeListener;
use vars('@ISA');
@ISA=('CwsCvsListener');

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    $self->{count} = 0;
    $self->{conflict_file} = 0;
    $self->{already_merged} = ();
    $self->{conflicts} = ();
    bless ($self, $class);
    return $self;
}

sub get_count
{
    my $self = shift;
    return $self->{count};
}

sub get_conflicts
{
    my $self = shift;
    return $self->{conflicts};
}

sub get_already_merged
{
    my $self = shift;
    return $self->{already_merged};
}

sub notify
{
    my $self  = shift;
    my $event = shift;

    if ( $event->isa('PCVSLib::ErrorMessageEvent') ) {
        my $message = $event->get_message();
        # collect conflicts
        if ( $message =~ /rcsmerge: warning: conflicts during merge/ ) {
            $self->{conflict_file}++;
        }
        else {
            $self->append_to_log($message);
        }
    }
    if ( $event->isa('PCVSLib::MessageEvent') ) {
        my $message = $event->get_message();
        if ( $message =~ /(\S*) already contains the differences between / ) {
            print '.';
            push(@{$self->{already_merged}}, $1);
            $self->{count}++;
        }
    }
    if ( $event->isa('PCVSLib::MergedEvent') ) {
            print '.';
            if ( $self->{conflict_file} ) {
                push(@{$self->{conflicts}}, $event->get_file()->path_name()->local_path());
                $self->{conflict_file} = 0;
            }
            $self->{count}++;
    }
    if ( $event->isa('PCVSLib::CreatedEvent') ) {
            print '.';
            $self->{count}++;
    }
    if ( $event->isa('PCVSLib::TerminatedEvent') ) {
        my $is_success = $event->is_success();
        if ( $is_success ) {
            $self->clear_log();
        }
        $self->{is_success_} = $is_success;
    }
}

package CwsCvsStatusListener;
use vars('@ISA');
@ISA=('CwsCvsListener');

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    $self->{stati} = ();
    $self->{cur_file} = undef;
    $self->{cur_status} = undef;
    $self->{cur_working_rev} = undef;
    $self->{cur_sticky_tag} = undef;
    $self->{cur_branch_rev} = undef;
    $self->{cur_sticky_opt} = undef;
    bless ($self, $class);
    return $self;
}

sub get_stati
{
    my $self = shift;
    return $self->{stati};
}

sub notify
{
    my $self  = shift;
    my $event = shift;

    if ( $event->isa('PCVSLib::ErrorMessageEvent') ) {
        my $message = $event->get_message();
        $self->append_to_log($message);
    }
    if ( $event->isa('PCVSLib::MessageEvent') ) {
        my $message = $event->get_message();
        if ( $message =~ /File: no file\s(\S+)\s+Status:\s(\S.*)$/ ) {
            my $file   = $1;
            my $status = $2;
            if ( $self->{cur_file} ) {
                push(@{$self->{stati}}, [
                                          $self->{cur_file},
                                          $self->{cur_status},
                                          $self->{cur_working_rev},
                                          $self->{cur_sticky_tag},
                                          $self->{cur_branch_rev},
                                          $self->{cur_sticky_opt}
                                        ]);
            }
            $self->{cur_file}   = $file;
            $self->{cur_status} = $status;
            $self->{cur_working_rev} = undef;
            $self->{cur_sticky_tag}  = undef;
            $self->{cur_branch_rev}  = undef;
            $self->{cur_sticky_opt}  = undef;
        }
        if ( $message =~ /File:\s(\S+)\s+Status:\s(\S+)/ ) {
            my $file   = $1;
            my $status = $2;
            if ( $self->{cur_file} ) {
                push(@{$self->{stati}}, [
                                          $self->{cur_file},
                                          $self->{cur_status},
                                          $self->{cur_working_rev},
                                          $self->{cur_sticky_tag},
                                          $self->{cur_branch_rev},
                                          $self->{cur_sticky_opt}
                                        ]);
            }
            $self->{cur_file}   = $file;
            $self->{cur_status} = $status;
        }
        if ( $message =~ /Working revision:\s+((\d|\.)+)/ ) {
            $self->{cur_working_rev} = $1;
        }
        if ( $message =~ /Sticky Tag:\s+(.+)/ ) {
            my $sticky_tag = $1;
            if ( $sticky_tag =~ /([\w\-]+) \(branch: ([\d\.]+)\)$/ ) {
                $self->{cur_sticky_tag} = $1;
                $self->{cur_branch_rev} = $2;
            }
            else {
                $self->{cur_sticky_tag} = $sticky_tag;
                $self->{cur_branch_rev} = '';
            }
        }
        if ( $message =~ /Sticky Options:\s+(.+)/ ) {
            $self->{cur_sticky_opt} = $1;
        }
    }
    if ( $event->isa('PCVSLib::TerminatedEvent') ) {
        if ( $self->{cur_file} ) {
            push(@{$self->{stati}}, [
                                      $self->{cur_file},
                                      $self->{cur_status},
                                      $self->{cur_working_rev},
                                      $self->{cur_sticky_tag},
                                      $self->{cur_branch_rev},
                                      $self->{cur_sticky_opt}
                                    ]);
        }
        $self->{cur_file}        = undef;
        $self->{cur_status}      = undef;
        $self->{cur_working_rev} = undef;
        $self->{cur_sticky_tag}  = undef;
        $self->{cur_branch_rev}  = undef;
        $self->{cur_sticky_opt}  = undef;
        my $is_success = $event->is_success();
        if ( $is_success ) {
            $self->clear_log();
        }
        $self->{is_success_} = $is_success;
    }
}

package CwsCvsLogListener;
use vars('@ISA');
@ISA=('CwsCvsListener');

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    $self->{logs} = ();
    $self->{in_comment} = 0;
    $self->{cur_file} = undef;
    $self->{cur_revision} = undef;
    $self->{cur_date} = undef;
    $self->{cur_author} = undef;
    $self->{cur_logs} = ();
    bless ($self, $class);
    return $self;
}

sub get_collected_logs
{
    my $self = shift;
    return $self->{logs};
}

sub notify
{
    my $self  = shift;
    my $event = shift;

    if ( $event->isa('PCVSLib::ErrorMessageEvent') ) {
        my $message = $event->get_message();
        $self->append_to_log($message);
    }
    if ( $event->isa('PCVSLib::MessageEvent') ) {
        my $message = $event->get_message();
        if ( $message =~ /^Working file: (\S+)$/ ) {
            my $file   = $1;
            if ( $self->{cur_file} ) {
                push(@{$self->{logs}}, [
                                          $self->{cur_file},
                                          $self->{cur_logs}
                                        ]);
            }
            $self->{cur_file}     = $file;
            $self->{cur_revision} = undef;
            $self->{cur_date}     = undef;
            $self->{cur_author}   = undef;
            $self->{cur_logs}     = ();
        }
        if ( $message =~ /^revision:\s+((\d|\.)+)$/ ) {
            $self->{cur_revision} = $1;
        }
        if ( $message =~ /^date: (\S+\s\S+);\s+author: (\S+);/ ) {
            $self->{cur_date}   = $1;
            $self->{cur_author} = $2;
            $self->{in_comment}++;
        }
        if ( $self->{in_comment} ) {
            if ( $message =~ /^branches: ((\d|\.)+;)+/ || $message =~ /^RESYNC:/) {
                # skip these
            }
            elsif ( $message =~ /^----------------------------$/
                   ||  $message =~ /^=============================================================================$/)
            {
                my $self->{in_comment} = 0;
            }
            else {
                push(@{$self->{cur_log}},
                              $self->{cur_date} . " " .
                              $self->{cur_author} .
                              $self->{cur_revision} .
                              " $message");
            }
        }
    }
    if ( $event->isa('PCVSLib::TerminatedEvent') ) {
        if ( $self->{cur_file} ) {
            push(@{$self->{logs}}, [
                                        $self->{cur_file},
                                        $self->{cur_logs}
                                    ]);
        }
        $self->{cur_file}     = undef;
        $self->{cur_revision} = undef;
        $self->{cur_date}     = undef;
        $self->{cur_author}   = undef;
        $self->{cur_logs}     = ();
        my $is_success = $event->is_success();
        if ( $is_success ) {
            $self->clear_log();
        }
        $self->{is_success_} = $is_success;
    }
}
1; # needed by "use" or "require"

# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:

