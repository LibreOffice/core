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

use Archive::Zip qw(:ERROR_CODES);
use File::Basename;

my $zipName = shift || die 'must provide a ext name';
my $rdbName = shift || die 'must provide a types library';
my $libName = shift || die 'must provide a component library';

die "can't access type library $rdbName" unless -f $rdbName;
die "can't access component library $libName" unless -f $libName;

# Read the zip
my $zip = Archive::Zip->new();

if ( -f $zipName )
{
    # be stupid and recreate zip every time
    # in another iteration lets try to overwrite it instead
    my $result = 0;
    $result = unlink($zipName);
    if ( result != 0 )
    {
        die 'can not delete old extension';
    }
}
my $rdb = basename( $rdbName );
my $lib = basename( $libName );

my $content2 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
$content2 .= "<!DOCTYPE manifest:manifest PUBLIC \"-//OpenOffice.org//DTD Manifest 1.0//EN\" \"Manifest.dtd\">\n";
$content2 .= "<manifest:manifest xmlns:manifest=\"http://openoffice.org/2001/manifest\">\n";
$content2 .= "  <manifest:file-entry manifest:media-type=\"application/vnd.sun.star.uno-typelibrary;type=RDB\" manifest:full-path=\"$rdb\"/>\n";
$content2 .= "  <manifest:file-entry manifest:media-type=\"application/vnd.sun.star.uno-component;type=native\"  manifest:full-path=\"$lib\"/>\n";
$content2 .= "</manifest:manifest>\n";

$zip->addFile( $rdbName, $rdb );
$zip->addFile( $libName, $lib );
$zip->addDirectory( "META-INF" );
$zip->addFile( "META-INF/manifest.xml", "manifest.xml" );
$zip->addString( $content2, "META-INF/manifest.xml" );
exit( $zip->writeToFileNamed($zipName) );
