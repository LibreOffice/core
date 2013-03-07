# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Extension_Extension,test-passive,desktop/test/deployment/passive))

$(eval $(call gb_Extension_add_file,test-passive,generic.components,$(call gb_Rdb_get_target,passive_generic)))

$(eval $(call gb_Extension_add_file,test-passive,platform.components,$(call gb_Rdb_get_target,passive_platform)))

$(eval $(call gb_Extension_add_files,test-passive,, \
    $(SRCDIR)/desktop/test/deployment/passive/Addons.xcu \
    $(SRCDIR)/desktop/test/deployment/passive/ProtocolHandler.xcu \
    $(SRCDIR)/desktop/test/deployment/passive/passive_python.py \
    $(call gb_Jar_get_outdir_target,passive_java) \
))

$(eval $(call gb_Extension_add_libraries,test-passive, \
    passive_native \
))

# vim: set noet sw=4 ts=4:
