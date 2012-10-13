# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
# Norbert Thiebaud <nthiebaud@gmail.com> (C) 2010, All Rights Reserved.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

ifeq ($(gb_FULLDEPS),$(true))
gb_cxx_dep_generation_options=-MMD -MT $(1) -MP -MF $(4)_
gb_cxx_dep_copy=&& mv $(4)_ $(4)
else
gb_cxx_dep_generation_options=
gb_cxx_dep_copy=
endif

# AsmObject class

gb_AsmObject_get_source = $(1)/$(2).s

# $(call gb_AsmObject__command,object,relative-source,source,dep-file)
define gb_AsmObject__command
$(call gb_Output_announce,$(2),$(true),ASM,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(4)) && cd $(SRCDIR) && \
	$(gb_CC) \
		$(DEFS) \
		$(if $(WARNINGS_NOT_ERRORS),,$(gb_CFLAGS_WERROR)) \
		$(T_CFLAGS) \
		-c $(3) \
		-o $(1) \
		-I$(dir $(3)) \
		$(INCLUDE)) && \
	echo "$(1) : $(3)" > $(4)
endef

# CObject class

# $(call gb_CObject__command,object,relative-source,source,dep-file)
define gb_CObject__command
$(call gb_Output_announce,$(2).c,$(true),C  ,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(4)) && cd $(SRCDIR) && \
	$(if $(COMPILER_PLUGINS),$(gb_COMPILER_PLUGINS_SETUP)) \
	$(gb_CC) \
		$(DEFS) \
		$(if $(filter Library,$(TARGETTYPE)),$(gb_Library_LTOFLAGS)) \
		$(if $(VISIBILITY),,$(gb_VISIBILITY_FLAGS)) \
		$(if $(WARNINGS_NOT_ERRORS),,$(gb_CFLAGS_WERROR)) \
		$(if $(COMPILER_PLUGINS),$(gb_COMPILER_PLUGINS)) \
		$(T_CFLAGS) \
		-c $(3) \
		-o $(1) \
		$(gb_cxx_dep_generation_options) \
		-I$(dir $(3)) \
		$(INCLUDE) \
	    $(gb_cxx_dep_copy) \
		)
endef

# CxxObject class

# $(call gb_CxxObject__command,object,relative-source,source,dep-file)
define gb_CxxObject__command
$(call gb_Output_announce,$(2).cxx,$(true),CXX,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(4)) && cd $(SRCDIR) && \
	$(if $(COMPILER_PLUGINS),$(gb_COMPILER_PLUGINS_SETUP)) \
	$(gb_CXX) \
		$(DEFS) \
		$(if $(filter Library,$(TARGETTYPE)),$(gb_Library_LTOFLAGS)) \
		$(if $(VISIBILITY),,$(gb_VISIBILITY_FLAGS)) \
		$(if $(WARNINGS_NOT_ERRORS),,$(gb_CXXFLAGS_WERROR)) \
		$(if $(COMPILER_PLUGINS),$(gb_COMPILER_PLUGINS)) \
		$(T_CXXFLAGS) \
		-c $(3) \
		-o $(1) \
	    $(gb_cxx_dep_generation_options) \
		-I$(dir $(3)) \
		$(INCLUDE_STL) $(INCLUDE) \
	    $(gb_cxx_dep_copy) \
		)
endef

define gb_SrsPartTarget__command_dep
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(call gb_SrsPartTarget_get_dep_target,$(1))) && cd $(SRCDIR) && \
	$(gb_GCCP) \
		-MM -MT $(call gb_SrsPartTarget_get_target,$(1)) \
		$(INCLUDE) \
		$(DEFS) \
		-c -x c++-header $(2) \
		-o $(call gb_SrsPartTarget_get_dep_target,$(1)))
endef

# YaccTarget class

ifeq ($(ANCIENT_BISON),YES)
#
# There are ancient versions of bison out there, which do not handle
# well .cxx extensions, nor do they support --defines=<file>. The
# result is that the header is named <foo>.cxx.h instead of <foo>.hxx
# so we queue a mv to rename the header accordingly.
# One example is XCode versions 2.x, which are used on OSX ppc
# machines.
#
define gb_YaccTarget__command
$(call gb_Output_announce,$(2),$(true),YAC,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(3)) && \
	$(gb_YACC) $(T_YACCFLAGS) -d -o $(5) $(1) && mv $(5).h $(4) && touch $(3) )

endef

else
define gb_YaccTarget__command
$(call gb_Output_announce,$(2),$(true),YAC,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(3)) && \
	$(gb_YACC) $(T_YACCFLAGS) --defines=$(4) -o $(5) $(1) && touch $(3) )

endef
endif

# vim: set noet sw=4 ts=4:
