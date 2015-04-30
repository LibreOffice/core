#!/bin/sh -e

DIR_IN=$1
DIR_OUT=$2
VER_APPEND=$3
DIR_WORK=$4
LIBLDAP_PACKAGE="libldap-2.4-2"
LIBLDAP_FILE="/usr/lib/libldap-2.4.so.2"
LO_BASE_VERSION="4.1"
LO_BASE_PATH=opt/libreoffice${LO_BASE_VERSION}

LO_LIBLDAP="${LO_BASE_PATH}/program/libldap50.so"
OPENSYMBOL_FONT="opens___.ttf"
OPENSYMBOL_PACKAGE="ttf-opensymbol-lhm"
OPENSYMBOL_PATH="${LO_BASE_PATH}/share/fonts/truetype/${OPENSYMBOL_FONT}"
LO_PATHS_XCU="${LO_BASE_PATH}/share/registry/data/org/libreoffice/Office/Paths.xcu"
LO_MAIN_XCD="${LO_BASE_PATH}/share/registry/main.xcd"
ICONS_HICOLOR_PATH="usr/share/icons/hicolor"

function usage {
	echo "    $1"
	echo
	echo "Usage: $0 [scan dir] [out dir] [append string]"
	echo
	echo "    Scan and output directory should not contain each other"
	exit $2
}

function copy_packages {

  OUTFOLDER=$DIR_WORK
  VERSION=$(date +"%y%m%d")
  cd ..
  echo "Kopiere Dateien nach $OUTFOLDER"

  mkdir -p $OUTFOLDER || false
  cd $OUTFOLDER
  pwd

  cp $WORKSPACE/workdir/unxlngi6.pro/installation/LibreOffice/deb/install/LibreOffice_4.1.6.*.?_Linux_x86_deb_download/LibreOffice_4.1.6.*.?_Linux_x86_deb.tar.gz .

  # Die Sprachpakete
  cp $WORKSPACE/workdir/unxlngi6.pro/installation/LibreOffice_languagepack/deb/install/LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_en-US_download/LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_en-US.tar.gz .
  cp $WORKSPACE/workdir/unxlngi6.pro/installation/LibreOffice_languagepack/deb/install/LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_de_download/LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_de.tar.gz .
  cp $WORKSPACE/workdir/unxlngi6.pro/installation/LibreOffice_languagepack/deb/install/LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_fr_download/LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_fr.tar.gz .
  cp $WORKSPACE/workdir/unxlngi6.pro/installation/LibreOffice_languagepack/deb/install/LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_es_download/LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_es.tar.gz .
  cp $WORKSPACE/workdir/unxlngi6.pro/installation/LibreOffice_languagepack/deb/install/LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_pt_download/LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_pt.tar.gz .
  cp $WORKSPACE/workdir/unxlngi6.pro/installation/LibreOffice_languagepack/deb/install/LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_it_download/LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_it.tar.gz .

  # Die Hilfepakete
  cp $WORKSPACE/workdir/unxlngi6.pro/installation/LibreOffice_helppack/deb/install/LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_fr_download/LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_fr.tar.gz .
  cp $WORKSPACE/workdir/unxlngi6.pro/installation/LibreOffice_helppack/deb/install/LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_it_download/LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_it.tar.gz .
  cp $WORKSPACE/workdir/unxlngi6.pro/installation/LibreOffice_helppack/deb/install/LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_de_download/LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_de.tar.gz .
  cp $WORKSPACE/workdir/unxlngi6.pro/installation/LibreOffice_helppack/deb/install/LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_es_download/LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_es.tar.gz .
  cp $WORKSPACE/workdir/unxlngi6.pro/installation/LibreOffice_helppack/deb/install/LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_en-US_download/LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_en-US.tar.gz .
  cp $WORKSPACE/workdir/unxlngi6.pro/installation/LibreOffice_helppack/deb/install/LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_pt_download/LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_pt.tar.gz .

  # Das SDK
  cp $WORKSPACE/workdir/unxlngi6.pro/installation/LibreOffice_SDK/deb/install/LibreOffice_4.1.6.*.?_Linux_x86_deb_sdk_download/LibreOffice_4.1.6.*.?_Linux_x86_deb_sdk.tar.gz .

  tar -xvzf LibreOffice_4.1.6.*.?_Linux_x86_deb.tar.gz
  tar -xvzf LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_de.tar.gz
  tar -xvzf LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_en-US.tar.gz
  tar -xvzf LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_es.tar.gz
  tar -xvzf LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_fr.tar.gz
  tar -xvzf LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_it.tar.gz
  tar -xvzf LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_pt.tar.gz
  tar -xvzf LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_fr.tar.gz
  tar -xvzf LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_it.tar.gz
  tar -xvzf LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_de.tar.gz
  tar -xvzf LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_es.tar.gz
  tar -xvzf LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_en-US.tar.gz
  tar -xvzf LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_pt.tar.gz
  tar -xvzf LibreOffice_4.1.6.*.?_Linux_x86_deb_sdk.tar.gz
  rm LibreOffice*.tar.gz

  rm -rf release
  mkdir release
  mkdir release/sdk
  mkdir release/main

  mv LibreOffice_4.1.6.*.?_Linux_x86_deb/DEBS/*.deb release/main
  mv LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_de/DEBS/*.deb release/main
  mv LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_en-US/DEBS/*.deb release/main
  mv LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_es/DEBS/*.deb release/main
  mv LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_fr/DEBS/*.deb release/main
  mv LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_it/DEBS/*.deb release/main
  mv LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_pt/DEBS/*.deb release/main
  mv LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_fr/DEBS/*.deb release/main
  mv LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_it/DEBS/*.deb release/main
  mv LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_de/DEBS/*.deb release/main
  mv LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_es/DEBS/*.deb release/main
  mv LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_pt/DEBS/*.deb release/main
  mv LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_en-US/DEBS/*.deb release/main
  mv LibreOffice_4.1.6.*.?_Linux_x86_deb_sdk/DEBS/*.deb release/sdk

  rm -rf LibreOffice_4.1.6.*.?_Linux_x86_deb
  rm -rf LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_de
  rm -rf LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_en-US
  rm -rf LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_es
  rm -rf LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_fr
  rm -rf LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_it
  rm -rf LibreOffice_4.1.6.*.?_Linux_x86_deb_langpack_pt
  rm -rf LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_de
  rm -rf LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_fr
  rm -rf LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_es
  rm -rf LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_it
  rm -rf LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_pt
  rm -rf LibreOffice_4.1.6.*.?_Linux_x86_deb_helppack_en-US
  rm -rf LibreOffice_4.1.6.*.?_Linux_x86_deb_sdk

  rm -rf patched

}

#copy_packages

if [ "x" = "x$DIR_IN" ]; then
	usage "Scan directory not supplied" 1
fi

if [ "x" = "x$(which fakeroot)" ]; then
	usage "This script needs fakeroot" 1
fi

if [ ! -d $DIR_IN ]; then
	usage "Scan directory not existing or no directory" 2
fi

if [ "x" = "x$DIR_OUT" ]; then
	usage "Output directory not supplied" 3
fi

if [ -d "$DIR_OUT" ]; then
	usage "Output directory already existing - please remove" 4
fi

if [ "x" = "x$VER_APPEND" ]; then
	usage "Version append string not given" 5
fi

DEBVER=$(ls "$DIR_IN"/main/libreoffice*base*.deb | awk -F '-' '{ print $3; }')
if [ "x" = "x${DEBVER}" ]; then
	echo "Couldn't extract debian version from .deb file"
	exit 1
fi

if [ -d "package-data" ]; then
	rm -rf "package-data"
fi

mkdir -p package-data
mkdir $DIR_OUT

function patch_libldap2 {
	if [ -e "package-data/${LO_LIBLDAP}" ]; then
		echo "Symlinking $(basename ${LO_LIBLDAP}) and adding libldap-2.4-2 depends"
		rm -f "package-data/${LO_LIBLDAP}"
		fakeroot ln -s ${LIBLDAP_FILE} "package-data/${LO_LIBLDAP}"
		fakeroot sed -i -e "s/Depends: .*/\0, ${LIBLDAP_PACKAGE}/" \
			package-data/DEBIAN/control
	fi
}

function patch_sdk_exec {
	if [ -d "package-data/${LO_BASE_PATH}/sdk/bin" ]; then
		# Rechte für Programme in SDK
		echo "Changing permissions in sdk/bin"
		fakeroot chmod a+x package-data/${LO_BASE_PATH}/sdk/bin/*
	fi
}

function patch_opensymbol_lhm {
	if [ -f "package-data/${OPENSYMBOL_PATH}" ]; then
		echo "Symlinking 'OpenSymbol TTF font' and adding ${OPENSYMBOL_PACKAGE} depends"
		rm -f "package-data/${OPENSYMBOL_PATH}"
		fakeroot sed -i -e "s/Depends: .*/\0, ${OPENSYMBOL_PACKAGE}/" \
			package-data/DEBIAN/control
		ln -s "/usr/share/fonts/truetype/${OPENSYMBOL_PACKAGE#ttf-}/${OPENSYMBOL_FONT}" \
			"package-data/${OPENSYMBOL_PATH}"
	fi
}

function patch_tmp_path {
	if [ -f "package-data/${LO_MAIN_XCD}" ]; then
		echo "patching temp path ..."
		sed -i -e 's|<node oor:name="Path"><node oor:name="Current"><prop oor:name="Temp" oor:type="xs:string"><value>$(temp)</value></prop></node></node>|<node oor:name="Path"><node oor:name="Current"><prop oor:name="Temp" oor:type="xs:string"><value>file:///var/tmp/</value></prop></node></node>|' package-data/${LO_MAIN_XCD}
		sed -i -e 's|<node oor:name="Path"><node oor:name="Current"><prop oor:name="Temp" oor:type="xs:string"><value>$(temp)</value></prop></node></node>|<node oor:name="Path"><node oor:name="Current"><prop oor:name="Temp" oor:type="xs:string"><value>file:///var/tmp/</value></prop></node></node>|' package-data/${LO_MAIN_XCD}
	fi
}

function patch_template_path {
        if [ -f "package-data/${LO_MAIN_XCD}" ]; then
                echo "patching Template path ..."
                sed -i -e 's|<node oor:name="Template" oor:op="fuse" oor:mandatory="true"><node oor:name="InternalPaths"><node oor:name="$(insturl)/share/template/$(vlang)" oor:op="fuse"/></node><prop oor:name="WritePath"><value>$(home)</value></prop></node>|<node oor:name="Template" oor:op="fuse" oor:mandatory="true"><node oor:name="InternalPaths"><node oor:name="$(insturl)/share/template/$(vlang)" oor:op="fuse"/></node><prop oor:name="WritePath"><value>$(insturl)/share/template/common</value></prop></node>|' package-data/${LO_MAIN_XCD}
        fi
}

function patch_paths_xcu {
	if [ -f "package-data/${LO_PATHS_XCU}" ]; then
		echo "Exchanging Paths.xcu"
		cat > "package-data/${LO_PATHS_XCU}" <<EOF
<?xml version='1.0' encoding='UTF-8'?>
<!--***********************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************ -->
<!DOCTYPE oor:component-data SYSTEM "../../../../component-update.dtd">
<oor:component-data oor:name="Paths" oor:package="org.openoffice.Office" xmlns:install="http://openoffice.org/2004/installation" xmlns:oor="http://openoffice.org/2001/registry" xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">

	<node oor:name="Paths">

		<node oor:name="Addin" oor:op="fuse" oor:mandatory="true">
			<node oor:name="InternalPaths">
			      <node oor:name="\$(progpath)/addin" oor:op="fuse" />
			</node>
		</node>

		<node oor:name="AutoCorrect" oor:op="fuse" oor:mandatory="true">
			<node oor:name="InternalPaths">
			      <node oor:name="\$(insturl)/share/autocorr" oor:op="fuse" />
			</node>
			<prop oor:name="WritePath">
			      <value>\$(userurl)/autocorr</value>
			</prop>
		</node>

		<node oor:name="AutoText" oor:op="fuse" oor:mandatory="true">
			<node oor:name="InternalPaths">
			      <node oor:name="\$(insturl)/share/autotext/\$(vlang)" oor:op="fuse" />
			</node>
			<prop oor:name="WritePath">
			      <value>\$(userurl)/autotext</value>
			</prop>
		</node>

		<node oor:name="Backup" oor:op="fuse" oor:mandatory="true">
			<prop oor:name="IsSinglePath" oor:finalized="true">
				<value>true</value>
			</prop>
			<node oor:name="InternalPaths" oor:finalized="true" />
			<prop oor:name="UserPaths" oor:finalized="true" />
			<prop oor:name="WritePath">
				<value>\$(userurl)/backup</value>
			</prop>
		</node>

		<node oor:name="Basic" oor:op="fuse" oor:mandatory="true">
			<node oor:name="InternalPaths">
			      <node oor:name="\$(insturl)/share/basic" oor:op="fuse" />
			</node>
			<prop oor:name="WritePath">
			      <value>\$(userurl)/basic</value>
			</prop>
		</node>

		<node oor:name="Bitmap" oor:op="fuse" oor:mandatory="true">
			<node oor:name="InternalPaths">
			      <node oor:name="\$(insturl)/share/config/symbol" oor:op="fuse" />
			</node>
		</node>

		<node oor:name="Config" oor:op="fuse" oor:mandatory="true">
			<node oor:name="InternalPaths">
			      <node oor:name="\$(insturl)/share/config" oor:op="fuse" />
			</node>
		</node>

		<node oor:name="Favorite" oor:op="fuse" oor:mandatory="true">
			<prop oor:name="WritePath">
			      <value>\$(userurl)/config/folders</value>
			</prop>
		</node>

		<node oor:name="Filter" oor:op="fuse" oor:mandatory="true">
			<node oor:name="InternalPaths">
			      <node oor:name="\$(progpath)/filter" oor:op="fuse" />
			</node>
		</node>

		<node oor:name="Gallery" oor:op="fuse" oor:mandatory="true">
			<node oor:name="InternalPaths">
				<node oor:name="\$(insturl)/share/gallery" oor:op="fuse" />
				<node oor:name="file:///usr/share/openoffice.org/gallery" oor:op="fuse" />
			</node>
			<prop oor:name="WritePath">
			      <value>\$(userurl)/gallery</value>
			</prop>
		</node>

		<node oor:name="Graphic" oor:op="fuse" oor:mandatory="true">
			<prop oor:name="IsSinglePath" oor:finalized="true">
				<value>true</value>
			</prop>
			<node oor:name="InternalPaths" oor:finalized="true" />
			<prop oor:name="UserPaths" oor:finalized="true" />
			<prop oor:name="WritePath">
				<value>\$(work)</value>
			</prop>
		</node>

		<node oor:name="Help" oor:op="fuse" oor:mandatory="true">
			<node oor:name="InternalPaths">
				<node oor:name="\$(instpath)/help" oor:op="fuse" />
			</node>
		</node>

		<node oor:name="Linguistic" oor:op="fuse" oor:mandatory="true">
			<node oor:name="InternalPaths">
			<node oor:name="\$(insturl)/share/dict" oor:op="fuse"/>
			<node oor:name="\$(insturl)/share/dict/ooo" oor:op="fuse"/>
		</node>
		<prop oor:name="UserPaths">
			<value>\$(userurl)/wordbook</value>
		</prop>
	</node>

	<node oor:name="Dictionary" oor:op="fuse" oor:mandatory="true">
		<node oor:name="InternalPaths">
			<node oor:name="\$(insturl)/share/wordbook/\$(vlang)" oor:op="fuse"/>
		</node>
		<prop oor:name="WritePath">
			<value>\$(userurl)/wordbook</value>
		</prop>
	</node>

	<node oor:name="Module" oor:op="fuse" oor:mandatory="true">
		<node oor:name="InternalPaths">
			<node oor:name="\$(progpath)" oor:op="fuse" />
		</node>
	</node>

	<node oor:name="Palette" oor:op="fuse" oor:mandatory="true">
		<prop oor:name="WritePath">
			<value>\$(userurl)/config</value>
		</prop>
	</node>

	<node oor:name="Plugin" oor:op="fuse" oor:mandatory="true">
		<node oor:name="InternalPaths">
			<node oor:name="\$(progpath)/plugin" oor:op="fuse" />
		</node>
	</node>

	<node oor:name="Fingerprint" oor:op="fuse" oor:mandatory="true">
		<node oor:name="InternalPaths">
			<node oor:name="\$(insturl)/share/fingerprint" oor:op="fuse" />
		</node>
	</node>

	<!-- deprecated ! -->
	<node oor:name="Storage" oor:op="fuse" oor:mandatory="true">
		<prop oor:name="WritePath">
			<value>\$(userpath)/store</value>
		</prop>
	</node>

	<node oor:name="Temp" oor:op="fuse" oor:mandatory="true">
		<prop oor:name="IsSinglePath" oor:finalized="true">
			<value>true</value>
		</prop>
		<node oor:name="InternalPaths" oor:finalized="true"/>
		<prop oor:name="UserPaths" oor:finalized="true"/>
		<prop oor:name="WritePath">
			<value install:module="macosx">\$(userurl)/temp</value>
			<value install:module="unxwnt">\$(temp)</value>
		</prop>
	</node>

	<node oor:name="Template" oor:op="fuse" oor:mandatory="true">
		<node oor:name="InternalPaths">
			<node oor:name="\$(insturl)/share/template/\$(vlang)" oor:op="fuse" />
		</node>
		<prop oor:name="WritePath">
			<value>\$(userurl)/template</value>
		</prop>
	</node>

	<node oor:name="UIConfig" oor:op="fuse" oor:mandatory="true">
		<node oor:name="InternalPaths">
			<node oor:name="\$(insturl)/share/config" oor:op="fuse" />
		</node>
	</node>

	<node oor:name="UserConfig" oor:op="fuse" oor:mandatory="true">
		<prop oor:name="IsSinglePath" oor:finalized="true">
			<value>true</value>
		</prop>
		<node oor:name="InternalPaths" oor:finalized="true" />
			<prop oor:name="UserPaths" oor:finalized="true" />
			<prop oor:name="WritePath">
				<value>\$(userurl)/config</value>
			</prop>
		</node>

		<node oor:name="Work" oor:op="fuse" oor:mandatory="true">
			<prop oor:name="IsSinglePath" oor:finalized="true">
				<value>true</value>
			</prop>
			<node oor:name="InternalPaths" oor:finalized="true" />
			<prop oor:name="UserPaths" oor:finalized="true" />
			<prop oor:name="WritePath">
				<value>\$(work)</value>
			</prop>
		</node>
	</node>

</oor:component-data>
EOF
	fi
}

function patch_icon_symlinks {
	# libreoffice-*.png instead of libreoffice$VERSION-*.png is needed to correctly
	# display the window icon (TRAC: #12093)
	for filename in package-data/${ICONS_HICOLOR_PATH}/*/apps/libreoffice${LO_BASE_VERSION}-*.png; do
		if [ -f "$filename" ]; then
			ln -s $(basename $filename) $(dirname $filename)/$(basename $filename | sed 's/[0-9]\.[0-9]-/-/')
		fi
	done
}

#
# $1 = Package
# $2 = Source package
#
function patch_deb {
	for DEB in $DEBFILES; do
		eval "NEW_DIR=${DIR_OUT}/$(dirname ${DEB#${DIR_IN}})"
		BASE=$(basename $DEB)
		NEW_BASE=$(echo $BASE | sed -e "s/_[^_]\+/\0${VER_APPEND}/")
		echo "Extracting ${DEB} ..."
		fakeroot dpkg -x ${DEB} package-data/
		fakeroot dpkg -e ${DEB} package-data/DEBIAN

		# Patching source and version
		echo "Patching ${NEW_BASE} control information ..."
		fakeroot sed -i -e "s/Version: .*/\0${VER_APPEND}/" \
				-e "/^\(Provides\|Depends\):/{s/_/-/g}" \
			package-data/DEBIAN/control
		echo "Source: $SOURCE" >> package-data/DEBIAN/control

		PACKAGE=$(sed -n -e 's/^Package: \(.*\)$/\1/p' package-data/DEBIAN/control)
		VERSION=$(sed -n -e 's/^Version: \(.*\)$/\1/p' package-data/DEBIAN/control)
		ARCH=$(sed -n -e 's/^Architecture: \(.*\)$/\1/p' package-data/DEBIAN/control)

		patch_libldap2
		patch_sdk_exec
		patch_paths_xcu
		patch_tmp_path
                patch_template_path
		#patch_opensymbol_lhm
		patch_icon_symlinks

		echo "Packaging ${NEW_BASE} ..."
		fakeroot mkdir -p "${NEW_DIR}"
		fakeroot dpkg-deb -b package-data/ "${NEW_DIR}/${PACKAGE}_${VERSION}_${ARCH}.deb"
		rm -rf package-data/
		mkdir package-data
	done
}

OLDIFS=$IFS
IFS=$'\n'

SOURCE="lo-orig"
DEBFILES=$(ls -1 "$DIR_IN"/main/*${DEBVER}*.deb "$DIR_IN"/sdk/*${DEBVER}*.deb)
patch_deb

SOURCE=$(basename $(ls "$DIR_IN"/main/libreoffice*-debian-menus_*.deb) | awk -F '_' '{ print $1; }')
DEBFILES=$(ls -1 "$DIR_IN"/main/libreoffice*-debian-menus_*.deb)
patch_deb

IFS=$OLDIFS

rm -rf package-data

