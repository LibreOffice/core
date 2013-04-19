# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Configuration_Configuration,presentation-minimizer,nodeliver))

$(eval $(call gb_Configuration_add_schemas,presentation-minimizer,sdext/source/minimizer/registry/schema,\
    org/openoffice/Office/extension/SunPresentationMinimizer.xcs \
))

$(eval $(call gb_Configuration_add_datas,presentation-minimizer,sdext/source/minimizer/registry/data,\
    org/openoffice/Office/ProtocolHandler.xcu \
))

$(eval $(call gb_Configuration_add_localized_datas,presentation-minimizer,sdext/source/minimizer/registry/data,\
    org/openoffice/Office/Addons.xcu \
    org/openoffice/Office/extension/SunPresentationMinimizer.xcu \
))

# vim:set noet sw=4 ts=4:
