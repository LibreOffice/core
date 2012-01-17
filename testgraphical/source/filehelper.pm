package filehelper;

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



use strict;
use warnings;
use strict;
use English;                  # $OSNAME, ...
use stringhelper;

BEGIN {
    use Exporter   ();
    our ($VERSION, @ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS);

    $VERSION     = 1.00;
    # if using RCS/CVS, this may be preferred
    $VERSION = do { my @r = (q$Revision: 1.1 $ =~ /\d+/g); sprintf "%d."."%02d" x $#r, @r }; # must be all one line, for MakeMaker
    @ISA         = qw(Exporter);
    @EXPORT      = qw(&dospath &unixpath &appendPath &appendClass &setPrefix &getToolsPrefix &rmkdir &getJavaPathSeparator &getJavaFileDirSeparator &getFromPathes &convertCygwinPath);
    %EXPORT_TAGS = ( );     # eg: TAG => [ qw!name1 name2! ],
    # your exported package globals go here,
    # as well as any optionally exported functions
    @EXPORT_OK   = ( ); # qw($Var1 %Hashit &func3);
}


# ------------------------------------------------------------------------------
# helper, to change all file separators
sub dospath($)
{
    my $sPath = shift;
    if ($OSNAME eq "MSWin32")
    {
        # make out of '/' a '\'
        $sPath =~ s/\//\\/g;
    }
    else
    {
    }
    return $sPath;
}

sub unixpath($)
{
    my $sPath = shift;
    if ($OSNAME ne "MSWin32")
    {
        # make out of '\' a '/'
        $sPath =~ s/\\/\//g;
    }
    else
    {
    }
    return $sPath;
}

# ------------------------------------------------------------------------------
# sub getGlobalInstSet()
# {
#     my $sJumbo;
#     if ($OSNAME eq "MSWin32")
#     {
#         # $sJumbo = "\\\\so-gfxcmp-lin\\jumbo_ship\\install";
#         $sJumbo = "\\\\jumbo.germany.sun.com\\ship\\install";
#     }
#     elsif ($OSNAME eq "cygwin")
#     {
#         $sJumbo = "//jumbo.germany.sun.com/ship/install";
#     }
#     else
#     {
#         $sJumbo = "/net/jumbo.germany.sun.com/ship/install";
#     }
#     return $sJumbo;
# }

# ------------------------------------------------------------------------------
# sub getSolarisLockFile()
# {
#     my $sSolarisLockFile = "/tmp/.ai.pkg.zone.lock-afdb66cf-1dd1-11b2-a049-000d560ddc3e";
#     return $sSolarisLockFile;
# }
#
# sub checkForSolarisLock()
# {
#     if ($OSNAME eq "solaris")
#     {
#         # wait until the internal installer lock is gone
#         while ( -e getSolarisLockFile() )
#         {
#             while ( -e getSolarisLockFile() )
#             {
#                 log_print( "Warning: Wait active until installer lock is gone. \n");
#                 sleep 1;
#             }
#             sleep 5;
#         }
#         log_print( "[ok], lock is gone.\n");
#     }
# }
#
# sub deleteSolarisLock()
# {
#     if ($OSNAME eq "solaris")
#     {
#         sleep 1;
#         unlink getSolarisLockFile();
#
#         sleep 1;
#         if ( -e getSolarisLockFile() )
#         {
#             # try delete the file as super user?
#             `sudo rm -f getSolarisLockFile()`;
#             sleep 1;
#         }
#     }
# }

# ------------------------------------------------------------------------------
sub appendPath($$)
{
    my $sPath = shift;
    my $sAddPath = shift;
    if ($sPath && $sAddPath)
    {
        if (! endswith($sPath, "/") &&
            ! endswith($sPath, "\\"))
        {
            # getJavaFileDirSeparator();
            $sPath .= "/";
        }
        $sPath .= $sAddPath;
    }
    return $sPath;
}

sub appendClass($$)
{
    my $sPath = shift;
    my $sAddPath = shift;

    my $sSeparator = getJavaPathSeparator();
    if ($sPath && $sAddPath)
    {
        if (! endswith($sPath, $sSeparator))
        {
            # getJavaFileDirSeparator();
            $sPath .= $sSeparator;
        }
        $sPath .= $sAddPath;
    }
    return $sPath;
}

# ------------------------------------------------------------------------------

our $sPrefix;
sub setPrefix($)
{
    $sPrefix = shift;
}

sub getPrefix()
{
    return $sPrefix;
}

# ------------------------------------------------------------------------------
our $programprefix;

# sub getProgramPrefix($)
# {
#     my $sDBDistinct = shift;
#
#     my $sProgramPrefix;
#     if (! $programprefix)
#     {
#         if ($OSNAME eq "MSWin32")
#         {
#             # $programprefix = "C:/gfxcmp/programs";
#             $programprefix = "C:";
#             if (getPrefix() eq "performance")
#             {
#                 $programprefix = "D:";
#             }
#             $programprefix = appendPath($programprefix, "gp");
#         }
#         elsif ($OSNAME eq "linux")
#         {
#             $programprefix = "/space/" . getPrefix() . "/programs";
#         }
#         elsif ($OSNAME eq "solaris")
#         {
#             $programprefix = "/space/" . getPrefix() . "/programs";
#         }
#         else
#         {
#             print "Error: This environment isn't supported yet.\n";
#             exit(1);
#         }
#     }
#     $sProgramPrefix = appendPath($programprefix, substr($sDBDistinct, 0, 19));
#     return $sProgramPrefix;
# }
# ------------------------------------------------------------------------------
our $toolsprefix;

sub getToolsPrefix()
{
    my $sToolsPrefix;
    if (! $toolsprefix)
    {
        if ($OSNAME eq "MSWin32")
        {
            $toolsprefix = "C:";
            if (getPrefix() eq "performance")
            {
                $toolsprefix = "D:";
            }
        }
        elsif ($OSNAME eq "linux")
        {
            $toolsprefix = "/space";
        }
        elsif ($OSNAME eq "solaris")
        {
            $toolsprefix = "/space";
        }
        else
        {
            print "Error: This environment isn't supported yet.\n";
            exit(1);
        }
        $toolsprefix = appendPath($toolsprefix, getPrefix());
        $toolsprefix = appendPath($toolsprefix, "tools");
    }
    $sToolsPrefix = $toolsprefix;
    return $sToolsPrefix;
}

# also Windows safe
sub rmkdir($)
{
    my($tpath) = shift;
    my $dir;
    my $accum = "";

    my @dirs = split(/\//, $tpath);
    if ( $#dirs eq 0 )
    {
        @dirs = split("\\\\", $tpath);
    }

    foreach $dir (@dirs)
    {
    $accum = "$accum$dir/";
    if($dir ne "")
    {
        if(! -d "$accum")
        {
        mkdir ($accum);
                chmod (0777,$accum);
        }
    }
    }
}

# ------------------------------------------------------------------------------
sub getJavaPathSeparator()
{
    my $ps = ":";
    if ($OSNAME eq "MSWin32" || $OSNAME eq "cygwin")
    {
        $ps = ";";
    }
    return $ps;
}
# ------------------------------------------------------------------------------
sub getJavaFileDirSeparator()
{
    my $sfs = "/";
    if ($OSNAME eq "MSWin32")
    {
        $sfs = "\\";
    }
    return $sfs;
}
# ------------------------------------------------------------------------------
sub getFromPathes($$)
{
    my $sPathesIni = shift;
    my $searchvalue = shift;
    my $sResult;
    if ( -e $sPathesIni)
    {
        local *PATHES;
        if (open(PATHES, "$sPathesIni"))
        {
            my $line;
            while ($line = <PATHES>)
            {
                chomp($line);
                if ($line =~ /^$searchvalue=(.*)$/)
                {
                    $sResult = $1;
                }
            }
            close(PATHES);
        }
    }
    return $sResult;
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



1;
