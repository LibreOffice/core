package ConvwatchHelper;

use English;
use warnings;
use strict;
use Cwd;
use Cwd 'chdir';

use CallExternals;
use stringhelper;
use filehelper;
use oshelper;
use loghelper;

BEGIN {
    use Exporter   ();
    our ($VERSION, @ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS);

    $VERSION     = 1.00;
    # if using RCS/CVS, this may be preferred
    $VERSION = do { my @r = (q$Revision: 1.39 $ =~ /\d+/g); sprintf "%d."."%02d" x $#r, @r }; # must be all one line, for MakeMaker
    @ISA         = qw(Exporter);
    @EXPORT      = qw(&getQADEVToolsPath &setProjectRoot &getProjectRoot &checkForStop &getSofficeExe &setINPATH);
    %EXPORT_TAGS = ( );     # eg: TAG => [ qw!name1 name2! ],
    # your exported package globals go here,
    # as well as any optionally exported functions
    @EXPORT_OK   = ( ); # qw($Var1 %Hashit &func3);
}

# ------------------------------------------------------------------------------
our $tempprefix;

# sub getTempDir()
# {
#     my $sTempDir;
#     if (! $tempprefix)
#     {
#         if ($OSNAME eq "MSWin32" || $OSNAME eq "cygwin")
#         {
#             # $tempdir = "C:/gfxcmp/temp";
#             $tempprefix = "//so-gfxcmp-lin/gfxcmp-data/wntmsci/temp";
#         }
#         elsif ($OSNAME eq "linux")
#         {
#             $tempprefix = "/net/so-gfxcmp-lin/export/gfxcmp/data/unxlngi/temp";
#         }
#         elsif ($OSNAME eq "solaris")
#         {
#             # $tempdir = "/space/gfxcmp/temp";
#             $tempprefix = "/net/so-gfxcmp-lin/export/gfxcmp/data/unxsoli/temp";
#         }
#         else
#         {
#             print "Error: This environment isn't supported yet.\n";
#             exit(1);
#         }
#     }
#     $sTempDir = $tempprefix;
#     return $sTempDir;
# }
# ------------------------------------------------------------------------------
# in filehelper
# our $programprefix;
#
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
#             $programprefix = "C:/gp";
#         }
#         elsif ($OSNAME eq "linux")
#         {
#             $programprefix = "/space/gfxcmp/programs";
#         }
#         elsif ($OSNAME eq "solaris")
#         {
#             $programprefix = "/space/gfxcmp/programs";
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
sub getQADEVToolsPath()
{
    my $sNewPath = appendPath(getToolsPrefix(), "qadev");
    $sNewPath = appendPath($sNewPath, "scripts");
    $sNewPath = appendPath($sNewPath, "gfxcmp_ui");
    return $sNewPath;
}

# in filehelper
# our $toolsprefix;
#
# sub getToolsPrefix()
# {
#     my $sToolsPrefix;
#     if (! $toolsprefix)
#     {
#         if ($OSNAME eq "MSWin32")
#         {
#             $toolsprefix = "C:/gfxcmp/tools";
#         }
#         elsif ($OSNAME eq "linux")
#         {
#             $toolsprefix = "/space/gfxcmp/tools";
#         }
#         elsif ($OSNAME eq "solaris")
#         {
#             $toolsprefix = "/space/gfxcmp/tools";
#         }
#         else
#         {
#             print "Error: This environment isn't supported yet.\n";
#             exit(1);
#         }
#     }
#     $sToolsPrefix = $toolsprefix;
#     return $sToolsPrefix;
# }
# ------------------------------------------------------------------------------
our $sProjectRoot;
sub setProjectRoot($)
{
    $sProjectRoot = shift;
    log_print "\$sProjectRoot := $sProjectRoot\n";
}
sub getProjectRoot()
{
    if ($sProjectRoot)
    {
        return $sProjectRoot;
    }
    die "setProjectRoot(PATH) not set.\n";
}

our $sINPATH;
sub setINPATH($)
{
    $sINPATH = shift;
}
sub getINPATH()
{
    if ($sINPATH)
    {
        return $sINPATH;
    }
    die "setINPATH(PATH) not set.\n";
}
our $dataprefix;

# sub getDataPrefix()
# {
#     my $sDataPrefix;
#     if (! $dataprefix)
#     {
#         if ($OSNAME eq "MSWin32" || $OSNAME eq "cygwin")
#         {
#             # $dataprefix = "C:/gfxcmp/data";
#             # $dataprefix = "//so-gfxcmp-lin/gfxcmp-data/wntmsci";
#             $dataprefix = getProjectRoot();
#         }
#         elsif ($OSNAME eq "linux")
#         {
#             # $dataprefix = "/space/gfxcmp/data";
#             # $dataprefix = "/net/so-gfxcmp-lin/export/gfxcmp/data/unxlngi";
#             $dataprefix = getProjectRoot();
#         }
#         elsif ($OSNAME eq "solaris")
#         {
#             # $dataprefix = "/space/gfxcmp/data";
#             # $dataprefix = "/net/so-gfxcmp-lin/export/gfxcmp/data/unxsoli";
#             $dataprefix = getProjectRoot();
#         }
#         else
#         {
#             print "Error: This environment isn't supported yet.\n";
#             exit(1);
#         }
#         $dataprefix = appendPath(getProjectRoot(), getINPATH());
#         $dataprefix = appendPath($dataprefix, "data");
#     }
#     $sDataPrefix = $dataprefix;
#     return $sDataPrefix;
# }

# ------------------------------------------------------------------------------

# sub _shortsleep($)
# {
# #    sleep 1;
#     select(undef, undef, undef, 0.333);
# }
#
# sub _waitInSeconds($)
# {
#     my $nLength = shift;
#     my $i;
#     my $j;
#
#     for ($j=0;$j<$nLength;$j++)
#     {
#         for ($i=0;$i<$j;$i++)
#         {
#             print ".";
#         }
#         for ($i=$j;$i<$nLength;$i++)
#         {
#             print " ";
#         }
#         _shortsleep( 1 );
#         print "\r";
#     }
#
#     for ($j=0;$j<=$nLength;$j++)
#     {
#         for ($i=0;$i<$j;$i++)
#         {
#             print " ";
#         }
#         for ($i=$j;$i<$nLength;$i++)
#         {
#             print ".";
#         }
#         _shortsleep( 1 );
#         print "\r";
#     }
# }
#
# sub wait30seconds()
# {
#     _waitInSeconds(20);
#     _waitInSeconds(20);
# }

sub checkForStop($)
{
    my $sStopFilename = shift;
    my $sStopFilePath;
    if ($OSNAME eq "MSWin32")
    {
        $sStopFilePath = "C:/temp/";
    }
    else
    {
        $sStopFilePath = "/tmp/";
    }
    my $sStopFile = $sStopFilePath . $sStopFilename;
    if ( -e "$sStopFile" )
    {
        print "Stop of Convwatch tool forced!\n";
        unlink($sStopFile);
        exit(2);
    }
}

# ----------------------------------------------------------------------------------------
sub readdirectory($$$);

sub readdirectory($$$)
{
    my $startdir = shift;
    my $sUserParameter  = shift;
    my $hook     = shift;

    my $myfile;

    local *DIR;
    chdir $startdir;
    cwd();

    my $nCountFiles = 0;
    if (opendir (DIR, $startdir))           # Directory oeffnen
    {
        while ($myfile = readdir(DIR))
        {                                  # ein filename holen
            #if (! -l $myfile)              # not a link
            #{
            if (-d $myfile ) # is a directory
            {
                if ( -l $myfile)
                {
                    next;
                }
                # if ( $myfile eq "help" ||
                #      $myfile eq "presets" ||
                #      $myfile eq "registry" ||
                #      $myfile eq "uno_packages" ||
                #      $myfile eq "lib" ||
                #      $myfile eq "user_tree" )
                # {
                #     next;
                # }

                if ($myfile ne "." && $myfile ne "..")
                {
                    my $sNewStartDir = appendPath($startdir, $myfile);    # neuen Directorystring erstellen
                    # if ($sNewStartDir =~ "^\/proc" ||
                    #     $sNewStartDir =~ "^\/dev" ||
                    #     $sNewStartDir =~ "^\/udev" ||
                    #     $sNewStartDir =~ "lost+found" )
                    # {
                    #     next;
                    # }
                    # my $sNewSUserParameter  = $sUserParameter . $myfile ."/";
                    # do a recursive call
                    # $nCountFiles++;
                    my $nFileCount = readdirectory($sNewStartDir, $sUserParameter, $hook);
                    # workOnDir($sNewDir, $nFileCount);
                    $nCountFiles += $nFileCount;

                    chdir ($startdir);                      # zurueckwechseln.
                    cwd();
                }
            }
            else
            {
                # File must exist, be a regular file and must not be the $onlyOnFile
                if (-f $myfile)
                {
                    # print STDERR " $startdir" . "$myfile\n";
                    $nCountFiles++;
                    # workOnFile($startdir, $myfile, $destdir);
                    $hook->($startdir, $myfile, $sUserParameter);
                }
            }
            #}
            #else
            #{
            #    print STDERR "linked file: $dir/$myfile\n";
            #}
        }
        closedir(DIR);
    }
    else
    {
        print STDERR "could not open $startdir\n";
    }
    return $nCountFiles;
}

our $lcl_sSofficeBinPath;
our $lcl_sSofficeBinName;

sub searchSofficeBin($$$)
{
    my $currentDir = shift;
    my $currentFile = shift;
    my $sUserParameter = shift;

    if ($currentFile eq $sUserParameter)
    {
        my $sSourceFilename;
        $sSourceFilename = appendPath($currentDir, $currentFile);

        if ( -e "$sSourceFilename" )
        {
            $lcl_sSofficeBinPath = $currentDir;
            $lcl_sSofficeBinName = $currentFile;
        }
    }
}

# our $lcl_sUnoPkgPath;
#
# sub searchUnoPkgBin($$$)
# {
#     my $currentDir = shift;
#     my $currentFile = shift;
#     my $sUserParameter = shift;
#
#     if ($currentFile eq $sUserParameter)
#     {
#         my $sSourceFilename;
#         $sSourceFilename = appendPath($currentDir, $currentFile);
#         if ( -e "$sSourceFilename" )
#         {
#             $lcl_sUnoPkgPath = $currentDir;
#         }
#     }
# }

# our $lcl_sJARPath;

# sub searchJARFile($$$)
# {
#     my $currentDir = shift;
#     my $currentFile = shift;
#     my $sUserParameter = shift;
#
#     if ($currentFile eq $sUserParameter)
#     {
#         my $sSourceFilename;
#         $sSourceFilename = appendPath($currentDir, $currentFile);
#         if ( -e "$sSourceFilename" )
#         {
#             $lcl_sJARPath = $currentDir;
#         }
#     }
# }

# return the PATH, where the file was found
# sub searchForJAR($$)
# {
#     my $sPathToInstallOffice = shift;
#     my $sJARFileName = shift;
#
#     my $sCurrentPath = cwd();
#
#     $lcl_sJARPath = "";
#     readdirectory(${sPathToInstallOffice}, ${sJARFileName}, \&searchJARFile);
#
#     chdir $sCurrentPath;
#     cwd();
#
#     return $lcl_sJARPath;
# }

# sub getUnoPkg($)
# {
#     my $sPathToInstallOffice = shift;
#
#     my $sUnoPkgName = "unopkg.bin";
#     if (isWindowsEnvironment())
#     {
#         $sUnoPkgName = "unopkg.exe";
#     }
#
#     my $sCurrentPath = cwd();
#
#     $lcl_sUnoPkgPath = "";
#     readdirectory(${sPathToInstallOffice}, ${sUnoPkgName}, \&searchUnoPkgBin);
#
#     chdir $sCurrentPath;
#     cwd();
#
#     return ($lcl_sUnoPkgPath, $sUnoPkgName);
# }

sub getSofficeExe($)
{
    my $sPathToOffice = shift;

    my $sSofficeExeName = "soffice";
    if (isWindowsEnvironment())
    {
        $sSofficeExeName = "soffice.exe";
    }

    my $sCurrentPath = cwd();

    $lcl_sSofficeBinPath = "";
    $lcl_sSofficeBinName = "";
    readdirectory(${sPathToOffice}, ${sSofficeExeName}, \&searchSofficeBin);

    chdir $sCurrentPath;
    cwd();

    return ($lcl_sSofficeBinPath, $lcl_sSofficeBinName);
}

# sub checkOfficeAlreadyInstalled($)
# {
#     my $sOfficePath = shift;
#
#     my $sCurrentPath = cwd();
#
#     $lcl_sSofficeBinPath = "";
#     my $sOldOfficePath = appendPath($sOfficePath, "program");
#     if ( -d "$sOldOfficePath" )
#     {
#         $sOldOfficePath = appendPath($sOldOfficePath, "soffice.bin");
#         if ( -e $sOldOfficePath )
#         {
#             return 1;
#         }
#     }
#     else
#     {
#         if (isWindowsEnvironment())
#         {
#             my $sThreeLayerOffice = appendPath($sOfficePath, "Sun");
#             $sThreeLayerOffice = appendPath($sThreeLayerOffice, "StarOffice 9");
#             $sThreeLayerOffice = appendPath($sThreeLayerOffice, "program");
#             $sThreeLayerOffice = appendPath($sThreeLayerOffice, "soffice.bin");
#
#             if ( -e "$sThreeLayerOffice" )
#             {
#                 return 1;
#             }
#         }
#         else
#         {
#             my $sThreeLayerOffice = appendPath($sOfficePath, "staroffice9");
#             $sThreeLayerOffice = appendPath($sThreeLayerOffice, "program");
#             $sThreeLayerOffice = appendPath($sThreeLayerOffice, "soffice.bin");
#             if ( -e "$sThreeLayerOffice" )
#             {
#                 return 1;
#             }
#         }
#     }
#
#     # soffice.bin not found in fast path
#     readdirectory($sOfficePath, "soffice.bin", \&searchSofficeBin);
#     chdir $sCurrentPath;
#     cwd();
#
#     if ( $lcl_sSofficeBinPath ne "" )
#     {
#         return 1;
#     }
#     return 0;
#     # this is the old check
#     # my $sOfficePathCheck = appendPath(${sPathToInstallOffice}, "program");
#     # $sOfficePathCheck = appendPath($sOfficePathCheck, "soffice.bin");
#     # if ( -e $sOfficePathCheck )
#     # {
#     #     return 1;
#     # }
#     #
#     # # check path system of tree layer office
#     # if ( isWindowsEnvironment() )
#     # {
#     #     $sOfficePathCheck = appendPath(${sPathToInstallOffice}, "Sun");
#     #     if ( ! -e $sOfficePathCheck)
#     #     {
#     #         # could be an OpenOffice.org
#     #         return 0;
#     #     }
#     #     else
#     #     {
#     #
#     #         $sOfficePathCheck = appendPath($sOfficePathCheck, "StarOffice 9");
#     #         $sOfficePathCheck = appendPath($sOfficePathCheck, "program");
#     #         $sOfficePathCheck = appendPath($sOfficePathCheck, "soffice.bin");
#     #         if ( -e $sOfficePathCheck )
#     #         {
#     #             return 1;
#     #         }
#     #         print "Error: There exist no Office, maybe an unsupported version?\n";
#     #     }
#     # }
#     # elsif ( isUnixEnvironment() )
#     # {
#     #     $sOfficePathCheck = appendPath(${sPathToInstallOffice}, "staroffice9");
#     #     $sOfficePathCheck = appendPath($sOfficePathCheck, "staroffice9");
#     #     $sOfficePathCheck = appendPath($sOfficePathCheck, "program");
#     #     $sOfficePathCheck = appendPath($sOfficePathCheck, "soffice.bin");
#     #     if ( -e $sOfficePathCheck )
#     #     {
#     #         return 1;
#     #     }
#     #     print "Error: There exist no Office, maybe an unsupported version?\n";
#     # }
#     # else
#     # {
#     #     print "Error: There exist no Office, maybe an unsupported version?\n";
#     # }
#     # return 0;
# }

1;
