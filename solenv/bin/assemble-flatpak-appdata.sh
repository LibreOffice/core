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
#     de.allotropia.ZetaOffice.appdata.xml file
# $2  "1" if a <releases> section shall be included in the target
#     de.allotropia.ZetaOffice.appdata.xml file, "0" if not

set -e

## de.allotropia.ZetaOffice.appdata.xml is manually derived from the various
## inst/share/metainfo/zetaoffice-*.appdata.xml (at least recent GNOME Software
## doesn't show more than five screenshots anyway, so restrict to one each from
## the five zetaoffice-*.appdata.xml: Writer, Calc, Impress, Draw, Base):
cat <<\EOF >"${1?}"de.allotropia.ZetaOffice.appdata.xml
<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop">
 <id>de.allotropia.ZetaOffice</id>
 <metadata_license>CC0-1.0</metadata_license>
 <project_license>MPL-2.0</project_license>
 <name>ZetaOffice</name>
 <summary>The ZetaOffice productivity suite</summary>
 <description>
  <p>ZetaOffice is a powerful office suite.  Its clean interface and
  feature-rich tools help you unleash your creativity and enhance your
  productivity.  ZetaOffice includes several applications that make it the most
  powerful Free and Open Source office suite on the market: Writer (word
  processing), Calc (spreadsheets), Impress (presentations), Draw (vector
  graphics and flowcharts), Base (databases), and Math (formula editing).</p>
  <p>ZetaOffice supports opening and saving into a wide variety of formats, so
  you can easily share documents with users of other popular office suites
  without worrying about compatibility.</p>
 </description>
 <launchable type="desktop-id">de.allotropia.ZetaOffice.desktop</launchable>
 <url type="homepage">http://zetaoffice.net</url>
 <screenshots>
  <screenshot type="default">
   <image>https://download.zetaoffice.net/appstream-screenshots/startcenter.png</image>
   <caption>The ZetaOffice productivity suite</caption>
  </screenshot>
  <screenshot>
   <image>https://download.zetaoffice.net/appstream-screenshots/writer.png</image>
   <caption>Sample Writer document</caption>
  </screenshot>
 </screenshots>
 <developer_name>allotropia software GmbH</developer_name>
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
 cat <<EOF >>"${1?}"de.allotropia.ZetaOffice.appdata.xml
 <releases>
  <release
    version="${LIBO_VERSION_MAJOR?}.${LIBO_VERSION_MINOR?}.${LIBO_VERSION_MICRO?}.${LIBO_VERSION_PATCH?}"
    date="$(date +%Y-%m-%d)"/>
 </releases>
EOF
fi

cat <<\EOF >>"${1?}"de.allotropia.ZetaOffice.appdata.xml
</component>
EOF
