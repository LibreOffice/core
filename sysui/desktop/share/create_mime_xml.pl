#!/usr/bin/perl
use File::Basename;
$basedir = dirname($0);
$productname = 'LibreOffice';
$mimedir = $basedir.'/../mimetypes';

$FS= ' ';       # field seperator (space) - for documents.ulf
$, = "\n";      # set output field separator (newline)
$\ = "\n";      # set output record separator (newline)

## get list of components and corresponding translations from documents.ulf
open(DOCUMENTS_ULF, $ARGV[0]) || die 'Cannot open "documents.ulf".';
while (<DOCUMENTS_ULF>) {
    if (/^\[/) {
        # section starts
        s/^\[(.*)]/$1/;
        chomp;
        $module = $_;
    } else {
        # translated strings
        ($lang,$junk,$comment) = split($FS, $_, 3);
        $comment =~ s/^"(.*)"$/$1/;
        $comment =~ s/%PRODUCTNAME/$productname/;
        chomp $lang;
        chomp $comment;
        if ($lang eq "en-US") {
            $value = "    <comment>$comment</comment>";
        } else {
            $value = '    <comment xml:lang="'.$lang.'">'.$comment.'</comment>';
        }
        push(@{$mimehash{$module}}, $value) unless $lang eq "";
    }
}
close DOCUMENTS_ULF;

## creating the xml on stdout
print '<?xml version="1.0" encoding="UTF-8"?>';
print '<mime-info xmlns="http://www.freedesktop.org/standards/shared-mime-info">';

foreach $component (sort(keys %mimehash)) {
    print STDERR "Processing entries for $component";
    # mimetype and glob
    getMimedata($component);
    print '  <mime-type type="'.$mimetype.'">';
    print (sort({customsort($a) cmp customsort($b)} @{$mimehash{$component}}));
    print '    <glob pattern="'.$glob.'"/>';
    if ( $component =~ /oasis/ ) {
        print '    <magic'.( $mimetype =~ /-/  ? ' priority="60"' : '').'>';
        print '        <match type="string" offset="38" value="'.$mimetype.'"/>';
        print '    </magic>';
    }
    print '  </mime-type>';
}
print '</mime-info>';

sub customsort {
        # sort <comment> before <comment xml:lang...>
        $in = shift;
        $in =~ tr/>/A/;
        $in =~ tr/ /Z/;
        return $in;
}

## get mime-type and glob from ../mimetype/*.desktop
sub getMimedata {
    $desktop_name = shift;
    $desktop_file = $mimedir.'/'.$desktop_name.'.desktop';
    $mimetype = "";
    $glob = "";
    open(DESKTOP_FILE, $desktop_file) || die 'Cannot open "desktop"-file.'.$!;
    while (<DESKTOP_FILE>) {
        ## get mimetype
        if (/^MimeType=/) {
            s/^MimeType=(.*)\n$/$1/;
            $mimetype = "$_";
        }
        ## get glob
        if (/^Patterns=/) {
            s/^Patterns=(.*)\n$/\1/;
            $glob = "$_";
        }
    }
    close DESKTOP_FILE;
    # die if we cannot determine the glob-pattern or mimtetype
    die "Could not get mime-type fom $desktop_file" if ($mimetype eq "");
    die "Could not get glob-pattern fom $desktop_file" if ($glob eq "");
}

## END vim: set ts=4:
