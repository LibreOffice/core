#!/usr/bin/perl
#*************************************************************************
#*
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: filter.pl,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 19:31:34 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#************************************************************************/

$debug = "";
$ctrue = 1;
$cfalse = 0;
# reads a block
# Parameter:   FileHandle
#              list of Regular Expressions which terminate the block.
#   for '#ifdef' block would then be ("^#else\$", "^#endif\$")

sub read_block {

  local($file) = @_;
  print "reading block '$file' ",scalar(@_),"\n" if $debug;
  while ( <$file> ) {
    chop;
    s/\s*$//;             # remove trailing whitespaces
    s/^\s*//;             # remove leading whitespaces
    print "Input : \"$_\"\n" if $debug;
    s/\/\/.*//;         # Remove line comment
    s/\/\*.*?\*\///g;   # remove comments within one line
    s/\s+/ /g;          # Change all whitespace into a single blank
    s/ *$//;            # Remove whitespace at end
    s/^# /#/;           # Change # <command> to #<command>


    @line = split(/ /,$_,3);

    $_ = $line[0];
    if ( $_ && /^#/ ) {                # Line starts with '#' -> preprocessor command
      print "proccessing line: @line\n" if $debug;
      if (/#define/)
      {
        if ( $line[1] =~ /^$namefilter/ )
        {
          $mykey = $line[1];
          $mykey =~ s/^$namefilter//;
          $count += 1;
          print OUT "$mykey    ", $line[2], "\n";
          print OUT2 "\t{ \"$mykey\", ", $line[2] ," },\n";
        }
      }
    }
  }
  print "Leaving read_block at the end\n" if $debug;
}

sub convert_path {

  $_ = $_[0];
  $GUI = $ENV {"GUI"};
  $use_shell = $ENV {"USE_SHELL"};
  if ( $GUI eq "WNT" )
  {
    if ( defined( $use_shell ) && "$use_shell" eq "4nt" )
    {
      s/\//\\/g;
    }
  }
  $_;
}

# Read a file.
# first parameter ist the filename
sub read_file {

  local ($filename,$file) = @_;
  $file++;                           # String increment
  local $TempFileName = &convert_path( $basename."/".$filename );
  print "reading file $TempFileName as $file\n" if $debug;
  open($file, $TempFileName) || die "error: Could not open file $TempFileName. ";

  &read_block($file);         # read data
  close($file);
  print "done reading $filename\n" if $debug;
}

# main starts here

print &convert_path ("/\n\n\n") if ( $debug );


$basename = ".";
$basename = $ARGV[0] if defined($ARGV[0]);

$filename = "app.hrc";
$filename = $ARGV[1] if defined($ARGV[1]);


$outfilebase = $filename;
$outfilebase =~ s/\.[^.]+$//;           # cut off suffix
$outfilebase = $ARGV[2] if defined($ARGV[2]);


$namefilter = $ARGV[3] if defined($ARGV[3]);


print "Generating $outfilebase:\n";

$count = 0;

open(OUT,">$outfilebase");
open(OUT2,">$outfilebase.hxx");
print OUT2 "\{\n";

&read_file ($filename,"f00");

print OUT2 "\t{ \"\" ,0 }\n\};\n";

close(OUT);
close(OUT2);

if ( $count == 0 )
{
  die "Error: No Entries Found generating \"$outfilebase.hxx\". Testtool will not work!"
}

