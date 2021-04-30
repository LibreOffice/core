# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,cui))

ifeq ($(OS),WNT)
$(eval $(call gb_UIConfig_add_uifiles,cui,\
	cui/uiconfig/ui/fileextcheckdialog \
))
endif

ifneq ($(ENABLE_WASM_STRIP_PINGUSER),TRUE)
$(eval $(call gb_UIConfig_add_uifiles,cui,\
	cui/uiconfig/ui/tipofthedaydialog \
))
endif

$(eval $(call gb_UIConfig_add_uifiles,cui,\
	cui/uiconfig/ui/aboutdialog \
	cui/uiconfig/ui/aboutconfigdialog\
	cui/uiconfig/ui/aboutconfigvaluedialog \
	cui/uiconfig/ui/accelconfigpage \
	cui/uiconfig/ui/additionsdialog \
	cui/uiconfig/ui/additionsfragment \
	cui/uiconfig/ui/agingdialog \
	cui/uiconfig/ui/acorexceptpage \
	cui/uiconfig/ui/acoroptionspage \
	cui/uiconfig/ui/acorreplacepage \
	cui/uiconfig/ui/applyautofmtpage \
	cui/uiconfig/ui/applylocalizedpage \
	cui/uiconfig/ui/areadialog \
	cui/uiconfig/ui/areatabpage \
	cui/uiconfig/ui/asiantypography \
	cui/uiconfig/ui/assigncomponentdialog \
	cui/uiconfig/ui/autocorrectdialog \
	cui/uiconfig/ui/baselinksdialog \
	cui/uiconfig/ui/imagetabpage \
	cui/uiconfig/ui/borderareatransparencydialog \
	cui/uiconfig/ui/borderbackgrounddialog \
	cui/uiconfig/ui/borderpage \
	cui/uiconfig/ui/bulletandposition \
	cui/uiconfig/ui/breaknumberoption \
	cui/uiconfig/ui/calloutdialog \
	cui/uiconfig/ui/calloutpage \
	cui/uiconfig/ui/cellalignment \
	cui/uiconfig/ui/certdialog \
	cui/uiconfig/ui/chapterfragment \
	cui/uiconfig/ui/charnamepage \
	cui/uiconfig/ui/colorconfigwin \
	cui/uiconfig/ui/colorfragment \
	cui/uiconfig/ui/colorpage \
	cui/uiconfig/ui/colorpickerdialog \
	cui/uiconfig/ui/comment \
	cui/uiconfig/ui/connectortabpage \
	cui/uiconfig/ui/connpooloptions \
	cui/uiconfig/ui/croppage \
	cui/uiconfig/ui/cuiimapdlg \
	cui/uiconfig/ui/databaselinkdialog \
	cui/uiconfig/ui/diagramdialog \
	cui/uiconfig/ui/dimensionlinestabpage \
	cui/uiconfig/ui/editdictionarydialog \
	cui/uiconfig/ui/editmodulesdialog \
	cui/uiconfig/ui/embossdialog \
	cui/uiconfig/ui/entrycontextmenu \
	cui/uiconfig/ui/eventassigndialog \
	cui/uiconfig/ui/eventassignpage \
	cui/uiconfig/ui/fontfragment \
	cui/uiconfig/ui/formatnumberdialog \
	cui/uiconfig/ui/fmsearchdialog \
	cui/uiconfig/ui/gradientpage \
	cui/uiconfig/ui/customizedialog \
	cui/uiconfig/ui/dbregisterpage \
	cui/uiconfig/ui/effectspage \
	cui/uiconfig/ui/eventsconfigpage \
	cui/uiconfig/ui/formatcellsdialog \
	cui/uiconfig/ui/fontfeaturesdialog \
	cui/uiconfig/ui/galleryapplyprogress \
	cui/uiconfig/ui/galleryfilespage \
	cui/uiconfig/ui/gallerygeneralpage \
	cui/uiconfig/ui/gallerysearchprogress \
	cui/uiconfig/ui/gallerythemedialog \
	cui/uiconfig/ui/gallerythemeiddialog \
	cui/uiconfig/ui/gallerytitledialog \
	cui/uiconfig/ui/galleryupdateprogress \
	cui/uiconfig/ui/graphictestdlg \
	cui/uiconfig/ui/graphictestentry \
	cui/uiconfig/ui/imageviewer \
	cui/uiconfig/ui/hangulhanjaadddialog \
	cui/uiconfig/ui/hangulhanjaeditdictdialog \
	cui/uiconfig/ui/hangulhanjaconversiondialog \
	cui/uiconfig/ui/hangulhanjaoptdialog \
	cui/uiconfig/ui/hatchpage \
	cui/uiconfig/ui/hyperlinkdialog \
	cui/uiconfig/ui/hyperlinkdocpage \
	cui/uiconfig/ui/hyperlinkinternetpage \
	cui/uiconfig/ui/hyperlinkmarkdialog \
	cui/uiconfig/ui/hyperlinkmailpage \
	cui/uiconfig/ui/hyperlinknewdocpage \
	cui/uiconfig/ui/hyphenate \
	cui/uiconfig/ui/iconchangedialog \
	cui/uiconfig/ui/iconselectordialog \
	cui/uiconfig/ui/insertfloatingframe \
	cui/uiconfig/ui/insertoleobject \
	cui/uiconfig/ui/insertrowcolumn \
	cui/uiconfig/ui/javaclasspathdialog \
	cui/uiconfig/ui/javastartparametersdialog \
	cui/uiconfig/ui/linedialog \
	cui/uiconfig/ui/linetabpage \
	cui/uiconfig/ui/lineendstabpage \
	cui/uiconfig/ui/linestyletabpage \
	cui/uiconfig/ui/macroassigndialog \
	cui/uiconfig/ui/macroassignpage \
	cui/uiconfig/ui/macroselectordialog \
	cui/uiconfig/ui/menuassignpage \
	cui/uiconfig/ui/mosaicdialog \
	cui/uiconfig/ui/movemenu \
	cui/uiconfig/ui/multipathdialog \
	cui/uiconfig/ui/namedialog \
	cui/uiconfig/ui/newlibdialog \
	cui/uiconfig/ui/newtabledialog \
	cui/uiconfig/ui/newtoolbardialog \
	cui/uiconfig/ui/numberingformatpage \
	cui/uiconfig/ui/numberingoptionspage \
	cui/uiconfig/ui/numberingpositionpage \
	cui/uiconfig/ui/objectnamedialog \
	cui/uiconfig/ui/objecttitledescdialog \
	cui/uiconfig/ui/optaccessibilitypage \
	cui/uiconfig/ui/optadvancedpage \
	cui/uiconfig/ui/optappearancepage \
	cui/uiconfig/ui/optasianpage \
	cui/uiconfig/ui/optbasicidepage \
	cui/uiconfig/ui/optctlpage \
	cui/uiconfig/ui/optchartcolorspage \
	cui/uiconfig/ui/optemailpage \
	cui/uiconfig/ui/optfltrpage \
	cui/uiconfig/ui/optfltrembedpage \
	cui/uiconfig/ui/optfontspage \
	cui/uiconfig/ui/optgeneralpage \
	cui/uiconfig/ui/opthtmlpage \
	cui/uiconfig/ui/optionsdialog \
	cui/uiconfig/ui/optjsearchpage \
	cui/uiconfig/ui/optlanguagespage \
	cui/uiconfig/ui/optlingupage \
	cui/uiconfig/ui/optnewdictionarydialog \
	cui/uiconfig/ui/optonlineupdatepage \
	$(call gb_Helper_optional,OPENCL, \
		cui/uiconfig/ui/optopenclpage) \
	cui/uiconfig/ui/optpathspage \
	cui/uiconfig/ui/optproxypage \
	cui/uiconfig/ui/optsavepage \
	cui/uiconfig/ui/optsecuritypage \
	cui/uiconfig/ui/optuserpage \
	cui/uiconfig/ui/optviewpage \
	cui/uiconfig/ui/pageformatpage \
	cui/uiconfig/ui/paragalignpage \
	cui/uiconfig/ui/paraindentspacing \
	cui/uiconfig/ui/paratabspage \
	cui/uiconfig/ui/password \
	cui/uiconfig/ui/screenshotannotationdialog \
	cui/uiconfig/ui/pastespecial \
	cui/uiconfig/ui/patterntabpage \
	cui/uiconfig/ui/percentdialog \
	cui/uiconfig/ui/personalization_tab \
	cui/uiconfig/ui/pickbulletpage \
	cui/uiconfig/ui/pickgraphicpage \
	cui/uiconfig/ui/picknumberingpage \
	cui/uiconfig/ui/pickoutlinepage \
	cui/uiconfig/ui/positionpage \
	cui/uiconfig/ui/positionsizedialog \
	cui/uiconfig/ui/possizetabpage \
	cui/uiconfig/ui/posterdialog \
	cui/uiconfig/ui/qrcodegen \
	cui/uiconfig/ui/querychangelineenddialog \
	cui/uiconfig/ui/querydeletebitmapdialog \
	cui/uiconfig/ui/querydeletechartcolordialog \
	cui/uiconfig/ui/querydeletecolordialog \
	cui/uiconfig/ui/querydeletedictionarydialog \
	cui/uiconfig/ui/querydeletegradientdialog \
	cui/uiconfig/ui/querydeletehatchdialog \
	cui/uiconfig/ui/querydeletelineenddialog \
	cui/uiconfig/ui/querydeletelinestyledialog \
	cui/uiconfig/ui/queryduplicatedialog \
	cui/uiconfig/ui/querynoloadedfiledialog \
	cui/uiconfig/ui/querynosavefiledialog \
	cui/uiconfig/ui/querysavelistdialog \
	cui/uiconfig/ui/queryupdategalleryfilelistdialog \
	cui/uiconfig/ui/recordnumberdialog \
	cui/uiconfig/ui/rotationtabpage \
	cui/uiconfig/ui/scriptorganizer \
	cui/uiconfig/ui/searchattrdialog \
	cui/uiconfig/ui/searchformatdialog \
	cui/uiconfig/ui/securityoptionsdialog \
	cui/uiconfig/ui/selectpathdialog \
	cui/uiconfig/ui/shadowtabpage \
	cui/uiconfig/ui/showcoldialog \
	cui/uiconfig/ui/similaritysearchdialog \
	cui/uiconfig/ui/signatureline \
	cui/uiconfig/ui/signsignatureline \
	cui/uiconfig/ui/slantcornertabpage \
	cui/uiconfig/ui/smarttagoptionspage \
	cui/uiconfig/ui/smoothdialog \
	cui/uiconfig/ui/solarizedialog \
	cui/uiconfig/ui/specialcharacters \
	cui/uiconfig/ui/spellingdialog \
	cui/uiconfig/ui/spelloptionsdialog \
	cui/uiconfig/ui/splitcellsdialog \
	cui/uiconfig/ui/storedwebconnectiondialog \
	cui/uiconfig/ui/swpossizepage \
	cui/uiconfig/ui/textattrtabpage \
	cui/uiconfig/ui/textanimtabpage \
	cui/uiconfig/ui/textcolumnstabpage \
	cui/uiconfig/ui/textdialog \
	cui/uiconfig/ui/textflowpage \
	cui/uiconfig/ui/thesaurus \
	cui/uiconfig/ui/toolbarmodedialog \
	cui/uiconfig/ui/transparencytabpage \
	cui/uiconfig/ui/tsaurldialog \
	cui/uiconfig/ui/twolinespage \
	cui/uiconfig/ui/wordcompletionpage \
	cui/uiconfig/ui/spinbox \
	cui/uiconfig/ui/zoomdialog \
))

# vim: set noet sw=4 ts=4:
