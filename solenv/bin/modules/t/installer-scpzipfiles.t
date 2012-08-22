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
# Major Contributor(s):
# [ Copyright (C) 2012 Tim Retout <tim@retout.co.uk> (initial developer) ]
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

use strict;
use warnings;

use lib '.';

use Test::More;

use installer::scpzipfiles;

my $vars = { foo => "bar" };

my %lines;
my $i = 0;
while (<DATA>) {
	push @{ $lines{$i++ % 3} }, $_;
}

my @file1 = @{ $lines{0} };
my @file2 = @{ $lines{0} };

# FIXME - Some of the files acted on by these methods contain variables
# of the form "${foo}" which are currently ignored - but if "foo" was ever
# added to the product list variables, they would suddenly start to be
# replaced.
#
# We ought to come up with a better escaping mechanism, and change those
# files to use it...

installer::scpzipfiles::replace_all_ziplistvariables_in_file(\@file1, $vars);
installer::scpzipfiles::replace_all_ziplistvariables_in_rtffile(\@file2, $vars);

is_deeply(\@file1, $lines{1}, 'replace_all_ziplistvariables_in_file works');
is_deeply(\@file2, $lines{2}, 'replace_all_ziplistvariables_in_rtffile works');

done_testing();

__DATA__
This is a test
This is a test
This is a test
A test of ${foo} replacement ${foo} but not ${bar}.
A test of bar replacement bar but not ${bar}.
A test of ${foo} replacement ${foo} but not ${bar}.
A test of RTF $\{foo\} replacement $\{foo\} but not $\{bar\} or ${bar}.
A test of RTF $\{foo\} replacement $\{foo\} but not $\{bar\} or ${bar}.
A test of RTF bar replacement bar but not $\{bar\} or ${bar}.
