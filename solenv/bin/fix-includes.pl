#!/usr/bin/env perl
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#
# fix-includes, a simple script replace local includes which should be global
# , to global includes. And global includes which should be local, to local includes.
# The script is expected to run in the root of the git repo, so it can fetch all the include directory's.
#
use strict;
use warnings;
use File::Basename;
use File::Find;
use IO::All;

my $dirname = "include";

# Fetch the list of includes
my @subdirs = map {basename $_} grep {-d} glob("$dirname/*");

# Add boost
push(@subdirs,"boost");

# Simple function to check and replace headers
sub check_headers
{
  my ($dir,$file, @includes) = @_;
  open(my $fh,"+<",$file) or die "Couldn't open file $file $!\n";
  my @content = <$fh>;
  my $line;

  # seek to the first line, so we can replace then lines correctly
  seek $fh,0,0;
  foreach $line (@content){
    if($line =~ m/#include "(\w*)\//){
      # If a include is local and it should be global, make it global
      if($1 ~~ @includes){
        print "local header $line\n";
        $line =~ s/"/</;
        $line =~ s/"/>/;
        print $fh $line;
        print "converted to global header $line\n";
      }
      else {
          print $fh $line;
      }
    }
    # If a local file is defined global, make it local
    elsif($line =~ /#include <((\w*)\.(hxx|h|hrc|src))>/){
      # check if file exists, then it must be local so replace the <> to ""
      if(-e "$dir/$1" ){
        print "global header $line\n";
        $line =~ s/</"/g;
        $line =~ s/>/"/g;
        print $fh $line;
        print "converted to local header $line\n";
      }
      else {
        print $fh $line;
      }
    }
    else {
      print $fh $line;
    }
  }
  close($fh);
}

# routine that checks the headers of every cxx,hxx,c,h,hrc,src file in a directory
sub check_routine
{
  my ($dir) = @_;
  opendir(my $fh, $dir) or die "Program stopping, couldn't open directory \n";
  while(my $file = readdir($fh)){
    if($file =~ m/\.(cxx|hxx|c|h|hrc|src)$/i ){
      check_headers($dir,"$dir/$file",@subdirs);
    }
  }
  closedir($fh);
}

# Expect ARGV[0] to be a directory, then fetch all subdirectory's and check the header files.
if(-d $ARGV[0]){
  my @directories = io->dir($ARGV[0])->All_Dirs;
  foreach my $dir (@directories){
    print "checking header files in $dir\n";
    check_routine($dir);
  }
}
else{
  print "$ARGV[0] isn't a directory\n";
}
