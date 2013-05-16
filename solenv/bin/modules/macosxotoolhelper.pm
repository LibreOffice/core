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

package macosxotoolhelper;
require Exporter;
our @ISA = Exporter;
our @EXPORT = otoolD;

sub otoolD($) {
    my ($file) = @_;
    my $call = "otool -D $file";
    open(IN, "-|", $call) or die "cannot $call";
    my $line = <IN>;
    $line =~ /^\Q$file\E:\n$/ or
        die "unexpected otool -D output (\"$line\", expecting \"$file:\")";
    $line = <IN>;
    <IN> == undef or die "unexpected otool -D output";
    close(IN);
    return $line;
}
