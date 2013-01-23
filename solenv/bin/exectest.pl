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

$#ARGV >= 1
    or die "Usage: $0 <input file>|-SUCCESS|-FAILURE <command> <arguments...>";
if ($ARGV[0] eq "-SUCCESS")
{
    $expect = "SUCCESS";
    $input = 0;
}
elsif ($ARGV[0] eq "-FAILURE")
{
    $expect = "FAILURE";
    $input = 0;
}
else
{
    open INPUT, $ARGV[0] or die "cannot open $ARGV[0]: $!";
    $input = 1;
}
shift @ARGV;
$failed = 0;
$open = 0;
while (1) {
    $eof = $input ? eof INPUT : $open;
    $in = <INPUT> if $input && !$eof;
    if (!$input || $eof
        || $in =~ /^EXPECT (SUCCESS|FAILURE|\d+)( "([^"]*)")?:\n$/)
    {
        if ($open)
        {
            close PIPE;
            if ($? % 256 == 0)
            {
                $exit = $? / 256;
                $ok = $expect eq "SUCCESS" ? $exit == 0
                    : $expect eq "FAILURE" ? $exit != 0 : $exit == $expect;
            }
            else
            {
                $exit = "signal";
                $ok = 0;
            }
            print "\"$title\", " if defined $title;
            print "expected $expect, got $exit ($?): ";
            if ($ok)
            {
                print "ok\n";
            }
            else
            {
                print "FAILED!\n";
                $failed = 1;
            }
        }
        last if $eof;
        $expect = $1 if $input;
        if (defined $3)
        {
            $title = $3;
        }
        else
        {
            undef $title;
        }
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
                $prog .= "'" . encode($ARGV[$i]) . "'";
                $assigns = 0;
            }
        }
        open PIPE, "| $prog" or die "cannot start process: $!";
        $open = 1;
    }
    elsif ($open && $input)
    {
        print PIPE $in or die "cannot write to pipe: $!";
    }
}
exit $failed;
