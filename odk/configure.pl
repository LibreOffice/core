#
# configure.pl - a perl script to set a minimal environment for the SDK.
#
# Copyright (c) 2000 Sun Microsystems, Inc.
#

use IO::File;

$main::OO_SDK_NAME=readSDKName();
$main::currentWorkingDir = `pwd`;
chop ($main::currentWorkingDir);
$main::operatingSystem = `uname -s`;
chop ($main::operatingSystem);
$main::OO_SDK_HOME = "";
$main::OO_SDK_HOME_SUGGESTION = $main::currentWorkingDir;

$main::OFFICE_HOME = "";
$main::OFFICE_HOME_SUGGESTION = searchprog("soffice");
$main::tmpOffice = readlink "$main::OFFICE_HOME_SUGGESTION/soffice";
if ( ! $main::tmpOffice eq "" )
{
    my $offset = rindex($main::tmpOffice, "/program/soffice");
    if ( $main::offset != -1 )
    {
    $main::OFFICE_HOME_SUGGESTION = substr($main::tmpOffice, 0, $offset);
    } else
    {
    $offset = rindex($main::tmpOffice, "/soffice");
    if ( $main::offset != -1 )
    {
        $main::OFFICE_HOME_SUGGESTION = substr($main::tmpOffice, 0, $offset);
    }
    }
}

$main::OO_SDK_MAKE_HOME = "";
$main::OO_SDK_MAKE_HOME_SUGGESTION = searchprog("make");
$main::makeVersion = "3.79.1";
$main::correctVersion = 0;

$main::OO_SDK_CPP_HOME = "";
$main::cppName = "gcc";
$main::cppVersion = "3.0.1";
if ( $main::operatingSystem eq "SunOS" )
{
    $main::cppName = "CC";
    $main::cppVersion = "5.2";
}

$main::OO_SDK_CPP_HOME_SUGGESTION = searchprog($main::cppName);

$main::OO_SDK_JAVA_HOME = "";
$main::OO_SDK_JAVA_HOME_SUGGESTION = searchprog("javac");
$main::javaVersion = "1.4.1_01";

$main::SDK_AUTO_DEPLOYMENT = "";
$main::SDK_AUTO_DEPLOYMENT_SUGGESTION = "YES";

$main::SDK_AUTO_DEPLOYMENT = "";
$main::SDK_AUTO_DEPLOYMENT_SUGGESTION = "YES";

$main::OO_SDK_OUTPUT_DIR = "";
#$main::OO_SDK_OUTPUT_DIR = '$HOME';
$main::OO_SDK_OUTPUT_DIR = "";
$main::skipOutputDir = 0;

$main::return = 0;

# prepare SDK path
while ( (! -d "$main::OO_SDK_HOME" ) ||
        ((-d "$main::OO_SDK_HOME") && (! -d "$main::OO_SDK_HOME/idl")) )
{
    print " Enter the Office Software Development Kit directory [$main::OO_SDK_HOME_SUGGESTION]: ";
    $main::OO_SDK_HOME = readStdIn();
    chop($main::OO_SDK_HOME);
    if ( $main::OO_SDK_HOME eq "" )
    {
        $main::OO_SDK_HOME = $main::OO_SDK_HOME_SUGGESTION;
    }
    if ( (! -d "$main::OO_SDK_HOME") ||
         ((-d "$main::OO_SDK_HOME") && (! -d "$main::OO_SDK_HOME/idl")) )
    {
        $main::OO_SDK_HOME = "";
        print " Error: An SDK is required, please specify the path to a valid installation.\n";
    }
}

# prepare Office path
while ( (! -d "$main::OFFICE_HOME" ) ||
        ((-d "$main::OFFICE_HOME") && (! -d "$main::OFFICE_HOME/program")) )
{
    print " Enter the Office installation directory [$main::OFFICE_HOME_SUGGESTION]: ";
    $main::OFFICE_HOME = readStdIn();
    chop($main::OFFICE_HOME);
    if ( $main::OFFICE_HOME eq "" )
    {
        $main::OFFICE_HOME = $main::OFFICE_HOME_SUGGESTION;
    }

    if ( ! -d "$main::OFFICE_HOME" )
    {
        $main::OFFICE_HOME = "";
        print " Error: An office installation is required, please specify the path to a valid installation.\n";
    } else
    {
        # special work for a network installation, no prgram directory but a link to the soffice binary
        if ( (! -d "$main::OFFICE_HOME/program") && (-e "$main::OFFICE_HOME/soffice") )
        {
            my $soserver = `ls -l $OFFICE_HOME_SUGGESTION/soffice | sed -n 's/.* -> //p'`;
            $soserver= substr($soserver, 0, rindex($soserver, "program") - 1);

            if ( ! -d $soserver )
            {
                $main::OFFICE_HOME = "";
                print " Error: An office installation is required, please specify the path to a valid installation.\n";
            } else
            {
                $main::OFFICE_HOME = $soserver;
            }
        }
    }
}

# prepare GNU make path
while ( (!$main::correctVersion) &&
        ((! -d "$main::OO_SDK_MAKE_HOME" ) ||
         ((-d "$main::OO_SDK_MAKE_HOME") && (! -e "$main::OO_SDK_MAKE_HOME/make"))) )
{
    print " Enter GNU make ($main::makeVersion or higher) tools directory [$main::OO_SDK_MAKE_HOME_SUGGESTION]: ";
    $main::OO_SDK_MAKE_HOME = readStdIn();
    chop($main::OO_SDK_MAKE_HOME);
    if ( $main::OO_SDK_MAKE_HOME eq "" )
    {
        $main::OO_SDK_MAKE_HOME = $main::OO_SDK_MAKE_HOME_SUGGESTION;
    }
    if ( (! -d "$main::OO_SDK_MAKE_HOME") ||
         ((-d "$main::OO_SDK_MAKE_HOME") && (! -e "$main::OO_SDK_MAKE_HOME/make")) )
    {
        $main::OO_SDK_MAKE_HOME = "";
        print " Error: GNU make is required, please specify a GNU make tools directory.\n";
    } else
    {
        #check version
        my $testVersion = `$OO_SDK_MAKE_HOME/make --version`;
        if ( $testVersion eq "")
        {
        $main::offset = rindex($main::OO_SDK_JAVA_HOME_SUGGESTION, "/bin");
if ( $main::offset != -1 )
{
    $main::OO_SDK_JAVA_HOME_SUGGESTION = substr($main::OO_SDK_JAVA_HOME_SUGGESTION, 0, $main::offset);
}
    print " The 'make' command found at $main::OO_SDK_MAKE_HOME/make is not GNU Make\n";
            print " Set the environment variable OO_SDK_MAKE_HOME to your GNU build tools directory.\n";
            print " GNU make version $main::makeVersion can be obtained at ftp://ftp.gnu.org/gnu/make/\n";
        } else
        {
            if ($testVersion =~ m#((\d+\.)+\d+)# )
            {
                $testVersion = $1;
            }
            $main::correctVersion = testVersion($main::makeVersion, $testVersion, "$main::OO_SDK_MAKE_HOME/make");
            if ( !$main::correctVersion )
            {
                print " The 'make' command found at '$main::OO_SDK_MAKE_HOME' has a wrong version\n";
                $main::OO_SDK_MAKE_HOME = "";
            }
        }
    }
}

# prepare C++ compiler path
$main::correctVersion = 0;
while ( (!$main::correctVersion) &&
        ((! -d "$main::OO_SDK_CPP_HOME" ) ||
         ((-d "$main::OO_SDK_CPP_HOME") && (! -e "$main::OO_SDK_CPP_HOME/$main::cpp"))) )
{
    print " C++ compilers where for example a language binding exist:\n";
    print "  - Solaris, Sun WorkShop 6 update 1 C++ 5.2 2000/09/11 or higher\n";
    print "  - Linux, GNU C++ compiler, gcc version 3.0.1 or higher\n";
    print " Enter the directory of the C++ compiler, the directory\n";
    print " where the compiler is located (optional) [$main::OO_SDK_CPP_HOME_SUGGESTION]: ";

    $main::OO_SDK_CPP_HOME = readStdIn();
    chop($main::OO_SDK_CPP_HOME);
    if ( $main::OO_SDK_CPP_HOME eq "" )
    {
        $main::OO_SDK_CPP_HOME = $main::OO_SDK_CPP_HOME_SUGGESTION;
    }$main::offset = rindex($main::OO_SDK_JAVA_HOME_SUGGESTION, "/bin");
if ( $main::offset != -1 )
{
    $main::OO_SDK_JAVA_HOME_SUGGESTION = substr($main::OO_SDK_JAVA_HOME_SUGGESTION, 0, $main::offset);
}

    if ( ! $main::OO_SDK_CPP_HOME eq "" )
    {
        if ( (! -d "$main::OO_SDK_CPP_HOME") ||
             ((-d "$main::OO_SDK_CPP_HOME") && (! -e "$main::OO_SDK_CPP_HOME/$main::cppName")) )
        {
            print " Error: Could not find directory '$main::OO_SDK_CPP_HOME'.\n";
            if ( skipChoice("C++ compiler") == 1 )
            {
                $main::correctVersion = 1;
            }
            $main::OO_SDK_CPP_HOME = "";
        } else
        {
            #check version
            if ( $main::cppName eq "gcc" )
            {
                my $testVersion = `$OO_SDK_CPP_HOME/$main::cppName -dumpversion`;
                if ( $testVersion eq "")
                {
                    print " The '$main::cppName' command found at $main::OO_SDK_CPP_HOME/$main::cppName is not a ";
                    print " GNU compiler.\nSet the environment variable OO_SDK_CPP_HOME to your GNU build tools ";
                    print " directory.\nA GNU compiler version $main::cppVersion can be obtained at ";
                    print " ftp://ftp.gnu.org/gnu/gcc/\n";
                } else
                {
                    $main::correctVersion = testVersion($main::cppVersion, $testVersion, "$main::OO_SDK_CPP_HOME/$main::cppName");
                    if ( !$main::correctVersion )
                    {
                        print " The '$main::cppName' command found at '$main::OO_SDK_CPP_HOME' has a wrong version\n";
                        if ( skipChoice("C++ compiler") == 1 )
                        {
                            $main::correctVersion = 1;
                        }

                        $main::OO_SDK_CPP_HOME = "";
                    }
                }
            } else
            {
                # for Solaris we have to check the version too
                open(FILE, "$OO_SDK_CPP_HOME/$main::cppName -V 2>&1 |");
                my @lines = <FILE>;
                my $testVersion = $lines[0];
                if ( $testVersion eq "")
                {
                    print " The '$main::cppName' command found at $main::OO_SDK_CPP_HOME/$main::cppName is not a ";
                    print " Solaris C++ compiler.\nSet the environment variable OO_SDK_CPP_HOME to your Solaris C++ compiler directory.\n";
                } else
                {
                    if ($testVersion =~ m#((\d+\.)+\d+)# )
                    {
                        $testVersion = $1;
                    }
                    $main::correctVersion = testVersion($main::cppVersion, $testVersion, "$main::OO_SDK_CPP_HOME/$main::cppName");
                    if ( !$main::correctVersion )
                    {
                        print " The '$main::cppName' command found at '$main::OO_SDK_CPP_HOME' has a wrong version\n";
                        if ( skipChoice("C++ compiler") == 1 )
                        {
                            $main::correctVersion = 1;
                        }

                        $main::OO_SDK_CPP_HOME = "";
                    }
                }
            }
        }
    } else
    {
        # the C++ compiler is optional
        $main::correctVersion = 1;
    }
}

# prepare Java path
$main::correctVersion = 0;

# prepare Java suggestion (cut bin directory to be in the root of the Java SDK)
$main::offset = rindex($main::OO_SDK_JAVA_HOME_SUGGESTION, "/bin");
if ( $main::offset != -1 )
{
    $main::OO_SDK_JAVA_HOME_SUGGESTION = substr($main::OO_SDK_JAVA_HOME_SUGGESTION, 0, $main::offset);
}

while ( (!$main::correctVersion) &&
         ((! -d "$main::OO_SDK_JAVA_HOME" ) ||
          ((-d "$main::OO_SDK_JAVA_HOME") && (! -e "$main::OO_SDK_JAVA_HOME/bin/javac"))) )
{
    print " Enter JAVA SDK (1.4.1_01 or higher) installation directory [$Main::OO_SDK_JAVA_HOME_SUGGESTION]: ";
    $main::OO_SDK_JAVA_HOME = readStdIn();
    chop($main::OO_SDK_JAVA_HOME);
    if ( $main::OO_SDK_JAVA_HOME eq "" )
    {
        $main::OO_SDK_JAVA_HOME = $main::OO_SDK_JAVA_HOME_SUGGESTION;
    }
    if ( (! -d "$main::OO_SDK_JAVA_HOME") ||
         ((-d "$main::OO_SDK_JAVA_HOME") && (! -e "$main::OO_SDK_JAVA_HOME/bin/javac")) )
    {
        $main::OO_SDK_JAVA_HOME = "";
        print " Error: A Java SDK is required, please specify a valid Java SDK directory.\n";
    } else
    {
        #check version
        my $testVersion = `$main::OO_SDK_JAVA_HOME/bin/java -version 2>&1 | egrep "java version" | head -1 | sed -e 's#.*version "##' | sed -e 's#".*##'`;
        $testVersion =~ s#([^\n]+)\n#$1#go;

        $main::correctVersion = testVersion($main::javaVersion, $testVersion, "$main::OO_SDK_JAVA_HOME/bin/java");
        if ( !$main::correctVersion )
        {
            $main::OO_SDK_JAVA_HOME = "";
        }
    }
}

# prepare output directory (optional)
while ( (!$main::skipOutputDir) &&
        (! -d "$main::OO_SDK_OUTPUT_DIR") )
{
    print " Default output directory is the SDK directory itself.\n";
    print " Enter an existent directory if you prefer a different output directory (optional) [$main::OO_SDK_OUTPUT_DIR]: ";

    $main::OO_SDK_OUTPUT_DIR = readStdIn();

    chop($main::OO_SDK_OUTPUT_DIR);
    if ( $main::OO_SDK_OUTPUT_DIR eq "" )
    {
        $main::OO_SDK_OUTPUT_DIR = $main::OO_SDK_OUTPUT_DIR_SUGGESTION;
    }
    if ( ! $main::OO_SDK_OUTPUT_DIR eq "" )
    {
        if ( ! -d "$main::OO_SDK_OUTPUT_DIR" )
        {
            print " Error: Could not find directory '$main::OO_SDK_OUTPUT_DIR'.\n";
            if ( skipChoice("optional output directory") == 1 )
            {
                $main::skipOutputDir = 1;
            }
            $main::OO_SDK_OUTPUT_DIR = "";
        }
    } else
    {
        # the output directory is optional
        $main::skipOutputDir = 1;
    }
}

# prepare auto deployment
while ( $main::SDK_AUTO_DEPLOYMENT eq "" ||
     ((! $main::SDK_AUTO_DEPLOYMENT eq "YES") &&
      (! $main::SDK_AUTO_DEPLOYMENT eq "NO")) )
{
    print " Automatic deployment of UNO components (YES/NO) [$main::SDK_AUTO_DEPLOYMENT_SUGGESTION]: ";
    $main::SDK_AUTO_DEPLOYMENT = <STDIN>;
    chop($main::SDK_AUTO_DEPLOYMENT);
    if ( $main::SDK_AUTO_DEPLOYMENT eq "" )
    {
    $main::SDK_AUTO_DEPLOYMENT = $main::SDK_AUTO_DEPLOYMENT_SUGGESTION;
    }
}


open ( FILEIN, "$main::currentWorkingDir/setsdkenv_unix.in" ) || die "\nERROR: could not open '$main::currentWorkingDir/setsdkenv_unix.in' for reading";
open ( FILEOUT, ">$main::currentWorkingDir/setsdkenv_unix" ) || die "\nERROR: could not open '$main::currentWorkingDir/setsdkenv_unix' for writing";

while ( <FILEIN> )
{
    $_ =~ s#\@OO_SDK_NAME\@#$main::OO_SDK_NAME#go;
    $_ =~ s#\@OO_SDK_HOME\@#$main::OO_SDK_HOME#go;
    $_ =~ s#\@OFFICE_HOME\@#$main::OFFICE_HOME#go;
    $_ =~ s#\@OO_SDK_MAKE_HOME\@#$main::OO_SDK_MAKE_HOME#go;
    $_ =~ s#\@OO_SDK_CPP_HOME\@#$main::OO_SDK_CPP_HOME#go;
    $_ =~ s#\@OO_SDK_JAVA_HOME\@#$main::OO_SDK_JAVA_HOME#go;
    $_ =~ s#\@SDK_AUTO_DEPLOYMENT\@#$main::SDK_AUTO_DEPLOYMENT#go;
    $_ =~ s#\@OO_SDK_OUTPUT_DIR\@#$main::OO_SDK_OUTPUT_DIR#go;

    print FILEOUT $_;
}

close FILEIN;
close FILEOUT;
chmod 0755, "$main::currentWorkingDir/setsdkenv_unix";

print "\n";
print " *****************************************************************\n";
print " * ... \"setsdkenv_unix\" has been prepared.                       *\n";
print " * For each time you want to use this configured SDK environment *\n";
print " * environment, please run the \"setsdkenv_unix\" script file!     *\n";
print " *****************************************************************\n\n";

exit $return;

sub skipChoice
{
    my $msg = shift;
    my $skip = "";
    while ( !( $skip eq "YES" || $skip eq "NO") )
    {
        print " Do you want to skip the choice of the '$msg' (YES/NO): [YES] ";
        $skip = <STDIN>;
        chop($skip);
        if ( $skip eq "" ) { $skip = "YES"; } # default
        if ( $skip eq "YES" )
        {
            return 1;
        }
    }
    return 0;
}

sub searchprog
{
    my $_search= shift;
    my $tmpPath = `echo "\$PATH"`;
    my @pathList = split(":" , $tmpPath);
    my $progDir = "";

    foreach $i (@pathList)
    {
        if ( -e "$i/$_search" )
        {
            if ( index($i, "/") == 0 )
            {
                # # absolute path; leave unchanged
                $progDir = $i;
            } else
            {
                $progDir = `cd "$i"; pwd`;
            }
            return $progDir
        }
    }
    return $progDir
}

sub testVersion
{
    my $tmpMustBeVersion = shift;
    my $tmpTestVersion = shift;
    my $toolName = shift;
    my @mustBeVersion = split(/\.|_|-/,$tmpMustBeVersion);
    my @testVersion = split(/\.|_|-/,$tmpTestVersion);
    my $length = $#mustBeVersion;

    if ($#testVersion < $#mustBeVersion) {
        $length = $#testVersion;
    }

    for ($i=0; $i <= $length; $i++ )
    {
        if ( @testVersion->[$i] < @mustBeVersion->[$i] )
        {
            print " The command '$toolName' has the version $tmpTestVersion.\n";
            print " The SDK requires at least the version $tmpMustBeVersion.\n";
            return 0;
        } else {
            if ( @testVersion->[$i] > @mustBeVersion->[$i] )
            {
                return 1; # 1 indicates a correct version
            }
        }
    }

    return 1; # 1 indicates a correct version
}

sub readSDKName
{
    my $mkfilename = "./settings/dk.mk";
    open ( FILEIN, $mkfilename ) || die "ERROR: could not open '$mkfilename' for reading";

    @lines = <FILEIN>;
    close( FILEIN );
    foreach $_ (@lines)
    {
        if( s#(SDKNAME=([\w]+)\n)#$1#go )
        {
            return $2;
            break;
        }
    }

    return "";
}

sub readStdIn
{
    my $tmpstdin = <STDIN>;
    if ( index($tmpstdin, "\$") != -1)
    {
        return `echo $tmpstdin`;
    }

    return $tmpstdin;
}
