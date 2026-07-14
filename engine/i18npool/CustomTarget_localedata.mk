# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,i18npool/localedata))

i18npool_LD_NAMES := $(basename $(notdir $(wildcard $(SRCDIR)/i18npool/source/localedata/data/*.xml)))

i18npool_LD_DIR := $(gb_CustomTarget_workdir)/i18npool/localedata

$(eval $(call gb_CustomTarget_register_targets,i18npool/localedata,\
	$(foreach name,$(i18npool_LD_NAMES),localedata_$(name).cxx) \
))

$(i18npool_LD_DIR)/localedata_%_new.cxx : \
		$(SRCDIR)/i18npool/source/localedata/data/%.xml \
		$(call gb_Executable_get_runtime_dependencies,saxparser)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),SAX,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),SAX)
	$(call gb_Helper_abbreviate_dirs, \
		$(call gb_Helper_print_on_error, \
			$(call gb_Helper_execute,saxparser) $* $< $@ \
				-env:LO_LIB_DIR=$(call gb_Helper_make_url,$(INSTROOT_FOR_BUILD)/$(LIBO_LIB_FOLDER)) \
				-env:URE_MORE_SERVICES=$(call gb_Helper_make_url,$(call gb_Rdb_get_target_for_build,saxparser))))
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),SAX)

.PRECIOUS: $(i18npool_LD_DIR)/localedata_%_new.cxx

# saxparser writes a fresh localedata_*_new.cxx whenever it (or its input .xml)
# is newer; we copy it onto the compiled localedata_*.cxx only when the content
# actually differs, so an unchanged regeneration (e.g. after saxparser was just
# relinked) does not force a recompile of the hundreds of large localedata
# objects.
#
# The copy is done for every locale in a single recipe gated on a stamp file,
# rather than by a per-locale rule. A per-locale copy rule leaves each target
# older than its _new prerequisite whenever the content matched (nothing was
# copied, so the target's timestamp never caught up), which made make re-run the
# comparison for all locales on every subsequent build. On Windows, where each
# process spawn costs tens of milliseconds, those hundreds of shell+cmp spawns
# dominated the incremental no-op build; on Linux they were unnoticeable. The
# stamp is touched after each run, so once it is up to date the recipe does not
# run again until a _new.cxx genuinely changes, and only the names are passed on
# the command line (the paths are rebuilt in the shell) to stay within the
# Windows command-line length limit.
$(i18npool_LD_DIR)/localedata.copied : \
		$(foreach name,$(i18npool_LD_NAMES),$(i18npool_LD_DIR)/localedata_$(name)_new.cxx) \
		| $(i18npool_LD_DIR)/.dir
	$(call gb_Helper_abbreviate_dirs, \
		for name in $(i18npool_LD_NAMES); do \
			new=$(i18npool_LD_DIR)/localedata_$${name}_new.cxx; \
			cxx=$(i18npool_LD_DIR)/localedata_$${name}.cxx; \
			cmp -s $$new $$cxx || cp $$new $$cxx || exit 1; \
		done)
	touch $@

# Connect the copy step into the graph and guarantee it runs before any
# localedata_*.cxx is compiled. Order-only so the stamp's timestamp never marks
# the (unchanged) sources out of date - only a real content change, which
# advances the individual localedata_*.cxx timestamp, triggers a recompile.
$(foreach name,$(i18npool_LD_NAMES),$(i18npool_LD_DIR)/localedata_$(name).cxx) :| \
	$(i18npool_LD_DIR)/localedata.copied

# vim: set noet sw=4 ts=4:
