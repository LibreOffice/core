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

BEGIN {
    use_ok('packager::work', qw(
           set_global_variable
           create_package_todos
           execute_system_calls
    ));
}

$packager::globals::compiler = 'gcc';
@packager::globals::logfileinfo = ();

my $packagelist = <<'END';
   # Comment
   some_product  gcc,gcc3.3  en_US,en_GB|fr_FR  some_target
   other_thing   x           y                  z
END

my @packagelist = split "\n", $packagelist;

my $targets = create_package_todos( \@packagelist );

is_deeply(\@packager::globals::logfileinfo,
          ["some_target_en_US_en_GB\n",
           "some_target_fr_FR\n"]);

is_deeply($targets,
          ["some_target_en_US_en_GB",
           "some_target_fr_FR"]);

done_testing();
