# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# for VERSION
include $(SRCDIR)/jfreereport/version.mk

$(eval $(call gb_ExternalPackage_ExternalPackage,jfreereport_flow_engine,jfreereport_flow_engine))

$(eval $(call gb_ExternalPackage_use_external_project,jfreereport_flow_engine,jfreereport_flow_engine))

$(eval $(call gb_ExternalPackage_add_jar_for_install,jfreereport_flow_engine,bin/flow-engine-$(FLOW_ENGINE_VERSION).jar,build/lib/flow-engine.jar))

# vim: set noet sw=4 ts=4:
