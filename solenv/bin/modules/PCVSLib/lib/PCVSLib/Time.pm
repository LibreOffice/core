#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: Time.pm,v $
#
# $Revision: 1.3 $
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


#
# Time.pm - package for converting time strings to seconds since epoch and back
#

package PCVSLib::Time;

use Carp;
use Time::Local;

use strict;
use warnings;

my %months = (Jan => 0, Feb => 1, Mar => 2, Apr => 3, May => 4, Jun => 5, Jul => 6,
              Aug => 7, Sep => 8, Oct => 9, Nov => 10, Dec => 11);

sub rfc822_1123_to_seconds
{
    my $str = shift;

    # CVS claims to use RFC 822 as modified by RFC 1123 time strings. This
    # seems not to be entirely true. One tested (older) CVS server sent
    # the following Mod-Time response:
    #
    # Mod-time 2 Aug 2005 14:4:56 -0000
    #
    # The minutes in this string are certainly not specified due to RFC 822/1123
    # way. Use a somewhat relaxed parsing of the time part of the string.
    if ( $str !~ /^(\d{1,2})\s+(\w\w\w)\s+(\d\d\d\d)\s+(\d{1,2}):(\d{1,2}):(\d{1,2})\s+([\+\-])(\d\d)(\d\d)/ )
    {
        return undef;
    }

    my $mday = $1;
    my $mon  = $months{$2};
    if ( !defined($mon) ) {
        croak("PCVSLIB::Time::rfc822_1123_to_seconds(): invalid time: $str");
    }
    my $year      = $3;
    my $hour      = $4;
    my $min       = $5;
    my $sec       = $6;
    my $tz_sign   = $7;
    my $tz_hours  = $8;
    my $tz_mins   = $9;

    my $offset_sign = ($tz_sign eq '+') ? +1 : -1;
    my $offset_secs = $offset_sign * ($tz_hours*3600 + $tz_mins*60);

    return (timegm($sec, $min, $hour, $mday, $mon, $year) - $offset_secs);
}

sub timestr_to_seconds
{
    my $str  = shift;

    if ( $str !~ /^(\w\w\w)\s+(\w\w\w)\s+(\d{1,2})\s+(\d\d):(\d\d):(\d\d)\s+(\d\d\d\d)$/ ) {
        return undef;
    }
    my $mon  = $months{$2};
    if ( !defined($mon) ) {
        croak("PCVSLIB::Entry::timestr_to_seconds(): invalid time: $str");
    }
    my $mday = $3;
    my $hour = $4;
    my $min  = $5;
    my $sec  = $6;
    my $year = $7;

    return timegm($sec, $min, $hour, $mday, $mon, $year);
}

sub seconds_to_timestr
{
   my $seconds = shift;

   my $str = gmtime($seconds);
   return $str;
}

sub seconds_to_rfc822_1123
{
   my $seconds = shift;

   my $str = gmtime($seconds);

   $str =~ /^(\w\w\w)\s+(\w\w\w)\s+(\d{1,2})\s+(\d\d):(\d\d):(\d\d)\s+(\d\d\d\d)$/;

   return "$3 $2 $7 $4:$5:$6 +0000";
}

1;
# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:
