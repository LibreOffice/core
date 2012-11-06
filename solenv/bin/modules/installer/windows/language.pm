#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

package installer::windows::language;

use installer::exiter;

####################################################
# Determining the Windows language (LCID)
# English: 1033
####################################################

sub get_windows_language
{
    my ($language) = @_;

    my $windowslanguage = "";

    if ( $installer::globals::msilanguage->{$language} ) { $windowslanguage = $installer::globals::msilanguage->{$language}; }

    if ( $windowslanguage eq "" ) { installer::exiter::exit_program("ERROR: Unknown language $language in function get_windows_language", "get_windows_language"); }

    return $windowslanguage;
}

1;
