# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,jfreereport_flow_engine))

$(eval $(call gb_UnpackedTarball_set_tarball,jfreereport_flow_engine,$(JFREEREPORT_FLOW_ENGINE_TARBALL),0))

$(eval $(call gb_UnpackedTarball_fix_end_of_line,jfreereport_flow_engine,\
	build.xml \
))

$(eval $(call gb_UnpackedTarball_add_patches,jfreereport_flow_engine,\
	external/jfreereport/patches/flow-engine.patch \
	external/jfreereport/patches/flow-engine_date_is_datetime.patch.1 \
	external/jfreereport/patches/pentaho-reporting-flow-engine-0.9.4-remove-commons-logging.patch.1 \
))

# vim: set noet sw=4 ts=4:
