:
eval 'exec perl -wS $0 ${1+"$@"}'
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
# deliver.pl - copy from module output tree to solver
#

use Cwd;
use File::Basename;
use File::Copy;
use File::DosGlob 'glob';
use File::Path;
use File::Spec;

#### script id #####

( $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

#### globals ####

### valid actions ###
# if you add a action 'foo', than add 'foo' to this list and
# implement 'do_foo()' in the implemented actions area
@action_list        =   (           # valid actions
                        'copy',
                        'dos',
                        'addincpath',
                        'linklib',
                        'mkdir',
                        'symlink',
                        'touch'
                        );

# copy filter: files matching these patterns won't be copied by
# the copy action
@copy_filter_patterns = (
                        );

$is_debug           = 0;

$error              = 0;
$module             = 0;            # module name
$repository         = 0;            # parent directory of this module
$base_dir           = 0;            # path to module base directory
$dlst_file          = 0;            # path to d.lst
$ilst_ext           = 'ilst';       # extension of image lists
$umask              = 22;           # default file/directory creation mask
$dest               = 0;            # optional destination path
$common_dest        = 0;            # common tree on solver

@action_data        = ();           # LoL with all action data
@macros             = ();           # d.lst macros
@addincpath_list    = ();           # files which have to be filtered through addincpath
@dirlist            = ();           # List of 'mkdir' targets
@zip_list           = ();           # files which have to be zipped
@common_zip_list    = ();           # common files which have to be zipped
@log_list           = ();           # LoL for logging all copy and link actions
@common_log_list    = ();           # LoL for logging all copy and link actions in common_dest
$logfiledate        = 0;            # Make log file as old as newest delivered file
$commonlogfiledate  = 0;            # Make log file as old as newest delivered file

$files_copied       = 0;            # statistics
$files_unchanged    = 0;            # statistics

$opt_force          = 0;            # option force copy
$opt_check          = 0;            # do actually execute any action
$opt_zip            = 0;            # create an additional zip file
$opt_silent         = 0;            # be silent, only report errors
$opt_verbose        = 0;            # be verbose (former default behaviour)
$opt_log            = 1;            # create an additional log file
$opt_link           = 0;            # hard link files into the solver to save disk space
$opt_deloutput      = 0;            # delete the output tree for the project once successfully delivered
$opt_checkdlst      = 0;
$delete_common      = 1;            # for "-delete": if defined delete files from common tree also

if ($^O ne 'cygwin') {              # iz59477 - cygwin needes a dot "." at the end of filenames to disable
    $maybedot     = '';             # some .exe transformation magic.
} else {
    my $cygvernum = `uname -r`;
    my @cygvernum = split( /\./, $cygvernum);
    $cygvernum = shift @cygvernum;
    $cygvernum .= shift @cygvernum;
    if ( $cygvernum < 17 ) {
        $maybedot     = '.';
    } else {
        $maybedot     = '';               # no longer works with cygwin 1.7. other magic below.
    }
}

($gui       = lc($ENV{GUI}))        || die "Can't determine 'GUI'. Please set environment.\n";
$tempcounter        = 0;

# zip is default for RE master builds
$opt_zip = 1 if ( defined($ENV{DELIVER_TO_ZIP}) && uc($ENV{DELIVER_TO_ZIP}) eq 'TRUE');

$has_symlinks       = 0;            # system supports symlinks

for (@action_list) {
    $action_hash{$_}++;
}

# trap normal signals (HUP, INT, PIPE, TERM)
# for clean up on unexpected termination
use sigtrap 'handler' => \&cleanup_and_die, 'normal-signals';

#### main ####

parse_options();

init_globals();
push_default_actions();
parse_dlst();
check_dlst() if $opt_checkdlst;
walk_action_data();
walk_addincpath_list();
write_log() if $opt_log;
zip_files() if $opt_zip;
cleanup() if $opt_delete;
delete_output() if $opt_deloutput;
print_stats();

exit($error);

#### implemented actions #####

sub do_copy
{
    # We need to copy two times:
    # from the platform dependent output tree
    # and from the common output tree
    my ($dependent, $common, $from, $to, $file_list);
    my $line = shift;
    my $touch = 0;

    $dependent = expand_macros($line);
    ($from, $to) = split(' ', $dependent);
    print "copy dependent: from: $from, to: $to\n" if $is_debug;
    glob_and_copy($from, $to, $touch);
}

sub do_dos
{
    my $line = shift;

    my $command = expand_macros($line);
    if ( $opt_check ) {
        print "DOS: $command\n";
    }
    else {
        # HACK: remove MACOSX stuff which is wrongly labled with dos
        # better: fix broken d.lst
        return if ( $command =~ /MACOSX/ );
        $command =~ s#/#\\#g if $^O eq 'MSWin32';
        system($command);
    }
}

sub do_addincpath
{
    # just collect all addincpath files, actual filtering is done later
    my $line = shift;
    my ($from, $to);
    my @globbed_files = ();

    $line = expand_macros($line);
    ($from, $to) = split(' ', $line);

    push( @addincpath_list, @{glob_line($from, $to)});
}

sub do_linklib
{
    my ($lib_base, $lib_major,$from_dir, $to_dir);
    my $lib = shift;
    my @globbed_files = ();
    my %globbed_hash = ();

    print "linklib: $lib\n" if $is_debug;
    print "has symlinks\n" if ( $has_symlinks && $is_debug );

    return unless $has_symlinks;

    $from_dir = expand_macros('../%__SRC%/lib');
    $to_dir = expand_macros('%_DEST%/lib');

    @globbed_files = glob("$from_dir/$lib");

    if ( $#globbed_files == -1 ) {
       return;
    }

    foreach $lib (@globbed_files) {
        $lib = basename($lib);
        if ( $lib =~ /^(lib\S+(\.so|\.dylib))\.(\d+)\.(\d+)(\.(\d+))?$/
             || $lib =~ /^(lib\S+(\.so|\.dylib))\.(\d+)$/ )
        {
           push(@{$globbed_hash{$1}}, $lib);
        }
        else {
            print_warning("invalid library name: $lib");
        }
    }

    foreach $lib_base ( sort keys %globbed_hash ) {
        $lib = get_latest_patchlevel(@{$globbed_hash{$lib_base}});

        if ( $lib =~ /^(lib\S+(\.so|\.dylib))\.(\d+)\.(\d+)(\.(\d+))?$/ )
        {
            $lib_major = "$lib_base.$3";
            $long = 1;
        }
        else
        {
            $long = 0;
        }

        if ( $opt_check ) {
            if ( $opt_delete ) {
                print "REMOVE: $to_dir/$lib_major\n" if $long;
                print "REMOVE: $to_dir/$lib_base\n";
            }
            else {
                print "LINKLIB: $to_dir/$lib -> $to_dir/$lib_major\n" if $long;
                print "LINKLIB: $to_dir/$lib -> $to_dir/$lib_base\n";
            }
        }
        else {
            if ( $opt_delete ) {
                print "REMOVE: $to_dir/$lib_major\n" if ($long && $opt_verbose);
                print "REMOVE: $to_dir/$lib_base\n" if $opt_verbose;
                unlink "$to_dir/$lib_major" if $long;
                unlink "$to_dir/$lib_base";
                if ( $opt_zip ) {
                    push_on_ziplist("$to_dir/$lib_major") if $long;
                    push_on_ziplist("$to_dir/$lib_base");
                }
                return;
            }
            my $symlib;
            my @symlibs;
            if ($long)
            {
                @symlibs = ("$to_dir/$lib_major", "$to_dir/$lib_base");
            }
            else
            {
                @symlibs = ("$to_dir/$lib_base");
            }
            # remove old symlinks
            unlink(@symlibs);
            foreach $symlib (@symlibs) {
                print "LINKLIB: $lib -> $symlib\n" if $opt_verbose;
                if ( !symlink("$lib", "$symlib") ) {
                    print_error("can't symlink $lib -> $symlib: $!",0);
                }
                else {
                    push_on_ziplist($symlib) if $opt_zip;
                    push_on_loglist("LINK", "$lib", "$symlib") if $opt_log;
                }
            }
        }
    }
}

sub do_mkdir
{
    my $path = expand_macros(shift);
    # strip whitespaces from path name
    $path =~ s/\s$//;
    if (( ! $opt_delete ) && ( ! -d $path )) {
        if ( $opt_check ) {
            print "MKDIR: $path\n";
        } else {
            mkpath($path, 0, 0777-$umask);
            if ( ! -d $path ) {
                print_error("mkdir: could not create directory '$path'", 0);
            }
        }
    }
}

sub do_symlink
{
    my $line = shift;

    $line = expand_macros($line);
    ($from, $to) = split(' ',$line);
    my $fullfrom = $from;
    if ( dirname($from) eq dirname($to) ) {
        $from = basename($from);
    }
    elsif ( dirname($from) eq '.' ) {
        # nothing to do
    }
    else {
        print_error("symlink: link must be in the same directory as file",0);
        return 0;
    }

    print "symlink: $from, to: $to\n" if $is_debug;

    return unless $has_symlinks;

    if ( $opt_check ) {
        if ( $opt_delete ) {
            print "REMOVE: $to\n";
        }
        else {
            print "SYMLINK $from -> $to\n";
        }
    }
    else {
        return unless -e $fullfrom;
        print "REMOVE: $to\n" if $opt_verbose;
        unlink $to;
        if ( $opt_delete ) {
            push_on_ziplist($to) if $opt_zip;
            return;
        }
        print "SYMLIB: $from -> $to\n" if $opt_verbose;
        if ( !symlink("$from", "$to") ) {
            print_error("can't symlink $from -> $to: $!",0);
        }
        else {
            push_on_ziplist($to) if $opt_zip;
            push_on_loglist("LINK", "$from", "$to") if $opt_log;
        }
    }
}

sub do_touch
{
    my ($from, $to);
    my $line = shift;
    my $touch = 1;

    $line = expand_macros($line);
    ($from, $to) = split(' ', $line);
    print "touch: $from, to: $to\n" if $is_debug;
    glob_and_copy($from, $to, $touch);
}

#### subroutines #####

sub parse_options
{
    my $arg;
    my $dontdeletecommon = 0;
    $opt_silent = 1 if ( defined $ENV{VERBOSE} && $ENV{VERBOSE} eq 'FALSE');
    $opt_verbose = 1 if ( defined $ENV{VERBOSE} && $ENV{VERBOSE} eq 'TRUE');
    while ( $arg = shift @ARGV ) {
        $arg =~ /^-force$/      and $opt_force  = 1  and next;
        $arg =~ /^-check$/      and $opt_check  = 1  and $opt_verbose = 1 and next;
        $arg =~ /^-quiet$/      and $opt_silent = 1  and next;
        $arg =~ /^-verbose$/    and $opt_verbose = 1 and next;
        $arg =~ /^-zip$/        and $opt_zip    = 1  and next;
        $arg =~ /^-delete$/     and $opt_delete = 1  and next;
        $arg =~ /^-dontdeletecommon$/ and $dontdeletecommon = 1 and next;
        $arg =~ /^-help$/       and $opt_help   = 1  and $arg = '';
        $arg =~ /^-link$/       and $ENV{GUI} ne 'WNT' and $opt_link = 1 and next;
        $arg =~ /^-deloutput$/  and $opt_deloutput = 1 and next;
        $arg =~ /^-debug$/      and $is_debug   = 1  and next;
        $arg =~ /^-checkdlst$/  and $opt_checkdlst = 1 and next;
        print_error("invalid option $arg") if ( $arg =~ /^-/ );
        if ( $arg =~ /^-/ || $opt_help || $#ARGV > -1 ) {
            usage(1);
        }
        $dest = $arg;
    }
    # $dest and $opt_zip or $opt_delete are mutually exclusive
    if ( $dest and ($opt_zip || $opt_delete) ) {
        usage(1);
    }
    # $opt_silent and $opt_check or $opt_verbose are mutually exclusive
    if ( ($opt_check or $opt_verbose) and $opt_silent ) {
        print STDERR "Error on command line: options '-check' and '-quiet' are mutually exclusive.\n";
        usage(1);
    }
    if ($dontdeletecommon) {
        if (!$opt_delete) {
            usage(1);
        }
        $delete_common = 0;
    };
    # $opt_delete implies $opt_force
    $opt_force = 1 if $opt_delete;
}

sub init_globals
{
    ($module, $repository, $base_dir, $dlst_file) =  get_base();

    print "Module=$module, Base_Dir=$base_dir, d.lst=$dlst_file\n" if $is_debug;

    $umask = umask();
    if ( !defined($umask) ) {
        $umask = 22;
    }

    my $common_outdir = $ENV{'COMMON_OUTDIR'};
    my $inpath        = $ENV{'INPATH'};
    my $solarversion  = $ENV{'SOLARVERSION'};

    # do we have a valid environment?
    if ( !defined($inpath) ) {
            print_error("no environment", 0);
            exit(3);
    }

    $common_outdir = $inpath;
    $dest = "$solarversion/$inpath" if ( !$dest );
    $common_dest = $dest;
    $dest =~ s#\\#/#g;
    $common_dest =~ s#\\#/#g;

    # the following macros are obsolete, will be flagged as error
    # %__WORKSTAMP%
    # %GUIBASE%
    # %SDK%
    # %SOLARVER%
    # %__OFFENV%
    # %DLLSUFFIX%'
    # %OUTPATH%
    # %L10N_FRAMEWORK%
    # %UPD%

    # valid macros
    @macros = (
                [ '%__PRJROOT%',        $base_dir       ],
                [ '%__SRC%',            $inpath         ],
                [ '%_DEST%',            $dest           ],
                [ '%COMMON_OUTDIR%',    $common_outdir  ],
                [ '%COMMON_DEST%',      $common_dest    ],
                [ '%GUI%',              $gui            ]
              );

    # find out if the *HOST* system supports symlinks. They all do except Windows
    $has_symlinks = $ENV{GUI} ne 'WNT';
}

sub get_base
{
    # a module base dir contains a subdir 'prj'
    # which in turn contains a file 'd.lst'
    my (@field, $repo, $base, $dlst);
    my $path = getcwd();

    @field = split(/\//, $path);

    while ( $#field != -1 ) {
        $base = join('/', @field);
        $dlst = $base . '/prj/d.lst';
        last if -e $dlst;
        pop @field;
    }

    if ( $#field == -1 ) {
        print_error("can't find d.lst");
        exit(2);
    }
    else {
        if ( defined $field[-2] ) {
            $repo = $field[-2];
        } else {
            print_error("Internal error: cannot determine module's parent directory");
        }
        return ($field[-1], $repo, $base, $dlst);
    }
}

sub parse_dlst
{
    my $line_cnt = 0;
    open(DLST, "<$dlst_file") or die "can't open d.lst";
    while(<DLST>) {
        $line_cnt++;
        tr/\r\n//d;
        next if /^#/;
        next if /^\s*$/;
        if (!$delete_common && /%COMMON_DEST%/) {
            # Just ignore all lines with %COMMON_DEST%
            next;
        };
        if ( /^\s*(\w+?):\s+(.*)$/ ) {
            if ( !exists $action_hash{$1} ) {
                print_error("unknown action: \'$1\'", $line_cnt);
                exit(4);
            }
            push(@action_data, [$1, $2]);
        }
        else {
            if ( /^\s*%(COMMON)?_DEST%\\/ ) {
                # only copy from source dir to solver, not from solver to solver
                print_warning("illegal copy action, ignored: \'$_\'", $line_cnt);
                next;
            }
            push(@action_data, ['copy', $_]);
            # for each resource file (.res) copy its image list (.ilst)
            if ( /\.res\s/ ) {
                my $imagelist = $_;
                $imagelist =~ s/\.res/\.$ilst_ext/g;
                $imagelist =~ s/DEST%\\bin\\/DEST%\\res\\img\\/;
                push(@action_data, ['copy', $imagelist]);
            }
        }
        # call expand_macros()just to find any undefined macros early
        # real expansion is done later
        expand_macros($_, $line_cnt);
    }
    close(DLST);
}

sub expand_macros
{
    # expand all macros and change backslashes to slashes
    my $line        = shift;
    my $line_cnt    = shift;
    my $i;

    for ($i=0; $i<=$#macros; $i++)  {
        $line =~ s/$macros[$i][0]/$macros[$i][1]/gi
    }
    if ( $line =~ /(%\w+%)/ ) {
        if ( $1 ne '%OS%' ) {   # %OS% looks like a macro but is not ...
            print_error("unknown/obsolete macro: \'$1\'", $line_cnt);
        }
    }
    $line =~ s#\\#/#g;
    return $line;
}

sub walk_action_data
{
    # all actions have to be excuted relative to the prj directory
    chdir("$base_dir/prj");
    # dispatch depending on action type
    for (my $i=0; $i <= $#action_data; $i++) {
            &{"do_".$action_data[$i][0]}($action_data[$i][1]);
            if ( $action_data[$i][0] eq 'mkdir' ) {
                # fill array with (possibly) created directories in
                # revers order for removal in 'cleanup'
                unshift @dirlist, $action_data[$i][1];
            }
    }
}

sub glob_line
{
    my $from = shift;
    my $to = shift;
    my $to_dir = shift;
    my $replace = 0;
    my @globbed_files = ();

    if ( ! ( $from && $to ) ) {
        print_warning("Error in d.lst? source: '$from' destination: '$to'");
        return \@globbed_files;
    }

    if ( $to =~ /[\*\?\[\]]/ ) {
        my $to_fname;
        ($to_fname, $to_dir) = fileparse($to);
        $replace = 1;
    }

    if ( $from =~ /[\*\?\[\]]/ ) {
        # globbing necessary, no renaming possible
        my $file;
        my @file_list = glob($from);

        foreach $file ( @file_list ) {
            next if ( -d $file); # we only copy files, not directories
            my ($fname, $dir) = fileparse($file);
            my $copy = ($replace) ? $to_dir . $fname : $to . '/' . $fname;
            push(@globbed_files, [$file, $copy]);
        }
    }
    else {
        # no globbing but renaming possible
        # #i89066#
        if (-d $to && -f $from) {
            my $filename = File::Basename::basename($from);
            $to .= '/' if ($to !~ /[\\|\/]$/);
            $to .= $filename;
        };
        push(@globbed_files, [$from, $to]);
    }
    if ( $opt_checkdlst ) {
        my $outtree = expand_macros("%__SRC%");
        my $commonouttree = expand_macros("%COMMON_OUTDIR%");
        if (( $from !~ /\Q$outtree\E/ ) && ( $from !~ /\Q$commonouttree\E/ )) {
            print_warning("'$from' does not match any file") if ( $#globbed_files == -1 );
        }
    }
    return \@globbed_files;
}


sub glob_and_copy
{
    my $from = shift;
    my $to = shift;
    my $touch = shift;

    my @copy_files = @{glob_line($from, $to)};

    for (my $i = 0; $i <= $#copy_files; $i++) {
        next if filter_out($copy_files[$i][0]); # apply copy filter
        copy_if_newer($copy_files[$i][0], $copy_files[$i][1], $touch)
                    ? $files_copied++ : $files_unchanged++;
    }
}

sub is_jar {
    my $file_name = shift;

    if (-f $file_name && (( `file $file_name` ) =~ /Zip archive/o)) {
        return '1' if ($file_name =~ /\.jar\.*/o);
    };
    return '';
}

sub execute_system {
    my $command = shift;
    if (system($command)) {
        print_error("Failed to execute $command");
        exit($?);
    };
};

sub copy_if_newer
{
    # return 0 if file is unchanged ( for whatever reason )
    # return 1 if file has been copied
    my $from = shift;
    my $to = shift;
    my $touch = shift;
    my $from_stat_ref;
    my $rc = 0;

    print "testing $from, $to\n" if $is_debug;
    push_on_ziplist($to) if $opt_zip;
    push_on_loglist("COPY", "$from", "$to") if $opt_log;
    return 0 unless ($from_stat_ref = is_newer($from, $to, $touch));

    if ( $opt_delete ) {
        print "REMOVE: $to\n" if $opt_verbose;
        $rc = unlink($to) unless $opt_check;
        return 1 if $opt_check;
        return $rc;
    }

    if( !$opt_check && $opt_link ) {
        # hard link if possible
        if( link($from, $to) ){
            print "LINK: $from -> $to\n" if $opt_verbose;
            return 1;
        }
    }

    if( $touch ) {
       print "TOUCH: $from -> $to\n" if $opt_verbose;
    }
    else {
       print "COPY: $from -> $to\n" if $opt_verbose;
    }

    return 1 if( $opt_check );

    #
    # copy to temporary file first and rename later
    # to minimize the possibility for race conditions
    local $temp_file = sprintf('%s.%d-%d', $to, $$, time());
    $rc = '';
    $rc = copy($from, $temp_file);
    if ( $rc) {
        if ( is_newer($temp_file, $from, 0) ) {
            $rc = utime($$from_stat_ref[9], $$from_stat_ref[9], $temp_file);
            if ( !$rc ) {
                print_warning("can't update temporary file modification time '$temp_file': $!\n
                               Check file permissions of '$from'.",0);
            }
        }
        fix_file_permissions($$from_stat_ref[2], $temp_file);
        # Ugly hack: on windows file locking(?) sometimes prevents renaming.
        # Until we've found and fixed the real reason try it repeatedly :-(
        my $try = 0;
        my $maxtries = 1;
        $maxtries = 5 if ( $^O eq 'MSWin32' );
        my $success = 0;
        while ( $try < $maxtries && ! $success ) {
            sleep $try;
            $try ++;
            $success = rename($temp_file, $to);
            if ( $^O eq 'cygwin' && $to =~ /\.bin$/) {
                # hack to survive automatically added .exe for executables renamed to
                # *.bin - will break if there is intentionally a .bin _and_ .bin.exe file.
                $success = rename( "$to.exe", $to ) if -f "$to.exe";
            }
        }
        if ( $success ) {
            # handle special packaging of *.dylib files for Mac OS X
            if ( $ENV{OS} eq 'MACOSX' )
            {
                system("macosx-create-bundle", "$to=$from.app") if ( -d "$from.app" );
                system("ranlib", "$to" ) if ( $to =~ /\.a/ );
            }
            if ( $try > 1 ) {
                print_warning("File '$to' temporarily locked. Dependency bug?");
            }
            return 1;
        }
        else {
            print_error("can't rename temporary file to $to: $!",0);
        }
    }
    else {
        print_error("can't copy $from: $!",0);
        my $destdir = dirname($to);
        if ( ! -d $destdir ) {
            print_error("directory '$destdir' does not exist", 0);
        }
    }
    unlink($temp_file);
    return 0;
}

sub is_newer
{
        # returns whole stat buffer if newer
        my $from = shift;
        my $to = shift;
        my $touch = shift;
        my (@from_stat, @to_stat);

        @from_stat = stat($from.$maybedot);
        if ( $opt_checkdlst ) {
            my $outtree = expand_macros("%__SRC%");
            my $commonouttree = expand_macros("%COMMON_OUTDIR%");
            if ( $from !~ /$outtree/ ) {
                if ( $from !~ /$commonouttree/ ) {
                    print_warning("'$from' does not exist") unless -e _;
                }
            }
        }
        return 0 unless -f _;

        if ( $touch ) {
            $from_stat[9] = time();
        }
        # adjust timestamps to even seconds
        # this is necessary since NT platforms have a
        # 2s modified time granularity while the timestamps
        # on Samba volumes have a 1s granularity

        $from_stat[9]-- if $from_stat[9] % 2;

        if ( $to =~ /^\Q$dest\E/ ) {
            if ( $from_stat[9] > $logfiledate ) {
                $logfiledate = $from_stat[9];
            }
        }

        @to_stat = stat($to.$maybedot);
        return \@from_stat unless -f _;

        if ( $opt_force ) {
            return \@from_stat;
        }
        else {
            return ($from_stat[9] > $to_stat[9]) ? \@from_stat : 0;
        }
}

sub filter_out
{
    my $file = shift;

    foreach my $pattern ( @copy_filter_patterns ) {
        if  ( $file =~ /$pattern/ ) {
           print "filter out: $file\n" if $is_debug;
           return 1;
        }
    }

    return 0;
}

sub fix_file_permissions
{
    my $mode = shift;
    my $file = shift;

    if ( ($mode >> 6) % 2 == 1 ) {
        $mode = 0777 & ~$umask;
    }
    else {
        $mode = 0666 & ~$umask;
    }
    chmod($mode, $file);
}

sub get_latest_patchlevel
{
    # note: feed only well formed library names to this function
    # of the form libfoo.so.x.y.z with x,y,z numbers

    my @sorted_files = sort by_rev @_;
    return $sorted_files[-1];

    sub by_rev {
    # comparison function for sorting
        my (@field_a, @field_b, $i);

        $a =~ /^(lib[\w-]+(\.so|\.dylib))\.(\d+)\.(\d+)\.(\d+)$/;
        @field_a = ($3, $4, $5);
        $b =~ /^(lib[\w-]+(\.so|\.dylib))\.(\d+)\.(\d+)\.(\d+)$/;
        @field_b = ($3, $4, $5);

        for ($i = 0; $i < 3; $i++)
          {
              if ( ($field_a[$i] < $field_b[$i]) ) {
                  return -1;
              }
              if ( ($field_a[$i] > $field_b[$i]) ) {
                  return 1;
              }
          }

        # can't happen
        return 0;
    }

}

sub push_default_actions
{
    # any default action (that is an action which must be done even without
    # a corresponding d.lst entry) should be pushed here on the
    # @action_data list.
    my $subdir;
    my @subdirs = (
                    'bin',
                    'doc',
                    'inc',
                    'lib',
                    'par',
                    'pck',
                    'rdb',
                    'res',
                    'xml'
                );
    push(@subdirs, 'zip') if $opt_zip;
    push(@subdirs, 'idl');
    push(@subdirs, 'pus');
    my @common_subdirs = (
                    'bin',
                    'idl',
                    'inc',
                    'pck',
                    'pus',
                    'res'
                );
    push(@common_subdirs, 'zip') if $opt_zip;

    if ( ! $opt_delete ) {
        # create all the subdirectories on solver
        foreach $subdir (@subdirs) {
            push(@action_data, ['mkdir', "%_DEST%/$subdir"]);
        }
    }
    push(@action_data, ['mkdir', "%_DEST%/inc/$module"]);
    push(@action_data, ['mkdir', "%_DEST%/res/img"]);

    # need to copy libstaticmxp.dylib for Mac OS X
    if ( $^O eq 'darwin' )
    {
        push(@action_data, ['copy', "../%__SRC%/lib/lib*static*.dylib %_DEST%/lib/lib*static*.dylib"]);
    }
}

sub walk_addincpath_list
{
    my (@addincpath_headers);
    return if $#addincpath_list == -1;

    # create hash with all addincpath header names
    for (my $i = 0; $i <= $#addincpath_list; $i++) {
        my @field = split('/', $addincpath_list[$i][0]);
        push (@addincpath_headers, $field[-1]);
    }

    # now stream all addincpath headers through addincpath filter
    for (my $i = 0; $i <= $#addincpath_list; $i++) {
        add_incpath_if_newer($addincpath_list[$i][0], $addincpath_list[$i][1], \@addincpath_headers)
                ? $files_copied++ : $files_unchanged++;
    }
}

sub add_incpath_if_newer
{
    my $from = shift;
    my $to = shift;
    my $modify_headers_ref = shift;
    my ($from_stat_ref, $header);

    push_on_ziplist($to) if $opt_zip;
    push_on_loglist("ADDINCPATH", "$from", "$to") if $opt_log;

    if ( $opt_delete ) {
        print "REMOVE: $to\n" if $opt_verbose;
        my $rc = unlink($to);
        return 1 if $rc;
        return 0;
    }

    if ( $from_stat_ref = is_newer($from, $to) ) {
        print "ADDINCPATH: $from -> $to\n" if $opt_verbose;

        return 1 if $opt_check;

        my $save = $/;
        undef $/;
        open(FROM, "<$from");
        # slurp whole file in one big string
        my $content = <FROM>;
        close(FROM);
        $/ = $save;

        foreach $header (@$modify_headers_ref) {
            $content =~ s/#include [<"]$header[>"]/#include <$module\/$header>/g;
        }

        open(TO, ">$to");
        print TO $content;
        close(TO);

        utime($$from_stat_ref[9], $$from_stat_ref[9], $to);
        fix_file_permissions($$from_stat_ref[2], $to);
        return 1;
    }
    return 0;
}

sub push_on_ziplist
{
    my $file = shift;
    return if ( $opt_check );
    # strip $dest from path since we don't want to record it in zip file
    if ( $file =~ s#^\Q$dest\E/##o ) {
        push(@zip_list, $file);
    } elsif ( $file =~ s#^\Q$common_dest\E/##o ) {
        push(@common_zip_list, $file);
    }
}

sub push_on_loglist
{
    my @entry = @_;
    return 0 if ( $opt_check );
    return -1 if ( $#entry != 2 );
    if (( $entry[0] eq "COPY" ) || ( $entry[0] eq "ADDINCPATH" )) {
        return 0 if ( ! -e $entry[1].$maybedot );
        # make 'from' relative to source root
        $entry[1] = $repository ."/" . $module . "/prj/" . $entry[1];
        $entry[1] =~ s/$module\/prj\/\.\./$module/;
    }
    # platform or common tree?
    my $common;
    if ( $entry[2] =~ /^\Q$dest\E/ ) {
        $common = 0;
    } else {
        warn "Neither common nor platform tree?";
        return;
    }
    # make 'to' relative to SOLARVERSION
    my $solarversion  = $ENV{'SOLARVERSION'};
    $solarversion =~ s#\\#/#g;
    $entry[2] =~ s/^\Q$solarversion\E\///;

    if ( $common ) {
        push @common_log_list, [@entry];
    } else {
        push @log_list, [@entry];
    }
    return 1;
}

sub zip_files
{
    my $zipexe = 'zip';
    $zipexe .= ' -y' unless  $^O eq 'MSWin32';

    my ($platform_zip_file, $common_zip_file);
    $platform_zip_file = "%_DEST%/zip/$module.zip";
    $platform_zip_file = expand_macros($platform_zip_file);
    my (%dest_dir, %list_ref);
    $dest_dir{$platform_zip_file} = $dest;
    $list_ref{$platform_zip_file} = \@zip_list;

    my @zipfiles;
    $zipfiles[0] = $platform_zip_file;

    foreach my $zip_file ( @zipfiles ) {
        print "ZIP: updating $zip_file\n" if $opt_verbose;
        next if ( $opt_check );

        if ( $opt_delete ) {
            if ( -e $zip_file ) {
                unlink $zip_file or die "Error: can't remove file '$zip_file': $!";
            }
            next;
        }

        local $work_file = "";
        if ( $zip_file eq $common_zip_file) {
            # Zip file in common tree: work on uniq copy to avoid collisions
            $work_file = $zip_file;
            $work_file =~ s/\.zip$//;
            $work_file .= (sprintf('.%d-%d', $$, time())) . ".zip";
            die "Error: temp file $work_file already exists" if ( -e $work_file);
            if ( -e $zip_file ) {
                if ( -z $zip_file) {
                    # sometimes there are files of 0 byte size - remove them
                    unlink $zip_file or print_error("can't remove empty file '$zip_file': $!",0);
                } else {
                    if ( ! copy($zip_file, $work_file)) {
                        # give a warning, not an error:
                        # we can zip from scratch instead of just updating the old zip file
                        print_warning("can't copy'$zip_file' into '$work_file': $!", 0);
                        unlink $work_file;
                    }
                }
            }
        } else {
            # No pre processing necessary, working directly on solver.
            $work_file = $zip_file;
        }

        # zip content has to be relative to $dest_dir
        chdir($dest_dir{$zip_file}) or die "Error: cannot chdir into $dest_dir{$zip_file}";
        my $this_ref = $list_ref{$zip_file};
        open(ZIP, "| $zipexe -q -o -u -@ $work_file") or die "error opening zip file";
        foreach $file ( @$this_ref ) {
            print "ZIP: adding $file to $zip_file\n" if $is_debug;
            print ZIP "$file\n";
        }
        close(ZIP);
        fix_broken_cygwin_created_zips($work_file) if $^O eq "cygwin";

        if ( $zip_file eq $common_zip_file) {
            # rename work file back
            if ( -e $work_file ) {
                if ( -e $zip_file) {
                    # do some tricks to be fast. otherwise we may disturb other platforms
                    # by unlinking a file which just gets copied -> stale file handle.
                    my $buffer_file=$work_file . '_rm';
                    rename($zip_file, $buffer_file) or warn "Warning: can't rename old zip file '$zip_file': $!";
                    if (! rename($work_file, $zip_file)) {
                        print_error("can't rename temporary file to $zip_file: $!",0);
                        unlink $work_file;
                    }
                    unlink $buffer_file;
                } else {
                    if (! rename($work_file, $zip_file)) {
                        print_error("can't rename temporary file to $zip_file: $!",0);
                        unlink $work_file;
                    }
                }
            }
        }
    }
}

sub fix_broken_cygwin_created_zips
# add given extension to or strip it from stored path
{
    require Archive::Zip; import Archive::Zip;
    my $zip_file = shift;

    $zip = Archive::Zip->new();
    unless ( $zip->read($work_file) == AZ_OK ) {
        die "Error: can't open zip file '$zip_file' to fix broken cygwin file permissions";
    }
    my $latest_member_mod_time = 0;
    foreach $member ( $zip->members() ) {
        my $attributes = $member->unixFileAttributes();
        $attributes &= ~0xFE00;
        print $member->fileName() . ": " . sprintf("%lo", $attributes) if $is_debug;
        $attributes |= 0x10; # add group write permission
        print "-> " . sprintf("%lo", $attributes) . "\n" if $is_debug;
        $member->unixFileAttributes($attributes);
        if ( $latest_member_mod_time < $member->lastModTime() ) {
            $latest_member_mod_time = $member->lastModTime();
        }
    }
    die "Error: can't overwrite zip file '$zip_file' for fixing permissions" unless $zip->overwrite() == AZ_OK;
    utime($latest_member_mod_time, $latest_member_mod_time, $zip_file);
}

sub get_tempfilename
{
    my $temp_dir = shift;
    $temp_dir = ( -d '/tmp' ? '/tmp' : $ENV{TMPDIR} || $ENV{TEMP} || '.' )
            unless defined($temp_dir);
     if ( ! -d $temp_dir ) {
        die "no temp directory $temp_dir\n";
    }
    my $base_name = sprintf( "%d-%di-%d", $$, time(), $tempcounter++ );
    return "$temp_dir/$base_name";
}

sub write_log
{
    my (%log_file, %file_date);
    $log_file{\@log_list} = "%_DEST%/inc/$module/deliver.log";
    $log_file{\@common_log_list} = "%COMMON_DEST%/inc/$module/deliver.log";
    $file_date{\@log_list} = $logfiledate;
    $file_date{\@common_log_list} = $commonlogfiledate;

    my @logs = ( \@log_list );
    foreach my $log ( @logs ) {
        $log_file{$log} = expand_macros( $log_file{$log} );
        if ( $opt_delete ) {
            print "LOG: removing $log_file{$log}\n" if $opt_verbose;
            next if ( $opt_check );
            unlink $log_file{$log};
        } else {
            print "LOG: writing $log_file{$log}\n" if $opt_verbose;
            next if ( $opt_check );
            open( LOGFILE, "> $log_file{$log}" ) or warn "Error: could not open log file.";
            foreach my $item ( @$log ) {
                print LOGFILE "@$item\n";
            }
            close( LOGFILE );
            utime($file_date{$log}, $file_date{$log}, $log_file{$log});
        }
        push_on_ziplist( $log_file{$log} ) if $opt_zip;
    }
    return;
}

sub check_dlst
{
    my %createddir;
    my %destdir;
    my %destfile;
    # get all checkable actions to perform
    foreach my $action ( @action_data ) {
        my $path = expand_macros( $$action[1] );
        if ( $$action[0] eq 'mkdir' ) {
            $createddir{$path} ++;
        } elsif (( $$action[0] eq 'copy' ) || ( $$action[0] eq 'addincpath' )) {
            my ($from, $to) = split(' ', $path);
            my ($to_fname, $to_dir);
            my $withwildcard = 0;
            if ( $from =~ /[\*\?\[\]]/ ) {
                $withwildcard = 1;
            }
            ($to_fname, $to_dir) = fileparse($to);
            if ( $withwildcard ) {
                if ( $to !~ /[\*\?\[\]]/ ) {
                    $to_dir = $to;
                    $to_fname ='';
                }
            }
            $to_dir =~ s/[\\\/\s]$//;
            $destdir{$to_dir} ++;
            # Check: copy into non existing directory?
            if ( ! $createddir{$to_dir} ) {
                # unfortunately it is not so easy: it's OK if a subdirectory of $to_dir
                # gets created, because mkpath creates the whole tree
                foreach my $directory ( keys %createddir ) {
                    if ( $directory =~ /^\Q$to_dir\E[\\\/]/ ) {
                        $createddir{$to_dir} ++;
                        last;
                    }
                }
                print_warning("Possibly copying into directory without creating in before: '$to_dir'")
                    unless $createddir{$to_dir};
            }
            # Check: overwrite file?
            if ( ! $to ) {
                if ( $destfile{$to} ) {
                    print_warning("Multiple entries copying to '$to'");
                }
                $destfile{$to} ++;
            }
        }
    }
}

sub cleanup
{
    # remove empty directories
    foreach my $path ( @dirlist ) {
        $path = expand_macros($path);
        if ( $opt_check ) {
            print "RMDIR: $path\n" if $opt_verbose;
        } else {
            rmdir $path;
        }
    }
}

sub delete_output
{
    my $output_path = expand_macros("../%__SRC%");
    if ( "$output_path" ne "../" ) {
        if ( rmtree([$output_path], 0, 1) ) {
            print "Deleted output tree.\n" if $opt_verbose;
        }
        else {
            print_error("Error deleting output tree $output_path: $!",0);
        }
    }
    else {
        print_error("Output not deleted - INPATH is not set");
    }
}

sub print_warning
{
    my $message = shift;
    my $line = shift;

    print STDERR "$script_name: ";
    if ( $dlst_file ) {
        print STDERR "$dlst_file: ";
    }
    if ( $line ) {
        print STDERR "line $line: ";
    }
    print STDERR "WARNING: $message\n";
}

sub print_error
{
    my $message = shift;
    my $line = shift;

    print STDERR "$script_name: ";
    if ( $dlst_file ) {
        print STDERR "$dlst_file: ";
    }
    if ( $line ) {
        print STDERR "line $line: ";
    }
    print STDERR "ERROR: $message\n";
    $error ++;
}

sub print_stats
{
    print "Module '$module' delivered ";
    if ( $error ) {
        print "with errors\n";
    } else {
        print "successfully.";
        if ( $opt_delete ) {
            print " $files_copied files removed,";
        }
        else {
            print " $files_copied files copied,";
        }
        print " $files_unchanged files unchanged\n";
    }
}

sub cleanup_and_die
{
    # clean up on unexpected termination
    my $sig = shift;
    if ( defined($temp_file) && -e $temp_file ) {
        unlink($temp_file);
    }
    if ( defined($work_file) && -e $work_file ) {
        unlink($work_file);
        print STDERR "$work_file removed\n";
    }

    die "caught unexpected signal $sig, terminating ...";
}

sub usage
{
    my $exit_code = shift;
    print STDERR "Usage:\ndeliver [OPTIONS] [DESTINATION-PATH]\n";
    print STDERR "Options:\n";
    print STDERR "  -check       just print what would happen, no actual copying of files\n";
    print STDERR "  -checkdlst   be verbose about (possible) d.lst bugs\n";
    print STDERR "  -delete      delete files (undeliver), use with care\n";
    print STDERR "  -deloutput   remove the output tree after copying\n";
    print STDERR "  -dontdeletecommon do not delete common files (for -delete option)\n";
    print STDERR "  -force       copy even if not newer\n";
    print STDERR "  -help        print this message\n";
    if ( !defined($ENV{GUI}) || $ENV{GUI} ne 'WNT' ) {
        print STDERR "  -link        hard link files into the solver to save disk space\n";
    }
    print STDERR "  -quiet       be quiet, only report errors\n";
    print STDERR "  -verbose     be verbose\n";
    print STDERR "  -zip         additionally create zip files of delivered content\n";
    print STDERR "Options '-zip' and a destination-path are mutually exclusive.\n";
    print STDERR "Options '-check' and '-quiet' are mutually exclusive.\n";
    exit($exit_code);
}

# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:
