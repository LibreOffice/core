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
	svx/uiconfig/ui/absrecbox \
	svx/uiconfig/ui/acceptrejectchangesdialog \
	svx/uiconfig/ui/accessibilitycheckdialog \
	svx/uiconfig/ui/accessibilitycheckentry \
	svx/uiconfig/ui/addconditiondialog \
	svx/uiconfig/ui/adddataitemdialog \
	svx/uiconfig/ui/addinstancedialog \
	svx/uiconfig/ui/addmodeldialog \
	svx/uiconfig/ui/addnamespacedialog \
	svx/uiconfig/ui/addsubmissiondialog \
	svx/uiconfig/ui/asianphoneticguidedialog \
	svx/uiconfig/ui/applystylebox \
	svx/uiconfig/ui/cellmenu \
	svx/uiconfig/ui/charsetmenu \
	svx/uiconfig/ui/checkbuttonbox \
	svx/uiconfig/ui/chineseconversiondialog \
	svx/uiconfig/ui/chinesedictionary \
	svx/uiconfig/ui/classificationdialog \
	svx/uiconfig/ui/clipboardmenu \
	svx/uiconfig/ui/colorwindow \
	svx/uiconfig/ui/currencywindow \
	svx/uiconfig/ui/colsmenu \
	svx/uiconfig/ui/columnswindow \
	svx/uiconfig/ui/compressgraphicdialog \
	svx/uiconfig/ui/convertmenu \
	svx/uiconfig/ui/crashreportdlg \
	svx/uiconfig/ui/datanavigator \
	svx/uiconfig/ui/defaultshapespanel \
	svx/uiconfig/ui/deleteheaderdialog \
	svx/uiconfig/ui/deletefooterdialog \
	svx/uiconfig/ui/depthwindow \
	svx/uiconfig/ui/directionwindow \
	svx/uiconfig/ui/docking3deffects \
	svx/uiconfig/ui/dockingcolorreplace \
	svx/uiconfig/ui/dockingcolorwindow \
	svx/uiconfig/ui/dockingfontwork \
	svx/uiconfig/ui/docrecoverybrokendialog \
	svx/uiconfig/ui/docrecoveryprogressdialog \
	svx/uiconfig/ui/docrecoveryrecoverdialog \
	svx/uiconfig/ui/docrecoverysavedialog \
	svx/uiconfig/ui/extrustiondepthdialog \
	svx/uiconfig/ui/fillctrlbox \
	svx/uiconfig/ui/filtermenu \
	svx/uiconfig/ui/filternavigator \
	svx/uiconfig/ui/findbox \
	svx/uiconfig/ui/findreplacedialog \
	svx/uiconfig/ui/findreplacedialog-mobile \
	svx/uiconfig/ui/floatingareastyle \
	svx/uiconfig/ui/floatingcontour \
	svx/uiconfig/ui/floatingframeborder \
	svx/uiconfig/ui/floatinglineend \
	svx/uiconfig/ui/floatinglineproperty \
	svx/uiconfig/ui/floatinglinestyle \
	svx/uiconfig/ui/floatingundoredo \
	svx/uiconfig/ui/fontworkalignmentcontrol \
	svx/uiconfig/ui/fontworkcharacterspacingcontrol \
	svx/uiconfig/ui/fontworkgallerydialog \
	svx/uiconfig/ui/fontworkspacingdialog \
	svx/uiconfig/ui/fontsizebox \
	svx/uiconfig/ui/fontnamebox \
	svx/uiconfig/ui/formdatamenu \
	svx/uiconfig/ui/formfielddialog \
	svx/uiconfig/ui/formlinkwarndialog \
	svx/uiconfig/ui/formnavigator \
	svx/uiconfig/ui/formnavimenu \
	svx/uiconfig/ui/formpropertydialog \
	svx/uiconfig/ui/functionmenu \
	svx/uiconfig/ui/gallerymenu1 \
	svx/uiconfig/ui/gallerymenu2 \
	svx/uiconfig/ui/grafctrlbox \
	svx/uiconfig/ui/grafmodebox \
	svx/uiconfig/ui/headfootformatpage \
	svx/uiconfig/ui/imapdialog \
	svx/uiconfig/ui/imapmenu \
	svx/uiconfig/ui/inspectortextpanel \
	svx/uiconfig/ui/labelbox \
	svx/uiconfig/ui/lightingwindow \
	svx/uiconfig/ui/linkwarndialog \
	svx/uiconfig/ui/measurewidthbar \
	svx/uiconfig/ui/medialine \
	svx/uiconfig/ui/mediaplayback \
	svx/uiconfig/ui/mediawindow \
	svx/uiconfig/ui/metricfieldbox \
	svx/uiconfig/ui/namespacedialog \
	svx/uiconfig/ui/navigationbar \
	svx/uiconfig/ui/numberingwindow \
	svx/uiconfig/ui/optgridpage \
	svx/uiconfig/ui/paralinespacingcontrol \
	svx/uiconfig/ui/paralrspacing \
	svx/uiconfig/ui/paraulspacing \
	svx/uiconfig/ui/passwd \
	svx/uiconfig/ui/presetmenu \
	svx/uiconfig/ui/profileexporteddialog \
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
	svx/uiconfig/ui/rowsmenu \
	svx/uiconfig/ui/rulermenu \
	svx/uiconfig/ui/safemodedialog \
	svx/uiconfig/ui/savemodifieddialog \
	svx/uiconfig/ui/selectionmenu \
	svx/uiconfig/ui/sidebararea \
	svx/uiconfig/ui/sidebarempty \
	svx/uiconfig/ui/sidebareffect \
	svx/uiconfig/ui/sidebarshadow \
	svx/uiconfig/ui/sidebargallery \
	svx/uiconfig/ui/sidebargraphic \
	svx/uiconfig/ui/sidebarline \
	svx/uiconfig/ui/sidebarparagraph \
	svx/uiconfig/ui/sidebarlists \
	svx/uiconfig/ui/sidebarpossize \
	svx/uiconfig/ui/sidebarstylespanel \
	svx/uiconfig/ui/sidebartextpanel \
	svx/uiconfig/ui/stylemenu \
	svx/uiconfig/ui/surfacewindow \
	svx/uiconfig/ui/tablewindow \
	svx/uiconfig/ui/stylespreview \
	svx/uiconfig/ui/textcharacterspacingcontrol \
	svx/uiconfig/ui/textcontrolchardialog \
	svx/uiconfig/ui/textcontrolparadialog \
	svx/uiconfig/ui/textunderlinecontrol \
	svx/uiconfig/ui/toolbarpopover \
	svx/uiconfig/ui/xmlsecstatmenu \
	svx/uiconfig/ui/xformspage \
	svx/uiconfig/ui/zoommenu \
))

# vim: set noet sw=4 ts=4:
