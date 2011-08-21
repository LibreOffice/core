#!/usr/bin/perl
################################################################################
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       Michael Koch <miko@gmx.ch>
#
# Major Contributor(s):
#       <name>
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
################################################################################
# Usage: "Usage: doubleNewlines.pl <directory>"
#        It is possible to enter more than one directory separated by spaces.
#        Instead of a directory you can also use one or more files as arguments.
################################################################################

use strict;
use warnings;

use File::Find;
use Cwd 'abs_path';

my $total = 0;

die "Usage: doubleNewlines.pl <directory>\n" unless (@ARGV);

# if path is relative, make it absolute
foreach (@ARGV){
    $_ = abs_path($_);
}

print "Following code files (.hxx and .cxx) are suspicious:\n";
find(\&processFile, @ARGV);     # processes all files in dir and subdirs
print "Found $total suspicious files.\n";

sub processFile {
    my $file = $File::Find::name;
    return unless $file =~ /(.cxx$)|(.hxx$)/;
    open FILE, $file or die "Can't open '$file': $!";
    my $lines = join '', <FILE>;
    if ($lines =~ /(\n{2,}.+){10}/) {    # ten consecutive occurrences of [empty line(s) - code line)]
        my $relPath = substr($file, index($file, "clone")+6);   # relative path beginning with repo name
        print "$relPath\n";
        $total++;
    }
}
