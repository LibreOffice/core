# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Extension_Extension,librelogo,librelogo/source))

$(eval $(call gb_Extension_add_files,librelogo,icons,\
    $(SRCDIR)/librelogo/source/icons/lc_arrowshapes.circular-arrow.png \
    $(SRCDIR)/librelogo/source/icons/lc_arrowshapes.circular-leftarrow.png \
    $(SRCDIR)/librelogo/source/icons/lc_arrowshapes.down-arrow.png \
    $(SRCDIR)/librelogo/source/icons/lc_arrowshapes.up-arrow.png \
    $(SRCDIR)/librelogo/source/icons/lc_basicstop.png \
    $(SRCDIR)/librelogo/source/icons/lc_editglossary.png \
    $(SRCDIR)/librelogo/source/icons/lc_navigationbarleft.png \
    $(SRCDIR)/librelogo/source/icons/lc_newdoc.png \
    $(SRCDIR)/librelogo/source/icons/lc_runbasic.png \
    $(SRCDIR)/librelogo/source/icons/sc_arrowshapes.circular-arrow.png \
    $(SRCDIR)/librelogo/source/icons/sc_arrowshapes.circular-leftarrow.png \
    $(SRCDIR)/librelogo/source/icons/sc_arrowshapes.down-arrow.png \
    $(SRCDIR)/librelogo/source/icons/sc_arrowshapes.up-arrow.png \
    $(SRCDIR)/librelogo/source/icons/sc_basicstop.png \
    $(SRCDIR)/librelogo/source/icons/sc_editglossary.png \
    $(SRCDIR)/librelogo/source/icons/sc_navigationbarleft.png \
    $(SRCDIR)/librelogo/source/icons/sc_newdoc.png \
    $(SRCDIR)/librelogo/source/icons/sc_runbasic.png \
))

$(eval $(call gb_Extension_add_file,librelogo,LibreLogoDummy.py,$(SRCDIR)/librelogo/source/LibreLogoDummy.py))
$(eval $(call gb_Extension_add_file,librelogo,pythonpath/librelogodummy_path.py,$(SRCDIR)/librelogo/source/pythonpath/librelogodummy_path.py))
$(eval $(call gb_Extension_add_file,librelogo,LibreLogo/LibreLogo.py,$(SRCDIR)/librelogo/source/LibreLogo/LibreLogo.py))
$(eval $(call gb_Extension_add_file,librelogo,Addons.xcu,$(call gb_XcuFile_for_extension,librelogo/source/registry/data/org/openoffice/Office/Addons.xcu)))
$(eval $(call gb_Extension_add_file,librelogo,Office/UI/StartModuleWindowState.xcu,$(call gb_XcuFile_for_extension,librelogo/source/registry/data/org/openoffice/Office/UI/StartModuleWindowState.xcu)))
$(eval $(call gb_Extension_add_file,librelogo,Office/UI/WriterWindowState.xcu,$(call gb_XcuFile_for_extension,librelogo/source/registry/data/org/openoffice/Office/UI/WriterWindowState.xcu)))

$(eval $(call gb_Extension_localize_properties,librelogo,pythonpath/LibreLogo_en_US.properties,$(SRCDIR)/librelogo/source/pythonpath/LibreLogo_en_US.properties))

$(eval $(call gb_Extension_add_helpfile,librelogo,$(SRCDIR)/librelogo/source/help/en-US,org.openoffice.comp.pyuno.LibreLogo/LibreLogo.xhp,LibreLogo.xhp))

$(eval $(call gb_Extension_add_helptreefile,librelogo,$(SRCDIR)/librelogo/source/help/en-US,/help.tree,/help.tree,org.openoffice.comp.pyuno.LibreLogo))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
