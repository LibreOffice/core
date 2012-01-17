package timehelper;

#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************



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
