# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,deployment))

$(eval $(call gb_AllLangResTarget_set_reslocation,deployment,desktop))

$(eval $(call gb_AllLangResTarget_add_srs,deployment,\
    deployment/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,deployment/res))

$(eval $(call gb_SrsTarget_set_include,deployment/res,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/inc \
    -I$(SRCDIR)/desktop/source/deployment/registry/inc \
    -I$(SRCDIR)/desktop/source/inc \
))

$(eval $(call gb_SrsTarget_add_files,deployment/res,\
    desktop/source/deployment/manager/dp_manager.src \
    desktop/source/deployment/misc/dp_misc.src \
    desktop/source/deployment/registry/component/dp_component.src \
    desktop/source/deployment/registry/configuration/dp_configuration.src \
    desktop/source/deployment/registry/dp_registry.src \
    desktop/source/deployment/registry/help/dp_help.src \
    desktop/source/deployment/registry/package/dp_package.src \
    desktop/source/deployment/registry/script/dp_script.src \
    desktop/source/deployment/registry/sfwk/dp_sfwk.src \
    desktop/source/deployment/unopkg/unopkg.src \
))

# vim: set ts=4 sw=4 et:
