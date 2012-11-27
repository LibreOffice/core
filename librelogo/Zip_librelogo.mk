# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,LibreLogo,$(WORKDIR)/Zip/LibreLogo))

$(eval $(call gb_Zip_add_file,LibreLogo,python/LibreLogo/LibreLogo.py,\
    $(SRCDIR)/librelogo/source/LibreLogo/LibreLogo.py \
))

$(eval $(call gb_Zip_add_file,LibreLogo,python/LibreLogo/LibreLogo_en_US.properties,\
    $(SRCDIR)/librelogo/source/pythonpath/LibreLogo_en_US.properties \
))

librelogo_LANGS := $(subst -,_,$(filter-out qtz,$(filter-out en-US,$(gb_WITH_LANG))))

$(eval $(foreach lang,$(librelogo_LANGS),\
    $(call gb_Zip_add_file,LibreLogo,python/LibreLogo/LibreLogo_$(lang).properties,\
    $(call gb_CustomTarget_get_workdir,librelogo/locproperties)/LibreLogo_$(lang).properties)))

$(eval $(foreach icon,\
    lc_arrowshapes.circular-arrow.png \
    lc_arrowshapes.circular-leftarrow.png \
    lc_arrowshapes.down-arrow.png \
    lc_arrowshapes.up-arrow.png \
    lc_basicstop.png \
    lc_editglossary.png \
    lc_navigationbarleft.png \
    lc_newdoc.png \
    lc_runbasic.png \
    sc_arrowshapes.circular-arrow.png \
    sc_arrowshapes.circular-leftarrow.png \
    sc_arrowshapes.down-arrow.png \
    sc_arrowshapes.up-arrow.png \
    sc_basicstop.png \
    sc_editglossary.png \
    sc_navigationbarleft.png \
    sc_newdoc.png \
    sc_runbasic.png, \
    $(call gb_Zip_add_file,LibreLogo,python/LibreLogo/icons/$(icon),\
    $(SRCDIR)/librelogo/source/icons/$(icon) \
)))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
