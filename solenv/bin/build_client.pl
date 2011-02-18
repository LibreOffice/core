:
eval 'exec perl -S $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************
#
# build_client - client for the build tool in server mode
#

use strict;
use Socket;
use Sys::Hostname;
use File::Temp qw(tmpnam);
use POSIX;
use Cwd qw (cwd);

$SIG{KILL} = \&handle_temp_files;
$SIG{INT} = \&handle_temp_files;

### main ###
my $enable_multiprocessing = 1;
my $server_list_file;
my $server_list_time_stamp = 0;
my %ENV_BACKUP;
$ENV_BACKUP{$_} = $ENV{$_} foreach (keys %ENV);

if ($^O eq 'MSWin32') {
    eval { require Win32::Process; import Win32::Process; };
    $enable_multiprocessing = 0 if ($@);
} else {
    use Cwd 'chdir';
};
my $processes_to_run = 1;

my %hosts_ports = ();
my $default_port = 7890;
my @ARGV_COPY = @ARGV; # @ARGV BACKUP
print "arguments: @ARGV\n";
get_options();

my $proto = getprotobyname('tcp');
my $paddr;
my $host = hostname();
my $current_server = '';
my $got_job = 0;
my %job_temp_files = ();
my %environments = (); # hash containing all environments
my $env_alias;
my %platform_rejects = (); # hash containing paddr of server, that replied "Wrong platform"

my $child = 0;
if ($processes_to_run > 1) {
    my $started_processes = 1;
    if ($^O eq 'MSWin32') {
        my $process_obj = undef;
        my $child_args = "perl $0";
        foreach (@ARGV_COPY) {
            /^-P(\d+)$/        and next;
            /^-P$/     and shift @ARGV_COPY  and next;
            $child_args .= " $_";
        };
        do {
            my $rc = Win32::Process::Create($process_obj, $^X,
                                            $child_args,
                                                0, 0, #NORMAL_PRIORITY_CLASS,
                                            ".");
            print_error("Cannot start child process") if (!$rc);
            $started_processes++;
        } while ($started_processes < $processes_to_run);
    } else {
        my $pid;
        do {
            if ($pid = fork) { # parent
                $started_processes++;
                print $started_processes . "\n";
            } elsif (defined $pid) { # child
                $child++;
            };
        } while (($started_processes < $processes_to_run) && !$child);
    };
};

run_client();
### end of main procedure ###

#########################
#                       #
#      Procedures       #
#                       #
#########################
sub handle_temp_files {
    print STDERR "Got signal - clearing up...\n";
    foreach (keys %job_temp_files) {
        if ($job_temp_files{$_}) {
            rename($_, $job_temp_files{$_}) or system("mv", $_, $job_temp_files{$_});
            print STDERR "Could not rename $_ to $job_temp_files{$_}\n" if (-e $_);
        } else {
            unlink $_ or system("rm -rf $_");
            print STDERR "Could not remove $_\n" if (-e $_);
        };
    };
    exit($?);
};

sub run_client {
# initialize host and port
    if (!scalar keys %hosts_ports) {
        $hosts_ports{localhost} = $default_port;
    }

    print "Started client with PID $$, hostname $host\n";

    my $message = '';
    my $current_port = '';
    my %active_servers = ();

    do {
        $got_job = 0;
        foreach $current_server (keys %hosts_ports) {
            foreach $current_port (keys %{$hosts_ports{$current_server}}) {

                #before each "inactive" server/port connect - connect to each "active" server/port
                next if (defined ${$active_servers{$current_server}}{$current_port});
                # "active" cycle
                foreach my $active_server (keys %active_servers) {
                    foreach my $active_port (keys %{$active_servers{$active_server}}) {
                        my $iaddr = inet_aton($active_server);
                        $paddr = sockaddr_in($active_port, $iaddr);
                        do {
                            my $server_is_active = 0;
                            $message = request_job($message, $active_server, $active_port);
                            $server_is_active++ if ($message);
                            if (!$server_is_active) {
                                delete ${$active_servers{$active_server}}{$active_port};
                                # throw away obsolete environments
                                foreach (keys %environments) {
                                    /^\d+@/;
                                    if ($' eq "$active_server:$active_port") {
                                        delete $environments{$_};
                                    };
                                };
                            };
                            $message = '' if ($message eq 'No job');
                        } while ($message);
                    };
                };

                # "inactive" cycle
                my $iaddr = inet_aton($current_server);
                $paddr = sockaddr_in($current_port, $iaddr);
                do {
                    $message = request_job($message, $current_server, $current_port);
                    if ($message) {
                        if (!defined $active_servers{$current_server}) {
                            my %ports;
                            $active_servers{$current_server} = \%ports;
                        };
                        ${$active_servers{$current_server}}{$current_port}++;
                    };
                    $message = '' if ($message eq 'No job');
                } while ($message);
            };
        };
        sleep 5 if (!$got_job);
        read_server_list();
    } while(1);
};

sub usage {
    my $error = shift;
    print STDERR "\nbuild_client\n";
    print STDERR "Syntax:    build_client [-PN] host1[:port1:...:portN] [host2[:port1:...:portN] ... hostN[:port1:...:portN]]|\@server_list_file\n";
    print STDERR "        -P           - start multiprocessing build, with number of processes passed\n";
    print STDERR "Example1:   build_client myserver1 myserver2:7891:7892\n";
    print STDERR "            the client will be asking for jobs on myserver1's default ports (7890-7894)\n";
    print STDERR "            and on myserver2's ports 7891 and 7892\n";
    print STDERR "Example2:   build_client -P2 myserver1:7990 myserver2\n";
    print STDERR "            start 2 clients which will be asking for jobs myserver1's port 7990\n";
    print STDERR "            and myserver2's default ports (7890-7894)\n";
    exit ($error);
};

sub get_options {
    my $arg;
    usage(1) if (!scalar @ARGV);
    while ($arg = shift @ARGV) {
        usage(0) if /^--help$/;
        usage(0) if /^-h$/;
        $arg =~ /^-P(\d+)$/        and $processes_to_run = $1 and next;
        $arg =~ /^-P$/            and $processes_to_run = shift @ARGV     and next;
        $arg =~ /^@(\S+)$/            and $server_list_file = $1    and next;
        store_server($arg);
    };
    if (($processes_to_run > 1) && (!$enable_multiprocessing)) {
        print_error("Cannot load Win32::Process module for multiple client start");
    };
    if ($server_list_file) {
        print_error("$server_list_file is not a regular file!!") if (!-f $server_list_file);
        read_server_list();
    }
    print_error("No server info") if (!scalar %hosts_ports);
};

sub store_server {
    my $server_string = shift;
    my @server_params = ();
    @server_params = split (/:/, $server_string);
    my $host = shift @server_params;
    my @names = gethostbyname($host);
    my $host_full_name = $names[0];
    my %ports = ();
    if (defined $hosts_ports{$host_full_name}) {
        %ports = %{$hosts_ports{$host_full_name}};
    };
    # To do: implement keys in form server:port -> priority
    if (defined $hosts_ports{$host_full_name}) {
        if (!$server_list_time_stamp) {
            print "The $host with ip address " . inet_ntoa(inet_aton($host)) . " is at least two times in the server list\n";
        };
    } else {
        print "Added server $host as $host_full_name\n";
    };
    if (scalar @server_params) {
         $ports{$_}++ foreach (@server_params);
    } else {
         $ports{$_}++ foreach ($default_port .. $default_port + 4);
    };
    $hosts_ports{$host_full_name} = \%ports;
};

sub read_server_list {
    open(SERVER_LIST, "<$server_list_file") or return;
    my $current_time_stamp = (stat($server_list_file))[9];
    return if ($server_list_time_stamp >= $current_time_stamp);
    my @server_array = ();
    foreach my $file_string(<SERVER_LIST>) {
        while ($file_string =~ /(\S+)/) {
            $file_string = $';
            store_server($1);
        };
    };
    close SERVER_LIST;
    $server_list_time_stamp = $current_time_stamp;
};

sub request_job {
    my ($message, $current_server, $current_port) = @_;
    $message = "platform=$ENV_BACKUP{OUTPATH} pid=$$ osname=$^O" if (!$message);
    # create the socket, connect to the port
    socket(SOCKET, PF_INET, SOCK_STREAM, $proto) or die "socket: $!";
    connect(SOCKET, $paddr) or return '';
    my $error_code = 1;
    $message .= "\n";
    syswrite SOCKET, $message, length $message;
    while (my $line = <SOCKET>) {
        chomp $line;
        if ($line eq 'No job') {
            close SOCKET or die "close: $!";
            return $line;
        };
        if ($line eq "Wrong platform") {
            if (!defined $platform_rejects{$paddr}) {
                $platform_rejects{$paddr}++;
                print STDERR $line . "\n";
            }
            close SOCKET or die "close: $!";
            delete $hosts_ports{$current_server};
            return '';
        } elsif (defined $platform_rejects{$paddr}) {
            delete $platform_rejects{$paddr};
        };
        $got_job++;
        $error_code = do_job($line . " server=$current_server port=$current_port");
    }
    close SOCKET or die "close: $!";
    return("result=$error_code pid=$$");
}

sub do_job {
    my @job_parameters = split(/ /, shift);
    my %job_hash = ();
    my $last_param;
    my $error_code;
    print "Client $$@" . "$host\n";
    foreach (@job_parameters) {
        if (/(=)/) {
            $job_hash{$`} = $';
            $last_param = $`;
        } else {
           $job_hash{$last_param} .= " $_";
        };
    };
    $env_alias = $job_hash{server_pid} . '@' . $job_hash{server} . ':' . $job_hash{port};
    my $result = "1"; # default value
    my $cmd_file = File::Temp::tmpnam($ENV_BACKUP{TMP});
    my $tmp_log_file = File::Temp::tmpnam($ENV_BACKUP{TMP});
    $job_temp_files{$tmp_log_file} = $job_hash{log};
    my $setenv_string = '';
    if (defined $job_hash{setenv_string}) {
        # use configuration string from server
        $setenv_string .= $job_hash{setenv_string};
        print "Environment: $setenv_string\n";

        my $directory = $job_hash{job_dir};
        open (COMMAND_FILE, ">$cmd_file");
        print COMMAND_FILE "$setenv_string\n";
        if (!defined $job_hash{job_dir}) {
            close COMMAND_FILE;
            print "No job_dir, cmd file: $cmd_file\n";
            foreach (keys %job_hash) {
                print "key: $_ $job_hash{$_}\n";
            };
            exit (1);
        };

        print COMMAND_FILE "pushd $job_hash{job_dir} && ";
        print COMMAND_FILE $job_hash{job} ." >& $tmp_log_file\n";
        print COMMAND_FILE "exit \$?\n";
        close COMMAND_FILE;
        $job_temp_files{$cmd_file} = 0;
        $job_temp_files{$tmp_log_file} = $job_hash{log};
        $error_code = system($ENV{SHELL}, $cmd_file);
        unlink $cmd_file or system("rm -rf $cmd_file");
        delete $job_temp_files{$cmd_file};
    } else {
        # generate setsolar string
        if (!defined $environments{$env_alias}) {
            $error_code = get_setsolar_environment(\%job_hash);
            return($error_code) if ($error_code);
        };
        my $solar_vars = $environments{$env_alias};

        delete $ENV{$_} foreach (keys %ENV);
        $ENV{$_} = $$solar_vars{$_} foreach (keys %$solar_vars);
        print 'Workspace: ';
        if (defined $ENV{CWS_WORK_STAMP}) {
            print $ENV{CWS_WORK_STAMP};
        } else {
            print $ENV{SOLARSRC};
        };

        print "\nplatform: $ENV{INPATH} $^O";
        print "\ndir: $job_hash{job_dir}\n";
        print "job: $job_hash{job}\n";
        chdir $job_hash{job_dir};
        getcwd();
        my $job_string = $job_hash{job} . ' > ' . $tmp_log_file . ' 2>&1';
        $error_code = system($job_string);
    };
    rename($tmp_log_file, $job_hash{log}) or system("mv", $tmp_log_file, $job_hash{log});
    delete $job_temp_files{$tmp_log_file};

    if ($error_code) {
        print "Error code = $error_code\n\n";
    } else {
        print "Success!!\n\n";
    };
    return $error_code;
};

sub get_setsolar_environment {
    my $job_hash = shift;
    my $server_pid = $$job_hash{server_pid};
    my $setsolar_string = $$job_hash{setsolar_cmd};
    # Prepare the string for the client
    $setsolar_string =~ s/\s-file\s\S+//g;
    my $error_code = 0;
    my $cmd_file = File::Temp::tmpnam($ENV_BACKUP{TMP});
    my $tmp_log_file = File::Temp::tmpnam($ENV_BACKUP{TMP});
    if (defined $$job_hash{updater}) {
        $ENV{UPDATER} = $$job_hash{updater};
    } else {
        undef $ENV{UPDATER} if (defined $ENV{UPDATER});
    };
    if (defined $$job_hash{source_root}) {
        $ENV{SOURCE_ROOT} = $$job_hash{source_root};
    } else {
        undef $ENV{SOURCE_ROOT} if (defined $ENV{SOURCE_ROOT});
    };
    $error_code = system("$setsolar_string -file $cmd_file");
    store_env_hash($cmd_file);
    return $error_code;
};

sub print_error {
    my $message = shift;
    print STDERR "\nERROR: $message\n";
    exit(1);
};
sub store_env_hash {
    my $ss_setenv_file = shift;
    my %solar_vars = ();
    my $cmd_file = File::Temp::tmpnam($ENV_BACKUP{TMP});
    my $env_vars_file = File::Temp::tmpnam($ENV_BACKUP{TMP});
    print "$cmd_file $env_vars_file\n";
    #get all env variables in $env_vars_file
    open (COMMAND_FILE, ">$cmd_file");
    print COMMAND_FILE "source $ss_setenv_file\n";
    print COMMAND_FILE "env > $env_vars_file\n";
    close COMMAND_FILE;
    system($ENV{SHELL}, $cmd_file);
    print_error($?) if ($?);
    unlink $cmd_file or system("rm -rf $cmd_file");
    unlink $ss_setenv_file or system("rm -rf $ss_setenv_file");

    open SOLARTABLE, "<$env_vars_file" or die "can´t open solarfile $env_vars_file";
    while(<SOLARTABLE>) {
        chomp;
        s/\r\n//o;
        /(=)/;
        $solar_vars{$`} = $';
    };
    close SOLARTABLE;
    unlink $env_vars_file or system("rm -rf $env_vars_file");
    $environments{$env_alias} = \%solar_vars;
};
