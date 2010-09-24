#!/usr/bin/perl -w

my @output_buffer = ();
my $fname;
my $detectedSomeGuff = 0;
sub pure_guff($)
{
  my $array = shift;
  my @lines = @{$array};
  my $contains_sense = '';
  my $contains_guff = '';
  while (scalar @lines)
  {
    my $line = pop @lines;
    if ($line =~ m/Test run started :/ ||
    $line =~ m/ITEM Assertion OK/ ||
    $line =~ m/Test run finished :/) {
      $contains_guff = '1';
    } elsif ($line =~ m/^[\+\-][^\-\+]/) {
      $contains_sense = '1';
    }
  }
  if ($contains_guff && $contains_sense) {
    print STDERR "Patch fragment with mixed good/bad changes in '$ARGV' near $line_index\n";
    $contains_guff = '';
  }
  elsif ( $contains_guff ) {
    $detectedSomeGuff++;
  }
#  print "contains guff: $contains_guff\n";
  return $contains_guff;
}

sub output_lines($)
{
  my $array = shift;
  my @lines = @{$array};

  if (pure_guff (\@lines)) {
    return;
  }

  while (scalar @lines)
  {
    my $line = pop @lines;
    push @output_buffer, $line;
  }
}

my $header;
my @lines;
my $frag_count = 0;
$line_index = 0;

while (<>) {
  if (/^\@\@/ || /^[^ \-\+]/) {
    output_lines (\@lines);
    @lines = ();
    $frag_count++;
  }
  unshift @lines, $_;
  $line_index++;
  close ARGV if eof;
}
output_lines(\@lines);

# $detectedSomeGuff contains the skipped hunks that contain acceptable diff
# e.g. a timestamp or an OK assertion that contains different content
# like perhaps a path
#print "frag_count = $frag_count fragstocount = $fragstocount detectedSomeGuff = $detectedSomeGuff \n";
if ($frag_count > $detectedSomeGuff) {
  print @output_buffer;
}
