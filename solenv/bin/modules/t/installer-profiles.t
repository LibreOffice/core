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
