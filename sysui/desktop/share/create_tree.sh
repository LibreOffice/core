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

mkdir -p "${DESTDIR}/${PREFIXDIR}/bin"

test -n "${OFFICE_PREFIX}" && office_prefix="${OFFICE_PREFIX}" || office_prefix=/opt
office_root=${office_prefix}/${PREFIX}

#this symlink is needed to have the API boostrap functions running right
ln -sf "${office_root}/program/soffice" "${DESTDIR}/${PREFIXDIR}/bin/${PREFIX}"

if test "${PREFIX}" != libreoffice${PRODUCTVERSION} -a "${PREFIX}" != libreofficedev${PRODUCTVERSION}  ; then
    # compat symlinks
    mkdir -p "${DESTDIR}${office_prefix}"
    ln -sf libreoffice${PRODUCTVERSION} "${DESTDIR}${office_root}"
    ln -sf /${PREFIXDIR}/bin/${PREFIX} "${DESTDIR}/${PREFIXDIR}/bin/libreoffice${PRODUCTVERSION}"
fi

test "${PREFIX}" = libreofficedev${PRODUCTVERSION} && mime_def_file="libreofficedev${PRODUCTVERSION}.xml" || mime_def_file="libreoffice${PRODUCTVERSION}.xml"
mkdir -p "${DESTDIR}/${PREFIXDIR}/share/mime/packages"
cp openoffice.org.xml "${DESTDIR}/${PREFIXDIR}/share/mime/packages/$mime_def_file"
chmod 0644 "${DESTDIR}/${PREFIXDIR}/share/mime/packages/$mime_def_file"

mkdir -p "${DESTDIR}/${PREFIXDIR}/share/applications"
for i in `cat launcherlist`; do
  ln -sf "${office_root}/share/xdg/${i}" "${DESTDIR}/${PREFIXDIR}/share/applications/${PREFIX}-${i}"
done

mkdir -p "${DESTDIR}/${PREFIXDIR}/share/appdata"
for i in base calc draw impress writer; do
    cp "${APPDATA_SOURCE_DIR}/libreoffice-${i}.appdata.xml" "${DESTDIR}/${PREFIXDIR}/share/appdata/${PREFIX}-${i}.appdata.xml"
done

# Generate gobject-introspection files
mkdir -p "${DESTDIR}/${PREFIXDIR}/share/gir-1.0"
g-ir-scanner "${SRCDIR}/include/LibreOfficeKit/LibreOfficeKitGtk.h" "${SRCDIR}/libreofficekit/source/gtk/lokdocview.cxx" \
             `${PKG_CONFIG} --cflags gobject-introspection-1.0 gtk+-3.0` -I"${SRCDIR}/include/" \
             --include=GLib-2.0 --include=GObject-2.0 --include=Gio-2.0 \
             --library=libreofficekitgtk --library-path="${DESTDIR}/${INSTALLDIR}/program" \
             --include=Gdk-3.0 --include=GdkPixbuf-2.0 --include=Gtk-3.0 \
             --namespace=LOKDocView --nsversion=0.1 --identifier-prefix=LOKDoc --symbol-prefix=lok_doc \
             --output="${DESTDIR}/${PREFIXDIR}/share/gir-1.0/LOKDocView-0.1.gir" --warn-all --no-libtool

mkdir -p "${DESTDIR}/${LIBDIR}/girepository-1.0"
g-ir-compiler "${DESTDIR}/${PREFIXDIR}/share/gir-1.0/LOKDocView-0.1.gir" \
              --output="${DESTDIR}/${LIBDIR}/girepository-1.0/LOKDocView-0.1.typelib"
