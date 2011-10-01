# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
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
# Major Contributor(s):
# Copyright (C) 2011 Norbert Thiebaud <nthiebaud@gmail.com>
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

gb_ExternalLib_REPOSITORYNAMES := $(gb_Helper_REPOSITORYNAMES)

gb_ExternalLib_get_src_package = $(TARFILE_LOCATION)/$(1)

define gb_ExternalLib_ExternalLib
$(call gb_ExternalLib_get_target,$(1)): T_PATCHES:=
$(call gb_ExternalLib_get_target,$(1)): T_POST_PATCHES:=
$(call gb_ExternalLib_get_target,$(1)): T_CONF_ARGS:=
$(call gb_ExternalLib_get_target,$(1)): T_BUILD_MODE:=$(2)
$$(eval $$(call gb_Module_register_target,$(call gb_ExternalLib_get_target,$(1)),$(call gb_ExternalLib_get_clean_target,$(1))))

ifeq ($(strip $(2)),autotools)
$(call gb_ExternalLib_get_target,$(1)) :
	$$(call gb_Output_announce,$*,$(true),MAK,3)
	$$(call gb_ExternalLib__command_autotools,$(1));
	touch $(call gb_ExternalLib_get_target,$(1));

.PHONY: $(call gb_ExternalLib_get_clean_target,$(1))
$(call gb_ExternalLib_get_clean_target,$(1)) :
	$$(call gb_Output_announce,$(1),$(false),MAK,3)
	$$(call gb_Helper_abbreviate_dirs,\
	    if test -f $$(call gb_ExternalLib_get_builddir,$(1))/Makefile ; then cd $$(call gb_ExternalLib_get_builddir,$(1)) && make uninstall ; fi ; \
	    rm -f $$(call gb_ExternalLib_get_target,$(1)) \
		rm -rf $(call gb_ExternalLib_get_workdir,$(1)))

else
	$$(call gb_Output_error,$(2) is not a supported ExternalLib mode)
endif

endef

define gb_ExternalLib_set_src_package
$(call gb_ExternalLib_get_target,$(1)): $(call gb_ExternalLib_get_src_package,$(2))
endef

define gb_ExternalLib_add_patch
$(call gb_ExternalLib_get_target,$(1)): T_PATCHES+=$(2)

endef

define gb_ExternalLib_add_patches
$(foreach patch,$(2),$(call gb_ExternalLib_add_patch,$(1),$(patch)))
endef

define gb_ExternalLib_add_post_patch
$(call gb_ExternalLib_get_target,$(1)): T_POST_PATCHES+=$(2)

endef

define gb_ExternalLib_add_post_patches
$(foreach patch,$(2),$(call gb_ExternalLib_add_patch,$(1),$(patch)))
endef

define gb_ExternalLib_add_conf_arg
$(call gb_ExternalLib_get_target,$(1)): T_CONF_ARGS+=$(2)
endef

define gb_ExternalLib_add_cxxflags
$(call gb_ExternalLib_get_target,$(1)) : T_CXXFLAGS += $(2)
endef

define gb_ExternalLib_add_cflags
$(call gb_ExternalLib_get_target,$(1)) : T_CFLAGS += $(2)
endef


define gb_ExternalLib__command_autotools
	rm -fr $(call gb_ExternalLib_get_workdir,$(1)) && \
	mkdir -p $(call gb_ExternalLib_get_builddir,$(1)) && \
	tar -x -C $(call gb_ExternalLib_get_builddir,$(1)) --strip-component=1 -f $< && \
	pushd $(call gb_ExternalLib_get_builddir,$(1)) && for p in $(T_PATCHES) ; do patch -p 1 < $(gb_REPOS)/$$p || exit 1; done && \
	CC="$(gb_CC)" CXX="$(gb_CXX)" CFLAGS="$(T_CFLAGS)" CXXFLAGS="$(T_CXXFLAGS)" PKG_CONFIG_PATH="$(OUTDIR)/lib/pkgconfig" ./configure --prefix=$(OUTDIR) $(T_CONF_ARGS) && \
	for p in $(T_POST_PATCHES) ; do patch -p 1 < $(gb_REPOS)/$p || exit 1; done
	#we don't want a deployed rpath pointing into our solver, to-do, set correct ORIGIN-foo ?
	pushd $(call gb_ExternalLib_get_builddir,$(1)) && if test -e libtool ; then sed -i 's,^hardcode_libdir_flag_spec=.*,hardcode_libdir_flag_spec="",g' libtool && sed -i 's,^runpath_var=LD_RUN_PATH,runpath_var=DIE_RPATH_DIE,g' libtool; fi
	+MAKEFLAGS=$(filterout r,$(MAKEFLAGS)) $(MAKE) -C $(call gb_ExternalLib_get_builddir,$(1))
	+$(MAKE) -C $(call gb_ExternalLib_get_builddir,$(1)) install
endef
