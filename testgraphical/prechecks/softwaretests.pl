eval 'exec perl -wS $0 ${1+\"$@\"}'
    if 0;

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

# This is a pre check, which checks if some extra software exists

BEGIN
{
    #       Adding the path of this script file to the include path in the hope
    #       that all used modules can be found in it.
    $0 =~ /^(.*)[\/\\]/;
    push @INC, $1;
}

use strict;
use English;                  # $OSNAME, ...
use Getopt::Long;
use Cwd;
use Cwd 'chdir';
my $cwd = getcwd();

our $help;                    # Help option flag
our $version;                 # Version option flag

# flush STDOUT
# my $old_handle = select (STDOUT); # "select" STDOUT and save # previously selected handle
# $| = 1; # perform flush after each write to STDOUT
# select ($old_handle); # restore previously selected handle

$OUTPUT_AUTOFLUSH=1; # works only if use English is used.

our $sGlobalIniFile;
our $verbose = 0;
our $ghostscript;
our $imagemagick;
our $java6;
our $printerdriver;

our $version_info = 'compare.pl';

GetOptions(
           "ghostscript"   => \$ghostscript,
           "imagemagick"   => \$imagemagick,
           "java6"         => \$java6,
           "printerdriver" => \$printerdriver,
           "verbose"       => \$verbose,

           "help"          => \$help,
           "version"       => \$version
           );

if ($help)
{
    print_usage(*STDOUT);
    exit(0);
}
# Check for version option
if ($version)
{
    print STDERR "$version_info\n";
    exit(0);
}

# prepare the GlobalIniFile

sub prepare()
{
    my $sEnv = "$ENV{PRJ}";
    if (! $sEnv)
    {
        print "Warning: Seems you are not in a makefile.mk environment.\n";
        $sEnv = "..";
    }
    my $sPath = getcwd();
    $sPath .= "/" . $sEnv;
    chdir ($sPath);
    cwd();
    $sPath = getcwd();
    my $sInpath = $ENV{INPATH};
    $sPath .= "/" . $sInpath . "/misc";
    $sGlobalIniFile = "$sPath/pathes.ini";
    print "Global Path ini file is: $sGlobalIniFile\n" if ($verbose);
}

sub unixpath($)
{
    my $path = shift;
    $path =~ s/\\/\//g; # make out of '\' a '/'
    return $path;
}

# search for file in a given path list.
# the path list should be separated as the path variable in the corresponding OS
sub searchForFileInPath($$)
{
    my $sFile = shift;
    my $sPathList = shift;

    my $sep = ':';
    if ($OSNAME eq "MSWin32")
    {
        $sep = ';';
    }
    my @path = split($sep, $sPathList);

    my $sPath;
    my $startdir;
    my $bFound = 0;
    my $olddir = getcwd();

    my $sWindowsHomeDir = unixpath(lc($ENV{WINDIR}));

    foreach $startdir (@path)
    {
        my $nCount = 0;
        #
        # IMPORTANT: leave out windir path.
        #
        if ($OSNAME eq "MSWin32" || $OSNAME eq "cygwin")
        {
            my $sPath = unixpath(lc(convertCygwinPath($startdir)));
            if ($sPath =~ /^$sWindowsHomeDir/ )
            {
                print "path: $startdir is windows path leave out.\n" if ($verbose);
                next;
            }
        }

        local *DIR;
        if (opendir (DIR, $startdir))           # open directory
        {
            print "path: $startdir" if ($verbose);
            chdir ($startdir);
            cwd();
            my $myfile;
            while ($myfile = readdir(DIR))      # get filename
            {
                if (-f $myfile )                # is it a file?
                {
                    $nCount ++;
                    if ($myfile eq $sFile)      # is it the real file?
                    {
                        $sPath = $startdir;
                        $bFound = 1;
                        last;
                    }
                }
            }
            closedir(DIR);
            print " ($nCount)\n" if ($verbose);
        }
        if ($bFound == 1)
        {
            last;
        }
    }
    chdir ($olddir);
    cwd();

    return $sPath;
}


prepare();
# don't remove the inifile, only build clean should do this.
# if ( -e "$sGlobalIniFile")
# {
#     unlink($sGlobalIniFile);
# }


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

# for every error we increment this variable by 1
our $nGlobalErrors = 0;

sub handleError($$)
{
    my $error = shift;
    my $sText = shift;
    if ($error != 0)
    {
        print "ERROR: search for $sText has failed with Errornumber: $error\n";
        $nGlobalErrors ++;
    }
}

sub convertCygwinPath($)
{
    my $sPath = shift;

    if ($OSNAME eq "cygwin")
    {
        # print "Cygwin Path Patch.\n" if ($verbose);
        if ($sPath =~ /\/cygdrive\/(.)/)
        {
            my $Letter = $1;
            $sPath =~ s/\/cygdrive\/${Letter}/${Letter}\:/;
            # print "Cygwin Path Patch: '$sPath'\n" if ($verbose);
        }
    }
    return $sPath;
}

# append key=value to GlobalIniFile
sub insertPath($$)
{
    my $sKey = shift;
    my $sValue = shift;

    $sValue = convertCygwinPath($sValue);
    my $sIniFile = convertCygwinPath($sGlobalIniFile);
    local *INIFILE;
    if (open(INIFILE, ">>" . $sIniFile ))
    {
        print INIFILE "$sKey=$sValue\n";
    }
    close(INIFILE);
}

sub getFastPath($)
{
    my $sKey = shift;
    my $sValue;
    local *INIFILE;
    my $sIniFile = convertCygwinPath($sGlobalIniFile);
    if (open(INIFILE, $sIniFile))
    {
        my $line;
        while ($line = <INIFILE>)
        {
            chomp($line);
            if ( $line =~ /^$sKey=(.*)$/ )
            {
                $sValue = $1;
                # print INIFILE "$sKey=$sValue\n";
            }
        }
        close(INIFILE);
    }
    return $sValue;
}

sub checkForGhostscript()
{
    print "Search for Ghostscript\n" if ($verbose);
    if ($OSNAME eq "linux" ||
        $OSNAME eq "solaris")
    {
        # search for ghostscript
        local *GHOSTSCRIPT;
        if (open(GHOSTSCRIPT, "which gs 2>&1 |"))
        {
            my $line;
            while ($line = <GHOSTSCRIPT>)
            {
                chomp($line);
                print "- $line\n" if ($verbose);
            }
            close(GHOSTSCRIPT);
        }
        my $error = errorAdaption($?);
        handleError($error, "Ghostscript");
    }
    elsif ($OSNAME eq "MSWin32" || $OSNAME eq "cygwin")
    {
        my $sGSExe = "gswin32c.exe";
        # my $sGSPath = "C:/gs/gs8.64/bin";
        my $sGSPath = getFastPath("gs.path");
        if (! $sGSPath)
        {
            $sGSPath = searchForFileInPath($sGSExe, $ENV{PATH});

            if ( ! -e "$sGSPath/$sGSExe")
            {
                $nGlobalErrors ++;
                print "ERROR: search for $sGSPath/$sGSExe failed.\n";
                print "Please install ghostscript from www.adobe.com to and make it available in \$PATH variable \n";
            }
            else
            {
                insertPath("gs.path", $sGSPath);
                insertPath("gs.exe", $sGSExe);
            }
        }
        if ( -e "$sGSPath/$sGSExe" )
        {
            print "Found Ghostscript: '$sGSPath'\n" if ($verbose);
        }
    }
    else
    {
        print "ERROR: Check for Ghostscript failed, due to unsupported '$OSNAME' environment.\n";
        $nGlobalErrors ++;
    }
}


sub checkForPSDriver()
{
    # we don't need to check for unix here, due to the fact, unix is per default be able to print in postscript
    if ($OSNAME eq "MSWin32" || $OSNAME eq "cygwin")
    {
        print "Check for postscript driver.\n" if ($verbose);
        my $sWindowsRoot = $ENV{windir};
        if (! $sWindowsRoot)
        {
            $sWindowsRoot = $ENV{WINDIR};
        }
        my $sCrossOfficeDriver = "${sWindowsRoot}/system32/crossoffice.ppd";
        if ( ! -e "$sCrossOfficeDriver")
        {
            print "ERROR: Don't found Postscript driver $sCrossOfficeDriver file\n";
            $nGlobalErrors ++;
            print "Take a look on: http://so-gfxcmp.germany.sun.com/docs/further/convwatch/convwatch.html.\n";
        }
    }
}

sub checkForImageMagick()
{
    print "Search for Imagemagick\n" if ($verbose);
    if ($OSNAME eq "linux" ||
        $OSNAME eq "solaris")
    {
        # search for imagemagick
        local *IMAGEMAGICK;
        if (open(IMAGEMAGICK, "which convert 2>&1 |"))
        {
            my $line;
            while ($line = <IMAGEMAGICK>)
            {
                chomp($line);
                print "- $line\n" if ($verbose);
            }
            close(IMAGEMAGICK);
        }
        my $error = errorAdaption($?);
        handleError($error, "Imagemagick");
    }
    elsif ($OSNAME eq "MSWin32" || $OSNAME eq "cygwin")
    {
        my $sImageMagickExe = "convert.exe";
        # my $sImageMagickPath = "C:/gs/gs8.64/bin";
        my $sImageMagickPath = getFastPath("imagemagick.path");
        if (! $sImageMagickPath)
        {
            $sImageMagickPath = searchForFileInPath($sImageMagickExe, $ENV{PATH});
            if ($sImageMagickPath)
            {
                if ( ! -e "$sImageMagickPath/$sImageMagickExe")
                {
                    $nGlobalErrors ++;
                    print "ERROR: search for $sImageMagickPath/$sImageMagickExe failed.\n";
                    print "Please install ImageMagick from www.imagemagick.org to and make it available in \$PATH variable \n";
                }
                else
                {
                    insertPath("imagemagick.path", $sImageMagickPath);
                    # insertPath("gs.exe", $sImageMagickExe);
                }
            }
            else
            {
                # next try, search image magick in $PROGRAMFILES
                my $sPrograms = unixpath($ENV{PROGRAMFILES});

                if (! $sPrograms)
                {
                    print "There exist no \$PROGRAMFILES path, wrong Windows version?\n";
                    $nGlobalErrors++;
                }
                else
                {
                    local *DIR;
                    if (opendir (DIR, $sPrograms))           # open program directory
                    {
                        my $myfile;
                        while ($myfile = readdir(DIR))       # get a filename
                        {
                            if ($myfile =~ /ImageMagick/)
                            {
                                $sImageMagickPath = $sPrograms . "/" . $myfile;
                                last;
                            }
                        }
                        closedir(DIR);
                    }
                    if (! -e $sImageMagickPath)
                    {
                        print "ImageMagick not found.\n";
                        $nGlobalErrors ++;
                    }
                    else
                    {
                        insertPath("imagemagick.path", $sImageMagickPath);
                    }
                }
            }

        }
        if ( -e "$sImageMagickPath/$sImageMagickExe" )
        {
            print "Found ImageMagick: '$sImageMagickPath'\n" if ($verbose);
        }
    }
    else
    {
        print "ERROR: not supported environment\n";
    }
}

sub checkForJava6()
{
    print "Search for Java6\n" if ($verbose);
    my $javaexe = "java";
    if ( $ENV{JAVA6} )
    {
        $javaexe = $ENV{JAVA6};
    }

    if ($OSNAME eq "linux" || $OSNAME eq "cygwin")
    {
        # search for imagemagick
        local *JAVA;
        if (open(JAVA, "$javaexe -version 2>&1 |"))
        {
            my $line;
            while ($line = <JAVA>)
            {
                chomp($line);
                print "- $line\n" if ($verbose);
                if ( $line =~ /java version "(.*)"/ )
                {
                    my $javaversion = $1;
                    my @version = split('\.', $javaversion);
                    print "Found Java version: $version[1]  the complete version: $javaversion\n" if ($verbose);
                    if ( $version[1] < 6)
                    {
                        print "Wrong Java version, at least Java version 6 is need but found $javaversion.\n";
                        $nGlobalErrors++;
                        print "It is possible to overwrite the java exe with environment variable JAVA6='path'.\n";
                    }
                    else
                    {
                        insertPath("java.exe", $javaexe);
                    }
                    last;
                }
            }
            close(JAVA);
        }
        my $error = errorAdaption($?);
        handleError($error, "Java");
    }
    elsif ($OSNAME eq "MSWin32")
    {
         my $javaexe = "java";
         if ( $ENV{JAVA6} )
         {
             $javaexe = $ENV{JAVA6};
         }

         if (! -e $javaexe)
         {
             print "Java not found.\n";
             $nGlobalErrors ++;
         }
         else
         {
             print "Found Java: '$javaexe'\n" if ($verbose);
             insertPath("java.exe", $javaexe);
         }
    }
    else
    {
        print "ERROR: Java not found.\n";
    }
}

# different checks
print "Environment '$OSNAME'\n" if ($verbose);

if ($printerdriver)
{
    checkForPSDriver();
}
if ($ghostscript)
{
    checkForGhostscript();
}
if ($imagemagick)
{
    checkForImageMagick();
}
if ($java6)
{
    checkForJava6();
}

# return with found errors
exit($nGlobalErrors);

# ------------------------------------------------------------------------------
sub print_usage(*)
{
    local *HANDLE = $_[0];
    my $tool_name = basename($0);

    print(HANDLE <<END_OF_USAGE);

Usage: $tool_name [OPTIONS]

    -ghostscript             Try to find ghostscript in your environment
    -imagemagick             Try to find imagemagick
    -java6                   Checks for java 1.6.x
    -printerdriver           Try to find printer driver, windows only
    -verbose                 be verbose

    -h, --help               Print this help, then exit
    -v, --version            Print version number, then exit

END_OF_USAGE
;
}
