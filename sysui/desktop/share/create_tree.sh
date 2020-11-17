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

umask 022

if [ "${KDEMAINDIR}" ]; then
  for i in `cd "${ICON_SOURCE_DIR}"; find ${ICON_THEMES:-hicolor/???x??? hicolor/??x??} -name "*.png"; find hicolor/scalable/apps -name "*.svg"`
  do
    targetdir=${DESTDIR}/${KDEMAINDIR}/share/icons/`dirname ${i}`
    mkdir -p "${targetdir}"
    destfile=$targetdir/${ICON_PREFIX:-}${ICON_PREFIX:+-}`basename ${i}`
    cp "${ICON_SOURCE_DIR}/${i}" "${destfile}"
    chmod 0644 "${destfile}"
  done
  unset targetdir destfile

fi

mkdir -p "${DESTDIR}/${BINDIR}"

test -n "${OFFICE_PREFIX}" && office_prefix="${OFFICE_PREFIX}" || office_prefix=/opt
office_root=${office_prefix}/${PREFIX}

#this symlink is needed to have the API bootstrap functions running right
ln -sf "${office_root}/program/soffice" "${DESTDIR}/${BINDIR}/${PREFIX}"

if test "${PREFIX}" != libreoffice${PRODUCTVERSION} -a "${PREFIX}" != libreofficedev${PRODUCTVERSION}  ; then
    # compat symlinks
    mkdir -p "${DESTDIR}${office_prefix}"
    ln -sf libreoffice${PRODUCTVERSION} "${DESTDIR}${office_root}"
    ln -sf /${BINDIR}/${PREFIX} "${DESTDIR}/${BINDIR}/libreoffice${PRODUCTVERSION}"
fi

test "${PREFIX}" = libreofficedev${PRODUCTVERSION} && mime_def_file="libreofficedev${PRODUCTVERSION}.xml" || mime_def_file="libreoffice${PRODUCTVERSION}.xml"
mkdir -p "${DESTDIR}/${PREFIXDIR}/share/mime/packages"
cp openoffice.org.xml "${DESTDIR}/${PREFIXDIR}/share/mime/packages/$mime_def_file"
chmod 0644 "${DESTDIR}/${PREFIXDIR}/share/mime/packages/$mime_def_file"

mkdir -p "${DESTDIR}/${PREFIXDIR}/share/applications"
for i in `cat launcherlist`; do
  ln -sf "${office_root}/share/xdg/${i}" "${DESTDIR}/${PREFIXDIR}/share/applications/${PREFIX}-${i}"
done

mkdir -p "${DESTDIR}/${PREFIXDIR}/share/metainfo"
for i in base calc draw impress writer; do
    cp "${APPDATA_SOURCE_DIR}/libreoffice-${i}.appdata.xml" "${DESTDIR}/${PREFIXDIR}/share/metainfo/${PREFIX}-${i}.appdata.xml"
done
cp "${APPDATA_SOURCE_DIR}/org.libreoffice.kde.metainfo.xml" "${DESTDIR}/${PREFIXDIR}/share/metainfo/org.${PREFIX}.kde.metainfo.xml"

