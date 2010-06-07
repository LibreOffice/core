package graphical_compare;

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

use CallExternals;
use stringhelper;
use timehelper;
use filehelper;
use loghelper;
use oshelper;
use cwstestresulthelper;
use solarenvhelper;
use ConvwatchHelper;

use strict;
use Cwd;
# use File::Basename;
use Getopt::Long;
use English;                  # $OSNAME, ...
use File::Path;
use Cwd 'chdir';
use Sys::Hostname;
use Time::localtime;

# my $cwd = getcwd();

BEGIN {
    use Exporter   ();
    our ($VERSION, @ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS);

    $VERSION     = 1.00;
    # if using RCS/CVS, this may be preferred
    $VERSION = do { my @r = (q$Revision: 1.2 $ =~ /\d+/g); sprintf "%d."."%02d" x $#r, @r }; # must be all one line, for MakeMaker
    @ISA         = qw(Exporter);
    @EXPORT      = qw(&SingleDocumentCompare &setPrefix &setConnectionString);
    %EXPORT_TAGS = ( );     # eg: TAG => [ qw!name1 name2! ],
    # your exported package globals go here,
    # as well as any optionally exported functions
    @EXPORT_OK   = ( ); # qw($Var1 %Hashit &func3);
}


our $nTimeOut = 300 * 1000;
our $viewable = 1;
our $port;
our $resolution;
our $overwritereference;
our $fixreference;
our $sConncectionString;

sub setConnectionString($)
{
    $sConncectionString=shift;
}

sub getOOoRunnerClasspath()
{
    my $sSourceRoot;
    my $sUPDExtensions = "";
    if (defined ($ENV{SOL_TMP}) && defined ($ENV{SOLARVERSION}))
    {
        $sSourceRoot = $ENV{SOLARVERSION};
    }
    elsif (defined $ENV{SOURCE_ROOT})
    {
        $sSourceRoot = $ENV{SOURCE_ROOT};
        $sSourceRoot = appendPath($sSourceRoot, $ENV{WORK_STAMP});
    }
    else
    {
        $sSourceRoot = $ENV{SOLARVERSION};
        $sUPDExtensions = ".$ENV{UPDMINOR}";
    }
    $sSourceRoot = appendPath($sSourceRoot, $ENV{INPATH});
    my $sSourceRootBin = appendPath($sSourceRoot, "bin" . $sUPDExtensions);
    my $sSourceRootLib = appendPath($sSourceRoot, "lib" . $sUPDExtensions);

    if (! -d $sSourceRoot )
    {
        log_print( "SourceRoot not found, search it in '$sSourceRoot'\n");
        return "";
    }

    my $sOOoRunnerPath = $sSourceRootBin;
    my $sUnoilPath   = $sSourceRootBin;
    my $sRidlPath    = $sSourceRootBin;
    my $sJurtPath    = $sSourceRootBin;
    my $sJuhPath     = $sSourceRootBin;
    my $sJavaUnoPath = $sSourceRootBin;

    my $sOOoRunnerClasspath =
        appendPath( $sRidlPath,      "ridl.jar") . getJavaPathSeparator() .
        appendPath( $sUnoilPath,     "unoil.jar") . getJavaPathSeparator() .
        appendPath( $sJurtPath,      "jurt.jar") . getJavaPathSeparator() .
        appendPath( $sJuhPath,       "juh.jar") . getJavaPathSeparator() .
        appendPath( $sJavaUnoPath,   "java_uno.jar") . getJavaPathSeparator() .
        appendPath( $sOOoRunnerPath, "OOoRunnerLight.jar");
    if (isWindowsEnvironment())
    {
        $sOOoRunnerClasspath .= getJavaPathSeparator() . $sSourceRootBin;
    }
    else
    {
        $sOOoRunnerClasspath .= getJavaPathSeparator() . $sSourceRootLib;
    }
    return $sOOoRunnerClasspath;
}

# ------------------------------------------------------------------------------
sub getTempPath()
{
    my $sTempPath;
    if (isWindowsEnvironment())
    {
        $sTempPath = "C:/temp";
    }
    elsif (isUnixEnvironment())
    {
        $sTempPath = "/tmp";
    }
    else
    {
        die "getTempPath() Failed, due to unsupported environment.\n";
    }
    return $sTempPath;
}
# ------------------------------------------------------------------------------

sub getProjectOutput()
{
    my $sOutput = appendPath(getProjectRoot(), $ENV{INPATH});
    $sOutput = appendPath($sOutput, "misc");
    return $sOutput;
}

# ------------------------------------------------------------------------------
sub getProjectOutputReference()
{
    my $sOutput = appendPath(getProjectRoot(), $ENV{INPATH});
    $sOutput = appendPath($sOutput, "reference");
    return $sOutput;
}


sub searchForReference($)
{
    my $sFile = shift;
    if ( -e $sFile )
    {
        return 0;
    }
    if ( -e $sFile . ".ps")
    {
        return 0;
    }
    if ( -e $sFile . ".pdf")
    {
        return 0;
    }
    return 1;
}
# ------------------------------------------------------------------------------

# my $sOfficeName = $officeprefixname . $officename;
sub SingleDocumentCompare($$$$$$)
{
    # get all about the document to compare
    my $sDocumentPoolPath = shift;
    my $sDocumentPool = shift;
    my $sDocumentName = shift;
    my $sDebug = "";

    # get all about the destination office
    my $sCreatorType = shift;
    if (! $sCreatorType)
    {
        # log_print( "parameter -creatortype not given. Use 'OOo'\n");
        $sCreatorType = "ps";
    }
    my $prepareonly = shift;
    my $show = shift;

    # my $nSimpleCompareTime = getTime();

    my $nConvwatchFailed = 0;
    set_logfile( appendPath(getProjectOutput(), $sDocumentName . ".txt" ));

    print("$sDocumentName");
    log_print("\n");
    log_print("Graphical compare on document: '$sDocumentName'\n");
    # ------------------------------------------------------------------------------
    # create postscript or pdf from first installed office
    # ------------------------------------------------------------------------------

    my $sOOoRunnerClasspath = quoteIfNeed(getOOoRunnerClasspath());
    if ($OSNAME eq "cygwin")
    {
        if (!startswith($sOOoRunnerClasspath, "\""))
        {
            $sOOoRunnerClasspath = quote($sOOoRunnerClasspath);
        }
    }
    if (length($sOOoRunnerClasspath) == 0)
    {
        $nConvwatchFailed == 1;
    }
    # ------------------------------------------------------------------------------
    # create postscript or pdf from second installed office
    # ------------------------------------------------------------------------------

    my $sPathesIni = appendPath(getProjectOutput(), "pathes.ini");
    my $gspath = getFromPathes($sPathesIni, "gs.path");
    my $gsexe = getFromPathes($sPathesIni, "gs.exe");
    my $impath = getFromPathes($sPathesIni, "imagemagick.path");
    my $javaexe = getFromPathes($sPathesIni, "java.exe");
    setJavaExecutable($javaexe);

    log_print("----- CREATE POSTSCRIPT OR PDF WITH RUNNING OFFICE -----\n");
    # my $nPrepareSecondPostscriptTime = getTime();
    if ($nConvwatchFailed == 0)
    {
        my $sInputPath = $sDocumentPoolPath;
        $sInputPath = appendPath($sInputPath, $sDocumentPool);
        $sInputPath = appendPath($sInputPath, $sDocumentName);

        if (! -f $sInputPath )
        {
            $nConvwatchFailed = 1;
            log_print("ERROR: File '$sInputPath' doesn't exists.\n");
        }
        else
        {
            my $sOutputPath = getProjectOutput();
            my $sPropertyFile = appendPath(getProjectOutput() , $sDocumentName . ".build.props");

            local *PROPERTYFILE;
            if (open(PROPERTYFILE, ">$sPropertyFile"))
            {
                print PROPERTYFILE "# This file is automatically created by graphical_compare.pl\n";
                print PROPERTYFILE "DOC_COMPARATOR_PRINT_MAX_PAGE=9999\n";
                print PROPERTYFILE "DOC_COMPARATOR_GFX_OUTPUT_DPI_RESOLUTION=180\n";
                print PROPERTYFILE "DOC_COMPARATOR_REFERENCE_CREATOR_TYPE=$sCreatorType\n";
                print PROPERTYFILE "TEMPPATH=" . getTempPath() . "\n";
                if ($sConncectionString)
                {
                    print PROPERTYFILE "ConnectionString=$sConncectionString\n";
                }
                else
                {
                    print PROPERTYFILE "ConnectionString=pipe,name=" . getUsername() . "\n";
                }
                print PROPERTYFILE "OFFICE_VIEWABLE=true\n";
                print PROPERTYFILE "CREATE_DEFAULT_REFERENCE=true\n";
                print PROPERTYFILE "DOC_COMPARATOR_INPUT_PATH=$sInputPath\n";
                print PROPERTYFILE "DOC_COMPARATOR_OUTPUT_PATH=$sOutputPath\n";
                if (isWindowsEnvironment())
                {
                    print PROPERTYFILE "DOC_COMPARATOR_PRINTER_NAME=CrossOffice Generic Printer\n";
                }
                print PROPERTYFILE "NoOffice=true\n";

                close(PROPERTYFILE);
            }
            else
            {
                print "Can't open '$sPropertyFile' for write.\n";
            }
            if ( -e "$sPropertyFile")
            {
                # start OOoRunner
                # sleep 10;
                # $sOOoRunnerClasspathFromDestinationName = quoteIfNeed(getOOoRunnerClasspath());
                my $sParams;
                if ( $ENV{PERL} )
                {
                    $sParams = "-Dperl.exe=" . convertCygwinPath($ENV{PERL});
                }

                $sParams .= " -cp " . $sOOoRunnerClasspath .
                    " org.openoffice.Runner" .
                    " -TimeOut $nTimeOut" .
                    " -tb java_complex" .
                    " -ini $sPropertyFile" .
                    " -o graphical.PostscriptCreator";
                # $sParams .= " -cs pipe,name=$USER";

                # $sDebug = "-Xdebug  -Xrunjdwp:transport=dt_socket,server=y,address=9001,suspend=y";
                my $err = calljava(getJavaExecutable(), $sParams, $sDebug);
                $sDebug = "";
                log_print( "\n\n");
                if ($err != 0)
                {
                    my $sFailure = "Failed after try to create Postscript/pdf document for " . $sDocumentName;
                    log_print("ERROR: $sFailure\n");
                    $nConvwatchFailed = 1;
                }
            }
            else
            {
                my $sFailure = "There is no propertyfile: $sPropertyFile";
                log_print( "ERROR: $sFailure\n");
                $nConvwatchFailed=1;
            }
        }

        # set prepareonly and it is possible to only create ps or pdf files
        if ($prepareonly)
        {
            print(" [only create ");
            if ($sCreatorType eq "ps" || $sCreatorType eq "pdf")
            {
                print(" $sCreatorType");
            }
            else
            {
                print(" (${sCreatorType}?)");
            }
            if ($nConvwatchFailed == 0)
            {
                print(" ok");
            }
            else
            {
                print(" failed")
            }
            print("]\n");
            return $nConvwatchFailed;
        }


        # ------------------------------------------------------------------------------
        # create jpeg from postscript or pdf from second installed office
        # ------------------------------------------------------------------------------

        if ($nConvwatchFailed == 0)
        {
            log_print("----- CREATE JPEG FROM POSTSCRIPT OR PDF FROM RUNNING OFFICE -----\n");
            # start OOoRunner
            my $sInputPath = getProjectOutput();
            $sInputPath = appendPath($sInputPath, $sDocumentName);

            my $sOutputPath = getProjectOutput();

            my $sParams = "-cp " . $sOOoRunnerClasspath .
                " org.openoffice.Runner" .
                " -TimeOut $nTimeOut" .
                " -tb java_complex" .
                " -DOC_COMPARATOR_INPUT_PATH " . quoteIfNeed($sInputPath) .
                " -DOC_COMPARATOR_OUTPUT_PATH "  . quoteIfNeed($sOutputPath) .
                " -DOC_COMPARATOR_REFERENCE_CREATOR_TYPE $sCreatorType" .
                " -NoOffice" .
                " -NoSmallPictures" .
                " -o graphical.JPEGCreator";
            if ($gspath)
            {
                $sParams .= " -gs.path " . quoteIfNeed($gspath);
            }
            if ($gsexe)
            {
                $sParams .= " -gs.exe $gsexe";
            }

            # $sDebug = "-Xdebug  -Xrunjdwp:transport=dt_socket,server=y,address=9001,suspend=y";
            my $err = calljava(getJavaExecutable(), $sParams, $sDebug);
            $sDebug = "";
            # log_print( "\n\n");
            if ($err != 0)
            {
                my $sFailure = "Failed after try to create JPEG from Postscript/pdf document for " . $sDocumentName;
                log_print("ERROR: $sFailure\n");
                $nConvwatchFailed = 1;
            }
        }
    }

    # ------------------------------------------------------------------------------
    # create jpeg from postscript or pdf from references
    # ------------------------------------------------------------------------------

    if ($nConvwatchFailed == 0)
    {
        log_print("----- CREATE JPEG FROM POSTSCRIPT OR PDF FROM REFERENCE -----\n");

        # start OOoRunner
        my $sInputPath = appendPath(getProjectRoot(), "references");
        $sInputPath = appendPath($sInputPath, getEnvironment());
        $sInputPath = appendPath($sInputPath, $sDocumentPool);
        $sInputPath = appendPath($sInputPath, $sDocumentName);

        my $err = searchForReference($sInputPath);
        if ($err != 0)
        {
            log_print("ERROR: Can't find Postscript or PDF reference for '$sInputPath'\n");
            $nConvwatchFailed = 1;
        }
        else
        {
            my $sOutputPath = getProjectOutputReference();
            rmkdir $sOutputPath;

            my $sIndexFile = appendPath($sOutputPath, "index.ini");
            # we need the index.ini for better run through
            local *INDEXINI;
            if ( ! -e $sIndexFile)
            {
                if (open(INDEXINI, ">$sIndexFile"))
                {
                    # print INDEXINI "[$sDocumentName]\n";
                    close(INDEXINI);
                }
            }
            my $sParams = "-cp " . $sOOoRunnerClasspath .
                " org.openoffice.Runner" .
                " -TimeOut $nTimeOut" .
                " -tb java_complex" .
                " -DOC_COMPARATOR_INPUT_PATH " . quoteIfNeed($sInputPath) .
                " -DOC_COMPARATOR_OUTPUT_PATH "  . quoteIfNeed($sOutputPath) .
                " -DOC_COMPARATOR_REFERENCE_CREATOR_TYPE $sCreatorType" .
                " -NoOffice" .
                " -NoSmallPictures" .
                " -o graphical.JPEGCreator";
            if ($gspath)
            {
                $sParams .= " -gs.path " . quoteIfNeed($gspath);
            }
            if ($gsexe)
            {
                $sParams .= " -gs.exe $gsexe";
            }

            # $sDebug = "-Xdebug  -Xrunjdwp:transport=dt_socket,server=y,address=9001,suspend=y";
            my $err = calljava(getJavaExecutable(), $sParams, $sDebug);
            $sDebug = "";
            # log_print( "\n\n");
            if ($err != 0)
            {
                my $sFailure = "Failed after try to create JPEG from Postscript/pdf document for references.";
                log_print("ERROR: $sFailure\n");
                $nConvwatchFailed = 1;
            }
        }
    }
    # ------------------------------------------------------------------------------
    # compare JPEGs
    # ------------------------------------------------------------------------------

    if ($nConvwatchFailed == 0)
    {
        log_print("----- COMPARE JPEGS -----\n");
        my $sInputPath = appendPath(getProjectOutputReference(), $sDocumentName);

        my $sOutputPath = getProjectOutput();

        my $sParams = "-Xmx512m" .
            " -cp " . $sOOoRunnerClasspath .
            " org.openoffice.Runner" .
            " -TimeOut $nTimeOut" .
            " -tb java_complex" .
            " -DOC_COMPARATOR_INPUT_PATH "  . quoteIfNeed($sInputPath) .
            " -DOC_COMPARATOR_OUTPUT_PATH " . quoteIfNeed($sOutputPath) .
            " -NoOffice" .
            " -NoSmallPictures" .
            " -o graphical.JPEGComparator";
        if ($impath)
        {
            $sParams .= " -imagemagick.path " . quoteIfNeed($impath);
        }

        # start OOoRunner
        # $sDebug = "-Xdebug  -Xrunjdwp:transport=dt_socket,server=y,address=9001,suspend=y";
        my $err = calljava(getJavaExecutable(), $sParams, $sDebug);
        $sDebug = "";
        log_print( "\n\n");
        if ($err != 0)
        {
            my $sFailure = "Failed after compare JPEGs $sDocumentName\n";
            log_print("ERROR: $sFailure\n");
            $nConvwatchFailed = 1;

            if ($show)
            {
                # try to execute new java tool to show graphical compare
                my $sJavaProgram = appendPath(getProjectRoot(), $ENV{INPATH});
                $sJavaProgram = appendPath($sJavaProgram, "class");
                $sJavaProgram = appendPath($sJavaProgram, "ConvwatchGUIProject.jar");
                if ( -e "$sJavaProgram")
                {
                    my $sInputPath = appendPath(getProjectOutput(), $sDocumentName . ".ps.ini");
                    if (! -e $sInputPath)
                    {
                        $sInputPath = appendPath(getProjectOutput(), $sDocumentName . ".pdf.ini");
                        if (! -e $sInputPath)
                        {
                            $sInputPath = 0;
                        }
                    }
                    if ($sInputPath)
                    {
                        my $sParams = "-Xms128m -Xmx512m -jar $sJavaProgram $sInputPath";
                        # $sParams .= " -cs pipe,name=$USER";
                        # my $sJavaExe = "C:/Program Files/Java/jdk1.6.0_16/bin/java.exe"; # getJavaExecutable()
                        my $sJavaExe = getJavaExecutable();
                        # $sDebug = "-Xdebug  -Xrunjdwp:transport=dt_socket,server=y,address=9001,suspend=y";
                        my $err = calljava($sJavaExe, $sParams, $sDebug);
                        # $sDebug = "";
                        # log_print( "\n\n");
                        # if ($err != 0)
                        # {
                        #     my $sFailure = "Failed after try to create Postscript/pdf document for " . $sDocumentName;
                        #     log_print("ERROR: $sFailure\n");
                        #     $nConvwatchFailed = 1;
                        # }
                    }
                }
                else
                {
                    print "WARNING: The show program '$sJavaProgram' doesn't exists.\n";
                }
            }
        }
    }

    log_print( "\n\n");
    close_logfile();

    if ($nConvwatchFailed == 0)
    {
        print(" [ok]\n");
    }
    else
    {
        print(" [FAILED]\n");
        print("\nPrint output of test: $sDocumentName\n");
        my $sLogFile = appendPath(getProjectOutput(), $sDocumentName . ".txt");
        showFile($sLogFile);
    }
    # printTime(endTime($nSimpleCompareTime));

    return $nConvwatchFailed;
}

# ------------------------------------------------------------------------------
# cat $file
sub showFile($)
{
    my $logfile = shift;
    local *LOGFILE;
    if (open(LOGFILE, "$logfile"))
    {
        my $line;
        while ($line = <LOGFILE>)
        {
            chomp($line);
            print $line ."\n";
        }
        close(LOGFILE);
    }
}


1;
