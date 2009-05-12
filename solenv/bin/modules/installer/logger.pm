#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: logger.pm,v $
#
# $Revision: 1.10 $
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

package installer::logger;

use installer::files;
use installer::globals;

####################################################
# Including header files into the logfile
####################################################

sub include_header_into_logfile
{
    my ($message) = @_;

    my $infoline;

    $infoline = "\n" . get_time_string();
    push( @installer::globals::logfileinfo, $infoline);

    $infoline = "######################################################\n";
    push( @installer::globals::logfileinfo, $infoline);

    $infoline = "$message\n";
    push( @installer::globals::logfileinfo, $infoline);


    $infoline = "######################################################\n";
    push( @installer::globals::logfileinfo, $infoline);
}

####################################################
# Including header files into the logfile
####################################################

sub include_header_into_globallogfile
{
    my ($message) = @_;

    my $infoline;

    $infoline = "\n" . get_time_string();
    push( @installer::globals::globallogfileinfo, $infoline);

    $infoline = "######################################################\n";
    push( @installer::globals::globallogfileinfo, $infoline);

    $infoline = "$message\n";
    push( @installer::globals::globallogfileinfo, $infoline);


    $infoline = "######################################################\n";
    push( @installer::globals::globallogfileinfo, $infoline);
}

####################################################
# Write timestamp into log file
####################################################

sub include_timestamp_into_logfile
{
    my ($message) = @_;

    my $infoline;
    my $timestring = get_time_string();
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

    $infoline = "\n" . get_time_string();
    push( @installer::globals::globallogfileinfo, $infoline);

    $infoline = "################################################################\n";
    push( @installer::globals::globallogfileinfo, $infoline);

    $infoline = "$message\n";
    push( @installer::globals::globallogfileinfo, $infoline);

    $infoline = "################################################################\n";
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
# For each product (new language) a new log file is created.
# Therefore the global logging has to be saved in this file.
###############################################################

sub debuginfo
{
    my  ( $message ) = @_;

    $message = $message . "\n";
    push(@installer::globals::functioncalls, $message);
}

###############################################################
# Saving the debug information.
###############################################################

sub savedebug
{
    my ( $outputdir ) = @_;

    installer::files::save_file($outputdir . $installer::globals::debugfilename, \@installer::globals::functioncalls);
    print_message( "... writing debug file " . $outputdir . $installer::globals::debugfilename . "\n" );
}

###############################################################
# Starting the time
###############################################################

sub starttime
{
    $installer::globals::starttime = time();
}

###############################################################
# Convert time string
###############################################################

sub convert_timestring
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

sub get_time_string
{
    my $currenttime = time();
    $currenttime = $currenttime - $installer::globals::starttime;
    $currenttime = convert_timestring($currenttime);
    $currenttime = localtime() . " \(" . $currenttime . "\)\n";
    return $currenttime;
}

###############################################################
# Returning the age of a file (in seconds)
###############################################################

sub get_file_age
{
    my ( $filename ) = @_;

    my $filetime = (stat($filename))[9];
    my $timediff = time() - $filetime;
    return $timediff;
}

###############################################################
# Stopping the time
###############################################################

sub stoptime
{
    my $infoline = get_time_string();
    print_message( "$infoline" );
}

###############################################################
# Set date string, format: yymmdd
###############################################################

sub set_installation_date
{
    my $datestring = "";

    my @timearray = localtime(time);

    my $day = $timearray[3];
    my $month = $timearray[4] + 1;
    my $year = $timearray[5] - 100;

    if ( $year < 10 ) { $year = "0" . $year; }
    if ( $month < 10 ) { $month = "0" . $month; }
    if ( $day < 10 ) { $day = "0" . $day; }

    $datestring = $year . $month . $day;

    return $datestring;
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

sub print_message_without_newline
{
    my $message = shift;
    chomp $message;
    print "$message" if ( ! $installer::globals::quiet );
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
