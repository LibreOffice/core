Summary: OpenOffice.org desktop integration
Name: openofficeorg-mandriva-menus
Version: 2.0
Release: 1
Group: Office
License: LGPL, SISSL
AutoReqProv: no
#
# FIXME: Limited Edition 2005 contains package mandrakelinux-release,
#        which provides 'mandrake-release'. We should leave 'mandrake-release'
#        here and check for the 'mandriva-release' in the future (next year).
#
Requires: openofficeorg-core01, mandrake-release
Provides: openofficeorg-mandrakelinux-menus
Obsoletes: openofficeorg-mandrakelinux-menus

%define _unpackaged_files_terminate_build 0

%define menuversion %(echo %version|cut -d'.' -f 1-2)
%{?!update_menus:%define update_menus if [ -x /usr/bin/update-menus ]; then /usr/bin/update-menus || true ; fi}
%{?!trigger_clean_menus:%define trigger_clean_menus if [ "$2" = "0" -a -x /usr/bin/update-menus ]; then /usr/bin/update-menus || true ; fi}

%description 
OpenOffice.org desktop integration

%triggerin -- openofficeorg-core01
# create file in /etc that contains the office installation path
cat > /tmp/install.$$ << EOF
while [ "\$TARGET" == "" ]
do
  sleep 2
  TARGET=\`rpm -q --qf '%{INSTALLPREFIX}' openofficeorg-core01 2>&1\` && ln -sf \$TARGET /etc/%PREFIX
  # some rpm versions do not wait for the shared lock
  echo \$TARGET | grep '/var/lib/rpm' && TARGET=""
done
rm -f /tmp/install.$$
EOF

/bin/sh /tmp/install.$$ &

# Update menus
#
# - core01 for base
# - core02 for spadmin (printeradmin)
#
%triggerin -- openofficeorg-core01 openofficeorg-calc openofficeorg-draw openofficeorg-impress openofficeorg-writer openofficeorg-math openofficeorg-core02
%{update_menus}

# Update menus
#
# - core01 for base
# - core02 for spadmin (printeradmin)
#
%triggerpostun -- openofficeorg-core01 openofficeorg-calc openofficeorg-draw openofficeorg-impress openofficeorg-writer openofficeorg-math openofficeorg-core02
%{trigger_clean_menus}

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
application/vnd.sun.xml.base; %PREFIX -view %s
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
%{update_menus}


%install

#
# Mandriva menus fun
#
# For more info, see:
#
#   http://qa.mandriva.com/twiki/bin/view/Main/MenuSystem
#
%define _menudir /usr/lib/menu
mkdir -p $RPM_BUILD_ROOT%{_menudir}
rm -f "$RPM_BUILD_ROOT%{_menudir}/%{name}"

GenerateMenu() {
[ -f "$RPM_BUILD_ROOT%{_menudir}/%{name}" ] || touch $RPM_BUILD_ROOT%{_menudir}/%{name}
mimetypes_item=
[ "$7" != "" ] && mimetypes_item="mimetypes=\"$7\""
cat >> $RPM_BUILD_ROOT%{_menudir}/%{name} << EOF
?package(openofficeorg-$6): needs=x11 section="$2" icon="%ICONPREFIX-$3.png" title="$4" longtitle="$5" command="$1 %U" \
$mimetypes_item kde_opt="InitialPreference=100" startup_notify="true"
EOF
}

#
# FIXME: Office/Database request in the Cooker ML, leave in Spreadsheets
# until new group will be created
#
GenerateMenu "%PREFIX -base" \
	"Applications/Databases" \
	"base" \
	"OpenOffice.org %{menuversion} Base" \
	"OpenOffice.org %{menuversion} Database" \
	"core01" \
	"application/vnd.oasis.opendocument.database,application/vnd.sun.xml.base"

GenerateMenu "%PREFIX -calc" \
	"Office/Spreadsheets" \
	"calc" \
	"OpenOffice.org %{menuversion} Calc" \
	"OpenOffice.org %{menuversion} Spreadsheet" \
	"calc" \
	"application/vnd.oasis.opendocument.spreadsheet,application/vnd.oasis.opendocument.spreadsheet-template,application/vnd.sun.xml.calc,application/vnd.sun.xml.calc.template,application/vnd.stardivision.calc,application/vnd.stardivision.chart,application/msexcel,application/vnd.ms-excel"

GenerateMenu "%PREFIX -draw" \
	"Office/Drawing" \
	"draw" \
	"OpenOffice.org %{menuversion} Draw" \
	"OpenOffice.org %{menuversion} Drawing" \
	"draw" \
	"application/vnd.oasis.opendocument.graphics,application/vnd.oasis.opendocument.graphics-template,application/vnd.sun.xml.draw,application/vnd.sun.xml.draw.template,application/vnd.stardivision.draw"

GenerateMenu "%PREFIX -impress" \
	"Office/Presentations" \
	"impress" \
	"OpenOffice.org %{menuversion} Impress" \
	"OpenOffice.org %{menuversion} Presentation" \
	"impress" \
	"application/vnd.oasis.opendocument.presentation,application/vnd.oasis.opendocument.presentation-template,application/vnd.sun.xml.impress,application/vnd.sun.xml.impress.template,application/vnd.stardivision.impress,application/mspowerpoint"

GenerateMenu "%PREFIX -writer" \
	"Office/Wordprocessors" \
	"writer" \
	"OpenOffice.org %{menuversion} Writer" \
	"OpenOffice.org %{menuversion} Word Processing Component" \
	"writer" \
	"application/vnd.oasis.opendocument.text,application/vnd.oasis.opendocument.text-template,application/vnd.oasis.opendocument.text-web,application/vnd.oasis.opendocument.text-master,application/vnd.sun.xml.writer,application/vnd.sun.xml.writer.template,application/vnd.sun.xml.writer.global,application/vnd.stardivision.writer,application/msword,application/vnd.ms-word,application/x-doc,text/rtf"

GenerateMenu "%PREFIX -math" \
	"Office/Wordprocessors" \
	"math" \
	"OpenOffice.org %{menuversion} Math" \
	"OpenOffice.org %{menuversion} Formula Editor" \
	"math" \
	"application/vnd.oasis.opendocument.formula,application/vnd.sun.xml.math,application/vnd.stardivision.math"

#
# FIXME: Is there a better group than System/Configuration/Printing? I think no ...
#
GenerateMenu "%PREFIX-printeradmin" \
    "System/Configuration/Printing" \
    "printeradmin" \
    "OpenOffice.org %{menuversion} Printeradmin" \
    "OpenOffice.org %{menuversion} Printer Administration" \
	"core02"

%preun
# remove from /etc/mailcap only on de-install
if [ "$1" = 0 ]
then
  # backing all entries pointing to our binary
  sed '/%PREFIX/d' /etc/mailcap 2>/dev/null >> /etc/mailcap.tmp$$

  # and replace the original file
  mv -f /etc/mailcap.tmp$$ /etc/mailcap
fi

%postun
%{update_menus}


%files
%attr(0755,root,root) %verify(not size md5) /usr/bin/%PREFIX
%attr(0755,root,root) /usr/bin/%PREFIX-printeradmin
%defattr(0644, root, root)
%ghost /etc/%PREFIX
%{_menudir}/%{name}
/usr/share/application-registry/%PREFIX.applications
/usr/share/applications/%PREFIX-writer.desktop
/usr/share/applications/%PREFIX-calc.desktop
/usr/share/applications/%PREFIX-draw.desktop
/usr/share/applications/%PREFIX-impress.desktop
/usr/share/applications/%PREFIX-math.desktop
/usr/share/applications/%PREFIX-base.desktop
/usr/share/applications/%PREFIX-printeradmin.desktop
/usr/share/mime-info/%PREFIX.keys
/usr/share/mime-info/%PREFIX.mime
/usr/share/mimelnk/application/%PREFIX-text.desktop
/usr/share/mimelnk/application/%PREFIX-text-template.desktop
/usr/share/mimelnk/application/%PREFIX-spreadsheet.desktop
/usr/share/mimelnk/application/%PREFIX-spreadsheet-template.desktop
/usr/share/mimelnk/application/%PREFIX-drawing.desktop
/usr/share/mimelnk/application/%PREFIX-drawing-template.desktop
/usr/share/mimelnk/application/%PREFIX-presentation.desktop
/usr/share/mimelnk/application/%PREFIX-presentation-template.desktop
/usr/share/mimelnk/application/%PREFIX-master-document.desktop
/usr/share/mimelnk/application/%PREFIX-formula.desktop
/usr/share/mimelnk/application/%PREFIX-oasis-text.desktop
/usr/share/mimelnk/application/%PREFIX-oasis-text-template.desktop
/usr/share/mimelnk/application/%PREFIX-oasis-spreadsheet.desktop
/usr/share/mimelnk/application/%PREFIX-oasis-spreadsheet-template.desktop
/usr/share/mimelnk/application/%PREFIX-oasis-drawing.desktop
/usr/share/mimelnk/application/%PREFIX-oasis-drawing-template.desktop
/usr/share/mimelnk/application/%PREFIX-oasis-presentation.desktop
/usr/share/mimelnk/application/%PREFIX-oasis-presentation-template.desktop
/usr/share/mimelnk/application/%PREFIX-oasis-master-document.desktop
/usr/share/mimelnk/application/%PREFIX-oasis-formula.desktop
/usr/share/mimelnk/application/%PREFIX-oasis-database.desktop
/usr/share/mimelnk/application/%PREFIX-oasis-web-template.desktop
/usr/share/icons/gnome/16x16/apps/%ICONPREFIX-writer.png
/usr/share/icons/gnome/16x16/apps/%ICONPREFIX-calc.png
/usr/share/icons/gnome/16x16/apps/%ICONPREFIX-draw.png
/usr/share/icons/gnome/16x16/apps/%ICONPREFIX-impress.png
/usr/share/icons/gnome/16x16/apps/%ICONPREFIX-math.png
/usr/share/icons/gnome/16x16/apps/%ICONPREFIX-base.png
/usr/share/icons/gnome/16x16/apps/%ICONPREFIX-printeradmin.png
/usr/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-text.png
/usr/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-text-template.png
/usr/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-spreadsheet.png
/usr/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-spreadsheet-template.png
/usr/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-drawing.png
/usr/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-drawing-template.png
/usr/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-presentation.png
/usr/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-presentation-template.png
/usr/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-master-document.png
/usr/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-formula.png
/usr/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-database.png
/usr/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-text.png
/usr/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-text-template.png
/usr/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-spreadsheet.png
/usr/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-spreadsheet-template.png
/usr/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-drawing.png
/usr/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-drawing-template.png
/usr/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-presentation.png
/usr/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-presentation-template.png
/usr/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-master-document.png
/usr/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-formula.png
/usr/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-database.png
/usr/share/icons/gnome/16x16/mimetypes/%ICONPREFIX-oasis-web-template.png
/usr/share/icons/gnome/32x32/apps/%ICONPREFIX-writer.png
/usr/share/icons/gnome/32x32/apps/%ICONPREFIX-calc.png
/usr/share/icons/gnome/32x32/apps/%ICONPREFIX-draw.png
/usr/share/icons/gnome/32x32/apps/%ICONPREFIX-impress.png
/usr/share/icons/gnome/32x32/apps/%ICONPREFIX-math.png
/usr/share/icons/gnome/32x32/apps/%ICONPREFIX-base.png
/usr/share/icons/gnome/32x32/apps/%ICONPREFIX-printeradmin.png
/usr/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-text.png
/usr/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-text-template.png
/usr/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-spreadsheet.png
/usr/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-spreadsheet-template.png
/usr/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-drawing.png
/usr/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-drawing-template.png
/usr/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-presentation.png
/usr/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-presentation-template.png
/usr/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-master-document.png
/usr/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-formula.png
/usr/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-database.png
/usr/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-text.png
/usr/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-text-template.png
/usr/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-spreadsheet.png
/usr/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-spreadsheet-template.png
/usr/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-drawing.png
/usr/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-drawing-template.png
/usr/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-presentation.png
/usr/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-presentation-template.png
/usr/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-master-document.png
/usr/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-formula.png
/usr/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-database.png
/usr/share/icons/gnome/32x32/mimetypes/%ICONPREFIX-oasis-web-template.png
/usr/share/icons/gnome/48x48/apps/%ICONPREFIX-writer.png
/usr/share/icons/gnome/48x48/apps/%ICONPREFIX-calc.png
/usr/share/icons/gnome/48x48/apps/%ICONPREFIX-draw.png
/usr/share/icons/gnome/48x48/apps/%ICONPREFIX-impress.png
/usr/share/icons/gnome/48x48/apps/%ICONPREFIX-math.png
/usr/share/icons/gnome/48x48/apps/%ICONPREFIX-base.png
/usr/share/icons/gnome/48x48/apps/%ICONPREFIX-printeradmin.png
/usr/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-text.png
/usr/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-text-template.png
/usr/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-spreadsheet.png
/usr/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-spreadsheet-template.png
/usr/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-drawing.png
/usr/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-drawing-template.png
/usr/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-presentation.png
/usr/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-presentation-template.png
/usr/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-master-document.png
/usr/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-formula.png
/usr/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-database.png
/usr/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-text.png
/usr/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-text-template.png
/usr/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-spreadsheet.png
/usr/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-spreadsheet-template.png
/usr/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-drawing.png
/usr/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-drawing-template.png
/usr/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-presentation.png
/usr/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-presentation-template.png
/usr/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-master-document.png
/usr/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-formula.png
/usr/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-database.png
/usr/share/icons/gnome/48x48/mimetypes/%ICONPREFIX-oasis-web-template.png
/usr/share/icons/HighContrast/16x16/apps/%ICONPREFIX-writer.png
/usr/share/icons/HighContrast/16x16/apps/%ICONPREFIX-calc.png
/usr/share/icons/HighContrast/16x16/apps/%ICONPREFIX-draw.png
/usr/share/icons/HighContrast/16x16/apps/%ICONPREFIX-impress.png
/usr/share/icons/HighContrast/16x16/apps/%ICONPREFIX-math.png
/usr/share/icons/HighContrast/16x16/apps/%ICONPREFIX-base.png
/usr/share/icons/HighContrast/16x16/apps/%ICONPREFIX-printeradmin.png
/usr/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-text.png
/usr/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-text-template.png
/usr/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-spreadsheet.png
/usr/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-spreadsheet-template.png
/usr/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-drawing.png
/usr/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-drawing-template.png
/usr/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-presentation.png
/usr/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-presentation-template.png
/usr/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-master-document.png
/usr/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-formula.png
/usr/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-database.png
/usr/share/icons/HighContrast/16x16/mimetypes/%ICONPREFIX-oasis-web-template.png
/usr/share/icons/HighContrast/32x32/apps/%ICONPREFIX-writer.png
/usr/share/icons/HighContrast/32x32/apps/%ICONPREFIX-calc.png
/usr/share/icons/HighContrast/32x32/apps/%ICONPREFIX-draw.png
/usr/share/icons/HighContrast/32x32/apps/%ICONPREFIX-impress.png
/usr/share/icons/HighContrast/32x32/apps/%ICONPREFIX-math.png
/usr/share/icons/HighContrast/32x32/apps/%ICONPREFIX-base.png
/usr/share/icons/HighContrast/32x32/apps/%ICONPREFIX-printeradmin.png
/usr/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-text.png
/usr/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-text-template.png
/usr/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-spreadsheet.png
/usr/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-spreadsheet-template.png
/usr/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-drawing.png
/usr/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-drawing-template.png
/usr/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-presentation.png
/usr/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-presentation-template.png
/usr/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-master-document.png
/usr/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-formula.png
/usr/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-database.png
/usr/share/icons/HighContrast/32x32/mimetypes/%ICONPREFIX-oasis-web-template.png
/usr/share/icons/HighContrast/48x48/apps/%ICONPREFIX-writer.png
/usr/share/icons/HighContrast/48x48/apps/%ICONPREFIX-calc.png
/usr/share/icons/HighContrast/48x48/apps/%ICONPREFIX-draw.png
/usr/share/icons/HighContrast/48x48/apps/%ICONPREFIX-impress.png
/usr/share/icons/HighContrast/48x48/apps/%ICONPREFIX-math.png
/usr/share/icons/HighContrast/48x48/apps/%ICONPREFIX-base.png
/usr/share/icons/HighContrast/48x48/apps/%ICONPREFIX-printeradmin.png
/usr/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-text.png
/usr/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-text-template.png
/usr/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-spreadsheet.png
/usr/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-spreadsheet-template.png
/usr/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-drawing.png
/usr/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-drawing-template.png
/usr/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-presentation.png
/usr/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-presentation-template.png
/usr/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-master-document.png
/usr/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-formula.png
/usr/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-database.png
/usr/share/icons/HighContrast/48x48/mimetypes/%ICONPREFIX-oasis-web-template.png
/usr/share/icons/hicolor/16x16/apps/%ICONPREFIX-writer.png
/usr/share/icons/hicolor/16x16/apps/%ICONPREFIX-calc.png
/usr/share/icons/hicolor/16x16/apps/%ICONPREFIX-draw.png
/usr/share/icons/hicolor/16x16/apps/%ICONPREFIX-impress.png
/usr/share/icons/hicolor/16x16/apps/%ICONPREFIX-math.png
/usr/share/icons/hicolor/16x16/apps/%ICONPREFIX-base.png
/usr/share/icons/hicolor/16x16/apps/%ICONPREFIX-printeradmin.png
/usr/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-text.png
/usr/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-text-template.png
/usr/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-spreadsheet.png
/usr/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-spreadsheet-template.png
/usr/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-drawing.png
/usr/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-drawing-template.png
/usr/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-presentation.png
/usr/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-presentation-template.png
/usr/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-master-document.png
/usr/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-formula.png
/usr/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-database.png
/usr/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-text.png
/usr/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-text-template.png
/usr/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-spreadsheet.png
/usr/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-spreadsheet-template.png
/usr/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-drawing.png
/usr/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-drawing-template.png
/usr/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-presentation.png
/usr/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-presentation-template.png
/usr/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-master-document.png
/usr/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-formula.png
/usr/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-database.png
/usr/share/icons/hicolor/16x16/mimetypes/%ICONPREFIX-oasis-web-template.png
/usr/share/icons/hicolor/32x32/apps/%ICONPREFIX-writer.png
/usr/share/icons/hicolor/32x32/apps/%ICONPREFIX-calc.png
/usr/share/icons/hicolor/32x32/apps/%ICONPREFIX-draw.png
/usr/share/icons/hicolor/32x32/apps/%ICONPREFIX-impress.png
/usr/share/icons/hicolor/32x32/apps/%ICONPREFIX-math.png
/usr/share/icons/hicolor/32x32/apps/%ICONPREFIX-base.png
/usr/share/icons/hicolor/32x32/apps/%ICONPREFIX-printeradmin.png
/usr/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-text.png
/usr/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-text-template.png
/usr/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-spreadsheet.png
/usr/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-spreadsheet-template.png
/usr/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-drawing.png
/usr/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-drawing-template.png
/usr/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-presentation.png
/usr/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-presentation-template.png
/usr/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-master-document.png
/usr/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-formula.png
/usr/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-database.png
/usr/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-text.png
/usr/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-text-template.png
/usr/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-spreadsheet.png
/usr/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-spreadsheet-template.png
/usr/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-drawing.png
/usr/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-drawing-template.png
/usr/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-presentation.png
/usr/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-presentation-template.png
/usr/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-master-document.png
/usr/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-formula.png
/usr/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-database.png
/usr/share/icons/hicolor/32x32/mimetypes/%ICONPREFIX-oasis-web-template.png
/usr/share/icons/hicolor/48x48/apps/%ICONPREFIX-writer.png
/usr/share/icons/hicolor/48x48/apps/%ICONPREFIX-calc.png
/usr/share/icons/hicolor/48x48/apps/%ICONPREFIX-draw.png
/usr/share/icons/hicolor/48x48/apps/%ICONPREFIX-impress.png
/usr/share/icons/hicolor/48x48/apps/%ICONPREFIX-math.png
/usr/share/icons/hicolor/48x48/apps/%ICONPREFIX-base.png
/usr/share/icons/hicolor/48x48/apps/%ICONPREFIX-printeradmin.png
/usr/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-text.png
/usr/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-text-template.png
/usr/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-spreadsheet.png
/usr/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-spreadsheet-template.png
/usr/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-drawing.png
/usr/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-drawing-template.png
/usr/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-presentation.png
/usr/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-presentation-template.png
/usr/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-master-document.png
/usr/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-formula.png
/usr/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-database.png
/usr/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-text.png
/usr/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-text-template.png
/usr/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-spreadsheet.png
/usr/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-spreadsheet-template.png
/usr/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-drawing.png
/usr/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-drawing-template.png
/usr/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-presentation.png
/usr/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-presentation-template.png
/usr/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-master-document.png
/usr/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-formula.png
/usr/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-database.png
/usr/share/icons/hicolor/48x48/mimetypes/%ICONPREFIX-oasis-web-template.png
/usr/share/icons/locolor/16x16/apps/%ICONPREFIX-writer.png
/usr/share/icons/locolor/16x16/apps/%ICONPREFIX-calc.png
/usr/share/icons/locolor/16x16/apps/%ICONPREFIX-draw.png
/usr/share/icons/locolor/16x16/apps/%ICONPREFIX-impress.png
/usr/share/icons/locolor/16x16/apps/%ICONPREFIX-math.png
/usr/share/icons/locolor/16x16/apps/%ICONPREFIX-base.png
/usr/share/icons/locolor/16x16/apps/%ICONPREFIX-printeradmin.png
/usr/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-text.png
/usr/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-text-template.png
/usr/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-spreadsheet.png
/usr/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-spreadsheet-template.png
/usr/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-drawing.png
/usr/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-drawing-template.png
/usr/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-presentation.png
/usr/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-presentation-template.png
/usr/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-master-document.png
/usr/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-formula.png
/usr/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-database.png
/usr/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-text.png
/usr/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-text-template.png
/usr/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-spreadsheet.png
/usr/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-spreadsheet-template.png
/usr/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-drawing.png
/usr/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-drawing-template.png
/usr/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-presentation.png
/usr/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-presentation-template.png
/usr/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-master-document.png
/usr/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-formula.png
/usr/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-database.png
/usr/share/icons/locolor/16x16/mimetypes/%ICONPREFIX-oasis-web-template.png
/usr/share/icons/locolor/32x32/apps/%ICONPREFIX-writer.png
/usr/share/icons/locolor/32x32/apps/%ICONPREFIX-calc.png
/usr/share/icons/locolor/32x32/apps/%ICONPREFIX-draw.png
/usr/share/icons/locolor/32x32/apps/%ICONPREFIX-impress.png
/usr/share/icons/locolor/32x32/apps/%ICONPREFIX-math.png
/usr/share/icons/locolor/32x32/apps/%ICONPREFIX-base.png
/usr/share/icons/locolor/32x32/apps/%ICONPREFIX-printeradmin.png
/usr/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-text.png
/usr/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-text-template.png
/usr/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-spreadsheet.png
/usr/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-spreadsheet-template.png
/usr/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-drawing.png
/usr/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-drawing-template.png
/usr/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-presentation.png
/usr/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-presentation-template.png
/usr/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-master-document.png
/usr/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-formula.png
/usr/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-database.png
/usr/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-text.png
/usr/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-text-template.png
/usr/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-spreadsheet.png
/usr/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-spreadsheet-template.png
/usr/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-drawing.png
/usr/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-drawing-template.png
/usr/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-presentation.png
/usr/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-presentation-template.png
/usr/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-master-document.png
/usr/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-formula.png
/usr/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-database.png
/usr/share/icons/locolor/32x32/mimetypes/%ICONPREFIX-oasis-web-template.png
