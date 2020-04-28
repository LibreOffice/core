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
# $1  pathname, ending in a slash, of the directory containing the source libreoffice-*.desktop
#     files
# $2  pathname, ending in a slash, of the directory into which to put the target
#     org.libreoffice.LibreOffice.*.desktop files

set -e

## libreoffice-*.desktop -> org.libreoffice.LibreOffice.*.desktop:
for i in "${1?}"libreoffice-*.desktop
do
 sed -e 's/^Icon=libreoffice-/Icon=org.libreoffice.LibreOffice./' "$i" \
  >"${2?}"org.libreoffice.LibreOffice."${i#"${1?}"libreoffice-}"
done
mv "${2?}"org.libreoffice.LibreOffice.startcenter.desktop "${2?}"org.libreoffice.LibreOffice.desktop

# Flatpak .desktop exports take precedence over system ones due to
# the order of XDG_DATA_DIRS - re-associating text/plain seems a bit much
sed -i "s/text\/plain;//" "${2?}"org.libreoffice.LibreOffice.writer.desktop

desktop-file-edit --set-key=X-Endless-Alias --set-value=libreoffice-startcenter \
 --set-key=X-Flatpak-RenamedFrom --set-value='libreoffice-startcenter.desktop;' \
 "${2?}"org.libreoffice.LibreOffice.desktop
for i in base calc draw impress math writer xsltfilter
do
 desktop-file-edit --set-key=X-Endless-Alias --set-value=libreoffice-"$i" \
  --set-key=X-Flatpak-RenamedFrom \
  --set-value="libreoffice-$i.desktop;org.libreoffice.LibreOffice-$i.desktop;" \
  "${2?}"org.libreoffice.LibreOffice."$i".desktop
done
