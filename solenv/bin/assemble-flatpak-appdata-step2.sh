#! /bin/bash
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Part of solenv/bin/assemble-flatpak.sh that is shared with a downstream mechanism of building a
# Flatpak from a Fedora libreoffice.spec file.
#
# Arguments:
# $1  pathname, ending in a slash, of the directory containing the source libreoffice-*.appdata.xml
#     files
# $2  pathname, ending in a slash, of the directory containing the target
#     org.libreoffice.LibreOffice.appdata.xml file

set -e

# append the appdata for the different components
for i in "${1?}"libreoffice-*.appdata.xml
do
  sed "1 d; s/<id>libreoffice-/<id>org.libreoffice.LibreOffice./" "$i" \
    >>"${2?}"org.libreoffice.LibreOffice.appdata.xml
done
