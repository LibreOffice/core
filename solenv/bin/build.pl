:
    eval 'exec perl -S $0 ${1+"$@"}'
        if 0;
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: build.pl,v $
#
#   $Revision: 1.164 $
#
#   last change: $Author: kz $ $Date: 2008-04-04 16:02:26 $
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
# build - build entire project
#
    use Config;
    use POSIX;
    use Cwd qw (cwd);
    use File::Path;
    use File::Temp qw(tmpnam);
    use File::Find;
    use Socket;
    use IO::Socket::INET;

    use lib ("$ENV{SOLARENV}/bin/modules");

    if (defined $ENV{COMMON_ENV_TOOLS}) {
        unshift(@INC, "$ENV{COMMON_ENV_TOOLS}/modules");
        require CopyPrj; import CopyPrj;
    };
    if (defined $ENV{CWS_WORK_STAMP}) {
        require Cws; import Cws;
        require CwsConfig; import CwsConfig;
        require CvsModule; import CvsModule;
        require GenInfoParser; import GenInfoParser;
        require IO::Handle; import IO::Handle;
    };
    my $enable_multiprocessing = 1;
    my $cygwin = 0;
    $cygwin++ if ($^O eq 'cygwin');
    if ($ENV{GUI} eq 'WNT' && !$cygwin) {
        eval { require Win32::Process; import Win32::Process; };
        $enable_multiprocessing = 0 if ($@);
    };

    ### for XML file format
    eval { require XMLBuildListParser; import XMLBuildListParser; };
    if (!$@) {
        $enable_xml = 1;
        @modes_array = split('\s' , $ENV{BUILD_TYPE});
    };
#### script id #####

    ( $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

    $id_str = ' $Revision: 1.164 $ ';
    $id_str =~ /Revision:\s+(\S+)\s+\$/
      ? ($script_rev = $1) : ($script_rev = "-");

    print "$script_name -- version: $script_rev\n";

#########################
#                       #
#   Globale Variablen   #
#                       #
#########################

    if (defined $ENV{CWS_WORK_STAMP}) {
        $vcsid = CwsConfig->new()->vcsid();
    };
    $modules_number++;
    $perl = "";
    $remove_command = "";
    if ( $^O eq 'MSWin32' ) {
        $perl = "$ENV{PERL}";
        $remove_command = "rmdir /S /Q";
        $nul = '> NULL';
    } else {
        use Cwd 'chdir';
        $perl = 'perl';
        $remove_command = 'rm -rf';
        $nul = '> /dev/null';
    };

    $QuantityToBuild = 0;
# delete $pid when not needed
    %projects_deps_hash = ();   # hash of projects with no dependencies,
                                # that could be built now
    %broken_build = ();         # hash of hashes of the modules,
                                # where build was broken (error occurred)
    %folders_hashes = ();
    %running_children = ();
    $dependencies_hash = 0;
    $cmd_file = '';
    $BuildAllParents = 0;
    $show = 0;
    $checkparents = 0;
    $deliver = 0;
    $pre_custom_job = 0;
    $custom_job = 0;
    $post_custom_job = 0;
    %LocalDepsHash = ();
    %BuildQueue = ();
    %PathHash = ();
    %PlatformHash = ();
    %AliveDependencies = ();
    %global_deps_hash = (); # hash of dependencies of the all modules
    %broken_modules_hashes = ();   # hash of modules hashes, which cannot be built further
    @broken_modules_names = ();   # array of modules, which cannot be built further
    @dmake_args = ();
    %dead_parents = ();
    $CurrentPrj = '';
    $no_projects = 0;
    $only_dependent = 0;
    $build_from = '';
    $build_all_cont = '';
    $build_since = '';
    $dlv_switch = '';
    $child = 0;
    %processes_hash = ();
#    %module_announced = ();
    $prepare = ''; # prepare for following incompatible build
    $ignore = '';
    $html = '';
    @ignored_errors = ();
    %incompatibles = ();
    $only_platform = ''; # the only platform to prepare
    $only_common = ''; # the only common output tree to delete when preparing
    %build_modes = ();
    $maximal_processes = 0; # the max number of the processes run
    %modules_types = (); # modules types ('mod', 'img', 'lnk') hash
    %platforms = (); # platforms available or being working with
    %platforms_to_copy = (); # copy output trees for the platforms when --prepare
    $tmp_dir = get_tmp_dir(); # temp directory for checkout and other actions
#    $dmake_batch = undef;     #
    @possible_build_lists = ('build.lst', 'build.xlist'); # build lists names
    %build_lists_hash = (); # hash of arrays $build_lists_hash{$module} = \($path, $xml_list_object)
    $pre_job = 'announce'; # job to add for not-single module build
    $post_job = 'deliver'; # -"-
    %windows_procs = ();
    @warnings = (); # array of warnings to be shown at the end of the process
    @errors = (); # array of errors to be shown at the end of the process
    %html_info = (); # hash containing all necessary info for generating of html page
    %module_by_hash = (); # hash containing all modules names as values and correspondent hashes as keys
    %build_in_progress = (); # hash of modules currently being built
    %build_is_finished = (); # hash of already built modules
    %modules_with_errors = (); # hash of modules with build errors
    %build_in_progress_shown = ();  # hash of modules being built,
                                    # and shown last time (to keep order)
    $build_time = time;
    $html_last_updated = 0;
    %jobs_hash = ();
    $html_path = undef;
    $html_file = CorrectPath($ENV{SRC_ROOT} . '/' . $ENV{INPATH}. '.build.html');
    $build_finished = 0;
    %had_error = (); # hack for misteriuos windows problems - try run dmake 2 times if first time there was an error
    $chekoutmissing = 1; # chekout missing modules (links still will be broken anyway)
    $mkout = CorrectPath("$ENV{SOLARENV}/bin/mkout.pl");
    %weights_hash = (); # hash contains info about how many modules are dependent from one module
#    %weight_stored = ();
    $grab_output = 1;
    $server_mode = 0;
    $setenv_string = ''; # string for configuration of the client environment
    $ports_string = ''; # string with possible ports for server
    @server_ports = ();
    $socket_obj = undef; # socket object for server
    my %clients_jobs = ();
    my %clients_times = ();
    my $client_timeout = 0; # time for client to build (in sec)...
                            # The longest time period after that
                            # the server considered as an error/client crash
    my %lost_client_jobs = (); # hash containing lost jobs
    my %job_jobdir = (); # hash containing job-dir pairs

### main ###

    get_options();

    $html_file = CorrectPath($html_path . '/' . $ENV{INPATH}. '.build.html') if (defined $html_path);
#    my $temp_html_file = CorrectPath($tmp_dir. '/' . $ENV{INPATH}. '.build.html');
    get_build_modes();
    %deliver_env = ();
    if ($prepare) {
        get_platforms(\%platforms);
        @modules_built = ();

        $deliver_env{'BUILD_SOSL'}++;
        $deliver_env{'COMMON_OUTDIR'}++;
        $deliver_env{'GUI'}++;
        $deliver_env{'INPATH'}++;
        $deliver_env{'OFFENV_PATH'}++;
        $deliver_env{'OUTPATH'}++;
        $deliver_env{'L10N_framework'}++;
    };

    $StandDir = get_stand_dir();   # This also sets $CurrentPrj
    provide_consistency() if (defined $ENV{CWS_WORK_STAMP} && defined($ENV{COMMON_ENV_TOOLS}));

    $deliver_command = $ENV{DELIVER};
    if ($dlv_switch) {
        $deliver_command .= ' '. $dlv_switch;
    } else {
        $deliver_command .= " -quiet";
    }
    $ENV{mk_tmp}++;
    %prj_platform = ();
    $check_error_string = '';
    $dmake = '';
#    $dmake_bin = '';
    $dmake_args = '';
    $echo = '';
    $new_line = "\n";

    get_commands();
    unlink ($cmd_file);
    if ($cmd_file) {
        if (open (CMD_FILE, ">>$cmd_file")) {
            select CMD_FILE;
            $echo = 'echo ';
            if ($ENV{GUI} ne 'UNX') {
                $new_line = "echo.\n";
                print "\@$echo off\npushd\n";
            } else {
                $new_line = $echo."\"\"\n";
            };
        } else {
            print_error ("Cannot open file $cmd_file");
        };
#    } elsif ($show) {
#        select STDOUT;
    };

    print $new_line;

    if ($checkparents) {
        GetParentDeps( $CurrentPrj, \%global_deps_hash );
    } else {
        BuildAll();
    }
    if (scalar keys %broken_build) {
        cancel_build();
    } elsif (!$custom_job && $post_custom_job) {
        do_post_custom_job(CorrectPath($StandDir.$CurrentPrj));
    };
    if (scalar @warnings) {
        print STDERR $_ foreach (@warnings);
    };
    if (scalar keys %dead_parents) {
        my ($DeadPrj);
        print $new_line.$new_line;
        print $echo."WARNING! Project(s):\n";
        foreach $DeadPrj (keys %dead_parents) {
            print $echo."$DeadPrj\n";
        };
        print $new_line;
        print $echo."not found and couldn't be built. Maybe you should correct build lists.\n";
        print $new_line;
        do_exit(1) if ($checkparents);
    };
    if (($ENV{GUI} ne 'UNX') && $cmd_file) {
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

#
# procedure retrieves build list path
# (all possibilities are taken into account)
#
sub get_build_list_path {
    my $module = shift;
    my @possible_dirs = ($module, $module. '.lnk');
    foreach (@possible_dirs) {
        my $possible_dir_path = $StandDir.$_.'/prj/';
        if (-d $possible_dir_path) {
            foreach (@possible_build_lists) {
                my $possible_build_list_path = $possible_dir_path . $_;
                return $possible_build_list_path if (-f $possible_build_list_path);
            }
            print_error("There's no build list for $module");
        };
    };
    $dead_parents{$module}++;
    return retrieve_build_list($module);
};

#
# Get dependencies hash of the current and all parent projects
#
sub GetParentDeps {
    my (%parents_deps_hash, $module, $parent);
    my $prj_dir = shift;
    my $deps_hash = shift;
    my @UnresolvedParents = get_parents_array($prj_dir);
    $parents_deps_hash{$_}++ foreach (@UnresolvedParents);
    $$deps_hash{$prj_dir} = \%parents_deps_hash;
    while ($module = pop(@UnresolvedParents)) {
        my %parents_deps_hash = ();
        $parents_deps_hash{$_}++ foreach (get_parents_array($module));
        $$deps_hash{$module} = \%parents_deps_hash;
        foreach $Parent (keys %parents_deps_hash) {
            if (!defined($$deps_hash{$Parent})) {
                push (@UnresolvedParents, $Parent);
            };
        };
    };
    check_deps_hash($deps_hash);
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
# This procedure implements complete in-depth weights search
# it's working, but very slow, so switched off in favor of the
# last (simplified) implementation...
#
#sub store_weights {
#    my $deps_hash = shift;
#    $weights_hash{$_} = {} foreach (keys %$deps_hash);
#    foreach (keys %$deps_hash) {
#        foreach my $module_deps_hash ($$deps_hash{$_}) {
#            foreach my $dependency (keys %$module_deps_hash) {
#                ${$weights_hash{$dependency}}{$_}++;
#            };
#        };
#    };
#    my $current_prj_preq = $$deps_hash{$CurrentPrj};
#    foreach (keys %$current_prj_preq) {
#        accumulate_weights($deps_hash, $_);
#    };
#};

#sub accumulate_weights {
#    my ($deps_hash, $module) = @_;
#    my @prerequisites = keys %{$$deps_hash{$module}};
#    my @is_prereq_for = keys %{$weights_hash{$module}};
#    foreach my $prereq_module (@prerequisites) {
##        next if (defined $weight_stored{$prereq_module});
##        $weight_stored{$prereq_module}++;
#        ${$weights_hash{$prereq_module}}{$_}++ foreach @is_prereq_for;
#        accumulate_weights($deps_hash, $prereq_module);
#    };
#};

#
# Build everything that should be built
#
sub BuildAll {
    if ($BuildAllParents) {
        my ($Prj, $PrjDir, $orig_prj);
        GetParentDeps( $CurrentPrj, \%global_deps_hash);
        modules_classify(keys %global_deps_hash);
        store_weights(\%global_deps_hash);
        prepare_build_from(\%global_deps_hash) if ($build_from);
        prepare_incompatible_build(\%global_deps_hash) if ($incompatible);
        if ($build_all_cont || $build_since) {
            prepare_build_all_cont(\%global_deps_hash);
        };
        $modules_number = scalar keys %global_deps_hash;
        initialize_html_info($_) foreach (keys %global_deps_hash);
        if ($QuantityToBuild) {
            build_multiprocessing();
            return;
        };
        if ($server_mode) {
            run_server();
        };
        while ($Prj = PickPrjToBuild(\%global_deps_hash)) {
            if (!defined $dead_parents{$Prj}) {
                print $new_line;
                if (scalar keys %broken_build) {
                    print $echo . "Skipping project $Prj because of error(s)\n";
                    RemoveFromDependencies($Prj, \%global_deps_hash);
                    $build_is_finished{$Prj}++;
                    next;
                };

                $PrjDir = CorrectPath($StandDir.$Prj);
                get_deps_hash($Prj, \%LocalDepsHash);
                my $info_hash = $html_info{$Prj};
                $$info_hash{DIRS} = check_deps_hash(\%LocalDepsHash, $Prj);
                $module_by_hash{\%LocalDepsHash} = $Prj;
                BuildDependent(\%LocalDepsHash);
                print $check_error_string;
            };

            RemoveFromDependencies($Prj, \%global_deps_hash);
            $build_is_finished{$Prj}++;
            $no_projects = 0;
        };
    } else {
        store_build_list_content($CurrentPrj);
        get_deps_hash($CurrentPrj, \%LocalDepsHash);
        initialize_html_info($CurrentPrj);
        my $info_hash = $html_info{$CurrentPrj};
        $$info_hash{DIRS} = check_deps_hash(\%LocalDepsHash, $CurrentPrj);
        $module_by_hash{\%LocalDepsHash} = $CurrentPrj;
        if ($server_mode) {
            run_server();
        } else {
            BuildDependent(\%LocalDepsHash);
        };
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
    my ($new_BuildDir, $OldBuildDir, $error_code);
    my $BuildDir = shift;
    $jobs_hash{$BuildDir}->{START_TIME} = time();
    $jobs_hash{$BuildDir}->{STATUS} = 'building';
    if ($BuildDir =~ /(\s)/o) {
        do_pre_job($`) if ($' eq $pre_job);
        $error_code = do_post_job($`, $BuildDir) if ($' eq $post_job);
        RemoveFromDependencies($BuildDir, \%LocalDepsHash);
        html_store_dmake_dir_info(\%LocalDepsHash, $BuildDir, $error_code);
    } else {
        html_store_dmake_dir_info(\%LocalDepsHash, $BuildDir);
        print_error("$BuildDir not found!!\n") if (!-d $BuildDir);
        if (!-d $BuildDir) {
            $new_BuildDir = $BuildDir;
            $new_BuildDir =~ s/_simple//g;
            if ((-d $new_BuildDir)) {
                print("\nTrying $new_BuildDir, $BuildDir not found!!\n");
                $BuildDir = $new_BuildDir;
            } else {
                print_error("\n$BuildDir not found!!\n");
            }
        }
        if ($cmd_file) {
            print "cd $BuildDir\n";
            print $check_error_string;
            print $echo.$BuildDir."\n";
            print "$dmake\n";
            print $check_error_string;
        } else {
            print "$BuildDir\n";
        };
        RemoveFromDependencies($BuildDir, \%LocalDepsHash) if (!$child);
        return if ($cmd_file || $show);
        chdir $BuildDir;
        getcwd();
        if ($html) {
            my $log_file = $jobs_hash{$BuildDir}->{LONG_LOG_PATH};
            my $log_dir = File::Basename::dirname($log_file);
            if (!-d $log_dir) {
                 system("$perl $mkout");
            };
            $error_code = system ("$dmake > $log_file 2>&1");
            if (!$grab_output && -f $log_file) {
                system("cat $log_file");
            };
        } else {
            $error_code = system ("$dmake");
        };
        html_store_dmake_dir_info(\%LocalDepsHash, $BuildDir, $error_code) if (!$child);
    };
    if ($error_code && $ignore) {
        push(@ignored_errors, $BuildDir);
        $error_code = 0;
    };
    if ($child) {
        my $oldfh = select STDERR;
        $| = 1;
        select $oldfh;
        $| =1;
        _exit($? >> 8) if ($? && ($? != -1));
        _exit(0);
    } elsif ($error_code && ($error_code != -1)) {
        print_error("Error $? occurred while making $BuildDir");
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
    my $xml_list = undef;
    if ($build_list_path =~ /\.xlist$/o) {
        print_error("XMLBuildListParser.pm couldn\'t be found, so XML format for build lists is not enabled") if (!defined $enable_xml);
        $xml_list = XMLBuildListParser->new();
        if (!$xml_list->loadXMLFile($build_list_path)) {
            print_error("Cannot use $build_list_path");
        };
        $build_lists_hash{$module} = $xml_list;
    } else {
        if (open (BUILD_LST, $build_list_path)) {
            my @build_lst = <BUILD_LST>;
            $build_lists_hash{$module} = \@build_lst;
            close BUILD_LST;
            return;
        }
        $dead_parents{$module}++;
    };
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
            if ($' =~ /\s*-\s+(\w+)[,\S+]*\s+(\S+)/ ) {
                my $platform = $1;
                my $alias = $2;
                print_error ("There is no correct alias set in the line $line!") if ($alias eq 'NULL');
                mark_platform($alias, $platform);
            } else {
                print_error("Misspelling in line: \n$_");
            };
        };
    };
#seek(BUILD_LST, 0, 0);
};

#
# Procedure populate the dependencies hash with
# information from XML build list object
#
sub get_deps_from_object {
    my ($module, $build_list_object, $dependencies_hash) = @_;

    foreach my $dir ($build_list_object->getJobDirectories("make", $ENV{GUI})) {
        $PathHash{$dir} = $StandDir . $module;
        $PathHash{$dir} .= $dir if ($dir ne '/');
        my %deps_hash = ();

        foreach my $dep ($build_list_object->getJobDependencies($dir, "make", $ENV{GUI})) {
            $deps_hash{$dep}++;
        };
        $$dependencies_hash{$dir} = \%deps_hash;
    };
};


#
# Getting hashes of all internal dependencies and additional
# information for given project
#
sub get_deps_hash {
    my ($dummy, $module_to_build);
    %DeadDependencies = ();
    $module_to_build = shift;
    my $dependencies_hash = shift;
    if ($deliver || $custom_job) {
        add_pre_job($dependencies_hash, $module_to_build);
        add_post_job($dependencies_hash, $module_to_build) if ($modules_types{$module_to_build} ne 'lnk');
        return;
    };
    if ( defined $modules_types{$module_to_build} && $modules_types{$module_to_build} ne 'mod') {
        add_pre_job($dependencies_hash, $module_to_build);
        return;
    };

    my  $build_list_ref = $build_lists_hash{$module_to_build};
    delete $build_lists_hash{$module_to_build};
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
                my ($Platform, $Dependencies, $Dir, $DirAlias);
                my %deps_hash = ();
                $Dependencies = $';
                $dummy = $`;
                $dummy =~ /(\S+)\s+(\S*)/o;
                $Dir = $2;
                $Dependencies =~ /(\w+)/o;
                $Platform = $1;
                $Dependencies = $';
                while ($Dependencies =~ /,(\w+)/o) {
                    $Dependencies = $';
                };
                $Dependencies =~ /\s+(\S+)\s+/o;
                $DirAlias = $1;
                if (!CheckPlatform($Platform)) {
                    next if (defined $PlatformHash{$DirAlias});
                    $DeadDependencies{$DirAlias}++;
                    next;
                };
                delete $DeadDependencies{$DirAlias} if (defined $DeadDependencies{$DirAlias});
                print_error("Directory alias $DirAlias is defined at least twice!! Please, correct build.lst in module $module_to_build") if (defined $$dependencies_hash{$DirAlias});
                $PlatformHash{$DirAlias}++;
                $Dependencies = $';
                print_error("$module_to_build/prj/build.lst has wrongly written dependencies string:\n$_\n") if (!$Dependencies);
                $deps_hash{$_}++ foreach (GetDependenciesArray($Dependencies));
                $$dependencies_hash{$DirAlias} = \%deps_hash;
                $BuildQueue{$DirAlias}++;
                if ($Dir =~ /(\\|\/)/o) {
                    $Dir = $module_to_build . $1 . $';
                } else {$Dir = $module_to_build;};
                $PathHash{$DirAlias} = CorrectPath($StandDir . $Dir);
            } elsif ($_ !~ /^\s*$/ && $_ !~ /^\w*\s/o) {
                chomp;
                push(@errors, $_);
            };
        };
        if (scalar @errors) {
            my $message = "$module_to_build/prj/build.lst has wrongly written string(s):\n";
            $message .= "$_\n" foreach(@errors);
            if ($QuantityToBuild) {
                $broken_build{$module_to_build} = $message;
                $dependencies_hash = undef;
                return;
            } else {
                print_error($message);
            };
        };
        foreach my $alias (keys %DeadDependencies) {
            next if defined $AliveDependencies{$alias};
            if (!IsHashNative($alias)) {
                RemoveFromDependencies($alias, $dependencies_hash);
                delete $DeadDependencies{$alias};
            };
        };
    };
#    check_deps_hash($dependencies_hash);
    resolve_aliases($dependencies_hash, \%PathHash);
    if (!$prepare) {
        add_pre_job($dependencies_hash, $module_to_build);
        add_post_job($dependencies_hash, $module_to_build) if ($module_to_build ne $CurrentPrj);
    };
    store_weights($dependencies_hash);
};

#
# procedure adds $pre_job to each module's dependancy hash
#
sub add_pre_job {
    my ($dependencies_hash, $module) = @_;
    my $job = "$module $pre_job";
    # $pre_job is independent while all other jobs are dependent from it
    foreach (keys %$dependencies_hash) {
        $deps_hash = $$dependencies_hash{$_};
        $$deps_hash{$job}++;
    };
    $$dependencies_hash{$job} = {};

};

#
# procedure adds $post_job to each module's dependancy hash
#
sub add_post_job {
    # $post_job is dependent from all jobs
    my ($dependencies_hash, $module) = @_;
    my %deps_hash = ();
    $deps_hash{$_}++ foreach (keys %$dependencies_hash);
    $$dependencies_hash{"$module $post_job"} = \%deps_hash;
};

#
# this procedure converts aliases to absolute paths
#
sub resolve_aliases {
    my ($dependencies_hash, $PathHash) = @_;
    foreach my $dir_alias (keys %$dependencies_hash) {
        my $aliases_hash_ref = $$dependencies_hash{$dir_alias};
        my %paths_hash = ();
        foreach (keys %$aliases_hash_ref) {
            $paths_hash{$$PathHash{$_}}++;
        };
        delete $$dependencies_hash{$dir_alias};
        $$dependencies_hash{$$PathHash{$dir_alias}} = \%paths_hash;
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
sub CorrectPath {
    $_ = shift;
    if ( ($^O eq 'MSWin32') && (!defined $ENV{SHELL})) {
        s/\//\\/g;
    } else {;
        s/\\/\//g;
    };
    return $_;
};


sub check_dmake {
#print "Checking dmake...";
#    my $dmake_batch = CorrectPath("$tmp_dir/dmake.bat");
    if ($QuantityToBuild && ($ENV{GUI} eq 'WNT') && ($ENV{USE_SHELL} eq '4nt')) {
        if (open(DMAKEVERSION, "where dmake |")) {
            my @output = <DMAKEVERSION>;
            close DMAKEVERSION;
            $dmake_bin = $output[0];
            $dmake_bin =~ /(\b)$/;
            $dmake_bin = $`;
        };
        return if (-e $dmake_bin);
    } elsif (open(DMAKEVERSION, "dmake -V |")) {
#    if (open(DMAKEVERSION, "dmake -V |")) {
        my @dmake_version = <DMAKEVERSION>;
        close DMAKEVERSION;
#       if ($dmake_version[0] =~ /^dmake\s\-\sCopyright\s\(c\)/) {
#            print " Using version $1\n" if ($dmake_version[0] =~ /Version\s(\d+\.*\d*)/);
#        };
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
        if ($ENV{GUI} eq 'UNX') {
            $check_error_string = "if \"\$?\" != \"0\" exit\n";
        } else {
            $check_error_string = "if \"\%?\" != \"0\" quit\n";
        };
    };

    $dmake_args = join(' ', 'dmake', @dmake_args);

    while ($arg = pop(@dmake_args)) {
        $dmake .= ' '.$arg;
    };
#    if (($ENV{GUI} eq 'WNT') && $QuantityToBuild) {
#        print_error("There is no such executable $_4nt_exe") if (!-e $_4nt_exe);
#        $dmake_batch = generate_4nt_batch();
#    };
};

#
# Procedure prooves if current dir is a root dir of the drive
#
#sub IsRootDir {
#    my ($Dir);
#    $Dir = shift;
#    if (        (($ENV{GUI} eq 'UNX') ||
#                 ($ENV{GUI} eq 'MACOSX')) &&
#                ($Dir eq '/')) {
#        return 1;
#    } elsif (    (($ENV{GUI} eq 'WNT') ||
#                 ($ENV{GUI} eq 'WIN') ||
#                 ($ENV{GUI} eq 'OS2')) &&
#                ($Dir =~ /\S:\/$/)) {
#        return 1;
#    } else {
#        return 0;
#    };
#};

#
# Procedure retrieves list of projects to be built from build.lst
#
sub get_stand_dir {
    if (!(defined $ENV{GUI})) {
        $ENV{mk_tmp} = '';
        die "No environment set\n";
    };
    my $StandDir;
    my $root_dir = File::Spec->rootdir();
    $StandDir = getcwd();
    do {
        foreach (@possible_build_lists) {# ('build.lst', 'build.xlist');
            if (-e $StandDir . '/prj/'.$_) {
                $StandDir =~ /([\.\w]+$)/;
                $StandDir = $`;
                $CurrentPrj = $1;
                return $StandDir;
            } elsif ($StandDir eq $root_dir) {
                $ENV{mk_tmp} = '';
                print_error('Found no project to build');
            };
        };
        $StandDir = Cwd::realpath($StandDir . '/..');
    }
    while (chdir '..');
};

#
# Picks project which can be built now from hash and then deletes it from hash
#
sub PickPrjToBuild {
    my $DepsHash = shift;
    handle_dead_children(0) if ($QuantityToBuild);
    my $Prj = FindIndepPrj($DepsHash);
    delete $$DepsHash{$Prj};
    generate_html_file();
    return $Prj;
};

#
# Make a decision if the project should be built on this platform
#
sub CheckPlatform {
    my $Platform = shift;
    return 1 if ($Platform eq 'all');
    return 1 if (($ENV{GUI} eq 'WIN') && ($Platform eq 'w'));
    return 1 if (($ENV{GUI} eq 'UNX') && ($Platform eq 'u'));
    return 1 if (($ENV{GUI} eq 'OS2') && ($Platform eq 'p'));
    return 1 if (($ENV{GUI} eq 'WNT') &&
                       (($Platform eq 'w') || ($Platform eq 'n')));
    return 0;
};

#
# Remove project to build ahead from dependencies and make an array
# of all from given project dependent projects
#
sub RemoveFromDependencies {
    my ($ExclPrj, $i, $Prj, $Dependencies);
    $ExclPrj = shift;
    my $ExclPrj_orig = '';
    $ExclPrj_orig = $` if ($ExclPrj =~ /\.lnk$/o);
    $Dependencies = shift;
    foreach $Prj (keys %$Dependencies) {
        my $prj_deps_hash = $$Dependencies{$Prj};
        delete $$prj_deps_hash{$ExclPrj} if (defined $$prj_deps_hash{$ExclPrj});
#        foreach (keys %$prj_deps_hash) {print ("$_ ")};

#        foreach $i (0 .. $#{$$Dependencies{$Prj}}) {
#            if ((${$$Dependencies{$Prj}}[$i] eq $ExclPrj) ||
#                (${$$Dependencies{$Prj}}[$i] eq $ExclPrj_orig)) {
#                splice (@{$$Dependencies{$Prj}}, $i, 1);
#                $i = 0;
#                last;
#            };
#        };
    };
};


#
# Check the hash for consistency
#
sub check_deps_hash {
    my ($deps_hash_ref, $module) = @_;
    my @possible_order;
    my $module_path = $StandDir . $module if (defined $module);
    return if (!scalar keys %$deps_hash_ref);
    my %deps_hash = %$deps_hash_ref;
    my $consistent;
    foreach $key (keys %$deps_hash_ref) {
        my %values_hash = %{$$deps_hash_ref{$key}};
        $deps_hash{$key} = \%values_hash;
    };
    my $string;
    my $log_name;
    my $build_number = 0;

    do {
        $consistent = '';
        foreach $key (sort keys %deps_hash) {
            $local_deps_ref = $deps_hash{$key};
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
                        $log_name = "$module custom_job";
                    };
                    $log_name =~ s/\\|\//\./g;
                    $log_name =~ s/\s/_/g;
                    $log_name = $module if ($log_name =~ /^\.+$/);
                    $log_name .= '.txt';
                    push(@possible_order, $key);
                    $jobs_hash{$key} = {    SHORT_NAME => $string,
                                            BUILD_NUMBER => $build_number,
                                            STATUS => 'waiting',
                                            LOG_PATH => $module . "/$ENV{INPATH}/misc/logs/$log_name",
                                            LONG_LOG_PATH => CorrectPath($StandDir . $module . "/$ENV{INPATH}/misc/logs/$log_name"),
                                            START_TIME => 0,
                                            FINISH_TIME => 0,
                                            CLIENT => '-'
                    };
#                    if (-f $jobs_hash{$key}->{LONG_LOG_PATH} && $html) {
#                        print STDERR 'Warning: log file ' . $jobs_hash{$key}->{LONG_LOG_PATH} . " already exists!!\n";
#                    };
                };
                RemoveFromDependencies($key, \%deps_hash);
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
sub FindIndepPrj {
    my ($Prj, @Prjs, $Dependencies, $i);
    my @candidates = ();
    my $children = children_number();
    return '' if (!$server_mode && $children && ($children >= $QuantityToBuild));
    $Dependencies = shift;
    @Prjs = keys %$Dependencies;
    if ($#Prjs != -1) {
        foreach $Prj (@Prjs) {
            next if (&IsHashNative($Prj));
            my $PrjDeps = $$Dependencies{$Prj};
            push(@candidates, $Prj) if (!scalar keys %$PrjDeps);
            #return $Prj if (!scalar keys %$PrjDeps);
        };
        if (scalar @candidates) {
            my $best_candidate = undef;
            my $weight = 0;
            foreach my $candidate (sort @candidates) {
#                if (defined $weights_hash{$candidate} && scalar keys %{$weights_hash{$candidate}} > $weight) {
                if (defined $weights_hash{$candidate} && $weights_hash{$candidate} > $weight) {
                    $best_candidate = $candidate;
#                    $weight = scalar keys %{$weights_hash{$candidate}};
                    $weight = $weights_hash{$candidate};
                };
            };
            if (defined $best_candidate) {
#                delete $weights_hash{$best_candidate};
                return $best_candidate;
            }
            my @sorted_candidates = sort(@candidates);
            return $sorted_candidates[0];
        };
        return '';
    } else {
        $no_projects = 1;
        return '';
    };
};

#
# Check if given entry is HASH-native, that is not a user-defined data
#
sub IsHashNative {
    my $Prj = shift;
    return 1 if ($Prj =~ /^HASH\(0x[\d | a | b | c | d | e | f]{6,}\)/);
    return 0;
};

#
# Getting array of dependencies from the string given
#
sub GetDependenciesArray {
    my ($DepString, @Dependencies, $ParentPrj, $prj, $string);
    @Dependencies = ();
    $DepString = shift;
    $string = $DepString;
    $prj = shift;
    while ($DepString !~ /^NULL/o) {
        print_error("Project $prj has wrongly written dependencies string:\n $string") if (!$DepString);
        $DepString =~ /(\S+)\s*/o;
        $ParentPrj = $1;
        $DepString = $';
        if ($ParentPrj =~ /\.(\w+)$/o) {
            $ParentPrj = $`;
            if (($prj_platform{$ParentPrj} ne $1) &&
                ($prj_platform{$ParentPrj} ne 'all')) {
                print_error ("$ParentPrj\.$1 is a wrongly dependency identifier!\nCheck if it is platform dependent");
            };
            $AliveDependencies{$ParentPrj}++ if (CheckPlatform($1));
            push(@Dependencies, $ParentPrj);
        } else {
            if ((exists($prj_platform{$ParentPrj})) &&
                ($prj_platform{$ParentPrj} ne 'all') ) {
                print_error("$ParentPrj is a wrongly used dependency identifier!\nCheck if it is platform dependent");
            };
            push(@Dependencies, $ParentPrj);
        };
    };
    return @Dependencies;
};


#
# Getting current directory list
#
sub GetDirectoryList {
    my ($Path);
    $Path = shift;
    opendir(CurrentDirList, $Path);
    @DirectoryList = readdir(CurrentDirList);
    closedir(CurrentDirList);
    return @DirectoryList;
};

sub print_error {
    my $message = shift;
    my $force = shift;
#    rmtree(CorrectPath($tmp_dir), 0, 1) if ($tmp_dir);
    $modules_number -= scalar keys %global_deps_hash;
    $modules_number -= 1;
    print STDERR "\nERROR: $message\n";
    $ENV{mk_tmp} = '';
    close CMD_FILE if ($cmd_file);
    unlink ($cmd_file);
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
    print STDERR "Syntax:    build    [--all|-a[:prj_name]]|[--from|-f prj_name1[:prj_name2] [prj_name3 [...]]]|[--since|-c prj_name] [--with_branches|-b]|[--prepare|-p][:platform] [--dontchekoutmissingmodules]] [--deliver|-d [--dlv_switch deliver_switch]]] [-P processes|--server [--setenvstring \"string\"] [--client_timeout MIN] [--port port1[:port2:...:portN]]] [--show|-s] [--help|-h] [--file|-F] [--ignore|-i] [--version|-V] [--mode|-m OOo[,SO[,EXT]] [--html [--html_path html_file_path] [--dontgraboutput]] [--pre_job=pre_job_sring] [--job=job_string|-j] [--post_job=post_job_sring]\n";
    print STDERR "Example1:    build --from sfx2\n";
    print STDERR "                     - build all projects dependent from sfx2, starting with sfx2, finishing with the current module\n";
    print STDERR "Example2:    build --all:sfx2\n";
    print STDERR "                     - the same as --all, but skip all projects that have been already built when using \"--all\" switch before sfx2\n";
    print STDERR "Example3:    build --all --server\n";
    print STDERR "                     - build all projects in server mode, use first available port from default range 7890-7894 (running clients required!!)\n";
    print STDERR "Example4(for unixes):\n";
    print STDERR "             build --all --pre_job=echo\\ Starting\\ job\\ in\\ \\\$PWD --job=some_script.sh --post_job=echo\\ Job\\ in\\ \\\$PWD\\ is\\ made\n";
    print STDERR "                     - go through all projects, echo \"Starting job in \$PWD\" in each module, execute script some_script.sh, and finally echo \"Job in \$PWD is made\"\n";
    print STDERR "\nSwitches:\n";
    print STDERR "        --all        - build all projects from very beginning till current one\n";
    print STDERR "        --from       - build all projects dependent from the specified (including it) till current one\n";
    print STDERR "        --mode OOo   - build only projects needed for OpenOffice.org\n";
    print STDERR "        --prepare    - clear all projects for incompatible build from prj_name till current one [for platform] (cws version)\n";
    print STDERR "        --with_branches- build all projects in neighbour branches and current branch starting from actual project\n";
    print STDERR "        --since      - build all projects beginning from the specified till current one (the same as \"--all:prj_name\", but skipping prj_name)\n";
    print STDERR "        --checkmodules      - check if all required parent projects are availlable\n";
    print STDERR "        --show       - show what is going to be built\n";
    print STDERR "        --file       - generate command file file_name\n";
    print STDERR "        --deliver    - only deliver, no build (usable for \'-all\' and \'-from\' keys)\n";
    print STDERR "        -P           - start multiprocessing build, with number of processes passed\n";
    print STDERR "        --server     - start build in server mode (clients required)\n";
    print STDERR "          --setenvstring  - string for configuration of the client environment\n";
    print STDERR "          --port          - set server port, default is 7890. You may pass several ports, the server will be started on the first available\n";
    print STDERR "                            otherwise the server will be started on first available port from the default range 7890-7894\n";
    print STDERR "          --client_timeout  - time frame after which the client/job is considered to be lost. Default is 120 min\n";
    print STDERR "        --dlv_switch - use deliver with the switch specified\n";
    print STDERR "        --help       - print help info\n";
    print STDERR "        --ignore     - force tool to ignore errors\n";
    print STDERR "        --html       - generate html page with build status\n";
    print STDERR "                       file named $ENV{INPATH}.build.html will be generated in $ENV{SRC_ROOT}\n";
    print STDERR "          --html_path      - set html page path\n";
    print STDERR "          --dontgraboutput - do not grab console output when generating html page\n";
    print STDERR "        --dontchekoutmissingmodules - do not chekout missing modules when running prepare (links still will be broken)\n";
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
    my ($arg, $dont_grab_output);
    while ($arg = shift @ARGV) {
        $arg =~ /^-P$/            and $QuantityToBuild = shift @ARGV     and next;
        $arg =~ /^-P(\d+)$/            and $QuantityToBuild = $1 and next;
        $arg =~ /^--all$/        and $BuildAllParents = 1             and next;
        $arg =~ /^-a$/        and $BuildAllParents = 1             and next;
        $arg =~ /^--show$/        and $show = 1                         and next;
        $arg =~ /^--checkmodules$/       and $checkparents = 1 and $ignore = 1 and next;
        $arg =~ /^-s$/        and $show = 1                         and next;
        $arg =~ /^--deliver$/    and $deliver = 1                     and next;
        $arg =~ /^(--job=)/       and $custom_job = $' and next;
        $arg =~ /^(--pre_job=)/       and $pre_custom_job = $' and next;
        $arg =~ /^(--post_job=)/       and $post_custom_job = $' and next;
        $arg =~ /^-d$/    and $deliver = 1                     and next;
        $arg =~ /^--dlv_switch$/    and $dlv_switch = shift @ARGV    and next;
        $arg =~ /^--file$/        and $cmd_file = shift @ARGV             and next;
        $arg =~ /^-F$/        and $cmd_file = shift @ARGV             and next;

        $arg =~ /^--with_branches$/        and $BuildAllParents = 1
                                and $build_from = shift @ARGV         and next;
        $arg =~ /^--dontchekoutmissingmodules$/    and $chekoutmissing = 0    and next;
        $arg =~ /^-b$/        and $BuildAllParents = 1
                                and $build_from = shift @ARGV         and next;

        $arg =~ /^--all:(\S+)$/ and $BuildAllParents = 1
                                and $build_all_cont = $1            and next;
        $arg =~ /^-a:(\S+)$/ and $BuildAllParents = 1
                                and $build_all_cont = $1            and next;
        if ($arg =~ /^--from$/ || $arg =~ /^-f$/) {
                                    $BuildAllParents = 1;
                                    get_incomp_projects();
                                    next;
        };
        $arg =~ /^--prepare$/    and $prepare = 1 and next;
        $arg =~ /^-p$/            and $prepare = 1 and next;
        $arg =~ /^--prepare:/    and $prepare = 1 and $only_platform = $' and next;
        $arg =~ /^-p:/            and $prepare = 1 and $only_platform = $' and next;
        $arg =~ /^--since$/        and $BuildAllParents = 1
                                and $build_since = shift @ARGV         and next;
        $arg =~ /^-c$/        and $BuildAllParents = 1
                                and $build_since = shift @ARGV         and next;
        $arg =~ /^-s$/            and $BuildAllParents = 1
                                and $build_since = shift @ARGV         and next;
        $arg =~ /^--help$/        and usage()                            and do_exit(0);
        $arg =~ /^-h$/        and usage()                            and do_exit(0);
        $arg =~ /^--ignore$/        and $ignore = 1                            and next;
        $arg =~ /^--html$/        and $html = 1                            and next;
        $arg =~ /^--dontgraboutput$/        and $dont_grab_output = 1      and next;
        $arg =~ /^--html_path$/ and $html_path = shift @ARGV  and next;
        $arg =~ /^-i$/        and $ignore = 1                            and next;
        $arg =~ /^--server$/        and $server_mode = 1                      and next;
        $arg =~ /^--client_timeout$/ and $client_timeout = (shift @ARGV)*60  and next;
        $arg =~ /^--setenvstring$/            and $setenv_string =  shift @ARGV         and next;
        $arg =~ /^--port$/            and $ports_string =  shift @ARGV         and next;
        $arg =~ /^--version$/   and do_exit(0);
        $arg =~ /^-V$/          and do_exit(0);
        $arg =~ /^-m$/            and get_modes()         and next;
        $arg =~ /^--mode$/        and get_modes()         and next;
        if ($arg =~ /^--$/) {
            push (@dmake_args, get_dmake_args()) if (!$custom_job);
            next;
        };
        push (@dmake_args, $arg);
    };
    if (!$html) {
        print_error("\"--html_path\" switch is used only with \"--html\"") if ($html_path);
        print_error("\"--dontgraboutput\" switch is used only with \"--html\"") if ($dont_grab_output);
    };
    $grab_output = 0 if ($dont_grab_output);
    print_error('Switches --with_branches and --all collision') if ($build_from && $build_all_cont);
    print_error('Please prepare the workspace on one of UNIX platforms') if ($prepare && ($ENV{GUI} ne 'UNX'));
    print_error('Switches --with_branches and --since collision') if ($build_from && $build_since);
    if ($show) {
        $QuantityToBuild = 0;
        $cmd_file = '';
    };
    print_error('Switches --job and --deliver collision') if ($custom_job && $deliver);
    $post_job = $custom_job if ($custom_job);
    $incompatible = scalar keys %incompatibles;
    if ($prepare && !$incompatible) {
        print_error("--prepare is for use with --from switch only!\n");
    };
    if ($QuantityToBuild) {
        if ($ignore && !$html) {
            print_error("Cannot ignore errors in multiprocessing build");
        };
        if (!$enable_multiprocessing) {
            print_error("Cannot load Win32::Process module for multiprocessing build");
        };
        if ($server_mode) {
            print_error("Switches -P and --server collision");
        };
    };
    if ($server_mode) {
        $html++;
        $client_timeout = 60 * 60 * 2 if (!$client_timeout);
    } else {
        print_error("--ports switch is for server mode only!!") if ($ports_string);
        print_error("--setenvstring switch is for server mode only!!") if ($setenv_string);
        print_error("--client_timeout switch is for server mode only!!") if ($client_timeout);
    };
#    $ignore++ if ($html);
    if ($only_platform) {
        $only_common = 'common';
        $only_common .= '.pro' if ($only_platform =~ /\.pro$/);
    };
    # Default build modes(for OpenOffice.org)
    $ENV{BUILD_TYPE} = 'OOo EXT' if (!defined $ENV{BUILD_TYPE});
    @ARGV = @dmake_args;
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
# get all options without '-'
#
sub get_switch_options {
    my $string = '';
    my $option = '';
    while ($option = shift @ARGV) {
        if (!($option =~ /^-+/)) {
            $string .= '-' . $option;
            $string .= ' ';
        } else {
            unshift(@ARGV, $option);
            last;
        };
    };
    $string =~ s/\s$//;
    return $string;
};

#
# cancel build when one of children has error exit code
#
sub cancel_build {
#    close_server_socket();
    $modules_number -= scalar keys %global_deps_hash;
    my $broken_modules_number = scalar @broken_modules_names;
    if ($broken_modules_number) {
        $modules_number -= $broken_modules_number;
        print "\n";
        print $broken_modules_number;
        print " module(s): ";
        foreach (@broken_modules_names) {
            print "\n\t$_";
#            RemoveFromDependencies($_, \%global_deps_hash);
        };
        print "\nneed(s) to be rebuilt\n\nReason(s):\n\n";
        foreach (keys %broken_build) {
            print "ERROR: error " . $broken_build{$_} . " occurred while making $_\n";
        };
        print "\nAttention: if you build and deliver the above module(s) you may prolongue your the build issuing command \"build --from @broken_modules_names\"\n";
    } else {
#        if ($ENV{GUI} eq 'WNT') {
            while (children_number()) {
                handle_dead_children(1);
            }
            foreach (keys %broken_build) {
                print "ERROR: error " . $broken_build{$_} . " occurred while making $_\n";
            };
#        } else {
#            kill 9 => -$$;
#        };
    };
    print "\n";
    do_exit(1);
};

#
# Function for storing error in multiprocessing AllParents build
#
sub store_error {
    my ($pid, $error_code) = @_;
    my $child_nick = $processes_hash{$pid};
    if ($ENV{GUI} eq 'WNT' && !$cygwin) {
        if (!defined $had_error{$child_nick}) {
            $had_error{$child_nick}++;
            return 1;
        };
    };
    $broken_modules_hashes{$folders_hashes{$child_nick}}++;
    $broken_build{$child_nick} = $error_code;
    return 0;
};

#
# child handler (clears (or stores info about) the terminated child)
#
sub handle_dead_children {
    my $running_children = children_number();
    return if (!$running_children);
    my $force_wait = shift;
    do {
        my $pid = 0;
        if ($ENV{GUI} eq 'WNT' && !$cygwin) {
            foreach $pid (keys %processes_hash) {
                my $exit_code  = undef;
                my $proc_obj = $windows_procs{$pid};
                $proc_obj->GetExitCode($exit_code);
                if ( $exit_code != 259 ) {
                    my $try_once_more = 0;
                    $try_once_more  = store_error($pid, $exit_code) if ($exit_code);
                    delete $windows_procs{$pid};
                    if ($try_once_more) {
                        # A malicious hack for misterious windows problems - try 2 times
                        # to run dmake in the same directory if errors occurs
                        my $child_nick = $processes_hash{$pid};
                        $running_children{$folders_hashes{$child_nick}}--;
                        delete $processes_hash{$pid};
                        start_child($child_nick, $folders_hashes{$child_nick});
                    } else {
                        clear_from_child($pid);
                    };
                };
            };
            sleep 1 if (children_number() >= $QuantityToBuild || ($force_wait && ($running_children == children_number())));
        } else {
            if (children_number() >= $QuantityToBuild ||
                    ($force_wait && ($running_children == children_number()))) {
                $pid = wait();
            } else {
                $pid = waitpid( -1, &WNOHANG);
            };
            if ($pid > 0) {
                store_error($pid, $?) if ($?);
                clear_from_child($pid);
            };
        };
    } while(children_number() >= $QuantityToBuild);
};

sub clear_from_child {
    my $pid = shift;
    my $child_nick = $processes_hash{$pid};
    my $error_code = 0;
    if (defined $broken_build{$child_nick}) {
        $error_code = $broken_build{$child_nick};
    } else {
        RemoveFromDependencies($child_nick,
                            $folders_hashes{$child_nick});
    };
    my $module = $module_by_hash{$folders_hashes{$child_nick}};
    html_store_dmake_dir_info($folders_hashes{$child_nick}, $child_nick, $error_code);
    $running_children{$folders_hashes{$child_nick}}--;
    delete $processes_hash{$pid};
    $only_dependent = 0;
    print 'Running processes: ' . children_number() . "\n";
};

#
# Build the entire project according to queue of dependencies
#
sub BuildDependent {
    $dependencies_hash = shift;
    my $pid = 0;
    my $child_nick = '';
    $running_children{$dependencies_hash} = 0 if (!defined $running_children{$dependencies_hash});
    while ($child_nick = PickPrjToBuild($dependencies_hash)) {
        if (($QuantityToBuild)) { # multiprocessing not for $BuildAllParents (-all etc)!!
            do {
                handle_dead_children(0);
                if (defined $broken_modules_hashes{$dependencies_hash} && !$ignore) {
                    return if ($BuildAllParents);
                    last;
                };
                # start current child & all
                # that could be started now
                start_child($child_nick, $dependencies_hash) if ($child_nick);
                $child_nick = PickPrjToBuild($dependencies_hash);
                if (!$child_nick) {
                    return if ($BuildAllParents);
                    handle_dead_children(1) if (!$no_projects);
                };
            } while (!$no_projects);
            return if ($BuildAllParents);
            while (children_number()) {
                handle_dead_children(1);
            };
#            if (defined $last_module) {
#                $build_is_finished{$last_module}++ if (!defined $modules_with_errors{$last_module});
#            };

            if (defined $broken_modules_hashes{$dependencies_hash}) {
                cancel_build();
            }
            mp_success_exit();
        } else {
            dmake_dir($child_nick);
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
        do_pre_job($`) if ($' eq $pre_job);
        $error_code = do_post_job($`, $job_dir) if ($' eq $post_job);
        html_store_dmake_dir_info($dependencies_hash, $job_dir, $error_code);
        if ($error_code) {
            # give windows (4nt) one more chance
            if ($ENV{GUI} eq 'WNT' && !$cygwin) {
                $error_code = do_post_job($`, $job_dir);
            };
            if ($error_code) {
                $broken_modules_hashes{$dependencies_hash}++;
                $broken_build{$job_dir} = $error_code;
            }
        } else {
            RemoveFromDependencies($job_dir, $dependencies_hash);
        };
        return;
    };
    html_store_dmake_dir_info($dependencies_hash, $job_dir);
    my $pid = undef;
    my $children_running;
    my $oldfh = select STDOUT;
    $| = 1;
    if ($ENV{GUI} eq 'WNT' && !$cygwin) {
        print "$job_dir\n";
        my $process_obj = undef;
        my $rc = Win32::Process::Create($process_obj, $dmake_bin,
                                    $dmake_args,
                                    0, 0, #NORMAL_PRIORITY_CLASS,
                                    $job_dir);
#        my $rc = Win32::Process::Create($process_obj, $_4nt_exe,
#                                    "/c $dmake_batch",
#                                   0, NORMAL_PRIORITY_CLASS,
#                                    $job_dir);
        print_error("Cannot start child process") if (!$rc);
        $pid = $process_obj->GetProcessID();
        $windows_procs{$pid} = $process_obj;
    } else {
        if ($pid = fork) { # parent
        } elsif (defined $pid) { # child
            select $oldfh;
            $child = 1;
            dmake_dir($job_dir);
            do_exit(1);
        };
    };
    select $oldfh;
    $processes_hash{$pid} = $job_dir;
    $children_running = children_number();
    print 'Running processes: ', $children_running, "\n";
    $maximal_processes = $children_running if ($children_running > $maximal_processes);
    $folders_hashes{$job_dir} = $dependencies_hash;
    $running_children{$dependencies_hash}++;
};

#
# Build everything that should be built multiprocessing version
#
sub build_multiprocessing {
    my $Prj;
    my @build_queue = ();        # array, containing queue of projects
                                # to build
    do {
        while ($Prj = PickPrjToBuild(\%global_deps_hash)) {
            my $module_type = $modules_types{$Prj};
            push @build_queue, $Prj;
            $projects_deps_hash{$Prj} = {};
            get_deps_hash($Prj, $projects_deps_hash{$Prj});
            my $info_hash = $html_info{$Prj};
            $$info_hash{DIRS} = check_deps_hash($projects_deps_hash{$Prj}, $Prj);
            $module_by_hash{$projects_deps_hash{$Prj}} = $Prj;
        };
        if (!$Prj || !defined $projects_deps_hash{$Prj}) {
            cancel_build() if (!scalar @build_queue && !children_number());
            handle_dead_children(1);
        }
        build_actual_queue(\@build_queue);
    } while (scalar (keys %global_deps_hash));
    # Let the last module be built till the end
    while (scalar @build_queue) {
        build_actual_queue(\@build_queue);
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
#    close_server_socket();
    if (!$custom_job && $post_custom_job) {
        do_post_custom_job(CorrectPath($StandDir.$CurrentPrj));
    };
    print "\nMultiprocessing build is finished\n";
    print "Maximal number of processes run: $maximal_processes\n";
    do_exit(0);
};

#
# Here the built queue is built as long as possible
#
sub build_actual_queue {
    my $build_queue = shift;
    my $i = 0;
    do {
        while ($i <= (scalar(@$build_queue) - 1)) {
            $Prj = $$build_queue[$i];
            if (defined $broken_modules_hashes{$projects_deps_hash{$Prj}} && !$ignore) {
                push (@broken_modules_names, $Prj);
                splice (@$build_queue, $i, 1);
                next;
            };
            $only_dependent = 0;
            $no_projects = 0;
            BuildDependent($projects_deps_hash{$Prj});
            handle_dead_children(0);
            if ($no_projects &&
                !$running_children{$projects_deps_hash{$Prj}}) {
                if (!defined $broken_modules_hashes{$projects_deps_hash{$Prj}} || $ignore)
                {
#                    chdir(&CorrectPath($StandDir.$Prj));
                    RemoveFromDependencies($Prj, \%global_deps_hash);
                    $build_is_finished{$Prj}++;
                    splice (@$build_queue, $i, 1);
                    next;
                };
            };
            $i++;
        };
        $i = 0;
    } while (!are_all_dependent($build_queue));
};


sub do_pre_job {
    my $module = shift;
    announce_module($module);
    if ($pre_custom_job && defined $modules_types{$module} && ($modules_types{$module} eq 'mod')) {
        my $module_path = CorrectPath($StandDir.$module);
        chdir $module_path;
        getcwd();
        my $cj_error_code = system ("$pre_custom_job");
        print_error("Cannot run pre job \"$pre_custom_job\"") if ($cj_error_code);
    };
};


sub do_post_custom_job {
    my $module_path = shift;
    chdir $module_path;
    getcwd();
    my $cj_error_code = system ("$post_custom_job");
    print_error("Cannot run post job \"$post_custom_job\"") if ($cj_error_code);
};

#
# Print announcement for module just started
#
sub announce_module {
    my $Prj = shift;
    $build_in_progress{$Prj}++;
    print_announce($Prj);
};

sub print_announce {
    my $Prj = shift;
    my $prj_type = '';
    $prj_type = $modules_types{$Prj} if (defined $modules_types{$Prj});
    my $text;
    if ($prj_type eq 'lnk') {
        $text = "Skipping link to $Prj\n";
        $build_is_finished{$Prj}++;
    } elsif ($prj_type eq 'img') {
#        return if (defined $module_announced{$`});
        $text = "Skipping incomplete $Prj\n";
        $build_is_finished{$Prj}++;
    } elsif ($custom_job) {
        $text = "Running custom job \"$custom_job\" in module $Prj\n";
    } else {
        $text = "Building module $Prj\n";
    };
    print $echo . "=============\n";
    print $echo . $text;
    print $echo . "=============\n";
};

sub are_all_dependent {
    my $build_queue = shift;
    my $folder = '';
    foreach my $prj (@$build_queue) {
        $folder = FindIndepPrj($projects_deps_hash{$prj});
        return '' if ($folder);
    };
    return '1';
};

#
# Procedure checks out module or its image ($prj_name/prj)
#
sub checkout_module {
    my ($prj_name, $image, $path) = @_;
    return '' if (!defined($ENV{CWS_WORK_STAMP}));
    $path = $StandDir if (!$path);
    my $cws = Cws->new();
    $cws->child($ENV{CWS_WORK_STAMP});
    $cws->master($ENV{WORK_STAMP});
    my $cvs_module = get_cvs_module($cws, $prj_name);
    print_error("Cannot get cvs_module for $prj_name") if (!$cvs_module);
    my ($master_branch_tag, $cws_branch_tag, $cws_root_tag, $master_milestone_tag) = $cws->get_tags();

    $cvs_module->verbose(1);
    $cvs_module->{MODULE} .= '/prj' if ($image);
    if ($show && ($path ne $tmp_dir)) {
        print "Checking out $prj_name...\n";
        return;
    };
    $cvs_module->checkout($path, $master_milestone_tag, '');
    # Quick hack, should not be there
    # if Heiner's Cws module had error handling
    if (!-d CorrectPath($path.'/'.$prj_name)) {
        $cvs_module->checkout($path, '', '');
        if (!-d CorrectPath($path.'/'.$prj_name)) {
            $dead_parents{$prj_name}++;
            my $warning_string = "Cannot checkout $prj_name. Check if you have to login to server or all build dependencies are consistent";
            push(@warnings, $warning_string);
            print STDERR ($warning_string);
            return;
        };
    };
    return 1 if ($image);
    copy_output_trees($prj_name, $path) if (defined $only_platform);
};

#
# Procedure unpacks output trees after checkout
#
sub copy_output_trees {
    return if (!defined $ENV{COMMON_ENV_TOOLS});
    return if (!scalar keys %platforms_to_copy);
    my $module_name = shift;
    my $src_dest = shift;
    print "copyprj $module_name\n";

    # hash, that should contain all the
    # data needed by CopyPrj module
    my %ENVHASH = ();
    my %projects_to_copy = ();
    $ENVHASH{'projects_hash'} = \%projects_to_copy;
    $ENVHASH{'prj_to_copy'} = '';
    $ENVHASH{'platforms_hash'} = \%platforms_to_copy;
    $ENVHASH{'no_otree'} = 1;
    $ENVHASH{'no_path'} = 1;
    $ENVHASH{'only_otree'} = 1;
    $ENVHASH{'only_update'} = 0;
    $ENVHASH{'last_minor'} = 0;
    $ENVHASH{'spec_src'} = 0;
    $ENVHASH{'dest'} = "$src_dest";
    $ENVHASH{'i_server'} = '';
    $ENVHASH{'current_dir'} = getcwd();
    $ENVHASH{'remote'} = '';
    # hack for SO environment
    $ENVHASH{'SRC_ROOT'} = '/so/ws/' . $ENV{WORK_STAMP} . '/src';
    $ENVHASH{'SRC_ROOT'} .= $ENV{UPDMINOREXT} if (defined $ENV{UPDMINOREXT});
    $projects_to_copy{$module_name}++;
    CopyPrj::copy_projects(\%ENVHASH);
};

#
# Procedure defines if the local directory is a
# complete module, an image or a link
# return values: lnk link
#                img incomplete (image)
#                mod complete (module)
#
sub modules_classify {
    my @modules = @_;
    foreach my $module (sort @modules) {
        if (-d $StandDir.$module) {
            $modules_types{$module} = 'mod';
            next;
        };
        if (-e $StandDir.$module.'.lnk') {
            $modules_types{$module} = 'lnk';
            next;
        };
        $modules_types{$module} = 'img';
    };
};

#
# This procedure provides consistency for cws
# and optimized build (ie in case of -with_branches, -all:prj_name
# and -since switches)
#
sub provide_consistency {
    check_dir();
    foreach $var_ref (\$build_from, \$build_all_cont, \$build_since) {
        if ($$var_ref) {
            return if (-d $StandDir.$$var_ref);
            $$var_ref .= '.lnk' and return if (-d $StandDir.$$var_ref.'.lnk');
            my $current_dir = getcwd();
            checkout_module($$var_ref, 'image');
            chdir $current_dir;
            getcwd();
            return;
        };
    };
};

#
# Retrieve CvsModule object for passed module.
# (Heiner's proprietary :)
#
sub get_cvs_module
{
    my $cws    = shift;
    my $module = shift;

    my $cvs_module = CvsModule->new();
    my ($method, $vcsid, $server, $repository) = get_cvs_root($cws, $module);

    return undef if  !($method && $vcsid && $server && $repository);

    $cvs_module->module($module);
    $cvs_module->cvs_method($method);
    $cvs_module->vcsid($vcsid);
    $cvs_module->cvs_server($server);
    $cvs_module->cvs_repository($repository);

    return $cvs_module;
};

#
# Try to get cvs coordinates via module link
#
sub get_link_cvs_root{
    my $module = shift;
    my $cvs_root_file = $StandDir.$module.'.lnk'.'/CVS/Root';
    if (!open(CVS_ROOT, $cvs_root_file)) {
        #print STDERR "Attention: cannot read $cvs_root_file!!\n";
        return '';
    };
    my @cvs_root = <CVS_ROOT>;
    close CVS_ROOT;
    $cvs_root[0] =~    s/[\r\n]+//o;
    return $cvs_root[0] if (!($cvs_root[0] =~ /\^\s*$/));
    return '';
};

#
# Find out which CVS server holds the module, returns
# the elements of CVSROOT.
# (Heiner's proprietary)
#
sub get_cvs_root
{
    my $cws    = shift;
    my $module = shift;
    my $cvsroot = get_link_cvs_root($module);
    if (!$cvsroot) {
        my $master = $cws->master();

        my $workspace_lst = get_workspace_lst();
        my $workspace_db = GenInfoParser->new();
        my $success = $workspace_db->load_list($workspace_lst);
        if ( !$success ) {
            print_error("Can't load workspace list '$workspace_lst'.", 4);
        }

        my $key = "$master/drives/o:/projects/$module/scs";
        $cvsroot = $workspace_db->get_value($key);

        if ( !$cvsroot  ) {
            print STDERR "\nWarning: No such module '$module' for '$master' in workspace database $workspace_lst. Maybe you should correct build lists.\n";
            $dead_parents{$module}++;
            return (undef, undef, undef, undef);
        }
    };
    my ($dummy1, $method, $user_at_server, $repository) = split(/:/, $cvsroot);
    my ($dummy2, $server) = split(/@/, $user_at_server);

    if ( ! ($method && $server && $repository ) ) {
        print_error("Can't determine CVS server for module '$module'.", 0);
        return (undef, undef, undef, undef);
    }

    return ($method, $vcsid, $server, $repository);
};

#
# Get the workspace list ('stand.lst'), either from 'localini'
# or, if this is not possible, from 'globalini.
# (Heiner's proprietary :)
#
sub get_workspace_lst
{
    my $home;
    if ( $^O eq 'MSWin32' ) {
        $home = $ENV{TEMP};
    }
    else {
        $home = $ENV{HOME};
    }
    my $localini = "$home/localini";
    if ( ! -f "$localini/stand.lst" ) {
        my $globalini = get_globalini();
        return "$globalini/stand.lst";
    }
    return "$localini/stand.lst";
}

#
# Procedure clears up module for incompatible build
#
sub ensure_clear_module {
    my $module = shift;
    my $module_type = $modules_types{$module};
    my $lnk_name = $module . '.lnk';
    if ($module_type eq 'mod') {
        if (-e ($StandDir.$lnk_name)) {
            print "Last checkout for $module seems to have been interrupted...\n";
            print "Checking it out again...\n";
            #rmtree("$StandDir$module", 0, 1);
            $module_type = 'lnk';
        } else {
            clear_module($module);
            return;
        };
    };
    if ($module_type eq 'lnk') {
        print "\nBreaking link $lnk_name...\n";
        return if ($show);
        checkout_module($module);
        my $action = '';
        if ( $^O eq 'MSWin32' ) {
            if(!rename("$StandDir$lnk_name", "$StandDir$module.backup.lnk")) {
                $action = 'rename';
            };
        } else {
            if(!unlink $StandDir.$lnk_name) {
                $action = 'remove';
            }
        };
        print_error("Cannot $action $StandDir$lnk_name. Please $action it manually") if ($action);
    } else {
        if ($chekoutmissing) {
            print "Checking out consistent " . $module . "...\n";
            checkout_module ($module) if (!$show);
        }
    };
};

#
# Procedure removes output tree from the module (without common trees)
#
sub clear_module {
    my $Prj = shift;
    print "Removing module's $Prj output trees...\n";
    print "\n" and return if ($show);
    opendir DIRHANDLE, $StandDir.$Prj;
    my @dir_content = readdir(DIRHANDLE);
    closedir(DIRHANDLE);
    foreach (@dir_content) {
        next if (/^\.+$/);
        my $dir = CorrectPath($StandDir.$Prj.'/'.$_);
        if ((!-d $dir.'/CVS') && is_output_tree($dir)) {
            #print "I would delete $dir\n";
            rmtree("$dir", 0, 1);
            if (defined $SIG{__WARN__} && -d $dir) {
                system("rm -rf $dir");
                if (-d $dir) {
                    print_error("Cannot delete $dir");
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
    if( defined($ENV{TMP}) ) {
       $tmp_dir = $ENV{TMP} . '/';
    } else {
       $tmp_dir = '/tmp/';
    }
    $tmp_dir .= $$ while (-d $tmp_dir);
    $tmp_dir = CorrectPath($tmp_dir);
    eval {mkpath($tmp_dir)};
    print_error("Cannot create temporary directory for checkout in $tmp_dir") if ($@);
    return $tmp_dir;
};


sub retrieve_build_list {
    my $module = shift;
    my $old_fh = select(STDOUT);

    # First try to get global depencies from solver's build.lst if such exists
    my $solver_inc_dir = "$ENV{SOLARVER}/common";
    $solver_inc_dir .= $ENV{PROEXT} if (defined $ENV{PROEXT});
    $solver_inc_dir .= '/inc';
    $solver_inc_dir .= $ENV{UPDMINOREXT} if (defined $ENV{UPDMINOREXT});
    $solver_inc_dir .= "/$module";
    $solver_inc_dir = CorrectPath($solver_inc_dir);
    $dead_parents{$module}++;
    print "Fetching dependencies for module $module from solver...";
    foreach (@possible_build_lists) {
        my $possible_build_lst = "$solver_inc_dir/$_";
        if (-e $possible_build_lst) {
            print " ok\n";
            select($old_fh);
            return $possible_build_lst;
        };
    }
    print " failed...\n";
    print "Fetching from CVS... ";
    if (!checkout_module($module, 'image', $tmp_dir)) {
        print " failed\n";
        if (!defined $dead_parents{$module}) {
            print "WARNING: Cannot figure out CWS for $module. Forgot to set CWS?\n";
        }
        select($old_fh);
        return undef;
    };
    # no need to announce this module
    print " ok\n";
    eval {
        mkpath($solver_inc_dir) if (!-e $solver_inc_dir);
    };
    print_error("Cannot create $solver_inc_dir") if (!-d $solver_inc_dir);
    my $success;
    foreach (@possible_build_lists) {
        my $tmp_build_lst = $tmp_dir . '/' . $module . '/prj/' . $_;
        $possible_build_lst = undef;
        next if (!-e $tmp_build_lst);
        $possible_build_lst = $solver_inc_dir . '/' .$_;
         my @from_stat = stat($tmp_build_lst);
        if (!File::Copy::move($tmp_build_lst, $solver_inc_dir)) {
            print_error("Cannot copy build list to $solver_inc_dir");
        };
        $success++;
        my @to_stat = stat($possible_build_lst);
        $from_stat[9]-- if $from_stat[9] % 2;
        utime ($from_stat[9], $from_stat[9], $possible_build_lst);
        last;
    };
    my $temp_checkout_path = CorrectPath($tmp_dir . '/' . $module);
    find(\&fix_permissions ,$temp_checkout_path);
    rmtree($temp_checkout_path, 0, 1);
    select($old_fh);
    return undef if (!$success);
    return $possible_build_lst;
};

sub fix_permissions {
     my $file = $File::Find::name;
     return unless -f $file;
     chmod '0664', $file;
};

#
# Removes projects which it is not necessary to build
# in incompatible build
#
sub prepare_incompatible_build {
    my ($prj, $deps_hash, @missing_modules);
    $deps_hash = shift;
    foreach (keys %incompatibles) {
        my $incomp_prj = $_;
        $incomp_prj .= '.lnk' if (!defined $$deps_hash{$_});
        delete $incompatibles{$_};
        $incompatibles{$incomp_prj} = $$deps_hash{$incomp_prj};
        delete $$deps_hash{$incomp_prj};
    }
    while ($prj = PickPrjToBuild($deps_hash)) {
        RemoveFromDependencies($prj, $deps_hash);
        RemoveFromDependencies($prj, \%incompatibles);
    };
    foreach (keys %incompatibles) {
        $$deps_hash{$_} = $incompatibles{$_};
    };
    if ($build_all_cont) {
        prepare_build_all_cont($deps_hash);
        delete $$deps_hash{$build_all_cont};
    };
    @modules_built = keys %$deps_hash;
    clear_delivered() if ($prepare);
    my $old_output_tree = '';
    foreach $prj (sort keys %$deps_hash) {
        if ($prepare) {
            ensure_clear_module($prj);
        } else {
            next if ($show);
            if ($modules_types{$prj} ne 'mod') {
                push(@missing_modules, $prj);
            } elsif (-d CorrectPath($StandDir.$prj.'/'. $ENV{INPATH})) {
                $old_output_tree++;
            };
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
    if (scalar @warnings) {
        print "WARNING(S):\n";
        print STDERR "$_\n" foreach (@warnings);
        print "\nATTENTION: If you are performing an incompatible build, please break the build with Ctrl+C and prepare the workspace with \"--prepare\" switch!\n\n";
        sleep(10);
    };
    if ($prepare) {
    print "\nPreparation finished";
        if (scalar @warnings) {
            print " with WARNINGS!!\n\n";
        } else {print " successfully\n\n";}
    }
    do_exit(0) if ($prepare);
};

#
# Removes projects which it is not necessary to build
# with -with_branches switch
#
sub prepare_build_from {
    my ($prj, $deps_hash);
    $deps_hash = shift;
    my %from_deps_hash = ();   # hash of dependencies of the -from project
    GetParentDeps($build_from, \%from_deps_hash);
    foreach $prj (keys %from_deps_hash) {
        delete $$deps_hash{$prj};
        RemoveFromDependencies($prj, $deps_hash);
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
    while ($prj = PickPrjToBuild($deps_hash)) {
        $orig_prj = '';
        $orig_prj = $` if ($prj =~ /\.lnk$/o);
        if (($border_prj ne $prj) &&
            ($border_prj ne $orig_prj)) {
            RemoveFromDependencies($prj, $deps_hash);
            next;
        } else {
            if ($build_all_cont) {
                $$deps_hash{$prj} = ();
            } else {
                RemoveFromDependencies($prj, $deps_hash);
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

sub get_incomp_projects {
    my $option = '';
    while ($option = shift @ARGV) {
        if ($option =~ /^-+/) {
            unshift(@ARGV, $option);
            return;
        } else {
            if ($option =~ /(:)/) {
                $option = $`;
                print_error("-from switch collision") if ($build_all_cont);
                $build_all_cont = $';
            };
            $incompatibles{$option}++;
        };
    };
};


sub get_platforms {
    my $platforms_ref = shift;
    if ($only_platform) {
        foreach (split(',', $only_platform)) {
            $$platforms_ref{$_}++;
        }
        $platforms_ref = \%platforms_to_copy;
    };

    my $workspace_lst = get_workspace_lst();
    my $workspace_db = GenInfoParser->new();
    my $success = $workspace_db->load_list($workspace_lst);
    if ( !$success ) {
        print_error("Can't load workspace list '$workspace_lst'.", 4);
    }
    my $access_path = $ENV{WORK_STAMP} . '/Environments';
    my @platforms_available = $workspace_db->get_keys($access_path);
    my $solver = $ENV{SOLARVERSION};
    foreach (@platforms_available) {
        my $s_path = $solver . '/' .  $_;
        $$platforms_ref{$_}++ if (-d $s_path);
    };

#    my ($iserverbin, @platforms_conf);
#    $iserverbin = "i_server -d ";
#    $iserverbin .= $ENV{SOLAR_ENV_ROOT} . '/b_server/config/stand.lst -i ';
#    my $workstamp = $ENV{WORK_STAMP};
#    @platforms_conf = `$iserverbin $workstamp/Environments -l`;
#    if ( $platforms_conf[0]  =~ /Environments/ ) {
#        shift @platforms_conf;
#    }

#    foreach (@platforms_conf) {
#        s/\s//g;
#        my $s_path = $solver . '/' .  $_;
#        $$platforms_ref{$_}++ if (-e $s_path);
#    };
#    delete $platforms_to_copy{$_} foreach (split(',', $only_platform));

    if (!scalar keys %platforms) {
        # An Auses wish - fallback to INPATH for new platforms
        if (defined $ENV{INPATH}) {
            $$platforms_ref{$ENV{INPATH}}++;
        } else {
            print_error("There is no platform found!!") ;
        };
    };
};

#
# This procedure clears solver from delivered
# by the modules to be build
#
sub clear_delivered {
    my $message = 'Clearing up delivered';
    my %backup_vars;
    my $deliver_delete_switches = '-delete';
    if (scalar keys %platforms < scalar keys %platforms_to_copy) {
        $message .= ' without common trees';
        $deliver_delete_switches .= ' -dontdeletecommon';
        $only_common = '';
    };
    print "$message\n";

    foreach my $platform (keys %platforms) {
        print "\nRemoving delivered for $platform\n";
        my %solar_vars = ();
        read_ssolar_vars($platform, \%solar_vars);
        foreach (keys %solar_vars) {
            if (!defined $backup_vars{$_}) {
                $backup_vars{$_} = $ENV{$_};
            };
            $ENV{$_} = $solar_vars{$_};
        };
        my $undeliver = "$deliver_command $deliver_delete_switches $nul";
        foreach my $module (sort @modules_built) {
            my $module_path = CorrectPath($StandDir.$module);
            print "Removing delivered from module $module\n";
            next if ($show);
            my $current_dir = getcwd();
            chdir($module_path.'.lnk') or chdir($module_path);
            if (system($undeliver)) {
                $ENV{$_} = $backup_vars{$_} foreach (keys %backup_vars);
                print_error("Cannot run: $undeliver");
            }
            chdir $current_dir;
            getcwd();
        };
    };
    $ENV{$_} = $backup_vars{$_} foreach (keys %backup_vars);
};

#
# Run setsolar for given platform and
# write all variables needed in %solar_vars hash
#
sub read_ssolar_vars {
    my ($setsolar, $entries_file, $tmp_file);
    $setsolar = $ENV{ENV_ROOT} . '/etools/setsolar.pl';
    my ($platform, $solar_vars) = @_;
    if ( $^O eq 'MSWin32' ) {
        $tmp_file = $ENV{TEMP} . "\\solar.env.$$.tmp";
    } else {
        $setsolar = '/net/jumbo2.germany/buildenv/r/etools/setsolar.pl' if ! -e $setsolar;
        $tmp_file = $ENV{HOME} . "/.solar.env.$$.tmp";
    };
    print_error('There is no setsolar found') if !-e $setsolar;
    my $pro = "";
    if ($platform =~ /\.pro$/) {
        $pro = "-pro";
        $platform = $`;
    };

    my ($verswitch, $source_root, $cwsname);
    $verswitch = "-ver $ENV{UPDMINOR}" if (defined $ENV{UPDMINOR});
    $source_root = '-sourceroot' if (defined $ENV{SOURCE_ROOT_USED});
    $cws_name = "-cwsname $ENV{CWS_WORK_STAMP}" if (defined $ENV{CWS_WORK_STAMP});

    my $param = "-$ENV{WORK_STAMP} $verswitch $source_root $cws_name $pro $platform";
    my $ss_command = "$perl $setsolar -file $tmp_file $param $nul";
    $entries_file = '/CVS/Entries';
    if (system($ss_command)) {
        unlink $tmp_file;
        print_error("Cannot run command:\n$ss_command");
    };
    get_solar_vars($solar_vars, $tmp_file);
};

#
# read variables to hash
#
sub get_solar_vars {
    my ($solar_vars, $file) = @_;
    my ($var, $value);
    open SOLARTABLE, "<$file" or die "can´t open solarfile $file";
    while(<SOLARTABLE>) {
        s/\r\n//o;
        next if(!/^\w+\s+(\w+)/o);
        next if (!defined $deliver_env{$1});
        $var = $1;
        if ( $^O eq 'MSWin32' ) {
            /$var=(\S+)$/o;
            $value = $1;
        } else {
            /\'(\S+)\'$/o;
            $value = $1;
        };
        $$solar_vars{$var} = $value;
    };
    close SOLARTABLE;
    unlink $file;
}

#
# Procedure checks out the module when we're
# in link
#
sub checkout_current_module {
    my $module_name = shift;
    my $link_name = $module_name . '.lnk';
    chdir $StandDir;
    getcwd();
    print "\nBreaking link to module $module_name";
    checkout_module($module_name);
    if (!-d $module_name && !$show) {
        print_error("Cannot checkout $module_name");
    };
    my $action;
    if ( $^O eq 'MSWin32' ) {
        $action = 'rename' if (!rename($link_name,
                                $module_name.'.backup.lnk'));
    } else {
        $action = 'remove' if (!unlink $link_name);
    };
    print_error("Cannot $action $link_name. Please $action it manually") if ($action);
    chdir $module_name;
    getcwd();
};

sub check_dir {
    my $start_dir = getcwd();
    my @dir_entries = split(/[\\\/]/, $start_dir);
    my $current_module = $dir_entries[$#dir_entries];
    $current_module = $` if ($current_module =~ /(\.lnk)$/);
    my $link_name = $ENV{SRC_ROOT}.'/'.$current_module.'.lnk';
    if ( $^O eq 'MSWin32' ) {
        $start_dir =~ s/\\/\//go;
        $link_name =~ s/\\/\//go;
        if (lc($start_dir) eq lc($link_name)) {
            checkout_current_module($current_module);
        };
    } elsif ((-l $link_name) && (chdir $link_name)) {
        if ($start_dir eq getcwd()) {
            # we're dealing with link => fallback to SRC_ROOT under UNIX
            $StandDir = $ENV{SRC_ROOT}.'/';
            checkout_current_module($current_module);
            return;
        } else {
            chdir $start_dir;
            getcwd();
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
#    close_server_socket();
    my $exit_code = shift;
    $build_finished++;
    generate_html_file(1);
    rmtree(CorrectPath($tmp_dir), 0, 1) if ($tmp_dir);
    exit($exit_code);
};

sub get_post_job {
    my $job = $deliver_command;
    $job = $custom_job if ($custom_job);
    return $job;
};

sub do_post_job {
    my $job = get_post_job();
    if ($show) {
        print $job . "\n";
        return;
    }
    my $module = shift;
    my $post_job_name = shift;
    delete $build_in_progress{$module};
    $module_path = CorrectPath($StandDir.$module);
    my $error_code = undef;
    if ($cmd_file) {
        print "cd $module_path\n";
        print "$job\n";
    } else {
        chdir $module_path;
        getcwd();
        if ($html) {
            # tested on Linux only!!
            my $log_file = $jobs_hash{$post_job_name}->{LONG_LOG_PATH};
            my $log_dir = File::Basename::dirname($log_file);
            if (!-d $log_dir) {
                eval {mkpath($log_dir)};
                print_error("Cannot create log directory $log_dir") if ($@);
            };
            $error_code = system ("$job > $log_file 2>&1");
            if (!$grab_output && -f $log_file) {
                system("cat $log_file");
            };
        } else {
            $error_code = system ("$job");
        }
    };
    $build_is_finished{$module}++ if (!defined $modules_with_errors{$module});
    if (!$error_code && $post_custom_job) {
        do_post_custom_job(CorrectPath($StandDir.$module));
    };
    return $error_code;
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
    @modules_order = sort keys %modules_with_errors;
    foreach (keys %modules_with_errors) {
        delete $build_in_progress{$_} if (defined $build_in_progress{$_});
        delete $build_is_finished{$_} if (defined $build_is_finished{$_});
        delete $build_in_progress_shown{$_} if (defined $build_in_progress_shown{$_});
    };
    $build_in_progress_shown{$_}++ foreach (keys %build_in_progress);
    push(@modules_order, $_) foreach (sort keys %build_in_progress_shown);
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
#    $force_update = 1;
#    my $html_file = CorrectPath($ENV{HOME} . '/work/' . $ENV{INPATH}. '.build.html');
#    my $write_secs = (stat($html_file))[9];
#    my @stat = stat($html_file);
#    if (defined $write_secs) {
        # Regular update no often than once in 5 sec
        return if (!$force_update && (time - $html_last_updated < 5));
#    }
    $html_last_updated = time;
    my @modules_order = sort_modules_appearance();
    my ($successes_percent, $errors_percent) = get_progress_percentage(scalar keys %html_info, scalar keys %build_is_finished, scalar keys %modules_with_errors);
    my $build_duration = get_time_line(time - $build_time);
    my $temp_html_file = File::Temp::tmpnam($ENV{TMP}),#scalar tmpnam();
    my $title;
    $title = $ENV{CWS_WORK_STAMP} . ': ' if (defined $ENV{CWS_WORK_STAMP});
    $title .= $ENV{INPATH};
    die("Cannot open $temp_html_file") if (!open(HTML, ">$temp_html_file"));
    print HTML '<html><head>';
    print HTML '<TITLE id=MainTitle>' . $title . '</TITLE>';
    print HTML '<script type="text/javascript">' . "\n";
    print HTML 'initFrames();' . "\n";
    print HTML 'var IntervalID;' . "\n";
    print HTML 'function loadFrame_0() {' . "\n";
    print HTML 'document.write("<html>");' . "\n";
    print HTML 'document.write("<head>");' . "\n";
    print HTML 'document.write("</head>");' . "\n";
    print HTML 'document.write("<body>");' . "\n";
    if ($build_finished) {
        print HTML 'document.write("<h3 align=center style=\"color:red\">Build process is finished</h3>");' . "\n";
        print HTML '        top.frames[0].clearInterval(top.frames[0].IntervalID);' . "\n";
    };
    if ($BuildAllParents) {
        print HTML 'document.write("<table valign=top cellpadding=0 hspace=0 vspace=0 cellspacing=0 border=0>");' . "\n";
        print HTML 'document.write("    <tr>");' . "\n";
        print HTML 'document.write("<td><a id=ErroneousModules href=\"javascript:top.Error(\'\', \'';
        print HTML join('<br>', sort keys %modules_with_errors);
        print HTML '\', \'\')\"); title=\"';
        print HTML scalar keys %modules_with_errors;
        print HTML ' module(s) with errors\">Total Progress:</a></td>");' . "\n";
        print HTML 'document.write("        <td height=8* width=';
        print HTML $successes_percent + $errors_percent;
        if (scalar keys %modules_with_errors) {
            print HTML '* bgcolor=red valign=top></td>");';
        } else {
            print HTML '* bgcolor=#25A528 valign=top></td>");' . "\n";
        };
        print HTML 'document.write("        <td width=';
        print HTML 100 - ($successes_percent + $errors_percent);
        print HTML '* bgcolor=lightgrey valign=top></td>");' . "\n";
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
        my ($errors_info_line, $dirs_info_line, $errors_number, $successes_percent, $errors_percent, $time) = get_html_info($_);
#<one module>
        print HTML 'document.write("<tr><td width=*>");' . "\n";

        if (defined $dirs_info_line) {
            print HTML 'document.write("<a id=';
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
            print HTML 'document.write("<em style=color:gray>' . $_ . '</em>");';
        };


        print HTML 'document.write("</td><td><table width=100% valign=top cellpadding=0 hspace=0 vspace=0 cellspacing=0 border=0><tr>");' . "\n";
        print HTML 'document.write("<td height=15* width=';

        print HTML $successes_percent + $errors_percent;
        if ($errors_number) {
            print HTML '* bgcolor=red valign=top></td>");' . "\n";
        } else {
            print HTML '* bgcolor=#25A528 valign=top></td>");' . "\n";
        };
        print HTML 'document.write("<td width=';

        print HTML 100 - ($successes_percent + $errors_percent);
        print HTML '* bgcolor=lightgrey valign=top></td>");' . "\n";
        print HTML 'document.write("</tr></table></td>");' . "\n";
        print HTML 'document.write("<td align=\"center\">', $time, '</td>");' . "\n";
        print HTML 'document.write("</tr>");' . "\n";
# </one module>
    }
    print HTML 'document.write("</table>");' . "\n";
    print HTML 'document.write("</body>");' . "\n";
    print HTML 'document.write("</html>");' . "\n";
    print HTML 'document.close();' . "\n";
    print HTML 'refreshInfoFrames();' . "\n";
    print HTML '}' . "\n";

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
    print HTML 'function ShowLog(LogFilePath) {' . "\n";
    if (defined $html_path) {
        print HTML '    top.innerFrame.frames[2].document.location.replace("file://"+LogFilePath);' . "\n";
    } else {
        print HTML '    top.innerFrame.frames[2].document.location.replace(LogFilePath);' . "\n";
    }
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
    print HTML '        top.innerFrame.frames[1].document.write("        <td width=* align=center><strong style=color:blue>Client</strong></td>");' . "\n" if ($server_mode);
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
    print HTML '                top.innerFrame.frames[1].document.write("        <td><a href=\"javascript:top.ShowLog(\'" + dir_info_array[4] + "\')\"); title=\"Show Log\">" + dir_info_array[1] + "</a></td>");' . "\n";
    print HTML '            };' . "\n";
    print HTML '            top.innerFrame.frames[1].document.write("        <td align=center>" + dir_info_array[2] + "</td>");' . "\n";
    print HTML '            top.innerFrame.frames[1].document.write("        <td align=center>" + dir_info_array[3] + "</td>");' . "\n";
    print HTML '            top.innerFrame.frames[1].document.write("        <td align=center>" + dir_info_array[5] + "</td>");' . "\n" if ($server_mode);
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
    print HTML '                    DirectoryInfos[1].innerHTML = "<a href=\"javascript:top.ShowLog(\'" + dir_info_array[4] + "\')\"); title=\"Show Log\">" + dir_info_array[1] + "</a>";' . "\n";
    print HTML '                };' . "\n";
    print HTML '                DirectoryInfos[2].innerHTML = dir_info_array[2];' . "\n";
    print HTML '                DirectoryInfos[3].innerHTML = dir_info_array[3];' . "\n";
    print HTML '                DirectoryInfos[4].innerHTML = dir_info_array[5];' . "\n" if ($server_mode);
    print HTML '            };' . "\n";
    print HTML '        };' . "\n";
    print HTML '    };' . "\n";
    print HTML '    top.innerFrame.frames[1].document.close();' . "\n";
    print HTML '};' . "\n";
    print HTML 'function Error(Module, Message1, Message2) {' . "\n";
    print HTML '    if (Module == \'\') {' . "\n";
    print HTML '        if (Message1 != \'\') {' . "\n";
    print HTML '            var erroneous_modules = Message1.split("<br>");' . "\n";
    print HTML '            var ErrorsNumber = erroneous_modules.length;' . "\n";
    print HTML '            var ErroneousModulesObj = top.innerFrame.frames[2].document.getElementById("ErroneousModules");' . "\n";
    print HTML '            var OldErrors = null;' . "\n";
    print HTML '            if (ErroneousModulesObj) {' . "\n";
    print HTML '                OldErrors = ErroneousModulesObj.getAttribute(\'errors\');' . "\n";
    print HTML '            }' . "\n";
    print HTML '            if ((OldErrors == null) || (OldErrors != ErrorsNumber)) {' . "\n";
    print HTML '                top.innerFrame.frames[2].document.write("<h3 id=ErroneousModules errors=" + erroneous_modules.length + ">Modules with errors:</h3>");' . "\n";
    print HTML '                for (i = 0; i < erroneous_modules.length; i++) {' . "\n";
    print HTML '                    top.innerFrame.frames[2].document.write("<a href=\"");' . "\n";
    print HTML '                    top.innerFrame.frames[2].document.write(top.frames[0].document.getElementById(erroneous_modules[i]).getAttribute(\'href\'));' . "\n";
    print HTML '                    top.innerFrame.frames[2].document.write("\"); title=\"");' . "\n";
    print HTML '                    top.innerFrame.frames[2].document.write(top.frames[0].document.getElementById(erroneous_modules[i]).getAttribute(\'title\'));' . "\n";
    print HTML '                    top.innerFrame.frames[2].document.write("\">" + erroneous_modules[i] + "</a>&nbsp ");' . "\n";
    print HTML '                }' . "\n";
    print HTML '                top.innerFrame.frames[2].document.close();' . "\n";
    print HTML '            }            ' . "\n";
    print HTML '        }' . "\n";
    print HTML '    } else {' . "\n";
    print HTML '        var ModuleNameObj = top.innerFrame.frames[2].document.getElementById("ModuleErrors");' . "\n";
    print HTML '        var OldErrors = null;' . "\n";
    print HTML '        var ErrorsNumber = Message1.split("<br>").length;' . "\n";
    print HTML '        if ((ModuleNameObj != null) && (Module == ModuleNameObj.getAttribute(\'name\')) ) {' . "\n";
    print HTML '            OldErrors = ModuleNameObj.getAttribute(\'errors\');' . "\n";
    print HTML '        }' . "\n";
    print HTML '        if ((OldErrors == null) || (OldErrors != ErrorsNumber)) {' . "\n";
    print HTML '            top.innerFrame.frames[2].document.write("<h3 id=ModuleErrors errors=" + ErrorsNumber + " name=\"" + Module + "\">Errors in module " + Module + ":</h3>");' . "\n";
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

    print HTML 'function setRefreshRate(RefreshRate) {' . "\n";
    print HTML '    if (isNaN(RefreshRate * 1)) {' . "\n";
    print HTML '        alert(RefreshRate + " is not a number. Ignored.");' . "\n";
    print HTML '    } else {' . "\n";
    print HTML '        top.frames[0].clearInterval(IntervalID);' . "\n";
    print HTML '        IntervalID = top.frames[0].setInterval("updateInnerFrame()", RefreshRate * 1000);' . "\n";
    print HTML '    };' . "\n";
    print HTML '};' . "\n";

    print HTML 'function initFrames() {' . "\n";
    print HTML '    var urlquery = location.href.split("?");' . "\n";
    print HTML '    if (urlquery.length == 1) {' . "\n";
    print HTML '        document.write("<html><head><TITLE id=MainTitle>unxsols4.pro</TITLE>");' . "\n";
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
    print HTML '        document.write("<FORM action=\"\?initTop\">");' . "\n";
    print HTML '        document.write("<input type=\"hidden\" name=\"initTop\" value=\"\"/>");' . "\n";
    print HTML '        document.write("<input type=\"text\" id=\"RateValue\" name=\"rate\" autocomplete=\"off\" value=\"" + UpdateRate + "\" size=\"1\"/>");' . "\n";
    print HTML '        document.write("<input type=\"button\" value=\"OK\">");' . "\n";
    print HTML '        document.write("</FORM>");' . "\n";
    print HTML '        document.write("</th></tr></table>");' . "\n";
    print HTML '        document.write("</div>");' . "\n";
    print HTML '        document.write("    </frameset>");' . "\n";
    print HTML '        document.write("</body></html>");' . "\n";
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
    print HTML '        document.write(\'?initFrame2"/>\');' . "\n";
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

    if(-e $temp_html_file) {
        rename($temp_html_file, $html_file) or system("mv", $temp_html_file, $html_file);
        if (-e $temp_html_file) {
            system("rm -rf $temp_html_file") if (!unlink $temp_html_file);
        };
    };
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
        $dirs_info_line .= 'file:///' if (($ENV{GUI} eq 'WNT') && (!$cygwin));
        $dirs_info_line .= $log_path_string;
    };
    $dirs_info_line .= '<br>';
    $dirs_info_line .= $jobs_hash{$job}->{CLIENT} . '<br>' if ($server_mode);
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
#        $dirs_info_line .= 'No information available yet';
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
#    if (defined $full_info) {
    my $time_line = get_time_line($$module_info_hash{BUILD_TIME});
        my ($successes_percent, $errors_percent) = get_progress_percentage($dirs_number, $successful_number, $errorful_number);
        return($errors_info_line, $dirs_info_line, $errorful_number, $successes_percent, $errors_percent, $time_line);
#    } else {
#        return($errors_info_line, $dirs_info_line, $errorful_number);
#    };
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
sub html_store_dmake_dir_info {
    return if (!$html);
    my ($deps_hash, $build_dir, $error_code) = @_;
    my $force_update = 0;
    if ($build_dir =~ /(\s)/o) {
        $error_code = 0 if (!defined $error_code);
    } else {
        $force_update++ if (!children_number());
    }
    my $module = $module_by_hash{$deps_hash};
    my $module_info_hash = $html_info{$module};
#    $$module_info_hash{BUILD_TIME} += $jobs_hash{$build_dir}->{FINISH_TIME} - $jobs_hash{$build_dir}->{START_TIME};
    if (defined $error_code) {
        $jobs_hash{$build_dir}->{FINISH_TIME} = time();
        $$module_info_hash{BUILD_TIME} += $jobs_hash{$build_dir}->{FINISH_TIME} - $jobs_hash{$build_dir}->{START_TIME};
    }
    my $dmake_array;
    if (defined $error_code) {
        if ($error_code) {
            $jobs_hash{$build_dir}->{STATUS} = 'error';
            $dmake_array = $$module_info_hash{ERRORFUL};
            $build_dir =~ s/\\/\//g;
            $modules_with_errors{$module}++;
        } else {
            $jobs_hash{$build_dir}->{STATUS} = 'success';
            $dmake_array = $$module_info_hash{SUCCESSFUL};
        };
        push (@$dmake_array, $build_dir);
    };
#    generate_html_file($force_update);
};

#sub close_server_socket {
#    # Actually, the "or warn..." part is for debugging,
#    # should be removed for release
#    if (defined $new_socket_obj) {
#       print $new_socket_obj 'No job';
#        close($new_socket_obj) or warn $! ? "Error closing connection: $!"
#            : "Exit status $? from closing connection";
#    };
#    if (defined $socket_obj) {
#        close($socket_obj) or warn $! ? "Error closing server socket: $!"
#            : "Exit status $? from closing server socket";
#    };
#};

#sub stop_server {
#    close_server_socket();
#    print "Server terminated\n";
#    exit($?);
#};

sub start_server_on_port {
    my $port = shift;
    if ($ENV{GUI} eq 'WNT') {
        $socket_obj = new IO::Socket::INET (#LocalAddr => hostname(),
                                  LocalPort => $port,
                                  Proto     => 'tcp',
                                  Listen    => 100); # 100 clients can be on queue, I think it is enough
    } else {
        $socket_obj = new IO::Socket::INET (#LocalAddr => hostname(),
                                  LocalPort => $port,
                                  Proto     => 'tcp',
                                  ReuseAddr     => 1,
                                  Listen    => 100); # 100 clients can be on queue, I think it is enough
    };
    return('Cannot create socket object') if (!defined $socket_obj);
    my $timeout = $socket_obj->timeout($client_timeout);
    $socket_obj->autoflush(1);
    print "SERVER started on port $port\n";
    return 0;
};

sub accept_connection {
    my $new_socket_obj = undef;
    do {
        $new_socket_obj = $socket_obj->accept();
        if (!$new_socket_obj) {
            print "Timeout on incoming connection\n";
            check_client_jobs();
        };
    } while (!$new_socket_obj);
    return $new_socket_obj;
};

sub check_client_jobs {
    foreach (keys %clients_times) {
        if (time - $clients_times{$_} > $client_timeout) {
            print "Client's $_ Job: \"$clients_jobs{$_}\" apparently got lost...\n";
            print "Scheduling for rebuild...\n";
            print "You might need to check the $_\n";
            $lost_client_jobs{$clients_jobs{$_}}++;
            delete $processes_hash{$_};
            delete $clients_jobs{$_};
            delete $clients_times{$_};
#        } else {
#            print time - $clients_times{$_} . "\n";
        };
    };
};

sub run_server {
    my @build_queue = ();        # array, containing queue of projects
                                # to build
    # use port 7890 as default
    my $default_port = 7890;
    if ($ports_string) {
        @server_ports = split( /:/, $ports_string);
    } else {
        @server_ports = ($default_port .. $default_port + 4);
    };
    my $error = 0;
    if (scalar @server_ports) {
        foreach (@server_ports) {
            $error = start_server_on_port($_);
            if ($error) {
                print STDERR "port $_: $error\n";
            } else {
#                $SIG{KILL} = \&stop_server;
#                $SIG{INT} = \&stop_server;
#                $SIG{TERM} = \&stop_server;
#                $SIG{QUIT} = \&stop_server;
                last;
            };
        };
    };
    print_error('It is impossible to start server on port(s): ' . "@server_ports\n") if ($error);

    my $client_addr;
    my $job_string_base = get_job_string_base();
    my $new_socket_obj;
     while ($new_socket_obj = accept_connection()) {
        check_client_jobs();
        # find out who connected
        my $client_ipnum = $new_socket_obj->peerhost();
        my $client_host = gethostbyaddr(inet_aton($client_ipnum), AF_INET);
        # print who is connected
#       print "got a connection from: $client_host", "[$client_ipnum]\n";
        # send them a message, close connection
        my $client_message = <$new_socket_obj>;
#        print $client_message;
        chomp $client_message;
        my @client_data = split(/ /, $client_message);
        my %client_hash = ();
        foreach (@client_data) {
            /(=)/;
            $client_hash{$`} = $';
        }
        my $pid = $client_hash{pid} . '@' . $client_host;
        if (defined $client_hash{platform}) {
            if ($client_hash{platform} ne $ENV{OUTPATH} || (defined $client_hash{osname} && ($^O ne $client_hash{osname}))) {
                print $new_socket_obj "Wrong platform";
                close($new_socket_obj);
                next;
            };
        } else {
#            handle_dead_children(0);
            if ($client_hash{result} eq "0") {
#                print "$clients_jobs{$pid} succedded on $pid\n";
            } else {
                print "Error $client_hash{result}\n";
                if (store_error($pid, $client_hash{result})) {
                    print $new_socket_obj $job_string_base . $clients_jobs{$pid};
                    close($new_socket_obj);
                    $clients_times{$pid} = time;
                    next;
                };
            };
            delete $clients_times{$pid};
            clear_from_child($pid);
            delete $clients_jobs{$pid};
            print 'Running processes: ', children_number(), "\n";
            # Actually, next 3 strings are only for even distribution
            # of clients if there are more than one build server running
            print $new_socket_obj 'No job';
            close($new_socket_obj);
            next;
        };
        my $job_string;
        my @lost_jobs = keys %lost_client_jobs;
        if (scalar @lost_jobs) {
            $job_string = $lost_jobs[0];
            delete $lost_client_jobs{$lost_jobs[0]};
        } else {
#            $job_string = get_job_string(\@build_queue, $pid);
            $job_string = get_job_string(\@build_queue);
        };
        if ($job_string) {
            my $job_dir = $job_jobdir{$job_string};
            $processes_hash{$pid} = $job_dir;
            $jobs_hash{$job_dir}->{CLIENT} = $pid;
            print "$pid got $job_dir\n";
            print $new_socket_obj $job_string_base . $job_string;
            $clients_jobs{$pid} = $job_string;
            $clients_times{$pid} = time;
            $children_running = children_number();
            print 'Running processes: ', $children_running, "\n";
            $maximal_processes = $children_running if ($children_running > $maximal_processes);
        } else {
            print $new_socket_obj 'No job';
        };
        close($new_socket_obj);
    };
};

#
# Procedure returns the part of the job string that is similar for all clients
#
sub get_job_string_base {
    if ($setenv_string) {
        return "setenv_string=$setenv_string ";
    };
    my $job_string_base = "server_pid=$$ setsolar_cmd=$ENV{SETSOLAR_CMD} ";
    $job_string_base .= "source_root=$ENV{SOURCE_ROOT} " if (defined $ENV{SOURCE_ROOT});
    $job_string_base .= "updater=$ENV{UPDATER} " if (defined $ENV{UPDATER});
    return $job_string_base;
};

sub get_job_string {
    my $build_queue = shift;
    my $job = $dmake;
    my ($job_dir, $dependencies_hash);
    if ($BuildAllParents) {
        fill_modules_queue($build_queue);
        do {
            ($job_dir, $dependencies_hash) = pick_jobdir($build_queue);
            return '' if (!$job_dir);
            $jobs_hash{$job_dir}->{START_TIME} = time();
            $jobs_hash{$job_dir}->{STATUS} = 'building';
            if ($job_dir =~ /(\s)/o) {
                if ($' eq $pre_job) {
                    do_pre_job($`);
                    html_store_dmake_dir_info($dependencies_hash, $job_dir, undef);
                    RemoveFromDependencies($job_dir, $dependencies_hash);
                    $job_dir = '';
                };
            };
        } while (!$job_dir);
    } else {
        $dependencies_hash = \%LocalDepsHash;
        do {
            $job_dir = PickPrjToBuild(\%LocalDepsHash);
            if (!$job_dir && !children_number()) {
                cancel_build() if (scalar keys %broken_build);
                mp_success_exit();
            };
            return '' if (!$job_dir);
            $jobs_hash{$job_dir}->{START_TIME} = time();
            $jobs_hash{$job_dir}->{STATUS} = 'building';
            if ($job_dir =~ /(\s)/o) {
                if ($' eq $pre_job) {
                    do_pre_job($`);
                    html_store_dmake_dir_info($dependencies_hash, $job_dir, undef);
                    RemoveFromDependencies($job_dir, $dependencies_hash);
                    $job_dir = '';
                }
            };
        } while (!$job_dir);
    };
    $running_children{$dependencies_hash}++;
#    $processes_hash{$pid} = $job_dir;
    $folders_hashes{$job_dir} = $dependencies_hash;
    my $log_file = $jobs_hash{$job_dir}->{LONG_LOG_PATH};
#    $jobs_hash{$job_dir}->{CLIENT} = $pid;
    my $full_job_dir = $job_dir;
    if (($job_dir =~ /(\s)/o) && ($' eq $post_job)) {
        $job = get_post_job();
        $full_job_dir = CorrectPath($StandDir.$`);
    }
    my $log_dir = File::Basename::dirname($log_file);
    if (!-d $log_dir) {
        chdir $full_job_dir;
        getcwd();
        system("$perl $mkout");
    };
    my $job_string = "job_dir=$full_job_dir job=$job log=$log_file";
    $job_jobdir{$job_string} = $job_dir;
    return $job_string;
};

sub pick_jobdir {
    my $build_queue = shift;
    my $i = 0;
    foreach (@$build_queue) {
        $Prj = $$build_queue[$i];
        my $prj_deps_hash = $projects_deps_hash{$Prj};
        if (defined $broken_modules_hashes{$prj_deps_hash} && !$ignore) {
            push (@broken_modules_names, $Prj);
            splice (@$build_queue, $i, 1);
            next;
        };
        $only_dependent = 0;
        $no_projects = 0;
        $running_children{$prj_deps_hash} = 0 if (!defined $running_children{$prj_deps_hash});
        $child_nick = PickPrjToBuild($prj_deps_hash);
        if ($child_nick) {
            return ($child_nick, $prj_deps_hash);
        }
        if ($no_projects && !$running_children{$prj_deps_hash}) {
            if (!defined $broken_modules_hashes{$prj_deps_hash} || $ignore)
            {
                RemoveFromDependencies($Prj, \%global_deps_hash);
                $build_is_finished{$Prj}++;
                splice (@$build_queue, $i, 1);
                next;
            };
        };
        $i++;
    };
};

sub fill_modules_queue {
    my $build_queue = shift;
    my $Prj;
    while ($Prj = PickPrjToBuild(\%global_deps_hash)) {
        my $module_type = $modules_types{$Prj};
        push @$build_queue, $Prj;
        $projects_deps_hash{$Prj} = {};
        get_deps_hash($Prj, $projects_deps_hash{$Prj});
        my $info_hash = $html_info{$Prj};
        $$info_hash{DIRS} = check_deps_hash($projects_deps_hash{$Prj}, $Prj);
        $module_by_hash{$projects_deps_hash{$Prj}} = $Prj;
    };
    if (!$Prj && !children_number() && (!scalar @$build_queue)) {
        cancel_build() if (scalar keys %broken_build);
        mp_success_exit();
    };
};
