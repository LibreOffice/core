#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#***********************************************************************/

PRJ = ..
PRJNAME = postprocess
TARGET = packcomponents

.INCLUDE: settings.mk

my_components = \
    component/animations/source/animcore/animcore \
    component/avmedia/util/avmedia \
    component/chart2/source/controller/chartcontroller \
    component/chart2/source/chartcore \
    component/canvas/source/factory/canvasfactory \
    component/canvas/source/simplecanvas/simplecanvas \
    component/canvas/source/vcl/vclcanvas \
    component/comphelper/util/comphelp \
    component/configmgr/source/configmgr \
    component/cppcanvas/source/uno/mtfrenderer \
    component/cui/util/cui \
    component/dtrans/util/mcnttype \
    component/embeddedobj/util/embobj \
    component/eventattacher/source/evtatt \
    component/fileaccess/source/fileacc \
    component/filter/source/config/cache/filterconfig1 \
    component/filter/source/flash/flash \
    component/filter/source/msfilter/msfilter \
    component/filter/source/odfflatxml/odfflatxml \
    component/filter/source/pdf/pdffilter \
    component/filter/source/placeware/placeware \
    component/filter/source/svg/svgfilter \
    component/filter/source/t602/t602filter \
    component/filter/source/textfilterdetect/textfd \
    component/filter/source/xmlfilteradaptor/xmlfa \
    component/filter/source/xmlfilterdetect/xmlfd \
    component/filter/source/xsltdialog/xsltdlg \
    component/filter/source/xsltfilter/xsltfilter \
    component/formula/util/for \
    component/fpicker/source/generic/fpicker \
    component/fpicker/source/office/fps_office \
    component/framework/util/fwk \
    component/framework/util/fwl \
    component/framework/util/fwm \
    component/hwpfilter/source/hwp \
    component/i18npool/source/search/i18nsearch \
    component/i18npool/util/i18npool \
    component/lingucomponent/source/hyphenator/altlinuxhyph/hyphen/hyphen \
    component/lingucomponent/source/languageguessing/guesslang \
    component/lingucomponent/source/spellcheck/spell/spell \
    component/lingucomponent/source/thesaurus/libnth/lnth \
    component/linguistic/source/lng \
    component/lotuswordpro/util/lwpfilter \
    component/oox/util/oox \
    component/package/source/xstor/xstor \
    component/package/util/package2 \
    component/sax/source/expatwrap/expwrap \
    component/sax/source/fastparser/fastsax \
    component/sc/util/sc \
    component/sc/util/scd \
    component/sc/util/scfilt \
    component/scaddins/source/analysis/analysis \
    component/scaddins/source/datefunc/date \
    component/sd/util/sd \
    component/sd/util/sdd \
    component/sd/util/sdfilt \
    component/sfx2/util/sfx \
    component/slideshow/util/slideshow \
    component/sot/util/sot \
    component/starmath/util/sm \
    component/starmath/util/smd \
    component/svl/source/fsstor/fsstorage \
    component/svl/source/passwordcontainer/passwordcontainer \
    component/svl/util/svl \
    component/svtools/source/hatchwindow/hatchwindowfactory \
    component/svtools/util/svt \
    component/svx/util/svx \
    component/svx/util/svxcore \
    component/svx/util/textconversiondlgs \
    component/sw/util/msword \
    component/sw/util/sw \
    component/sw/util/swd \
    component/toolkit/util/tk \
    component/ucb/source/sorter/srtrs1 \
    component/ucb/source/core/ucb1 \
    component/ucb/source/cacher/cached1 \
    component/ucb/source/ucp/cmis/ucpcmis1 \
    component/ucb/source/ucp/expand/ucpexpand1 \
    component/ucb/source/ucp/ext/ucpext \
    component/ucb/source/ucp/file/ucpfile1 \
    component/ucb/source/ucp/ftp/ucpftp1 \
    component/ucb/source/ucp/hierarchy/ucphier1 \
    component/ucb/source/ucp/package/ucppkg1 \
    component/ucb/source/ucp/tdoc/ucptdoc1 \
    component/UnoControls/util/ctl \
    component/unotools/util/utl \
    component/unoxml/source/rdf/unordf \
    component/unoxml/source/service/unoxml \
    component/uui/util/uui \
    component/xmloff/source/transform/xof \
    component/xmloff/util/xo \
    component/xmlscript/util/xmlscript \
    component/xmlsecurity/util/xmlsecurity \
    component/xmlsecurity/util/xsec_fw \

.IF "$(BUILD_TYPE)" != "$(BUILD_TYPE:s/DBCONNECTIVITY//)"

my_components += \
    component/connectivity/source/drivers/calc/calc \
    component/dbaccess/source/ext/macromigration/dbmm \
    component/dbaccess/source/filter/xml/dbaxml \
    component/dbaccess/util/dba \
    component/dbaccess/util/dbu \
    component/dbaccess/util/sdbt \
    component/forms/util/frm \
    component/reportdesign/util/rpt \
    component/reportdesign/util/rptui \
    component/reportdesign/util/rptxml \
    component/connectivity/source/drivers/dbase/dbase \
    component/connectivity/source/cpool/dbpool2 \
    component/connectivity/source/dbtools/dbtools \
    component/connectivity/source/drivers/flat/flat \
    component/connectivity/source/drivers/mysql/mysql \
    component/connectivity/source/manager/sdbc2 \
    component/shell/source/backends/localebe/localebe1 \

.IF "$(OS)" != "ANDROID" && "$(OS)" != "IOS"
my_components += \
    component/connectivity/source/drivers/odbc/odbc \

.ENDIF

.ENDIF

.IF "$(BUILD_TYPE)" != "$(BUILD_TYPE:s/DESKTOP//)"

my_components += \
    component/desktop/source/deployment/deployment \
    component/desktop/source/deployment/gui/deploymentgui \
    component/desktop/source/migration/services/migrationoo2 \
    component/desktop/source/migration/services/migrationoo3 \
    component/desktop/source/offacc/offacc \
    component/desktop/source/splash/spl \
    component/extensions/source/abpilot/abp \
    component/extensions/source/bibliography/bib \
	component/extensions/source/config/ldap/ldapbe2 \
    component/extensions/source/dbpilots/dbp \
    component/extensions/source/logging/log \
    component/extensions/source/propctrlr/pcr \
    component/extensions/source/resource/res \
    component/extensions/source/scanner/scn \
    component/extensions/source/update/feed/updatefeed \
    component/xmlhelp/source/treeview/tvhlp1 \
    component/xmlhelp/util/ucpchelp1 \

.IF "$(GUI)" == "UNX"
my_components += \
    component/shell/source/unix/exec/syssh \

.ENDIF

.ENDIF

.IF "$(DISABLE_SCRIPTING)" != "TRUE"

my_components += \
    component/basctl/util/basctl \
    component/basic/util/sb \
    component/sc/util/vbaobj \
    component/scripting/source/basprov/basprov \
    component/scripting/source/dlgprov/dlgprov \
    component/scripting/source/protocolhandler/protocolhandler \
    component/scripting/source/pyprov/mailmerge \
    component/wizards/com/sun/star/wizards/fax/fax \
    component/scripting/source/stringresource/stringresource \
    component/scripting/source/vbaevents/vbaevents \
    component/scripting/util/scriptframe \
    component/sw/util/vbaswobj \
    component/vbahelper/util/msforms \

.ENDIF

.IF "$(DISABLE_PYTHON)" != "TRUE"

my_components += \
    component/pyuno/source/loader/pythonloader \

.ENDIF

.IF "$(OS)" != "IOS"

my_components += \
    component/sccomp/source/solver/solver \
    component/writerfilter/util/writerfilter \
    component/writerperfect/util/wpftwriter \
    component/writerperfect/util/wpftdraw \

.ENDIF

.IF "$(OS)" != "WNT" && "$(OS)" != "MACOSX" && "$(OS)" != "IOS" && "$(OS)" != "ANDROID" && "$(GUIBASE)" != "headless"

my_components += \
    component/desktop/unx/splash/splash \

.ENDIF

.IF "$(DISABLE_ATL)" == ""
.IF "$(OS)" == "WNT"
my_components += component/embeddedobj/source/msole/emboleobj.windows
.ELSE
my_components += component/embeddedobj/source/msole/emboleobj
.ENDIF
.END

.IF "$(DISABLE_NEON)" != "TRUE"
my_components += component/ucb/source/ucp/webdav-neon/ucpdav1
.END

.IF "$(ENABLE_CAIRO_CANVAS)" == "TRUE"
my_components += component/canvas/source/cairo/cairocanvas
.END

.IF "$(ENABLE_GCONF)" != ""
my_components += component/shell/source/backends/gconfbe/gconfbe1
.END

.IF "$(ENABLE_GIO)" != ""
my_components += component/ucb/source/ucp/gio/ucpgio
.END

.IF "$(ENABLE_GNOMEVFS)" != ""
my_components += component/ucb/source/ucp/gvfs/ucpgvfs
.END

.IF "$(ENABLE_TDEAB)" != ""
my_components += component/connectivity/source/drivers/kab/tdeab1
.END

.IF "$(ENABLE_TDE)" != ""
my_components += component/shell/source/backends/kdebe/tdebe1
.END

.IF "$(ENABLE_KAB)" != ""
my_components += component/connectivity/source/drivers/kab/kab1
.END

.IF "$(ENABLE_KDE)" != ""
my_components += component/shell/source/backends/kdebe/kdebe1
.END

.IF "$(ENABLE_KDE4)" != ""
my_components += component/shell/source/backends/kde4be/kde4be1
.END

.IF "$(ENABLE_ONLINE_UPDATE)" == "TRUE"
my_components += \
    component/extensions/source/update/check/updchk.uno \
    component/extensions/source/update/ui/updchk
.END

.IF "$(ENABLE_OPENGL)" == "TRUE"
my_components += component/slideshow/source/engine/OGLTrans/ogltrans
.END

.IF "$(ENABLE_LOMENUBAR)" == "TRUE"
my_components += component/framework/util/lomenubar
.END

.IF "$(SOLAR_JAVA)" == "TRUE"
my_components += \
    component/xmerge/source/bridge/XMergeBridge \
    component/filter/source/xsltfilter/XSLTFilter.jar \
    component/filter/source/xsltvalidate/XSLTValidate \
    component/scripting/java/ScriptFramework \
    component/scripting/java/ScriptProviderForJava \
    component/wizards/com/sun/star/wizards/agenda/agenda \
    component/wizards/com/sun/star/wizards/form/form \
    component/connectivity/source/drivers/hsqldb/hsqldb \
    component/connectivity/source/drivers/jdbc/jdbc \
    component/wizards/com/sun/star/wizards/letter/letter \
    component/wizards/com/sun/star/wizards/query/query \
    component/wizards/com/sun/star/wizards/report/report \
    component/wizards/com/sun/star/wizards/table/table \
    component/wizards/com/sun/star/wizards/web/web
.END

.IF "$(WITH_BINFILTER)" != "NO"
my_components += \
    bf_migratefilter \
    bindet
.END

.IF "$(WITH_MOZILLA)" != "NO"
my_components += component/extensions/source/plugin/pl
.END

.IF "$(OS)" == "WNT"
my_components += component/xmlsecurity/util/xsec_xmlsec.windows
.ELIF "$(OS)" != "ANDROID" && "$(OS)" != "IOS" #FIXME, get nss&xmlsec building
my_components += component/xmlsecurity/util/xsec_xmlsec
.END

.IF "$(OS)" == "MACOSX"
my_components += \
    component/connectivity/source/drivers/macab/macab1 \
    component/fpicker/source/aqua/fps_aqua \
    component/shell/source/backends/macbe/macbe1 \
    component/vcl/vcl.macosx
.END

.IF "$(GUIBASE)" == "aqua"
.IF "$(CPUNAME)" != "X86_64"
my_components += \
    component/avmedia/source/quicktime/avmediaQuickTime
.ENDIF
my_components += \
    component/lingucomponent/source/spellcheck/macosxspell/MacOSXSpell
.END

.IF "$(OS)" == "WNT"
my_components += \
    component/connectivity/source/drivers/ado/ado \
    component/dtrans/source/generic/dtrans \
    component/dtrans/util/dnd \
    component/dtrans/util/ftransl \
    component/dtrans/util/sysdtrans \
    component/fpicker/source/win32/fps \
    component/shell/source/backends/wininetbe/wininetbe1 \
    component/shell/source/win32/simplemail/smplmail \
    component/shell/source/win32/syssh \
    component/vcl/vcl.windows
.IF "$(SOLAR_JAVA)" == "TRUE"
my_components += \
    component/accessibility/bridge/org/openoffice/accessibility/java_uno_accessbridge
.END
.END

.IF "$(GUIBASE)" == "headless"
my_components += \
    component/vcl/vcl.headless
.END

.IF "$(OS)" != "MACOSX" && "$(OS)" != "WNT" && "$(OS)" != "IOS" && "$(OS)" != "ANDROID" && "$(GUIBASE)" != "headless"
my_components += \
    component/shell/source/backends/desktopbe/desktopbe1 \
    component/vcl/vcl.unx
.END

.IF "$(OS)" == "WNT" && "$(DISABLE_ATL)" == ""
my_components += \
    component/embedserv/util/emser \
    component/extensions/source/ole/oleautobridge
.END

.IF "$(OS)" == "WNT" && "$(ENABLE_DIRECTX)" != ""
my_components += \
    component/avmedia/source/win/avmediawin \
    component/canvas/source/directx/directx9canvas \
    component/canvas/source/directx/gdipluscanvas
.END

.IF "$(OS)" == "WNT"
.IF "$(SYSTEM_MOZILLA)" != "YES" && "$(WITH_MOZILLA)" != "NO"
my_components += component/connectivity/source/drivers/mozab/mozab
.ELSE
my_components += component/connectivity/source/drivers/mozab/bootstrap/mozbootstrap
.END
.ELIF "$(OS)" != "ANDROID" && "$(OS)" != "IOS"
my_components += component/connectivity/source/drivers/mork/mork
.END

.IF "$(OS)" != "WNT" && "$(OS)" != "ANDROID" && "$(OS)" != "IOS" && "$(OS)" != "headless"
my_components += component/shell/source/cmdmail/cmdmail
.END

.IF "$(OS)" != "WNT" && "$(ENABLE_EVOAB2)" != ""
my_components += component/connectivity/source/drivers/evoab2/evoab
.END

.IF "$(OS)" != "WNT" && "$(ENABLE_GSTREAMER)" != ""
my_components += component/avmedia/source/gstreamer/avmediagstreamer
.END

.IF "$(OS)" != "WNT" && "$(ENABLE_GSTREAMER_0_10)" != ""
my_components += component/avmedia/source/gstreamer/avmediagstreamer_0_10
.END

.INCLUDE: target.mk

ALLTAR : $(MISC)/services.rdb

$(MISC)/services.rdb .ERRREMOVE : $(SOLARENV)/bin/packcomponents.xslt \
        $(MISC)/services.input $(my_components:^"$(SOLARXMLDIR)/":+".component")
    $(XSLTPROC) --nonet --stringparam prefix $(SOLARXMLDIR)/ -o $@ \
        $(SOLARENV)/bin/packcomponents.xslt $(MISC)/services.input

$(MISC)/services.input : makefile.mk
    echo \
        '<list>$(my_components:^"<filename>":+".component</filename>")</list>' \
        > $@

