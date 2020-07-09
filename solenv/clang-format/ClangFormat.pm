# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

package ClangFormat;

use strict;
use warnings;

our @EXPORT_OK = qw(get_excludelist set_excludelist get_wanted_version get_own_directory get_extension_regex find check_style);

# Reads the excludelist.
sub get_excludelist()
{
    my $src = "c|cpp|cxx|h|hxx|inl";
    my %excludelist_names = ();

    # Read the excludelist.
    if (open(LINES, "solenv/clang-format/excludelist"))
    {
        while (my $line = <LINES>)
        {
            chomp $line;
            $excludelist_names{$line} = 1;
        }
    }

    return \%excludelist_names;
}

# Writes the excludelist.
# The single argument is a reference to an array.
sub set_excludelist
{
    my @filenames = @{$_[0]};
    open my $fh, ">", "solenv/clang-format/excludelist" or die $!;
    print $fh "$_\n" for @filenames;
    close $fh;
}

# Returns the clang-format version used of style enforcement.
sub get_wanted_version()
{
    return "5.0.0";
}

# Returns the directory that can host a binary which is used automatically, even
# if it's not in PATH.
sub get_own_directory()
{
    return "/opt/lo/bin";
}

# Returns a regex matching filenames we clang-format.
sub get_extension_regex()
{
    return "c|cpp|cxx|h|hxx|inl";
}

# Use clang-format from CLANG_FORMAT, from our dedicated directory or from
# PATH, in this order.
sub find()
{
    my $version = get_wanted_version();
    my $opt_lo = get_own_directory();
    my $clang_format;
    if (!(defined($ENV{CLANG_FORMAT}) && is_matching_clang_format_version($ENV{CLANG_FORMAT}, $version)))
    {
        my @dirs = split /:/, $ENV{PATH};
        unshift(@dirs, $opt_lo);

        foreach my $dir (@dirs)
        {
            if (is_matching_clang_format_version("$dir/clang-format", $version))
            {
                $clang_format = "$dir/clang-format";
                last;
            }
        }
    }
    else
    {
        $clang_format = $ENV{CLANG_FORMAT};
    }

    if ($^O eq "cygwin" && defined($clang_format))
    {
        $clang_format = `cygpath -m '$clang_format'`;
        chomp $clang_format;
    }

    return $clang_format;
}

# Diffs the original and the formatted version of a single file from the index.
sub check_style($$)
{
    # Make sure that not staged changes are not considered when diffing.
    my ($clang_format, $filename) = @_;
    my $index = $filename . ".index";
    system("git show :$filename > $index");
    my $format = $index . ".format";
    system("'$clang_format' -assume-filename=$filename $index > $format");
    my $ret = system("git --no-pager diff --no-index --exit-code $index $format") == 0;
    unlink($index);
    unlink($format);
    return $ret;
}

# Private functions.

# Is this binary the version we standardize on?
sub is_matching_clang_format_version($$)
{
    my ($clang_format, $version) = @_;
    if (! -x $clang_format)
    {
        return 0;
    }

    return `'$clang_format' -version` =~ /^clang-format version $version(-\d+)? \(tags/;
}

1;

# vim: set shiftwidth=4 softtabstop=4 expandtab:
