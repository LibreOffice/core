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



package installer::logger;

use installer::files;
use installer::globals;
use Time::HiRes qw(gettimeofday tv_interval);
use English;
use IO::Handle;
use strict;

my $StartTime = undef;

sub PrintStackTrace ();
sub Die ($);

=head1 NAME

    installer::logger

    Logging for the installer modules.

=cut

=head1 DESCRIPTION

    This module is in a transition state from a set of loosly connected functions to a single class.

    There are three globaly available logger objects:

=over

=item $Lang

    is language specific and writes messages to a log file.

=cut

=item $Glob

    is independent of the current language.  Its messages are prepended to each $Lang logger.

=cut

=item $Info

    is for output to the console.

=cut

=back

=cut


our $Global = installer::logger->new("glob",
    'is_save_lines' => 1,
    'is_print_to_console' => 0,
    'is_show_relative_time' => 1);
our $Lang = installer::logger->new("lang",
    'is_print_to_console' => 0,
    'is_show_relative_time' => 1,
    'is_show_log_id' => 1
    );
our $Info = installer::logger->new("info",
    'is_show_relative_time' => 0,
    'is_show_process_id' => 0,
    'is_show_log_id' => 0
    );



=head2 SetupSimpleLogging ($filename)

    Setup logging so that $Global, $Lang and $Info all print to the console.
    If $filename is given then logging also goes to that file.

=cut
sub SetupSimpleLogging (;$)
{
    my ($log_filename) = @_;

    $Info = installer::logger->new("info",
        'is_print_to_console' => 1,
        'is_show_relative_time' => 1,
        );
    $Global = installer::logger->new("glob",
        'is_print_to_console' => 0,
        'is_show_relative_time' => 1,
        'forward' => [$Info]
        );
    $Lang = installer::logger->new("lang",
        'is_print_to_console' => 0,
        'is_show_relative_time' => 1,
        'forward' => [$Info]
        );
    if (defined $log_filename)
    {
        $Info->set_filename($log_filename);
    }
    $Info->{'is_print_to_console'} = 1;
    $installer::globals::quiet = 0;
    starttime();
}




=head2 new($class, $id, @arguments)

    Create a new instance of the logger class.
    @arguments lets you override default values.

=cut

sub new ($$@)
{
    my ($class, $id, @arguments) = @_;

    my $self = {
        'id' => $id,
        'filename' => "",
        # When set then lines are printed to this file.
        'file' => undef,
        # When true then lines are printed to the console.
        'is_print_to_console' => 1,
        'is_save_lines' => 0,
        # A container of printed lines.  Lines are added only when 'is_save_lines' is true.
        'lines' => [],
        # Another logger to which all prints are forwarded.
        'forward' => [],
        # A filter function that for example can recoginze build errors.
        'filter' => undef,
        # Show relative time
        'is_show_relative_time' => 0,
        # Show log id (mostly for debugging the logger)
        'is_show_log_id' => 0,
        # Show the process id, useful on the console when doing a multiprocessor build.
        'is_show_process_id' => 0,
        # Current indentation
        'indentation' => "",
    };
    while (scalar @arguments >= 2)
    {
        my $key = shift @arguments;
        my $value = shift @arguments;
        $self->{$key} = $value;
    }

    bless($self, $class);

    return $self;
}



=head2 printf($self, $message, @arguments)

    Identical in syntax and semantics to the usual perl (s)printf.

=cut
sub printf ($$@)
{
    my ($self, $format, @arguments) = @_;

    if ($format =~ /\%\{/)
    {
        printf(">%s<\n", $format);
        PrintStackTrace();
    }
    my $message = sprintf($format, @arguments);
    $self->print($message, 0);
}




=head2 print ($self, $message, [optional] $force)

    Print the given message.
    If the optional $force parameter is given and it evaluates to true then the message
    is printed even when the golbal $installer::globals::quiet is true.

=cut
sub print ($$;$)
{
    my ($self, $message, $force) = @_;

    Die "newline at start of line" if ($message =~ /^\n.+/);

    $force = 0 unless defined $force;

    my $relative_time = tv_interval($StartTime, [gettimeofday()]);
    foreach my $target ($self, @{$self->{'forward'}})
    {
        $target->process_line(
            $relative_time,
            $self->{'id'},
            $PID,
            $message,
            $force);
    }
}




=head2 process_line ($self, $relative_time, $log_id, $pid, $message, $force)

    Internal function that decides whether to
    a) write to a log file
    b) print to the console
    c) store in an array for later use
    the preformatted message.

=cut
sub process_line ($$$$$$)
{
    my ($self, $relative_time, $log_id, $pid, $message, $force) = @_;

    # Apply the line filter.
    if (defined $self->{'filter'})
    {
        $message = &{$self->{'filter'}}($relative_time, $log_id, $pid, $message);
    }

    # Format the line.
    my $line = "";
    if ($self->{'is_show_relative_time'})
    {
        $line .= sprintf("%12.6f : ", $relative_time);
    }
    if ($self->{'is_show_log_id'})
    {
        $line .= $log_id . " : ";
    }
    if ($self->{'is_show_process_id'})
    {
        $line .= $pid . " : ";
    }
    $line .= $self->{'indentation'};
    $line .= $message;

    # Print the line to a file or to the console or store it for later use.
    my $fid = $self->{'file'};
    if (defined $fid)
    {
        print $fid ($line);
    }
    if (($force || ! $installer::globals::quiet)
        && $self->{'is_print_to_console'})
    {
        print($line);
    }
    if ($self->{'is_save_lines'})
    {
        push @{$self->{'lines'}}, [$relative_time, $log_id, $pid, $message, $force];
    }
}




=head2 set_filename (Self, $filename)

    When the name of a writable file is given then all future messages will go to that file.
    Output to the console is turned off.
    This method is typically used to tie the language dependent $Lang logger to different log files.

=cut
sub set_filename ($$)
{
    my ($self, $filename) = @_;

    $filename = "" unless defined $filename;
    if ($self->{'filename'} ne $filename)
    {
        if (defined $self->{'file'})
        {
            $self->{'is_print_to_console'} = 1;
            close $self->{'file'};
            $self->{'file'} = undef;
        }

        $self->{'filename'} = $filename;

        if ($filename ne "")
        {
            open $self->{'file'}, ">", $self->{'filename'}
            || Die "can not open log file ".$self->{'filename'}." for writing";
            $self->{'is_print_to_console'} = 0;

            # Make all writes synchronous so that we don't loose any messages on an
            # 'abrupt' end.
            my $handle = select $self->{'file'};
            $| = 1;
            select $handle;
        }
    }
}




=head2 set_filter ($self, $filter)

    Sets $filter (a function reference) as line filter.  It is applied to each line.
    The filter can extract information from the given message and modify it before it is printed.

=cut
sub set_filter ($$)
{
    my ($self, $filter) = @_;
    $self->{'filter'} = $filter;
}




=head2 add_timestamp ($self, $message)

    Print the given message together with the current (absolute) time.

=cut
sub add_timestamp ($$)
{
    my ($self, $message) = @_;

    my $timestring = get_time_string();
    $self->printf("%s\t%s", $message, $timestring);
}



=head2 copy_lines_from ($self, $other)

    Copy saved lines from another logger object.

=cut
sub copy_lines_from ($$)
{
    my ($self, $other) = @_;

    my $is_print_to_console = $self->{'is_print_to_console'};
    my $is_save_lines = $self->{'is_save_lines'};
    my $fid = $self->{'file'};

    foreach my $line (@{$other->{'lines'}})
    {
        $self->process_line(@$line);
    }
}




=head2 set_forward ($self, $other)

    Set a forwarding target.  All future messages are forwarded (copied) to $other.
    A typical use is to tie $Info to $Lang so that all messages sent to $Info are
    printed to the console AND written to the log file.

=cut
sub set_forward ($$)
{
    my ($self, $other) = @_;

    # At the moment at most one forward target is allowed.
    if (defined $other)
    {
        $self->{'forward'} = [$other];
    }
    else
    {
        $self->{'forward'} = [];
    }
}




sub increase_indentation ($)
{
    my ($self) = @_;
    $self->{'indentation'} .= "    ";
}




sub decrease_indentation ($)
{
    my ($self) = @_;
    $self->{'indentation'} = substr($self->{'indentation'}, 4);
}




####################################################
# Including header files into the logfile
####################################################

sub include_header_into_logfile
{
    my ($message) = @_;

    $Lang->print("\n");
    $Lang->print(get_time_string());
    $Lang->print("######################################################\n");
    $Lang->print($message."\n");
    $Lang->print("######################################################\n");
}

####################################################
# Including header files into the logfile
####################################################

sub include_header_into_globallogfile
{
    my ($message) = @_;

    $Global->print("\n");
    $Global->print(get_time_string());
    $Global->print("######################################################\n");
    $Global->print($message."\n");
    $Global->print("######################################################\n");
}

####################################################
# Write timestamp into log file
####################################################

sub include_timestamp_into_logfile
{
    Die "deprected";
    my ($message) = @_;

    my $infoline;
    my $timestring = get_time_string();
    $Lang->printf("%s\t%s", $message, $timestring);
}

####################################################
# Writing all variables content into the log file
####################################################

sub log_hashref
{
    my ($hashref) = @_;

    $Global->print("\n");
    $Global->print("Logging variable settings:\n");

    my $itemkey;

    foreach $itemkey ( keys %{$hashref} )
    {
        my $line = "";
        my $itemvalue = "";
        if ( $hashref->{$itemkey} ) { $itemvalue = $hashref->{$itemkey}; }
        $Global->printf("%s=%s\n", $itemkey, $itemvalue);
    }

    $Global->print("\n");
}

#########################################################
# Including global logging info into global log array
#########################################################

sub globallog
{
    my ($message) = @_;

    my $infoline;

    $Global->print("\n");
    $Global->print(get_time_string());
    $Global->print("################################################################\n");
    $Global->print($message."\n");
    $Global->print("################################################################\n");
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
    $StartTime = [gettimeofday()];

    my $localtime = localtime();
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
    my $localtime = localtime();
    $Info->printf("stopping log at %s\n", $localtime);
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
    Die "print_message is deprecated";

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

    PrintError($message);

    print STDERR "\n";
    print STDERR "**************************************************\n";
    print STDERR "ERROR: $message";
    print STDERR "\n";
    print STDERR "**************************************************\n";
    return;
}




sub PrintError ($@)
{
    my ($format, @arguments) = @_;

    $Info->printf("Error: ".$format, @arguments);
}




=head2 PrintStackTrace()
    This is for debugging the print and printf methods of the logger class and their use.
    Therefore we use the Perl print/printf directly and not the logger methods to avoid loops in case of errors.
=cut
sub PrintStackTrace ()
{
    print "Stack Trace:\n";
    my $i = 1;
    while ((my @call_details = (caller($i++))))
    {
        printf("%s:%s in function %s\n", $call_details[1], $call_details[2], $call_details[3]);
    }
}


sub Die ($)
{
    my ($message) = @_;
    PrintStackTrace();
    die $message;
}



1;
