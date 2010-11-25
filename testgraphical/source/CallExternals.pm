package CallExternals;

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

use English;
use warnings;
use strict;
use loghelper;

BEGIN {
    use Exporter   ();
    our ($VERSION, @ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS);

    $VERSION     = 1.00;
    # if using RCS/CVS, this may be preferred
    $VERSION = do { my @r = (q$Revision: 1.29 $ =~ /\d+/g); sprintf "%d."."%02d" x $#r, @r }; # must be all one line, for MakeMaker
    @ISA         = qw(Exporter);
    @EXPORT      = qw(&callphp &getPHPExecutable &ExecSQL &callperl &getPerlExecutable &calljava &setJavaExecutable &getJavaExecutable &setToolsPath &quote &quoteIfNeed &set_logfile &close_logfile );
    %EXPORT_TAGS = ( );     # eg: TAG => [ qw!name1 name2! ],
    # your exported package globals go here,
    # as well as any optionally exported functions
    @EXPORT_OK   = ( ); # qw($Var1 %Hashit &func3);
}

# ------------------------------------------------------------------------------
# small helper, which replaces the return code
sub errorAdaption($)
{
    my $error = shift;
    if ($error != 0)
    {
        $error = $error / 256;
    }
    if ($error > 127)
    {
        $error = $error - 256;
    }
    return $error;
}
# ------------------------------------------------------------------------------
# helper to call external php with popen
sub callphp($$$)
{
    local *IN_FILE;
    my $phpexe = shift;
    my $phpprogram = shift;
    my $sParams = shift;
    my $line;
    my $error;
    my @result;

    # print "Will send: $phpexe $sParams\n";
    # log_print("CALLPHP: $phpexe $phpprogram $sParams\n");
#    if (open(IN_FILE, "$phpexe $sParams 2>&1 |"))
    if (open(IN_FILE, "$phpexe $phpprogram $sParams |"))
    {
        while ($line = <IN_FILE>)
        {
            chomp($line);
            # $line .= " ";
            push(@result, $line);
            # print "callphp output: $line\n";
        }
        close(IN_FILE);
        $error = errorAdaption($?);
    }
    else
    {
        print "callphp(): Can't popen '$phpexe' with parameter: '$sParams'\n";
        $error = 1;
    }
    return $error, @result;
}

# ------------------------------------------------------------------------------
sub getPHPExecutable()
{
    my $phpexe;
    if ($OSNAME eq "solaris")
    {
        $phpexe = "php5";
    }
    elsif ($OSNAME eq "linux")
    {
        if ( -e "/usr/bin/php5") # Suse :-(
        {
            $phpexe = "php5";
        }
        elsif ( -e "/usr/bin/php") # Gentoo
        {
            $phpexe = "php";
        }
        else
        {
            print "getPHPExecutable(): no php exec found.\n";
        }
    }
    elsif ( $OSNAME eq "MSWin32" )
    {
        $phpexe = "C:/programme/php/php.exe";
        # add second try (xampp)
        if (! -e $phpexe)
        {
            $phpexe = "C:/xampp/php/php.exe";
        }
    }
    elsif ( $OSNAME eq "cygwin" )
    {
        $phpexe = "/cygdrive/c/programme/php/php";
    }
    else
    {
        print "getPHPExecutable(): unknown environment. ($OSNAME)\n";
    }
    if (! $phpexe)
    {
        print "getPHPExecutable(): ERROR: php executable not found.\n";
        exit(1);
    }
    return $phpexe;
}
# ------------------------------------------------------------------------------
# helper to call external java with popen
sub calljava($$$)
{
    local *IN_FILE;
    my $javaexe = shift;
    my $sParams = shift;
    my $sDebug = shift;
    my $line;
    my $error = 1;

    if (! $javaexe)
    {
        log_print("ERROR: javaexe not set.\n");
        return;
    }
    if (! $sDebug)
    {
        $sDebug = "";
    }
    $javaexe = quoteIfNeed($javaexe);
    log_print ("CALLJAVA: $javaexe $sDebug $sParams\n");
    if (open(IN_FILE, "$javaexe $sDebug $sParams 2>&1 |"))
    {
        while ($line = <IN_FILE>)
        {
            chomp($line);
            log_print ("- $line\n");
        }
        close(IN_FILE);
        $error = errorAdaption($?);
    }
    else
    {
        log_print ("calljava(): Can't popen '$javaexe' with parameter '$sParams'\n");
        $error = 1;
    }
    return $error;
}

# ------------------------------------------------------------------------------
sub getPerlExecutable()
{
    my $perlexe;
    if ( $ENV{PERL} )
    {
        $perlexe = $ENV{PERL};
    }
    elsif ( $ENV{PERLEXE} )
    {
        $perlexe = $ENV{PERLEXE};
    }
    else
    {
        if ($OSNAME eq "MSWin32")
        {
            $perlexe="C:/xampp/perl/bin/perl.exe";
            if (! -e $perlexe)
            {
                $perlexe="r:/btw/perl/bin/perl";
            }
            if (! -e $perlexe)
            {
                $perlexe="C:/Programme/Perl/bin/perl.exe";
            }
        }
        elsif ($OSNAME eq "cygwin")
        {
            $perlexe = "perl";
        }
        elsif ($OSNAME eq "solaris")
        {
            $perlexe="/so/env/bt_solaris_intel/bin/perl";
        }
        elsif ($OSNAME eq "linux")
        {
            $perlexe="/so/env/bt_linux_libc2.32/DEV300/bin/perl";
        }
        else
        {
            log_print "WARNING: Use only the fallback of perl executable.\n";
            $perlexe = "perl"; # FALLBACK
        }
    }
    if ( ! -e $perlexe)
    {
        log_print "getPerlExecutable(): There exist no perl executable.\n";
        exit(1);
    }
    return $perlexe;
}
# ------------------------------------------------------------------------------
# helper to call external perl with popen
sub callperl($$$)
{
    local *IN_FILE;
    my $perlexe = shift;
    my $perlprogram = shift;
    my $sParams = shift;
    my $line;
    my $error;

    log_print("CALLPERL: $perlexe $perlprogram $sParams\n");
#    if (open(IN_FILE, "$perlexe $sParams 2>&1 |"))
    if (open(IN_FILE, "$perlexe $perlprogram $sParams |"))
    {
        while ($line = <IN_FILE>)
        {
            chomp($line);
            log_print ("- $line\n");
        }
        close(IN_FILE);
        $error = errorAdaption($?);
    }
    else
    {
        log_print ("Can't popen '$perlexe' with parameter: '$sParams'\n");
        $error = 1;
    }
    return $error;
}
# ------------------------------------------------------------------------------
our $sJavaExecutable;
sub setJavaExecutable($)
{
    $sJavaExecutable = shift;
}

# sub getJava14()
# {
#     my $sJava14;
#     if ($OSNAME eq "MSWin32")
#     {
#         if ($sJavaExecutable)
#         {
#             $sJava14 = $sJavaExecutable;
#         }
#         else
#         {
#             # HARDCODE!
#             $sJava14 = "C:\\Programme\\Java\\j2re1.4.2_10\\bin\\java.exe";
#         }
#     }
#     else
#     {
#         if ($sJavaExecutable)
#         {
#             $sJava14 = $sJavaExecutable;
#         }
#         else
#         {
#             # HARDCODE!
#             $sJava14 = "/opt/java14/bin/java";
#         }
#     }
#     if ( ! -e $sJava14 )
#     {
#         log_print ("Java14 not found. Is searched in '$sJava14'\n");
#         # exit(1);
#         return "";
#     }
#     return $sJava14;
# }
# ------------------------------------------------------------------------------
sub getJava15()
{
    my $sJava15;
    if ($sJavaExecutable)
    {
        $sJava15 = $sJavaExecutable;
    }
    else
    {
        if ($OSNAME eq "MSWin32")
        {
            # HARDCODE!
            $sJava15 = "C:\\Programme\\Java\\jre1.5.0_22\\bin\\java.exe";
            if ( ! -e $sJava15)
            {
                $sJava15 = "C:\\Program Files\\Java\\jre6\\bin\\java.exe";
            }
            if ( ! -e $sJava15)
            {
                $sJava15 = "C:\\Java\\jdk1.6\\bin\\java.exe";
            }
        }
        elsif ($OSNAME eq "cygwin")
        {
            $sJava15 = "java";
        }
        else
        {
            # HARDCODE!
            if ($OSNAME eq "solaris")
            {
                $sJava15 = "/usr/bin/java";
            }
            else
            {
                $sJava15 = "/usr/bin/java";
                if ( ! -e $sJava15 )
                {
                    $sJava15 = "/opt/java15/bin/java";
                }
            }
        }
        if ( ! -e $sJava15 )
        {
            log_print ("Java15 not found. Is searched in '$sJava15'\n");
            # exit(1);
            return "";
        }
    }
    return $sJava15;
}
# ------------------------------------------------------------------------------
sub getJava16()
{
    my $sJava16;
    if ($sJavaExecutable)
    {
        $sJava16 = $sJavaExecutable;
    }
    else
    {
        if ($OSNAME eq "MSWin32")
        {
            # HARDCODE!
            $sJava16 = "C:\\Programme\\Java\\jre1.6.0_16\\bin\\java.exe";
            if ( ! -e $sJava16)
            {
                $sJava16 = "C:\\Program Files\\Java\\jre6\\bin\\java.exe";
            }
            if ( ! -e $sJava16)
            {
                $sJava16 = "C:\\Java\\jdk1.6\\bin\\java.exe";
            }
            # }
        }
        elsif ($OSNAME eq "cygwin")
        {
            # $sJava16 = "java";
            $sJava16 = "C:/Program Files/Java/jdk1.6.0_16/bin/java.exe";
        }
        else
        {
            # HARDCODE!
            if ($OSNAME eq "solaris")
            {
                $sJava16 = "/usr/bin/java";
            }
            else
            {
                $sJava16 = "/usr/bin/java";
                if ( ! -e $sJava16 )
                {
                    $sJava16 = "/opt/java16/bin/java";
                }
            }
        }
        if ( ! -e $sJava16 )
        {
            log_print ("Java16 not found. Is searched in '$sJava16'\n");
            # exit(1);
            return "";
        }
    }
    return $sJava16;
}

# ------------------------------------------------------------------------------
sub getJavaExecutable()
{
    return getJava16();
}

# ------------------------------------------------------------------------------
# this function is a helper for parameters
# if quotes the whole string with 'STR' or "STR" and replace quotes in it's content for the right.
sub singleQuote($)
{
    my $sStr = shift;
    if ( $OSNAME eq "MSWin32")
    {
        # we are MSWin32 (quote \" stronger)
        # $sStr =~ s/\'/\"/g;
        $sStr =~ s/\'/\\\"/g;
        return "\"" . $sStr . "\"";
    }
    else
    {
        if (index($sStr, "'") >= 0)
        {
            # replace all single quotes ("'") by "\""
            $sStr =~ s/\'/\"/g;
        }
    }
    return "'" . $sStr . "'";
}

sub quote($)
{
    my $sName = shift;
    return "\"" . $sName . "\"";
}

sub quoteIfNeed($)
{
    my $sName = shift;
    if (-1 != index($sName, " "))
    {
        return quote($sName);
    }
    return $sName;
}


# ------------------------------------------------------------------------------
our $sToolsPath;
sub setToolsPath($)
{
    my $sNewPath = shift;
    $sToolsPath = $sNewPath;
}

sub ExecSQL($)
{
    my $sSQL = shift;

    my $error;
    my @aResult;
    my $sSQLDirect;
    if ($sToolsPath)
    {
        $sSQLDirect = $sToolsPath;
        $sSQLDirect .= "/";
    }
    $sSQLDirect .= "sql_direct.php";

    # select(undef, undef, undef, 0.060);
    # log_print("ExecSQL: $sSQL\n");
    # sleep (1);
    ($error, @aResult) = callphp(getPHPExecutable(), $sSQLDirect, singleQuote($sSQL));
    if ($error)
    {
        log_print ("ExecSQL: An Error occured.\n");
        log_print ("PHP: " . getPHPExecutable() . "\n");
        log_print ("SQL Statement: " . singleQuote($sSQL) . "\n");
        # exit(1);
    }
    # select(undef, undef, undef, 0.125);
    # sleep (1);
    return @aResult;
}

# ------------------------------------------------------------------------------
# helper to call external php with popen
# sub callexe($$$)
# {
#     local *IN_FILE;
#     my $exe = shift;
#     my $program = shift;
#     my $sParams = shift;
#     my $line;
#     my $error;
#     my @result;
#
#     $exe = quoteIfNeed($exe);
#     $program = quoteIfNeed($program);
#
#     # print "Will send: $exe $sParams\n";
#     # log_print("CALLEXE: $exe $program $sParams\n");
#     if (open(IN_FILE, "$exe $program $sParams |"))
#     {
#         while ($line = <IN_FILE>)
#         {
#             chomp($line);
#             # $line .= " ";
#             push(@result, $line);
#             # print "callphp output: $line\n";
#         }
#         close(IN_FILE);
#         $error = errorAdaption($?);
#     }
#     else
#     {
#         print "Can't popen '$exe' with parameter: '$sParams'\n";
#         $error = 1;
#     }
#     return $error, @result;
# }

1;
