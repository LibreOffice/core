: # -*- perl -*-
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
# create setup self extracting script

if( $#ARGV < 2 )
  {
    print <<ENDHELP;
USAGE: $0 <inputshellscript> <libraryfile> <outputshellscript>
    <inputshellscript>: the start shell script, located next to this perl script
    <libraryfile>: the library file, that is included into the shell script
    <outfile>: the target shellscript

ENDHELP
  exit;
  }

$infile     = $ARGV[0];
$library    = $ARGV[1];
$outfile    = $ARGV[2];
$infile     =~ tr/[A-Z]/[a-z]/;

# read script header
open( SCRIPT, "<$infile" ) || die "cannot open $infile";
open( OUTFILE, ">$outfile$$.tmp" ) || die "cannot open $outfile";
@scriptlines = <SCRIPT>;
$linenum = $#scriptlines+2;
foreach (@scriptlines)
{
  # lineend conversion (be on the safe side)
  chomp;
  $_ =~ tr/\r//;
  s/^\s*linenum=.*$/linenum=$linenum/;
  print OUTFILE "$_\n";
}
close( SCRIPT );
close( OUTFILE );

system( "cat $library >>$outfile$$.tmp" );
rename "$outfile$$.tmp", "$outfile";

chmod 0775, $outfile;

exit;
