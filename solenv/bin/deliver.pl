:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   $RCSfile: deliver.pl,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: hr $ $Date: 2001-04-19 15:54:36 $
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
# deliver - copy from module output tree to solver
#

use Cwd;
use File::Basename;
use File::Copy;
use File::DosGlob 'glob';
use File::Path;

#### script id #####

( $script_name = $0 ) =~ s/^.*\W(\w+).pl$/$1/;

$id_str = ' $Revision: 1.2 $ ';
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
                        'ln',
                        'mkdir',
                        'touch'
                        );

$is_debug           = 0;

$module             = 0;            # module name
$base_dir           = 0;            # path to module base directory
$dlst_file          = 0;            # path to d.lst
$dir_mode           = 0755;         # default directory creation mode

@action_data        = ();           # LoL with all action data
@macros             = ();           # d.lst macros

$files_copied       = 0;            # statistics
$files_unchanged    = 0;            # statistics

$opt_force          = 0;            # option force copy
$opt_minor          = 0;            # option deliver in minor
$opt_check          = 0;            # do actually execute any action


for (@action_list) {
    $action_hash{$_}++;
}

#### main ####

parse_options();
init_globals();
push_default_actions();
parse_dlst();
walk_action_data();
print_stats();

exit(0);

#### implemented actions #####

sub do_copy {
    # we need to copy twice: once from the platform dependent output tree
    # and once from the common output tree, in this order
    my ($dependent, $common, $from, $to, $file_list);
    my $line = shift;

    $dependent = expand_macros($line);
    ($from, $to) = split(' ', $dependent);
    print "copy dependant: from: $from, to: $to\n" if $is_debug;
    glob_and_copy($from, $to);

    $line =~ s/%__SRC%/%COMMON_OUTDIR%/ig;
    $common = expand_macros($line);
    ($from, $to) = split(' ', $common);
    print "copy common: from: $from, to: $to\n" if $is_debug;
    glob_and_copy($from, $to);
}

sub do_dos {
    my $line = shift;
    # no macro expansion, no slash conversion
    if ( $opt_check ) {
        print "DOS: $line\n";
    }
    else {
        system($line);
    }
}

sub do_hedabu {
#   do_copy(shift);
    print_error("action 'hedabu' not yet implemented");
}

sub do_ln {
    print_error("action 'ln' not yet implemented");
}

sub do_mkdir {
    my $path = expand_macros(shift);
    if ( $opt_check ) {
        print "MKDIR: $path\n";
    }
    else {
        mkpath($path, 0, $dir_mode);
    }
}

sub do_touch {
    my $line = shift;

    print_error("action 'touch' not yet implemented");
}

#### subroutines #####

sub parse_options {
    foreach $arg (@ARGV) {
        $arg =~ /^-force$/ and $opt_force = 1 and next;
        $arg =~ /^-minor$/ and $opt_minor = 1 and next;
        $arg =~ /^-check$/ and $opt_check = 1 and next;
        print_error("invalid option $arg");
        usage();
        exit(1);
    }
}

sub init_globals {
    my ($dllsuffix, $gui, $inpath, $offenv, $outpath, $solarversion, $updminor);
    my ($ext, $umask);
    ($module, $base_dir, $dlst_file) =  get_base();
    print "Module=$module, Base_Diri=$base_dir, d.lst=$dlst_file\n" if $is_debug;

    $umask = umask();
    if ( defined($umask) ) {
        $dir_mode = 0777 - $umask;
    }

    $common_outdir  = $ENV{COMMON_OUTDIR};
    $dllsuffix      = $ENV{DLLSUFFIX};
    $gui            = lc($ENV{GUI});
    $inpath         = $ENV{INPATH};
    $offenv         = $ENV{OFFENV_PATH};
    $outpath        = $ENV{OUTPATH};
    $solarversion   = $ENV{SOLARVERSION};
    $updminor       = $ENV{UPDMINOR} ? $ENV{UPDMINOR} : $ENV{DMAKE_MINOR};

    # product build?
    $common_outdir = $common_outdir . ".pro" if $inpath =~ /\.pro$/;

    $ext = "";
    if ( $opt_minor ) {
        if ( $updminor ) {
            $ext = ".$updminor";
        }
        else {
            print_error("can't determine UPDMINOR", 0);
            exit(3);
        }
    }

    $dest = "$solarversion/$inpath";

    # the following macros are obsolete, will be flagged as error
    # %__WORKSTAMP%
    # %GUIBASE%
    # %SDK%
    # %SOLARVER%
    # %UPD%

    # valid macros
    @macros = (
                [ '%__OFFENV%',         $offenv         ],
                [ '%__PRJROOT%',        $base_dir       ],
                [ '%__SRC%',            $inpath         ],
                [ '%_DEST%',            $dest           ],
                [ '%_EXT%',             $ext            ],
                [ '%COMMON_OUTDIR%',    $common_outdir  ],
                [ '%DLLSUFFIX%',        $dllsuffix      ],
                [ '%GUI%',              $gui            ],
                [ '%OUTPATH%',          $outpath        ]
              );
}

sub get_base {
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

sub parse_dlst {
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

sub expand_macros {
    # expand all macros and change backslashes to slashes
    my $line        = shift;
    my $line_cnt    = shift;
    my $i;

    for ($i=0; $i<=$#macros; $i++)  {
        $line =~ s/$macros[$i][0]/$macros[$i][1]/i
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

sub walk_action_data {
    # all actions have to be excuted relative to the prj directory
    chdir("$base_dir/prj");
    # dispatch depending on action type
    for ($i=0; $i <= $#action_data; $i++) {
            &{"do_".$action_data[$i][0]}($action_data[$i][1]);
    }
}

sub glob_and_copy {
    my $from = shift;
    my $to = shift;
    my $to_dir;
    my $replace = 0;

    # if $to contains wildcards, replace wildcards with filename
    # otherwise consider it as directory and attach filename
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
            copy_if_newer($file, $copy) ? $files_copied++ : $files_unchanged++;
        }
    }
    else {
        # no globbing but renaming possible
        copy_if_newer($from, $to) ? $files_copied++ : $files_unchanged++;
    }
}

sub copy_if_newer {
    # return 0 if file is unchanged ( for whatever reason )
    # return 1 if file has been copied
    my $from = shift;
    my $to = shift;
    my $new_time;

    print "testing $from, $to\n" if $is_debug;
    return 0 unless ($new_time = is_newer($from, $to));

    print "COPY: $from -> $to\n";
    if ( $opt_check ) {
        return 1;
    }
    else {
        my $rc = copy($from, $to);
        if ( $rc) {
            utime($new_time, $new_time, $to);
            return 1;
        }
        else {
            print_error("can't copy $from: $!",0);
        }
    }
}

sub is_newer {
        # returns new time if newer or zero if older
        my $from = shift;
        my $to = shift;
        my (@from_stat, @to_stat);

        @from_stat = stat($from);
        return 0 unless -f _;

        # adjust timestamps to even seconds
        # this is necessary since NT platforms have a
        # 2s modified time granularity while the timestamps
        # on Samba volumes have a 1s granularity

        $from_stat[9]-- if $from_stat[9] % 2;

        @to_stat = stat($to);
        return $from_stat[9] unless -f _;

        if ( $opt_force ) {
            return $from_stat[9];
        }
        else {
            return ($from_stat[9] > $to_stat[9]) ? $from_stat[9] : 0;
        }
}

sub push_default_actions {
    # any default action (that is an action which must be done even without
    # a corresponding d.lst entry) should be pushed here on the
    # @action_data list. Currently we have just create the dfault solver
    # directories
    my $subdir;
    my @subdirs = (
                    'bin/app',
                    'idl',
                    'inc',
                    'lib',
                    'rdb',
                    'res',
                    'xml'
                );

    # create all the subdirectories on solver
    foreach $subdir (@subdirs) {
        push(@action_data, ['mkdir', "%_DEST%/$subdir"]);
    }
}

sub print_error {
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

sub print_stats {
    print "Statistics:\n";
    print "Files copied: $files_copied\n";
    print "Files unchanged/not matching: $files_unchanged\n";
}

sub usage {
    print STDERR "Usage:\ndeliver [-force] [-minor] [-check]\n";
    print STDERR "Options:\n  -force\tcopy even if not newer\n";
    print STDERR "  -minor\tdeliver into minor\n";
    print STDERR "  -check\tjust print what would happen, no actual copying of files\n";
}

# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:
