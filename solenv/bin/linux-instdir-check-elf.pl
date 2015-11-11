#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Check that all ELF files (i.e., executables and dynamic libraries) in $INSTDIR
# have non-suspicious NEEDED and RPATH entries:
#
# * No NEEDED entry shall contain a slash; that catches cases where external
#   dynamic libraries outside the LO installation are referenced with absolute
#   paths.
#
# * The RPATH entry shall either be absent or match one of "$ORIGIN" or
#   $ORIGIN/../../program (cf. gb_LinkTarget__RPATHS in
#   solenv/gbuild/platform/unxgcc.mk).
#
# This requires the file(1) and readelf(1) utilities to be available via PATH.

sub quoteArg {
    my ($arg) = @_;
    $arg =~ s/'/'\\''/;
    return '\'' . $arg . '\'';
}

sub visitFile {
    my ($file) = @_;
    my $elf = 0;
    my $call = 'file -b ' . quoteArg($file);
    open(IN, '-|', $call) or die "cannot $call";
    my $first = 1;
    while (<IN>) {
        $first or die "multi-line output from $call";
        if (m'^ELF ') {
            $elf = 1;
        }
        $first = 0;
    }
    close(IN) or die "got $? from $call";
    not $first or die "no output from $call";
    if (not $elf) {
        return;
    }
    $call = 'readelf -d ' . quoteArg($file);
    open(IN, '-|', $call) or die "cannot $call";
    while (<IN>) {
        if (m'\(NEEDED\).*\[([^]]*)\]') {
            $s = $1;
            if ($s =~ '/') {
                print STDERR "$file: bad NEEDED '$s'\n";
                exit(1);
            }
        } elsif (m'\(RPATH\).*\[([^]]*)\]') {
            $s = $1;
            if ($s ne '$ORIGIN' and $1 ne '$ORIGIN/../../program') {
                print STDERR "$file: bad RPATH '$s'\n";
                exit(1);
            }
        }
    }
    close(IN) or die "got $? from $call";
}

sub visitDir {
    my ($dir) = @_;
    opendir my $handle, $dir or die "cannot opendir $dir";
    while (my $file = readdir $handle) {
        next if $file eq '.' or $file eq '..';
        my $path = "$dir/$file";
        if (-f $path) {
            visitFile("$path");
        } elsif (-d $path) {
            visitDir("$path");
        }
    }
    close $handle;
}

$instdir = $ENV{'INSTDIR'} or die 'missing INSTDIR';
visitDir("$instdir");
