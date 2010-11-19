#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2009 by Sun Microsystems, Inc.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************


# outdir target pattern

gb_ComponentTarget_get_outdir_target = $(OUTDIR)/xml/component/$(1).component
gb_Executable_get_target = $(OUTDIR)/bin/$(1)$(gb_Executable_EXT)
gb_PackagePart_get_destinations = $(OUTDIR)/xml $(OUTDIR)/inc $(OUTDIR)/bin
gb_PackagePart_get_target = $(OUTDIR)/$(1)
gb_ResTarget_get_outdir_imagelist_target = $(OUTDIR)/res/img/$(1).ilst
gb_ResTarget_get_outdir_target = $(OUTDIR)/bin/$(1).res

define gb_Library_get_target
$(patsubst $(1):%,$(gb_Library_OUTDIRLOCATION)/%,$(filter $(filter $(1),$(gb_Library_TARGETS)):%,$(gb_Library_FILENAMES)))
endef

define gb_StaticLibrary_get_target
$(patsubst $(1):%,$(gb_StaticLibrary_OUTDIRLOCATION)/%,$(filter $(filter $(1),$(gb_StaticLibrary_TARGETS)):%,$(gb_StaticLibrary_FILENAMES)))
endef


# workdir target patterns

gb_AllLangResTarget_get_target = $(WORKDIR)/AllLangRes/$(1)
gb_CObject_get_target = $(WORKDIR)/CObject/$(1).o
gb_ComponentTarget_get_target = $(WORKDIR)/ComponentTarget/$(1).component
gb_CxxObject_get_target = $(WORKDIR)/CxxObject/$(1).o
gb_Executable_get_external_headers_target = $(WORKDIR)/ExternalHeaders/Executable/$(1)
gb_Executable_get_headers_target = $(WORKDIR)/Headers/Executable/$(1)
gb_LinkTarget_get_external_headers_target = $(WORKDIR)/ExternalHeaders/$(1)
gb_LinkTarget_get_headers_target = $(WORKDIR)/Headers/$(1)
gb_LinkTarget_get_target = $(WORKDIR)/LinkTarget/$(1)
gb_Module_get_target = $(WORKDIR)/Module/$(1)
gb_NoexPrecompiledHeader_get_target = $(WORKDIR)/NoexPrecompiledHeader/$(gb_NoexPrecompiledHeader_DEBUGDIR)/$(1).hxx.pch
gb_ObjCxxObject_get_target = $(WORKDIR)/ObjCxxObject/$(1).o
gb_Package_get_target = $(WORKDIR)/Package/$(1)
gb_PrecompiledHeader_get_target = $(WORKDIR)/PrecompiledHeader/$(gb_PrecompiledHeader_DEBUGDIR)/$(1).hxx.pch
gb_ResTarget_get_imagelist_target = $(WORKDIR)/ResTarget/$(1).ilst
gb_ResTarget_get_target = $(WORKDIR)/ResTarget/$(1).res
gb_SdiTarget_get_target = $(WORKDIR)/SdiTarget/$(1)
gb_SrsPartMergeTarget_get_target = $(WORKDIR)/SrsPartMergeTarget/$(1)
gb_SrsPartTarget_get_target = $(WORKDIR)/SrsPartTarget/$(1)
gb_SrsTarget_get_target = $(WORKDIR)/SrsTarget/$(1).srs

define gb_Library_get_external_headers_target
$(patsubst $(1):%,$(WORKDIR)/ExternalHeaders/Library/%,$(filter $(1):%,$(gb_Library_FILENAMES)))
endef

define gb_Library_get_headers_target
$(patsubst $(1):%,$(WORKDIR)/Headers/Library/%,$(filter $(1):%,$(gb_Library_FILENAMES)))
endef

define gb_StaticLibrary_get_external_headers_target
$(patsubst $(1):%,$(WORKDIR)/ExternalHeaders/StaticLibrary/%,$(filter $(1):%,$(gb_StaticLibrary_FILENAMES)))
endef

define gb_StaticLibrary_get_headers_target
$(patsubst $(1):%,$(WORKDIR)/Headers/StaticLibrary/%,$(filter $(1):%,$(gb_StaticLibrary_FILENAMES)))
endef

$(eval $(call gb_Helper_make_clean_targets,\
    AllLangResTarget \
    ComponentTarget \
    LinkTarget \
    Module \
    NoexPrecompiledHeader \
    PackagePart \
    PrecompiledHeader \
    ResTarget \
    SdiTarget \
    SrsTarget \
))

$(eval $(call gb_Helper_make_outdir_clean_targets,\
    Executable \
    Library \
    Package \
    StaticLibrary \
))

$(eval $(call gb_Helper_make_dep_targets,\
    CObject \
    CxxObject \
    ObjCxxObject \
    LinkTarget \
    SrsPartTarget \
    SrsTarget \
    PrecompiledHeader \
    NoexPrecompiledHeader \
))


# other getters

gb_Library_get_linktargetname = Library/$(1)
gb_StaticLibrary_get_linktargetname = StaticLibrary/$(1)

define gb_LinkTarget_get_layer
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_LinkTarget_LAYER)))
endef

define gb_Library_get_filename
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_Library_FILENAMES)))
endef

define gb_StaticLibrary_get_filename
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_StaticLibrary_FILENAMES)))
endef


# defined here so it is available early

define gb_Module_register_target
gb_Module_TARGETSTACK := $(1) $(gb_Module_TARGETSTACK)
gb_Module_CLEANTARGETSTACK := $(2) $(gb_Module_CLEANTARGETSTACK)

endef

# static members declared here because they are used globally

gb_Library_OUTDIRLOCATION := $(OUTDIR)/lib
gb_Library_DLLDIR := $(WORKDIR)/LinkTarget/Library
gb_StaticLibrary_OUTDIRLOCATION := $(OUTDIR)/lib

# vim: set noet sw=4 ts=4:
