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
Name: %pkgprefix-freedesktop-menus
#BuildRequires: sed
#BuildRequires: perl
Group: Office
License: LGPLv3 with MPLv2, ALv2 and others
Provides: libreoffice-desktop-integration
Conflicts: %pkgprefix-suse-menus
Conflicts: %pkgprefix-debian-menus
Conflicts: %pkgprefix-redhat-menus
Conflicts: %pkgprefix-mandriva-menus
BuildArch: noarch
AutoReqProv: no
%define _binary_filedigest_algorithm 1
%define _binary_payload w9.gzdio

%define gnome_dir /usr
%define gnome_mime_theme hicolor

%description
%productname desktop integration for desktop-environments that implement
the menu- and mime-related specifications from http://www.freedesktop.org
These specifications are implemented by all current distributions.

%install
rm -rf $RPM_BUILD_ROOT

# hack/workaround to make SuSE's brp-symlink-script happy. It wants the targets of all links
# to be present on the build-system/the buildroot. But the point is that we generate stale
# links intentionally (until we find a better solution) #46226
export NO_BRP_STALE_LINK_ERROR=yes

mkdir -p $RPM_BUILD_ROOT

# set parameters for the create_tree script
export DESTDIR=$RPM_BUILD_ROOT
export KDEMAINDIR=/usr
export PREFIXDIR=/usr
export GNOMEDIR=%{gnome_dir}

./create_tree.sh

cd $RPM_BUILD_ROOT

# freedesktop-based desktop-environments don't need/use this.
rm -rf usr/share/application-registry
rm -rf usr/share/applications.flag
rm -rf usr/share/mime-info
rm -rf usr/share/mimelnk
rm -rf usr/share/applnk-redhat
#find usr/share/icons -name '*.png' -exec chmod g+w {} \;

%clean
rm -rf $RPM_BUILD_ROOT

%triggerin -- %pkgprefix, %pkgprefix-writer, %pkgprefix-calc, %pkgprefix-draw, %pkgprefix-impress, %pkgprefix-math
# this is run when one of the above packages is already installed and the menu
# package gets installed OR when the menu-package is already installed and one
# of the above listed packages gets installed

# Dut to a bug in rpm it is not possible to check why the script is triggered...
# This is how it should be: 1st arg: number of this package, 2nd arg: number of
# package that triggers - the bug is that rpm reports the same number for both
# (the value of the 2nd one), so just run this always...
# http://rhn.redhat.com/errata/RHBA-2004-098.html
# https://bugzilla.redhat.com/bugzilla/show_bug.cgi?id=100509

if [ -x /opt/gnome/bin/update-desktop-database ]; then
    /opt/gnome/bin/update-desktop-database -q
elif (which update-desktop-database); then
  update-desktop-database -q /usr/share/applications
fi

%triggerun -- %pkgprefix, %pkgprefix-writer, %pkgprefix-calc, %pkgprefix-draw, %pkgprefix-impress, %pkgprefix-math
if [ "$1" = "0" ] ; then
  # the menu-package gets uninstalled/updated - postun will run the command
  exit 0
fi
if [ "$2" = "0" ] ; then
  # the triggering package gets removed
  if [ -x /opt/gnome/bin/update-desktop-database ]; then
      /opt/gnome/bin/update-desktop-database -q
  elif (which update-desktop-database); then
    update-desktop-database -q /usr/share/applications
  fi
fi

%post
# no need to run it when updating, since %postun of the old package is run
# afterwards

if [ "$1" = "1" ] ; then  # first install
  if [ -x /opt/gnome/bin/update-desktop-database ]; then
    /opt/gnome/bin/update-desktop-database -q
  elif (which update-desktop-database); then
    update-desktop-database -q /usr/share/applications
  fi

  if (which update-mime-database); then
    update-mime-database /usr/share/mime
  fi
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
  icon=${link_target_dir}%iconprefix-oasis-master-document-template.png;       test -f ${link_dir}/$icon && ln -sf ${icon} ${link_dir}/gnome-mime-application-vnd.oasis.opendocument.text-master-template.png
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

#run always
for theme in gnome hicolor locolor; do
    if [ -e /usr/share/icons/$theme/icon-theme.cache ] ; then
        # touch it, just in case we cannot find the binary...
        touch /usr/share/icons/$theme
        if [ -x /opt/gnome/bin/gtk-update-icon-cache ]; then
            /opt/gnome/bin/gtk-update-icon-cache -q /usr/share/icons/$theme
        elif (which gtk-update-icon-cache); then
            gtk-update-icon-cache -q /usr/share/icons/$theme
        fi
        # ignore errors (e.g. when there is a cache, but no index.theme)
        true
    fi
done


if [ -x /opt/gnome/bin/update-desktop-database ]; then
    /opt/gnome/bin/update-desktop-database -q
elif (which update-desktop-database); then
  update-desktop-database -q /usr/share/applications
fi

%preun

%postun
if [ "$1" = 0 ] ; then # only run when erasing the package - other cases handled by the triggers
  if [ -x /opt/gnome/bin/update-desktop-database ]; then
    /opt/gnome/bin/update-desktop-database -q
  elif (which update-desktop-database); then
    update-desktop-database -q
  fi
# run always - both when upgrading as well as when erasing the package
  if (which update-mime-database); then
    update-mime-database /usr/share/mime
  fi
fi

#run always
for theme in gnome hicolor locolor; do
  if [ -e /usr/share/icons/$theme/icon-theme.cache ] ; then
   # touch it, just in case we cannot find the binary...
    touch /usr/share/icons/$theme
    if [ -x /opt/gnome/bin/gtk-update-icon-cache ]; then
      /opt/gnome/bin/gtk-update-icon-cache -q /usr/share/icons/$theme
    elif (which gtk-update-icon-cache); then
      gtk-update-icon-cache -q /usr/share/icons/$theme
    fi
    # ignore errors (e.g. when there is a cache, but no index.theme)
    true
  fi
done

%files
# specify stale symlinks verbatim, not as glob - a change in recent versions of
# glibc breaks rpm unless rpm is build with internal glob-matching (issue 49374)
# https://bugzilla.redhat.com/beta/show_bug.cgi?id=134362
%defattr(-, root, root)
%if "%unixfilename" != "libreoffice%productversion" && "%unixfilename" != "libreofficedev%productversion"
# compat symlinks
%attr(0755,root,root) /opt/%unixfilename
%endif
%attr(0755, root, root) /usr/bin/*
/usr/share/applications/%unixfilename-base.desktop
/usr/share/applications/%unixfilename-calc.desktop
/usr/share/applications/%unixfilename-draw.desktop
/usr/share/applications/%unixfilename-impress.desktop
/usr/share/applications/%unixfilename-math.desktop
/usr/share/applications/%unixfilename-writer.desktop
/usr/share/applications/%unixfilename-startcenter.desktop
/usr/share/applications/%unixfilename-xsltfilter.desktop
/usr/share/icons/gnome/*/apps/*png
/usr/share/icons/gnome/*/mimetypes/*png
/usr/share/icons/gnome/scalable/apps/*svg
/usr/share/icons/gnome/scalable/mimetypes/*svg
/usr/share/icons/hicolor/*/apps/*png
/usr/share/icons/hicolor/*/mimetypes/*png
/usr/share/icons/hicolor/scalable/apps/*svg
/usr/share/icons/hicolor/scalable/mimetypes/*svg
/usr/share/icons/locolor/*/apps/*png
/usr/share/icons/locolor/*/mimetypes/*png
/usr/share/mime/packages/*
/usr/share/appdata/*
