#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************


# Static Library class

# defined globally in gbuild.mk
#  gb_StaticLibrary_OUTDIRLOCATION := $(OUTDIR)/lib
# defined by platform
#  gb_StaticLibrary_DEFS
#  gb_StaticLibrary_FILENAMES
#  gb_StaticLibrary_TARGETS

gb_StaticLibrary__get_linktargetname = StaticLibrary/$(call gb_StaticLibrary_get_filename,$(1))

# EVIL: gb_StaticLibrary and gb_Library need the same deliver rule because they are indistinguishable on windows
.PHONY : $(WORKDIR)/Clean/OutDir/lib/%$(gb_StaticLibrary_PLAINEXT)
$(WORKDIR)/Clean/OutDir/lib/%$(gb_StaticLibrary_PLAINEXT) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(OUTDIR)/lib/$*$(gb_StaticLibrary_PLAINEXT) \
			$(AUXTARGETS))

# EVIL: gb_StaticLibrary and gb_Library need the same deliver rule because they are indistinguishable on windows
$(gb_StaticLibrary_OUTDIRLOCATION)/%$(gb_StaticLibrary_PLAINEXT) : 
	$(call gb_Helper_abbreviate_dirs,\
		$(call gb_Deliver_deliver,$<,$@) \
			$(foreach target,$(AUXTARGETS), && $(call gb_Deliver_deliver,$(dir $<)/$(notdir $(target)),$(target))))

define gb_StaticLibrary_StaticLibrary
ifeq (,$$(findstring $(1),$$(gb_StaticLibrary_KNOWNLIBS)))
$$(eval $$(call gb_Output_info,Currently known static libraries are: $(sort $(gb_StaticLibrary_KNOWNLIBS)),ALL))
$$(eval $$(call gb_Output_error,Static library $(1) must be registered in Repository.mk))
endif
$(call gb_StaticLibrary_get_target,$(1)) : AUXTARGETS :=
$(call gb_StaticLibrary__StaticLibrary_impl,$(1),$(call gb_StaticLibrary__get_linktargetname,$(1)))

endef

define gb_StaticLibrary__StaticLibrary_impl
$(call gb_LinkTarget_LinkTarget,$(2))
$(call gb_LinkTarget_set_targettype,$(2),StaticLibrary)
$(call gb_LinkTarget_set_defs,$(2),\
	$$(DEFS) \
	$(gb_StaticLibrary_DEFS) \
)
$(call gb_StaticLibrary_get_target,$(1)) : $(call gb_LinkTarget_get_target,$(2))
$(call gb_StaticLibrary_get_clean_target,$(1)) : $(call gb_LinkTarget_get_clean_target,$(2))
$(call gb_StaticLibrary_StaticLibrary_platform,$(1),$(2))
$$(eval $$(call gb_Module_register_target,$(call gb_StaticLibrary_get_target,$(1)),$(call gb_StaticLibrary_get_clean_target,$(1))))
$(call gb_Deliver_add_deliverable,$(call gb_StaticLibrary_get_target,$(1)),$(call gb_LinkTarget_get_target,$(2)))

endef

define gb_StaticLibrary_forward_to_Linktarget
gb_StaticLibrary_$(1) = $$(call gb_LinkTarget_$(1),$$(call gb_StaticLibrary__get_linktargetname,$$(1)),$$(2),$$(3))

endef

$(eval $(foreach method,\
	add_cobject \
	add_cobjects \
	add_cxxobject \
	add_cxxobjects \
	add_objcxxobject \
	add_objcxxobjects \
	add_exception_objects \
	add_noexception_objects \
	add_generated_exception_objects \
	set_cflags \
	set_cxxflags \
	set_objcxxflags \
	set_defs \
	set_include \
	set_ldflags \
	set_library_path_flags \
	add_linked_libs \
	add_linked_static_libs \
	add_package_headers \
	add_sdi_headers \
	add_precompiled_header \
,\
	$(call gb_StaticLibrary_forward_to_Linktarget,$(method))\
))

# vim: set noet sw=4:
