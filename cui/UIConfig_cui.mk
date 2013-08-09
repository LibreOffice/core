# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,cui))

$(eval $(call gb_UIConfig_add_uifiles,cui,\
	cui/uiconfig/ui/aboutdialog \
	cui/uiconfig/ui/aboutconfigdialog\
	cui/uiconfig/ui/acorexceptpage \
	cui/uiconfig/ui/acoroptionspage \
	cui/uiconfig/ui/acorreplacepage \
	cui/uiconfig/ui/applyautofmtpage \
	cui/uiconfig/ui/applylocalizedpage \
	cui/uiconfig/ui/areadialog \
	cui/uiconfig/ui/areatabpage \
	cui/uiconfig/ui/asiantypography \
	cui/uiconfig/ui/autocorrectdialog \
	cui/uiconfig/ui/backgroundpage \
	cui/uiconfig/ui/bitmaptabpage \
	cui/uiconfig/ui/borderpage \
	cui/uiconfig/ui/cellalignment \
	cui/uiconfig/ui/charnamepage \
	cui/uiconfig/ui/colorpage \
	cui/uiconfig/ui/comment \
	cui/uiconfig/ui/cuiimapdlg \
	cui/uiconfig/ui/editdictionarydialog \
	cui/uiconfig/ui/formatnumberdialog \
	cui/uiconfig/ui/gradientpage \
	cui/uiconfig/ui/colorconfigwin \
	cui/uiconfig/ui/effectspage \
	cui/uiconfig/ui/hatchpage \
	cui/uiconfig/ui/hyphenate \
	cui/uiconfig/ui/insertfloatingframe \
	cui/uiconfig/ui/insertoleobject \
	cui/uiconfig/ui/insertplugin \
	cui/uiconfig/ui/insertrowcolumn \
	cui/uiconfig/ui/linedialog \
	cui/uiconfig/ui/linetabpage \
	cui/uiconfig/ui/lineendstabpage \
	cui/uiconfig/ui/linestyletabpage \
	cui/uiconfig/ui/macroselectordialog \
	cui/uiconfig/ui/messbox \
	cui/uiconfig/ui/namedialog \
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
	cui/uiconfig/ui/optbrowserpage \
	cui/uiconfig/ui/optctlpage \
	cui/uiconfig/ui/optchartcolorspage \
	cui/uiconfig/ui/optemailpage \
	cui/uiconfig/ui/optfltrpage \
	cui/uiconfig/ui/optfontspage \
	cui/uiconfig/ui/optgeneralpage \
	cui/uiconfig/ui/opthtmlpage \
	cui/uiconfig/ui/optjsearchpage \
	cui/uiconfig/ui/optlanguagespage \
	cui/uiconfig/ui/optmemorypage \
	cui/uiconfig/ui/optnewdictionarydialog \
	cui/uiconfig/ui/optonlineupdatepage \
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
	cui/uiconfig/ui/pastespecial \
	cui/uiconfig/ui/percentdialog \
	cui/uiconfig/ui/personalization_tab \
	cui/uiconfig/ui/pickbulletpage \
	cui/uiconfig/ui/pickgraphicpage \
	cui/uiconfig/ui/picknumberingpage \
	cui/uiconfig/ui/pickoutlinepage \
	cui/uiconfig/ui/positionpage \
	cui/uiconfig/ui/querychangelineenddialog \
	cui/uiconfig/ui/querydeletechartcolordialog \
	cui/uiconfig/ui/querydeletedictionarydialog \
	cui/uiconfig/ui/querydeletelineenddialog \
	cui/uiconfig/ui/querydeletelinestyledialog \
	cui/uiconfig/ui/queryduplicatedialog \
	cui/uiconfig/ui/querynoloadedfiledialog \
	cui/uiconfig/ui/querynosavefiledialog \
	cui/uiconfig/ui/querysavelistdialog \
	cui/uiconfig/ui/scriptorganizer \
	cui/uiconfig/ui/securityoptionsdialog \
	cui/uiconfig/ui/select_persona_dialog \
	cui/uiconfig/ui/shadowtabpage \
	cui/uiconfig/ui/specialcharacters \
	cui/uiconfig/ui/spellingdialog \
	cui/uiconfig/ui/splitcellsdialog \
	cui/uiconfig/ui/storedwebconnectiondialog \
	cui/uiconfig/ui/textflowpage \
	cui/uiconfig/ui/thesaurus \
	cui/uiconfig/ui/transparencytabpage \
	cui/uiconfig/ui/twolinespage \
	cui/uiconfig/ui/zoomdialog \
))

# vim: set noet sw=4 ts=4:
