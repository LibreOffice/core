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
# $1  pathname, ending in a slash, of the directory containing the source zetaoffice-*.desktop
#     files
# $2  pathname, ending in a slash, of the directory into which to put the target
#     de.allotropia.ZetaOffice.*.desktop files

set -e

## zetaoffice-*.desktop -> de.allotropia.ZetaOffice.*.desktop:
for i in "${1?}"zetaoffice-*.desktop
do
 sed -e 's/^Icon=zetaoffice-/Icon=de.allotropia.ZetaOffice./' "$i" \
  >"${2?}"de.allotropia.ZetaOffice."${i#"${1?}"zetaoffice-}"
done
mv "${2?}"de.allotropia.ZetaOffice.startcenter.desktop "${2?}"de.allotropia.ZetaOffice.desktop

# Flatpak .desktop exports take precedence over system ones due to
# the order of XDG_DATA_DIRS - re-associating text/plain seems a bit much
sed -i "s/text\/plain;//" "${2?}"de.allotropia.ZetaOffice.writer.desktop

desktop-file-edit --set-key=X-Endless-Alias --set-value=zetaoffice-startcenter \
 --set-key=X-Flatpak-RenamedFrom --set-value='zetaoffice-startcenter.desktop;' \
 "${2?}"de.allotropia.ZetaOffice.desktop
for i in base calc draw impress math writer xsltfilter
do
 desktop-file-edit --set-key=X-Endless-Alias --set-value=zetaoffice-"$i" \
  --set-key=X-Flatpak-RenamedFrom \
  --set-value="zetaoffice-$i.desktop;de.allotropia.ZetaOffice-$i.desktop;" \
  "${2?}"de.allotropia.ZetaOffice."$i".desktop
done
