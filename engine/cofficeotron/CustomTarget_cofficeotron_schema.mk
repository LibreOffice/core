# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,cofficeotron/schema))

cofficeotron_schema_DIR := $(gb_CustomTarget_workdir)/cofficeotron/schema

# Assembles the schema directory the validator reads: the ODF schemas
# from the source tree, and the ECMA-376 schemas unpacked and patched
# from the downloaded standard archives.
$(call gb_CustomTarget_get_target,cofficeotron/schema): \
		$(call gb_UnpackedTarball_get_target,ecma376_opc) \
		$(call gb_UnpackedTarball_get_target,ecma376_transitional) \
		$(wildcard $(SRCDIR)/cofficeotron/etc/schema/odf/*/*)
	$(call gb_Output_announce,cofficeotron/schema,$(true),CUS,1)
	rm -rf $(cofficeotron_schema_DIR)
	mkdir -p $(cofficeotron_schema_DIR)/29500T $(cofficeotron_schema_DIR)/OPC
	cp -r $(SRCDIR)/cofficeotron/etc/schema/odf $(cofficeotron_schema_DIR)/odf
	cp $(gb_UnpackedTarball_workdir)/ecma376_transitional/*.rnc $(cofficeotron_schema_DIR)/29500T/
	cp $(gb_UnpackedTarball_workdir)/ecma376_opc/*.rnc $(cofficeotron_schema_DIR)/OPC/
	mkdir -p $(dir $@) && touch $@

# vim: set noet sw=4 ts=4:
