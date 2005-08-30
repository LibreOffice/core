# version and release passed by command-line
Version: %version
Release: %release
Summary: OpenOffice.org desktop integration
Name: openoffice.org-redhat-menus
Group: Office
License: LGPL, SISSL
Vendor: OpenOffice.org
AutoReqProv: no
BuildArch: noarch
Requires: openoffice.org-core01, redhat-release
Provides: openoffice.org-desktop-integration
Obsoletes: openofficeorg-redhat-menus
%define _unpackaged_files_terminate_build 0
%description 
OpenOffice.org desktop integration

%install
## add symlinks so that nautilus can identify the mime-icons 
## not strictly freedesktop-stuff but there is no common naming scheme yet.
## One proposal is "mime-application:vnd.oasis.opendocument.spreadsheet.png"
## for e.g. application/vnd.oasis.opendocument.spreadsheet
cd $RPM_BUILD_ROOT/usr/share/icons/hicolor
originalname=%unixfilename
iconname=`echo $originalname | sed -e 's/\.//g'`
for dir in *; do
  cd $dir/mimetypes
  ln -sf $iconname-drawing.png                gnome-mime-application-vnd.sun.xml.draw.png
  ln -sf $iconname-drawing-template.png       gnome-mime-application-vnd.sun.xml.draw.template.png
  ln -sf $iconname-formula.png                gnome-mime-application-vnd.sun.xml.math.png
  ln -sf $iconname-master-document.png        gnome-mime-application-vnd.sun.xml.writer.global.png
  ln -sf $iconname-oasis-database.png         gnome-mime-application-vnd.sun.xml.base.png
  ln -sf $iconname-oasis-database.png         gnome-mime-application-vnd.oasis.opendocument.database.png
  ln -sf $iconname-oasis-drawing.png          gnome-mime-application-vnd.oasis.opendocument.graphics.png
  ln -sf $iconname-oasis-drawing-template.png gnome-mime-application-vnd.oasis.opendocument.graphics-template.png
  ln -sf $iconname-oasis-formula.png          gnome-mime-application-vnd.oasis.opendocument.formula.png
  ln -sf $iconname-oasis-master-document.png  gnome-mime-application-vnd.oasis.opendocument.text-master.png
  ln -sf $iconname-oasis-presentation.png     gnome-mime-application-vnd.oasis.opendocument.presentation.png
  ln -sf $iconname-oasis-presentation-template.png gnome-mime-application-vnd.oasis.opendocument.presentation-template.png
  ln -sf $iconname-oasis-spreadsheet.png           gnome-mime-application-vnd.oasis.opendocument.spreadsheet.png
  ln -sf $iconname-oasis-spreadsheet-template.png  gnome-mime-application-vnd.oasis.opendocument.spreadsheet-template.png
  ln -sf $iconname-oasis-text.png             gnome-mime-application-vnd.oasis.opendocument.text.png
  ln -sf $iconname-oasis-text-template.png    gnome-mime-application-vnd.oasis.opendocument.text-template.png
  ln -sf $iconname-oasis-web-template.png     gnome-mime-application-vnd.oasis.opendocument.text-web.png
  ln -sf $iconname-presentation.png           gnome-mime-application-vnd.sun.xml.impress.png
  ln -sf $iconname-presentation-template.png  gnome-mime-application-vnd.sun.xml.impress.template.png
  ln -sf $iconname-spreadsheet.png            gnome-mime-application-vnd.sun.xml.calc.png
  ln -sf $iconname-spreadsheet-template.png   gnome-mime-application-vnd.sun.xml.calc.template.png
  ln -sf $iconname-text.png                   gnome-mime-application-vnd.sun.xml.writer.png
  ln -sf $iconname-text-template.png          gnome-mime-application-vnd.sun.xml.writer.template.png
  cd ../..  
done

%triggerin -- openoffice.org-core01
# create file in /etc that contains the office installation path
cat > /tmp/install.$$ << EOF
while [ "\$TARGET" == "" ]
do
  sleep 2
  TARGET=\`rpm -q --qf '%{INSTALLPREFIX}' openoffice.org-core01 2>&1\` && ln -snf \$TARGET /etc/%unixfilename
  # some rpm versions do not wait for the shared lock
  echo \$TARGET | grep '/var/lib/rpm' && TARGET=""
done
if [ -x /usr/bin/update-desktop-database ]; then
  update-desktop-database -q /usr/share/applications
fi
rm -f /tmp/install.$$
EOF

/bin/sh /tmp/install.$$ &

%triggerin -- openoffice.org-writer, openoffice.org-calc, openoffice.org-draw, openoffice.org-impress, openoffice.org-base, openoffice.org-math
if [ -x /usr/bin/update-desktop-database -a -h /etc/%unixfilename ]; then
  update-desktop-database -q /usr/share/applications
fi

%triggerun -- openoffice.org-writer, openoffice.org-calc, openoffice.org-draw, openoffice.org-impress, openoffice.org-base, openoffice.org-math
if [ "$1" = "0" ] ; then  
  # the menu-package gets uninstalled/updated - postun will run the command
  exit 0
fi
if [ "$2" = "0" ] ; then  
  # the triggering package gets removed
  if [ -x /usr/bin/update-desktop-database ]; then
    update-desktop-database -q /usr/share/applications
  fi
fi

%post
# run always, since there are versions of this package that did not include
# a shared-mime-info xml file
if [ -x /usr/bin/update-mime-database ]; then
  update-mime-database /usr/share/mime
fi

# update /etc/mime.types
# backing out existing entries to avoid duplicates
sed '
/application\/vnd\.oasis\.opendocument/d
/application\/vnd\.sun/d
/application\/vnd\.stardivision/d
' /etc/mime.types 2>/dev/null >> /etc/mime.types.tmp$$

# now append our stuff to the temporary file
cat >> /etc/mime.types.tmp$$ << END
application/vnd.oasis.opendocument.text	odt
application/vnd.oasis.opendocument.text-template ott
application/vnd.oasis.opendocument.text-web oth
application/vnd.oasis.opendocument.text-master odm
application/vnd.oasis.opendocument.graphics odg
application/vnd.oasis.opendocument.graphics-template otg
application/vnd.oasis.opendocument.presentation odp
application/vnd.oasis.opendocument.presentation-template otp
application/vnd.oasis.opendocument.spreadsheet ods
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
application/vnd.oasis.opendocument.graphics-template; %unixfilename -view %s
application/vnd.sun.xml.draw; %unixfilename -view %s
application/vnd.sun.xml.draw.template; %unixfilename -view %s
application/vnd.stardivision.draw; %unixfilename -view %s
application/x-stardraw; %unixfilename -view %s
application/vnd.oasis.opendocument.database; %unixfilename -view %s
application/vnd.sun.xml.base; %unixfilename -view %s
application/vnd.writerperfect; %unixfilename -view %s
application/wordperfect5.1; %unixfilename -view %s
application/x-wordperfect; %unixfilename -view %s
application/wordperfect; %unixfilename -view %s
application/wpwin; %unixfilename -view %s
END

  # and replace the original file
  mv -f /etc/mailcap.tmp$$ /etc/mailcap
fi

# Check whether to activate the gnome-set-default-application stuff (GNOME < 2.6) 
if [ "$1" = "1" ]
then
  if [ -x /usr/bin/gnome-panel ]
  then
    /usr/bin/gnome-panel --version | grep ' 2\.[024]\.' > /dev/null
    if [ "$?" = "0" ]; then
      cat >> /usr/bin/%unixfilename.tmp$$ << EOF
#!/bin/sh
USERDIR=\`sed -n -e 's/UserInstallation=//p' /etc/%unixfilename/program/bootstraprc | sed -e "s|.SYSUSERCONFIG|\$HOME|"\`
# Run gnome-set-default-application on first office launch
if [ ! -d \$USERDIR ]
then
  /etc/%unixfilename/program/gnome-set-default-application '%unixfilename' 'application/vnd.oasis.opendocument' 'application/vnd.sun.xml' 'application/vnd.stardivision'
fi
EOF
      sed -n -e '2,$ p' /usr/bin/%unixfilename >> /usr/bin/%unixfilename.tmp$$
      mv -f /usr/bin/%unixfilename.tmp$$ /usr/bin/%unixfilename
      chmod 0755 /usr/bin/%unixfilename
    fi
  fi
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
  if [ -x /usr/bin/update-desktop-database ]; then
    update-desktop-database -q /usr/share/applications
  fi
  if [ -x /usr/bin/update-mime-database ]; then
    update-mime-database /usr/share/mime
  fi
fi

%files
%attr(0755,root,root) /usr/bin/soffice
%attr(0755,root,root) %verify(not size md5) /usr/bin/%unixfilename
%attr(0755,root,root) /usr/bin/%unixfilename-printeradmin
%defattr(0644, root, root)
%ghost /etc/%unixfilename
/usr/share/application-registry/*.applications
/usr/share/applications/%unixfilename-writer.desktop
/usr/share/applications/%unixfilename-calc.desktop
/usr/share/applications/%unixfilename-draw.desktop
/usr/share/applications/%unixfilename-impress.desktop
/usr/share/applications/%unixfilename-math.desktop
/usr/share/applications/%unixfilename-base.desktop
/usr/share/applications/%unixfilename-printeradmin.desktop
/usr/share/applnk-redhat/Office/%unixfilename-writer.desktop
/usr/share/applnk-redhat/Office/%unixfilename-calc.desktop
/usr/share/applnk-redhat/Office/%unixfilename-draw.desktop
/usr/share/applnk-redhat/Office/%unixfilename-impress.desktop
/usr/share/applnk-redhat/Office/%unixfilename-math.desktop
/usr/share/applnk-redhat/Office/%unixfilename-base.desktop
/usr/share/applnk-redhat/Office/%unixfilename-printeradmin.desktop
/usr/share/mime-info/*.keys
/usr/share/mime-info/*.mime
/usr/share/mimelnk/application/*.desktop
/usr/share/icons/gnome/*/apps/*png
/usr/share/icons/gnome/*/mimetypes/*png
/usr/share/icons/hicolor/*/apps/*png
/usr/share/icons/hicolor/*/mimetypes/*png
/usr/share/icons/locolor/*/apps/*png
/usr/share/icons/locolor/*/mimetypes/*png
/usr/share/mime/packages/*
