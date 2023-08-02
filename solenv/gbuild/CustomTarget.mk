# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

# the .dir is for make 3.81, which ignores trailing /
$(call gb_CustomTarget_get_workdir,%)/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_CustomTarget_get_target,%) :
	$(call gb_Output_announce,$*,$(true),CUS,3)
	$(call gb_Trace_MakeMark,$*,CUS)
	touch $@

.PHONY: $(call gb_CustomTarget_get_clean_target,%)
$(call gb_CustomTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),CUS,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf $(call gb_CustomTarget_get_workdir,$*) && \
		rm -f $(call gb_CustomTarget_get_target,$*))

define gb_CustomTarget_CustomTarget
$(eval $(call gb_Module_register_target,$(call gb_CustomTarget_get_target,$(1)),$(call gb_CustomTarget_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),CustomTarget)
$(call gb_CustomTarget_get_target,$(1)) :| $(dir $(call gb_CustomTarget_get_target,$(1))).dir

endef

define gb_CustomTarget_register_target
$(call gb_CustomTarget_get_target,$(1)) : $(call gb_CustomTarget_get_workdir,$(1))/$(2)
$(call gb_CustomTarget_get_workdir,$(1))/$(2) :| $(dir $(call gb_CustomTarget_get_workdir,$(1))/$(2)).dir

endef

define gb_CustomTarget_register_targets
$(foreach target,$(2),$(call gb_CustomTarget_register_target,$(1),$(target)))

endef

ifneq ($(WITH_LANG),)

# $(call gb_CustomTarget_ulfex__command,ulftarget,ulfsource,pofiles)
define gb_CustomTarget_ulfex__command
$(call gb_Output_announce,$(1),$(true),ULF,1)
	$(call gb_Trace_StartRange,$(1),ULF)
MERGEINPUT=$(call gb_var2file,$(shell $(gb_MKTEMP)),$(3)) && \
$(call gb_Helper_abbreviate_dirs,\
	$(call gb_Executable_get_command,ulfex) -i $(2) -o $(1) -m $${MERGEINPUT} -l all) && \
rm -rf $${MERGEINPUT}
	$(call gb_Trace_EndRange,$(1),ULF)
endef

else

define gb_CustomTarget_ulfex__command
cp $(2) $(1)
endef

endif

# $(call gb_CustomTarget_ulfex_rule,ulftargetpattern,ulfsource,pofiles)
define gb_CustomTarget_ulfex_rule
$(1) : $(2) $(if $(WITH_LANG),$(call gb_Executable_get_runtime_dependencies,ulfex)) | $(dir $(1)).dir
	$$(call gb_CustomTarget_ulfex__command,$$@,$(subst %,$$*,$(strip $(2))),$(strip $(3)))

endef

#$(call gb_CustomTarget_token_hash,oox/generated,tokenhash.inc,tokenhash.gperf)
define gb_CustomTarget_token_hash
$(call gb_CustomTarget_get_target,$(1)) : $(call gb_CustomTarget_get_workdir,$(1))/$(2)
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : $(call gb_CustomTarget_get_workdir,$(1))/misc/$(3)
	$$(call gb_Output_announce,$$(subst $(WORKDIR)/,,$$@),build,GPF,1)
	$$(call gb_Helper_wsl_path,$(GPERF) --compare-strncmp --switch=2 --readonly-tables $$<) \
		| sed -e '/^#line/d' -e 's/(char\*)0/(char\*)0, 0/g' > $$@

endef

#$(call gb_CustomTarget_generate_tokens,oox/generated,oox,oox/source/token,
#namespaces,namespace,namespaces.txt,namespaces-strict,namespaces.pl)
define gb_CustomTarget_generate_tokens
$(call gb_CustomTarget_get_workdir,$(1))/misc/$(5)ids.inc \
$(call gb_CustomTarget_get_workdir,$(1))/$(5)names.inc \
$(if $(6),$(call gb_CustomTarget_get_workdir,$(1))/misc/$(6)) \
$(if $(7),$(call gb_CustomTarget_get_workdir,$(1))/$(7)names.inc) : \
		$(call gb_CustomTarget_get_workdir,$(1))/$(2)/token/$(4).hxx
	touch $$@

$(call gb_CustomTarget_get_workdir,$(1))/$(2)/token/$(4).hxx : \
		$(call gb_ExternalExecutable_get_dependencies,python) \
		$(if $(7),$(SRCDIR)/$(3)/$(7).txt) \
		$(if $(8),$(SRCDIR)/$(3)/$(8),$(SRCDIR)/solenv/bin/generate-tokens.py) \
		$(SRCDIR)/$(3)/$(4).txt \
		$(SRCDIR)/$(3)/$(4).hxx.head \
		$(SRCDIR)/$(3)/$(4).hxx.tail
	$$(call gb_Output_announce,$$(subst $(WORKDIR)/,,$$@),build,PRL,1)
	$$(call gb_Trace_StartRange,$$(subst $(WORKDIR)/,,$$@),PRL)
	mkdir -p $(call gb_CustomTarget_get_workdir,$(1))/misc \
	    	$(call gb_CustomTarget_get_workdir,$(1)) \
		$(call gb_CustomTarget_get_workdir,$(1))/$(2)/token
	$(call gb_ExternalExecutable_get_command,python) $(if $(8),$(SRCDIR)/$(3)/$(8),$(SRCDIR)/solenv/bin/generate-tokens.py) \
	    	$(SRCDIR)/$(3)/$(4).txt \
		$(call gb_CustomTarget_get_workdir,$(1))/misc/$(5)ids.inc \
		$(call gb_CustomTarget_get_workdir,$(1))/$(5)names.inc \
		$(if $(6), $(call gb_CustomTarget_get_workdir,$(1))/misc/$(6)) \
		$(if $(7), $(SRCDIR)/$(3)/$(7).txt \
			$(call gb_CustomTarget_get_workdir,$(1))/$(7)names.inc) \
	&& cat $(SRCDIR)/$(3)/$(4).hxx.head \
		$(call gb_CustomTarget_get_workdir,$(1))/misc/$(5)ids.inc \
		$(SRCDIR)/$(3)/$(4).hxx.tail \
		> $(call gb_CustomTarget_get_workdir,$(1))/$(2)/token/$(4).hxx \
	&& touch $$@
	$$(call gb_Trace_EndRange,$$(subst $(WORKDIR)/,,$$@),PRL)

$(call gb_CustomTarget_get_target,$(1)) : \
    $(call gb_CustomTarget_get_workdir,$(1))/$(5)names.inc \
    $(if $(7),$(call gb_CustomTarget_get_workdir,$(1))/$(7)names.inc) \
    $(call gb_CustomTarget_get_workdir,$(1))/$(2)/token/$(4).hxx \
    $(if $(6),$(call gb_CustomTarget_get_workdir,$(1))/misc/$(6)) \

endef

# vim: set noet sw=4:
