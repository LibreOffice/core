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

#
# checkit - a perl script to check the output of 'dircmp'
#

$return = 0;
$possible_error = 0;
$possible_error_descript = "";
while( <STDIN> )
{
    if( /^diff/ )
    {
        $possible_error = 1;
        $possible_error_descript = $_;
    }
    elsif( /^Binary/ )
    {
        print STDERR "ERROR : $_";
        $return++;
    }
    elsif( /^[0-9]/ && $possible_error == 1 )
    {
        print STDERR "ERROR : diff ".$possible_error_descript;
        $return++;
        $possible_error = 0;
    }
    else
    {
        $possible_error = 0;
    }
}
if( $return != 0 )
{
    unlink "$ARGV[0]";
}
exit $return;
