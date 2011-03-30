# version and release passed by command-line
Version: %version
Release: %release
Summary: %productname desktop integration
Name: %pkgprefix-suse-menus
Group: Office
License: LGPL
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
export GNOMEDIR=/opt/gnome
export GNOME_MIME_THEME=hicolor

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
# OpenOffice.org
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
%if %unixfilename != libreoffice
# compat symlinks
%attr(0755,root,root) /opt/%unixfilename
%attr(0755,root,root) /usr/bin/libreoffice
%attr(0755,root,root) /usr/bin/libreoffice-printeradmin
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
/usr/share/applications/%unixfilename-printeradmin.desktop
/usr/share/applications/%unixfilename-startcenter.desktop
/usr/share/applications/%unixfilename-javafilter.desktop
/opt/gnome/share/mime-info/*.keys
/opt/gnome/share/mime-info/*.mime
/opt/kde3/share/mimelnk/application/*.desktop
/opt/gnome/share/icons/gnome/*/apps/*png
/opt/gnome/share/icons/gnome/*/mimetypes/*png
/opt/gnome/share/icons/hicolor/*/mimetypes/*png
/opt/kde3/share/icons/hicolor/*/apps/*png
/opt/kde3/share/icons/hicolor/*/mimetypes/*png
/opt/kde3/share/icons/locolor/*/apps/*png
/opt/kde3/share/icons/locolor/*/mimetypes/*png
/usr/share/mime/packages/*
