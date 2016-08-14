# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,svx))

$(eval $(call gb_UIConfig_add_uifiles,svx,\
	svx/uiconfig/ui/acceptrejectchangesdialog \
	svx/uiconfig/ui/addconditiondialog \
	svx/uiconfig/ui/adddataitemdialog \
	svx/uiconfig/ui/addinstancedialog \
	svx/uiconfig/ui/addmodeldialog \
	svx/uiconfig/ui/addnamespacedialog \
	svx/uiconfig/ui/addsubmissiondialog \
	svx/uiconfig/ui/asianphoneticguidedialog \
	svx/uiconfig/ui/chineseconversiondialog \
	svx/uiconfig/ui/chinesedictionary \
	svx/uiconfig/ui/colorwindow \
	svx/uiconfig/ui/compressgraphicdialog \
	svx/uiconfig/ui/crashreportdlg \
	svx/uiconfig/ui/datanavigator \
	svx/uiconfig/ui/defaultshapespanel \
	svx/uiconfig/ui/deleteheaderdialog \
	svx/uiconfig/ui/deletefooterdialog \
	svx/uiconfig/ui/docking3deffects \
	svx/uiconfig/ui/dockingcolorreplace \
	svx/uiconfig/ui/dockingfontwork \
	svx/uiconfig/ui/docrecoverybrokendialog \
	svx/uiconfig/ui/docrecoveryprogressdialog \
	svx/uiconfig/ui/docrecoveryrecoverdialog \
	svx/uiconfig/ui/docrecoverysavedialog \
	svx/uiconfig/ui/extrustiondepthdialog \
	svx/uiconfig/ui/findreplacedialog \
	svx/uiconfig/ui/floatingcontour \
	svx/uiconfig/ui/floatinglineproperty \
	svx/uiconfig/ui/floatingundoredo \
	svx/uiconfig/ui/fontworkgallerydialog \
	svx/uiconfig/ui/fontworkspacingdialog \
	svx/uiconfig/ui/formlinkwarndialog \
	svx/uiconfig/ui/headfootformatpage \
	svx/uiconfig/ui/imapdialog \
	svx/uiconfig/ui/linkwarndialog \
	svx/uiconfig/ui/mediaplayback \
	svx/uiconfig/ui/namespacedialog \
	svx/uiconfig/ui/optgridpage \
	svx/uiconfig/ui/paralinespacingcontrol \
	svx/uiconfig/ui/paralrspacing \
	svx/uiconfig/ui/paraulspacing \
	svx/uiconfig/ui/passwd \
	svx/uiconfig/ui/querydeletecontourdialog \
	svx/uiconfig/ui/querydeleteobjectdialog \
	svx/uiconfig/ui/querydeletethemedialog \
	svx/uiconfig/ui/querymodifyimagemapchangesdialog \
	svx/uiconfig/ui/querynewcontourdialog \
	svx/uiconfig/ui/querysavecontchangesdialog \
	svx/uiconfig/ui/querysaveimagemapchangesdialog \
	svx/uiconfig/ui/queryunlinkgraphicsdialog \
	svx/uiconfig/ui/redlinecontrol \
	svx/uiconfig/ui/redlinefilterpage \
	svx/uiconfig/ui/redlineviewpage \
	svx/uiconfig/ui/savemodifieddialog \
	svx/uiconfig/ui/sidebararea \
	svx/uiconfig/ui/sidebarshadow \
	svx/uiconfig/ui/sidebargraphic \
	svx/uiconfig/ui/sidebarline \
	svx/uiconfig/ui/sidebarparagraph \
	svx/uiconfig/ui/sidebarpossize \
	svx/uiconfig/ui/sidebarstylespanel \
	svx/uiconfig/ui/sidebartextpanel \
	svx/uiconfig/ui/textcharacterspacingcontrol \
	svx/uiconfig/ui/textcontrolchardialog \
	svx/uiconfig/ui/textcontrolparadialog \
	svx/uiconfig/ui/textunderlinecontrol \
	svx/uiconfig/ui/xformspage \
))

# vim: set noet sw=4 ts=4:
