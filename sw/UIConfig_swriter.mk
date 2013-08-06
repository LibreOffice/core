# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,modules/swriter))

$(eval $(call gb_UIConfig_add_menubarfiles,modules/swriter,\
	sw/uiconfig/swriter/menubar/menubar \
))

$(eval $(call gb_UIConfig_add_statusbarfiles,modules/swriter,\
	sw/uiconfig/swriter/statusbar/statusbar \
))

$(eval $(call gb_UIConfig_add_toolbarfiles,modules/swriter,\
	sw/uiconfig/swriter/toolbar/alignmentbar \
	sw/uiconfig/swriter/toolbar/arrowshapes \
	sw/uiconfig/swriter/toolbar/basicshapes \
	sw/uiconfig/swriter/toolbar/bezierobjectbar \
	sw/uiconfig/swriter/toolbar/calloutshapes \
	sw/uiconfig/swriter/toolbar/colorbar \
	sw/uiconfig/swriter/toolbar/drawbar \
	sw/uiconfig/swriter/toolbar/drawingobjectbar \
	sw/uiconfig/swriter/toolbar/drawtextobjectbar \
	sw/uiconfig/swriter/toolbar/extrusionobjectbar \
	sw/uiconfig/swriter/toolbar/findbar \
	sw/uiconfig/swriter/toolbar/flowchartshapes \
	sw/uiconfig/swriter/toolbar/fontworkobjectbar \
	sw/uiconfig/swriter/toolbar/fontworkshapetype \
	sw/uiconfig/swriter/toolbar/formcontrols \
	sw/uiconfig/swriter/toolbar/formdesign \
	sw/uiconfig/swriter/toolbar/formsfilterbar \
	sw/uiconfig/swriter/toolbar/formsnavigationbar \
	sw/uiconfig/swriter/toolbar/formtextobjectbar \
	sw/uiconfig/swriter/toolbar/frameobjectbar \
	sw/uiconfig/swriter/toolbar/fullscreenbar \
	sw/uiconfig/swriter/toolbar/graffilterbar \
	sw/uiconfig/swriter/toolbar/graphicobjectbar \
	sw/uiconfig/swriter/toolbar/insertbar \
	sw/uiconfig/swriter/toolbar/mediaobjectbar \
	sw/uiconfig/swriter/toolbar/moreformcontrols \
	sw/uiconfig/swriter/toolbar/navigationobjectbar \
	sw/uiconfig/swriter/toolbar/numobjectbar \
	sw/uiconfig/swriter/toolbar/oleobjectbar \
	sw/uiconfig/swriter/toolbar/optimizetablebar \
	sw/uiconfig/swriter/toolbar/previewobjectbar \
	sw/uiconfig/swriter/toolbar/standardbar \
	sw/uiconfig/swriter/toolbar/starshapes \
	sw/uiconfig/swriter/toolbar/symbolshapes \
	sw/uiconfig/swriter/toolbar/tableobjectbar \
	sw/uiconfig/swriter/toolbar/textobjectbar \
	sw/uiconfig/swriter/toolbar/toolbar \
	sw/uiconfig/swriter/toolbar/viewerbar \
))

$(eval $(call gb_UIConfig_add_uifiles,modules/swriter,\
	sw/uiconfig/swriter/ui/abstractdialog \
	sw/uiconfig/swriter/ui/asciifilterdialog \
	sw/uiconfig/swriter/ui/authenticationsettingsdialog \
	sw/uiconfig/swriter/ui/autoformattable \
	sw/uiconfig/swriter/ui/autotext \
	sw/uiconfig/swriter/ui/bibliographyentry \
	sw/uiconfig/swriter/ui/bulletsandnumbering \
	sw/uiconfig/swriter/ui/cannotsavelabeldialog \
	sw/uiconfig/swriter/ui/captionoptions \
	sw/uiconfig/swriter/ui/characterproperties \
	sw/uiconfig/swriter/ui/charurlpage \
	sw/uiconfig/swriter/ui/columndialog \
	sw/uiconfig/swriter/ui/columnpage \
	sw/uiconfig/swriter/ui/columnwidth \
	sw/uiconfig/swriter/ui/converttexttable \
	sw/uiconfig/swriter/ui/createauthorentry \
	sw/uiconfig/swriter/ui/createautomarkdialog \
	sw/uiconfig/swriter/ui/datasourcesunavailabledialog \
	sw/uiconfig/swriter/ui/dropcapspage \
	sw/uiconfig/swriter/ui/dropdownfielddialog \
	sw/uiconfig/swriter/ui/endnotepage \
	sw/uiconfig/swriter/ui/editcategories \
	sw/uiconfig/swriter/ui/editsectiondialog \
	sw/uiconfig/swriter/ui/exchangedatabases \
	sw/uiconfig/swriter/ui/formatsectiondialog \
	sw/uiconfig/swriter/ui/formattablepage \
	sw/uiconfig/swriter/ui/footendnotedialog \
	sw/uiconfig/swriter/ui/footnotepage \
	sw/uiconfig/swriter/ui/footnoteareapage \
	sw/uiconfig/swriter/ui/footnotesendnotestabpage \
	sw/uiconfig/swriter/ui/frmaddpage \
	sw/uiconfig/swriter/ui/frmurlpage \
	sw/uiconfig/swriter/ui/indexentry \
	sw/uiconfig/swriter/ui/inputfielddialog \
	sw/uiconfig/swriter/ui/indentpage \
	sw/uiconfig/swriter/ui/insertautotextdialog \
	sw/uiconfig/swriter/ui/insertbookmark \
	sw/uiconfig/swriter/ui/insertbreak \
	sw/uiconfig/swriter/ui/insertcaption \
	sw/uiconfig/swriter/ui/insertfootnote \
	sw/uiconfig/swriter/ui/insertsectiondialog \
	sw/uiconfig/swriter/ui/insertscript \
	sw/uiconfig/swriter/ui/inserttable \
	sw/uiconfig/swriter/ui/labeloptionspage \
	sw/uiconfig/swriter/ui/linenumbering \
	sw/uiconfig/swriter/ui/mailconfigpage \
	sw/uiconfig/swriter/ui/mailmergedialog \
	sw/uiconfig/swriter/ui/mergetabledialog \
	sw/uiconfig/swriter/ui/newuserindexdialog \
	sw/uiconfig/swriter/ui/numparapage \
	sw/uiconfig/swriter/ui/numberingnamedialog \
	sw/uiconfig/swriter/ui/optcomparison \
	sw/uiconfig/swriter/ui/optfonttabpage \
	sw/uiconfig/swriter/ui/optformataidspage \
	sw/uiconfig/swriter/ui/optgeneralpage \
    sw/uiconfig/swriter/ui/optredlinepage \
	sw/uiconfig/swriter/ui/opttablepage \
	sw/uiconfig/swriter/ui/opttestpage \
	sw/uiconfig/swriter/ui/outlinenumbering \
	sw/uiconfig/swriter/ui/outlinenumberingpage \
	sw/uiconfig/swriter/ui/outlinepositionpage \
	sw/uiconfig/swriter/ui/querydefaultcompatdialog \
	sw/uiconfig/swriter/ui/querysavelabeldialog \
	sw/uiconfig/swriter/ui/queryshowchangesdialog \
	sw/uiconfig/swriter/ui/paradialog \
	sw/uiconfig/swriter/ui/previewzoomdialog \
	sw/uiconfig/swriter/ui/printoptionspage \
	sw/uiconfig/swriter/ui/printeroptions \
	sw/uiconfig/swriter/ui/renameautotextdialog \
	sw/uiconfig/swriter/ui/renameobjectdialog \
	sw/uiconfig/swriter/ui/rowheight \
	sw/uiconfig/swriter/ui/saveashtmldialog \
	sw/uiconfig/swriter/ui/savelabeldialog \
	sw/uiconfig/swriter/ui/sectionpage \
	sw/uiconfig/swriter/ui/sidebarpage \
	sw/uiconfig/swriter/ui/sidebarwrap \
	sw/uiconfig/swriter/ui/sortdialog \
	sw/uiconfig/swriter/ui/splittable \
	sw/uiconfig/swriter/ui/statisticsinfopage \
	sw/uiconfig/swriter/ui/stringinput \
	sw/uiconfig/swriter/ui/tablecolumnpage \
	sw/uiconfig/swriter/ui/tableproperties \
	sw/uiconfig/swriter/ui/tabletextflowpage \
	sw/uiconfig/swriter/ui/textgridpage \
	sw/uiconfig/swriter/ui/titlepage \
	sw/uiconfig/swriter/ui/viewoptionspage \
	sw/uiconfig/swriter/ui/wordcount \
	sw/uiconfig/swriter/ui/wrappage \
))

# vim: set noet sw=4 ts=4:
