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
    use_ok('installer::setupscript', qw(
        add_installationobject_to_variables
        add_lowercase_productname_setupscriptvariable
        add_predefined_folder
        get_all_items_from_script
        get_all_scriptvariables_from_installation_object
        replace_all_setupscriptvariables_in_script
        replace_preset_properties
        resolve_lowercase_productname_setupscriptvariable
        set_setupscript_name
    ));
}

my @folders = (
    { gid => '123' },
    { gid => 'PREDEFINED_FOO' },
    { gid => 'PREDEFINED_BAR' },
);

my @folderitems = (
    { FolderID => 'PREDEFINED_AUTOSTART' },
    { FolderID => 'PREDEFINED_BAR' },
    { FolderID => '456' },
);

my @expected_folders = (
    { gid => '123' },
    { gid => 'PREDEFINED_FOO' },
    { gid => 'PREDEFINED_BAR' },
    {
      ismultilingual => 0,
      Name => "",
      gid => 'PREDEFINED_AUTOSTART'
    },
);

add_predefined_folder(\@folderitems, \@folders);

is_deeply(\@folders, \@expected_folders);

done_testing();
