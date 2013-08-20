# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Rdb_Rdb,services))

$(eval $(call gb_Rdb_add_components,services,\
	animations/source/animcore/animcore \
	avmedia/util/avmedia \
	chart2/source/controller/chartcontroller \
	chart2/source/chartcore \
	canvas/source/factory/canvasfactory \
	canvas/source/simplecanvas/simplecanvas \
	canvas/source/vcl/vclcanvas \
	comphelper/util/comphelp \
	configmgr/source/configmgr \
	cppcanvas/source/uno/mtfrenderer \
	cui/util/cui \
	desktop/source/deployment/deployment \
	drawinglayer/drawinglayer \
	dtrans/util/mcnttype \
	embeddedobj/util/embobj \
	eventattacher/source/evtatt \
	fileaccess/source/fileacc \
	filter/source/config/cache/filterconfig1 \
	filter/source/flash/flash \
	filter/source/graphic/graphicfilter \
	filter/source/msfilter/msfilter \
	filter/source/odfflatxml/odfflatxml \
	filter/source/pdf/pdffilter \
	filter/source/placeware/placeware \
	filter/source/svg/svgfilter \
	filter/source/t602/t602filter \
	filter/source/textfilterdetect/textfd \
	filter/source/xmlfilteradaptor/xmlfa \
	filter/source/xmlfilterdetect/xmlfd \
	filter/source/xsltdialog/xsltdlg \
	filter/source/xsltfilter/xsltfilter \
	formula/util/for \
	fpicker/source/generic/fpicker \
	fpicker/source/office/fps_office \
	framework/util/fwk \
	framework/util/fwl \
	framework/util/fwm \
	hwpfilter/source/hwp \
	i18npool/source/search/i18nsearch \
	i18npool/util/i18npool \
	lingucomponent/source/hyphenator/altlinuxhyph/hyphen/hyphen \
	lingucomponent/source/languageguessing/guesslang \
	lingucomponent/source/spellcheck/spell/spell \
	lingucomponent/source/thesaurus/libnth/lnth \
	linguistic/source/lng \
	$(if $(ENABLE_LWP), \
	    lotuswordpro/util/lwpfilter \
	) \
	oox/util/oox \
	package/source/xstor/xstor \
	package/util/package2 \
	sax/source/expatwrap/expwrap \
	sax/source/fastparser/fastsax \
	sc/util/sc \
	sc/util/scd \
	sc/util/scfilt \
	scaddins/source/analysis/analysis \
	scaddins/source/datefunc/date \
	scaddins/source/pricing/pricing \
	sd/util/sd \
	sd/util/sdd \
	sd/util/sdfilt \
	sdext/source/presenter/presenter \
	sfx2/util/sfx \
	slideshow/util/slideshow \
	sot/util/sot \
	starmath/util/sm \
	starmath/util/smd \
	svl/source/fsstor/fsstorage \
	svl/source/passwordcontainer/passwordcontainer \
	svl/util/svl \
	svtools/source/hatchwindow/hatchwindowfactory \
	svtools/util/svt \
	svgio/svgio \
	svx/util/svx \
	svx/util/svxcore \
	svx/util/textconversiondlgs \
	sw/util/msword \
	sw/util/sw \
	sw/util/swd \
	toolkit/util/tk \
	ucb/source/sorter/srtrs1 \
	ucb/source/core/ucb1 \
	ucb/source/cacher/cached1 \
	ucb/source/ucp/cmis/ucpcmis1 \
	ucb/source/ucp/expand/ucpexpand1 \
	ucb/source/ucp/ext/ucpext \
	ucb/source/ucp/file/ucpfile1 \
	ucb/source/ucp/ftp/ucpftp1 \
	ucb/source/ucp/hierarchy/ucphier1 \
	ucb/source/ucp/package/ucppkg1 \
	ucb/source/ucp/tdoc/ucptdoc1 \
	UnoControls/util/ctl \
	unotools/util/utl \
	unoxml/source/rdf/unordf \
	unoxml/source/service/unoxml \
	uui/util/uui \
	xmloff/source/transform/xof \
	xmloff/util/xo \
	xmlscript/util/xmlscript \
	xmlsecurity/util/xmlsecurity \
	xmlsecurity/util/xsec_fw \
	$(if $(ENABLE_LPSOLVE), \
		sccomp/source/solver/solver \
	) \
	writerfilter/util/writerfilter \
	writerperfect/util/wpftwriter \
	writerperfect/util/wpftdraw \
	$(if $(filter MACOSX,$(OS)), \
		$(if $(filter-out X86_64,$(CPUNAME)), \
			avmedia/source/quicktime/avmediaQuickTime \
		) \
		lingucomponent/source/spellcheck/macosxspell/MacOSXSpell \
		fpicker/source/aqua/fps_aqua \
		shell/source/backends/macbe/macbe1 \
		vcl/vcl.macosx \
	) \
	$(if $(filter WNT,$(OS)), \
		dtrans/source/generic/dtrans \
		dtrans/util/dnd \
		dtrans/util/ftransl \
		dtrans/util/sysdtrans \
		fpicker/source/win32/fps \
		shell/source/backends/wininetbe/wininetbe1 \
		shell/source/win32/simplemail/smplmail \
		shell/source/win32/syssh \
		vcl/vcl.windows \
		$(if $(ENABLE_JAVA), \
			accessibility/bridge/org/openoffice/accessibility/java_uno_accessbridge \
		) \
	) \
	$(if $(ENABLE_HEADLESS), \
		vcl/vcl.headless \
	) \
	$(if $(filter-out WNT,$(OS)), \
		embeddedobj/source/msole/emboleobj \
	) \
	$(if $(DISABLE_ATL),, \
		$(if $(filter WNT,$(OS)), \
			embeddedobj/source/msole/emboleobj.windows \
			embedserv/util/emser \
			extensions/source/ole/oleautobridge \
		) \
	) \
	$(if $(DISABLE_NEON),, \
		ucb/source/ucp/webdav-neon/ucpdav1 \
	) \
	$(if $(DISABLE_SCRIPTING),, \
		basctl/util/basctl \
		basic/util/sb \
		sc/util/vbaobj \
		scripting/source/basprov/basprov \
		scripting/source/dlgprov/dlgprov \
		scripting/source/protocolhandler/protocolhandler \
		scripting/source/pyprov/mailmerge \
		wizards/com/sun/star/wizards/fax/fax \
		wizards/com/sun/star/wizards/letter/letter \
		wizards/com/sun/star/wizards/agenda/agenda \
		wizards/com/sun/star/wizards/web/web \
		scripting/source/stringresource/stringresource \
		scripting/source/vbaevents/vbaevents \
		scripting/util/scriptframe \
		sw/util/vbaswobj \
		vbahelper/util/msforms \
	) \
	$(if $(ENABLE_CAIRO_CANVAS), \
		canvas/source/cairo/cairocanvas \
	) \
	$(if $(ENABLE_DIRECTX), \
		avmedia/source/win/avmediawin \
		canvas/source/directx/directx9canvas \
		canvas/source/directx/gdipluscanvas \
	) \
	$(if $(ENABLE_GCONF), \
		shell/source/backends/gconfbe/gconfbe1 \
	) \
	$(if $(ENABLE_GIO), \
		shell/source/sessioninstall/losessioninstall \
		ucb/source/ucp/gio/ucpgio \
	) \
	$(if $(ENABLE_GNOMEVFS), \
		ucb/source/ucp/gvfs/ucpgvfs \
	) \
	$(if $(ENABLE_GSTREAMER), \
		avmedia/source/gstreamer/avmediagstreamer \
	) \
	$(if $(ENABLE_GSTREAMER_0_10), \
		avmedia/source/gstreamer/avmediagstreamer_0_10 \
	) \
	$(if $(ENABLE_VLC), \
		avmedia/source/vlc/avmediavlc \
	) \
	$(if $(ENABLE_KDE), \
		shell/source/backends/kdebe/kdebe1 \
	) \
	$(if $(ENABLE_KDE4), \
		shell/source/backends/kde4be/kde4be1 \
	) \
	$(if $(ENABLE_ONLINE_UPDATE), \
		extensions/source/update/check/updchk.uno \
		extensions/source/update/ui/updchk \
	) \
	$(if $(ENABLE_OPENGL), \
		slideshow/source/engine/OGLTrans/ogltrans \
	) \
	$(if $(ENABLE_TDE), \
		shell/source/backends/kdebe/tdebe1 \
	) \
	$(if $(ENABLE_JAVA), \
		xmerge/source/bridge/XMergeBridge \
		scripting/java/ScriptFramework \
		scripting/java/ScriptProviderForJava \
		smoketest/org/libreoffice/smoketest/smoketest \
		wizards/com/sun/star/wizards/form/form \
		wizards/com/sun/star/wizards/query/query \
		wizards/com/sun/star/wizards/report/report \
		wizards/com/sun/star/wizards/table/table \
	) \
))

ifeq (DBCONNECTIVITY,$(filter DBCONNECTIVITY,$(BUILD_TYPE)))

$(eval $(call gb_Rdb_add_components,services,\
	extensions/source/bibliography/bib \
	extensions/source/dbpilots/dbp \
	extensions/source/propctrlr/pcr \
	connectivity/source/cpool/dbpool2 \
	connectivity/source/dbtools/dbtools \
	$(if $(filter WNT,$(OS)), \
		connectivity/source/drivers/ado/ado \
	) \
	connectivity/source/drivers/calc/calc \
	connectivity/source/drivers/dbase/dbase \
	$(if $(ENABLE_EVOAB2), \
		connectivity/source/drivers/evoab2/evoab \
	) \
	$(if $(filter $(ENABLE_FIREBIRD_SDBC),TRUE), \
		connectivity/source/drivers/firebird/firebird_sdbc \
	) \
	connectivity/source/drivers/flat/flat \
	$(if $(ENABLE_KAB), \
		connectivity/source/drivers/kab/kab1 \
	) \
	connectivity/source/drivers/mysql/mysql \
	$(if $(filter MACOSX,$(OS)), \
		connectivity/source/drivers/macab/macab1 \
	) \
	$(if $(ENABLE_TDEAB), \
		connectivity/source/drivers/kab/tdeab1 \
	) \
	$(if $(SOLAR_JAVA), \
		connectivity/source/drivers/hsqldb/hsqldb \
		connectivity/source/drivers/jdbc/jdbc \
	) \
	connectivity/source/manager/sdbc2 \
	dbaccess/source/ext/macromigration/dbmm \
	dbaccess/source/filter/xml/dbaxml \
	dbaccess/util/dba \
	dbaccess/util/dbu \
	dbaccess/util/sdbt \
	forms/util/frm \
	reportdesign/util/rpt \
	reportdesign/util/rptui \
	reportdesign/util/rptxml \
	shell/source/backends/localebe/localebe1 \
	$(if $(filter-out ANDROID IOS,$(OS)),\
		connectivity/source/drivers/odbc/odbc \
	) \
))

ifeq ($(OS),WNT)

ifeq ($(WITH_MOZAB4WIN),YES)
$(eval $(call gb_Rdb_add_components,services,\
	connectivity/source/drivers/mozab/mozab \
))
else
$(eval $(call gb_Rdb_add_components,services,\
	connectivity/source/drivers/mozab/bootstrap/mozbootstrap \
))
endif

else

$(eval $(call gb_Rdb_add_components,services,\
	connectivity/source/drivers/mork/mork \
	connectivity/source/drivers/mozab/bootstrap/mozbootstrap \
))

endif # WNT

endif # DBCONNECTIVITY

ifeq (DESKTOP,$(filter DESKTOP,$(BUILD_TYPE)))

$(eval $(call gb_Rdb_add_components,services,\
	desktop/source/deployment/gui/deploymentgui \
	desktop/source/migration/services/migrationoo2 \
	desktop/source/migration/services/migrationoo3 \
	desktop/source/offacc/offacc \
	$(if $(ENABLE_HEADLESS),,desktop/source/splash/spl) \
	extensions/source/abpilot/abp \
	extensions/source/config/ldap/ldapbe2 \
	extensions/source/logging/log \
	extensions/source/resource/res \
	extensions/source/scanner/scn \
	extensions/source/update/feed/updatefeed \
	xmlhelp/source/treeview/tvhlp1 \
	xmlhelp/util/ucpchelp1 \
	xmlsecurity/util/xsec_xmlsec$(if $(filter WNT,$(OS)),.windows) \
	$(if $(filter-out WNT,$(OS)),\
		shell/source/cmdmail/cmdmail \
		shell/source/unix/exec/syssh \
	) \
	$(if $(filter-out MACOSX WNT,$(OS)), \
		desktop/unx/splash/splash \
		$(if $(ENABLE_HEADLESS),, \
			shell/source/backends/desktopbe/desktopbe1 \
			vcl/vcl.unx \
		) \
	) \
))

endif # DESKTOP

ifeq ($(ENABLE_NPAPI_FROM_BROWSER),YES)
$(eval $(call gb_Rdb_add_components,services,\
	extensions/source/plugin/pl \
))
endif

ifeq ($(ENABLE_PDFIMPORT),TRUE)
$(eval $(call gb_Rdb_add_components,services,\
	sdext/source/pdfimport/pdfimport \
))
endif

ifeq ($(ENABLE_REPORTBUILDER),TRUE)
$(eval $(call gb_Rdb_add_components,services,\
	reportbuilder/java/reportbuilder \
))
endif

# vim: set noet sw=4 ts=4:
