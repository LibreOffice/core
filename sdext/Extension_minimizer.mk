# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Extension_Extension,presentation-minimizer,sdext/source/minimizer))

$(eval $(call gb_Extension_use_default_description,presentation-minimizer))
$(eval $(call gb_Extension_use_default_license,presentation-minimizer))

$(eval $(call gb_Extension_add_libraries,presentation-minimizer,\
    SunPresentationMinimizer \
))

$(eval $(call gb_Extension_add_file,presentation-minimizer,components.rdb,$(call gb_Rdb_get_target,minimizer)))

$(eval $(call gb_Extension_add_files,presentation-minimizer,bitmaps,\
    $(SRCDIR)/icon-themes/galaxy/desktop/res/extension_32.png \
    $(SRCDIR)/icon-themes/galaxy/minimizer/minimizepresi_80.png \
    $(SRCDIR)/icon-themes/galaxy/minimizer/opt_16.png \
    $(SRCDIR)/icon-themes/galaxy/minimizer/opt_26.png \
))

$(eval $(call gb_Extension_add_file,presentation-minimizer,registry/schema/org/openoffice/Office/extension/SunPresentationMinimizer.xcs,$(call gb_XcsTarget_get_target,sdext/source/minimizer/registry/schema/org/openoffice/Office/extension/SunPresentationMinimizer.xcs)))

$(eval $(call gb_Extension_add_files,presentation-minimizer,registry/data/org/openoffice/Office,\
    $(call gb_XcuFile_for_extension,sdext/source/minimizer/registry/data/org/openoffice/Office/Addons.xcu) \
    $(call gb_XcuDataTarget_get_target,sdext/source/minimizer/registry/data/org/openoffice/Office/ProtocolHandler.xcu) \
))

$(eval $(call gb_Extension_add_file,presentation-minimizer,registry/data/org/openoffice/Office/extension/SunPresentationMinimizer.xcu,$(call gb_XcuFile_for_extension,sdext/source/minimizer/registry/data/org/openoffice/Office/extension/SunPresentationMinimizer.xcu)))

# vim:set noet sw=4 ts=4:
