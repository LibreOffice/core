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

package installer::logger;

use strict;
use warnings;

use base 'Exporter';

use installer::files;
use installer::globals;

our @EXPORT_OK = qw(
    include_header_into_logfile
    include_timestamp_into_logfile
    log_hashref
    globallog
    copy_globalinfo_into_logfile
    starttime
    stoptime
    print_message
    print_warning
    print_error
);

my $starttime;

####################################################
# Including header files into the logfile
####################################################

sub include_header_into_logfile
{
    my ($message) = @_;

    my $infoline;

    $infoline = "\n" . _get_time_string();
    push( @installer::globals::logfileinfo, $infoline);

    $infoline = "######################################################\n";
    push( @installer::globals::logfileinfo, $infoline);

    $infoline = "$message\n";
    push( @installer::globals::logfileinfo, $infoline);


    $infoline = "######################################################\n";
    push( @installer::globals::logfileinfo, $infoline);
}

####################################################
# Write timestamp into log file
####################################################

sub include_timestamp_into_logfile
{
    my ($message) = @_;

    my $infoline;
    my $timestring = _get_time_string();
    $infoline = "$message\t$timestring";
    push( @installer::globals::logfileinfo, $infoline);
}

####################################################
# Writing all variables content into the log file
####################################################

sub log_hashref
{
    my ($hashref) = @_;

    my $infoline = "\nLogging variable settings:\n";
    push(@installer::globals::globallogfileinfo, $infoline);

    my $itemkey;

    foreach $itemkey ( keys %{$hashref} )
    {
        my $line = "";
        my $itemvalue = "";
        if ( $hashref->{$itemkey} ) { $itemvalue = $hashref->{$itemkey}; }
        $line = $itemkey . "=" . $itemvalue . "\n";
        push(@installer::globals::globallogfileinfo, $line);
    }

    $infoline = "\n";
    push(@installer::globals::globallogfileinfo, $infoline);
}

#########################################################
# Including global logging info into global log array
#########################################################

sub globallog
{
    my ($message) = @_;

    my $infoline;

    $infoline = "\n" . _get_time_string();
    push( @installer::globals::globallogfileinfo, $infoline);

    $infoline = "######################################################\n";
    push( @installer::globals::globallogfileinfo, $infoline);

    $infoline = "$message\n";
    push( @installer::globals::globallogfileinfo, $infoline);

    $infoline = "######################################################\n";
    push( @installer::globals::globallogfileinfo, $infoline);

}

###############################################################
# For each product (new language) a new log file is created.
# Therefore the global logging has to be saved in this file.
###############################################################

sub copy_globalinfo_into_logfile
{
    for ( my $i = 0; $i <= $#installer::globals::globallogfileinfo; $i++ )
    {
        push(@installer::globals::logfileinfo, $installer::globals::globallogfileinfo[$i]);
    }
}

###############################################################
# Starting the time
###############################################################

sub starttime
{
    $starttime = time();
}

###############################################################
# Convert time string
###############################################################

sub _convert_timestring
{
    my ($secondstring) = @_;

    my $timestring = "";

    if ( $secondstring < 60 )    # less than a minute
    {
        if ( $secondstring < 10 ) { $secondstring = "0" . $secondstring; }
        $timestring = "00\:$secondstring min\.";
    }
    elsif ( $secondstring < 3600 )
    {
        my $minutes = $secondstring / 60;
        my $seconds = $secondstring % 60;
        if ( $minutes =~ /(\d*)\.\d*/ ) { $minutes = $1; }
        if ( $minutes < 10 ) { $minutes = "0" . $minutes; }
        if ( $seconds < 10 ) { $seconds = "0" . $seconds; }
        $timestring = "$minutes\:$seconds min\.";
    }
    else    # more than one hour
    {
        my $hours = $secondstring / 3600;
        my $secondstring = $secondstring % 3600;
        my $minutes = $secondstring / 60;
        my $seconds = $secondstring % 60;
        if ( $hours =~ /(\d*)\.\d*/ ) { $hours = $1; }
        if ( $minutes =~ /(\d*)\.\d*/ ) { $minutes = $1; }
        if ( $hours < 10 ) { $hours = "0" . $hours; }
        if ( $minutes < 10 ) { $minutes = "0" . $minutes; }
        if ( $seconds < 10 ) { $seconds = "0" . $seconds; }
        $timestring = "$hours\:$minutes\:$seconds hours";
    }

    return $timestring;
}

###############################################################
# Returning time string for logging
###############################################################

sub _get_time_string
{
    my $currenttime = time();
    $currenttime = $currenttime - $starttime;
    $currenttime = _convert_timestring($currenttime);
    $currenttime = localtime() . " \(" . $currenttime . "\)\n";
    return $currenttime;
}

###############################################################
# Stopping the time
###############################################################

sub stoptime
{
    print_message( _get_time_string() );
}

###############################################################
# Console output: messages
###############################################################

sub print_message
{
    my $message = shift;
    chomp $message;
    my $force = shift || 0;
    print "$message\n" if ( $force || ! $installer::globals::quiet );
    return;
}

###############################################################
# Console output: warnings
###############################################################

sub print_warning
{
    my $message = shift;
    chomp $message;
    print STDERR "WARNING: $message";
    return;
}

###############################################################
# Console output: errors
###############################################################

sub print_error
{
    my $message = shift;
    chomp $message;
    print STDERR "\n**************************************************\n";
    print STDERR "ERROR: $message";
    print STDERR "\n**************************************************\n";
    return;
}

1;
