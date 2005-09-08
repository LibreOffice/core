#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: logger.pm,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 09:05:05 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
# Stopping the time
###############################################################

sub stoptime
{
    my $infoline = get_time_string();
    print_message( "$infoline" );
}

###############################################################
# Console output: messages
###############################################################

sub print_message
{
    my $message = shift;
    chomp $message;
    print "$message\n" if ( ! $installer::globals::quiet );
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
