# version and release passed by command-line
Version: %version
Release: %release
Summary: OpenOffice.org desktop integration
Name: openofficeorg-freedesktop-menus
BuildRoot: %_tmppath/%name-%version-build%unique
#BuildRequires: ed
#BuildRequires: perl
Group: Office
License: LGPL / SISSL
BuildArch: noarch
AutoReqProv: no

%description
OpenOffice.org desktop integration

%prep
# create & change to rpm-Build-Dir
%setup -c -T -n %name-%version-build%unique

# let's copy everything we need to the builddir.
# basedir is passed on the commandline
cp -a %source/usr .

%build
# freedesktop-based desktop-environments don't need/use this.
rm -rf usr/share/application-registry
rm -rf usr/share/applications.flag
rm -rf usr/share/mime-info
rm -rf usr/share/mimelnk
rm -rf usr/share/applnk-redhat

# Get rid of "X-Red-Hat-Base" category-keyword
#for i in usr/share/applications/*; do
#ed "$i" <<EOF
#,s#X-Red-Hat-Base;##
#wq
#EOF
#done

## create shared-mime-info file 
mkdir -p usr/share/mime/packages
perl %basedir/create_mime_xml.pl > usr/share/mime/packages/openoffice.org.xml

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/etc
# copy everything to the buildroot
cp -a * $RPM_BUILD_ROOT
# dummy for %ghost
touch $RPM_BUILD_ROOT/etc/%unixfilename

%clean
cd ..
rm -rf $RPM_BUILD_ROOT $RPM_BUILD_DIR/%name-%version-build%unique

%triggerin -- openofficeorg-core01, openofficeorg-writer, openofficeorg-calc, openofficeorg-draw, openofficeorg-impress, openofficeorg-math
# create file in /etc that contains the office installation path
cat > /tmp/install.$$ << EOF
sleep 2
coreprefix=\`rpm -q --qf '%{INSTALLPREFIX}' openofficeorg-core01\`
if [ \$coreprefix == "(none)" ]; then 
	coreprefix="/opt/openoffice.org%version"
fi
ln -sf \$coreprefix /etc/%unixfilename

# no need to run it when updating, since %postun of the old package is run
# afterwards
if [ "$2" = "1" ] ; then  # first install
  if (which update-desktop-database); then
    update-desktop-database /usr/share/applications
  fi
fi

rm -f /tmp/install.$$
EOF

/bin/sh /tmp/install.$$ &

%post 
# no need to run it when updating, since %postun of the old package is run
# afterwards
if [ "$1" = "1" ] ; then  # first install
  if (which update-mime-database); then
    update-mime-database /usr/share/mime
  fi
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
# run always - both when upgrading as well as when erasing the package
if (which update-desktop-database); then
  update-desktop-database /usr/share/applications
fi
if (which update-mime-database); then
  update-mime-database /usr/share/mime
fi

%files 
%defattr(-, root, root)
%ghost /etc/%unixfilename
%attr(0755, root, root) /usr/bin/*
/usr/share/applications/*desktop
/usr/share/icons/gnome/*/apps/*png
/usr/share/icons/gnome/*/mimetypes/*png
/usr/share/icons/hicolor/*/apps/*png
/usr/share/icons/hicolor/*/mimetypes/*png
/usr/share/icons/HighContrast/*/apps/*.png
/usr/share/icons/HighContrast/*/mimetypes/*.png
/usr/share/icons/locolor/*/apps/*png
/usr/share/icons/locolor/*/mimetypes/*png
/usr/share/mime/packages/*
