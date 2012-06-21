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

package pre2par::directory;

use pre2par::pathanalyzer;
use pre2par::systemactions;

############################################
# Checking, whether the output directories
# exist. If not, they are created.
############################################

sub check_directory
{
    my ($parfilename) = @_;

    my $productdirectory = $parfilename;
    pre2par::pathanalyzer::get_path_from_fullqualifiedname(\$productdirectory);
    $productdirectory =~ s/\Q$pre2par::globals::separator\E\s*$//;

    my $pardirectory = $productdirectory;
    pre2par::pathanalyzer::get_path_from_fullqualifiedname(\$pardirectory);
    $pardirectory =~ s/\Q$pre2par::globals::separator\E\s*$//;

    if ( ! -d $pardirectory ) { pre2par::systemactions::create_directory($pardirectory); }
    if ( ! -d $productdirectory ) { pre2par::systemactions::create_directory($productdirectory); }
}

1;