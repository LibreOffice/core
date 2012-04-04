# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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

#the following user-defined variables are supported:
# YACCFLAGS
# CPPFLAGS
# CFLAGS
# CXXFLAGS
# OBJCFLAGS
# OBJCXXFLAGS
# LDFLAGS

# CFLAGS from environment override debug/optimization flags
ifeq ($(gb_DEBUGLEVEL),2)
CFLAGS ?= $(gb_COMPILEROPTFLAGS) $(gb_DEBUG_CFLAGS)
CXXFLAGS ?= $(gb_COMPILEROPTFLAGS) $(gb_DEBUG_CFLAGS) $(gb_DEBUG_CXXFLAGS)
OBJCXXFLAGS ?= $(gb_COMPILEROPTFLAGS) $(gb_DEBUG_CFLAGS) $(gb_DEBUG_CXXFLAGS)
else
CFLAGS ?= $(gb_COMPILEROPTFLAGS)
CXXFLAGS ?= $(gb_COMPILEROPTFLAGS)
OBJCXXFLAGS ?= $(gb_COMPILEROPTFLAGS)
endif

# Overview of dependencies and tasks of LinkTarget
#
# target                      task                         depends on
# LinkTarget                  linking                      AsmObject CObject CxxObject GenCObject GenCxxObject ObjCObject ObjCxxObject
#                                                          LinkTarget/headers
# LinkTarget/dep              joined dep file              AsmObject/dep CObject/dep CxxObject/dep GenCObject/dep GenCxxObject/dep ObjCObject/dep ObjCxxObject/dep
#                                                          | LinkTarget/headers
# LinkTarget/headers          all headers available        LinkTarget/external_headers
#                              including own generated     own generated headers
# LinkTarget/external_headers all external headers avail.  header files of linked libs
#
# CObject                     plain c compile              | LinkTarget/headers
# CxxObject                   c++ compile                  | LinkTarget/headers
# GenCObject                  plain c compile from         | LinkTarget/headers
#                              generated source
# GenCxxObject                C++ compile from             | LinkTarget/headers
#                              generated source
# ObjCObject                  objective c compile          | LinkTarget/headers
# ObjCxxObject                objective c++ compile        | LinkTarget/headers
#
# AsmObject                   asm compile                  | LinkTarget
#
# CObject/dep                 dependencies                 these targets generate empty dep files
# CxxObject/dep               dependencies                 that are populated upon compile
# GenCObject/dep              dependencies
# GenCxxObject/dep            dependencies
# ObjCObject/dep            dependencies
# ObjCxxObject/dep            dependencies
# AsmObject/dep               dependencies

# LinkTarget/headers means gb_LinkTarget_get_headers_target etc.
# dependencies prefixed with | are build-order only dependencies


# check that objects are only linked into one link target:
# multiple linking may cause problems because different link targets may
# require different compiler flags
define gb_Object__owner
$$(if $$(OBJECTOWNER),\
  $$(call gb_Output_error,fdo#47246: $(1) is linked in by $$(OBJECTOWNER) $(2)))$(2)
endef

# For every object there is a dep file (if gb_FULLDEPS is active).
# The dep file depends on the object: the Object__command also updates the
# dep file as a side effect.
# In the dep file rule just touch it so it's newer than the object.

# The gb_Object__command_dep generates an "always rebuild" dep file;
# It is _only_ used in case the user deletes the object dep file.
ifeq ($(gb_FULLDEPS),$(true))
define gb_Object__command_dep
mkdir -p $(dir $(1)) && \
	echo '$(2) : $$(gb_Helper_PHONY)' > $(1)

endef
else
gb_Object__command_dep = \
 $(call gb_Output_error,gb_Object__command_dep is only for gb_FULLDEPS)
endif


# CObject class

gb_CObject_get_source = $(1)/$(2).c
# defined by platform
#  gb_CObject__command

$(call gb_CObject_get_target,%) : $(call gb_CObject_get_source,$(SRCDIR),%)
	$(call gb_CObject__command,$@,$*,$<,$(call gb_CObject_get_dep_target,$*))

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_CObject_get_dep_target,%) : $(call gb_CObject_get_target,%)
	$(if $(wildcard $@),touch $@,\
	  $(call gb_Object__command_dep,$@,$(call gb_CObject_get_target,$*)))

endif


# CxxObject class

gb_CxxObject_get_source = $(1)/$(2).cxx
# defined by platform
#  gb_CxxObject__command

$(call gb_CxxObject_get_target,%) : $(call gb_CxxObject_get_source,$(SRCDIR),%)
	$(call gb_CxxObject__command,$@,$*,$<,$(call gb_CxxObject_get_dep_target,$*))

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_CxxObject_get_dep_target,%) : $(call gb_CxxObject_get_target,%)
	$(if $(wildcard $@),touch $@,\
	  $(call gb_Object__command_dep,$@,$(call gb_CxxObject_get_target,$*)))

endif


# GenCObject class

gb_GenCObject_get_source = $(WORKDIR)/$(1).c
# defined by platform
#  gb_CObject__command

$(call gb_GenCObject_get_target,%) : $(call gb_GenCObject_get_source,%)
	test -f $< || (echo "Missing generated source file $<" && false)
	$(call gb_CObject__command,$@,$*,$<,$(call gb_GenCObject_get_dep_target,$*))

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_GenCObject_get_dep_target,%) : $(call gb_GenCObject_get_target,%)
	$(if $(wildcard $@),touch $@,\
	  $(call gb_Object__command_dep,$@,$(call gb_GenCObject_get_target,$*)))

endif


# GenCxxObject class

gb_GenCxxObject_get_source = $(WORKDIR)/$(1).cxx
# defined by platform
#  gb_CxxObject__command

$(call gb_GenCxxObject_get_target,%) : $(call gb_GenCxxObject_get_source,%)
	test -f $< || (echo "Missing generated source file $<" && false)
	$(call gb_CxxObject__command,$@,$*,$<,$(call gb_GenCxxObject_get_dep_target,$*))

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_GenCxxObject_get_dep_target,%) : $(call gb_GenCxxObject_get_target,%)
	$(if $(wildcard $@),touch $@,\
	  $(call gb_Object__command_dep,$@,$(call gb_GenCxxObject_get_target,$*)))

endif


# YaccTarget class

# XXX: This is more complicated than necessary, but we cannot just use
# the generated C++ file as the main target, because we need to let the
# header depend on that to ensure the header is present before anything
# tries to use it.

gb_YaccTarget_get_source = $(1)/$(2).y
# defined by platform
#  gb_YaccTarget__command(grammar-file, stem-for-message, source-target, include-target)

.PHONY : $(call gb_YaccTarget_get_clean_target,%)
$(call gb_YaccTarget_get_clean_target,%) :
	$(call gb_Output_announce,$(2),$(false),YAC,3)
	$(call gb_Helper_abbreviate_dirs,\
	    rm -f $(call gb_YaccTarget_get_grammar_target,$*) $(call gb_YaccTarget_get_header_target,$*) $(call gb_YaccTarget_get_target,$*))

$(call gb_YaccTarget_get_target,%) : $(call gb_YaccTarget_get_source,$(SRCDIR),%)
	$(call gb_YaccTarget__command,$<,$*,$@,$(call gb_YaccTarget_get_header_target,$*),$(call gb_YaccTarget_get_grammar_target,$*))

define gb_YaccTarget_YaccTarget
$(call gb_YaccTarget_get_grammar_target,$(1)) :| $(call gb_YaccTarget_get_target,$(1))
$(call gb_YaccTarget_get_header_target,$(1)) :| $(call gb_YaccTarget_get_target,$(1))

endef

gb_YACC := bison


# ObjCxxObject class
#

gb_ObjCxxObject_get_source = $(1)/$(2).mm
# defined by platform
#  gb_ObjCxxObject__command

$(call gb_ObjCxxObject_get_target,%) : $(call gb_ObjCxxObject_get_source,$(SRCDIR),%)
	$(call gb_ObjCxxObject__command,$@,$*,$<,$(call gb_ObjCxxObject_get_dep_target,$*))

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_ObjCxxObject_get_dep_target,%) : $(call gb_ObjCxxObject_get_target,%)
	$(if $(wildcard $@),touch $@,\
	  $(call gb_Object__command_dep,$@,$(call gb_ObjCxxObject_get_target,$*)))

endif


# ObjCObject class
#

gb_ObjCObject_get_source = $(1)/$(2).m
# defined by platform
#  gb_ObjCObject__command

$(call gb_ObjCObject_get_target,%) : $(call gb_ObjCObject_get_source,$(SRCDIR),%)
	$(call gb_ObjCObject__command,$@,$*,$<,$(call gb_ObjCObject_get_dep_target,$*))

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_ObjCObject_get_dep_target,%) : $(call gb_ObjCObject_get_target,%)
	$(if $(wildcard $@),touch $@,\
	  $(call gb_Object__command_dep,$@,$(call gb_ObjCObject_get_target,$*)))

endif


# AsmObject class

# defined by platform
#  gb_AsmObject_get_source (.asm on Windows, .s elsewhere)
#  gb_AsmObject__command

$(call gb_AsmObject_get_target,%) : $(call gb_AsmObject_get_source,$(SRCDIR),%)
	$(call gb_AsmObject__command,$@,$*,$<,$(call gb_AsmObject_get_dep_target,$*))

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_AsmObject_get_dep_target,%) : $(call gb_AsmObject_get_target,%)
	$(if $(wildcard $@),touch $@,\
	  $(call gb_Object__command_dep,$@,$(call gb_AsmObject_get_target,$*)))

endif


# LinkTarget class

gb_LinkTarget_DEFAULTDEFS := $(gb_GLOBALDEFS)

# defined by platform
#  gb_LinkTarget_CXXFLAGS
#  gb_LinkTarget_LDFLAGS
#  gb_LinkTarget_INCLUDE
#  gb_LinkTarget_INCLUDE_STL

.PHONY : $(call gb_LinkTarget_get_clean_target,%)
$(call gb_LinkTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),LNK,4)
	RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),200,\
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_dep_target,$(object))) \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_dep_target,$(object))) \
		$(foreach object,$(OBJCOBJECTS),$(call gb_ObjCObject_get_target,$(object))) \
		$(foreach object,$(OBJCOBJECTS),$(call gb_ObjCObject_get_dep_target,$(object))) \
		$(foreach object,$(OBJCXXOBJECTS),$(call gb_ObjCxxObject_get_target,$(object))) \
		$(foreach object,$(OBJCXXOBJECTS),$(call gb_ObjCxxObject_get_dep_target,$(object))) \
		$(foreach object,$(ASMOBJECTS),$(call gb_AsmObject_get_target,$(object))) \
		$(foreach object,$(ASMOBJECTS),$(call gb_AsmObject_get_dep_target,$(object))) \
		$(foreach object,$(GENCOBJECTS),$(call gb_GenCObject_get_target,$(object))) \
		$(foreach object,$(GENCOBJECTS),$(call gb_GenCObject_get_dep_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_dep_target,$(object))) \
		$(call gb_LinkTarget_get_target,$*) \
		$(call gb_LinkTarget_get_dep_target,$*) \
		$(call gb_LinkTarget_get_headers_target,$*) \
		$(call gb_LinkTarget_get_external_headers_target,$*) \
		$(call gb_LinkTarget_get_objects_list,$*) \
		$(DLLTARGET) \
		$(AUXTARGETS)) && \
		cat $${RESPONSEFILE} /dev/null | xargs -n 200 rm -fr && \
		rm -f $${RESPONSEFILE}


# cat the deps of all objects in one file, then we need only open that one file
define gb_LinkTarget__command_dep
$(call gb_Output_announce,LNK:$(2),$(true),DEP,1)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),200,\
		$(foreach object,$(3),$(call gb_CObject_get_dep_target,$(object))) \
		$(foreach object,$(4),$(call gb_CxxObject_get_dep_target,$(object))) \
		$(foreach object,$(5),$(call gb_ObjCObject_get_dep_target,$(object)))\
		$(foreach object,$(6),$(call gb_ObjCxxObject_get_dep_target,$(object)))\
		$(foreach object,$(7),$(call gb_AsmObject_get_dep_target,$(object)))\
		$(foreach object,$(8),$(call gb_GenCObject_get_dep_target,$(object))) \
		$(foreach object,$(9),$(call gb_GenCxxObject_get_dep_target,$(object))) \
		) && \
	$(SOLARENV)/bin/concat-deps $${RESPONSEFILE} > $(1)) && \
	rm -f $${RESPONSEFILE}

endef

define gb_LinkTarget__command_objectlist
TEMPFILE=$(call var2file,$(shell $(gb_MKTEMP)),200,\
	$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
	$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
	$(foreach object,$(OBJCOBJECTS),$(call gb_ObjCObject_get_target,$(object))) \
	$(foreach object,$(OBJCXXOBJECTS),$(call gb_ObjCxxObject_get_target,$(object))) \
	$(foreach object,$(ASMOBJECTS),$(call gb_AsmObject_get_target,$(object))) \
	$(foreach object,$(GENCOBJECTS),$(call gb_GenCObject_get_target,$(object))) \
	$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object)))) && \
$(if $(EXTRAOBJECTLISTS),cat $(EXTRAOBJECTLISTS) >> $${TEMPFILE} && ) \
mv $${TEMPFILE} $(call gb_LinkTarget_get_objects_list,$(2))

endef

# If object files from this library are merged, create just empty file
$(call gb_LinkTarget_get_target,%) : $(call gb_LinkTarget_get_headers_target,%) $(gb_Helper_MISCDUMMY)
	$(if $(filter $*,$(foreach lib,$(gb_MERGEDLIBS),$(call gb_Library_get_linktargetname,$(lib)))), \
		touch $@, $(call gb_LinkTarget__command,$@,$*))
	$(call gb_LinkTarget__command_objectlist,$@,$*)

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_target,%) : $(call gb_LinkTarget_get_dep_target,%)
$(call gb_LinkTarget_get_dep_target,%) : | $(call gb_LinkTarget_get_headers_target,%)
	$(call gb_LinkTarget__command_dep,$@,$*,$(COBJECTS),$(CXXOBJECTS),$(OBJCOBJECTS),$(OBJCXXOBJECTS),$(ASMOBJECTS),$(GENCOBJECTS),$(GENCXXOBJECTS))
endif

# Ok, this is some dark voodoo: When declaring a linktarget with
# gb_LinkTarget_LinkTarget we set SELF in the headertarget to name of the
# target. When the rule for the headertarget is executed and SELF does not
# match the target name, we are depending on a linktarget that was never
# declared. In a full build exclusively in gbuild that should never happen.
# However, partial gbuild build will not know about how to build lower level
# linktargets, just as gbuild can not know about linktargets generated in the
# old build.pl/dmake system. Once all is migrated, gbuild should error out
# when is is told to depend on a linktarget it does not know about and not
# only warn.
define gb_LinkTarget__get_external_headers_check
ifneq ($$(SELF),$$*)
$$(eval $$(call gb_Output_info,LinkTarget $$* not defined: Assuming headers to be there!,ALL))
endif
$$@ : COMMAND := $$(call gb_Helper_abbreviate_dirs, mkdir -p $$(dir $$@) && touch $$@ && mkdir -p $(call gb_LinkTarget_get_target,)pdb/$$(dir $$*))

endef

$(call gb_LinkTarget_get_external_headers_target,%) :
	$(eval $(gb_LinkTarget__get_external_headers_check))
	$(COMMAND)

$(call gb_LinkTarget_get_headers_target,%) : $(call gb_LinkTarget_get_external_headers_target,%)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && touch $@)

# Explanation of some of the targets:
# - gb_LinkTarget_get_external_headers_target is the targets that guarantees all
#   headers from linked against libraries are in OUTDIR.
# - gb_LinkTarget_get_headers_target is the target that guarantees all headers
#   from the linked against the libraries and the linktargets own headers
#   (including generated headers) are in the OUTDIR.
# - gb_LinkTarget_get_target links the objects into a file in WORKDIR.
# gb_LinkTarget_get_target depends on gb_LinkTarget_get_headers_target which in
# turn depends gb_LinkTarget_get_external_headers_target.
# gb_LinkTarget_get_target depends additionally on the objects, which in turn
# depend build-order only on the gb_LinkTarget_get_headers_target. The build
# order-only dependency ensures all headers to be there for compiling and
# dependency generation without causing all objects to be rebuild when one
# header changes. Only the ones with an explicit dependency in their generated
# dependency file will be rebuild.
#
# gb_LinkTarget_get_target is the target that links the objects into a file in
# WORKDIR
# Explanation of some of the variables:
# - AUXTARGETS are the additionally generated files that need to be cleaned out
#   on clean.
# - TARGETTYPE is the type of linktarget as some platforms need very different
#   command to link different targettypes.
# - LIBRARY_X64 is only relevent for building a x64 library on windows.
#
# Since most variables are set on the linktarget and not on the object, the
# object learns about these setting via GNU makes scoping of target variables.
# Therefore it is important that objects are only directly depended on by the
# linktarget. This for example means that you cannot build a single object
# alone, because then you would directly depend on the object.
#
# A note about flags: because the overriding the global variables with a target
# local variable of the same name is considered obscure, the target local
# variables have a T_ prefix.
define gb_LinkTarget_LinkTarget
$(call gb_LinkTarget_get_clean_target,$(1)) : AUXTARGETS :=
$(call gb_LinkTarget_get_external_headers_target,$(1)) : SELF := $(1)
$(call gb_LinkTarget_get_target,$(1)) : DLLTARGET :=
$(call gb_LinkTarget_get_clean_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : COBJECTS :=
$(call gb_LinkTarget_get_clean_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : CXXOBJECTS :=
$(call gb_LinkTarget_get_clean_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : YACCOBJECT :=
$(call gb_LinkTarget_get_clean_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : T_YACCFLAGS := $$(gb_LinkTarget_YYACFLAGS) $(YACCFLAGS)
$(call gb_LinkTarget_get_clean_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : OBJCOBJECTS :=
$(call gb_LinkTarget_get_clean_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : OBJCXXOBJECTS :=
$(call gb_LinkTarget_get_clean_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : ASMOBJECTS :=
$(call gb_LinkTarget_get_clean_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : GENCOBJECTS :=
$(call gb_LinkTarget_get_clean_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : GENCXXOBJECTS :=
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : T_CFLAGS := $$(gb_LinkTarget_CFLAGS) $(CFLAGS)
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : T_CXXFLAGS := $$(gb_LinkTarget_CXXFLAGS)
$(call gb_LinkTarget_get_target,$(1)) : T_OBJCXXFLAGS := $$(gb_LinkTarget_OBJCXXFLAGS) $(OBJCXXFLAGS)
$(call gb_LinkTarget_get_target,$(1)) : T_OBJCFLAGS := $$(gb_LinkTarget_OBJCFLAGS) $(OBJCFLAGS)
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : DEFS := $$(gb_LinkTarget_DEFAULTDEFS) $(CPPFLAGS)
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : INCLUDE := $$(gb_LinkTarget_INCLUDE)
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : INCLUDE_STL := $$(gb_LinkTarget_INCLUDE_STL)
$(call gb_LinkTarget_get_target,$(1)) : T_LDFLAGS := $$(gb_LinkTarget_LDFLAGS) $(LDFLAGS)
$(call gb_LinkTarget_get_target,$(1)) : LINKED_LIBS :=
$(call gb_LinkTarget_get_target,$(1)) : LINKED_STATIC_LIBS :=
$(call gb_LinkTarget_get_target,$(1)) : LIBS :=
$(call gb_LinkTarget_get_target,$(1)) : TARGETTYPE :=
$(call gb_LinkTarget_get_target,$(1)) : LIBRARY_X64 :=
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : PDBFILE :=
$(call gb_LinkTarget_get_target,$(1)) : EXTRAOBJECTLISTS :=
$(call gb_LinkTarget_get_target,$(1)) : NATIVERES :=
$(call gb_LinkTarget_get_target,$(1)) : WARNINGS_NOT_ERRORS :=

ifeq ($(gb_FULLDEPS),$(true))
-include $(call gb_LinkTarget_get_dep_target,$(1))
$(call gb_LinkTarget_get_dep_target,$(1)) : COBJECTS :=
$(call gb_LinkTarget_get_dep_target,$(1)) : CXXOBJECTS :=
$(call gb_LinkTarget_get_dep_target,$(1)) : OBJCOBJECTS :=
$(call gb_LinkTarget_get_dep_target,$(1)) : OBJCXXOBJECTS :=
$(call gb_LinkTarget_get_dep_target,$(1)) : ASMOBJECTS :=
$(call gb_LinkTarget_get_dep_target,$(1)) : GENCOBJECTS :=
$(call gb_LinkTarget_get_dep_target,$(1)) : GENCXXOBJECTS :=
$(call gb_LinkTarget_get_dep_target,$(1)) : YACCOBJECTS :=
$(call gb_LinkTarget_get_dep_target,$(1)) : T_CFLAGS := $$(gb_LinkTarget_CFLAGS) $(CFLAGS)
$(call gb_LinkTarget_get_dep_target,$(1)) : T_CXXFLAGS := $$(gb_LinkTarget_CXXFLAGS)
$(call gb_LinkTarget_get_dep_target,$(1)) : T_OBJCXXFLAGS := $$(gb_LinkTarget_OBJCXXFLAGS) $(OBJCXXFLAGS)
$(call gb_LinkTarget_get_dep_target,$(1)) : T_OBJCFLAGS := $$(gb_LinkTarget_OBJCFLAGS) $(OBJCFLAGS)
$(call gb_LinkTarget_get_dep_target,$(1)) : T_YACCFLAGS := $$(gb_LinkTarget_YYACFLAGS) $(YACCFLAGS)
$(call gb_LinkTarget_get_dep_target,$(1)) : DEFS := $$(gb_LinkTarget_DEFAULTDEFS) $(CPPFLAGS)
$(call gb_LinkTarget_get_dep_target,$(1)) : INCLUDE := $$(gb_LinkTarget_INCLUDE)
$(call gb_LinkTarget_get_dep_target,$(1)) : INCLUDE_STL := $$(gb_LinkTarget_INCLUDE_STL)
$(call gb_LinkTarget_get_dep_target,$(1)) : TARGETTYPE :=
$(call gb_LinkTarget_get_dep_target,$(1)) : LIBRARY_X64 :=
$(call gb_LinkTarget_get_dep_target,$(1)) : EXTRAOBJECTLISTS :=
$(call gb_LinkTarget_get_dep_target,$(1)) : WARNINGS_NOT_ERRORS :=
endif

endef

define gb_LinkTarget_add_defs
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : DEFS += $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : DEFS += $(2)
endif
endef

define gb_LinkTarget_set_defs
ifeq (,)
$$(call gb_Output_error,\
 gb_LinkTarget_set_defs: use gb_LinkTarget_add_defs instead.)
else
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : DEFS := $(2)

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : DEFS := $(2)
endif
endif

endef

define gb_LinkTarget_add_cflags
$(call gb_LinkTarget_get_target,$(1)) : T_CFLAGS += $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : T_CFLAGS += $(2)
endif

endef

define gb_LinkTarget_set_cflags
ifeq (,)
$$(call gb_Output_error,\
 gb_LinkTarget_set_cflags: use gb_LinkTarget_add_cflags instead.)
else
$(call gb_LinkTarget_get_target,$(1)) : T_CFLAGS := $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : T_CFLAGS := $(2)
endif
endif

endef

define gb_LinkTarget_add_cxxflags
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : T_CXXFLAGS += $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : T_CXXFLAGS += $(2)
endif
endef

define gb_LinkTarget_set_cxxflags
ifeq (,)
$$(call gb_Output_error,\
 gb_LinkTarget_set_cxxflags: use gb_LinkTarget_add_cxxflags instead.)
else
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : T_CXXFLAGS := $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : T_CXXFLAGS := $(2)
endif
endif

endef

define gb_LinkTarget_add_objcxxflags
$(call gb_LinkTarget_get_target,$(1)) : T_OBJCXXFLAGS += $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : T_OBJCXXFLAGS += $(2)
endif
endef

define gb_LinkTarget_set_objcxxflags
ifeq (,)
$$(call gb_Output_error,\
 gb_LinkTarget_set_objcxxflags: use gb_LinkTarget_add_objcxxflags instead.)
else
$(call gb_LinkTarget_get_target,$(1)) : T_OBJCXXFLAGS := $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : T_OBJCXXFLAGS := $(2)
endif
endif

endef

define gb_LinkTarget_set_objcflags
$(call gb_LinkTarget_get_target,$(1)) : T_OBJCFLAGS := $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : T_OBJCFLAGS := $(2)
endif

endef

define gb_LinkTarget_set_include
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : INCLUDE := $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : INCLUDE := $(2)
endif

endef

define gb_LinkTarget_set_include_stl
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : INCLUDE_STL := $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : INCLUDE_STL := $(2)
endif

endef

define gb_LinkTarget_add_ldflags
$(call gb_LinkTarget_get_target,$(1)) : T_LDFLAGS += $(2)
endef

# real use in RepositoryExternal.mk
define gb_LinkTarget_set_ldflags
$(call gb_LinkTarget_get_target,$(1)) : T_LDFLAGS := $(2)
endef

define gb_LinkTarget_add_libs
$(call gb_LinkTarget_get_target,$(1)) : LIBS += $(2)
endef

define gb_LinkTarget_add_api
$(call gb_LinkTarget_get_external_headers_target,$(1)) :| \
	$$(foreach api,$(2),$$(call gb_Package_get_target,$$(api)_inc))
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : INCLUDE += $$(foreach api,$(2),-I$(OUTDIR)/inc/$$(api))
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : INCLUDE += $$(foreach api,$(2),-I$(OUTDIR)/inc/$$(api))
endif

endef

define gb_LinkTarget_add_linked_libs
ifneq (,$$(filter-out $(gb_Library_KNOWNLIBS),$(2)))
$$(eval $$(call gb_Output_info,currently known libraries are: $(sort $(gb_Library_KNOWNLIBS)),ALL))
$$(eval $$(call gb_Output_error,Cannot link against library/libraries $$(filter-out $(gb_Library_KNOWNLIBS),$(2)). Libraries must be registered in Repository.mk))
endif

gb_LINKED_LIBS := $(if $(filter $(gb_MERGEDLIBS),$(2)), \
	$(if $(filter $(1),$(foreach lib,$(gb_MERGEDLIBS),$(call gb_Library_get_linktargetname,$(lib)))),, merged)) \
	$(filter-out $(gb_MERGEDLIBS),$(2))

$(call gb_LinkTarget_get_target,$(1)) : LINKED_LIBS += $$(gb_LINKED_LIBS)

$(call gb_LinkTarget_get_target,$(1)) : $$(foreach lib,$$(gb_LINKED_LIBS),$$(call gb_Library_get_target,$$(lib)))
$(call gb_LinkTarget_get_external_headers_target,$(1)) : \
$$(foreach lib,$$(gb_LINKED_LIBS),$$(call gb_Library_get_headers_target,$$(lib)))

endef

define gb_LinkTarget_add_linked_static_libs
ifneq (,$$(filter-out $(gb_StaticLibrary_KNOWNLIBS),$(2)))
$$(eval $$(call gb_Output_info, currently known static libraries are: $(sort $(gb_StaticLibrary_KNOWNLIBS)),ALL))
$$(eval $$(call gb_Output_error,Cannot link against static library/libraries $$(filter-out $(gb_StaticLibrary_KNOWNLIBS),$(2)). Static libraries must be registered in Repository.mk))
endif

$(call gb_LinkTarget_get_target,$(1)) : LINKED_STATIC_LIBS += $(2)

$(call gb_LinkTarget_get_target,$(1)) : $$(foreach lib,$(2),$$(call gb_StaticLibrary_get_target,$$(lib)))
$(call gb_LinkTarget_get_external_headers_target,$(1)) : \
$$(foreach lib,$(2),$$(call gb_StaticLibrary_get_headers_target,$$(lib)))

endef

define gb_LinkTarget_add_linked_static_external_libs

$(call gb_LinkTarget_get_target,$(1)) : LINKED_STATIC_LIBS += $(2)

$(call gb_LinkTarget_get_target,$(1)) : $$(foreach lib,$(2),$$(call gb_ExternalLibs_get_target,$$(lib)))
$(call gb_LinkTarget_get_external_headers_target,$(1)) : $$(foreach lib,$(2),$$(call gb_ExternalLibs_get_target,$$(lib)))

endef

define gb_LinkTarget_add_cobject
$(if $(wildcard $(call gb_CObject_get_source,$(SRCDIR),$(2))),,$(eval $(call gb_Output_error,No such source file $(call gb_CObject_get_source,$(SRCDIR),$(2)))))
$(call gb_LinkTarget_get_target,$(1)) : COBJECTS += $(2)
$(call gb_LinkTarget_get_clean_target,$(1)) : COBJECTS += $(2)

$(call gb_LinkTarget_get_target,$(1)) : $(call gb_CObject_get_target,$(2))
$(call gb_CObject_get_target,$(2)) : | $(call gb_LinkTarget_get_headers_target,$(1))
$(call gb_CObject_get_target,$(2)) : T_CFLAGS += $(3)
$(call gb_CObject_get_target,$(2)) : \
	OBJECTOWNER := $(call gb_Object__owner,$(2),$(1))

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : COBJECTS += $(2)
$(call gb_LinkTarget_get_dep_target,$(1)) : $(call gb_CObject_get_dep_target,$(2))
endif

endef

define gb_LinkTarget_add_cxxobject
$(if $(wildcard $(call gb_CxxObject_get_source,$(SRCDIR),$(2))),,$(eval $(call gb_Output_error,No such source file $(call gb_CxxObject_get_source,$(SRCDIR),$(2)))))
$(call gb_LinkTarget_get_target,$(1)) : CXXOBJECTS += $(2)
$(call gb_LinkTarget_get_clean_target,$(1)) : CXXOBJECTS += $(2)

$(call gb_LinkTarget_get_target,$(1)) : $(call gb_CxxObject_get_target,$(2))
$(call gb_CxxObject_get_target,$(2)) : | $(call gb_LinkTarget_get_headers_target,$(1))
$(call gb_CxxObject_get_target,$(2)) : T_CXXFLAGS += $(3)
$(call gb_CxxObject_get_target,$(2)) : \
	OBJECTOWNER := $(call gb_Object__owner,$(2),$(1))

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : CXXOBJECTS += $(2)
$(call gb_LinkTarget_get_dep_target,$(1)) : $(call gb_CxxObject_get_dep_target,$(2))
endif

endef

define gb_LinkTarget_add_objcobject
$(if $(wildcard $(call gb_ObjCObject_get_source,$(SRCDIR),$(2))),,$(eval $(call gb_Output_error,No such source file $(call gb_ObjCObject_get_source,$(SRCDIR),$(2)))))
$(call gb_LinkTarget_get_target,$(1)) : OBJCOBJECTS += $(2)
$(call gb_LinkTarget_get_clean_target,$(1)) : OBJCOBJECTS += $(2)

$(call gb_LinkTarget_get_target,$(1)) : $(call gb_ObjCObject_get_target,$(2))
$(call gb_ObjCObject_get_target,$(2)) : | $(call gb_LinkTarget_get_headers_target,$(1))
$(call gb_ObjCObject_get_target,$(2)) : T_OBJCFLAGS += $(3)
$(call gb_ObjCObject_get_target,$(2)) : \
	OBJECTOWNER := $(call gb_Object__owner,$(2),$(1))

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : OBJCOBJECTS += $(2)
$(call gb_LinkTarget_get_dep_target,$(1)) : $(call gb_ObjCObject_get_dep_target,$(2))
endif

endef

define gb_LinkTarget_add_objcxxobject
$(if $(wildcard $(call gb_ObjCxxObject_get_source,$(SRCDIR),$(2))),,$(eval $(call gb_Output_error,No such source file $(call gb_ObjCxxObject_get_source,$(SRCDIR),$(2)))))
$(call gb_LinkTarget_get_target,$(1)) : OBJCXXOBJECTS += $(2)
$(call gb_LinkTarget_get_clean_target,$(1)) : OBJCXXOBJECTS += $(2)

$(call gb_LinkTarget_get_target,$(1)) : $(call gb_ObjCxxObject_get_target,$(2))
$(call gb_ObjCxxObject_get_target,$(2)) : | $(call gb_LinkTarget_get_headers_target,$(1))
$(call gb_ObjCxxObject_get_target,$(2)) : T_OBJCXXFLAGS += $(3)
$(call gb_ObjCxxObject_get_target,$(2)) : \
	OBJECTOWNER := $(call gb_Object__owner,$(2),$(1))

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : OBJCXXOBJECTS += $(2)
$(call gb_LinkTarget_get_dep_target,$(1)) : $(call gb_ObjCxxObject_get_dep_target,$(2))
endif

endef

define gb_LinkTarget_add_asmobject
$(if $(wildcard $(call gb_AsmObject_get_source,$(SRCDIR),$(2))),,$(eval $(call gb_Output_error,No such source file $(call gb_AsmObject_get_source,$(SRCDIR),$(2)))))
$(call gb_LinkTarget_get_target,$(1)) : ASMOBJECTS += $(2)
$(call gb_LinkTarget_get_clean_target,$(1)) : ASMOBJECTS += $(2)

$(call gb_LinkTarget_get_target,$(1)) : $(call gb_AsmObject_get_target,$(2))
$(call gb_AsmObject_get_target,$(2)) : | $(call gb_LinkTarget_get_headers_target,$(1))
$(call gb_AsmObject_get_target,$(2)) : \
	OBJECTOWNER := $(call gb_Object__owner,$(2),$(1))

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : ASMOBJECTS += $(2)
$(call gb_LinkTarget_get_dep_target,$(1)) : $(call gb_AsmObject_get_dep_target,$(2))
endif

endef

define gb_LinkTarget_add_generated_c_object
$(call gb_LinkTarget_get_target,$(1)) : GENCOBJECTS += $(2)
$(call gb_LinkTarget_get_clean_target,$(1)) : GENCOBJECTS += $(2)

# Make just needs to know gb_GenCObject_get_source is a real target.
# Then it can use implicit rule for gb_GenCObject_get_target.
$(call gb_GenCObject_get_source,$(2)) : | $(gb_Helper_MISCDUMMY)
$(call gb_LinkTarget_get_target,$(1)) : $(call gb_GenCObject_get_target,$(2))
$(call gb_GenCObject_get_target,$(2)) : | $(call gb_LinkTarget_get_headers_target,$(1))
$(call gb_GenCObject_get_target,$(2)) : T_CFLAGS += $(3)
$(call gb_GenCObject_get_target,$(2)) : \
	OBJECTOWNER := $(call gb_Object__owner,$(2),$(1))

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : GENCOBJECTS += $(2)
$(call gb_LinkTarget_get_dep_target,$(1)) : $(call gb_GenCObject_get_dep_target,$(2))
endif

endef

define gb_LinkTarget_add_generated_cxx_object
$(call gb_LinkTarget_get_target,$(1)) : GENCXXOBJECTS += $(2)
$(call gb_LinkTarget_get_clean_target,$(1)) : GENCXXOBJECTS += $(2)

# Make just needs to know gb_GenCxxObject_get_source is a real target.
# Then it can use implicit rule for gb_GenCxxObject_get_target.
$(call gb_GenCxxObject_get_source,$(2)) : | $(gb_Helper_MISCDUMMY)
$(call gb_LinkTarget_get_target,$(1)) : $(call gb_GenCxxObject_get_target,$(2))
$(call gb_GenCxxObject_get_target,$(2)) : | $(call gb_LinkTarget_get_headers_target,$(1))
$(call gb_GenCxxObject_get_target,$(2)) : T_CXXFLAGS += $(3)
$(call gb_GenCxxObject_get_target,$(2)) : \
	OBJECTOWNER := $(call gb_Object__owner,$(2),$(1))

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : GENCXXOBJECTS += $(2)
$(call gb_LinkTarget_get_dep_target,$(1)) : $(call gb_GenCxxObject_get_dep_target,$(2))
endif

endef

# Add a bison grammar to the build.
# gb_LinkTarget_add_grammar(<component>,<grammar file>)
define gb_LinkTarget_add_grammar
$(call gb_YaccTarget_YaccTarget,$(2))
$(call gb_LinkTarget_add_generated_cxx_object,$(1),YaccTarget/$(2))
$(call gb_LinkTarget_get_clean_target,$(1)) : $(call gb_YaccTarget_get_clean_target,$(2))
$(call gb_LinkTarget__add_internal_headers,$(1),$(call gb_YaccTarget_get_header_target,$(2)))

endef

# Add bison grammars to the build.
# gb_LinkTarget_add_grammars(<component>,<grammar file> [<grammar file>*])
define gb_LinkTarget_add_grammars
$(foreach grammar,$(2),$(call gb_LinkTarget_add_grammar,$(1),$(grammar)))
endef

define gb_LinkTarget_add_noexception_object
$(call gb_LinkTarget_add_cxxobject,$(1),$(2),$(gb_LinkTarget_NOEXCEPTIONFLAGS) $(CXXFLAGS))
endef

define gb_LinkTarget_add_exception_object
$(call gb_LinkTarget_add_cxxobject,$(1),$(2),$(gb_LinkTarget_EXCEPTIONFLAGS) $(CXXFLAGS))
endef

define gb_LinkTarget_add_cobjects
$(foreach obj,$(2),$(call gb_LinkTarget_add_cobject,$(1),$(obj),$(3)))
endef

define gb_LinkTarget_add_linktarget_objects
$(call gb_LinkTarget_get_target,$(1)) : $(foreach linktarget,$(2),$(call gb_LinkTarget_get_target,$(linktarget)))
ifneq ($(OS),IOS)
$(call gb_LinkTarget_get_target,$(1)) : EXTRAOBJECTLISTS += $(foreach linktarget,$(2),$(call gb_LinkTarget_get_objects_list,$(linktarget)))
endif

endef

define gb_LinkTarget_add_library_objects
ifneq (,$$(filter-out $(gb_Library_KNOWNLIBS),$(2)))
$$(eval $$(call gb_Output_info,currently known libraries are: $(sort $(gb_Library_KNOWNLIBS)),ALL))
$$(eval $$(call gb_Output_error,Cannot import objects library/libraries $$(filter-out $(gb_Library_KNOWNLIBS),$(2)). Libraries must be registered in Repository.mk))
endif
$(call gb_LinkTarget_add_linktarget_objects,$(1),$(foreach lib,$(2),$(call gb_Library_get_linktargetname,$(lib))))

endef

define gb_LinkTarget_add_executable_objects
$(call gb_LinkTarget_add_linktarget_objects,$(1),$(foreach exe,$(2),$(call gb_Executable_get_linktargetname,$(lib))))

endef

define gb_LinkTarget_add_cxxobjects
$(foreach obj,$(2),$(call gb_LinkTarget_add_cxxobject,$(1),$(obj),$(3)))
endef

define gb_LinkTarget_add_objcobjects
$(foreach obj,$(2),$(call gb_LinkTarget_add_objcobject,$(1),$(obj),$(3)))
endef

define gb_LinkTarget_add_objcxxobjects
$(foreach obj,$(2),$(call gb_LinkTarget_add_objcxxobject,$(1),$(obj),$(3)))
endef

define gb_LinkTarget_add_asmobjects
$(foreach obj,$(2),$(call gb_LinkTarget_add_asmobject,$(1),$(obj),$(3)))
endef

define gb_LinkTarget_add_noexception_objects
$(foreach obj,$(2),$(call gb_LinkTarget_add_noexception_object,$(1),$(obj)))
endef

define gb_LinkTarget_add_exception_objects
$(foreach obj,$(2),$(call gb_LinkTarget_add_exception_object,$(1),$(obj)))
endef

#only useful for building x64 libraries on windows
define gb_LinkTarget_add_x64_generated_exception_objects
$(foreach obj,$(2),$(call gb_LinkTarget_add_generated_exception_object,$(1),$(obj)))
$(foreach obj,$(2),$(eval $(call gb_GenCxxObject_get_target,$(obj)) : CXXOBJECT_X64 := YES))
endef

define gb_LinkTarget_add_generated_cobjects
$(foreach obj,$(2),$(call gb_LinkTarget_add_generated_c_object,$(1),$(obj),$(3)))
endef

define gb_LinkTarget_add_generated_cxxobjects
$(foreach obj,$(2),$(call gb_LinkTarget_add_generated_cxx_object,$(1),$(obj),$(3)))
endef

define gb_LinkTarget_add_generated_exception_object
$(call gb_LinkTarget_add_generated_cxx_object,$(1),$(2),$(gb_LinkTarget_EXCEPTIONFLAGS) $(gb_COMPILEROPTFLAGS) $(CXXFLAGS))
endef

define gb_LinkTarget_add_generated_exception_objects
$(foreach obj,$(2),$(call gb_LinkTarget_add_generated_exception_object,$(1),$(obj)))
endef

define gb_LinkTarget_set_targettype
$(call gb_LinkTarget_get_target,$(1)) \
$(call gb_LinkTarget_get_dep_target,$(1)) : TARGETTYPE := $(2)
endef

define gb_LinkTarget_set_x64
$(call gb_LinkTarget_get_target,$(1)) \
$(call gb_LinkTarget_get_dep_target,$(1)) : LIBRARY_X64 := $(2)
endef

define gb_LinkTarget_set_dlltarget
$(call gb_LinkTarget_get_clean_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : DLLTARGET := $(2)
endef

define gb_LinkTarget_set_auxtargets
$$(call gb_Output_error,\
 gb_LinkTarget_set_auxtargets: use gb_LinkTarget_add_auxtargets instead.)
endef

define gb_LinkTarget_add_auxtargets
$(call gb_LinkTarget_get_clean_target,$(1)) : AUXTARGETS += $(2)
endef

define gb_LinkTarget__add_internal_headers
$(call gb_LinkTarget_get_headers_target,$(1)) : $(2)
$(2) :|	$(call gb_LinkTarget_get_external_headers_target,$(1))

endef

define gb_LinkTarget_add_custom_headers
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : INCLUDE += -I$(call gb_CustomTarget_get_workdir,$(2))
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : INCLUDE += -I$(call gb_CustomTarget_get_workdir,$(2))
endif
$(call gb_LinkTarget__add_internal_headers,$(1),$(call gb_CustomTarget_get_target,$(2)))
$(call gb_LinkTarget_get_clean_target,$(1)) : $(call gb_CustomTarget_get_clean_target,$(2))

endef

define gb_LinkTarget_add_package_headers
$(foreach package,$(2),$(call gb_LinkTarget__add_internal_headers,$(1),$(call gb_Package_get_target,$(package))))
$(call gb_LinkTarget_get_clean_target,$(1)) : $(foreach package,$(2),$(call gb_Package_get_clean_target,$(package)))

endef

define gb_LinkTarget_add_sdi_headers
$(call gb_LinkTarget__add_internal_headers,$(1),$(foreach sdi,$(2),$(call gb_SdiTarget_get_target,$(sdi))))
$(call gb_LinkTarget_get_clean_target,$(1)) : $(foreach sdi,$(2),$(call gb_SdiTarget_get_clean_target,$(sdi)))
endef

define gb_LinkTarget_add_external_headers
$(call gb_LinkTarget_get_headers_target,$(1) : |$(call gb_Package_get_target,$(2)))
endef

# this forwards to functions that must be defined in RepositoryExternal.mk.
# $(eval $(call gb_LinkTarget_use_external,library,external))
define gb_LinkTarget_use_external
$(if $(filter undefined,$(origin gb_LinkTarget__use_$(2))),\
  $(error gb_LinkTarget_use_external: unknown external: $(2)),\
  $(call gb_LinkTarget__use_$(2),$(1)))
endef

# $(call gb_LinkTarget_use_externals,library,externals)
gb_LinkTarget_use_externals = \
 $(foreach external,$(2),$(call gb_LinkTarget_use_external,$(1),$(external)))

define gb_LinkTarget_set_warnings_not_errors
$(call gb_LinkTarget_get_target,$(1)) : WARNINGS_NOT_ERRORS := $(true)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : WARNINGS_NOT_ERRORS := $(true)
endif

endef

# vim: set noet sw=4:
