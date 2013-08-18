# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,librelogo,$(SRCDIR)/librelogo/source))

$(eval $(call gb_Package_set_outdir,librelogo,$(INSTDIR)))

$(eval $(call gb_Package_add_files,librelogo,$(LIBO_SHARE_FOLDER)/Scripts/python/LibreLogo,\
    LibreLogo/LibreLogo.py \
    pythonpath/LibreLogo_en_US.properties \
))

$(eval $(call gb_Package_add_files,librelogo,$(LIBO_SHARE_FOLDER)/Scripts/python/LibreLogo/icons,\
    icons/lc_arrowshapes.circular-arrow.png \
    icons/lc_arrowshapes.circular-leftarrow.png \
    icons/lc_arrowshapes.down-arrow.png \
    icons/lc_arrowshapes.up-arrow.png \
    icons/lc_basicstop.png \
    icons/lc_editglossary.png \
    icons/lc_navigationbarleft.png \
    icons/lc_newdoc.png \
    icons/lc_runbasic.png \
    icons/sc_arrowshapes.circular-arrow.png \
    icons/sc_arrowshapes.circular-leftarrow.png \
    icons/sc_arrowshapes.down-arrow.png \
    icons/sc_arrowshapes.up-arrow.png \
    icons/sc_basicstop.png \
    icons/sc_editglossary.png \
    icons/sc_navigationbarleft.png \
    icons/sc_newdoc.png \
    icons/sc_runbasic.png \
))

# vim:set noet sw=4 ts=4:
