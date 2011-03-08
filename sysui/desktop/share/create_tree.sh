umask 022

if [ "${KDEMAINDIR}" ]
then
  echo Copying icons ..
  for i in `cd ${ICON_SOURCE_DIR}; find ${ICON_THEMES:-hicolor/???x??? hicolor/??x?? locolor} -name "*.png"`
  do
    targetdir=${DESTDIR}/${KDEMAINDIR}/share/icons/`dirname $i`
    mkdir -p $targetdir
    destfile=$targetdir/${ICON_PREFIX:-}${ICON_PREFIX:+-}`basename $i`
    cp ${ICON_SOURCE_DIR}/$i $destfile
    chmod 0644 $destfile
  done

  mkdir -p ${DESTDIR}/${KDEMAINDIR}/share/mimelnk/application
  for i in `cat mimelnklist`; do
    cp $i ${DESTDIR}/${KDEMAINDIR}/share/mimelnk/application/${PREFIX}-$i
  done
  chmod 0644 ${DESTDIR}/${KDEMAINDIR}/share/mimelnk/application/*
fi

if [ "${GNOMEDIR}" ]
then
  echo Copying GNOME icons ..
  for i in `cd ${ICON_SOURCE_DIR}/hicolor; find ??x?? ???x??? -name "*.png"`
  do
    targetdir=${DESTDIR}/${GNOMEDIR}/share/icons/gnome/`dirname $i`
    mkdir -p $targetdir
    destfile=$targetdir/${ICON_PREFIX:-}${ICON_PREFIX:+-}`basename $i`
    cp ${ICON_SOURCE_DIR}/hicolor/$i $destfile
    chmod 0644 $destfile
  done

  mkdir -p ${DESTDIR}/${GNOMEDIR}/share/mime-info
  cp openoffice.mime ${DESTDIR}/${GNOMEDIR}/share/mime-info/${PREFIX}.mime
  cp openoffice.keys ${DESTDIR}/${GNOMEDIR}/share/mime-info/${PREFIX}.keys
  chmod 0644 ${DESTDIR}/${GNOMEDIR}/share/mime-info/${PREFIX}.*

  mkdir -p ${DESTDIR}/${GNOMEDIR}/share/application-registry
  cp openoffice.applications ${DESTDIR}/${GNOMEDIR}/share/application-registry/${PREFIX}.applications
  chmod 0644 ${DESTDIR}/${GNOMEDIR}/share/application-registry/${PREFIX}.*

  if [ "${GNOME_MIME_THEME}" ]
  then
    echo "Creating legacy mimetype symlinks for GNOME .."
    # add symlinks so that nautilus can identify the mime-icons
    # not strictly freedesktop-stuff but there is no common naming scheme yet.
    # One proposal is "mime-application:vnd.oasis.opendocument.spreadsheet.png"
    # for e.g. application/vnd.oasis.opendocument.spreadsheet
    link_target_root="${DESTDIR}/${GNOMEDIR}/share/icons/${GNOME_MIME_THEME}"
    if [ ! -d "${link_target_root}" ]
    then
      link_target_root="${DESTDIR}/${GNOMEDIR}/share/icons/gnome"
    fi

    for subdir in `cd ${link_target_root}; ls -d *`
    do
      link_target_dir=""
      link_dir="${DESTDIR}/${GNOMEDIR}/share/icons/${GNOME_MIME_THEME}/$subdir/mimetypes"
      if [ ! -d "${link_dir}" ]
      then
        mkdir -p "${link_dir}"
    link_target_dir="../../../gnome/$subdir/mimetypes/"
      fi
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}drawing.png                     ${link_dir}/gnome-mime-application-vnd.sun.xml.draw.png
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}drawing-template.png            ${link_dir}/gnome-mime-application-vnd.sun.xml.draw.template.png
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}formula.png                     ${link_dir}/gnome-mime-application-vnd.sun.xml.math.png
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}master-document.png             ${link_dir}/gnome-mime-application-vnd.sun.xml.writer.global.png
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}oasis-database.png              ${link_dir}/gnome-mime-application-vnd.sun.xml.base.png
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}oasis-database.png              ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.database.png
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}oasis-drawing.png               ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.graphics.png
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}oasis-drawing-template.png      ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.graphics-template.png
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}oasis-formula.png               ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.formula.png
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}oasis-master-document.png       ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.text-master.png
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}oasis-presentation.png          ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.presentation.png
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}oasis-presentation-template.png ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.presentation-template.png
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}oasis-spreadsheet.png           ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.spreadsheet.png
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}oasis-spreadsheet-template.png  ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.spreadsheet-template.png
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}oasis-text.png                  ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.text.png
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}oasis-text-template.png         ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.text-template.png
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}oasis-web-template.png          ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.text-web.png
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}presentation.png                ${link_dir}/gnome-mime-application-vnd.sun.xml.impress.png
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}presentation-template.png       ${link_dir}/gnome-mime-application-vnd.sun.xml.impress.template.png
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}spreadsheet.png                 ${link_dir}/gnome-mime-application-vnd.sun.xml.calc.png
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}spreadsheet-template.png        ${link_dir}/gnome-mime-application-vnd.sun.xml.calc.template.png
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}text.png                        ${link_dir}/gnome-mime-application-vnd.sun.xml.writer.png
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}text-template.png               ${link_dir}/gnome-mime-application-vnd.sun.xml.writer.template.png
      ln -sf $link_target_dir${ICON_PREFIX:-}${ICON_PREFIX:+-}extension.png                   ${link_dir}/gnome-mime-application-vnd.openofficeorg.extension.png
    done
  fi
fi

mkdir -p ${DESTDIR}/usr/bin

office_prefix=/opt
office_root=${office_prefix}/${PREFIX}

#this symlink is needed to have the API boostrap functions running right
ln -sf ${office_root}/program/soffice ${DESTDIR}/usr/bin/libreoffice

cp printeradmin.sh ${DESTDIR}/usr/bin/${PREFIX}-printeradmin
chmod 0755 ${DESTDIR}/usr/bin/${PREFIX} ${DESTDIR}/usr/bin/${PREFIX}-printeradmin

if test "${PREFIX}" != libreoffice ; then
    # compat symlinks
    mkdir -p ${DESTDIR}${office_prefix}
    ln -sf libreoffice ${DESTDIR}${office_root}
    ln -sf /usr/bin/${PREFIX} ${DESTDIR}/usr/bin/libreoffice
    ln -sf /usr/bin/${PREFIX}-printeradmin ${DESTDIR}/usr/bin/libreoffice-printeradmin
fi

mkdir -p ${DESTDIR}/usr/share/mime/packages
cp openoffice.org.xml ${DESTDIR}/usr/share/mime/packages/libreoffice.xml
chmod 0644 ${DESTDIR}/usr/share/mime/packages/libreoffice.xml

mkdir -p ${DESTDIR}/usr/share/applications
for i in `cat launcherlist`; do
  ln -sf ${office_root}/share/xdg/$i ${DESTDIR}/usr/share/applications/${PREFIX}-$i
done

