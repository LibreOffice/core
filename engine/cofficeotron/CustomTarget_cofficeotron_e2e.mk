# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,cofficeotron/e2e))

# Compares the validator's output over the bundled test documents with
# the expected copies in tests/expected.
$(call gb_CustomTarget_get_target,cofficeotron/e2e): \
		$(call gb_Executable_get_target,cofficeotron) \
		$(call gb_CustomTarget_get_target,cofficeotron/schema) \
		$(SRCDIR)/cofficeotron/tests/run-e2e.sh \
		$(wildcard $(SRCDIR)/cofficeotron/tests/expected/*.txt) \
		$(wildcard $(SRCDIR)/cofficeotron/etc/test-data/*)
	$(call gb_Output_announce,cofficeotron/e2e,$(true),CHK,1)
	$(gb_Helper_set_ld_path) \
		COFFICEOTRON=$(call gb_Executable_get_target,cofficeotron) \
		COFFICEOTRON_SCHEMA_DIR=$(gb_CustomTarget_workdir)/cofficeotron/schema \
		$(SRCDIR)/cofficeotron/tests/run-e2e.sh
	mkdir -p $(dir $@) && touch $@

# vim: set noet sw=4 ts=4:
