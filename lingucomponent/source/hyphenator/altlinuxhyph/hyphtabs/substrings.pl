#!/usr/bin/perl
# A utility for finding substring embeddings in patterns

$fn = $ARGV[0];
if (!-e $fn) { $fn = "hyphen.us"; }
open HYPH, $fn;
open OUT, ">$ARGV[1]";

while (<HYPH>)
{
    if (/^\%/) {
    #comment, ignore
    } elsif (/^(.+)\%/) {
        $origpat = $1;
    $pat = $1;
    $pat =~ s/\d//g;
    push @patlist, $pat;
    $pattab{$pat} = $origpat;
    } elsif (/^(.+)$/) {
    $origpat = $1;
    $pat = $1;
    $pat =~ s/\d//g;
    push @patlist, $pat;
    $pattab{$pat} = $origpat;
    }
}

foreach $pat (@patlist) {
    $patsize = length $pat;
    for $i (0..$patsize - 1) {
    for $j (1..$patsize - $i) {
        $subpat = substr ($pat, $i, $j);
#       print "$pattab{$pat} $i $j $subpat $pattab{$subpat}\n";
        if (defined $pattab{$subpat}) {
        print "$pattab{$subpat} is embedded in $pattab{$pat}\n";
        $newpat = substr $pat, 0, $i + $j;
        if (!defined $newpattab{$newpat}) {
            $newpattab{$newpat} =
            substr ($pat, 0, $i).$pattab{$subpat};
            $ss = substr ($pat, 0, $i);
            print "$ss+$pattab{$subpat}\n";
            push @newpatlist, $newpat;
        } else {
            $tmp =  $newpattab{$newpat};
            $newpattab{$newpat} =
            combine ($newpattab{$newpat}, $pattab{$subpat});
            print "$tmp + $pattab{$subpat} -> $newpattab{$newpat}\n";
        }
        }
    }
    }
}

foreach $pat (@newpatlist) {
    print OUT $newpattab{$pat}."\n";
}

#convert 'n1im' to 0n1i0m0 expresed as a list
sub expand {
    my ($pat) = @_;
    my $last = '.';
    my @exp = ();

    foreach $c (split (//, $pat)) {
    if ($last =~ /[\D]/ && $c =~ /[\D]/) {
        push @exp, 0;
    }
    push @exp, $c;
    $last = $c;
    }
    if ($last =~ /[\D]/) {
    push @exp, 0;
    }
    return @exp;
}

# Combine two patterns, i.e. .ad4der + a2d becomes .a2d4der
# The second pattern needs to be a substring of the first (modulo digits)
sub combine {
    my @exp = expand shift;
    my @subexp = expand shift;
    my $pat1, $pat2;
    my $i;

    $pat1 = join ('', map { $_ =~ /\d/ ? () : $_ } @exp);
    $pat2 = join ('', map { $_ =~ /\d/ ? () : $_ } @subexp);

    for $i (0..length ($pat1) - length ($pat2)) {
    if (substr ($pat1, $i, length $pat2) eq $subpat) {
        for ($j = 0; $j < @subexp; $j += 2) {
#       print ("$i $j $subexp[$j] $exp[2 * $i + $j]\n");
        if ($subexp[$j] > $exp[2 * $i + $j]) {
            $exp[2 * $i + $j] = $subexp[$j];
        }
        }
        print ("$pat1 includes $pat2 at pos $i\n");
    }
    }
    return join ('', map { $_ eq '0' ? () : $_ } @exp);
}


