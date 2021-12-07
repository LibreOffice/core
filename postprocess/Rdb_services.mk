# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Rdb_Rdb_install,services))

# see Makefile.gbuild for the gb_Rdb_add_components,services calls
ifeq (,$(ENABLE_SERVICES_RDB_FROM_BUILD))

#
# "minimal" list of services for cross-compiling build tools
#
$(eval $(call gb_Rdb_add_components,services,\
	$(if $(DISABLE_GUI),,$(call gb_Helper_optional,AVMEDIA,avmedia/util/avmedia)) \
	$(call gb_Helper_optional,SCRIPTING,basic/util/sb) \
	canvas/source/factory/canvasfactory \
	canvas/source/simplecanvas/simplecanvas \
	canvas/source/vcl/vclcanvas \
	$(if $(ENABLE_CAIRO_CANVAS),canvas/source/cairo/cairocanvas) \
	$(if $(ENABLE_OPENGL_CANVAS),canvas/source/opengl/oglcanvas) \
	$(if $(filter WNT,$(OS)), \
		canvas/source/directx/directx9canvas \
		canvas/source/directx/gdipluscanvas \
	) \
	comphelper/util/comphelp \
	configmgr/source/configmgr \
	$(if $(filter DBCONNECTIVITY,$(BUILD_TYPE)), \
		connectivity/source/dbtools/dbtools \
		connectivity/source/drivers/dbase/dbase \
	) \
	cppcanvas/source/uno/mtfrenderer \
	drawinglayer/drawinglayer \
	emfio/emfio \
	filter/source/config/cache/filterconfig1 \
	filter/source/graphic/graphicfilter \
	filter/source/svg/svgfilter \
	framework/util/fwk \
	i18npool/source/search/i18nsearch \
	$(if $(filter en,$(WITH_LOCALES)),i18npool/util/i18npool.en,i18npool/util/i18npool) \
	package/source/xstor/xstor \
	package/util/package2 \
	sax/source/expatwrap/expwrap \
	sfx2/util/sfx \
	sot/util/sot \
	svgio/svgio \
	svl/source/fsstor/fsstorage \
	svl/source/passwordcontainer/passwordcontainer \
	svl/util/svl \
	svtools/util/svt \
	svx/util/svxcore \
	toolkit/util/tk \
	ucb/source/core/ucb1 \
	ucb/source/ucp/file/ucpfile1 \
	unotools/util/utl \
	unoxml/source/service/unoxml \
	uui/util/uui \
	vcl/vcl.common \
	$(if $(filter ANDROID,$(OS)),vcl/vcl.android) \
	$(if $(DISABLE_GUI),vcl/vcl.headless) \
	$(if $(filter iOS,$(OS)),vcl/vcl.ios) \
	$(if $(filter MACOSX,$(OS)),vcl/vcl.macosx) \
	$(if $(filter WNT,$(OS)),\
		vcl/vcl.windows \
		vcl/vclplug_win \
	) \
))

ifeq ($(gb_Side),host)

$(eval $(call gb_Rdb_add_components,services,\
	animations/source/animcore/animcore \
	chart2/source/controller/chartcontroller \
	chart2/source/chartcore \
	cui/util/cui \
	desktop/source/deployment/deployment \
	embeddedobj/util/embobj \
	eventattacher/source/evtatt \
	filter/source/msfilter/msfilter \
	filter/source/odfflatxml/odfflatxml \
	filter/source/pdf/pdffilter \
	filter/source/storagefilterdetect/storagefd \
	filter/source/t602/t602filter \
	filter/source/textfilterdetect/textfd \
	filter/source/xmlfilteradaptor/xmlfa \
	filter/source/xmlfilterdetect/xmlfd \
	filter/source/xsltdialog/xsltdlg \
	filter/source/xsltfilter/xsltfilter \
	formula/util/for \
	$(call gb_Helper_optional,DESKTOP,fpicker/source/office/fps_office) \
	$(if $(filter MACOSX,$(OS)),fpicker/source/aqua/fps_aqua) \
	hwpfilter/source/hwp \
	lingucomponent/source/hyphenator/hyphen/hyphen \
	lingucomponent/source/languageguessing/guesslang \
	lingucomponent/source/spellcheck/spell/spell \
	lingucomponent/source/thesaurus/libnth/lnth \
	lingucomponent/source/numbertext/numbertext \
	linguistic/source/lng \
	$(if $(ENABLE_LWP), \
	    lotuswordpro/util/lwpfilter \
	) \
	oox/util/oox \
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
	sdext/source/minimizer/minimizer \
	slideshow/util/slideshow \
	starmath/util/sm \
	starmath/util/smd \
    $(call gb_Helper_optional,BREAKPAD,svx/util/crashreport) \
	svx/util/svx \
	svx/util/textconversiondlgs \
	sw/util/msword \
	sw/util/sw \
	sw/util/swd \
	ucb/source/sorter/srtrs1 \
	ucb/source/cacher/cached1 \
	$(if $(ENABLE_LIBCMIS),ucb/source/ucp/cmis/ucpcmis1) \
	ucb/source/ucp/expand/ucpexpand1 \
	ucb/source/ucp/ext/ucpext \
	$(if $(ENABLE_CURL),ucb/source/ucp/ftp/ucpftp1) \
	ucb/source/ucp/hierarchy/ucphier1 \
	ucb/source/ucp/image/ucpimage \
	ucb/source/ucp/package/ucppkg1 \
	ucb/source/ucp/tdoc/ucptdoc1 \
	UnoControls/util/ctl \
	unoxml/source/rdf/unordf \
	xmloff/source/transform/xof \
	xmloff/util/xo \
	xmlscript/util/xmlscript \
	$(if $(ENABLE_NSS), \
		xmlsecurity/util/xmlsecurity \
		xmlsecurity/util/xsec_xmlsec) \
	$(if $(ENABLE_COINMP), \
		sccomp/source/solver/coinmpsolver \
	) \
	$(if $(ENABLE_LPSOLVE), \
		sccomp/source/solver/lpsolvesolver \
	) \
	sccomp/source/solver/swarmsolver \
	writerfilter/util/writerfilter \
	writerperfect/source/draw/wpftdraw \
	writerperfect/source/impress/wpftimpress \
	writerperfect/source/writer/wpftwriter \
	writerperfect/source/calc/wpftcalc \
	$(if $(filter MACOSX,$(OS)), \
		$(call gb_Helper_optional,AVMEDIA,avmedia/source/macavf/avmediaMacAVF) \
		shell/source/backends/macbe/macbe1 \
	) \
	$(if $(filter iOS MACOSX,$(OS)), \
		lingucomponent/source/spellcheck/macosxspell/MacOSXSpell \
	) \
	$(if $(filter WNT,$(OS)), \
		$(call gb_Helper_optional,AVMEDIA,avmedia/source/win/avmediawin) \
		embeddedobj/source/msole/emboleobj.windows \
		embedserv/util/emser \
		extensions/source/ole/oleautobridge \
		fpicker/source/win32/fps \
		shell/source/backends/wininetbe/wininetbe1 \
		shell/source/win32/simplemail/smplmail \
		shell/source/win32/syssh \
		winaccessibility/source/service/winaccessibility \
	) \
	$(if $(filter-out WNT,$(OS)), \
		embeddedobj/source/msole/emboleobj \
	) \
	$(if $(filter neon,$(WITH_WEBDAV)), \
		ucb/source/ucp/webdav-neon/ucpdav1 \
	) \
	$(if $(filter serf,$(WITH_WEBDAV)), \
		ucb/source/ucp/webdav/ucpdav1 \
	) \
	$(if $(filter curl,$(WITH_WEBDAV)), \
		ucb/source/ucp/webdav-curl/ucpdav1 \
	) \
	$(call gb_Helper_optional,SCRIPTING, \
		basctl/util/basctl \
		sc/util/vbaobj \
		scripting/source/basprov/basprov \
		scripting/source/dlgprov/dlgprov \
		scripting/source/protocolhandler/protocolhandler \
		scripting/source/stringresource/stringresource \
		scripting/source/vbaevents/vbaevents \
		scripting/util/scriptframe \
		sw/util/vbaswobj \
		vbahelper/util/msforms \
		$(if $(DISABLE_PYTHON),, \
			scripting/source/pyprov/mailmerge \
			wizards/com/sun/star/wizards/agenda/agenda \
			wizards/com/sun/star/wizards/fax/fax \
			wizards/com/sun/star/wizards/letter/letter \
		) \
	) \
	$(if $(ENABLE_GIO), \
		shell/source/sessioninstall/losessioninstall \
		ucb/source/ucp/gio/ucpgio \
	) \
	$(if $(ENABLE_GSTREAMER_1_0), \
		$(call gb_Helper_optional,AVMEDIA,avmedia/source/gstreamer/avmediagstreamer) \
	) \
	$(if $(ENABLE_KF5), \
		shell/source/backends/kf5be/kf5be1 \
	) \
	$(if $(ENABLE_ONLINE_UPDATE), \
		extensions/source/update/check/updchk.uno \
		extensions/source/update/ui/updchk \
	) \
	$(if $(ENABLE_OPENGL_TRANSITIONS), \
		slideshow/source/engine/opengl/ogltrans \
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
    $(call gb_Helper_optional,XMLHELP,xmlhelp/util/ucpchelp1) \
))

$(eval $(call gb_Rdb_add_components,services,\
	extensions/source/bibliography/bib \
	dbaccess/util/dba \
))

ifneq (,$(filter DBCONNECTIVITY,$(BUILD_TYPE)))

$(eval $(call gb_Rdb_add_components,services,\
	extensions/source/dbpilots/dbp \
	extensions/source/propctrlr/pcr \
	connectivity/source/cpool/dbpool2 \
	$(if $(filter WNT,$(OS)), \
		connectivity/source/drivers/ado/ado \
	) \
	connectivity/source/drivers/calc/calc \
	$(if $(ENABLE_EVOAB2), \
		connectivity/source/drivers/evoab2/evoab \
	) \
	$(if $(filter $(ENABLE_FIREBIRD_SDBC),TRUE), \
		connectivity/source/drivers/firebird/firebird_sdbc \
	) \
	connectivity/source/drivers/flat/flat \
	$(if $(ENABLE_MARIADBC), \
		connectivity/source/drivers/mysqlc/mysqlc \
	) \
	$(if $(filter MACOSX,$(OS)), \
		connectivity/source/drivers/macab/macab1 \
	) \
	$(if $(ENABLE_JAVA), \
		connectivity/source/drivers/hsqldb/hsqldb \
		connectivity/source/drivers/jdbc/jdbc \
	) \
	connectivity/source/drivers/mysql_jdbc/mysql_jdbc \
	connectivity/source/manager/sdbc2 \
	connectivity/source/drivers/writer/writer \
	dbaccess/source/filter/xml/dbaxml \
	dbaccess/util/dbu \
	dbaccess/util/sdbt \
	forms/util/frm \
	reportdesign/util/rpt \
	reportdesign/util/rptui \
	reportdesign/util/rptxml \
	shell/source/backends/localebe/localebe1 \
    sw/util/sw_mm \
	$(if $(filter-out ANDROID iOS,$(OS)),\
		connectivity/source/drivers/odbc/odbc \
	) \
))

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
$(eval $(call gb_Rdb_add_components,services,\
	connectivity/source/drivers/mozab/bootstrap/mozbootstrap \
))
endif # DESKTOP

endif # DBCONNECTIVITY

ifeq (DESKTOP,$(filter DESKTOP,$(BUILD_TYPE)))

$(eval $(call gb_Rdb_add_components,services,\
	desktop/source/deployment/gui/deploymentgui \
	desktop/source/migration/services/migrationoo2 \
	desktop/source/migration/services/migrationoo3 \
	desktop/source/offacc/offacc \
	$(if $(DISABLE_GUI),,desktop/source/splash/spl) \
	extensions/source/abpilot/abp \
	$(if $(ENABLE_LDAP),extensions/source/config/ldap/ldapbe2) \
	$(if $(filter WNT,$(OS)),\
		extensions/source/config/WinUserInfo/WinUserInfoBe \
	) \
	extensions/source/logging/log \
	extensions/source/scanner/scn \
	extensions/source/update/feed/updatefeed \
	$(if $(filter-out WNT,$(OS)),\
		shell/source/cmdmail/cmdmail \
	) \
	$(if $(filter-out WNT,$(OS)),\
		shell/source/unix/exec/syssh \
	) \
	$(if $(filter-out MACOSX WNT,$(OS)), \
		$(if $(DISABLE_GUI),, \
			shell/source/backends/desktopbe/desktopbe1 \
			vcl/vcl.unx \
		) \
	) \
))

else # DESKTOP

$(eval $(call gb_Rdb_add_components,services,\
	$(if $(filter ANDROID,$(OS)),\
		extensions/source/logging/log \
	) \
))

endif # DESKTOP

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

endif # $(gb_Side),host

endif # !ENABLE_SERVICES_RDB_FROM_BUILD

# vim: set noet sw=4 ts=4:
