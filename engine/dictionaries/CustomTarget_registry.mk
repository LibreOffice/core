# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,dictionaries/registry))

dictionaries_REGISTRY_DIR := $(gb_CustomTarget_workdir)/dictionaries/registry
dictionaries_REGISTRY_MAKER := $(SRCDIR)/dictionaries/util/mkregistry.py

# Every dictionaries.xcu in the tree, so the layer is rebuilt when any of
# them changes. Which of them it carries is decided by gb_Dictionary_ALL
# when the recipe runs, and that is only the ones this configuration builds.
$(dictionaries_REGISTRY_DIR)/dictionaries.xcd : \
		$(dictionaries_REGISTRY_MAKER) \
		$(wildcard $(SRCDIR)/dictionaries/*/dictionaries.xcu) \
		$(wildcard $(SRCDIR)/dictionaries/*/Linguistic.xcu) \
		| $(dictionaries_REGISTRY_DIR)/.dir \
		$(call gb_ExternalExecutable_get_dependencies,python)
	$(call gb_Output_announce,dictionaries.xcd,$(true),XCD,1)
	$(call gb_Helper_abbreviate_dirs, \
		$(call gb_ExternalExecutable_get_command,python) $(dictionaries_REGISTRY_MAKER) \
			$(SRCDIR) $(gb_Dictionary_INSTDIR) $@ $(gb_Dictionary_ALL))

# vim: set noet sw=4 ts=4:
