#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: createExtPackage.pl,v $
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
