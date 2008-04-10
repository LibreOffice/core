:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: testlog.pl,v $
#
# $Revision: 1.6 $
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

### globals ###

$is_debug = 0;

$global_log = "log.dat";

%logfiles_list = ("writer", "swlog.dat",
                "math", "smalog.dat",
                "HTML", "shptlog.dat",
                "draw", "sdrwlog.dat",
                "impress", "sdlog.dat",
                "calc", "sclog.dat",
                "chart", "schlog.dat",
                "Java", "javalog.dat",
                "Database", "dblog.dat",
                "Extension", "extlog.dat"
);

%log = ();
@ApplicationLog = ();
$dont_kill ="dont_deinstall";
$error_str = "error";
$tests_complete = "---";
$gui = $ENV{GUI};
$inpath = $ENV{INPATH};
$cygwin = "cygwin";

if ($^O =~ /cygwin/) {
        $gui = $cygwin;
}

if (($gui eq "UNX") or ($gui eq $cygwin)) {
    $pathslash = "/";
}
else
{
    $pathslash = "\\";
}

$misc = $inpath . $pathslash . "misc" . $pathslash;
$okfile = $misc . "ok.bat";
umask (02);

### sub routines ###

sub test_logfile {
    my ($file, $testname) = @_;
    my ($line, $failed, $complete, $linecount, $LastLineError);

    $failed = 0;
    $complete = 0;
    $linecount = -1;
    $LastLineError = 0;

    if (! -e $file) {
        print "error: $testname failed! Logfile is missing.\n" if $is_debug;

        return (0,0);
    }

    open TABLE, "<$file" or die "Error: can´t open log file $file]";

    while(<TABLE>) {
        $line = $_;
        chomp $line;
        $linecount++;
        if ( $line =~ /$error_str/ ) {
             print "error: $testname: $line\n";
             $failed = 1;
             $LastLineError = 1;
        }
        elsif ( $line =~ /$tests_complete/ ) {
             $complete = 1;
             print "$file: $line\n" if $is_debug;
        }
        else
        {
             print "$testname: $line\n" if $is_debug;
             $LastLineError = 0;
        }
    }

    close TABLE;

    print "$failed $complete $LastLineError $linecount\n" if $is_debug;

    if (!$complete) {
        my $message = "error: $testname: the test was not complete!";
        if ((!$failed) || (($failed) && (!$LastLineError))) {
            my $errormessage = getLog ($testname, $linecount+1);
            if ($errormessage ne "") {
                $message .= " $errormessage possibly failed!";
            }
        }
        if (!$failed && !$is_testerror) {
            print "$message\n";
        }
    }

    if (!$failed && $complete) {
        print "true\n" if $is_debug;
        return (1,1);
    }
    else
    {
        print "false\n" if $is_debug;
        return (0,1);
    }
}

sub readGlobalLog {
    my ($line);
    my $logfilename = $logfiledir . $pathslash . $global_log;
    if (! -e $logfilename) {
        print "$logfilename: file is missing\n" if $is_debug;
        return 0;
    }

    open TABLE, "<$logfilename" or die "Error: can´t open log file $logfilename]";

    my $failed = 0;
    my $complete = 0;
    my $FirstLine = 1;
    while(defined($line = <TABLE>) and !$complete) {
        chomp $line;
        if ($FirstLine) {
            if ( $line =~ /Sequence of testing/ ) {
                $FirstLine = 0;
                next;
            }
            else {
                print "$logfilename: $line\n" if $is_debug;
                $failed = 1;
                return 0;
            }
        }
        else {
            if ( $line eq "" ) {
                $complete = 1;
                next;
            }
            my @splitedLine = split(/:/,$line);
            my $testApplication = $splitedLine [0];
            $testApplication =~ s/^ *(.*?) *$/$1/g; #truncate
            my $testAction = $splitedLine [1];
            @splitedLine = split(/,/,$testAction);
            my @log_array = ();
            foreach my $action (@splitedLine) {
                $action =~ s/^ *(.*?) *$/$1/g; #truncate
                if ($action =~ /\//) {
                    my @splitAction = split(/\//,$action);
                    my @specialAction;
                    foreach my $doubleaction (@splitAction) {
                        $doubleaction =~ s/^ *(.*?) *$/$1/g; #truncate
                        push (@specialAction, $doubleaction);
                    }
                    $action = join (' or ', @specialAction);
                    foreach my $doubleaction (@splitAction) {
                        push (@log_array, $action);
                    }
                }
                else {
                    push (@log_array, $action);
                }
            }
            push (@ApplicationLog, $testApplication);
            $log{$testApplication} = \@log_array;
        }
    }

    close TABLE;

    return 1;
}

sub getLog {
    my ($testname, $linecount) = @_;
    if ($linecount <= $#{@{$log{$testname}}}) {
        return $log{$testname}[$linecount];
    }
    else {
        return "";
    }
}

### main ###

$idStr = ' $Revision: 1.6 $ ';
$idStr =~ /Revision:\s+(\S+)\s+\$/
  ? ($cpflat2minor_rev = $1) : ($cpflat2minor_rev = "-");

print "TestLog -- Version: $cpflat2minor_rev\n";

if (-e $okfile) {
        unlink ($okfile);
}

if (  ($#ARGV >-1) && ($#ARGV < 1) ) {
    $ARGV[0] =~ s/\"//g;
}

if ( ! ( ($#ARGV < 1) && $ARGV[0] && (-d $ARGV[0]) ) ) {
    print "Error! Usage: testlog <log_directory>\n" ;
    exit(1);
}

$logfiledir = $ARGV[0];
$is_testerror = 0;
$is_OneTestAvailable = 0;

print "%logfiles_list\n" if $is_debug;

readGlobalLog();

foreach my $applog (@ApplicationLog) {
    if (!exists($logfiles_list{$applog})) {
        next;
    }
    my $logname = $logfiles_list{$applog};
    $current_file = $logfiledir . $pathslash . $logname;
    my ($error, $logfile) = test_logfile ($current_file, $applog);

    if ($logfile) {
        $is_OneTestAvailable = 1;
    }
    elsif (!$is_testerror) {
        print "error: $applog failed! Logfile is missing.\n";
    }

    if (!$error) {
        $is_testerror = 1;
    }

}

# write file to prevent deinstallation of office
if ($is_testerror) {
    $dont_del_file = $logfiledir . $pathslash . $dont_kill;
    open (ERRFILE, ">$dont_del_file");
    print ERRFILE "dont delete flag";
    close (ERRFILE);
}
else {
    print "$okfile\n" if $is_debug;
    open( OKFILE, ">$okfile");
    print OKFILE "echo ok!\n";
    close( OKFILE );
    chmod (0775, "$okfile");
}

if (!$is_OneTestAvailable) {
    print "error: no test succeeded! Maybe Office crashed during starting!\n";
    $is_testerror = 1;
}

exit($is_testerror);
