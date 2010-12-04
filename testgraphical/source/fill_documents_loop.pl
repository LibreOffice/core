eval 'exec perl -wS $0 ${1+\"$@\"}'
    if 0;

# This program has to start for the new convwatch,
# once on Windows environment and once on Linux environment
# Solaris is handled by the linux also.
#
# This program polls the database (documentcompare) every 60s for new jobs
# it runs over the given directory from documentpoolpath and pool, and create for every file
# a new database entry in documents.
#

BEGIN
{
        #       Adding the path of this script file to the include path in the hope
        #       that all used modules can be found in it.
        $0 =~ /^(.*)[\/\\]/;
        push @INC, $1;
}

use ConvwatchHelper;
use CallExternals;
use stringhelper;
use filehelper;
use oshelper;
use timehelper;
use cwstestresulthelper;

use strict;
use Cwd;
use File::Basename;
use English;                  # $OSNAME, ...
use Getopt::Long;
use File::Path;
use Cwd 'chdir';

my $cwd = getcwd();

our $help;                    # Help option flag
our $version;                 # Version option flag
our $test;

our $version_info = 'convwatch.pl $Revision: 1.24 $ ';

our $SOLARENV;
our $COMMON_ENV_TOOLS;


our $documentpoolname;
our $documentpoolpath;
our $dbdistinct;
our $sParentDistinct;
our $sCurrentDocumentPool;

our $fs;
our @aEntries;

# Prototypes
# sub getJavaFileDirSeparator();
sub readdirectory($$$);
sub putDocumentInDB($$$);

# flush STDOUT
my $old_handle = select (STDOUT); # "select" STDOUT and save # previously selected handle
$| = 1; # perform flush after each write to STDOUT
select ($old_handle); # restore previously selected handle

setPrefix("gfxcmp");

if (!GetOptions(
                "test"          => \$test,
                "help"          => \$help,
                "version"       => \$version
                ))
{
    print_usage(*STDERR);
    exit(1);
}
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

# ------------------------------------------------------------------------------
# within mysql it is better to use only '/'
$fs = "/"; # getJavaFileDirSeparator();
# ------------------------------------------------------------------------------
sub readdirectory($$$)
{
    my $startdir = shift;
    my $sValues  = shift;
    my $hook     = shift;

    my $myfile;

    local *DIR;
    chdir $startdir;
    cwd();
    if (! endswith($startdir, $fs))
    {
        $startdir .= $fs;
    }

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
                if ($myfile ne "." && $myfile ne "..")
                {
                    my $sNewStartDir = $startdir . $myfile ."/";    # neuen Directorystring erstellen
                    if ($sNewStartDir =~ "^\/proc" ||
                        $sNewStartDir =~ "^\/dev" ||
                        $sNewStartDir =~ "^\/udev" ||
                        $sNewStartDir =~ "lost+found" )
                    {
                        next;
                    }
                    # my $sNewDestDir  = $destdir . $myfile ."/";
                    # do a recursive call
                    # $nCountFiles++;
                    my $nFileCount = readdirectory($sNewStartDir, $sValues, $hook);
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
                    # print " $startdir" . "$myfile\n";
                    $nCountFiles++;
                    # workOnFile($startdir, $myfile, $destdir);
                    $hook->($startdir, $myfile, $sValues);
                }
            }
            #}
            #else
            #{
            #    print "linked file: $dir/$myfile\n";
            #}
        }
        closedir(DIR);
    }
    else
    {
        print "could not open $startdir\n";
    }
    return $nCountFiles;
}
# ------------------------------------------------------------------------------
sub putDocumentInDB($$$)
{
    my $currentDir = shift;
    my $currentFile = shift;
    my $sValues = shift;

    my $sSourceFilename = $currentDir . $currentFile;
    # we cut down all the previous names like documentpoolpath and the documentpoolname
    $sSourceFilename = substr($sSourceFilename, length($sCurrentDocumentPool . $fs));

    my $sSQL = "INSERT INTO documents (dbdistinct2, name, pagecount, priority, parentdistinct) VALUES";
    $sSQL .= "('" . $dbdistinct . "', '" . $sSourceFilename . "', 0, 1, '". $sParentDistinct . "')";
    # print $sSQL . "\n";

    push(@aEntries, $sSQL);
    # ExecSQL($sSQL);
}

# ------------------------------------------------------------------------------
sub createDBEntriesForEveryDocument($)
{
    my $sStr = shift;
    if ($sStr =~ /^MySQL-Error/ )
    {
        # we don't do anything if an error occurred
        return;
    }

    # interpret the follows string
    # documentpoolpath='//so-gfxcmp-documents/doc-pool', documentpool='demo_lla', dbdistinct=62,

    # my $sDocumentPoolDir;
    if ( $sStr =~ /documentpoolpath='(.*?)',/ )
    {
        $documentpoolpath = $1;
    }
    if (! $documentpoolpath)
    {
        print "Error: no value for documentpoolpath found.\n";
        return;
    }

    # my $sDocumentPool;
    if ( $sStr =~ /documentpool='(.*?)',/ )
    {
        $documentpoolname = $1;
    }
    if (! $documentpoolname)
    {
        print "Error: no value for documentpool found.\n";
        return;
    }
    # my $dbdistinct;
    if ( $sStr =~ /dbdistinct2='(\S*?)',/ )
    {
        $dbdistinct = $1;
    }
    if (! $dbdistinct)
    {
        print "Error: no dbdistinct given.\n";
        return;
    }

    if (! -d $documentpoolpath )
    {
        my $sEnv = getEnvironment();
        if ( isUnixEnvironment() )
        {
            $documentpoolpath = "/net/so-gfxcmp-documents" . $documentpoolpath;
        }
        if ( -d $documentpoolpath )
        {
            print "Warning: given documentpoolpath seems to be local, fix to '$documentpoolpath'\n";
            my $sSQL = "UPDATE documentcompare SET documentpoolpath='$documentpoolpath' WHERE dbdistinct2='$dbdistinct'";
            print "$sSQL\n";
            ExecSQL($sSQL);
        }
        else
        {
            print "Error: documentpoolpath '$documentpoolpath' not found. Don't insert anything.\n";
            my $sSQL = "UPDATE documentcompare SET state='failed',info='documentpoolpath not found.' WHERE dbdistinct2='$dbdistinct'";
            print "$sSQL\n";
            ExecSQL($sSQL);
            return;
        }
    }
    # create the documentpool directory, to run through
    $sCurrentDocumentPool = $documentpoolpath;
    if (! endswith($sCurrentDocumentPool, $fs))
    {
        $sCurrentDocumentPool .= $fs;
    }
    $sCurrentDocumentPool .= $documentpoolname;

    if ( -d $sCurrentDocumentPool )
    {
        if ( $sStr =~ /parentdistinct='(.*?)',/ )
        {
            $sParentDistinct = $1;
        }
        else
        {
            $sParentDistinct = "";
        }

        # remove any doubles, if any
        my $sSQL = "DELETE FROM documents WHERE dbdistinct2='$dbdistinct'";
        print "$sSQL\n";
        ExecSQL($sSQL);

        # run over the whole given document pool and store every found document name in the database
        readdirectory($sCurrentDocumentPool, "", \&putDocumentInDB);

        chdir $cwd;
        cwd();

        foreach $sSQL (@aEntries)
        {
            # print "# $sSQL\n";
            print "$sSQL\n";
            ExecSQL($sSQL);
        }

        my $sSQL = "UPDATE documentcompare SET state='inprogress' WHERE dbdistinct2='$dbdistinct'";
        print "$sSQL\n";
        ExecSQL($sSQL);
        print "----------------------------------------------------------------------\n";
        $sParentDistinct = "";
        @aEntries = ();
    }
    else
    {
        print "Error: Given document pool '$sCurrentDocumentPool' doesn't exists.\n";
        my $sSQL = "UPDATE documentcompare SET state='cancelled' WHERE dbdistinct2='$dbdistinct'";
        ExecSQL($sSQL);
        return;
    }
    # Send Mail, due to startconvwatch now
    sendMail($sStr, $documentpoolname, $dbdistinct);
}

# ------------------------------------------------------------------------------
sub sendMail($$$)
{
    my $sStr = shift;
    my $documentpool = shift;
    my $dbdistinct = shift;
    my $sourceversion;
    if ( $sStr =~ /sourceversion='(.*?)',/ )
    {
        $sourceversion = $1;
    }
    if (! $sourceversion)
    {
        print "Warning: no value for sourceversion found.\n";
        return;
    }
    my $destinationversion;
    if ( $sStr =~ /destinationversion='(.*?)',/ )
    {
        $destinationversion = $1;
    }
    if (! $destinationversion)
    {
        print "Warning: no value for destinationversion found.\n";
        return;
    }
    my $mailaddress;
    if ( $sStr =~ /mailfeedback='(.*?)',/ )
    {
        $mailaddress = $1;
    }
    if (! $mailaddress)
    {
        print "Warning: no value for mailfeedback found.\n";
        return;
    }

    # state is 'inprogress', so send a mail
    # my $sMailAddress = getMailAddress($sDoneStr);
    my $sParams = "$sourceversion";
    $sParams .= " $destinationversion";
    $sParams .= " $documentpool";
    $sParams .= " $dbdistinct";
    $sParams .= " $mailaddress";
    $sParams .= " starts";                    # run through state of convwatch

    my $sMailProgram = appendPath(getQADEVToolsPath(), "mailsend.php");

    my $err;
    my @lines;
    my $sLine;
    ($err, @lines) = callphp(getPHPExecutable(), $sMailProgram, $sParams);
    foreach $sLine (@lines)
    {
        log_print( "Mail: $sLine\n");
    }

    if ($documentpool eq "EIS-tests")
    {
        cwstestresult("running", $dbdistinct, $sourceversion, $destinationversion, $SOLARENV, $COMMON_ENV_TOOLS);
    }
}
# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------

my $sEnvironmentCondition;
if (isWindowsEnvironment())
{
    $sEnvironmentCondition = "environment='" . getEnvironment() . "'";
}
elsif (isUnixEnvironment())
{
    # $sEnvironmentCondition = " ( environment='unxlngi' OR environment='unxsoli' ) ";
    $sEnvironmentCondition = " environment='" . getEnvironment() . "'";
}
else
{
    print "Error: wrong environment.\n";
    exit(1);
}
my $sWhereClause = "WHERE ";
if ($sEnvironmentCondition)
{
    $sWhereClause .= $sEnvironmentCondition . " AND ";
}
$sWhereClause .= " state='new'";

setToolsPath(getQADEVToolsPath());

# ---------------------------------- main loop ----------------------------------
while (1)
{
    my @aResult;
    my $sSQL = "SELECT documentpoolpath,documentpool,dbdistinct2,sourceversion,destinationversion,mailfeedback,parentdistinct FROM documentcompare $sWhereClause";
    @aResult = ExecSQL($sSQL);

    my $aValue;
    foreach $aValue (@aResult)
    {
        # print "# $nValue\n";
        createDBEntriesForEveryDocument($aValue);
    }
    if ($test)
    {
        last;
    }

    # wait 30sec.
    # wait30seconds();
    waitAMinute();
    checkForStop("stop_fill_documents_loop");
}
