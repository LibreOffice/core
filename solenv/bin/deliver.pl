:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   $RCSfile: deliver.pl,v $
#
#   $Revision: 1.50 $
#
#   last change: $Author: hr $ $Date: 2003-07-16 18:19:10 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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

#### script id #####

( $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

$id_str = ' $Revision: 1.50 $ ';
$id_str =~ /Revision:\s+(\S+)\s+\$/
  ? ($script_rev = $1) : ($script_rev = "-");

print "$script_name -- version: $script_rev\n";

#### globals ####

### valid actions ###
# if you add a action 'foo', than add 'foo' to this list and
# implement 'do_foo()' in the implemented actions area
@action_list        =   (           # valid actions
                        'copy',
                        'dos',
                        'hedabu',
                        'linklib',
                        'mkdir',
                        'symlink',
                        'touch'
                        );

# copy filter: files matching these patterns won't be copied by
# the copy action
@copy_filter_patterns = (
                        '\/_[\w\-]+\.dll$' # Win32 debug dll's
                        );

$is_debug           = 0;

$module             = 0;            # module name
$base_dir           = 0;            # path to module base directory
$dlst_file          = 0;            # path to d.lst
$umask              = 22;           # default file/directory creation mask
$dest               = 0;            # optional destination path
$common_dest        = 0;            # common tree on solver

@action_data        = ();           # LoL with all action data
@macros             = ();           # d.lst macros
@hedabu_list        = ();           # files which have to be filtered through hedabu
@zip_list           = ();           # files which have to be zipped
@common_zip_list    = ();           # common files which have to be zipped

$files_copied       = 0;            # statistics
$files_unchanged    = 0;            # statistics

$opt_force          = 0;            # option force copy
$opt_minor          = 0;            # option deliver in minor
$opt_check          = 0;            # do actually execute any action
$opt_zip            = 0;            # create an additional zip file
$opt_link           = 0;            # hard link files into the solver to save disk space
$opt_deloutput      = 0;            # delete the output tree for the project once successfully delivered

# zip is default for RE
$opt_zip = 1 if ( defined($ENV{UPDATER}) && $ENV{UPDATER} eq 'YES' && !defined($ENV{BUILD_SOSL}) );

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
walk_action_data();
walk_hedabu_list();
zip_files() if $opt_zip;
delete_output() if $opt_deloutput;
print_stats();

exit(0);

#### implemented actions #####

sub do_copy
{
    # We need to copy four times:
    # from the platform dependent output tree,
    # from the so platform dependent tree,
    # from the common output tree,
    # and from the so common output tree
    # in this order.
    my ($dependent, $common, $from, $to, $file_list);
    my $line = shift;
    my $touch = 0;

    $dependent = expand_macros($line);
    ($from, $to) = split(' ', $dependent);
    print "copy dependent: from: $from, to: $to\n" if $is_debug;
    glob_and_copy($from, $to, $touch);

    my $line_so = mod_so($line);
    if ( $line_so ) {
        my $dependent = expand_macros($line_so);
        ($from, $to) = split(' ', $dependent);
        print "copy dependent: from: $from, to: $to\n" if $is_debug;
        glob_and_copy($from, $to, $touch);
    }

    $line =~ s/%__SRC%/%COMMON_OUTDIR%/ig;
    if ( $line =~ /%COMMON_OUTDIR%/ ) {
        $line =~ s/%_DEST%/%COMMON_DEST%/ig;
        $common = expand_macros($line);
        ($from, $to) = split(' ', $common);
        print "copy common: from: $from, to: $to\n" if $is_debug;
        glob_and_copy($from, $to, $touch);

        my $line_so = mod_so($line);
        if ( $line_so ) {
            $common = expand_macros($line_so);
            ($from, $to) = split(' ', $common);
            print "copy common: from: $from, to: $to\n" if $is_debug;
            glob_and_copy($from, $to, $touch);
        }
    }
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

sub do_hedabu
{
    # just collect all hedabu files, actual filtering is done later
    my $line = shift;
    my ($from, $to);
    my @globbed_files = ();

    $line = expand_macros($line);
    ($from, $to) = split(' ', $line);

    push( @hedabu_list, @{glob_line($from, $to)});
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
    $to_dir = expand_macros('%_DEST%/lib%_EXT%');

    @globbed_files = glob("$from_dir/$lib");

    if ( $#globbed_files == -1 ) {
       $files_unchanged++;
       return;
    }

    foreach $lib (@globbed_files) {
        $lib = basename($lib);
        if ( $lib =~ /^(lib[\w-]+(\.so|\.dylib))\.(\d+)\.(\d+)(\.(\d+))?$/ ) {
           push(@{$globbed_hash{$1}}, $lib);
        }
        else {
            print_error("invalid library name: $lib");
        }
    }

    foreach $lib_base ( sort keys %globbed_hash ) {
        $lib = get_latest_patchlevel(@{$globbed_hash{$lib_base}});

        $lib =~ /^(lib[\w-]+(\.so|\.dylib))\.(\d+)\.(\d+)(\.(\d+))?$/;
        $lib_major = "$lib_base.$3";

        if ( $opt_check ) {
            if ( $opt_delete ) {
                print "REMOVE: $to_dir/$lib_major\n";
                print "REMOVE: $to_dir/$lib_base\n";
            }
            else {
                print "LINKLIB: $to_dir/$lib -> $to_dir/$lib_major\n";
                print "LINKLIB: $to_dir/$lib -> $to_dir/$lib_base\n";
            }
        }
        else {
            if ( $opt_delete ) {
                print "REMOVE: $to_dir/$lib_major\n";
                print "REMOVE: $to_dir/$lib_base\n";
                unlink "$to_dir/$lib_major";
                unlink "$to_dir/$lib_base";
                if ( $opt_zip ) {
                    push_on_ziplist("$to_dir/$lib_major");
                    push_on_ziplist("$to_dir/$lib_base");
                }
                return;
            }
            my $symlib;
            my @symlibs = ("$to_dir/$lib_major", "$to_dir/$lib_base");
            # remove old symlinks
            unlink(@symlibs);
            foreach $symlib (@symlibs) {
                print "LINKLIB: $lib -> $symlib\n";
                if ( !symlink("$lib", "$symlib") ) {
                    print_error("can't symlink $lib -> $symlib: $!",0);
                }
                else {
                    push_on_ziplist($symlib) if $opt_zip;
                }
            }
        }
    }
}

sub do_mkdir
{
    my $path = expand_macros(shift);
    if ( $opt_check ) {
        print "MKDIR: $path\n";
    }
    else {
        mkpath($path, 0, 0777-$umask);
    }
}

sub do_symlink
{
    my $line = shift;

    $line = expand_macros($line);
    ($from, $to) = split(' ',$line);
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
        print "REMOVE: $to\n";
        unlink $to;
        if ( $opt_delete ) {
            push_on_ziplist($to) if $opt_zip;
            return;
        }

        print "SYMLIB: $from -> $to\n";
        if ( !symlink("$from", "$to") ) {
            print_error("can't symlink $from -> $to: $!",0);
        }
        else {
            push_on_ziplist($to) if $opt_zip;
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
    while ( $arg = shift @ARGV ) {
        $arg =~ /^-force$/  and $opt_force  = 1 and next;
        $arg =~ /^-minor$/  and $opt_minor  = 1 and next;
        $arg =~ /^-check$/  and $opt_check  = 1 and next;
        $arg =~ /^-zip$/    and $opt_zip    = 1 and next;
        $arg =~ /^-delete$/ and $opt_delete = 1 and next;
        $arg =~ /^-link$/ and $ENV{GUI} ne 'WNT' and $opt_link = 1 and next;
        $arg =~ /^-deloutput$/ and $opt_deloutput = 1 and next;
        print_error("invalid option $arg") if ( $arg =~ /-/ );
        if ( $arg =~ /-/ || $#ARGV > -1 ) {
            usage();
            exit(1);
        }
        $dest = $arg;
    }
    # $dest and $opt_zip or $opt_delete are mutually exclusive
    if ( $dest and ($opt_zip || $opt_delete) ) {
        usage();
        exit(1);
    }
    # $opt_delete implies $opt_force
    $opt_force = 1 if $opt_delete;
}

sub init_globals
{
    my $ext;
    ($module, $base_dir, $dlst_file) =  get_base();

    # for CWS:
    $module =~ s/\.lnk$//;

    print "Module=$module, Base_Diri=$base_dir, d.lst=$dlst_file\n" if $is_debug;

    $umask = umask();
    if ( !defined($umask) ) {
        $umask = 22;
    }

    my $build_sosl    = $ENV{'BUILD_SOSL'};
    my $common_outdir = $ENV{'COMMON_OUTDIR'};
    my $dllsuffix     = $ENV{'DLLSUFFIX'};
    my $gui           = lc($ENV{'GUI'});
    my $inpath        = $ENV{'INPATH'};
    my $offenv        = $ENV{'OFFENV_PATH'};
    my $outpath       = $ENV{'OUTPATH'};
    my $solarversion  = $ENV{'SOLARVERSION'};
    my $updater       = $ENV{'UPDATER'};
    my $upd           = $ENV{'UPD'};
    my $updminor      = $ENV{'UPDMINOR'};
    my $work_stamp    = $ENV{'WORK_STAMP'};

    my $l10n_framework    = $ENV{'L10N_framework'};
    $l10n_framework = "INVALID" if ! defined $l10n_framework;

    # special security check for release engineers
    if ( defined($updater) && !defined($build_sosl) && !$opt_force) {
        my $path = cwd();
        if ( $path !~ /$work_stamp/io ) {
            print_error("can't deliver from local directory to SOLARVERSION");
            print STDERR "\nDANGER! Release Engineer:\n";
            print STDERR "do you really want to deliver from $path to SOLARVERSION?\n";
            print STDERR "If so, please use the -force switch\n\n";
            exit(7);
        }
    }

    # do we have a valid environment?
    if ( !defined($inpath) ) {
            print_error("no environment", 0);
            exit(3);
    }

    # product build?
    if ($common_outdir ne "") {
        $common_outdir = $common_outdir . ".pro" if $inpath =~ /\.pro$/;
    }

    $ext = "";
    if ( ($opt_minor || $updminor) && !$dest ) {
        if ( $updminor ) {
            $ext = ".$updminor";
        }
        else {
            print_error("can't determine UPDMINOR", 0);
            exit(3);
        }
    }

    if ($common_outdir ne "") {
        $common_dest = "$solarversion/$common_outdir" if ( !$dest );
        $dest = "$solarversion/$inpath" if ( !$dest );
    } else {
        $dest = "$solarversion/$inpath" if ( !$dest );
        $common_dest = $dest;
    }

    # the following macros are obsolete, will be flagged as error
    # %__WORKSTAMP%
    # %GUIBASE%
    # %SDK%
    # %SOLARVER%

    # valid macros
    @macros = (
                [ '%__OFFENV%',         $offenv         ],
                [ '%__PRJROOT%',        $base_dir       ],
                [ '%__SRC%',            $inpath         ],
                [ '%_DEST%',            $dest           ],
                [ '%_EXT%',             $ext            ],
                [ '%COMMON_OUTDIR%',    $common_outdir  ],
                [ '%COMMON_DEST%',      $common_dest    ],
                [ '%DLLSUFFIX%',        $dllsuffix      ],
                [ '%GUI%',              $gui            ],
                [ '%OUTPATH%',          $outpath        ],
                [ '%UPD%',              $upd            ],
                [ '%L10N_FRAMEWORK%',   $l10n_framework ]
              );

    # find out if the system supports symlinks
    $has_symlinks = eval { symlink("",""); 1 };
}

sub get_base
{
    # a module base dir contains a subdir 'prj'
    # which in turn contains a file 'd.lst'
    my (@field, $base, $dlst);
    my $path = cwd();

    @field = split(/\//, $path);

    while ( $#field != -1 ) {
        $base = join('/', @field);
        $dlst = $base . '/prj/d.lst';
        last if -e $dlst;
        pop @field;
    }

    if ( $#field == -1 ) {
        print_error("can't determine module");
        exit(2);
    }
    else {
        return ($field[-1], $base, $dlst);
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
        if ( /^\s*(\w+?):\s+(.*)$/ ) {
            if ( !exists $action_hash{$1} ) {
                print_error("unknown action: \'$1\'", $line_cnt);
                exit(4);
            }
            push(@action_data, [$1, $2]);
        }
        else {
            push(@action_data, ['copy', $_]);
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
    #   exit(5);
    }
    $line =~ s#\\#/#g;
    return $line;
}

sub mod_so
{
    my $line = shift;

    if ( $line =~ s/(%__SRC%[\\|\/]bin)/$1\\so/i ) {
        $line =~ s/(%_DEST%[\\|\/]bin%_EXT%)/$1\\so/i;
        return $line;
    }
    elsif ( $line =~ s/(%COMMON_OUTDIR%[\\|\/]bin)/$1\\so/i ) {
        $line =~ s/(%COMMON_DEST%[\\|\/]bin%_EXT%)/$1\\so/i;
        return $line;
    }
    else {
        return undef;
    }
}

sub walk_action_data
{
    # all actions have to be excuted relative to the prj directory
    chdir("$base_dir/prj");
    # dispatch depending on action type
    for (my $i=0; $i <= $#action_data; $i++) {
            &{"do_".$action_data[$i][0]}($action_data[$i][1]);
    }
    # special actions for Mac OS X
    system("create-libstatic-link", expand_macros("%_DEST%/lib%_EXT%")) if ( $^O eq 'darwin' );
}

sub glob_line
{
    my $from = shift;
    my $to = shift;
    my $to_dir = shift;
    my $replace = 0;
    my @globbed_files = ();

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
            my ($fname, $dir) = fileparse($file);
            my $copy = ($replace) ? $to_dir . $fname : $to . '/' . $fname;
            push(@globbed_files, [$file, $copy]);
        }
    }
    else {
        # no globbing but renaming possible
        push(@globbed_files, [$from, $to]);
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

sub copy_if_newer
{
    # return 0 if file is unchanged ( for whatever reason )
    # return 1 if file has been copied
    my $from = shift;
    my $to = shift;
    my $touch = shift;
    my $from_stat_ref;

    print "testing $from, $to\n" if $is_debug;
    push_on_ziplist($to) if $opt_zip;
    return 0 unless ($from_stat_ref = is_newer($from, $to, $touch));

    if ( $opt_delete ) {
        print "REMOVE: $to\n";
        return 1 if $opt_check;
        my $rc = unlink($to);
        return 1 if $rc;
        return 0;
    }

    if( !$opt_check && $opt_link ) {
        # hard link if possible
        if( link($from, $to) ){
            print "LINK: $from -> $to\n";
            return 1;
        }
    }

    if( $touch ) {
       print "TOUCH: $from -> $to\n";
    }
    else {
       print "COPY: $from -> $to\n";
    }

    return 1 if( $opt_check );

    #
    # copy to temporary file first and rename later
    # to minimize the possibility for race conditions
    local $temp_file = sprintf('%s.%d-%d', $to, $$, time());
    my $rc = copy($from, $temp_file);
    if ( $rc) {
        $rc = utime($$from_stat_ref[9], $$from_stat_ref[9], $temp_file);
        if ( !$rc ) {
            print_error("can't update temporary file modification time '$temp_file': $!",0);
        }
        fix_file_permissions($$from_stat_ref[2], $temp_file);
        $rc = rename($temp_file, $to);
        if ( $rc ) {
            # handle special packaging of *.dylib files for Mac OS X
            if ( $^O eq 'darwin' )
            {
                system("create-bundle", $to) if ( $to =~ /\.dylib/ );
                system("create-bundle", "$to=$from.app") if ( -d "$from.app" );
                system("ranlib", "$to" ) if ( $to =~ /\.a/ );
            }
            return 1;
        }
        else {
            print_error("can't rename temporary file to $to: $!",0);
        }
    }
    else {
        print_error("can't copy $from: $!",0);
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

        @from_stat = stat($from);
        return 0 unless -f _;

        if ( $touch ) {
            $from_stat[9] = time();
        }
        # adjust timestamps to even seconds
        # this is necessary since NT platforms have a
        # 2s modified time granularity while the timestamps
        # on Samba volumes have a 1s granularity

        $from_stat[9]-- if $from_stat[9] % 2;

        @to_stat = stat($to);
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

    if ( $mode%2 == 1 ) {
        $mode = 0777 - $umask;
    }
    else {
        $mode = 0666 - $umask;
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
                    'idl',
                    'inc',
                    'lib',
                    'rdb',
                    'res',
                    'xml',
                );
    push(@subdirs, 'zip') if $opt_zip;
    my @common_subdirs = (
                    'bin',
                    'idl',
                    'inc',
                    'pck',
                );
    push(@common_subdirs, 'zip') if $opt_zip;

    if ( ! $opt_delete ) {
        # create all the subdirectories on solver
        foreach $subdir (@subdirs) {
            push(@action_data, ['mkdir', "%_DEST%/$subdir%_EXT%"]);
        }
        foreach $subdir (@common_subdirs) {
            push(@action_data, ['mkdir', "%COMMON_DEST%/$subdir%_EXT%"]);
        }
        push(@action_data, ['mkdir', "%_DEST%/bin%_EXT%/so"]);
        push(@action_data, ['mkdir', "%COMMON_DEST%/bin%_EXT%/so"]);

        # deliver build.lst to $dest/inc/$module
        push(@action_data, ['mkdir', "%_DEST%/inc%_EXT%/$module"]); # might be necessary
        push(@action_data, ['copy', "build.lst %_DEST%/inc%_EXT%/$module/build.lst"]);
        # and to $common_dest/inc/$module
        push(@action_data, ['mkdir', "%COMMON_DEST%/inc%_EXT%/$module"]); # might be necessary
        push(@action_data, ['copy', "build.lst %COMMON_DEST%/inc%_EXT%/$module/build.lst"]);
    }

    # need to copy libstaticmxp.dylib for Mac OS X
    if ( $^O eq 'darwin' )
    {
        push(@action_data, ['copy', "../%__SRC%/misc/*staticdatamembers.cxx %_DEST%/inc%_EXT%/*staticdatamembers.cxx"]);
        push(@action_data, ['copy', "../%__SRC%/misc/*staticdatamembers.h* %_DEST%/inc%_EXT%/*staticdatamembers.H*"]);
        push(@action_data, ['copy', "../%__SRC%/lib/lib*static*.dylib %_DEST%/lib%_EXT%/lib*static*.dylib"]);
    }
}

sub walk_hedabu_list
{
    my (@hedabu_headers);
    return if $#hedabu_list == -1;

    # create hash with all hedabu header names
    for (my $i = 0; $i <= $#hedabu_list; $i++) {
        my @field = split('/', $hedabu_list[$i][0]);
        push (@hedabu_headers, $field[-1]);
    }

    # now stream all hedabu headers through hedabu filter
    for (my $i = 0; $i <= $#hedabu_list; $i++) {
        hedabu_if_newer($hedabu_list[$i][0], $hedabu_list[$i][1], \@hedabu_headers)
                ? $files_copied++ : $files_unchanged++;
    }
}

sub hedabu_if_newer
{
    my $from = shift;
    my $to = shift;
    my $hedabu_headers_ref = shift;
    my ($from_stat_ref, $header);

    push_on_ziplist($to) if $opt_zip;

    if ( $opt_delete ) {
        print "REMOVE: $to\n";
        my $rc = unlink($to);
        return 1 if $rc;
        return 0;
    }

    if ( $from_stat_ref = is_newer($from, $to) ) {
        print "HEDABU: $from -> $to\n";

        return 1 if $opt_check;

        my $save = $/;
        undef $/;
        open(FROM, "<$from");
        # slurp whole file in one big string
        my $content = <FROM>;
        close(FROM);
        $/ = $save;

        # strip any carriage returns
        $content =~ tr/\r//d;
        # squeeze lines with white space only
        $content =~ s/\n\s+\n/\n\n/sg;
        # squeeze multiple blank lines
        $content =~ s/\n{3,}/\n\n/sg;

        foreach $header (@$hedabu_headers_ref) {
            $content =~ s/#include [<"]$header[>"]/#include <$module\/$header>/g;
        }

        # __SOLAR_PRIVATE hack
        $content =~ s/#if _SOLAR__PRIVATE/#if 0 \/\/ _SOLAR__PRIVATE/g;

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
    $dest =~ s#\\#/#g;
    $common_dest =~ s#\\#/#g;
    if ( $file =~ s#^$dest/##o ) {
        if ( $opt_minor ){
            # strip minor from path
            my $ext = "%_EXT%";
            $ext = expand_macros($ext);
            $file =~ s#^$ext##o;
        }
        push(@zip_list, $file);
    } elsif ( $file =~ s#^$common_dest/##o ) {
        if ( $opt_minor ){
            # strip minor from path
            my $ext = "%_EXT%";
            $ext = expand_macros($ext);
            $file =~ s#^$ext##o;
        }
        push(@common_zip_list, $file);
    }
}

sub zip_files
{
    my $file;
    my $zipexe = 'zip';
    $zipexe .= ' -y' unless  $^O eq 'MSWin32';

    my $platform_zip_file = "%_DEST%/zip%_EXT%/$module.zip";
    $platform_zip_file = expand_macros($platform_zip_file);
    my $common_zip_file = "%COMMON_DEST%/zip%_EXT%/$module.zip";
    $common_zip_file = expand_macros($common_zip_file);
    my (%dest_dir, %list_ref);
    $dest_dir{$platform_zip_file} = $dest;
    $dest_dir{$common_zip_file}   = $common_dest;
    $list_ref{$platform_zip_file} = \@zip_list;
    $list_ref{$common_zip_file}   = \@common_zip_list;

    foreach my $zip_file ( $platform_zip_file, $common_zip_file) {
        print "ZIP: updating $zip_file\n";
        # zip content has to be relative to $dest_dir
        chdir($dest_dir{$zip_file});
        my $this_ref = $list_ref{$zip_file};
        if ( $opt_delete ) {
            open(ZIP, "| $zipexe -q -o -d -@ $zip_file");
            foreach $file ( @$this_ref ) {
                print "ZIP: removing $file from $platform_zip_file\n" if $is_debug;
                print ZIP "$file\n";
            }
            close(ZIP);
        } else {
            open(ZIP, "| $zipexe -q -o -u -@ $zip_file");
            foreach $file ( @$this_ref ) {
                print "ZIP: adding $file to $zip_file\n" if $is_debug;
                print ZIP "$file\n";
            }
            close(ZIP);
        }
    }
}

sub delete_output
{
    my $output_path = expand_macros("../%__SRC%");
    if ( "$output_path" ne "../" ) {
        if ( rmtree([$output_path], 0, 1) ) {
            print "Deleted output tree.\n";
        }
        else {
            print_error("Error deleting output tree $output_path: $!",0);
        }
    }
    else {
        print_error("Output not deleted - INPATH is not set");
    }
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
}

sub print_stats
{
    print "Statistics:\n";
    if ( $opt_delete ) {
        print "Files removed $files_copied\n";
    }
    else {
        print "Files copied: $files_copied\n";
    }
    print "Files unchanged/not matching: $files_unchanged\n";
}

sub cleanup_and_die
{
    # clean up on unexpected termination
    my $sig = shift;
    if ( defined($temp_file) && -e $temp_file ) {
        unlink($temp_file);
    }

    die "caught unexpected signal $sig, terminating ...";
}

sub usage
{
    print STDERR "Usage:\ndeliver [-force] [-minor] [-check] [-zip] [-delete] [destination-path]\n";
    print STDERR "Options:\n  -force\tcopy even if not newer\n";
    print STDERR "  -minor\tdeliver into minor\n";
    print STDERR "  -check\tjust print what would happen, no actual copying of files\n";
    print STDERR "  -zip  \tcreate additional zip files of delivered content\n";
    print STDERR "  -delete\tdelete files, use with care\n";
    if ( !defined($ENV{GUI}) || $ENV{GUI} ne 'WNT' ) {
        print STDERR "  -link  \thard link files into the solver to save disk space\n";
    }
    print STDERR "  -deloutput\tremove the output tree\n";
    print STDERR "The option -zip and a destination-path are mutually exclusive\n";
}

# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:
