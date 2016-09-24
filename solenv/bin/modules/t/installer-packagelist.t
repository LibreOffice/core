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

BEGIN {
    use_ok('installer::packagelist');
}

my @packagemodules = (
    { allmodules => [qw(a b c d)] },
    { allmodules => [qw(a b c)] },
    { allmodules => [qw(e f g)] },
    { allmodules => [qw(h)] },
    { allmodules => [qw(a b g)] },
);

my @expected_packagemodules = (
    { allmodules => [qw(d)] },
    { allmodules => [qw(c)] },
    { allmodules => [qw(e f)] },
    { allmodules => [qw(h)] },
    { allmodules => [qw(a b g)] },
);

installer::packagelist::remove_multiple_modules_packages(\@packagemodules);

is_deeply(\@packagemodules, \@expected_packagemodules);

done_testing();
