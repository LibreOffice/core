Summary: OpenOffice.org desktop integration
Name: openofficeorg-suse-menus
Version: 2.0
Release: 1
Group: Office
Copyright: LGPL / SISSL
AutoReqProv: no
Requires: openofficeorg-core, suse-release
%define _unpackaged_files_terminate_build 0
%description 
OpenOffice.org desktop integration

%triggerin -- openofficeorg-core
# create file in /etc that contains the office installation path
cat > /tmp/install.$$ << EOF
sleep 2
ln -sf \`rpm -q --qf '%{INSTALLPREFIX}' openofficeorg-core\` /etc/%PREFIX
rm -f /tmp/install.$$
EOF

/bin/sh /tmp/install.$$ &

%post
# update /etc/mime.types
# backing out existing entries to avoid duplicates
sed '
/application\/vnd\.oasis\.opendocument/d
/application\/vnd\.sun/d
/application\/vnd\.stardivision/d
' /etc/mime.types 2>/dev/null >> /etc/mime.types.tmp$$

# now append our stuff to the temporary file
cat >> /etc/mime.types.tmp$$ << END
application/vnd.oasis.opendocument.text odt
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
application/vnd.oasis.opendocument.database odb
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
application/vnd.oasis.opendocument.text; %PREFIX -view %s
application/vnd.oasis.opendocument.text-template; %PREFIX -view %s
application/vnd.oasis.opendocument.text-web; %PREFIX -view %s
application/vnd.oasis.opendocument.text-master; %PREFIX -view %s
application/vnd.sun.xml.writer; %PREFIX -view %s
application/vnd.sun.xml.writer.template; %PREFIX -view %s
application/vnd.sun.xml.writer.global; %PREFIX -view %s
application/vnd.stardivision.writer; %PREFIX -view %s
application/vnd.stardivision.writer-global; %PREFIX -view %s
application/x-starwriter; %PREFIX -view %s
application/vnd.oasis.opendocument.formula; %PREFIX -view %s
application/vnd.sun.xml.math; %PREFIX -view %s
application/vnd.stardivision.math; %PREFIX -view %s
application/x-starmath; %PREFIX -view %s
application/msword; %PREFIX -view %s
application/vnd.oasis.opendocument.spreadsheet; %PREFIX -view %s
application/vnd.oasis.opendocument.spreadsheet-template; %PREFIX -view %s
application/vnd.sun.xml.calc; %PREFIX -view %s
application/vnd.sun.xml.calc.template; %PREFIX -view %s
application/vnd.stardivision.calc; %PREFIX -view %s
application/x-starcalc; %PREFIX -view %s
application/vnd.stardivision.chart; %PREFIX -view %s
application/x-starchart; %PREFIX -view %s
application/excel; %PREFIX -view %s
application/msexcel; %PREFIX -view %s
application/vnd.ms-excel; %PREFIX -view %s
application/x-msexcel; %PREFIX -view %s
application/vnd.oasis.opendocument.presentation; %PREFIX -view %s
application/vnd.oasis.opendocument.presentation-template; %PREFIX -view %s
application/vnd.sun.xml.impress; %PREFIX -view %s
application/vnd.sun.xml.impress.template; %PREFIX -view %s
application/vnd.stardivision.impress; %PREFIX -view %s
application/x-starimpress; %PREFIX -view %s
application/powerpoint; %PREFIX -view %s
application/mspowerpoint; %PREFIX -view %s
application/vnd.ms-powerpoint; %PREFIX -view %s
application/x-mspowerpoint; %PREFIX -view %s
application/vnd.oasis.opendocument.graphics; %PREFIX -view %s
application/vnd.oasis.opendocument.graphics-template; %PREFIX -view %s
application/vnd.sun.xml.draw; %PREFIX -view %s
application/vnd.sun.xml.draw.template; %PREFIX -view %s
application/vnd.stardivision.draw; %PREFIX -view %s
application/x-stardraw; %PREFIX -view %s
application/vnd.oasis.opendocument.database; %PREFIX -view %s
END

  # and replace the original file
  mv -f /etc/mailcap.tmp$$ /etc/mailcapl
fi

# Check whether to activate the gnome-set-default-application stuff (GNOME < 2.6) 
if [ "$1" = "1" ]
then
  if [ -x /opt/gnome/bin/gnome-panel ]
  then
    /opt/gnome/bin/gnome-panel --version | grep ' 2\.[024]\.' > /dev/null
    if [ "$?" = "0" ]; then
      cat >> /usr/bin/%PREFIX.tmp$$ << EOF
#!/bin/sh
USERDIR=\`sed -n -e 's/UserInstallation=//p' /etc/%PREFIX/program/bootstraprc | sed -e "s|.SYSUSERCONFIG|\$HOME|"\`
# Run gnome-set-default-application on first office launch
if [ ! -d \$USERDIR ]
then
  /etc/%PREFIX/program/gnome-set-default-application '%PREFIX' 'application/vnd.oasis.opendocument' 'application/vnd.sun.xml' 'application/vnd.stardivision'
fi
EOF
      sed -n -e '2,$ p' /usr/bin/%PREFIX >> /usr/bin/%PREFIX.tmp$$
      mv -f /usr/bin/%PREFIX.tmp$$ /usr/bin/%PREFIX
      chmod 0755 /usr/bin/%PREFIX
    fi
  fi
fi

%preun
# remove from /etc/mailcap only on de-install
if [ "$1" = 0 ]
then
  # backing all entries pointing to our binary
  sed '/%PREFIX/d' /etc/mailcap 2>/dev/null >> /etc/mailcap.tmp$$

  # and replace the original file
  mv -f /etc/mailcap.tmp$$ /etc/mailcap
fi

%files
%attr(0755,root,root) %verify(not size md5) /usr/bin/%PREFIX
%attr(0755,root,root) /usr/bin/%PREFIX-printeradmin
%defattr(0644, root, root)
%ghost /etc/%PREFIX
/opt/gnome/share/application-registry/%PREFIX.applications
/opt/gnome/share/applications/%PREFIX-writer.desktop
/opt/gnome/share/applications/%PREFIX-calc.desktop
/opt/gnome/share/applications/%PREFIX-draw.desktop
/opt/gnome/share/applications/%PREFIX-impress.desktop
/opt/gnome/share/applications/%PREFIX-math.desktop
/opt/gnome/share/applications/%PREFIX-base.desktop
/opt/gnome/share/applications/%PREFIX-printeradmin.desktop
/opt/kde3/share/applnk/Office/%PREFIX-writer.desktop
/opt/kde3/share/applnk/Office/%PREFIX-calc.desktop
/opt/kde3/share/applnk/Office/%PREFIX-draw.desktop
/opt/kde3/share/applnk/Office/%PREFIX-impress.desktop
/opt/kde3/share/applnk/Office/%PREFIX-math.desktop
/opt/kde3/share/applnk/Office/%PREFIX-base.desktop
/opt/kde3/share/applnk/Office/%PREFIX-printeradmin.desktop
/opt/gnome/share/mime-info/%PREFIX.keys
/opt/gnome/share/mime-info/%PREFIX.mime
/opt/kde3/share/mimelnk/application/%PREFIX-text.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-text-template.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-spreadsheet.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-spreadsheet-template.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-drawing.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-drawing-template.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-presentation.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-presentation-template.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-master-document.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-formula.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-oasis-text.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-oasis-text-template.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-oasis-spreadsheet.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-oasis-spreadsheet-template.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-oasis-drawing.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-oasis-drawing-template.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-oasis-presentation.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-oasis-presentation-template.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-oasis-master-document.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-oasis-formula.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-oasis-database.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-oasis-web-template.desktop
/opt/gnome/share/icons/gnome/16x16/apps/%ICONPREFIX-writer.png
/opt/gnome/share/icons/gnome/16x16/apps/%ICONPREFIX-calc.png
/opt/gnome/share/icons/gnome/16x16/apps/%ICONPREFIX-draw.png
/opt/gnome/share/icons/gnome/16x16/apps/%ICONPREFIX-impress.png
/opt/gnome/share/icons/gnome/16x16/apps/%ICONPREFIX-math.png
/opt/gnome/share/icons/gnome/16x16/apps/%ICONPREFIX-base.png
/opt/gnome/share/icons/gnome/16x16/apps/%ICONPREFIX-printeradmin.png
/opt/gnome/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-text.png
/opt/gnome/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-text-template.png
/opt/gnome/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-spreadsheet.png
/opt/gnome/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-spreadsheet-template.png
/opt/gnome/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-drawing.png
/opt/gnome/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-drawing-template.png
/opt/gnome/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-presentation.png
/opt/gnome/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-presentation-template.png
/opt/gnome/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-master-document.png
/opt/gnome/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-formula.png
/opt/gnome/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-database.png
/opt/gnome/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-text.png
/opt/gnome/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-text-template.png
/opt/gnome/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-spreadsheet.png
/opt/gnome/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-spreadsheet-template.png
/opt/gnome/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-drawing.png
/opt/gnome/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-drawing-template.png
/opt/gnome/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-presentation.png
/opt/gnome/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-presentation-template.png
/opt/gnome/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-master-document.png
/opt/gnome/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-formula.png
/opt/gnome/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-database.png
/opt/gnome/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-web-template.png
/opt/gnome/share/icons/gnome/32x32/apps/%ICONPREFIX-writer.png
/opt/gnome/share/icons/gnome/32x32/apps/%ICONPREFIX-calc.png
/opt/gnome/share/icons/gnome/32x32/apps/%ICONPREFIX-draw.png
/opt/gnome/share/icons/gnome/32x32/apps/%ICONPREFIX-impress.png
/opt/gnome/share/icons/gnome/32x32/apps/%ICONPREFIX-math.png
/opt/gnome/share/icons/gnome/32x32/apps/%ICONPREFIX-base.png
/opt/gnome/share/icons/gnome/32x32/apps/%ICONPREFIX-printeradmin.png
/opt/gnome/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-text.png
/opt/gnome/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-text-template.png
/opt/gnome/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-spreadsheet.png
/opt/gnome/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-spreadsheet-template.png
/opt/gnome/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-drawing.png
/opt/gnome/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-drawing-template.png
/opt/gnome/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-presentation.png
/opt/gnome/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-presentation-template.png
/opt/gnome/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-master-document.png
/opt/gnome/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-formula.png
/opt/gnome/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-database.png
/opt/gnome/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-text.png
/opt/gnome/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-text-template.png
/opt/gnome/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-spreadsheet.png
/opt/gnome/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-spreadsheet-template.png
/opt/gnome/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-drawing.png
/opt/gnome/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-drawing-template.png
/opt/gnome/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-presentation.png
/opt/gnome/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-presentation-template.png
/opt/gnome/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-master-document.png
/opt/gnome/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-formula.png
/opt/gnome/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-database.png
/opt/gnome/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-web-template.png
/opt/gnome/share/icons/gnome/48x48/apps/%ICONPREFIX-writer.png
/opt/gnome/share/icons/gnome/48x48/apps/%ICONPREFIX-calc.png
/opt/gnome/share/icons/gnome/48x48/apps/%ICONPREFIX-draw.png
/opt/gnome/share/icons/gnome/48x48/apps/%ICONPREFIX-impress.png
/opt/gnome/share/icons/gnome/48x48/apps/%ICONPREFIX-math.png
/opt/gnome/share/icons/gnome/48x48/apps/%ICONPREFIX-base.png
/opt/gnome/share/icons/gnome/48x48/apps/%ICONPREFIX-printeradmin.png
/opt/gnome/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-text.png
/opt/gnome/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-text-template.png
/opt/gnome/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-spreadsheet.png
/opt/gnome/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-spreadsheet-template.png
/opt/gnome/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-drawing.png
/opt/gnome/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-drawing-template.png
/opt/gnome/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-presentation.png
/opt/gnome/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-presentation-template.png
/opt/gnome/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-master-document.png
/opt/gnome/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-formula.png
/opt/gnome/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-database.png
/opt/gnome/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-text.png
/opt/gnome/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-text-template.png
/opt/gnome/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-spreadsheet.png
/opt/gnome/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-spreadsheet-template.png
/opt/gnome/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-drawing.png
/opt/gnome/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-drawing-template.png
/opt/gnome/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-presentation.png
/opt/gnome/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-presentation-template.png
/opt/gnome/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-master-document.png
/opt/gnome/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-formula.png
/opt/gnome/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-database.png
/opt/gnome/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-web-template.png
/opt/gnome/share/icons/HighContrast/16x16/apps/%ICONPREFIX-writer.png
/opt/gnome/share/icons/HighContrast/16x16/apps/%ICONPREFIX-calc.png
/opt/gnome/share/icons/HighContrast/16x16/apps/%ICONPREFIX-draw.png
/opt/gnome/share/icons/HighContrast/16x16/apps/%ICONPREFIX-impress.png
/opt/gnome/share/icons/HighContrast/16x16/apps/%ICONPREFIX-math.png
/opt/gnome/share/icons/HighContrast/16x16/apps/%ICONPREFIX-base.png
/opt/gnome/share/icons/HighContrast/16x16/apps/%ICONPREFIX-printeradmin.png
/opt/gnome/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-text.png
/opt/gnome/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-text-template.png
/opt/gnome/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-spreadsheet.png
/opt/gnome/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-spreadsheet-template.png
/opt/gnome/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-drawing.png
/opt/gnome/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-drawing-template.png
/opt/gnome/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-presentation.png
/opt/gnome/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-presentation-template.png
/opt/gnome/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-master-document.png
/opt/gnome/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-formula.png
/opt/gnome/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-database.png
/opt/gnome/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-web-template.png
/opt/gnome/share/icons/HighContrast/32x32/apps/%ICONPREFIX-writer.png
/opt/gnome/share/icons/HighContrast/32x32/apps/%ICONPREFIX-calc.png
/opt/gnome/share/icons/HighContrast/32x32/apps/%ICONPREFIX-draw.png
/opt/gnome/share/icons/HighContrast/32x32/apps/%ICONPREFIX-impress.png
/opt/gnome/share/icons/HighContrast/32x32/apps/%ICONPREFIX-math.png
/opt/gnome/share/icons/HighContrast/32x32/apps/%ICONPREFIX-base.png
/opt/gnome/share/icons/HighContrast/32x32/apps/%ICONPREFIX-printeradmin.png
/opt/gnome/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-text.png
/opt/gnome/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-text-template.png
/opt/gnome/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-spreadsheet.png
/opt/gnome/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-spreadsheet-template.png
/opt/gnome/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-drawing.png
/opt/gnome/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-drawing-template.png
/opt/gnome/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-presentation.png
/opt/gnome/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-presentation-template.png
/opt/gnome/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-master-document.png
/opt/gnome/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-formula.png
/opt/gnome/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-database.png
/opt/gnome/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-web-template.png
/opt/gnome/share/icons/HighContrast/48x48/apps/%ICONPREFIX-writer.png
/opt/gnome/share/icons/HighContrast/48x48/apps/%ICONPREFIX-calc.png
/opt/gnome/share/icons/HighContrast/48x48/apps/%ICONPREFIX-draw.png
/opt/gnome/share/icons/HighContrast/48x48/apps/%ICONPREFIX-impress.png
/opt/gnome/share/icons/HighContrast/48x48/apps/%ICONPREFIX-math.png
/opt/gnome/share/icons/HighContrast/48x48/apps/%ICONPREFIX-base.png
/opt/gnome/share/icons/HighContrast/48x48/apps/%ICONPREFIX-printeradmin.png
/opt/gnome/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-text.png
/opt/gnome/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-text-template.png
/opt/gnome/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-spreadsheet.png
/opt/gnome/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-spreadsheet-template.png
/opt/gnome/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-drawing.png
/opt/gnome/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-drawing-template.png
/opt/gnome/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-presentation.png
/opt/gnome/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-presentation-template.png
/opt/gnome/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-master-document.png
/opt/gnome/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-formula.png
/opt/gnome/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-database.png
/opt/gnome/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-web-template.png
/opt/kde3/share/icons/hicolor/16x16/apps/%ICONPREFIX-writer.png
/opt/kde3/share/icons/hicolor/16x16/apps/%ICONPREFIX-calc.png
/opt/kde3/share/icons/hicolor/16x16/apps/%ICONPREFIX-draw.png
/opt/kde3/share/icons/hicolor/16x16/apps/%ICONPREFIX-impress.png
/opt/kde3/share/icons/hicolor/16x16/apps/%ICONPREFIX-math.png
/opt/kde3/share/icons/hicolor/16x16/apps/%ICONPREFIX-base.png
/opt/kde3/share/icons/hicolor/16x16/apps/%ICONPREFIX-printeradmin.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-text.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-text-template.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-spreadsheet.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-spreadsheet-template.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-drawing.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-drawing-template.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-presentation.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-presentation-template.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-master-document.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-formula.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-database.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-text.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-text-template.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-spreadsheet.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-spreadsheet-template.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-drawing.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-drawing-template.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-presentation.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-presentation-template.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-master-document.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-formula.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-database.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-web-template.png
/opt/kde3/share/icons/hicolor/32x32/apps/%ICONPREFIX-writer.png
/opt/kde3/share/icons/hicolor/32x32/apps/%ICONPREFIX-calc.png
/opt/kde3/share/icons/hicolor/32x32/apps/%ICONPREFIX-draw.png
/opt/kde3/share/icons/hicolor/32x32/apps/%ICONPREFIX-impress.png
/opt/kde3/share/icons/hicolor/32x32/apps/%ICONPREFIX-math.png
/opt/kde3/share/icons/hicolor/32x32/apps/%ICONPREFIX-base.png
/opt/kde3/share/icons/hicolor/32x32/apps/%ICONPREFIX-printeradmin.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-text.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-text-template.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-spreadsheet.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-spreadsheet-template.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-drawing.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-drawing-template.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-presentation.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-presentation-template.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-master-document.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-formula.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-database.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-text.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-text-template.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-spreadsheet.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-spreadsheet-template.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-drawing.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-drawing-template.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-presentation.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-presentation-template.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-master-document.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-formula.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-database.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-web-template.png
/opt/kde3/share/icons/hicolor/48x48/apps/%ICONPREFIX-writer.png
/opt/kde3/share/icons/hicolor/48x48/apps/%ICONPREFIX-calc.png
/opt/kde3/share/icons/hicolor/48x48/apps/%ICONPREFIX-draw.png
/opt/kde3/share/icons/hicolor/48x48/apps/%ICONPREFIX-impress.png
/opt/kde3/share/icons/hicolor/48x48/apps/%ICONPREFIX-math.png
/opt/kde3/share/icons/hicolor/48x48/apps/%ICONPREFIX-base.png
/opt/kde3/share/icons/hicolor/48x48/apps/%ICONPREFIX-printeradmin.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-text.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-text-template.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-spreadsheet.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-spreadsheet-template.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-drawing.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-drawing-template.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-presentation.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-presentation-template.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-master-document.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-formula.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-database.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-text.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-text-template.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-spreadsheet.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-spreadsheet-template.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-drawing.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-drawing-template.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-presentation.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-presentation-template.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-master-document.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-formula.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-database.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-web-template.png
/opt/kde3/share/icons/locolor/16x16/apps/%ICONPREFIX-writer.png
/opt/kde3/share/icons/locolor/16x16/apps/%ICONPREFIX-calc.png
/opt/kde3/share/icons/locolor/16x16/apps/%ICONPREFIX-draw.png
/opt/kde3/share/icons/locolor/16x16/apps/%ICONPREFIX-impress.png
/opt/kde3/share/icons/locolor/16x16/apps/%ICONPREFIX-math.png
/opt/kde3/share/icons/locolor/16x16/apps/%ICONPREFIX-base.png
/opt/kde3/share/icons/locolor/16x16/apps/%ICONPREFIX-printeradmin.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-text.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-text-template.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-spreadsheet.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-spreadsheet-template.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-drawing.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-drawing-template.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-presentation.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-presentation-template.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-master-document.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-formula.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-database.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-text.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-text-template.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-spreadsheet.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-spreadsheet-template.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-drawing.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-drawing-template.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-presentation.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-presentation-template.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-master-document.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-formula.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-database.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-web-template.png
/opt/kde3/share/icons/locolor/32x32/apps/%ICONPREFIX-writer.png
/opt/kde3/share/icons/locolor/32x32/apps/%ICONPREFIX-calc.png
/opt/kde3/share/icons/locolor/32x32/apps/%ICONPREFIX-draw.png
/opt/kde3/share/icons/locolor/32x32/apps/%ICONPREFIX-impress.png
/opt/kde3/share/icons/locolor/32x32/apps/%ICONPREFIX-math.png
/opt/kde3/share/icons/locolor/32x32/apps/%ICONPREFIX-base.png
/opt/kde3/share/icons/locolor/32x32/apps/%ICONPREFIX-printeradmin.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-text.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-text-template.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-spreadsheet.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-spreadsheet-template.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-drawing.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-drawing-template.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-presentation.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-presentation-template.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-master-document.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-formula.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-database.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-text.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-text-template.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-spreadsheet.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-spreadsheet-template.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-drawing.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-drawing-template.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-presentation.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-presentation-template.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-master-document.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-formula.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-database.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-web-template.png
%define _rpmdir /export/home/obr/workspaces/sysui03/sysui/unxlngi6.pro/bin
