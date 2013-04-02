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
# version and release passed by command-line
Version: %version
Release: %release
Summary: %productname desktop integration
Name: %pkgprefix-suse-menus
Group: Office
License: LGPLv3 with MPLv2, ALv2 and others
Vendor: The Document Foundation
AutoReqProv: no
BuildArch: noarch
# /etc/SuSE-release for SuSE, SLES and Novell Linux Desktop ..
Requires: /etc/SuSE-release
# .. but not for Sun JDS
Conflicts: SunDesktopVersion
Provides: libreoffice-desktop-integration
%define _unpackaged_files_terminate_build 0
%define _binary_filedigest_algorithm 1
%define _binary_payload w9.gzdio

%define gnome_dir /opt/gnome
%define gnome_mime_theme hicolor

%description
%productname desktop integration

%install
rm -rf $RPM_BUILD_ROOT/*

# hack/workaround to make SuSE's brp-symlink-script happy. It wants the targets of all links
# to be present on the build-system/the buildroot. But the point is that we generate stale
# links intentionally (until we find a better solution) #46226
export NO_BRP_STALE_LINK_ERROR=yes

mkdir -p $RPM_BUILD_ROOT

# set parameters for the create_tree script
export DESTDIR=$RPM_BUILD_ROOT
export KDEMAINDIR=/opt/kde3
export GNOMEDIR=%{gnome_dir}

./create_tree.sh

%clean
rm -rf $RPM_BUILD_ROOT

%triggerin -- %pkgprefix, %pkgprefix-writer, %pkgprefix-calc, %pkgprefix-draw, %pkgprefix-impress, %pkgprefix-base, %pkgprefix-math
if [ -x /opt/gnome/bin/update-desktop-database ]; then
  echo Update Desktop Database
  /opt/gnome/bin/update-desktop-database -q /usr/share/applications
  /opt/gnome/bin/update-desktop-database
fi

%triggerun -- %pkgprefix, %pkgprefix-writer, %pkgprefix-calc, %pkgprefix-draw, %pkgprefix-impress, %pkgprefix-base, %pkgprefix-math
if [ "$1" = "0" ] ; then
  # the menu-package gets uninstalled/updated - postun will run the command
  exit 0
fi
if [ "$2" = "0" ] ; then
  # the triggering package gets removed
  if [ -x /opt/gnome/bin/update-desktop-database ]; then
    echo Update Desktop Database
    /opt/gnome/bin/update-desktop-database -q /usr/share/applications
    /opt/gnome/bin/update-desktop-database
fi
fi

%post
# run always, since there are versions of this package that did not include
# a shared-mime-info xml file
if [ -x /usr/bin/update-mime-database ]; then
  update-mime-database /usr/share/mime
fi

if [ -x /opt/gnome/bin/update-desktop-database ]; then
  echo Update Desktop Database
  /opt/gnome/bin/update-desktop-database -q /usr/share/applications
  /opt/gnome/bin/update-desktop-database
fi

# add symlinks so that nautilus can identify the mime-icons
# not strictly freedesktop-stuff but there is no common naming scheme yet.
# One proposal is "mime-application:vnd.oasis.opendocument.spreadsheet.png"
# for e.g. application/vnd.oasis.opendocument.spreadsheet
link_target_root="%{gnome_dir}/share/icons/%{gnome_mime_theme}"

if [ ! -d "${link_target_root}" ]
then
  link_target_root="%{gnome_dir}/share/icons/gnome"
fi

for subdir in `cd ${link_target_root}; ls -d *`
do
  link_dir="%{gnome_dir}/share/icons/%{gnome_mime_theme}/$subdir/mimetypes"
  link_target_dir="../../../gnome/$subdir/mimetypes/"

  test -d "${link_dir}" || mkdir -p "${link_dir}"
  test -d ${link_dir}/${link_target_dir} || continue

  icon=${link_target_dir}%iconprefix-drawing.png;                     test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.sun.xml.draw.png
  icon=${link_target_dir}%iconprefix-drawing-template.png;            test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.sun.xml.draw.template.png
  icon=${link_target_dir}%iconprefix-formula.png;                     test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.sun.xml.math.png
  icon=${link_target_dir}%iconprefix-master-document.png;             test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.sun.xml.writer.global.png
  icon=${link_target_dir}%iconprefix-oasis-database.png;              test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.sun.xml.base.png
  icon=${link_target_dir}%iconprefix-oasis-database.png;              test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.database.png
  icon=${link_target_dir}%iconprefix-oasis-drawing.png;               test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.graphics.png
  icon=${link_target_dir}%iconprefix-oasis-drawing-template.png;      test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.graphics-template.png
  icon=${link_target_dir}%iconprefix-oasis-formula.png;               test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.formula.png
  icon=${link_target_dir}%iconprefix-oasis-master-document.png;       test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.text-master.png
  icon=${link_target_dir}%iconprefix-oasis-presentation.png;          test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.presentation.png
  icon=${link_target_dir}%iconprefix-oasis-presentation-template.png; test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.presentation-template.png
  icon=${link_target_dir}%iconprefix-oasis-spreadsheet.png;           test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.spreadsheet.png
  icon=${link_target_dir}%iconprefix-oasis-spreadsheet-template.png;  test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.spreadsheet-template.png
  icon=${link_target_dir}%iconprefix-oasis-text.png;                  test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.text.png
  icon=${link_target_dir}%iconprefix-oasis-text-template.png;         test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.text-template.png
  icon=${link_target_dir}%iconprefix-oasis-web-template.png;          test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.text-web.png
  icon=${link_target_dir}%iconprefix-presentation.png;                test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.sun.xml.impress.png
  icon=${link_target_dir}%iconprefix-presentation-template.png;       test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.sun.xml.impress.template.png
  icon=${link_target_dir}%iconprefix-spreadsheet.png;                 test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.sun.xml.calc.png
  icon=${link_target_dir}%iconprefix-spreadsheet-template.png;        test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.sun.xml.calc.template.png
  icon=${link_target_dir}%iconprefix-text.png;                        test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.sun.xml.writer.png
  icon=${link_target_dir}%iconprefix-text-template.png;               test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.sun.xml.writer.template.png
  icon=${link_target_dir}%iconprefix-extension.png;                   test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.openofficeorg.extension.png
done

# run only on first install, since postun is run when updating
# post would be run before the old files are removed
if [ "$1" = "1" ] ; then  # first install
  for themedir in /opt/gnome/share/icons/gnome /opt/gnome/share/icons/hicolor /opt/kde3/share/icons/hicolor /opt/kde3/share/icons/locolor; do
    if [ -e $themedir/icon-theme.cache ] ; then
      # touch in, in case we cannot find gtk-update-icon-cache (just to make sure)
      touch $themedir
      # path to gtk-update-icon-cache is not in rpm's install_script_path by default.
      if [ -x /opt/gnome/bin/gtk-update-icon-cache ]; then
        /opt/gnome/bin/gtk-update-icon-cache $themedir
      fi
      # ignore errors (e.g. when there is a cache, but no index.theme)
      true
    fi
  done
fi

# update /etc/mime.types
# backing out existing entries to avoid duplicates
sed '
/application\/vnd\.oasis\.opendocument/d
/application\/vnd\.openofficeorg/d
/application\/vnd\.sun/d
/application\/vnd\.stardivision/d
' /etc/mime.types 2>/dev/null >> /etc/mime.types.tmp$$


# now append our stuff to the temporary file
cat >> /etc/mime.types.tmp$$ << END
application/vnd.oasis.opendocument.text odt
application/vnd.oasis.opendocument.text-flat-xml fodt
application/vnd.oasis.opendocument.text-template ott
application/vnd.oasis.opendocument.text-web oth
application/vnd.oasis.opendocument.text-master odm
application/vnd.oasis.opendocument.graphics odg
application/vnd.oasis.opendocument.graphics-flat-xml fodg
application/vnd.oasis.opendocument.graphics-template otg
application/vnd.oasis.opendocument.presentation odp
application/vnd.oasis.opendocument.presentation-flat-xml fodp
application/vnd.oasis.opendocument.presentation-template otp
application/vnd.oasis.opendocument.spreadsheet ods
application/vnd.oasis.opendocument.spreadsheet-flat-xml fods
application/vnd.oasis.opendocument.spreadsheet-template ots
application/vnd.oasis.opendocument.chart odc
application/vnd.oasis.opendocument.formula odf
application/vnd.oasis.opendocument.image odi
application/vnd.sun.xml.writer sxw
application/vnd.sun.xml.writer.template stw
application/vnd.sun.xml.writer.global sxg
application/vnd.stardivision.writer sdw vor
application/vnd.stardivision.writer-global sgl
application/vnd.sun.xml.calc sxc
application/vnd.sun.xml.calc.template stc
application/vnd.stardivision.calc sdc
application/vnd.stardivision.chart sds
application/vnd.sun.xml.impress sxi
application/vnd.sun.xml.impress.template sti
application/vnd.stardivision.impress sdd sdp
application/vnd.sun.xml.draw sxd
application/vnd.sun.xml.draw.template std
application/vnd.stardivision.draw sda
application/vnd.sun.xml.math sxm
application/vnd.stardivision.math smf
application/vnd.sun.xml.base odb
application/vnd.openofficeorg.extension oxt
application/vnd.openxmlformats-officedocument.wordprocessingml.document docx
application/vnd.ms-word.document.macroenabled.12 docm
application/vnd.openxmlformats-officedocument.wordprocessingml.template dotx
application/vnd.ms-word.template.macroenabled.12 dotm
application/vnd.openxmlformats-officedocument.spreadsheetml.sheet xlsx
application/vnd.ms-excel.sheet.macroenabled.12 xlsm
application/vnd.openxmlformats-officedocument.spreadsheetml.template xltx
application/vnd.ms-excel.template.macroenabled.12 xltm
application/vnd.openxmlformats-officedocument.presentationml.presentation pptx
application/vnd.ms-powerpoint.presentation.macroenabled.12 pptm
application/vnd.openxmlformats-officedocument.presentationml.template potx
application/vnd.ms-powerpoint.template.macroenabled.12 potm
END

# and replace the original file
mv -f /etc/mime.types.tmp$$ /etc/mime.types 2>/dev/null

# update /etc/mailcap only at initial install
if [ "$1" = 1 ]
then
  # backing out existing entries to avoid duplicates
  sed '
/^# OpenOffice.org/d
/^application\/vnd\.oasis\.opendocument/d
/^application\/vnd\.openofficeorg/d
/^application\/vnd\.sun/d
/^application\/vnd\.stardivision/d
/^application\/vnd\.ms-word/d
/^application\/vnd\.ms-excel/d
/^application\/vnd\.ms-powerpoint/d
/^application\/x-star/d
/excel/d
/ms[-]*word/d
/powerpoint/d
' /etc/mailcap 2>/dev/null >> /etc/mailcap.tmp$$

  # now append our stuff to the temporary file
  cat >> /etc/mailcap.tmp$$ << END
# LibreOffice
application/vnd.oasis.opendocument.text; %unixfilename -view %s
application/vnd.oasis.opendocument.text-flat-xml; %unixfilename -view %s
application/vnd.oasis.opendocument.text-template; %unixfilename -view %s
application/vnd.oasis.opendocument.text-web; %unixfilename -view %s
application/vnd.oasis.opendocument.text-master; %unixfilename -view %s
application/vnd.sun.xml.writer; %unixfilename -view %s
application/vnd.sun.xml.writer.template; %unixfilename -view %s
application/vnd.sun.xml.writer.global; %unixfilename -view %s
application/vnd.stardivision.writer; %unixfilename -view %s
application/vnd.stardivision.writer-global; %unixfilename -view %s
application/x-starwriter; %unixfilename -view %s
application/vnd.oasis.opendocument.formula; %unixfilename -view %s
application/vnd.sun.xml.math; %unixfilename -view %s
application/vnd.stardivision.math; %unixfilename -view %s
application/x-starmath; %unixfilename -view %s
application/msword; %unixfilename -view %s
application/vnd.oasis.opendocument.spreadsheet; %unixfilename -view %s
application/vnd.oasis.opendocument.spreadsheet-flat-xml; %unixfilename -view %s
application/vnd.oasis.opendocument.spreadsheet-template; %unixfilename -view %s
application/vnd.sun.xml.calc; %unixfilename -view %s
application/vnd.sun.xml.calc.template; %unixfilename -view %s
application/vnd.stardivision.calc; %unixfilename -view %s
application/x-starcalc; %unixfilename -view %s
application/vnd.stardivision.chart; %unixfilename -view %s
application/x-starchart; %unixfilename -view %s
application/excel; %unixfilename -view %s
application/msexcel; %unixfilename -view %s
application/vnd.ms-excel; %unixfilename -view %s
application/x-msexcel; %unixfilename -view %s
application/vnd.oasis.opendocument.presentation; %unixfilename -view %s
application/vnd.oasis.opendocument.presentation-flat-xml; %unixfilename -view %s
application/vnd.oasis.opendocument.presentation-template; %unixfilename -view %s
application/vnd.sun.xml.impress; %unixfilename -view %s
application/vnd.sun.xml.impress.template; %unixfilename -view %s
application/vnd.stardivision.impress; %unixfilename -view %s
application/x-starimpress; %unixfilename -view %s
application/powerpoint; %unixfilename -view %s
application/mspowerpoint; %unixfilename -view %s
application/vnd.ms-powerpoint; %unixfilename -view %s
application/x-mspowerpoint; %unixfilename -view %s
application/vnd.oasis.opendocument.graphics; %unixfilename -view %s
application/vnd.oasis.opendocument.graphics-flat-xml; %unixfilename -view %s
application/vnd.oasis.opendocument.graphics-template; %unixfilename -view %s
application/vnd.sun.xml.draw; %unixfilename -view %s
application/vnd.sun.xml.draw.template; %unixfilename -view %s
application/vnd.stardivision.draw; %unixfilename -view %s
application/x-stardraw; %unixfilename -view %s
application/vnd.oasis.opendocument.database; %unixfilename -view %s
application/vnd.sun.xml.base; %unixfilename -view %s
application/vnd.wordperfect; %unixfilename -view %s
application/wordperfect5.1; %unixfilename -view %s
application/x-wordperfect; %unixfilename -view %s
application/wordperfect; %unixfilename -view %s
application/vnd.lotus-wordpro; %unixfilename -view %s
application/wpwin; %unixfilename -view %s
application/vnd.openofficeorg.extension; %unixfilename %s
application/vnd.openxmlformats-officedocument.wordprocessingml.document; %unixfilename -view %s
application/vnd.ms-word.document.macroenabled.12;%unixfilename -view %s
application/vnd.openxmlformats-officedocument.wordprocessingml.template; %unixfilename -view %s
application/vnd.ms-word.template.macroenabled.12; %unixfilename -view %s
application/vnd.openxmlformats-officedocument.spreadsheetml.sheet; %unixfilename -view %s
application/vnd.ms-excel.sheet.macroenabled.12; %unixfilename -view %s
application/vnd.openxmlformats-officedocument.spreadsheetml.template; %unixfilename -view %s
application/vnd.ms-excel.template.macroenabled.12; %unixfilename -view %s
application/vnd.openxmlformats-officedocument.presentationml.presentation; %unixfilename -view %s
application/vnd.ms-powerpoint.presentation.macroenabled.12; %unixfilename -view %s
application/vnd.openxmlformats-officedocument.presentationml.template; %unixfilename -view %s
application/vnd.ms-powerpoint.template.macroenabled.12; %unixfilename -view %s
application/x-hwp; %unixfilename -view %s
END

  # and replace the original file
  mv -f /etc/mailcap.tmp$$ /etc/mailcap
fi

%preun
# remove from /etc/mailcap only on de-install
if [ "$1" = 0 ]
then
  # backing all entries pointing to our binary
  sed '/%unixfilename/d' /etc/mailcap 2>/dev/null >> /etc/mailcap.tmp$$

  # and replace the original file
  mv -f /etc/mailcap.tmp$$ /etc/mailcap
fi

%postun
# run only when erasing this package, since %post of the new package ran
# previously or updates already handled by triggers.
if [ "$1" = 0 ] ; then
  if [ -x /opt/gnome/bin/update-desktop-database ]; then
    /opt/gnome/bin/update-desktop-database -q /usr/share/applications
  fi
  if [ -x /usr/bin/update-mime-database ]; then
    update-mime-database /usr/share/mime
  fi
fi
#run always
for themedir in /opt/gnome/share/icons/gnome /opt/gnome/share/icons/hicolor /opt/kde3/share/icons/hicolor /opt/kde3/share/icons/locolor; do
  if [ -e $themedir/icon-theme.cache ] ; then
    # touch in, in case we cannot find gtk-update-icon-cache (just to make sure)
    touch $themedir
    # path to gtk-update-icon-cache is not in rpm's install_script_path by default.
    if [ -x /opt/gnome/bin/gtk-update-icon-cache ]; then
      /opt/gnome/bin/gtk-update-icon-cache $themedir
    fi
    # ignore errors (e.g. when there is a cache, but no index.theme)
    true
  fi
done

%files
%if "%unixfilename" != "libreoffice%productversion" && "%unixfilename" != "libreofficedev%productversion"
# compat symlinks
%attr(0755,root,root) /opt/%unixfilename
%attr(0755,root,root) /usr/bin/libreoffice%productversion
%attr(0755,root,root) /usr/bin/libreoffice%productversion-printeradmin
%endif
%attr(0755,root,root) %verify(not size md5) /usr/bin/%unixfilename
%attr(0755,root,root) /usr/bin/%unixfilename-printeradmin
%defattr(0644, root, root)
/opt/gnome/share/application-registry/*.applications
/usr/share/applications/%unixfilename-writer.desktop
/usr/share/applications/%unixfilename-calc.desktop
/usr/share/applications/%unixfilename-draw.desktop
/usr/share/applications/%unixfilename-impress.desktop
/usr/share/applications/%unixfilename-math.desktop
/usr/share/applications/%unixfilename-base.desktop
/usr/share/applications/%unixfilename-startcenter.desktop
/usr/share/applications/%unixfilename-xsltfilter.desktop
/opt/gnome/share/mime-info/*.keys
/opt/gnome/share/mime-info/*.mime
/opt/kde3/share/mimelnk/application/*.desktop
/opt/gnome/share/icons/gnome/*/apps/*png
/opt/gnome/share/icons/gnome/*/mimetypes/*png
/opt/gnome/share/icons/gnome/scalable/apps/*svg
/opt/gnome/share/icons/gnome/scalable/mimetypes/*svg
/opt/kde3/share/icons/hicolor/*/apps/*png
/opt/kde3/share/icons/hicolor/*/mimetypes/*png
/opt/kde3/share/icons/hicolor/scalable/apps/*svg
/opt/kde3/share/icons/hicolor/scalable/mimetypes/*svg
/opt/kde3/share/icons/locolor/*/apps/*png
/opt/kde3/share/icons/locolor/*/mimetypes/*png
/usr/share/mime/packages/*
