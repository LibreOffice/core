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
# build - build entire project
#
    use strict;
    use Config;
    use POSIX;
    use Cwd qw (cwd);
    use File::Path;
    use File::Temp qw(tmpnam tempdir);
    use File::Find;
    use Socket;
    use IO::Socket::INET;
    use IO::Select;
    use Fcntl;
    use POSIX qw(:errno_h);
    use Sys::Hostname;
    use IPC::Open3;

    use lib ("$ENV{SOLARENV}/bin/modules");
    use SourceConfig;
    use RepositoryHelper;
    use Cwd 'chdir';

    my $in_so_env = 0;
    if (defined $ENV{COMMON_ENV_TOOLS}) {
        unshift(@INC, "$ENV{COMMON_ENV_TOOLS}/modules");
        $in_so_env++;
    };
    my $verbose_mode = 0;
    if (defined $ENV{verbose} || defined $ENV{VERBOSE}) {
        $verbose_mode = ($ENV{verbose} =~ /^t\S*$/i);
    }

    ### for XML file format
    eval { require XMLBuildListParser; import XMLBuildListParser; };
    my $enable_xml = 0;
    my @modes_array = ();
    if (!$@) {
        $enable_xml = 1;
        @modes_array = split('\s' , $ENV{BUILD_TYPE});
    };
#### script id #####

#########################
#                       #
#   Global Variables    #
#                       #
#########################

    my $modules_number++;
    my $perl = 'perl';
    my $remove_command = 'rm -rf';
    my $nul = '> /dev/null';

    my $processes_to_run = 0;
# delete $pid when not needed
    my %projects_deps_hash = ();   # hash of projects with no dependencies,
                                # that could be built now
    my %broken_build = ();         # hash of hashes of the modules,
                                # where build was broken (error occurred)
    my %folders_hashes = ();
    my %running_children = ();
    my $dependencies_hash = 0;
    my $cmd_file = '';
    my $build_all_parents = 0;
    my $show = 0;
    my $checkparents = 0;
    my $deliver = 0;
    my $pre_custom_job = '';
    my $custom_job = '';
    my $post_custom_job = '';
    my %local_deps_hash = ();
    my %path_hash = ();
    my %platform_hash = ();
    my %alive_dependencies = ();
    my %global_deps_hash = (); # hash of dependencies of the all modules
    my %global_deps_hash_backup = (); # backup hash of external dependencies of the all modules
    my %module_deps_hash_backup = (); # backup hash of internal dependencies for each module
    my @broken_modules_names = ();   # array of modules, which cannot be built further
    my @dmake_args = ();
    my %dead_parents = ();
    my $initial_module = '';
    my $all_dependent = 1;  # a flag indicating if the hash has independent keys
    my $build_from_with_branches = '';
    my $build_all_cont = '';
    my $build_since = '';
    my $dlv_switch = '';
    my $child = 0;
    my %processes_hash = ();
    my %module_announced = ();
    my $ignore = '';
    my $html = '';
    my @ignored_errors = ();
    my %incompatibles = ();
    my %skip_modules = ();
    my %exclude_branches = ();
    my $only_platform = ''; # the only platform to prepare
    my $only_common = ''; # the only common output tree to delete when preparing
    my %build_modes = ();
    my $maximal_processes = 0; # the max number of the processes run
    my %modules_types = (); # modules types ('mod', 'img', 'lnk') hash
    my %platforms = (); # platforms available or being working with
    my %platforms_to_copy = (); # copy output trees for the platforms when --prepare
    my $tmp_dir = get_tmp_dir(); # temp directory for checkout and other actions

    my %build_list_paths = (); # build lists names
    my %build_lists_hash = (); # hash of arrays $build_lists_hash{$module} = \($path, $xml_list_object)
    my $pre_job = 'announce'; # job to add for not-single module build
    my $post_job = '';        # -"-
    my @warnings = (); # array of warnings to be shown at the end of the process
    my @errors = (); # array of errors to be shown at the end of the process
    my %html_info = (); # hash containing all necessary info for generating of html page
    my %module_by_hash = (); # hash containing all modules names as values and correspondent hashes as keys
    my %build_in_progress = (); # hash of modules currently being built
    my %build_is_finished = (); # hash of already built modules
    my %modules_with_errors = (); # hash of modules with build errors
    my %build_in_progress_shown = ();  # hash of modules being built,
                                    # and shown last time (to keep order)
    my $build_time = time;
    my %jobs_hash = ();
    my $html_path = undef;
    my $build_finished = 0;
    my $html_file = '';
    my %had_error = (); # hack for mysterious windows problems - try run dmake 2 times if first time there was an error
    my $mkout = correct_path("$ENV{SOLARENV}/bin/mkout.pl");
    my %weights_hash = (); # hash contains info about how many modules are dependent from one module
    my $stop_build_on_error = 0; # for multiprocessing mode: do not build further module if there is an error
    my $interactive = 0; # for interactive mode... (for testing purpose enabled by default)
    my $parent_process = 1;
    my @server_ports = ();
    my $html_port = 0;
    my $html_socket_obj = undef; # socket object for server
    my $client_timeout = 0; # time for client to build (in sec)...
                            # The longest time period after that
                            # the server considered as an error/client crash
    my $reschedule_queue = 0;
    my %module_build_queue = ();
    my %reversed_dependencies = ();
    my %module_paths = (); # hash with absolute module paths
    my %active_modules = ();
    my $finished_children = 0;
    my $debug = 0;
    my %module_deps_hash_pids = ();
    my @argv = @ARGV;
    my $zenity_pid = 0;
    my $zenity_in = '';
    my $zenity_out = '';
    my $zenity_err = '';
    my $last_message_time = 0;
    my $verbose = 0;

    my @modules_built = ();
    my $deliver_command = "deliver.pl";
    my %prj_platform = ();
    my $check_error_string = '';
    my $dmake = '';
    my $dmake_args = '';
    my $echo = '';
    my $new_line = "\n";
    my $incompatible = 0;
    my $local_host_ip = 'localhost';
    my $tail_build_modules_mk = "$ENV{SOLARENV}/gbuild/tail_build_modules.mk";
    my $tail_build_module_dir = $ENV{"SRCDIR"};
    my $tail_build_prj = "tail_build";
    my $cross_tail_build_prj = "cross_tail_build";
    my $total_modules = 0;

### main ###

    get_options();

    zenity_open();
    zenity_tooltip("Starting build.");

    get_build_modes();
    my %deliver_env = ();
    my $workspace_path = get_workspace_path();   # This also sets $initial_module
    my $build_error_log = Cwd::realpath(correct_path($workspace_path)) ."/build_error.log";
    my $workdir = $ENV{WORKDIR};
    my $source_config = SourceConfig -> new($workspace_path);
    check_partial_gnumake_build($initial_module);

    system("rm -f $build_error_log");
    if ($html) {
        if (defined $html_path) {
            $html_file = correct_path($html_path . '/' . $ENV{INPATH}. '.build.html');
        } else {
            my $log_directory = Cwd::realpath(correct_path($workspace_path . '/..')) . '/log';
            if ((!-d $log_directory) && (!mkdir($log_directory))) {
                print_error("Cannot create $log_directory for writing html file\n");
            };
            $html_file = $log_directory . '/' . $ENV{INPATH}. '.build.html';
            print "\nPath to html status page: $html_file\n";
        };
    };

    get_module_and_buildlist_paths();

    $deliver_command .= ' -verbose' if ($html || $verbose);
    $deliver_command .= ' '. $dlv_switch if ($dlv_switch);
    $ENV{mk_tmp}++;

    get_commands();
    unlink ($cmd_file);
    if ($cmd_file) {
        if (open (CMD_FILE, ">>$cmd_file")) {
            select CMD_FILE;
            $echo = 'echo ';
            if ($ENV{GUI_FOR_BUILD} ne 'UNX') {
                $new_line = "echo.\n";
                print "\@$echo off\npushd\n";
            } else {
                $new_line = $echo."\"\"\n";
            };
        } else {
            print_error ("Cannot open file $cmd_file");
        };
    };

    print $new_line;
    get_server_ports();
    start_interactive() if ($interactive);

    if ($checkparents) {
        get_parent_deps( $initial_module, \%global_deps_hash );
    } else {
        build_all();
    }
    if (scalar keys %broken_build) {
        cancel_build();
    };
    print_warnings();
    if (scalar keys %active_modules) {
        foreach (keys %dead_parents) {
            delete $dead_parents{$_} if (!defined $active_modules{$_});
        };
    };
    if (scalar keys %dead_parents) {
        print $new_line.$new_line;
        print $echo."WARNING! Project(s):\n";
        foreach (keys %dead_parents) {
            print $echo."$_\n";
        };
        print $new_line;
        print $echo."not found and couldn't be built. dependencies on that module(s) ignored. Maybe you should correct build lists.\n";
        print $new_line;
        do_exit(1) if ($checkparents);
    };
    if (($ENV{GUI_FOR_BUILD} ne 'UNX') && $cmd_file) {
        print "popd\n";
    };
    $ENV{mk_tmp} = '';
    if ($cmd_file) {
        close CMD_FILE;
        print STDOUT "Script $cmd_file generated\n";
    };
    if ($ignore && scalar @ignored_errors) {
        print STDERR "\nERROR: next directories could not be built:\n";
        foreach (@ignored_errors) {
            print STDERR "\t$_\n";
        };
        print STDERR "\nERROR: please check these directories and build the corresponding module(s) anew!!\n\n";
        do_exit(1);
    };
    do_exit(0);


#########################
#                       #
#      Procedures       #
#                       #
#########################

sub print_warnings {
    if (scalar @warnings) {
        print STDERR "\nWARNING(S):\n";
        print STDERR $_ foreach (@warnings);
    };
};

sub rename_file {
    my ($old_file_name, $new_file_name, $throw_error) = @_;

    if(-e $old_file_name) {
        rename($old_file_name, $new_file_name) or system("mv", $old_file_name, $new_file_name);
        if (-e $old_file_name) {
            system("rm -rf $old_file_name") if (!unlink $old_file_name);
        };
    } elsif ($throw_error) {
        print_error("No such file $old_file_name");
    };
};


sub start_interactive {
    my $pid = open(HTML_PIPE, "-|");
    print "Pipe is open\n";

    if ($pid) {   # parent
        # make file handle non-blocking
        my $flags = '';
        fcntl(HTML_PIPE, F_GETFL, $flags);
        $flags |= O_NONBLOCK;
        fcntl(HTML_PIPE, F_SETFL, $flags);
    } else {      # child
        $parent_process = 0;
        start_html_listener();
    };
};

sub start_html_listener {
    $html_port = $server_ports[$#server_ports];
    do {
        $html_port++
    } while (start_server_on_port($html_port, \$html_socket_obj));
    print "html_port:$html_port html_socket_obj: $html_socket_obj\n";
    my $new_socket_obj;
    do {
        $new_socket_obj = accept_html_connection();
        if (defined $new_socket_obj) {
            my $html_message;
            $html_message = <$new_socket_obj>;
            chomp $html_message;
            print $html_message . "\n";
            my $socket_message = '';
            for my $action ('rebuild', 'delete') {
                if ($html_message =~ /$action=(\S+)/) {
                    print $new_socket_obj "Module $1 is scheduled for $action";
                };
            };
            close($new_socket_obj);
        } else {
            sleep(10);
        };
    } while(1);
};

sub get_html_orders {
    return if (!$interactive);
    my $buffer_size = 1024;
    my $buffer;
    my $rv;
    my $full_buffer = '';
    my %modules_to_rebuild = ();
    my %modules_to_delete = ();
    while ($rv = sysread(HTML_PIPE, $buffer, $buffer_size)) {
        $full_buffer .= $buffer;
    };
    my @html_messages = split(/\n/, $full_buffer);
    foreach (@html_messages) {
        if (/^html_port:(\d+)/) {
            $html_port = $1;
            print "Html port is: $html_port\n";
            next;
        };# GET /rebuild=officenames HTTP/1.0
        print "Message: $_\n";
        chomp;
        if (/GET\s+\/delete=(\S+)[:(\S+)]*\s*HTTP/) {
            $modules_to_delete{$1} = $2;
            print "$1 scheduled for removal from build for \n";
        }
        if (/GET\s+\/rebuild=(\S+)[:(\S+)]*\s*HTTP/) {
            if (defined $global_deps_hash{$1}) {
                print "!!! /tarModule $1 has not been built. Html order ignored\n";
            } else {
                $modules_to_rebuild{$1} = $2;
                print "Scheduled $1 for rebuild\n";
            }
        }
    };
    if (scalar keys %modules_to_delete) {
        $reschedule_queue++;
        schedule_delete(\%modules_to_delete);
        generate_html_file();
    };
    if (scalar keys %modules_to_rebuild) {
        $reschedule_queue++;
        schedule_rebuild(\%modules_to_rebuild);
        generate_html_file();
    };
};

sub schedule_delete {
    my $modules_to_delete = shift;
    foreach (keys %$modules_to_delete) {
        print "Schedule module $_ for delete\n";
        delete ($global_deps_hash{$_});
        delete ($global_deps_hash_backup{$_});
        if (scalar keys %{$module_deps_hash_pids{$projects_deps_hash{$_}}}) {
            kill 9, keys %{$module_deps_hash_pids{$projects_deps_hash{$_}}};
            handle_dead_children(0);
        };
        remove_from_dependencies($_, \%global_deps_hash);
        remove_from_dependencies($_, \%global_deps_hash_backup);
        delete $reversed_dependencies{$_};
        delete $build_is_finished{$_} if defined $build_is_finished{$_};
        delete $modules_with_errors{$_} if defined $modules_with_errors{$_};
        delete $module_announced{$_} if defined $module_announced{$_};
        delete $html_info{$_} if defined $html_info{$_};
        delete $projects_deps_hash{$_} if defined $projects_deps_hash{$_};
    };
};

sub schedule_rebuild {
    my $modules_to_rebuild = shift;
    foreach (keys %$modules_to_rebuild) {
        if (defined $$modules_to_rebuild{$_}) {
            print "Schedule directory for rebuild";
        } else {
            print "Schedule complete $_ module for rebuild\n";
            if (scalar keys %{$module_deps_hash_pids{$projects_deps_hash{$_}}}) {
                kill 9, keys %{$module_deps_hash_pids{$projects_deps_hash{$_}}};
                handle_dead_children(0);
            };
            delete $build_is_finished{$_} if defined $build_is_finished{$_};
            delete $modules_with_errors{$_} if defined $modules_with_errors{$_};
            delete $module_announced{$_};
            initialize_html_info($_);

            foreach my $waiter (keys %{$reversed_dependencies{$_}}) {
                # for rebuild_all_dependent - refacture "if" condition
                ${$global_deps_hash{$waiter}}{$_}++ if (!defined $build_is_finished{$waiter});
            };
            delete $projects_deps_hash{$_} if defined $projects_deps_hash{$_};
            my %single_module_dep_hash = ();
            foreach my $module (keys %{$global_deps_hash_backup{$_}}) {
                if (defined ${$global_deps_hash_backup{$_}}{$module} && (!defined $build_is_finished{$module})) {
                    $single_module_dep_hash{$module}++;
                };
            };
            $global_deps_hash{$_} = \%single_module_dep_hash;
        };
    };
};

#
# procedure retrieves build list path
# (all possibilities are taken into account)
#
sub get_build_list_path {
    my $module = shift;
    return $build_list_paths{$module} if (defined $build_list_paths{$module});
    return $build_list_paths{$module} if (defined $build_list_paths{$module});
    my $possible_dir_path = $module_paths{$module}.'/prj/';
    if (-d $possible_dir_path)
    {
        my $possible_build_list_path = correct_path($possible_dir_path . "build.lst");
        if (-f $possible_build_list_path)
        {
            $build_list_paths{$module} = $possible_build_list_path;
            return $possible_build_list_path;
        };
        print_error("There's no build.lst for $module");
    };
    $dead_parents{$module}++;
    return $build_list_paths{$module};
};

#
# Get dependencies hash of the current and all parent projects
#
sub get_parent_deps {
    my $prj_dir = shift;
    my $deps_hash = shift;
    my @unresolved_parents = ($prj_dir);
    my %skipped_branches = ();
    while (my $module = pop(@unresolved_parents)) {
        next if (defined $$deps_hash{$module});
        my %parents_deps_hash = ();
        foreach (get_parents_array($module)) {
            if (defined $exclude_branches{$_}) {
                $skipped_branches{$_}++;
                next;
            };
            $parents_deps_hash{$_}++;
        }
        $$deps_hash{$module} = \%parents_deps_hash;
        foreach my $parent (keys %parents_deps_hash) {
            if (!defined($$deps_hash{$parent}) && (!defined $exclude_branches{$module})) {
                push (@unresolved_parents, $parent);
            };
        };
    };
    check_deps_hash($deps_hash);
    foreach (keys %skipped_branches) {
        print $echo . "Skipping module's $_ branch\n";
        delete $exclude_branches{$_};
    };
    my @missing_branches = keys %exclude_branches;
    if (scalar @missing_branches) {
        print_error("For $prj_dir branche(s): \"@missing_branches\" not found\n");
    };
};

sub store_weights {
    my $deps_hash = shift;
    foreach (keys %$deps_hash) {
        foreach my $module_deps_hash ($$deps_hash{$_}) {
            foreach my $dependency (keys %$module_deps_hash) {
                $weights_hash{$dependency}++;
            };
        };
    };
};

#
# This procedure builds comlete dependency for each module, ie if the deps look like:
# mod1 -> mod2 -> mod3 -> mod4,mod5,
# than mod1 get mod3,mod4,mod5 as eplicit list of deps, not only mod2 as earlier
#
sub expand_dependencies {
    my $deps_hash = shift;

    foreach my $module1 (keys %$deps_hash) {
        foreach my $module2 (keys %$deps_hash) {
            next if ($module1 eq $module2);
            if (defined ${$$deps_hash{$module2}}{$module1}) {
                ${$$deps_hash{$module2}}{$_}++ foreach (keys %{$$deps_hash{$module1}})
            };
        };
    };
};

#
# Gets list of tail_build modules.
#
sub get_tail_build_modules {
    my $tail_build_prj = shift;
    my $make = $ENV{'GNUMAKE'};
    my $build_type = $ENV{'BUILD_TYPE'};

    my $tail_build_mk = "$tail_build_module_dir/Module_$tail_build_prj.mk";
    my $modules_str = `$make --no-print-directory -r -f $tail_build_modules_mk get_modules TAIL_BUILD_MK=$tail_build_mk BUILD_TYPE='$build_type'`;
    chomp $modules_str;

    my %modules = ();
    foreach my $module (split /\s+/, $modules_str) {
        $modules{$module} = 1;
    }
    return %modules;
}

sub _filter_tail_build_dependencies {
    my $deps_hash = shift;
    my $tail_build_prj = shift;

    if (!defined $$deps_hash{$tail_build_prj}) {
        # nothing to do
        return;
    }

    my %tail_build_modules = get_tail_build_modules($tail_build_prj);

    # first remove tail_build modules from deps.
    foreach my $prj (keys %tail_build_modules) {
        if (defined $$deps_hash{$prj}) {
            delete $$deps_hash{$prj};
        }
    }

    # do the actual replacement
    foreach my $prj (keys %$deps_hash) {
        my @tail_build_deps = ();
        my $deps = $$deps_hash{$prj};

        # remove deps. that are in tail_build
        foreach my $dep (keys %$deps) {
            if (defined $tail_build_modules{$dep}) {
                print "$prj depends on $tail_build_prj\[$dep\]\n";
                push @tail_build_deps, $dep;
                delete $$deps{$dep};
            }
        }

        # insert dep. on tail_build, if necessary
        if (@tail_build_deps && !defined $$deps{$tail_build_prj}) {
            $$deps{$tail_build_prj} = 1;
        }
    }
}

#
# Replaces all deps on modules from tail_build by dep on tail_build
# itself. I.e., if a module foo depends on (sal, sfx2, svx) and (sfx2,
# svx) are in tail_build, foo will be depending on (sal, tail_build).
#
# Works on cross_tail_build too, in case of cross-compilation.
#
sub filter_tail_build_dependencies {
    my $deps_hash = shift;
    _filter_tail_build_dependencies($deps_hash, $tail_build_prj);
    _filter_tail_build_dependencies($deps_hash, $cross_tail_build_prj);
}

#
# This procedure fills the second hash with reversed dependencies,
# ie, with info about modules "waiting" for the module
#
sub reverse_dependencies {
    my ($deps_hash, $reversed) = @_;
    foreach my $module (keys %$deps_hash) {
        foreach (keys %{$$deps_hash{$module}}) {
            if (defined $$reversed{$_}) {
                ${$$reversed{$_}}{$module}++
            } else {
                my %single_module_dep_hash = ($module => 1);
                $$reversed{$_} = \%single_module_dep_hash;
            };
        };
    };
};

#
# Build everything that should be built
#
sub build_all {
    if ($build_all_parents) {
        my ($prj, $prj_dir, $orig_prj);
        get_parent_deps( $initial_module, \%global_deps_hash);
        filter_tail_build_dependencies(\%global_deps_hash);
        if (scalar keys %active_modules) {
            $active_modules{$initial_module}++;
            $modules_types{$initial_module} = 'mod';
        };
        expand_dependencies (\%global_deps_hash);
        prepare_incompatible_build(\%global_deps_hash) if ($incompatible && (!$build_from_with_branches));
        if ($build_from_with_branches) {
            my %reversed_full_deps_hash = ();
            reverse_dependencies(\%global_deps_hash, \%reversed_full_deps_hash);
            prepare_build_from_with_branches(\%global_deps_hash, \%reversed_full_deps_hash);
        }
        if ($build_all_cont || $build_since) {
            store_weights(\%global_deps_hash);
            prepare_build_all_cont(\%global_deps_hash);
            %weights_hash = ();
        };
        if ($incompatible) {
            my @missing_modules = ();
            foreach (sort keys %global_deps_hash) {
                push(@missing_modules, $_) if (!defined $active_modules{$_});
            };
        };
        foreach my $module (keys %dead_parents, keys %skip_modules) {
            remove_from_dependencies($module, \%global_deps_hash);
            delete ($global_deps_hash{$module}) if (defined $global_deps_hash{$module});
        };
        store_weights(\%global_deps_hash);
        backup_deps_hash(\%global_deps_hash, \%global_deps_hash_backup);
        reverse_dependencies(\%global_deps_hash_backup, \%reversed_dependencies);
        $modules_number = scalar keys %global_deps_hash;
        initialize_html_info($_) foreach (keys %global_deps_hash);
        if ($processes_to_run) {
            build_multiprocessing();
            return;
        };
        while ($prj = pick_prj_to_build(\%global_deps_hash)) {
            if (!defined $dead_parents{$prj}) {
                if (scalar keys %broken_build) {
                    print $echo . "Skipping project $prj because of error(s)\n";
                    remove_from_dependencies($prj, \%global_deps_hash);
                    $build_is_finished{$prj}++;
                    next;
                };

                $prj_dir = $module_paths{$prj};
                get_module_dep_hash($prj, \%local_deps_hash);
                my $info_hash = $html_info{$prj};
                $$info_hash{DIRS} = check_deps_hash(\%local_deps_hash, $prj);
                $module_by_hash{\%local_deps_hash} = $prj;
                build_dependent(\%local_deps_hash);
                print $check_error_string;
            };

            remove_from_dependencies($prj, \%global_deps_hash);
            $build_is_finished{$prj}++;
        };
    } else {
        store_build_list_content($initial_module);
        get_module_dep_hash($initial_module, \%local_deps_hash);
        initialize_html_info($initial_module);
        my $info_hash = $html_info{$initial_module};
        $$info_hash{DIRS} = check_deps_hash(\%local_deps_hash, $initial_module);
        $module_by_hash{\%local_deps_hash} = $initial_module;
        build_dependent(\%local_deps_hash);
    };
};

sub backup_deps_hash {
    my $source_hash = shift;
    my $backup_hash = shift;
    foreach my $key (keys %$source_hash) {
        my %values_hash = %{$$source_hash{$key}};
        $$backup_hash{$key} = \%values_hash;
    };
};

sub initialize_html_info {
    my $module = shift;
    return if (defined $dead_parents{$module});
    $html_info{$module} = { 'DIRS' => [],
                            'ERRORFUL' => [],
                            'SUCCESSFUL' => [],
                            'BUILD_TIME' => 0};
}

#
# Do job
#
sub dmake_dir {
    my ($new_job_name, $error_code);
    my $job_name = shift;
    $jobs_hash{$job_name}->{START_TIME} = time();
    $jobs_hash{$job_name}->{STATUS} = 'building';

    if ($job_name =~ /(\s)/o && (!-d $job_name)) {
        $error_code = do_custom_job($job_name, \%local_deps_hash);
    } else {
        html_store_job_info(\%local_deps_hash, $job_name);
        print_error("$job_name not found!!\n") if (!-d $job_name);
        if (!-d $job_name) {
            $new_job_name = $job_name;
            $new_job_name =~ s/_simple//g;
            if ((-d $new_job_name)) {
                print("\nTrying $new_job_name, $job_name not found!!\n");
                $job_name = $new_job_name;
            } else {
                print_error("\n$job_name not found!!\n");
            }
        }
        if ($cmd_file) {
            print "cd $job_name\n";
            print $check_error_string;
            print $echo.$job_name."\n";
            print "$dmake\n";
            print $check_error_string;
        } else {
            print "Entering $job_name\n";
        };
        remove_from_dependencies($job_name, \%local_deps_hash) if (!$child);
        return if ($cmd_file || $show);
        $error_code = run_job($dmake, $job_name);

        #if dmake fails, have a go at regenerating the dependencies
        #and try again. dmakes normal failure is 255, while death on signal is 254
        my $real_exit_code = $error_code >> 8;
        if (($ENV{GUI_FOR_BUILD} eq 'WNT') && ($real_exit_code == 255) && ($ENV{nodep} eq '') && ($ENV{depend} eq '')) {
            print "Retrying $job_name\n";
            $error_code = run_job($dmake, $job_name);
        }

        html_store_job_info(\%local_deps_hash, $job_name, $error_code) if (!$child);
    };

    if ($error_code && $ignore) {
        push(@ignored_errors, $job_name);
        $error_code = 0;
    };
    if ($child) {
        my $oldfh = select STDERR;
        $| = 1;
        select $oldfh;
        $| =1;
        if ($error_code) {
            _exit($error_code >> 8);
        } else {
            _exit($? >> 8) if ($? && ($? != -1));
        };
        _exit(0);
    } elsif ($error_code && ($error_code != -1)) {
        $broken_build{$job_name} = $error_code;
        return $error_code;
    };
};

#
# Procedure stores information about build list (and)
# build list object in build_lists_hash
#
sub store_build_list_content {
    my $module = shift;
    my $build_list_path = get_build_list_path($module);

    return undef if (!defined $build_list_path);
    return if (!$build_list_path);

    if (open (BUILD_LST, $build_list_path))
    {
        my @build_lst = <BUILD_LST>;
        $build_lists_hash{$module} = \@build_lst;
        close BUILD_LST;
        return;
    };
    $dead_parents{$module}++;
}
#
# Get string (list) of parent projects to build
#
sub get_parents_array {
    my $module = shift;
    store_build_list_content($module);
    my $build_list_ref = $build_lists_hash{$module};

    if (ref($build_list_ref) eq 'XMLBuildListParser') {
        return $build_list_ref->getModuleDependencies(\@modes_array);
    };
    foreach (@$build_list_ref) {
        if ($_ =~ /#/) {
            if ($`) {
                $_ = $`;
            } else {
                next;
            };
        };
        s/\r\n//;
        if ($_ =~ /\:+\s+/) {
            return pick_for_build_type($');
        };
    };
    return ();
};

#
# get folders' platform infos
#
sub get_prj_platform {
    my $build_list_ref = shift;
    my ($prj_alias, $line);
    foreach(@$build_list_ref) {
        s/\r\n//;
        $line++;
        if ($_ =~ /\snmake\s/) {
            if ($' =~ /\s*-\s+(\w+)[,\S+]*\s+(\S+)/ ) { #'
                my $platform = $1;
                my $alias = $2;
                print_error ("There is no correct alias set in the line $line!") if ($alias eq 'NULL');
                mark_platform($alias, $platform);
            } else {
                print_error("Misspelling in line: \n$_");
            };
        };
    };
};

#
# Procedure populate the dependencies hash with
# information from XML build list object
#
sub get_deps_from_object {
    my ($module, $build_list_object, $dependencies_hash) = @_;

    foreach my $dir ($build_list_object->getJobDirectories("make", $ENV{GUI})) {
        $path_hash{$dir} = $module_paths{$module};
        $path_hash{$dir} .= $dir if ($dir ne '/');
        my %deps_hash = ();

        foreach my $dep ($build_list_object->getJobDependencies($dir, "make", $ENV{GUI})) {
            $deps_hash{$dep}++;
        };
        $$dependencies_hash{$dir} = \%deps_hash;
    };
};

#
# this function wraps the get_module_dep_hash and backups the resultung hash
#
sub get_module_dep_hash {
    my ($module, $module_dep_hash) = @_;
    if (defined $module_deps_hash_backup{$module}) {
        backup_deps_hash($module_deps_hash_backup{$module}, $module_dep_hash);
    } else {
        get_deps_hash($module, $module_dep_hash);
        my %values_hash = ();
        backup_deps_hash($module_dep_hash, \%values_hash);
        $module_deps_hash_backup{$module} = \%values_hash;
    }
};

#
# Getting hashes of all internal dependencies and additional
# information for given project
#
sub get_deps_hash {
    my ($dummy, $module_to_build);
    my %dead_dependencies = ();
    $module_to_build = shift;
    my $dependencies_hash = shift;
    if ($custom_job)
    {
        add_prerequisite_job($dependencies_hash, $module_to_build, $pre_custom_job);
        add_prerequisite_job($dependencies_hash, $module_to_build, $pre_job);
        add_dependent_job($dependencies_hash, $module_to_build, $custom_job);
        add_dependent_job($dependencies_hash, $module_to_build, $post_job);
        add_dependent_job($dependencies_hash, $module_to_build, $post_custom_job);
        return;
    };

    my  $build_list_ref = $build_lists_hash{$module_to_build};
    if (ref($build_list_ref) eq 'XMLBuildListParser') {
        get_deps_from_object($module_to_build, $build_list_ref, $dependencies_hash);
    } else {
        get_prj_platform($build_list_ref);
        foreach (@$build_list_ref) {
            if ($_ =~ /#/o) {
                next if (!$`);
                $_ = $`;
            };
            s/\r\n//;
            if ($_ =~ /\s+nmake\s+/o) {
                my ($platform, $dependencies, $dir, $dir_alias);
                my %deps_hash = ();
                $dependencies = $';
                $dummy = $`;
                $dummy =~ /(\S+)\s+(\S*)/o;
                $dir = $2;
                $dependencies =~ /(\w+)/o;
                $platform = $1;
                $dependencies = $';
                while ($dependencies =~ /,(\w+)/o) {
                    $dependencies = $'; #'
                };
                $dependencies =~ /\s+(\S+)\s+/o;
                $dir_alias = $1;
                if (!check_platform($platform)) {
                    next if (defined $platform_hash{$dir_alias});
                    $dead_dependencies{$dir_alias}++;
                    next;
                };
                delete $dead_dependencies{$dir_alias} if (defined $dead_dependencies{$dir_alias});
                print_error("Directory alias $dir_alias is defined at least twice!! Please, correct build.lst in module $module_to_build") if (defined $$dependencies_hash{$dir_alias});
                $platform_hash{$dir_alias}++;
                $dependencies = $'; #'
                print_error("$module_to_build/prj/build.lst has wrongly written dependencies string:\n$_\n") if (!$dependencies);
                $deps_hash{$_}++ foreach (get_dependency_array($dependencies));
                $$dependencies_hash{$dir_alias} = \%deps_hash;
                my $local_dir = '';
                if ($dir =~ /(\\|\/)/o) {
                    $local_dir = "/$'";
                };
                $path_hash{$dir_alias} = correct_path($module_paths{$module_to_build} . $local_dir);
            } elsif ($_ !~ /^\s*$/ && $_ !~ /^\w*\s/o) {
                chomp;
                push(@errors, $_);
            };
        };
        if (scalar @errors) {
            my $message = "$module_to_build/prj/build.lst has wrongly written string(s):\n";
            $message .= "$_\n" foreach(@errors);
            if ($processes_to_run) {
                $broken_build{$module_to_build} = $message;
                $dependencies_hash = undef;
                return;
            } else {
                print_error($message);
            };
        };
        foreach my $alias (keys %dead_dependencies) {
            next if defined $alive_dependencies{$alias};
#            if (!IsHashNative($alias)) {
                remove_from_dependencies($alias, $dependencies_hash);
                delete $dead_dependencies{$alias};
#            };
        };
    };
    resolve_aliases($dependencies_hash, \%path_hash);
    add_prerequisite_job($dependencies_hash, $module_to_build, $pre_custom_job);
    add_prerequisite_job($dependencies_hash, $module_to_build, $pre_job);
    add_dependent_job($dependencies_hash, $module_to_build, $custom_job);
    add_dependent_job($dependencies_hash, $module_to_build, $post_job) if ($module_to_build ne $initial_module);
    add_dependent_job($dependencies_hash, $module_to_build, $post_custom_job);
    store_weights($dependencies_hash);
};

#
# procedure adds which is independent from anothers, but anothers are dependent from it
#
sub add_prerequisite_job {
    my ($dependencies_hash, $module, $job) = @_;
    return if (!$job);
    $job = "$module $job";
    foreach (keys %$dependencies_hash) {
        my $deps_hash = $$dependencies_hash{$_};
        $$deps_hash{$job}++;
    };
    $$dependencies_hash{$job} = {};
};

#
# procedure adds a job wich is dependent from all already registered jobs
#
sub add_dependent_job {
    # $post_job is dependent from all jobs
    my ($dependencies_hash, $module, $job) = @_;
    return if (!$job);
    my %deps_hash = ();
    $deps_hash{$_}++ foreach (keys %$dependencies_hash);
    $$dependencies_hash{"$module $job"} = \%deps_hash;
};

#
# this procedure converts aliases to absolute paths
#
sub resolve_aliases {
    my ($dependencies_hash, $path_hash) = @_;
    foreach my $dir_alias (keys %$dependencies_hash) {
        my $aliases_hash_ref = $$dependencies_hash{$dir_alias};
        my %paths_hash = ();
        foreach (keys %$aliases_hash_ref) {
            $paths_hash{$$path_hash{$_}}++;
        };
        delete $$dependencies_hash{$dir_alias};
        $$dependencies_hash{$$path_hash{$dir_alias}} = \%paths_hash;
    };
};

#
# mark platform in order to prove if alias has been used according to specs
#
sub mark_platform {
    my $prj_alias = shift;
    if (exists $prj_platform{$prj_alias}) {
        $prj_platform{$prj_alias} = 'all';
    } else {
        $prj_platform{$prj_alias} = shift;
    };
};

#
# Convert path from abstract (with '\' and/or '/' delimiters)
# to system-independent
#
sub correct_path {
    $_ = shift;
    s/\\/\//g;
    return $_;
};


sub check_dmake {
    if (open(DMAKEVERSION, "dmake -V |")) {
        my @dmake_version = <DMAKEVERSION>;
        close DMAKEVERSION;
        return;
    };
    my $error_message = 'dmake: Command not found.';
    $error_message .= ' Please rerun bootstrap' if (!defined $ENV{COMMON_ENV_TOOLS});
    print_error($error_message);
};

#
# Get platform-dependent commands
#
sub get_commands {
    my $arg = '';
    # Setting alias for dmake
    $dmake = 'dmake';
    check_dmake();

    if ($cmd_file) {
        if ($ENV{GUI_FOR_BUILD} eq 'UNX') {
            $check_error_string = "if \"\$?\" != \"0\" exit\n";
        } else {
            $check_error_string = "if \"\%?\" != \"0\" quit\n";
        };
    };

    $dmake_args = join(' ', 'dmake', @dmake_args);

    while ($arg = pop(@dmake_args)) {
        $dmake .= ' '.$arg;
    };
    $dmake .= ' verbose=true' if ($html || $verbose);
};

#
# Procedure retrieves list of projects to be built from build.lst
#
sub get_workspace_path {
    if (!defined $ENV{GUI}) {
        $ENV{mk_tmp} = '';
        die "No environment set\n";
    };
    my $repository_helper = RepositoryHelper->new();
    my $workspace_path = $repository_helper->get_repository_root();
    my $initial_dir = $repository_helper->get_initial_directory();
    if ($workspace_path eq $initial_dir) {
        print_error('Found no project to build');
    };
    $initial_module = substr($initial_dir, length($workspace_path) + 1);
    if ($initial_module =~ /(\\|\/)/) {
        $initial_module = $`;
    };
    $module_paths{$initial_module} = $workspace_path . "/$initial_module";
    return $workspace_path;
};

#
# Picks project which can be built now from hash and then deletes it from hash
#
sub pick_prj_to_build {
    my $deps_hash = shift;
    get_html_orders();
    my $prj = find_indep_prj($deps_hash);
    if ($prj) {
        delete $$deps_hash{$prj};
        generate_html_file();
    };
    return $prj;
};

#
# Make a decision if the project should be built on this platform
#
sub check_platform {
    my $platform = shift;
    return 1 if ($platform eq 'all');
    return 1 if (($ENV{GUI} eq 'UNX') && ($platform eq 'u'));
    return 1 if (($ENV{GUI} eq 'WNT') &&
                 (($platform eq 'w') || ($platform eq 'n')));
    return 0;
};

#
# Remove project to build ahead from dependencies and make an array
# of all from given project dependent projects
#
sub remove_from_dependencies {
    my ($exclude_prj, $i, $prj, $dependencies);
    $exclude_prj = shift;
    $dependencies = shift;
    foreach $prj (keys %$dependencies) {
        my $prj_deps_hash = $$dependencies{$prj};
        delete $$prj_deps_hash{$exclude_prj} if (defined $$prj_deps_hash{$exclude_prj});
    };
};


#
# Check the hash for consistency
#
sub check_deps_hash {
    my ($deps_hash_ref, $module) = @_;
    my @possible_order;
    my $module_path = $module_paths{$module} if (defined $module);
    return if (!scalar keys %$deps_hash_ref);
    my %deps_hash = ();
    my $consistent;
    backup_deps_hash($deps_hash_ref, \%deps_hash);
    my $string;
    my $log_name;
    my $log_path;
    my $long_log_path;
    my $build_number = 0;

    do {
        $consistent = '';
        foreach my $key (sort keys %deps_hash) {
            my $local_deps_ref = $deps_hash{$key};
            if (!scalar keys %$local_deps_ref) {
                if (defined $module) {
                    $build_number++;
                    $string = undef;
                    if ($key =~ /(\s)/o) {
                        $string = $key;
                    } else {
                        if (length($key) == length($module_path)) {
                            $string = './';
                        } else {
                            $string = substr($key, length($module_path) + 1);
                            $string =~ s/\\/\//go;
                        };
                    };
                    $log_name = $string;
                    if ($log_name eq "$module $custom_job") {
                        $log_name = "custom_job";
                    };
                    if ($log_name eq "$module $pre_custom_job") {
                        $log_name = "pre_custom_job";
                    };
                    if ($log_name eq "$module $post_custom_job") {
                        $log_name = "post_custom_job";
                    };
                    $log_name =~ s/\\|\//\./g;
                    $log_name =~ s/\s/_/g;
                    $log_name = $module if ($log_name =~ /^\.+$/);
                    $log_name .= '.txt';

                    if ( $source_config->is_gbuild($module) )
                    {
                        $log_path = correct_path("$workdir/Logs/${module}_${log_name}");
                        $long_log_path = correct_path("$workdir/Logs/${module}_${log_name}");
                    }
                    else
                    {
                        $log_path = '../' . $source_config->get_module_repository($module) . "/$module/$ENV{INPATH}/misc/logs/$log_name",
                        $long_log_path = correct_path($module_paths{$module} . "/$ENV{INPATH}/misc/logs/$log_name"),
                    }

                    push(@possible_order, $key);
                    $jobs_hash{$key} = {    SHORT_NAME => $string,
                                            BUILD_NUMBER => $build_number,
                                            STATUS => 'waiting',
                                            LOG_PATH => $log_path,
                                            LONG_LOG_PATH => $long_log_path,
                                            MODULE => $module,
                                            START_TIME => 0,
                                            FINISH_TIME => 0,
                                            CLIENT => '-'
                    };
                };
                remove_from_dependencies($key, \%deps_hash);
                delete $deps_hash{$key};
                $consistent++;
            };
        };
    } while ($consistent && (scalar keys %deps_hash));
    return \@possible_order if ($consistent);
    print STDERR "Fatal error:";
    foreach (keys %deps_hash) {
        print STDERR "\n\t$_ depends on: ";
        foreach my $i (keys %{$deps_hash{$_}}) {
            print STDERR (' ', $i);
        };
    };
    if ($child) {
        my $oldfh = select STDERR;
        $| = 1;
        _do_exit(1);
    } else {
        print_error("There are dead or circular dependencies\n");
    };
};

#
# Find project with no dependencies left.
#
sub find_indep_prj {
    my ($dependencies, $i);
    my @candidates = ();
    $all_dependent = 1;
    handle_dead_children(0) if ($processes_to_run);
    my $children = children_number();
    return '' if ($children && ($children >= $processes_to_run));
    $dependencies = shift;
    if (scalar keys %$dependencies) {
        foreach my $job (keys %$dependencies) {
            if (!scalar keys %{$$dependencies{$job}}) {
                push(@candidates, $job);
                last if (!$processes_to_run);
            };
        };
        if (scalar @candidates) {
            $all_dependent = 0;
            my $best_candidate = undef;
            my $best_weight = 0;
            if (scalar @candidates > 1) {
                foreach my $candidate (@candidates) {
                    my $candidate_weight = get_waiters_number($candidate);
                    if ($candidate_weight > $best_weight) {
                        $best_candidate = $candidate;
                        $best_weight = $candidate_weight;
                    };
                };
                if (defined $best_candidate) {
                    return $best_candidate;
                }
            }
            my @sorted_candidates = sort(@candidates);
            return $sorted_candidates[0];
        };
    };
    return '';
};

sub get_waiters_number {
    my $module = shift;
    if (defined $weights_hash{$module}) {
        return $weights_hash{$module};
    };
    if (defined $reversed_dependencies{$module}) {
        return scalar keys %{$reversed_dependencies{$module}};
    };
    return 0;
};

#
# Check if given entry is HASH-native, that is not a user-defined data
#
#sub IsHashNative {
#    my $prj = shift;
#    return 1 if ($prj =~ /^HASH\(0x[\d | a | b | c | d | e | f]{6,}\)/);
#    return 0;
#};

#
# Getting array of dependencies from the string given
#
sub get_dependency_array {
    my ($dep_string, @dependencies, $parent_prj, $prj, $string);
    @dependencies = ();
    $dep_string = shift;
    $string = $dep_string;
    $prj = shift;
    while ($dep_string !~ /^NULL/o) {
        print_error("Project $prj has wrongly written dependencies string:\n $string") if (!$dep_string);
        $dep_string =~ /(\S+)\s*/o;
        $parent_prj = $1;
        $dep_string = $'; #'
        if ($parent_prj =~ /\.(\w+)$/o) {
            $parent_prj = $`;
            if (($prj_platform{$parent_prj} ne $1) &&
                ($prj_platform{$parent_prj} ne 'all')) {
                print_error ("$parent_prj\.$1 is a wrongly dependency identifier!\nCheck if it is platform dependent");
            };
            $alive_dependencies{$parent_prj}++ if (check_platform($1));
            push(@dependencies, $parent_prj);
        } else {
            if ((exists($prj_platform{$parent_prj})) &&
                ($prj_platform{$parent_prj} ne 'all') ) {
                print_error("$parent_prj is a wrongly used dependency identifier!\nCheck if it is platform dependent");
            };
            push(@dependencies, $parent_prj);
        };
    };
    return @dependencies;
};

sub print_error {
    my $message = shift;
    my $force = shift;
    $modules_number -= scalar keys %global_deps_hash;
    $modules_number -= 1;
    print STDERR "\nERROR: $message\n";
    $ENV{mk_tmp} = '';
    if ($cmd_file) {
        close CMD_FILE;
        unlink ($cmd_file);
    };
    if (!$child) {
        $ENV{mk_tmp} = '';
        close CMD_FILE if ($cmd_file);
        unlink ($cmd_file);
        do_exit(1);
    };
    do_exit(1) if (defined $force);
};

sub usage {
    print STDERR "\nbuild\n";
    print STDERR "Syntax:    build    [--all|-a[:prj_name]]|[--from|-f prj_name1[:prj_name2] [prj_name3 [...]]]|[--since|-c prj_name] [--with_branches prj_name1[:prj_name2] [--skip prj_name1[:prj_name2] [prj_name3 [...]] [prj_name3 [...]|-b] [--deliver|-d [--dlv_switch deliver_switch]]] [-P processes] [--show|-s] [--help|-h] [--file|-F] [--ignore|-i] [--version|-V] [--mode|-m OOo[,SO[,EXT]] [--html [--html_path html_file_path]] [--pre_job=pre_job_sring] [--job=job_string|-j] [--post_job=post_job_sring] [--stoponerror] [--exclude_branch_from prj_name1[:prj_name2] [prj_name3 [...]]] [--interactive] [--verbose]\n";
    print STDERR "Example1:    build --from sfx2\n";
    print STDERR "                     - build all projects dependent from sfx2, starting with sfx2, finishing with the current module\n";
    print STDERR "Example2:    build --all:sfx2\n";
    print STDERR "                     - the same as --all, but skip all projects that have been already built when using \"--all\" switch before sfx2\n";
    print STDERR "Example3(for unixes):\n";
    print STDERR "             build --all --pre_job=echo\\ Starting\\ job\\ in\\ \\\$PWD --job=some_script.sh --post_job=echo\\ Job\\ in\\ \\\$PWD\\ is\\ made\n";
    print STDERR "                     - go through all projects, echo \"Starting job in \$PWD\" in each module, execute script some_script.sh, and finally echo \"Job in \$PWD is made\"\n";
    print STDERR "\nSwitches:\n";
    print STDERR "        --all        - build all projects from very beginning till current one\n";
    print STDERR "        --from       - build all projects dependent from the specified (including it) till current one\n";
    print STDERR "        --exclude_branch_from    - exclude module(s) and its branch from the build\n";
    print STDERR "        --mode OOo   - build only projects needed for OpenOffice.org\n";
    print STDERR "        --with_branches- the same as \"--from\" but with build all projects in neighbour branches\n";
    print STDERR "        --skip       - do not build certain module(s)\n";
    print STDERR "        --since      - build all projects beginning from the specified till current one (the same as \"--all:prj_name\", but skipping prj_name)\n";
    print STDERR "        --checkmodules      - check if all required parent projects are availlable\n";
    print STDERR "        --show       - show what is going to be built\n";
    print STDERR "        --file       - generate command file file_name\n";
    print STDERR "        --deliver    - only deliver, no build (usable for \'-all\' and \'-from\' keys)\n";
    print STDERR "        -P           - start multiprocessing build, with number of processes passed\n";
    print STDERR "        --dlv_switch - use deliver with the switch specified\n";
    print STDERR "        --help       - print help info\n";
    print STDERR "        --ignore     - force tool to ignore errors\n";
    print STDERR "        --html       - generate html page with build status\n";
    print STDERR "                       file named $ENV{INPATH}.build.html will be generated in $ENV{SRC_ROOT}\n";
    print STDERR "          --html_path      - set html page path\n";
    print STDERR "        --stoponerror      - stop build when error occurs (for mp builds)\n";
    print STDERR "        --interactive      - start interactive build process (process can be managed via html page)\n";
    print STDERR "        --verbose          - generates a detailed output of the build process\n";
    print STDERR "   Custom jobs:\n";
    print STDERR "        --job=job_string        - execute custom job in (each) module. job_string is a shell script/command to be executed instead of regular dmake jobs\n";
    print STDERR "        --pre_job=pre_job_string        - execute preliminary job in (each) module. pre_job_string is a shell script/command to be executed before regular job in the module\n";
    print STDERR "        --post_job=job_string        - execute a postprocess job in (each) module. post_job_string is a shell script/command to be executed after regular job in the module\n";
    print STDERR "Default:             - build current project\n";
    print STDERR "Unknown switches passed to dmake\n";
};

#
# Get all options passed
#
sub get_options {
    my $arg;
    while ($arg = shift @ARGV) {
        if ($arg =~ /^-P$/)              { $processes_to_run = shift @ARGV;     next; }
        if ($arg =~ /^-P(\d+)$/)         { $processes_to_run = $1;              next; }
        if ($arg =~ /^--all$/)           { $build_all_parents = 1;              next; }
        if ($arg =~ /^-a$/)              { $build_all_parents = 1;              next; }
        if ($arg =~ /^--show$/)          { $show = 1;                           next; }
        if ($arg =~ /^--checkmodules$/)  { $checkparents = 1; $ignore = 1;      next; }
        if ($arg =~ /^-s$/)              { $show = 1;                           next; }
        if ($arg =~ /^--deliver$/)       { $deliver = 1;                        next; }
        if ($arg =~ /^(--job=)/)         { $custom_job = $';                    next; }
        if ($arg =~ /^(--pre_job=)/)     { $pre_custom_job = $';                next; }
        if ($arg =~ /^(--post_job=)/)    { $post_custom_job = $';               next; } #'
        if ($arg =~ /^-d$/)              { $deliver = 1;                        next; }
        if ($arg =~ /^--dlv_switch$/)    { $dlv_switch = shift @ARGV;           next; }
        if ($arg =~ /^--file$/)          { $cmd_file = shift @ARGV;             next; }
        if ($arg =~ /^-F$/)              { $cmd_file = shift @ARGV;             next; }
        if ($arg =~ /^--skip$/)          { get_modules_passed(\%skip_modules);  next; }

        if ($arg =~ /^--all:(\S+)$/)     { $build_all_parents = 1;
                                           $build_all_cont = $1;                next; }
        if ($arg =~ /^-a:(\S+)$/)        { $build_all_parents = 1;
                                           $build_all_cont = $1;                next; }
        if ($arg =~ /^--from$/ || $arg =~ /^-f$/) {
                                           $build_all_parents = 1;
                                           get_modules_passed(\%incompatibles); next; }
        if ($arg =~ /^--since$/)         { $build_all_parents = 1;
                                           $build_since = shift @ARGV;          next; }
        if ($arg =~ /^-c$/)              { $build_all_parents = 1;
                                           $build_since = shift @ARGV;          next; }
        if ($arg =~ /^-s$/)              { $build_all_parents = 1;
                                           $build_since = shift @ARGV;          next; }
        if ($arg =~ /^--help$/)          { usage();                             do_exit(0); }
        if ($arg =~ /^-h$/)              { usage();                             do_exit(0); }
        if ($arg =~ /^--ignore$/)        { $ignore = 1;                         next; }
        if ($arg =~ /^--html$/)          { $html = 1;                           next; }
        if ($arg =~ /^--html_path$/)     { $html_path = shift @ARGV;            next; }
        if ($arg =~ /^-i$/)              { $ignore = 1;                         next; }
        if ($arg =~ /^--version$/)       {                                      do_exit(0); }
        if ($arg =~ /^-V$/)              {                                      do_exit(0); }
        if ($arg =~ /^-m$/)              { get_modes();                         next; }
        if ($arg =~ /^--mode$/)          { get_modes();                         next; }
        if ($arg =~ /^--stoponerror$/)   { $stop_build_on_error = 1;            next; }
        if ($arg =~ /^--interactive$/)   { $interactive = 1;                    next; }
        if ($arg =~ /^--verbose$/)       { $verbose = 1;                        next; }
        if ($arg =~ /^--$/) {
            push (@dmake_args, get_dmake_args()) if (!$custom_job);
            next;
        };
        push (@dmake_args, $arg);
    };
    if (!$html) {
        print_error("\"--html_path\" switch is used only with \"--html\"") if ($html_path);
    };
    if ((scalar keys %exclude_branches) && !$build_all_parents) {
        print_error("\"--exclude_branch_from\" is not applicable for one module builds!!");
    };
    print_error('Switches --with_branches and --all collision') if ($build_from_with_branches && $build_all_cont);
    print_error('Switch --skip is for building multiple modules only!!') if ((scalar keys %skip_modules) && (!$build_all_parents));
    print_error('Switches --with_branches and --since collision') if ($build_from_with_branches && $build_since);
    if ($show) {
        $processes_to_run = 0;
        $cmd_file = '';
    };
    print_error('Switches --job and --deliver collision') if ($custom_job && $deliver);
    $custom_job = 'deliver' if $deliver;
    $post_job = 'deliver' if (!$custom_job);
    $incompatible = scalar keys %incompatibles;
    if ($processes_to_run) {
        if ($ignore && !$html) {
            print_error("Cannot ignore errors in multiprocessing build");
        };
    } elsif ($stop_build_on_error) {
        print_error("Switch --stoponerror is only for multiprocessing builds");
    };

    if ($only_platform) {
        $only_common = 'common';
        $only_common .= '.pro' if ($only_platform =~ /\.pro$/);
    };
    if ($interactive) {
        $html++; # enable html page generation...
        my $local_host_name = hostname();
        $local_host_ip = inet_ntoa(scalar(gethostbyname($local_host_name)) || 'localhost');
    }
    # Default build modes(for OpenOffice.org)
    $ENV{BUILD_TYPE} = 'OOo EXT' if (!defined $ENV{BUILD_TYPE});
    @ARGV = @dmake_args;
    foreach $arg (@dmake_args) {
        $arg =~ /^verbose=(\S+)$/i and $verbose_mode = ($1 =~ /^t\S*$/i);
    }
};

sub get_module_and_buildlist_paths {
    if ($build_all_parents || $checkparents) {
        $active_modules{$_}++ foreach ($source_config->get_active_modules());
        my %active_modules_copy = %active_modules;
        foreach my $module ($source_config->get_all_modules()) {
            delete $active_modules_copy{$module} if defined($active_modules_copy{$module});
            $module_paths{$module} = $source_config->get_module_path($module);
            $build_list_paths{$module} = $source_config->get_module_build_list($module);
        }
        $dead_parents{$_}++ foreach (keys %active_modules_copy);
    };
};


sub get_dmake_args {
    my $arg;
    my @job_args = ();
    while ($arg = shift @ARGV) {
        next if ($arg =~ /^--$/);
        push (@job_args, $arg);
    };
    return @job_args;
};

#
# cancel build when one of children has error exit code
#
sub cancel_build {
    my $broken_modules_number = scalar @broken_modules_names;

    print STDERR "\n";
    print STDERR "-----------------------------------------------------------------------\n";
    print STDERR "        Oh dear - something failed during the build - sorry !\n";
    print STDERR "  For more help with debugging build errors, please see the section in:\n";
    print STDERR "            http://wiki.documentfoundation.org/Development\n";
    print STDERR "\n";

    if (!$broken_modules_number || !$build_all_parents) {
        while (children_number()) {
            handle_dead_children(1);
        }
    }

    if (keys %broken_build) {
        print STDERR "  internal build errors:\n\n";
        foreach (keys %broken_build) {
            print STDERR "ERROR: error " . $broken_build{$_} . " occurred while making $_\n";
        };
        print STDERR "\n";
    }

    my $module = shift @broken_modules_names;
    if ($broken_modules_number > 1) {
        print STDERR " it seems you are using a threaded build, which means that the\n";
        print STDERR " actual compile error is probably hidden far above, and could be\n";
        print STDERR " inside any of these other modules:\n";
        print STDERR "     @broken_modules_names\n";
        print STDERR " please re-run build inside each one to isolate the problem.\n";
    } else {
        print STDERR " it seems that the error is inside '$module', please re-run build\n";
        print STDERR " inside this module to isolate the error and/or test your fix:\n";
    }
    print STDERR "\n";
    print STDERR "build_error.log should contain the captured output of the failed module(s)\n";
    print STDERR "\n";
    print STDERR "-----------------------------------------------------------------------\n";
    print STDERR "To rebuild a specific module:\n";
    print STDERR "\n";
    if ($module eq 'tail_build') {
	print STDERR "$ENV{GNUMAKE} $module.clean # not recommended, this will re-build almost everything\n";
    } else {
	print STDERR "$ENV{GNUMAKE} $module.clean # optional\n";
    }
    print STDERR "$ENV{GNUMAKE} $module\n";
    print STDERR "\n";
    print STDERR "when the problem is isolated and fixed, re-run '$ENV{GNUMAKE}'\n";
    zenity_message("LibreOffice Build Failed!");
    zenity_close();

    do_exit(1);
};

#
# Function for storing errors in multiprocessing AllParents build
#
sub store_error {
    my ($pid, $error_code) = @_;
    return 0 if (!$error_code);

    #we don't care if zenity itself crashes, e.g. rhbz#670895
    if (zenity_enabled()) {
        return 0 if ($zenity_pid == $pid);
    }

    my $child_nick = $processes_hash{$pid};

    if ($ENV{GUI_FOR_BUILD} eq 'WNT') {
        if (!defined $had_error{$child_nick}) {
            $had_error{$child_nick}++;
            return 1;
        };
    };
    $modules_with_errors{$folders_hashes{$child_nick}}++;
    $broken_build{$child_nick} = $error_code;
    if ($stop_build_on_error) {
        clear_from_child($pid);
        # Let all children finish their work
        while (children_number()) {
            handle_dead_children(1);
        };
        cancel_build();
    };
    return 0;
};

#
# child handler (clears (or stores info about) the terminated child)
#
sub handle_dead_children {
    my $running_children = children_number();
    return if (!$running_children);
    my $force_wait = shift;
    my $try_once_more = 0;
    do {
        my $pid = 0;
        if (children_number() >= $processes_to_run ||
                ($force_wait && ($running_children == children_number()))) {
            $pid = wait();
        } else {
            $pid = waitpid( -1, &WNOHANG);
        };
        if ($pid > 0) {
            $try_once_more = store_error($pid, $?);
            if ($try_once_more) {
                give_second_chance($pid);
            } else {
                clear_from_child($pid);
            };
            $finished_children++;
        };
    } while(children_number() >= $processes_to_run);
};

sub give_second_chance {
    my $pid = shift;
    # A malicious hack for mysterious windows problems - try 2 times
    # to run dmake in the same directory if errors occurs
    my $child_nick = $processes_hash{$pid};
    $running_children{$folders_hashes{$child_nick}}--;
    delete $processes_hash{$pid};
    start_child($child_nick, $folders_hashes{$child_nick});
};

sub clear_from_child {
    my $pid = shift;
    my $child_nick = $processes_hash{$pid};
    my $error_code = 0;
    if (defined $broken_build{$child_nick}) {
        $error_code = $broken_build{$child_nick};
    } else {
        remove_from_dependencies($child_nick,
                            $folders_hashes{$child_nick});
    };
    foreach (keys %module_deps_hash_pids) {
        delete ${$module_deps_hash_pids{$_}}{$pid} if defined (${$module_deps_hash_pids{$_}}{$pid});
    };
    my $module = $module_by_hash{$folders_hashes{$child_nick}};
    html_store_job_info($folders_hashes{$child_nick}, $child_nick, $error_code);
    $running_children{$folders_hashes{$child_nick}}--;
    delete $processes_hash{$pid};
    $verbose_mode && print 'Running processes: ' . children_number() . "\n";
};

#
# Build the entire project according to queue of dependencies
#
sub build_dependent {
    $dependencies_hash = shift;
    my $pid = 0;
    my $child_nick = '';
    $running_children{$dependencies_hash} = 0 if (!defined $running_children{$dependencies_hash});
    while ($child_nick = pick_prj_to_build($dependencies_hash)) {
        if ($processes_to_run) {
            do {
                if (defined $modules_with_errors{$dependencies_hash} && !$ignore) {
                    return 0 if ($build_all_parents);
                    last;
                };
                # start current child & all
                # that could be started now
                if ($child_nick) {
                    start_child($child_nick, $dependencies_hash);
                    return 1 if ($build_all_parents);
                } else {
                    return 0 if ($build_all_parents);
                    if (scalar keys %$dependencies_hash) {
                        handle_dead_children(1);
                    };
                };
                $child_nick = pick_prj_to_build($dependencies_hash);
            } while (scalar keys %$dependencies_hash || $child_nick);
            while (children_number()) {
                handle_dead_children(1);
            };

            if (defined $modules_with_errors{$dependencies_hash}) {
                push(@broken_modules_names, $module_by_hash{$dependencies_hash});
                cancel_build();
            }
            mp_success_exit();
        } else {
            if (dmake_dir($child_nick)) {
                push(@broken_modules_names, $module_by_hash{$dependencies_hash});
                cancel_build();
            };
        };
        $child_nick = '';
    };
};

sub children_number {
    return scalar keys %processes_hash;
};

sub start_child {
    my ($job_dir, $dependencies_hash) = @_;
    $jobs_hash{$job_dir}->{START_TIME} = time();
    $jobs_hash{$job_dir}->{STATUS} = 'building';
    if ($job_dir =~ /(\s)/o) {
        my $error_code = undef;
        if ($job_dir !~ /\sdeliver$/o) {
            $error_code = do_custom_job($job_dir, $dependencies_hash);
            return;
        }
    };
    $build_in_progress{$module_by_hash{$dependencies_hash}}++;
    html_store_job_info($dependencies_hash, $job_dir);
    my $pid = undef;
    my $children_running;
    my $oldfh = select STDOUT;
    $| = 1;
    if ($pid = fork) { # parent
        select $oldfh;
        $processes_hash{$pid} = $job_dir;
        $children_running = children_number();
        $verbose_mode && print 'Running processes: ', $children_running, "\n";
        $maximal_processes = $children_running if ($children_running > $maximal_processes);
        $folders_hashes{$job_dir} = $dependencies_hash;
        store_pid($dependencies_hash, $pid);
        $running_children{$dependencies_hash}++;
    } elsif (defined $pid) { # child
        select $oldfh;
        $child = 1;
        dmake_dir($job_dir);
        do_exit(1);
    };
};

sub store_pid {
    my ($deps_hash, $pid) = @_;
    if (!defined $module_deps_hash_pids{$deps_hash}) {
        my %module_hash_pids = ();
        $module_deps_hash_pids{$deps_hash} = \%module_hash_pids;
    };
    ${$module_deps_hash_pids{$deps_hash}}{$pid}++;
};

#
# Build everything that should be built multiprocessing version
#
sub build_multiprocessing {
    my $prj;
    do {
        my $got_module = 0;
        $finished_children = 0;
        while ($prj = pick_prj_to_build(\%global_deps_hash)) {
            if (!defined $projects_deps_hash{$prj}) {
                $projects_deps_hash{$prj} = {};
                get_module_dep_hash($prj, $projects_deps_hash{$prj});
                my $info_hash = $html_info{$prj};
                $$info_hash{DIRS} = check_deps_hash($projects_deps_hash{$prj}, $prj);
                $module_by_hash{$projects_deps_hash{$prj}} = $prj;
            }
            $module_build_queue{$prj}++;
            $got_module++;
        };
        if (!$got_module) {
            cancel_build() if ((!scalar keys %module_build_queue) && !children_number());
            if (!$finished_children) {
                handle_dead_children(1);
            };
        };
        build_actual_queue(\%module_build_queue);
    } while (scalar keys %global_deps_hash);
    # Let the last module be built till the end
    while (scalar keys %module_build_queue) {
        build_actual_queue(\%module_build_queue);
        handle_dead_children(1);
    };
    # Let all children finish their work
    while (children_number()) {
        handle_dead_children(1);
    };
    cancel_build() if (scalar keys %broken_build);
    mp_success_exit();
};

sub mp_success_exit {
    print "\nMultiprocessing build is finished\n";
    print "Maximal number of processes run: $maximal_processes\n";
    zenity_message("LibreOffice Build Success!");
    zenity_close();
    do_exit(0);
};

#
# Here the built queue is built as long as possible
#
sub build_actual_queue {
    my $build_queue = shift;
    my $finished_projects = 0;
    do {
        my @sorted_queue = sort {(scalar keys %{$projects_deps_hash{$a}}) <=> (scalar keys %{$projects_deps_hash{$b}})} keys %$build_queue;
        my $started_children = 0;
        foreach my $prj (keys %$build_queue) {
            get_html_orders();
            if ($reschedule_queue) {
                $reschedule_queue = 0;
                foreach (keys %$build_queue) {
                    # Remove the module from the build queue if there is a dependency emerged
                    if ((defined $global_deps_hash{$_}) && (scalar keys %{$global_deps_hash{$_}})) {
                        delete $$build_queue{$_};
                    };
                    delete $$build_queue{$_} if (!defined $global_deps_hash_backup{$_})
                };
                return;
            };
            if (defined $modules_with_errors{$projects_deps_hash{$prj}} && !$ignore) {
                push (@broken_modules_names, $prj);
                delete $$build_queue{$prj};
                next;
            };
            $started_children += build_dependent($projects_deps_hash{$prj});
            if ((!scalar keys %{$projects_deps_hash{$prj}}) &&
                !$running_children{$projects_deps_hash{$prj}}) {
                if (!defined $modules_with_errors{$projects_deps_hash{$prj}} || $ignore)
                {
                    remove_from_dependencies($prj, \%global_deps_hash);
                    $build_is_finished{$prj}++;
                    delete $$build_queue{$prj};
                    $finished_projects++;
                };
            };
        };
        # trigger wait
        if (!$started_children) {
            if ($finished_projects) {
                return;
            } else {
                handle_dead_children(1);
            };
        };
    } while (scalar keys %$build_queue);
};

sub run_job {
    my ($job, $path, $registered_name) = @_;
    my $job_to_do = $job;
    my $error_code = 0;

    print "$registered_name\n";
    return 0 if ( $show );
    $job_to_do = $deliver_command if ($job eq 'deliver');
    $registered_name = $path if (!defined $registered_name);
    chdir $path;
    getcwd();

    my $log_file = $jobs_hash{$registered_name}->{LONG_LOG_PATH};

    my $log_dir = File::Basename::dirname($log_file);

    if ( $source_config->is_gbuild($jobs_hash{$registered_name}->{MODULE}) )
    {
        if ( $job eq 'deliver' )
        {
            return 0;
        }
        else
        {
            return 1 if (! $path =~ /prj$/ );
            mkpath("$workdir/Logs");
            my $gbuild_flags = '-j' . $ENV{GMAKE_MODULE_PARALLELISM};
            my $gbuild_target = 'all slowcheck';
            if ($registered_name =~ /tail_build\/prj$/ )
            {
                $gbuild_flags = '-j' . $ENV{GMAKE_PARALLELISM};
                $gbuild_target = $ENV{gb_TAILBUILDTARGET};
            }
            $gbuild_flags .= ' ' . $ENV{GMAKE_OPTIONS};
            $job_to_do = "$ENV{GNUMAKE} -f Makefile $gbuild_flags $gbuild_target gb_PARTIALBUILD=T";
            my $make_path = $path;
            $make_path =~ s!/prj$!!;
            chdir $make_path;
            getcwd();
            print "gbuild module $make_path: $job_to_do\n";
        }
    }
    else
    {
        if (!-d $log_dir)
        {
            system("$perl $mkout");
        };
    }
    open (MAKE, "$job_to_do 2>&1 |") or return 8;
    open (LOGFILE, "> $log_file") or return 8;
    while (<MAKE>) { print LOGFILE $_; print $_ }
    close MAKE;
    $error_code = $?;
    close LOGFILE;
    if ( $error_code != 0)
    {
        system("echo \"log for $path\" >> $build_error_log");
        system("cat $log_file >> $build_error_log");
    }

    return $error_code;
};

sub do_custom_job {
    my ($module_job, $dependencies_hash) = @_;
    $module_job =~ /(\s)/o;
    my $module = $`;
    my $job = $'; #'
    html_store_job_info($dependencies_hash, $module_job);
    my $error_code = 0;
    if ($job eq $pre_job) {
        announce_module($module);
        remove_from_dependencies($module_job, $dependencies_hash);
    } else {
        $error_code = run_job($job, $module_paths{$module}, $module_job);
        if ($error_code) {
            # give windows one more chance
            if ($ENV{GUI_FOR_BUILD} eq 'WNT') {
                $error_code = run_job($job, $module_paths{$module}, $module_job);
            };
        };
        if ($error_code && $ignore) {
            push(@ignored_errors, $module_job);
            $error_code = 0;
        };
        if ($error_code) {
            $modules_with_errors{$dependencies_hash}++;
#            $broken_build{$module_job} = $error_code;
        } else {
            remove_from_dependencies($module_job, $dependencies_hash);
        };
    };
    html_store_job_info($dependencies_hash, $module_job, $error_code);
    return $error_code;
};

#
# Print announcement for module just started
#
sub announce_module {
    my $prj = shift;
    $build_in_progress{$prj}++;
    print_announce($prj);
};

sub print_announce {
    my $prj = shift;
    return if (defined $module_announced{$prj});
    my $text;
    if ($custom_job)
    {
        $text = "Running custom job \"$custom_job\" in module $prj\n";
    }
    else
    {
        $text = "Building module $prj\n";
    };

    if (!$total_modules) {
        $total_modules = scalar(keys %global_deps_hash) + 1;
    }
    my $modules_started = scalar(keys %module_announced) + 1;
    $text = "($modules_started/$total_modules) $text";

    my $announce_string = $new_line;
    $announce_string .= $echo . "=============\n";
    $announce_string .= $echo . $text;
    $announce_string .= $echo . "=============\n";
    print $announce_string;

    my $percent_progress = $modules_started / $total_modules;
    zenity_tooltip("$text");
    my $zenity_icon = create_progress_svg($percent_progress);
    zenity_icon( $zenity_icon ) if ( $zenity_icon );

    $module_announced{$prj}++;
};

sub create_progress_svg {
    # The colors are rather arbitrarily chosen, but with a very minor attempt
    #   to be readable by color-blind folks.  A second round cut might make
    #   the fill and stroke colors configurable.

    # This function currently leaves a stray svg file in the tmp directory.
    #  This shouldn't be too much of a problem, but if the next person in line
    #  wants to remove this, go ahead.

    if (! zenity_enabled()) {
        return undef;
    }

    my $pi = 3.1415923;
    my $percentage = shift;
    my $path = $percentage > .5 ? '1,1' : '0,1';

    # Creating a "clock" progress meter that starts from the 12 position; the
    #  cursor starts in the center (M50,50), then goes immediately vertical
    #  (v-50).  Associating sin with x, because it's the /difference/ of where
    #  the cursor needs to move.  Other relevent documentation may be found at
    #
    #  http://www.w3.org/TR/SVG11/paths.html#PathElement and
    #  http://www.w3.org/TR/SVG11/images/paths/arcs02.svg

    my $x = 50 * sin( $percentage * 2 * $pi );
    my $y = 50 - 50 * cos( $percentage * 2 * $pi );

    my $progress_file = "$tmp_dir/lo_build_progress.svg";
    open( PROGRESS, '>', $progress_file ) or return undef;
    print PROGRESS <<EOF;
<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
	xmlns:dc="http://purl.org/dc/elements/1.1/"
	xmlns:cc="http://creativecommons.org/ns#"
	xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
	xmlns:svg="http://www.w3.org/2000/svg"
	xmlns="http://www.w3.org/2000/svg"
	version="1.1"
	viewBox="0 0 100 100">
	<path
		d="M50,50 v-50 a50,50 0 $path $x,$y z"
		fill="#aaa"
		stroke="#00f"
		stroke-width="1" />
</svg>
EOF

    close( PROGRESS );

    return $progress_file;
}

sub zenity_enabled {
    return 0 if ($ENV{ENABLE_ZENITY} ne "TRUE");
    return 0 if (!defined $ENV{DISPLAY});
    return 1 if ($ENV{DISPLAY} =~ m/^:/); # local displays only
    return 0;
}

sub zenity_open {
    if (zenity_enabled()) {
        $SIG{PIPE} = 'IGNORE';
        $zenity_pid = open3($zenity_in, $zenity_out, $zenity_err,
                               "zenity --notification --listen");
    };
};

sub zenity_close {
    if (zenity_enabled()) {
        sleep(1); # Give Zenity a chance to show the message.
        close($zenity_in);
    };
};

sub zenity_icon {
    if (zenity_enabled()) {
        my $filename = shift;
        print $zenity_in "icon: $filename\n";
    };
};

sub zenity_tooltip {
    my $current_message_time = time();
    return if (!($current_message_time > $last_message_time + 5));
    $last_message_time = $current_message_time;

    if (zenity_enabled()) {
        my $text = shift;
        print $zenity_in "tooltip: LibreOffice Build: $text\n";
    };
};

sub zenity_message {
    if (zenity_enabled()) {
        my $text = shift;
        print $zenity_in "message: $text\n";
    };
};

#
# Procedure removes output tree from the module (without common trees)
#
sub clear_module {
    my $module = shift;
    print "Removing module's $module output trees...\n";
    print "\n" and return if ($show);
    opendir DIRHANDLE, $module_paths{$module};
    my @dir_content = readdir(DIRHANDLE);
    closedir(DIRHANDLE);
    foreach (@dir_content) {
        next if (/^\.+$/);
        my $dir = correct_path($module_paths{$module}.'/'.$_);
        if ((!-d $dir.'/.svn') && is_output_tree($dir)) {
            rmtree("$dir", 0, 1);
            if (-d $dir) {
                system("$remove_command $dir");
                if (-d $dir) {
                    push(@warnings, "Cannot delete $dir");
                } else {
                    print STDERR (">>> Removed $dir by force\n");
                };
            };
        };
    };
};

#
# Figure out if the directory is an output tree
#
sub is_output_tree {
    my $dir = shift;
    $dir =~ /([\w\d\.]+)$/;
    $_ = $1;
    return '1' if (defined $platforms{$_});
    if ($only_common) {
        return '1' if ($_ eq $only_common);
    } else {
        if (scalar keys %platforms < scalar keys %platforms_to_copy) {
            return '';
        };
        return '1' if (/^common$/);
        return '1' if (/^common\.pro$/);
    };
    return '';
};
sub get_tmp_dir {
    my $tmp_dir;
    if( defined($ENV{TMPDIR}) ) {
       $tmp_dir = $ENV{TMPDIR} . '/';
    } elsif( defined($ENV{TMP}) ) {
       $tmp_dir = $ENV{TMP} . '/';
    } else {
       $tmp_dir = '/tmp/';
    }
    $tmp_dir = tempdir ( DIR => $tmp_dir );
    if (!-d $tmp_dir) {
        print_error("Cannot create temporary directory for checkout in $tmp_dir") if ($@);
    };
    return $tmp_dir;
};

sub prepare_build_from_with_branches {
    my ($full_deps_hash, $reversed_full_deps_hash) = @_;
    foreach my $prerequisite (keys %$full_deps_hash) {
        foreach my $dependent_module (keys %incompatibles) {
            if (defined ${$$reversed_full_deps_hash{$prerequisite}}{$dependent_module}) {
                remove_from_dependencies($prerequisite, $full_deps_hash);
                delete $$full_deps_hash{$prerequisite};
                last;
            };
        };
    };
};

#
# Removes projects which it is not necessary to build
# in incompatible build
#
sub prepare_incompatible_build {
    my ($prj, $deps_hash, @missing_modules);
    $deps_hash = shift;
    foreach my $module (keys %incompatibles) {
        if (!defined $$deps_hash{$module}) {
            print_error("The module $initial_module is independent from $module\n");
        }
        $incompatibles{$module} = $$deps_hash{$module};
        delete $$deps_hash{$module};
    }
    while ($prj = pick_prj_to_build($deps_hash)) {
        remove_from_dependencies($prj, $deps_hash);
        remove_from_dependencies($prj, \%incompatibles);
    };
    foreach (keys %incompatibles) {
        $$deps_hash{$_} = $incompatibles{$_};
    };
    if ($build_all_cont) {
        prepare_build_all_cont($deps_hash);
        delete $$deps_hash{$build_all_cont};
    };
    @modules_built = keys %$deps_hash;

    my $old_output_tree = '';
    foreach $prj (sort keys %$deps_hash)
    {
        next if ($show);
        if ($modules_types{$prj} ne 'mod')
        {
            push(@missing_modules, $prj);
        }
        elsif (-d $module_paths{$prj}. '/'. $ENV{INPATH})
        {
            $old_output_tree++;
        };
    };

    if (scalar @missing_modules) {
        my $warning_string = 'Following modules are inconsistent/missing: ' . "@missing_modules";
        push(@warnings, $warning_string);
    };
    if ($build_all_cont) {
        $$deps_hash{$build_all_cont} = ();
        $build_all_cont = '';
    };
    if ($old_output_tree) {
        push(@warnings, 'Some module(s) contain old output tree(s)!');
    };
    if (scalar @warnings)
    {
        print "WARNING(S):\n";
        print STDERR "$_\n" foreach (@warnings);
        print "\nATTENTION: If you are performing an incompatible build, please break the build with Ctrl+C and prepare the workspace with \"--prepare\" switch!\n\n";
        sleep(5);
    };
};

#
# Removes projects which it is not necessary to build
# with --all:prj_name or --since switch
#
sub prepare_build_all_cont {
    my ($prj, $deps_hash, $border_prj);
    $deps_hash = shift;
    $border_prj = $build_all_cont if ($build_all_cont);
    $border_prj = $build_since if ($build_since);
    while ($prj = pick_prj_to_build($deps_hash)) {
        if (($border_prj ne $prj) &&
            ($border_prj ne '')) {
            remove_from_dependencies($prj, $deps_hash);
            next;
        } else {
            if ($build_all_cont) {
                $$deps_hash{$prj} = ();
            } else {
                remove_from_dependencies($prj, $deps_hash);
            };
            return;
        };
    };
};

sub get_modes {
    my $option = '';
    while ($option = shift @ARGV) {
        if ($option =~ /^-+/) {
            unshift(@ARGV, $option);
            return;
        } else {
            if ($option =~ /,/) {
                $build_modes{$`}++;
                unshift(@ARGV, $') if ($');
            } else {$build_modes{$option}++;};
        };
    };
    $build_modes{$option}++;
};

sub get_modules_passed {
    my $hash_ref = shift;
    my $option = '';
    while ($option = shift @ARGV) {
        if ($option =~ /^-+/) {
            unshift(@ARGV, $option);
            return;
        } else {
            if ($option =~ /(:)/) {
                $option = $`;
                print_error("\'--from\' switch collision") if ($build_all_cont);
                $build_all_cont = $'; #'
            };
            $$hash_ref{$option}++;
        };
    };
};

#
# Store all available build modi in %build_modes
#
sub get_build_modes {
    return if (scalar keys %build_modes);
    if (defined $ENV{BUILD_TYPE}) {
        if ($ENV{BUILD_TYPE} =~ /\s+/o) {
            my @build_modes = split (/\s+/, $ENV{BUILD_TYPE});
            $build_modes{$_}++ foreach (@build_modes);
        } else {
            $build_modes{$ENV{BUILD_TYPE}}++;
        };
        return;
    };
};

#
# pick only the modules, that should be built for
# build types from %build_modes
#
sub pick_for_build_type {
    my $modules = shift;
    my @mod_array = split(/\s+/, $modules);
    print_error("Wrongly written dependencies string:\n $modules\n") if ($mod_array[$#mod_array] ne 'NULL');
    pop @mod_array;
    my @modules_to_build;
    foreach (@mod_array) {
        if (/(\w+):(\S+)/o) {
            push(@modules_to_build, $2) if (defined $build_modes{$1});
            next;
        };
        push(@modules_to_build, $_);
    };
    return @modules_to_build;
};

sub do_exit {
    my $exit_code = shift;
    $build_finished++;
    generate_html_file(1);
    rmtree(correct_path($tmp_dir), 0, 0) if ($tmp_dir);
    print STDERR "Cannot delete $tmp_dir. Please remove it manually\n" if (-d $tmp_dir);
    exit($exit_code);
};

#
# Procedure sorts module in user-frendly order
#
sub sort_modules_appearance {
    foreach (keys %dead_parents) {
        delete $build_is_finished{$_} if (defined $build_is_finished{$_});
        delete $build_in_progress{$_} if (defined $build_in_progress{$_});
    };
    foreach (keys %build_is_finished) {
        delete $build_in_progress{$_} if (defined $build_in_progress{$_});
        delete $build_in_progress_shown{$_} if (defined $build_in_progress_shown{$_});
    };
    my @modules_order = sort keys %modules_with_errors;
    foreach (keys %modules_with_errors) {
        delete $build_in_progress{$_} if (defined $build_in_progress{$_});
        delete $build_is_finished{$_} if (defined $build_is_finished{$_});
        delete $build_in_progress_shown{$_} if (defined $build_in_progress_shown{$_});
    };
    $build_in_progress_shown{$_}++ foreach (keys %build_in_progress);
    push(@modules_order, $_) foreach (sort { $build_in_progress_shown{$b} <=> $build_in_progress_shown{$a} }  keys %build_in_progress_shown);
    push(@modules_order, $_) foreach (sort keys %build_is_finished);
    foreach(sort keys %html_info) {
        next if (defined $build_is_finished{$_} || defined $build_in_progress{$_} || defined $modules_with_errors{$_});
        push(@modules_order, $_);
    };
    return @modules_order;
};

sub generate_html_file {
    return if (!$html);
    my $force_update = shift;
    $force_update++ if ($debug);
    my @modules_order = sort_modules_appearance();
    my ($successes_percent, $errors_percent) = get_progress_percentage(scalar keys %html_info, scalar keys %build_is_finished, scalar keys %modules_with_errors);
    my $build_duration = get_time_line(time - $build_time);
    my $temp_html_file = File::Temp::tmpnam($tmp_dir);
    my $title;
    $title = $ENV{INPATH};
    die("Cannot open $temp_html_file") if (!open(HTML, ">$temp_html_file"));
    print HTML '<html><head>';
    print HTML '<TITLE id=MainTitle>' . $title . '</TITLE>';
    print HTML '<script type="text/javascript">' . "\n";
    print HTML 'initFrames();' . "\n";
    print HTML 'var IntervalID;' . "\n";
    print HTML 'function loadFrame_0() {' . "\n";
    print HTML '    document.write("<html>");' . "\n";
    print HTML '    document.write("<head>");' . "\n";
    print HTML '    document.write("</head>");' . "\n";
    print HTML '    document.write("<body>");' . "\n";
    if ($build_finished) {
        print HTML 'document.write("<h3 align=center style=\"color:red\">Build process is finished</h3>");' . "\n";
        print HTML '        top.frames[0].clearInterval(top.frames[0].IntervalID);' . "\n";
    } elsif ($interactive) {
        print HTML 'document.write("    <div id=divContext style=\"border: 1px solid; display: none; position: absolute\">");' . "\n";
        print HTML 'document.write("        <ul style=\"margin: 0; padding: 0.3em; list-style-type: none; background-color: lightgrey;\" :li:hover {} :hr {border: 0; border-bottom: 1px solid grey; margin: 3px 0px 3px 0px; width: 10em;} :a {border: 0 !important;} >");' . "\n";
        print HTML 'document.write("            <li><a onmouseover=\"this.style.color=\'red\'\" onmouseout=\"this.style.color=\'black\'\" id=aRebuild href=\"#\">Rebuild module</a></li>");' . "\n";
        print HTML 'document.write("            <li><a onmouseover=\"this.style.color=\'red\'\" onmouseout=\"this.style.color=\'black\'\" id=aDelete href=\"#\" >Remove module</a></li>");' . "\n";
        print HTML 'document.write("        </ul>");' . "\n";
        print HTML 'document.write("    </div>");' . "\n";
    };
    if ($build_all_parents) {
        print HTML 'document.write("<table valign=top cellpadding=0 hspace=0 vspace=0 cellspacing=0 border=0>");' . "\n";
        print HTML 'document.write("    <tr>");' . "\n";
        print HTML 'document.write("        <td><a id=ErroneousModules href=\"javascript:top.Error(\'\', \'';
        print HTML join('<br>', sort keys %modules_with_errors);
        print HTML '\', \'\')\"); title=\"';
        print HTML scalar keys %modules_with_errors;
        print HTML ' module(s) with errors\">Total Progress:</a></td>");' . "\n";
        print HTML 'document.write("        <td>");' . "\n";
        print HTML 'document.write("            <table width=100px valign=top cellpadding=0 hspace=0 vspace=0 cellspacing=0 border=0>");' . "\n";
        print HTML 'document.write("                <tr>");' . "\n";
        print HTML 'document.write("                    <td height=20px width=';
        print HTML $successes_percent + $errors_percent;
        if (scalar keys %modules_with_errors) {
            print HTML '% bgcolor=red valign=top></td>");' . "\n";
        } else {
            print HTML '% bgcolor=#25A528 valign=top></td>");' . "\n";
        };
        print HTML 'document.write("                    <td width=';
        print HTML 100 - ($successes_percent + $errors_percent);
        print HTML '% bgcolor=lightgrey valign=top></td>");' . "\n";
        print HTML 'document.write("                </tr>");' . "\n";
        print HTML 'document.write("            </table>");' . "\n";
        print HTML 'document.write("        </td>");' . "\n";
        print HTML 'document.write("        <td align=right>&nbsp Build time: ' . $build_duration .'</td>");' . "\n";
        print HTML 'document.write("    </tr>");' . "\n";
        print HTML 'document.write("</table>");' . "\n";
    };

    print HTML 'document.write("<table width=100% bgcolor=white>");' . "\n";
    print HTML 'document.write("    <tr>");' . "\n";
    print HTML 'document.write("        <td width=30% align=\"center\"><strong style=\"color:blue\">Module</strong></td>");' . "\n";
    print HTML 'document.write("        <td width=* align=\"center\"><strong style=\"color:blue\">Status</strong></td>");' . "\n";
    print HTML 'document.write("        <td width=15% align=\"center\"><strong style=\"color:blue\">CPU Time</strong></td>");' . "\n";
    print HTML 'document.write("    </tr>");' . "\n";

    foreach (@modules_order) {
        next if ($modules_types{$_} eq 'lnk');
        next if (!defined $active_modules{$_});
        my ($errors_info_line, $dirs_info_line, $errors_number, $successes_percent, $errors_percent, $time) = get_html_info($_);
#<one module>
        print HTML 'document.write("    <tr>");' . "\n";
        print HTML 'document.write("        <td width=*>");' . "\n";

        if (defined $dirs_info_line) {
            print HTML 'document.write("            <a id=';
            print HTML $_;
            print HTML ' href=\"javascript:top.Error(\'';
            print HTML $_ , '\', ' ;
            print HTML $errors_info_line;
            print HTML ',';
            print HTML $dirs_info_line;
            print HTML ')\"); title=\"';
            print HTML $errors_number;
            print HTML ' error(s)\">', $_, '</a>");' . "\n";
        } else {
            print HTML 'document.write("            <a target=\'infoframe\' id=';
            print HTML $_;
            print HTML ' href=\"javascript:void(0)\"; title=\"Remove module\">' . $_ . '</a>");' . "\n";
        };


        print HTML 'document.write("        </td>");' . "\n";
        print HTML 'document.write("        <td>");' . "\n";
        print HTML 'document.write("            <table width=100% valign=top cellpadding=0 hspace=0 vspace=0 cellspacing=0 border=0>");' . "\n";
        print HTML 'document.write("                <tr>");' . "\n";
        print HTML 'document.write("                    <td height=15* width=';

        print HTML $successes_percent + $errors_percent;
        if ($errors_number) {
            print HTML '% bgcolor=red valign=top></td>");' . "\n";
        } else {
            print HTML '% bgcolor=#25A528 valign=top></td>");' . "\n";
        };
        print HTML 'document.write("                    <td width=';

        print HTML 100 - ($successes_percent + $errors_percent);
        print HTML '% bgcolor=lightgrey valign=top></td>");' . "\n";
        print HTML 'document.write("                </tr>");' . "\n";
        print HTML 'document.write("            </table>");' . "\n";
        print HTML 'document.write("        </td>");' . "\n";
        print HTML 'document.write("        <td align=\"center\">', $time, '</td>");' . "\n";
        print HTML 'document.write("    </tr>");' . "\n";
# </one module>
    }
    print HTML 'document.write("        </table>");' . "\n";
    print HTML 'document.write("    </body>");' . "\n";
    print HTML 'document.write("</html>");' . "\n";
    print HTML 'document.close();' . "\n";
    print HTML 'refreshInfoFrames();' . "\n";
    print HTML '}' . "\n";


    if (!$build_finished && $interactive ) {
        print HTML 'var _replaceContext = false;' . "\n";
        print HTML 'var _mouseOverContext = false;' . "\n";
        print HTML 'var _noContext = false;' . "\n";
        print HTML 'var _divContext = $(\'divContext\');' . "\n";
        print HTML 'var activeElement = 0;' . "\n";
        print HTML 'function $(id) {return document.getElementById(id);}' . "\n";
        print HTML 'InitContext();' . "\n";
        print HTML 'function InitContext()' . "\n";
        print HTML '{' . "\n";
        print HTML '    $(\'aRebuild\').target = \'infoframe\';' . "\n";
        print HTML '    $(\'aDelete\').target = \'infoframe\';' . "\n";
        print HTML '    $(\'aRebuild\').style.color = \'black\';' . "\n";
        print HTML '    $(\'aDelete\').style.color = \'black\';' . "\n";
        print HTML '    _divContext.onmouseover = function() { _mouseOverContext = true; };' . "\n";
        print HTML '    _divContext.onmouseout = function() { _mouseOverContext = false; };' . "\n";
        print HTML '    _divContext.onclick = function() { _divContext.style.display = \'none\'; };' . "\n";
        print HTML '    document.body.onmousedown = ContextMouseDown;' . "\n";
        print HTML '    document.body.oncontextmenu = ContextShow;' . "\n";
        print HTML '}' . "\n";
        print HTML 'function ContextMouseDown(event) {' . "\n";
        print HTML '    if (_noContext || _mouseOverContext) return;' . "\n";
        print HTML '    if (event == null) event = window.event;' . "\n";
        print HTML '    var target = event.target != null ? event.target : event.srcElement;' . "\n";
        print HTML '    if (event.button == 2 && target.tagName.toLowerCase() == \'a\')' . "\n";
        print HTML '        _replaceContext = true;' . "\n";
        print HTML '    else if (!_mouseOverContext)' . "\n";
        print HTML '        _divContext.style.display = \'none\';' . "\n";
        print HTML '}' . "\n";
        print HTML 'function ContextShow(event) {' . "\n";
        print HTML '    if (_noContext || _mouseOverContext) return;' . "\n";
        print HTML '    if (event == null) event = window.event;' . "\n";
        print HTML '    var target = event.target != null ? event.target : event.srcElement;' . "\n";
        print HTML '    if (_replaceContext) {' . "\n";
        print HTML '        $(\'aRebuild\').href = \'http://'. $local_host_ip .':' . $html_port . '/rebuild=\' + target.id;' . "\n";
        print HTML '        $(\'aDelete\').href = \'http://'. $local_host_ip .':' . $html_port . '/delete=\' + target.id' . "\n";
        print HTML '        var scrollTop = document.body.scrollTop ? document.body.scrollTop : ';
        print HTML 'document.documentElement.scrollTop;' . "\n";
        print HTML '        var scrollLeft = document.body.scrollLeft ? document.body.scrollLeft : ';
        print HTML 'document.documentElement.scrollLeft;' . "\n";
        print HTML '        _divContext.style.display = \'none\';' . "\n";
        print HTML '        _divContext.style.left = event.clientX + scrollLeft + \'px\';' . "\n";
        print HTML '        _divContext.style.top = event.clientY + scrollTop + \'px\';' . "\n";
        print HTML '        _divContext.style.display = \'block\';' . "\n";
        print HTML '        _replaceContext = false;' . "\n";
        print HTML '        return false;' . "\n";
        print HTML '    }' . "\n";
        print HTML '}' . "\n";
    };

    print HTML 'function refreshInfoFrames() {        ' . "\n";
    print HTML '    var ModuleNameObj = top.innerFrame.frames[2].document.getElementById("ModuleErrors");' . "\n";
    print HTML '    if (ModuleNameObj != null) {' . "\n";
    print HTML '        var ModuleName = ModuleNameObj.getAttribute(\'name\');' . "\n";
    print HTML '        var ModuleHref = top.innerFrame.frames[0].document.getElementById(ModuleName).getAttribute(\'href\');' . "\n";
    print HTML '         eval(ModuleHref);' . "\n";
    print HTML '    } else if (top.innerFrame.frames[2].document.getElementById("ErroneousModules") != null) {' . "\n";
    print HTML '        var ModuleHref = top.innerFrame.frames[0].document.getElementById("ErroneousModules").getAttribute(\'href\');' . "\n";
    print HTML '        eval(ModuleHref);' . "\n";
    print HTML '        if (top.innerFrame.frames[1].document.getElementById("ModuleJobs") != null) {' . "\n";
    print HTML '            var ModuleName = top.innerFrame.frames[1].document.getElementById("ModuleJobs").getAttribute(\'name\');' . "\n";
    print HTML '            ModuleHref = top.innerFrame.frames[0].document.getElementById(ModuleName).getAttribute(\'href\');' . "\n";
    print HTML '            var HrefString = ModuleHref.toString();' . "\n";
    print HTML '            var RefEntries = HrefString.split(",");' . "\n";
    print HTML '            var RefreshParams = new Array();' . "\n";
    print HTML '            for (i = 0; i < RefEntries.length; i++) {' . "\n";
    print HTML '                RefreshParams[i] = RefEntries[i].substring(RefEntries[i].indexOf("\'") + 1, RefEntries[i].lastIndexOf("\'"));' . "\n";
    print HTML '            };' . "\n";
    print HTML '            FillFrame_1(RefreshParams[0], RefreshParams[1], RefreshParams[2]);' . "\n";
    print HTML '        }' . "\n";
    print HTML '    };' . "\n";
    print HTML '}' . "\n";
    print HTML 'function loadFrame_1() {' . "\n";
    print HTML '    document.write("<h3 align=center>Jobs</h3>");' . "\n";
    print HTML '    document.write("Click on the project of interest");' . "\n";
    print HTML '    document.close();' . "\n";
    print HTML '}' . "\n";
    print HTML 'function loadFrame_2() {' . "\n";
    print HTML '    document.write("<tr bgcolor=lightgrey<td><h3>Errors</h3></pre></td></tr>");' . "\n";
    print HTML '    document.write("Click on the project of interest");' . "\n";
    print HTML '    document.close();' . "\n";
    print HTML '}    function getStatusInnerHTML(Status) {        var StatusInnerHtml;' . "\n";
    print HTML '    if (Status == "success") {' . "\n";
    print HTML '        StatusInnerHtml = "<em style=color:green>";' . "\n";
    print HTML '    } else if (Status == "building") {' . "\n";
    print HTML '        StatusInnerHtml = "<em style=color:blue>";' . "\n";
    print HTML '    } else if (Status == "error") {' . "\n";
    print HTML '        StatusInnerHtml = "<em style=color:red>";' . "\n";
    print HTML '    } else {' . "\n";
    print HTML '        StatusInnerHtml = "<em style=color:gray>";' . "\n";
    print HTML '    };' . "\n";
    print HTML '    StatusInnerHtml += Status + "</em>";' . "\n";
    print HTML '    return StatusInnerHtml;' . "\n";
    print HTML '}    ' . "\n";
    print HTML 'function ShowLog(LogFilePath, ModuleJob) {' . "\n";
    print HTML '    top.innerFrame.frames[2].document.write("<h3 id=ModuleErrors name=\"" + null + "\">Log for " + ModuleJob + "</h3>");' . "\n";
    print HTML '    top.innerFrame.frames[2].document.write("<iframe id=LogFile name=Log src="';
    if (defined $html_path) {
        print HTML 'file://';
    }
    print HTML '+ LogFilePath + " width=100%></iframe>");' . "\n";
    print HTML '    top.innerFrame.frames[2].document.close();' . "\n";
    print HTML '};' . "\n";
    print HTML 'function FillFrame_1(Module, Message1, Message2) {' . "\n";
    print HTML '    var FullUpdate = 1;' . "\n";
    print HTML '    if (top.innerFrame.frames[1].document.getElementById("ModuleJobs") != null) {' . "\n";
    print HTML '        var ModuleName = top.innerFrame.frames[1].document.getElementById("ModuleJobs").getAttribute(\'name\');' . "\n";
    print HTML '        if (Module == ModuleName) FullUpdate = 0;' . "\n";
    print HTML '    }' . "\n";
    print HTML '    if (FullUpdate) {' . "\n";
    print HTML '        top.innerFrame.frames[1].document.write("<h3 align=center>Jobs in module " + Module + ":</h3>");' . "\n";
    print HTML '        top.innerFrame.frames[1].document.write("<table id=ModuleJobs  name=" + Module + " width=100% bgcolor=white>");' . "\n";
    print HTML '        top.innerFrame.frames[1].document.write("    <tr>");' . "\n";
    print HTML '        top.innerFrame.frames[1].document.write("        <td width=* align=center><strong style=color:blue>Status</strong></td>");' . "\n";
    print HTML '        top.innerFrame.frames[1].document.write("        <td width=* align=center><strong style=color:blue>Job</strong></td>");' . "\n";
    print HTML '        top.innerFrame.frames[1].document.write("        <td width=* align=center><strong style=color:blue>Start Time</strong></td>");' . "\n";
    print HTML '        top.innerFrame.frames[1].document.write("        <td width=* align=center><strong style=color:blue>Finish Time</strong></td>");' . "\n";
    print HTML '        top.innerFrame.frames[1].document.write("    </tr>");' . "\n";
    print HTML '        var dir_info_strings = Message2.split("<br><br>");' . "\n";
    print HTML '        for (i = 0; i < dir_info_strings.length; i++) {' . "\n";
    print HTML '            var dir_info_array = dir_info_strings[i].split("<br>");' . "\n";
    print HTML '            top.innerFrame.frames[1].document.write("    <tr status=" + dir_info_array[0] + ">");' . "\n";
    print HTML '            top.innerFrame.frames[1].document.write("        <td align=center>");' . "\n";
    print HTML '            top.innerFrame.frames[1].document.write(               getStatusInnerHTML(dir_info_array[0]) + "&nbsp");' . "\n";
    print HTML '            top.innerFrame.frames[1].document.write("        </td>");' . "\n";
    print HTML '            if (dir_info_array[4] == "@") {' . "\n";
    print HTML '                top.innerFrame.frames[1].document.write("        <td style=white-space:nowrap>" + dir_info_array[1] + "</td>");' . "\n";
    print HTML '            } else {' . "\n";
    print HTML '                top.innerFrame.frames[1].document.write("        <td><a href=\"javascript:top.ShowLog(\'" + dir_info_array[4] + "\', \'" + dir_info_array[1] + "\')\"); title=\"Show Log\">" + dir_info_array[1] + "</a></td>");' . "\n";
    print HTML '            };' . "\n";
    print HTML '            top.innerFrame.frames[1].document.write("        <td align=center>" + dir_info_array[2] + "</td>");' . "\n";
    print HTML '            top.innerFrame.frames[1].document.write("        <td align=center>" + dir_info_array[3] + "</td>");' . "\n";
    print HTML '            top.innerFrame.frames[1].document.write("    </tr>");' . "\n";
    print HTML '        };' . "\n";
    print HTML '        top.innerFrame.frames[1].document.write("</table>");' . "\n";
    print HTML '    } else {' . "\n";
    print HTML '        var dir_info_strings = Message2.split("<br><br>");' . "\n";
    print HTML '        var ModuleRows = top.innerFrame.frames[1].document.getElementById("ModuleJobs").rows;' . "\n";
    print HTML '        for (i = 0; i < dir_info_strings.length; i++) {' . "\n";
    print HTML '            var dir_info_array = dir_info_strings[i].split("<br>");' . "\n";
    print HTML '            var OldStatus = ModuleRows[i + 1].getAttribute(\'status\');' . "\n";
    print HTML '            if(dir_info_array[0] != OldStatus) {' . "\n";
    print HTML '                var DirectoryInfos = ModuleRows[i + 1].cells;' . "\n";
    print HTML '                DirectoryInfos[0].innerHTML = getStatusInnerHTML(dir_info_array[0]) + "&nbsp";' . "\n";
    print HTML '                if (dir_info_array[4] != "@") {' . "\n";
    print HTML '                    DirectoryInfos[1].innerHTML = "<a href=\"javascript:top.ShowLog(\'" + dir_info_array[4] + "\', \'" + dir_info_array[1] + "\')\"); title=\"Show Log\">" + dir_info_array[1] + "</a>";' . "\n";
    print HTML '                };' . "\n";
    print HTML '                DirectoryInfos[2].innerHTML = dir_info_array[2];' . "\n";
    print HTML '                DirectoryInfos[3].innerHTML = dir_info_array[3];' . "\n";
    print HTML '            };' . "\n";
    print HTML '        };' . "\n";
    print HTML '    };' . "\n";
    print HTML '    top.innerFrame.frames[1].document.close();' . "\n";
    print HTML '};' . "\n";
    print HTML 'function Error(Module, Message1, Message2) {' . "\n";
    print HTML '    if (Module == \'\') {' . "\n";
    print HTML '        if (Message1 != \'\') {' . "\n";
    print HTML '            var erroneous_modules = Message1.split("<br>");' . "\n";
    print HTML '            var ErrorNumber = erroneous_modules.length;' . "\n";

    print HTML '            top.innerFrame.frames[2].document.write("<h3 id=ErroneousModules errors=" + erroneous_modules.length + ">Modules with errors:</h3>");' . "\n";
    print HTML '            for (i = 0; i < ErrorNumber; i++) {' . "\n";
    print HTML '                var ModuleObj = top.innerFrame.frames[0].document.getElementById(erroneous_modules[i]);' . "\n";
    print HTML '                top.innerFrame.frames[2].document.write("<a href=\"");' . "\n";
    print HTML '                top.innerFrame.frames[2].document.write(ModuleObj.getAttribute(\'href\'));' . "\n";
    print HTML '                top.innerFrame.frames[2].document.write("\"); title=\"");' . "\n";
    print HTML '                top.innerFrame.frames[2].document.write("\">" + erroneous_modules[i] + "</a>&nbsp ");' . "\n";
    print HTML '            };' . "\n";
    print HTML '            top.innerFrame.frames[2].document.close();' . "\n";
    print HTML '        };' . "\n";
    print HTML '    } else {' . "\n";
    print HTML '        var ModuleNameObj = top.innerFrame.frames[2].document.getElementById("ModuleErrors");' . "\n";
    print HTML '        var OldErrors = null;' . "\n";
    print HTML '        var ErrorNumber = Message1.split("<br>").length;' . "\n";
    print HTML '        if ((ModuleNameObj != null) && (Module == ModuleNameObj.getAttribute(\'name\')) ) {' . "\n";
    print HTML '            OldErrors = ModuleNameObj.getAttribute(\'errors\');' . "\n";
    print HTML '        }' . "\n";
    print HTML '        if ((OldErrors == null) || (OldErrors != ErrorNumber)) {' . "\n";
    print HTML '            top.innerFrame.frames[2].document.write("<h3 id=ModuleErrors errors=" + ErrorNumber + " name=\"" + Module + "\">Errors in module " + Module + ":</h3>");' . "\n";
    print HTML '            top.innerFrame.frames[2].document.write(Message1);' . "\n";
    print HTML '            top.innerFrame.frames[2].document.close();' . "\n";
    print HTML '        }' . "\n";
    print HTML '        FillFrame_1(Module, Message1, Message2);' . "\n";
    print HTML '    }' . "\n";
    print HTML '}' . "\n";
    print HTML 'function updateInnerFrame() {' . "\n";
    print HTML '     top.innerFrame.frames[0].document.location.reload();' . "\n";
    print HTML '     refreshInfoFrames();' . "\n";
    print HTML '};' . "\n\n";

    print HTML 'function setRefreshRate() {' . "\n";
    print HTML '    RefreshRate = document.Formular.rate.value;' . "\n";
    print HTML '    if (!isNaN(RefreshRate * 1)) {' . "\n";
    print HTML '        top.frames[0].clearInterval(IntervalID);' . "\n";
    print HTML '        IntervalID = top.frames[0].setInterval("updateInnerFrame()", RefreshRate * 1000);' . "\n";
    print HTML '    };' . "\n";
    print HTML '};' . "\n";

    print HTML 'function initFrames() {' . "\n";
    print HTML '    var urlquery = location.href.split("?");' . "\n";
    print HTML '    if (urlquery.length == 1) {' . "\n";
    print HTML '        document.write("<html><head><TITLE id=MainTitle>' . $ENV{INPATH} .'</TITLE>");' . "\n";
    print HTML '        document.write("    <frameset rows=\"12%,88%\">");' . "\n";
    print HTML '        document.write("        <frame name=\"topFrame\" src=\"" + urlquery + "?initTop\"/>");' . "\n";
    print HTML '        document.write("        <frame name=\"innerFrame\" src=\"" + urlquery + "?initInnerPage\"/>");' . "\n";
    print HTML '        document.write("    </frameset>");' . "\n";
    print HTML '        document.write("</head></html>");' . "\n";
    print HTML '    } else if (urlquery[1].substring(0,7) == "initTop") {' . "\n";
    print HTML '        var urlquerycontent = urlquery[1].split("=");' . "\n";
    print HTML '        var UpdateRate = 10' . "\n";
    print HTML '        if (urlquerycontent.length > 2) {' . "\n";
    print HTML '            if (isNaN(urlquerycontent[2] * 1)) {' . "\n";
    print HTML '                alert(urlquerycontent[2] + " is not a number. Ignored.");' . "\n";
    print HTML '            } else {' . "\n";
    print HTML '                UpdateRate = urlquerycontent[2];' . "\n";
    print HTML '            };' . "\n";
    print HTML '        };' . "\n";
    print HTML '        document.write("<html><body>");' . "\n";
    print HTML '        document.write("<h3 align=center>Build process progress status</h3>");' . "\n";
    print HTML '        document.write("<div align=\"right\">");' . "\n";
    print HTML '        document.write("    <table border=\"0\"> <tr>");' . "\n";
    print HTML '        document.write("<td>Refresh rate(sec):</td>");' . "\n";
    print HTML '        document.write("<th>");' . "\n";
    print HTML '        document.write("<FORM name=\"Formular\" onsubmit=\"setRefreshRate()\">");' . "\n";
    print HTML '        document.write("<input type=\"hidden\" name=\"initTop\" value=\"\"/>");' . "\n";
    print HTML '        document.write("<input type=\"text\" id=\"RateValue\" name=\"rate\" autocomplete=\"off\" value=\"" + UpdateRate + "\" size=\"1\"/>");' . "\n";
    print HTML '        document.write("<input type=\"submit\" value=\"OK\">");' . "\n";
    print HTML '        document.write("</FORM>");' . "\n";
    print HTML '        document.write("</th></tr></table>");' . "\n";
    print HTML '        document.write("</div>");' . "\n";
    print HTML '        document.write("    </frameset>");' . "\n";
    print HTML '        document.write("</body></html>");' . "\n";
    print HTML '        top.frames[0].clearInterval(IntervalID);' . "\n";
    print HTML '        IntervalID = top.frames[0].setInterval("updateInnerFrame()", UpdateRate * 1000);' . "\n";
    print HTML '    } else if (urlquery[1] == "initInnerPage") {' . "\n";
    print HTML '        document.write("<html><head>");' . "\n";
    print HTML '        document.write(\'    <frameset rows="80%,20%\">\');' . "\n";
    print HTML '        document.write(\'        <frameset cols="70%,30%">\');' . "\n";
    print HTML '        document.write(\'            <frame src="\');' . "\n";
    print HTML '        document.write(urlquery[0]);' . "\n";
    print HTML '        document.write(\'?initFrame0"/>\');' . "\n";
    print HTML '        document.write(\'            <frame src="\');' . "\n";
    print HTML '        document.write(urlquery[0]);' . "\n";
    print HTML '        document.write(\'?initFrame1"/>\');' . "\n";
    print HTML '        document.write(\'        </frameset>\');' . "\n";
    print HTML '        document.write(\'            <frame src="\');' . "\n";
    print HTML '        document.write(urlquery[0]);' . "\n";
    print HTML '        document.write(\'?initFrame2"  name="infoframe"/>\');' . "\n";
    print HTML '        document.write(\'    </frameset>\');' . "\n";
    print HTML '        document.write("</head></html>");' . "\n";
    print HTML '    } else {' . "\n";
    print HTML '        if (urlquery[1] == "initFrame0" ) {' . "\n";
    print HTML '            loadFrame_0();' . "\n";
    print HTML '        } else if (urlquery[1] == "initFrame1" ) {          ' . "\n";
    print HTML '            loadFrame_1();' . "\n";
    print HTML '        } else if (urlquery[1] == "initFrame2" ) {' . "\n";
    print HTML '            loadFrame_2();' . "\n";
    print HTML '        }' . "\n";
    print HTML '    };' . "\n";
    print HTML '};' . "\n";
    print HTML '</script><noscript>Your browser doesn\'t support JavaScript!</noscript></head></html>' . "\n";
    close HTML;
    rename_file($temp_html_file, $html_file);
};

sub get_local_time_line {
    my $epoch_time = shift;
    my $local_time_line;
    my @time_array;
    if ($epoch_time) {
        @time_array = localtime($epoch_time);
        $local_time_line = sprintf("%02d:%02d:%02d", $time_array[2], $time_array[1], $time_array[0]);
    } else {
        $local_time_line = '-';
    };
    return $local_time_line;
};

sub get_dirs_info_line {
    my $job = shift;
    my $dirs_info_line = $jobs_hash{$job}->{STATUS} . '<br>';
    my @time_array;
    my $log_path_string;
    $dirs_info_line .= $jobs_hash{$job}->{SHORT_NAME} . '<br>';
    $dirs_info_line .= get_local_time_line($jobs_hash{$job}->{START_TIME}) . '<br>';
    $dirs_info_line .= get_local_time_line($jobs_hash{$job}->{FINISH_TIME}) . '<br>';
    if ($jobs_hash{$job}->{STATUS} eq 'waiting' || (!-f $jobs_hash{$job}->{LONG_LOG_PATH})) {
        $dirs_info_line .= '@';
    } else {
        if (defined $html_path) {
            $log_path_string = $jobs_hash{$job}->{LONG_LOG_PATH};
        } else {
            $log_path_string = $jobs_hash{$job}->{LOG_PATH};
        };
        $log_path_string =~ s/\\/\//g;
        $dirs_info_line .= $log_path_string;
    };
    $dirs_info_line .= '<br>';
    return $dirs_info_line;
};

sub get_html_info {
    my $module = shift;
    my $module_info_hash = $html_info{$module};
    my $dirs = $$module_info_hash{DIRS};
    my $dirs_number = scalar @$dirs;
    my $dirs_info_line = '\'';
    if ($dirs_number) {
        my %dirs_sorted_by_order = ();
        foreach (@$dirs) {
            $dirs_sorted_by_order{$jobs_hash{$_}->{BUILD_NUMBER}} = $_;
        }
        foreach (sort {$a <=> $b} keys %dirs_sorted_by_order) {
            $dirs_info_line .= get_dirs_info_line($dirs_sorted_by_order{$_}) . '<br>';
        }
    } else {
        return(undef, undef, 0, 0, 0, '-');
    };
    $dirs_info_line =~ s/(<br>)*$//o;
    $dirs_info_line .= '\'';
    $dirs = $$module_info_hash{SUCCESSFUL};
    my $successful_number = scalar @$dirs;
    $dirs = $$module_info_hash{ERRORFUL};
    my $errorful_number = scalar @$dirs;
    my $errors_info_line = '\'';
    if ($errorful_number) {
        $errors_info_line .= $_ . '<br>' foreach (@$dirs);
    } else {
        $errors_info_line .= 'No errors';
    };
    $errors_info_line .= '\'';
    my $time_line = get_time_line($$module_info_hash{BUILD_TIME});
    my ($successes_percent, $errors_percent) = get_progress_percentage($dirs_number - 1, $successful_number - 1, $errorful_number);
    return($errors_info_line, $dirs_info_line, $errorful_number, $successes_percent, $errors_percent, $time_line);
};

sub get_time_line {
    use integer;
    my $seconds = shift;
    my $hours = $seconds/3600;
    my $minits = ($seconds/60)%60;
    $seconds -= ($hours*3600 + $minits*60);
    return(sprintf("%02d\:%02d\:%02d" , $hours, $minits, $seconds));
};

sub get_progress_percentage {
    use integer;
    my ($dirs_number, $successful_number, $errorful_number) = @_;
    return (0 ,0) if (!$dirs_number);
    my $errors_percent = ($errorful_number * 100)/ $dirs_number;
    my $successes_percent;
    if ($dirs_number == ($successful_number + $errorful_number)) {
        $successes_percent = 100 - $errors_percent;
    } else {
        $successes_percent = ($successful_number * 100)/ $dirs_number;
    };
    return ($successes_percent, $errors_percent);
};

#
# This procedure stores the dmake result in %html_info
#
sub html_store_job_info {
    return if (!$html);
    my ($deps_hash, $build_dir, $error_code) = @_;
    my $force_update = 0;
    if ($build_dir =~ /(\s)/o && (defined $error_code)) {
        $force_update++ if (!children_number());
    }
    my $module = $module_by_hash{$deps_hash};
    my $module_info_hash = $html_info{$module};
    my $dmake_array;
    if (defined $error_code) {
        $jobs_hash{$build_dir}->{FINISH_TIME} = time();
        $$module_info_hash{BUILD_TIME} += $jobs_hash{$build_dir}->{FINISH_TIME} - $jobs_hash{$build_dir}->{START_TIME};
        if ($error_code) {
            $jobs_hash{$build_dir}->{STATUS} = 'error';
            $dmake_array = $$module_info_hash{ERRORFUL};
            $build_dir =~ s/\\/\//g;
            $modules_with_errors{$module}++;
        } else {
            if ($build_dir =~ /(\s)announce/o) {
                $jobs_hash{$build_dir}->{STATUS} = '-';
            } else {
                $jobs_hash{$build_dir}->{STATUS} = 'success';
            };
            $dmake_array = $$module_info_hash{SUCCESSFUL};
        };
        push (@$dmake_array, $build_dir);
    };
};

sub start_server_on_port {
    my $port = shift;
    my $socket_obj = shift;
    $client_timeout = 1 if (!$parent_process);
    if ($ENV{GUI_FOR_BUILD} eq 'WNT') {
        $$socket_obj = new IO::Socket::INET (#LocalAddr => hostname(),
                                  LocalPort => $port,
                                  Proto     => 'tcp',
                                  Listen    => 100); # 100 clients can be on queue, I think it is enough
    } else {
        $$socket_obj = new IO::Socket::INET (#LocalAddr => hostname(),
                                  LocalPort => $port,
                                  Proto     => 'tcp',
                                  ReuseAddr     => 1,
                                  Listen    => 100); # 100 clients can be on queue, I think it is enough
    };
    return('Cannot create socket object') if (!defined $$socket_obj);
    my $timeout = $$socket_obj->timeout($client_timeout);
    $$socket_obj->autoflush(1);
    if ($parent_process && $debug) {
        print "SERVER started on port $port\n";
    } else {
        print "html_port:$html_port html_socket_obj: $html_socket_obj\n";
    };
    return 0;
};

sub accept_html_connection {
    my $new_socket_obj = undef;
    $new_socket_obj = $html_socket_obj->accept();
    return $new_socket_obj;
};

sub get_server_ports {
    # use port 7890 as default
    my $default_port = 7890;
    @server_ports = ($default_port .. $default_port + 4);
};

sub check_partial_gnumake_build
{
    if(!$build_all_parents && $source_config->is_gbuild(shift) )
    {
        print "This module has been migrated to GNU make.\n";
        print "You can only use build --all/--since here with build.pl.\n";
        print "To do the equivalent of 'build && deliver' call:\n";
        print "\t$ENV{GNUMAKE} -r\n";
        print "in the module root.\n";
        exit 1;
    }
}
