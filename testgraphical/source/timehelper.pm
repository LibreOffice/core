package timehelper;

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

use POSIX qw(strftime);
use POSIX qw(time difftime);
# use POSIX qw(localtime);
use strict;
# use Time::localtime;
use loghelper;

BEGIN {
    use Exporter   ();
    our ($VERSION, @ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS);

    $VERSION     = 1.00;
    # if using RCS/CVS, this may be preferred
    $VERSION = do { my @r = (q$Revision: 1.1 $ =~ /\d+/g); sprintf "%d."."%02d" x $#r, @r }; # must be all one line, for MakeMaker
    @ISA         = qw(Exporter);
    @EXPORT      = qw(&getTime &endTime &printTime &waitAMinute );
    %EXPORT_TAGS = ( );     # eg: TAG => [ qw!name1 name2! ],
    # your exported package globals go here,
    # as well as any optionally exported functions
    @EXPORT_OK   = ( ); # qw($Var1 %Hashit &func3);
}


# ------------------------------------------------------------------------------
# our $starttime;
sub getTime()
{
    my $nValue;
    # $nValue = localtime->sec();
    # $nValue += 60 * localtime->min();
    # $nValue += 3600 * localtime->hour();
    $nValue = time();
    return $nValue;
}
# sub startTime()
# {
#     $starttime = getTime();
# }
sub endTime($)
{
    my $starttime = shift;

    my $endtime = getTime();
    my $nTime = difftime($endtime, $starttime);
    # my $nTime = $endtime - $starttime;
    # if ($nTime < 0)
    # {
    #     $nTime += 24 * 3600; # add 24 hours
    # }
    return $nTime;
}
sub printTime($)
{
    my $nTime = shift;
    print( "Time: " . $nTime . " seconds.\n\n");
}


# sub waitAMinute()
# {
#     # _waitInSeconds(20);
#     # _waitInSeconds(20);
#     my $now_string = strftime "%a %b %e %H:%M:%S %Y", localtime;
#     print $now_string . "\n";
#     # print getCurrentDateString() . "\n";
#     sleep(60);
# }
#

1;
