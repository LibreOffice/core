#!/usr/bin/perl
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#
use strict;
use warnings;
use File::Find qw(finddepth);
use File::Basename;

# Find dirs in:
# workdir/Dep/CxxObject/
# workdir/Dep/CObject
#
# Concat these files and compare them with the output of
# `git ls-tree HEAD -r --name-only` and report files in the git ls-tree that aren't in the first.

my @files;
my $tmp;
my %data = ();

# define a wanted function
sub wanted {
  return if($_ eq '.' || $_ eq '..' || -d $_);
  $tmp = basename($File::Find::name);
  # remove file extension ( .o )
  $tmp =~ s/\.[^.]*$//;
  $data{$tmp} = $File::Find::name;
}

finddepth(\&wanted, 'workdir/Dep/CxxObject');
finddepth(\&wanted, 'workdir/Dep/CObject');

my @gitfiles = `git ls-tree HEAD -r --name-only`;

# loop over found gitfiles
foreach my $file (@gitfiles){
  if($file =~ /\.[hxx|h|c|cxx]$/){
    $tmp = basename($file);
    $tmp =~ s/\.[^.]*$//;
    chomp($tmp);
    if(!exists($data{$tmp})){
      print $file;
    }
  }
}
