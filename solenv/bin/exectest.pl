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

sub encode($)
{
    my ($arg) = @_;
    $arg =~ s/'/'\\''/g;
    return $arg
}

$#ARGV >= 2
    or die "Usage: $0 <input file> <temp file> <command> <arguments...>";
open INPUT, '<', $ARGV[0] or die "cannot open $ARGV[0]: $!";
shift @ARGV;
$temp = $ARGV[0];
shift @ARGV;
$failed = 0;
$open = 0;
while (1) {
    $eof = eof INPUT;
    $in = <INPUT> unless $eof;
    if ($eof || $in =~ /^EXPECT (SUCCESS|FAILURE) "([^"]*)"?:\n$/)
    {
        if ($open)
        {
            close OUTPUT;
            my $prog = '';
            my $assigns = 1;
            for ($i = 0; $i != scalar(@ARGV); ++$i)
            {
                $prog .= ' ' unless $i == 0;
                if ($assigns && $ARGV[$i] =~ /^([A-Za-z_][A-Za-z0-9_]+)=(.*)$/)
                {
                    $prog .= $1 . "='" . encode($2) . "'";
                }
                else
                {
                    if ($ARGV[$i] =~ /^{}$/)
                    {
                        $prog .= "'" . encode($temp) . "'";
                    }
                    else
                    {
                        $prog .= "'" . encode($ARGV[$i]) . "'";
                    }
                    $assigns = 0;
                }
            }
            system("$prog");
            unlink $temp;
            if ($? % 256 == 0)
            {
                $exit = $? / 256;
                $ok = ($? == 0) == ($expect eq "SUCCESS");
            }
            else
            {
                $exit = "signal";
                $ok = 0;
            }
            print "\"$title\" expected $expect, got $exit ($?): ";
            if ($ok)
            {
                print "ok\n";
            }
            else
            {
                print "FAILED!\n";
                exit(1);
            }
        }
        last if $eof;
        $expect = $1;
        $title = $2;
        open OUTPUT, '>', $temp or die "cannot open $temp: $!";
        $open = 1;
    }
    elsif ($open)
    {
        print OUTPUT $in or die "cannot write to $temp: $!";
    }
}
exit(0);
