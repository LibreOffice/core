# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,languagetool))

$(eval $(call gb_UnpackedTarball_set_tarball,languagetool,$(LANGUAGETOOL_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,languagetool,3))

$(eval $(call gb_UnpackedTarball_add_patches,languagetool,\
    external/languagetool/JLanguageTool-1.7.0.patch \
    external/languagetool/JLanguageTool-1.4.0-no-hc.patch \
    external/languagetool/MessageBox-apichange.patch \
    external/languagetool/english.ireland.patch \
    external/languagetool/esperanto.territory.patch \
    external/languagetool/french.haiti.patch \
))

# vim: set noet sw=4 ts=4:
