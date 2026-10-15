#!/usr/bin/env perl
# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

use strict;
use warnings;
use utf8;

use Archive::Zip qw( :ERROR_CODES );

# minimalist script to rewrite the timestamps in a zip file to make them deterministic for
# reproducible builds. Other tools also reorder members and also potentially recompress, but we
# control order in our makefiles and want e.g. the mimetype file to be first member and also be
# uncompressed for OpenDocument files, so we just rewrite the timestamps here.

my $zip = Archive::Zip->new();
my $zip_file = shift @ARGV;
my $timestamp = shift @ARGV;

$zip->read($zip_file) == Archive::Zip::AZ_OK or die "Error reading file $zip_file ($!)\n";

# this will only rewrite standard metadata, files with extended attributes aren't covered
foreach my $member ( $zip->members() ) {
    $member->setLastModFileDateTimeFromUnix($timestamp);
}

# overwrite saves to temp file first, then renames the original file and only then renames the temp
# file to the original name, should be atomic enough, esp. given that some recipes create zipfiles
# in multiple steps and are more prone to errors in case the build is killed midway.
$zip->overwrite() == Archive::Zip::AZ_OK or die "Error writing file $zip_file ($!)\n";
chmod 0644, $zip_file;

# vim: set noet sw=4 ts=4:
