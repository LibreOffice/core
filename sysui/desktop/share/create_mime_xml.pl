#!/usr/bin/perl
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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
    # mimetype and glob
    getMimedata($component);
    print '  <mime-type type="'.$mimetype.'">';
    print (sort({customsort($a) cmp customsort($b)} @{$mimehash{$component}}));
    print '    <glob pattern="'.$glob.'"/>';
    if ( $component =~ /oasis/ ) {
        if ( $component =~ /flat-xml/ ) {
            print '    <sub-class-of type="application/xml"/>';
            print '    <magic'.( $mimetype =~ /-/  ? ' priority="60"' : '').'>';
            print '        <match value="&lt;?xml" type="string" offset="0">';
            print '            <match value="office:document" type="string" offset="4:100">';
            print '                <match value="office:mimetype=&quot;' . $mimetype . '&quot;" type="string" offset="100:4000"/>';
            print '            </match>';
            print '        </match>';
            print '    </magic>';
        } else {
            print '    <magic'.( $mimetype =~ /-/  ? ' priority="60"' : '').'>';
            print '        <match value="PK\003\004" type="string" offset="0">';
            print '            <match value="mimetype" type="string" offset="30">';
            print '                <match value="' . $mimetype . '" type="string" offset="38"/>';
            print '            </match>';
            print '        </match>';
            print '    </magic>';
        }
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
