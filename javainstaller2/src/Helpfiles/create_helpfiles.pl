: # -*- perl -*-
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: create_helpfiles.pl,v $
#
# $Revision: 1.3 $
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

# create java installer help files in html format for all languages defined in ulf file

use Cwd;
use File::Copy;

if( $#ARGV < 2 )
  {
    print <<ENDHELP;
USAGE: $0 <separator> <ulf_file_path> <outputpath>
    <separator>: separator, used on the platform (slash or backslash)
    <ulf_file_path>: path, in which the ulf file(s) can be found
    <outputpath>: path, in which the help files will be created
ENDHELP
  exit;
  }

$separator = $ARGV[0];
$inputpath = $ARGV[1];
$outputpath = $ARGV[2];

$inputpath =~ s/\Q$separator\E\s*$//;
$outputpath =~ s/\Q$separator\E\s*$//;

if ( ! -d $outputpath ) { mkdir $outputpath; }

print "Separator: $separator \n";
print "Input path: $inputpath \n";
print "Output path: $outputpath \n";

my $localdir = cwd();

my $ulffilename = "setupstrings.ulf";
my $helpfilename = "helpfilenames.txt";
my $defaultlanguage = "en-US";

$ulffilename = $inputpath . $separator . $ulffilename;
my $ulffile = read_file($ulffilename);

my $helpfilenames = read_file($helpfilename);
my $allhelpfilenames = collect_helpfile_names($helpfilenames);

my $alllanguages = get_all_languages($ulffile);
my @allnewpropertyfiles = ();

for ( my $i = 0; $i <= $#{$allhelpfilenames}; $i++ )
{
  my $helpfilename = ${$allhelpfilenames}[$i];

  for ( my $j = 0; $j <= $#{$alllanguages}; $j++ )
  {
    my $language = ${$alllanguages}[$j];

    # Creating content of help file
    my $helpfilecontent = collect_helpfile_content($helpfilename, $ulffile, $language);

    # Saving helpfile
    my $savefilename = $helpfilename . "_" . $language . ".html";
    $savefilename = $outputpath . $separator . $savefilename;
    save_file($savefilename, $helpfilecontent);

    if ( $language eq $defaultlanguage )
    {
      $savefilename = $helpfilename . ".html";
      $savefilename = $outputpath . $separator . $savefilename;
      save_file($savefilename, $helpfilecontent);
    }
  }
}

exit;

sub main::read_directory
{
  my ($dir, $ext) = @_;

  my @content = ();
  my $direntry;
  opendir(DIR, $dir);

  foreach $direntry (readdir (DIR))
  {
    next if $direntry eq ".";
    next if $direntry eq "..";
    next if ( ! ( $direntry =~ /\.\Q$ext\E\s*$/ ));

    # my $completeentry = $dir . $separator . $direntry;
    # push(@content, $completeentry);
    push(@content, $direntry);
  }

  closedir(DIR);
  return \@content;
}

sub main::read_file
{
  my ($filename) = @_;

  open( IN, "<$filename" ) || die "cannot open $filename";
  my @content = <IN>;
  close( IN );

  return \@content;
}

sub main::collect_helpfile_content
{
  my ($helpfilename, $ulffile, $language) = @_;

  my @helpfilecontent = ();
  my $stringhash = create_string_hash($ulffile, $language);

  # Collecting all strings for one html file.
  # For "Prologue_de.html" all files need to begin with "STRING_PROLOGUE_X"
  # The "X" is the ordering number.

  my $basestring = "STRING_" . uc($helpfilename) . "_";

  for ( my $i = 0; $i <= 10; $i++ )  # 10 strings possible for each html file
  {
    my $key = $basestring . $i;
    if ( exists $stringhash->{$key} )
    {
      my $content = $stringhash->{$key} . "\n<p>\n";
      push(@helpfilecontent, $content);
    }
  }

  return \@helpfilecontent;
}

sub main::collect_helpfile_names
{
    my ($helpfilecontent) = @_;

    my @allhelpfiles = ();

    for ( my $i = 0; $i <= $#{$helpfilecontent}; $i++ )
    {
        if ( ${$helpfilecontent}[$i] =~ /^\s*#/ ) { next; }  # comment line
        if ( ${$helpfilecontent}[$i] =~ /^\s*$/ ) { next; }  # empty line
        my $filename = ${$helpfilecontent}[$i];
        $filename =~ s/\s//g;
        push(@allhelpfiles, $filename);
    }

    return \@allhelpfiles;
}

sub main::get_all_languages
{
  my ($ulffile) = @_;

  my @languages = ();
  my $record = 0;

  for ( my $i = 0; $i <= $#{$ulffile}; $i++ )
  {
    if (( ${$ulffile}[$i] =~ /^\s*\[.*]\s*$/ ) && ( $record )) { last; }
    if (( ${$ulffile}[$i] =~ /^\s*\[.*]\s*$/ ) && ( $record == 0 )) { $record = 1; }

    if (( $record ) && ( ${$ulffile}[$i] =~ /^\s*(.+?)\s*\=/ ))
    {
      $language = $1;
      push(@languages, $language);
    }
  }

  my $languagestring = "";
  for ( my $i = 0; $i <= $#languages; $i++ ) { $languagestring = $languagestring . $languages[$i] . ","; }
  $languagestring =~ s/,\s*$//;
  print "Languages: $languagestring\n";

  return \@languages;
}

sub main::create_string_hash
{
  my ($ulffile, $language) = @_;

  my %stringhash = ();
  my $key = "";
  my $value_defined = 0;

  for ( my $i = 0; $i <= $#{$ulffile}; $i++ )
  {
    if ( ${$ulffile}[$i] =~ /^\s*\[(.*)\]\s*$/ )
    {
      $key = $1;
      $value_defined = 0;
    }

    if (( ${$ulffile}[$i] =~ /^\s*\Q$defaultlanguage\E\s*=\s*\"(.*)\"\s*$/ ) && ( ! $value_defined ))
    {
      $value = $1;  # defaulting to english
      $stringhash{$key} = $value;
    }

    if (( ${$ulffile}[$i] =~ /^\s*\Q$language\E\s*=\s*\"(.*)\"\s*$/ ) && ( ! $value_defined ))
    {
      $value = $1;
      $stringhash{$key} = $value;
      $value_defined = 1;
    }
  }

  # additional replacement for ${LANGUAGE}, not defined in ulf file
  my $languagekey = "LANGUAGE";
  $stringhash{$languagekey} = $language;

  # print_hash(\%stringhash);

  return \%stringhash;
}

sub main::print_hash
{
  my ( $hashref ) = @_;

  print "Hash contains:\n";

  my $key;
  foreach $key (keys %{$hashref} ) { print "Key: $key, Value: $hashref->{$key}\n"; }
}

sub main::save_file
{
  my ($filename, $filecontent) = @_;

  if ( open( OUT, ">$filename" ) )
  {
    print OUT @{$filecontent};
    close( OUT);
  }

  push(@allnewpropertyfiles, $filename);
  print "Created file: $filename\n";
}
