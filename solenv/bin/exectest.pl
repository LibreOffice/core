#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

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
        open PIPE, "| @ARGV" or die "cannot start process: $!";
        $open = 1;
    }
    elsif ($open && $input)
    {
        print PIPE $in or die "cannot write to pipe: $!";
    }
}
exit $failed;
