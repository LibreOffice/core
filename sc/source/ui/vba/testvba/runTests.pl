#!/usr/bin/perl -w
use URI::Escape;
use File::Basename;
use Cwd;
use Cwd 'abs_path';

$numArgs = $#ARGV + 1;
print "thanks, you gave me $numArgs command-line arguments.\n";

foreach $argnum (0 .. $#ARGV) {
   print "$ARGV[$argnum]\n";
}


my $binDir = abs_path( dirname($0) );

my $sysDir = "unix";
my $fileSep = "/";
my $theResult;
my $officepath = shift || die "please specify path to office installation program dir";
my $DocName = shift || "";
my $programpath = "$officepath"."3/program:$officepath/program:";
my $basiclibrarypath = "$officepath/basis3.1/program";
my $urelibpath = "$officepath/ure/lib";
my $binext = "";
my $testDocDir = "$binDir/TestDocuments";
my $testLogDir = "$binDir/Logs";
my $testclientname = "testclient";
my $buildtestclient = "../../../../unxlngi6.pro/bin/$testclientname";

# test testclient
if ( -e "$buildtestclient" )
{
    print "use the latest build\n";
    system( "cp $buildtestclient ." );
}
elsif ( !( -e "$testclientname" ) )
{
    print "$testclientname do not exist\n";
    exit;
}

# test for uname
system("uname");
$exit_value  = $? >> 8;
$signal_num  = $? & 127;
$dumped_core = $? & 128;

$failed = ( $exit_value || $signal_num || $dumped_core );

print "$failed = ( $exit_value || $signal_num || $dumped_core )\n";

if ( !$failed && open(UNAME, "uname -a|") ) {
   $theResult = <UNAME>;
   close(UNAME);
   if (  $theResult =~ /^CYGWIN/  ) {
      # windows under cygwin
      $sysDir = "win" ;
      $tmpPath=$ENV{"PATH"};
      $ENV{"PATH"} = "$officepath:$tmpPath";
      $testDocDir=`cygpath -m  $testDocDir`;
      uri_escape($testDocDir);
      # hacky windows url construction
      $testDocDir="file:///$testDocDir";

      chomp($testDocDir);
      #print "*** doc dir is $testDocDir\n";
      $testLogDir = `cygpath -m  "$testLogDir"`;
      uri_escape($testLogDir);
      $testLogDir="file:///$testLogDir";
      chomp($testLogDir);
      #print "*** log dir is $testLogDir\n";
      $binext = ".exe";
   }
   else{
      # unix we need to find sal etc. ( from the office path )
      my $tmpPath=$ENV{"PATH"};
      $ENV{"PATH"} = "$programpath:$basiclibrarypath:$urelibpath/../bin:$tmpPath";
      $tmpPATH = $ENV{"LD_LIBRARY_PATH"};
      $ENV{"LD_LIBRARY_PATH"} = "$officepath:$programpath:$basiclibrarypath:$urelibpath:$urelibpath../bin/javaldx:$urelibpath/../bin:$tmpPATH";
      $ENV{"LD_LIBRARY_PATH"} = "$officepath:$programpath:$basiclibrarypath:$urelibpath:$tmpPATH";
      my $testPath = $ENV{"LD_LIBRARY_PATH"};
      print "$testPath\n";
      $testPath = $ENV{"PATH"};
      print "$testPath\n";
      $ENV{"STAR_RESOURCEPATH"} = "$officepath/basis3.0/program/resource";
      $ENV{"SAL_ALLOW_LINKOO_SYMLINKS"} = "1";
      $testPath = $ENV{"LANG"};
      print "$testPath\n";
   }
}
else
{
      # ordinary windows, not sure if this will actually work
      $sysDir = "win" ;
      $tmpPath=$ENV{"PATH"};
      $ENV{"PATH"} = "$tmpPath;$officepath";
      $binext = ".exe";
}

# the exe needs system paths or urls ( urls are by far the least troublesome )

my $runCmd = "";
my $analyseCmd = "";

if ( "$DocName" eq "" )
{
    $runCmd = "$binDir/testclient$binext $testDocDir $testLogDir";
    $analyseCmd = "perl $binDir/testResults.pl $binDir/Logs $binDir/TestDocuments/logs/$sysDir";
}
else
{
    $runCmd = "$binDir/testclient$binext $testDocDir $testLogDir $testDocDir/$DocName";
    $analyseCmd = "perl $binDir/testResult.pl $binDir/Logs $binDir/TestDocuments/logs/$sysDir $DocName";
}
print "runCmd = $runCmd\n";

system ("rm -rf $testLogDir/*");
my $status = system( $runCmd );
print "analyseCmd = $analyseCmd\n";
$status = system( $analyseCmd );
