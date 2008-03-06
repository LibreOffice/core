# version and release passed by command-line
Version: %version
Release: %release
Summary: %productname desktop integration
Name: %pkgprefix-mandriva-menus
Group: Office
License: LGPL
AutoReqProv: no
BuildArch: noarch
#
# FIXME: Limited Edition 2005 contains package mandrakelinux-release,
#        which provides 'mandrake-release'. We should leave 'mandrake-release'
#        here and check for the 'mandriva-release' in the future (next year).
#
Requires: %pkgprefix-core01, mandrake-release
Provides: openoffice.org-desktop-integration
Obsoletes: openofficeorg-mandrakelinux-menus, openofficeorg-mandriva-menus

%define _unpackaged_files_terminate_build 0

%define menuversion %(echo %version|cut -d'.' -f 1-2)
%{?!update_menus:%define update_menus if [ -x /usr/bin/update-menus ]; then /usr/bin/update-menus || true ; fi}
%{?!trigger_clean_menus:%define trigger_clean_menus if [ "$2" = "0" -a -x /usr/bin/update-menus ]; then /usr/bin/update-menus || true ; fi}

%description 
%productname desktop integration

#include<symlink_triggers>

# Update menus
#
# - core01 for base
# - core02 for spadmin (printeradmin)
#
%triggerin -- %pkgprefix-core01 %pkgprefix-calc %pkgprefix-draw %pkgprefix-impress %pkgprefix-writer %pkgprefix-math %pkgprefix-core02
%{update_menus}

# Update menus
#
# - core01 for base
# - core02 for spadmin (printeradmin)
#
%triggerpostun -- %pkgprefix-core01 %pkgprefix-calc %pkgprefix-draw %pkgprefix-impress %pkgprefix-writer %pkgprefix-math %pkgprefix-core02
%{trigger_clean_menus}

%post

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
application/vnd.openofficeorg.extension oxt
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
application/vnd.openofficeorg.extension; unopkg gui %s
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
%{update_menus}


%install
rm -rf $RPM_BUILD_ROOT/*

# hack/workaround to make SuSE's brp-symlink-script happy. It wants the targets of all links
# to be present on the build-system/the buildroot. But the point is that we generate stale
# links intentionally (until we find a better solution) #46226
export NO_BRP_STALE_LINK_ERROR=yes

# enable relocation in create_tree.sh
mkdir -p $RPM_BUILD_ROOT/etc
touch $RPM_BUILD_ROOT/etc/%unixfilename

export DESTDIR=$RPM_BUILD_ROOT
export KDEMAINDIR=/usr
export GNOMEDIR=/usr

./create_tree.sh

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
?package(%pkgprefix-$6): needs=x11 section="$2" icon="%iconprefix-$3.png" title="$4" longtitle="$5" command="$1" \
$mimetypes_item kde_opt="InitialPreference=100" startup_notify="true"
EOF
}

#
# FIXME: Office/Database request in the Cooker ML, leave in Spreadsheets
# until new group will be created
#
GenerateMenu "%unixfilename -base" \
	"More Applications/Databases" \
	"base" \
	"%productname %{menuversion} Base" \
	"%productname %{menuversion} Database" \
	"core01" \
	"application/vnd.oasis.opendocument.database,application/vnd.sun.xml.base"

GenerateMenu "%unixfilename -calc" \
	"Office/Spreadsheets" \
	"calc" \
	"%productname %{menuversion} Calc" \
	"%productname %{menuversion} Spreadsheet" \
	"calc" \
	"application/vnd.oasis.opendocument.spreadsheet,application/vnd.oasis.opendocument.spreadsheet-template,application/vnd.sun.xml.calc,application/vnd.sun.xml.calc.template,application/vnd.stardivision.calc,application/vnd.stardivision.chart,application/msexcel,application/vnd.ms-excel"

GenerateMenu "%unixfilename -draw" \
	"Office/Drawing" \
	"draw" \
	"%productname %{menuversion} Draw" \
	"%productname %{menuversion} Drawing" \
	"draw" \
	"application/vnd.oasis.opendocument.graphics,application/vnd.oasis.opendocument.graphics-template,application/vnd.sun.xml.draw,application/vnd.sun.xml.draw.template,application/vnd.stardivision.draw"

GenerateMenu "%unixfilename -impress" \
	"Office/Presentations" \
	"impress" \
	"%productname %{menuversion} Impress" \
	"%productname %{menuversion} Presentation" \
	"impress" \
	"application/vnd.oasis.opendocument.presentation,application/vnd.oasis.opendocument.presentation-template,application/vnd.sun.xml.impress,application/vnd.sun.xml.impress.template,application/vnd.stardivision.impress,application/mspowerpoint"

GenerateMenu "%unixfilename -writer" \
	"Office/Wordprocessors" \
	"writer" \
	"%productname %{menuversion} Writer" \
	"%productname %{menuversion} Word Processing Component" \
	"writer" \
	"application/vnd.oasis.opendocument.text,application/vnd.oasis.opendocument.text-template,application/vnd.oasis.opendocument.text-web,application/vnd.oasis.opendocument.text-master,application/vnd.sun.xml.writer,application/vnd.sun.xml.writer.template,application/vnd.sun.xml.writer.global,application/vnd.stardivision.writer,application/msword,application/vnd.ms-word,application/x-doc,application/rtf"

GenerateMenu "%unixfilename -math" \
	"Office/Wordprocessors" \
	"math" \
	"%productname %{menuversion} Math" \
	"%productname %{menuversion} Formula Editor" \
	"math" \
	"application/vnd.oasis.opendocument.formula,application/vnd.sun.xml.math,application/vnd.stardivision.math"

#
# FIXME: Is there a better group than System/Configuration/Printing? I think no ...
#
GenerateMenu "%unixfilename-printeradmin" \
    "System/Configuration/Printing" \
    "printeradmin" \
    "%productname %{menuversion} Printeradmin" \
    "%productname %{menuversion} Printer Administration" \
	"core02"

%clean
rm -rf $RPM_BUILD_ROOT/*

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
%{update_menus}


%files
%attr(0755,root,root) /usr/bin/soffice
%attr(0755,root,root) /usr/bin/unopkg
%attr(0755,root,root) %verify(not size md5) /usr/bin/%unixfilename
%attr(0755,root,root) /usr/bin/%unixfilename-printeradmin
%defattr(0644, root, root)
%ghost /etc/%unixfilename
%{_menudir}/%{name}
/usr/share/application-registry/*.applications
/usr/share/applications/%unixfilename-writer.desktop
/usr/share/applications/%unixfilename-calc.desktop
/usr/share/applications/%unixfilename-draw.desktop
/usr/share/applications/%unixfilename-impress.desktop
/usr/share/applications/%unixfilename-math.desktop
/usr/share/applications/%unixfilename-base.desktop
/usr/share/applications/%unixfilename-printeradmin.desktop
/usr/share/applications/%unixfilename-extensionmgr.desktop
/usr/share/mime-info/*.keys
/usr/share/mime-info/*.mime
/usr/share/mimelnk/application/*.desktop
/usr/share/icons/gnome/*/apps/*png
/usr/share/icons/gnome/*/mimetypes/*png
/usr/share/icons/hicolor/*/apps/*png
/usr/share/icons/hicolor/*/mimetypes/*png
/usr/share/icons/locolor/*/apps/*png
/usr/share/icons/locolor/*/mimetypes/*png

