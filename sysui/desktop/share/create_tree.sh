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
  for i in `cd "${ICON_SOURCE_DIR}"; find ${ICON_THEMES:-hicolor/???x??? hicolor/??x?? hicolor/scalable locolor} -name "*.png" -o -name "*.svg"`
  do
    targetdir=${DESTDIR}/${KDEMAINDIR}/share/icons/`dirname ${i}`
    mkdir -p "${targetdir}"
    destfile=$targetdir/${ICON_PREFIX:-}${ICON_PREFIX:+-}`basename ${i}`
    cp "${ICON_SOURCE_DIR}/${i}" "${destfile}"
    chmod 0644 "${destfile}"
  done
  unset targetdir destfile

  mkdir -p "${DESTDIR}/${KDEMAINDIR}/share/mimelnk/application"
  for i in `cat mimelnklist`; do
    cp "${i}" "${DESTDIR}/${KDEMAINDIR}/share/mimelnk/application/${PREFIX}-`basename ${i}`"
  done
  chmod 0644 "${DESTDIR}/${KDEMAINDIR}/share/mimelnk/application/"*
fi

if [ "${GNOMEDIR}" ]; then
  for i in `cd "${ICON_SOURCE_DIR}/hicolor"; find ??x?? ???x??? scalable -name "*.png" -o -name "*.svg"`
  do
    targetdir=${DESTDIR}/${GNOMEDIR}/share/icons/gnome/`dirname ${i}`
    mkdir -p "${targetdir}"
    destfile=$targetdir/${ICON_PREFIX:-}${ICON_PREFIX:+-}`basename ${i}`
    cp "${ICON_SOURCE_DIR}/hicolor/${i}" "${destfile}"
    chmod 0644 "${destfile}"
  done
  unset targetdir destfile

  mkdir -p "${DESTDIR}/${GNOMEDIR}/share/mime-info"
  cp openoffice.mime "${DESTDIR}/${GNOMEDIR}/share/mime-info/${PREFIX}.mime"
  cp openoffice.keys "${DESTDIR}/${GNOMEDIR}/share/mime-info/${PREFIX}.keys"
  chmod 0644 "${DESTDIR}/${GNOMEDIR}/share/mime-info/${PREFIX}".*

  mkdir -p "${DESTDIR}/${GNOMEDIR}/share/application-registry"
  cp openoffice.applications "${DESTDIR}/${GNOMEDIR}/share/application-registry/${PREFIX}.applications"
  chmod 0644 "${DESTDIR}/${GNOMEDIR}/share/application-registry/${PREFIX}".*
fi

mkdir -p "${DESTDIR}/usr/bin"

test -n "${OFFICE_PREFIX}" && office_prefix="${OFFICE_PREFIX}" || office_prefix=/opt
office_root=${office_prefix}/${PREFIX}

#this symlink is needed to have the API boostrap functions running right
ln -sf "${office_root}/program/soffice" "${DESTDIR}/usr/bin/${PREFIX}"

if test "${PREFIX}" != libreoffice${PRODUCTVERSION} -a "${PREFIX}" != libreofficedev${PRODUCTVERSION}  ; then
    # compat symlinks
    mkdir -p "${DESTDIR}${office_prefix}"
    ln -sf libreoffice${PRODUCTVERSION} "${DESTDIR}${office_root}"
    ln -sf /usr/bin/${PREFIX} "${DESTDIR}/usr/bin/libreoffice${PRODUCTVERSION}"
fi

test "${PREFIX}" = libreofficedev${PRODUCTVERSION} && mime_def_file="libreofficedev${PRODUCTVERSION}.xml" || mime_def_file="libreoffice${PRODUCTVERSION}.xml"
mkdir -p "${DESTDIR}/usr/share/mime/packages"
cp openoffice.org.xml "${DESTDIR}/usr/share/mime/packages/$mime_def_file"
chmod 0644 "${DESTDIR}/usr/share/mime/packages/$mime_def_file"

mkdir -p "${DESTDIR}/usr/share/applications"
for i in `cat launcherlist`; do
  ln -sf "${office_root}/share/xdg/${i}" "${DESTDIR}/usr/share/applications/${PREFIX}-${i}"
done

mkdir -p "${DESTDIR}/usr/share/appdata"
for i in base calc draw impress writer; do
    cp "${APPDATA_SOURCE_DIR}/libreoffice-${i}.appdata.xml" "${DESTDIR}/usr/share/appdata/${PREFIX}-${i}.appdata.xml"
done

