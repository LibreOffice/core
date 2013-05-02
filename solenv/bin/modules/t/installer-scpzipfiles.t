#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

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
