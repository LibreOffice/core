#!/usr/bin/perl
use strict;

my $left = shift @ARGV;
my $right = shift @ARGV;
unless ($left && $right)
{
  print "\nThis tool compares two git branches left and right from their common anchestor\n";
  print "(the merge-base) to the tip *by the first line of the commit message*\n";
  print "- Since we don't compare by commit hashes, the comparison is fuzzy\n\n";
  print "It prints the log entries that are contained in BOTH, in LEFT branch only and\n";
  print "in RIGHT branch only.\n\n";
  print "Usage: $0 left right\n";
  exit 1;
}

my $mergeBase = `git merge-base $left $right`;
chop $mergeBase;

my %V1;
foreach my $el (`git log --oneline --no-merges $mergeBase..$left`)
{
  my ($hash, $msg) = split " ", $el,2;
  $V1{$msg} = $hash; 
}

foreach my $el (`git log --oneline --no-merges $mergeBase..$right`)
{
  my ($hash, $msg) = split " ", $el,2;
  if(exists $V1{$msg})
  {
    print "IN BOTH: $V1{$msg} $el";
    delete $V1{$msg};
  }
  else
  {
    print "IN RIGHT: $el";
  }
}

foreach my $key (keys %V1)
{
    print "IN LEFT: $V1{$key} $key";
}

