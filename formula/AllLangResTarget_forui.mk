# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,forui))

$(eval $(call gb_AllLangResTarget_set_reslocation,forui,formula))

$(eval $(call gb_AllLangResTarget_add_srs,forui,\
    formula/ui \
))

$(eval $(call gb_SrsTarget_SrsTarget,formula/ui))

$(eval $(call gb_SrsTarget_use_packages,formula/ui,\
))

$(eval $(call gb_SrsTarget_set_include,formula/ui,\
    $$(INCLUDE) \
    -I$(SRCDIR)/formula/inc \
    -I$(SRCDIR)/formula/source/ui/inc \
))

$(eval $(call gb_SrsTarget_add_files,formula/ui,\
    formula/source/ui/dlg/formdlgs.src \
    formula/source/ui/dlg/parawin.src \
))

# vim: set noet sw=4 ts=4:
