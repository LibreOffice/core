#!/bin/sh
umask 022

if [ "${KDEMAINDIR}" ]; then
  echo "Copying icons..."
  for i in `cd "${ICON_SOURCE_DIR}"; find ${ICON_THEMES:-hicolor/???x??? hicolor/??x?? locolor} -name "*.png"`
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
    cp "${i}" "${DESTDIR}/${KDEMAINDIR}/share/mimelnk/application/${PREFIX}-${i}"
  done
  chmod 0644 "${DESTDIR}/${KDEMAINDIR}/share/mimelnk/application/"*
fi

if [ "${GNOMEDIR}" ]; then
  echo "Copying GNOME icons..."
  for i in `cd "${ICON_SOURCE_DIR}/hicolor"; find ??x?? ???x??? -name "*.png"`
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

mkdir -p "${DESTDIR}$PREFIXDIR/bin"

test -n "${OFFICE_PREFIX}" && office_prefix="${OFFICE_PREFIX}" || office_prefix=/opt
office_root=${office_prefix}/${PREFIX}

#this symlink is needed to have the API boostrap functions running right
ln -sf "${office_root}/program/soffice" "${DESTDIR}$PREFIXDIR/bin/${PREFIX}"

cp printeradmin.sh "${DESTDIR}$PREFIXDIR/bin/${PREFIX}-printeradmin"
chmod 0755 "${DESTDIR}$PREFIXDIR/bin/${PREFIX}-printeradmin"

if test "${PREFIX}" != libreoffice${PRODUCTVERSION} ; then
    # compat symlinks
    mkdir -p "${DESTDIR}${office_prefix}"
    ln -sf libreoffice${PRODUCTVERSION} "${DESTDIR}${office_root}"
    ln -sf $PREFIXDIR/bin/${PREFIX} "${DESTDIR}$PREFIXDIR/bin/libreoffice${PRODUCTVERSION}"
    ln -sf $PREFIXDIR/bin/${PREFIX}-printeradmin "${DESTDIR}$PREFIXDIR/bin/libreoffice${PRODUCTVERSION}-printeradmin"
fi

mkdir -p "${DESTDIR}$PREFIXDIR/share/mime/packages"
cp openoffice.org.xml "${DESTDIR}$PREFIXDIR/share/mime/packages/libreoffice${PRODUCTVERSION}.xml"
chmod 0644 "${DESTDIR}$PREFIXDIR/share/mime/packages/libreoffice${PRODUCTVERSION}.xml"

mkdir -p "${DESTDIR}$PREFIXDIR/share/applications"
for i in `cat launcherlist`; do
  ln -sf "${office_root}/share/xdg/${i}" "${DESTDIR}$PREFIXDIR/share/applications/${PREFIX}-${i}"
done

