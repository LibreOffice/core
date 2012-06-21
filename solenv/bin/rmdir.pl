:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
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

my $r_code=0;
if ( ! defined $ARGV[0] || "$ARGV[0]" eq "" ) {
    print STDERR "Nothing to delete\n";
    exit 1;
}
while ( defined $ARGV[0] ) {
if ( rmdir $ARGV[0] ) {
#    exit 0;
} else {
    print STDERR "ERROR removing $ARGV[0]: $!\n";
    $r_code = 1;
}
shift @ARGV;
}
exit $r_code;

