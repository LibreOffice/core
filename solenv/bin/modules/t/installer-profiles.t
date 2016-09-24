#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

use strict;
use warnings;

use Test::More;

use lib '.';

use installer::profiles;

my @input = map { "$_\n" } split "\n", <<'END';
  [foo]
1
NOT SEEN
  [bar]
3
  [foo]
2
[bar]
4
END

my @expected = map { "$_\n" } split "\n", <<'END';
[foo]
1
2
[bar]
3
4
END

my $result = installer::profiles::sorting_profile(\@input);

is_deeply($result, \@expected);

done_testing();
