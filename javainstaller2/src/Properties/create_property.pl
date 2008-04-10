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
# $RCSfile: create_property.pl,v $
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

# create java installer property files for all languages defined in jlf file

use Cwd;
use File::Copy;

if( $#ARGV < 2 )
  {
    print <<ENDHELP;
USAGE: $0 <separator> <jlf_file_path> <outputpath>
    <separator>: separator, used on the platform (slash or backslash)
    <jlf_file_path>: path, in which the jlf file(s) can be found
    <outputpath>: path, in which the property files will be created
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
my $all_template_files = read_directory($localdir, "properties");
my $all_jlf_files = read_directory($inputpath, "jlf");
my $defaultlanguage = "en-US";
my $missing_jlf_file = "setupfiles.jlf";
my $alllanguages = get_all_languages($all_jlf_files);
my @allnewpropertyfiles = ();

for ( my $i = 0; $i <= $#{$all_template_files}; $i++ )
{
  my $template_file_name = ${$all_template_files}[$i];
  my $complete_template_file_name = $localdir . $separator . $template_file_name;
  my $jlf_file_name = get_jlf_file_name($template_file_name);
  my $complete_jlf_file_name = $inputpath . $separator . $jlf_file_name;
  print "Using template file: $complete_template_file_name\n";
  my $jlf_file = "";
  if ( ! ( $jlf_file_name eq $missing_jlf_file ))
  {
    print "Using translation file: $complete_jlf_file_name\n";
    $jlf_file = read_file($complete_jlf_file_name);
  }

  for ( my $j = 0; $j <= $#{$alllanguages}; $j++ )
  {
    my $language = ${$alllanguages}[$j];
    my $template_file = read_file($complete_template_file_name);
    my $stringhash = create_string_hash($jlf_file, $language);
    create_property_file($template_file, $stringhash);
    my $filename = generate_filename($template_file_name, $language);

    if ( $language eq $defaultlanguage )
    {
      # Creating language indenpendent english file
      make_propertyfile_language_independent($template_file);
      $filename = generate_filename($template_file_name, "");
      save_file($outputpath, $filename, $template_file);
    }
    else
    {
      # Saving the non-english files
      save_file($outputpath, $filename, $template_file);
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

sub main::get_jlf_file_name
{
  my ($tempfilename) = @_;

  my $jlffilename = "";

  if ( $tempfilename =~ /^\s*(\w+)_template/ ) { $tempfilename = $1; }
  $jlffilename = $tempfilename . "\.jlf";

  return $jlffilename;
}

sub main::get_all_languages
{
  my ($alljlffiles) = @_;

  my @languages = ();
  my $record = 0;

  my $first_jlf_file_name = $inputpath . $separator . ${$alljlffiles}[0];
  my $jlffile = read_file($first_jlf_file_name);

  for ( my $i = 0; $i <= $#{$jlffile}; $i++ )
  {
    if (( ${$jlffile}[$i] =~ /^\s*\[.*]\s*$/ ) && ( $record )) { last; }
    if (( ${$jlffile}[$i] =~ /^\s*\[.*]\s*$/ ) && ( $record == 0 )) { $record = 1; }

    if (( $record ) && ( ${$jlffile}[$i] =~ /^\s*(.+?)\s*\=/ ))
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
  my ($jlffile, $language) = @_;

  my %stringhash = ();
  my $key = "";
  my $value_defined = 0;

  for ( my $i = 0; $i <= $#{$jlffile}; $i++ )
  {
    if ( ${$jlffile}[$i] =~ /^\s*\[(.*)\]\s*$/ )
    {
      $key = $1;
      $value_defined = 0;
    }

    if (( ${$jlffile}[$i] =~ /^\s*\Q$defaultlanguage\E\s*=\s*\"(.*)\"\s*$/ ) && ( ! $value_defined ))
    {
      $value = $1;  # defaulting to english
      $stringhash{$key} = $value;
    }

    if (( ${$jlffile}[$i] =~ /^\s*\Q$language\E\s*=\s*\"(.*)\"\s*$/ ) && ( ! $value_defined ))
    {
      $value = $1;
      $stringhash{$key} = $value;
      $value_defined = 1;
    }
  }

  # additional replacement for ${LANGUAGE}, not defined in jlf file
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

sub main::create_property_file
{
  my ($template_file, $stringhash) = @_;

  for ( my $i = 0; $i <= $#{$template_file}; $i++ )
  {
    if ( ${$template_file}[$i] =~ /\$\{(\w+)\}/ )
    {
      my $key = $1;

      if ( exists($stringhash->{$key}) )
      {
        my $value = $stringhash->{$key};
        ${$template_file}[$i] =~ s/\$\{\Q$key\E\}/$value/g;
      }
      else
      {
        print "Error: No value found for key: $key\n";
        exit;
      }
    }
  }
}

sub main::generate_filename
{
  my ($template_file_name, $onelanguage) = @_;

  my $filename = $template_file_name;

  if ( $onelanguage )
  {
    $onelanguage =~ s/-/_/;   # zh-TW -> zh_TW
    $onelanguage = "_" . $onelanguage;
    $filename =~ s/_template\./$onelanguage\./;
  }
  else
  {
    $filename =~ s/_template//;
  }

  return $filename;
}

sub make_propertyfile_language_independent
{
  my ($property_file) = @_;

  for ( my $i = 0; $i <= $#{$property_file}; $i++ )
  {
#    if ( ${$property_file}[$i] =~ /^\s*#/ ) # only comment lines
#    {
      ${$property_file}[$i] =~ s/_\Q$defaultlanguage\E//;
#    }
  }
}

sub main::save_file
{
  my ($outputpath, $filename, $filecontent) = @_;

  $filename = $outputpath . $separator . $filename;

  if ( open( OUT, ">$filename" ) )
  {
    print OUT @{$filecontent};
    close( OUT);
  }

  push(@allnewpropertyfiles, $filename);
  print "Created file: $filename\n";
}
