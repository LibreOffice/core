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
# $1  pathname, ending in a slash, of the directory into which to put the target
#     $3.appdata.xml file
# $2  "1" if a <releases> section shall be included in the target
#     $3.appdata.xml file, "0" if not
# $3  the flatpak ID

set -e

## $3.appdata.xml is manually derived from the various
## inst/share/metainfo/collaboraoffice-*.appdata.xml (at least recent GNOME Software
## doesn't show more than five screenshots anyway, so restrict to one each from
## the five collaboraoffice-*.appdata.xml: Writer, Calc, Impress, Draw, Base):
cat <<EOF >"${1?}${3?}".appdata.xml
<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop">
 <id>${3?}</id>
 <metadata_license>CC0-1.0</metadata_license>
 <project_license>MPL-2.0</project_license>
 <name>Collabora Office Desktop</name>
 <summary>The Collabora Office productivity suite</summary>
 <description>
  <p>Collabora Office Desktop is a powerful office suite.</p>
 </description>
 <launchable type="desktop-id">${3?}.desktop</launchable>
 <url type="homepage">https://collaboraoffice.com</url>
 <screenshots>
  <screenshot type="default">
   <image>writer.png</image>
   <caption>Sample Writer document</caption>
  </screenshot>
  <screenshot>
   <image>calc.png</image>
   <caption>Sample Calc document</caption>
  </screenshot>
  <screenshot>
   <image>impress.png</image>
   <caption>Sample Impress document</caption>
  </screenshot>
  <screenshot>
   <image>draw.png</image>
   <caption>Sample Draw document</caption>
  </screenshot>
 </screenshots>
 <developer_name>Collabora Productivity Limited</developer_name>
 <kudos>
  <kudo>HiDpiIcon</kudo>
  <kudo>HighContrast</kudo>
  <kudo>ModernToolkit</kudo>
  <kudo>UserDocs</kudo>
 </kudos>
 <content_rating type="oars-1.0"/>
EOF

if [ "${2?}" = 1 ]
then
 cat <<EOF >>"${1?}${3?}".appdata.xml
 <releases>
  <release
    version="${LIBO_VERSION_MAJOR?}.${LIBO_VERSION_MINOR?}.${LIBO_VERSION_MICRO?}.${LIBO_VERSION_PATCH?}"
    date="$(date +%Y-%m-%d)"/>
 </releases>
EOF
fi

cat <<\EOF >>"${1?}${3?}".appdata.xml
</component>
EOF
