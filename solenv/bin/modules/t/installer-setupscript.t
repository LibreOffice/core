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

BEGIN {
    use_ok('installer::setupscript', qw(
        add_forced_properties
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
