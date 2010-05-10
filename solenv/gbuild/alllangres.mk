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


# SrsPartTarget class

# defined by platform
#  gb_SrsPartTarget_RSCTARGET
#  gb_SrsPartTarget_RSCCOMMAND
#  gb_SrsPartTarget__command_dep

$(call gb_SrsPartTarget_get_target,%) : $(SRCDIR)/% $(gb_Helper_MISCDUMMY) | $(gb_SrsPartTarget_RSCTARGET)
    $(call gb_SrsPartTarget__command_dep,$*,$<,$(INCLUDE),$(DEFS))
    $(call gb_Helper_abbreviate_dirs_native,\
        mkdir -p $(dir $@) && \
        RESPONSEFILE=`$(gb_MKTEMP) $(gb_Helper_MISC)` && \
        echo "-s \
            $(INCLUDE) \
            -I$(dir $<) \
            $(DEFS) \
            -fp=$@ \
            $<" > $${RESPONSEFILE} && \
        $(gb_SrsPartTarget_RSCCOMMAND) -presponse @$${RESPONSEFILE} && \
        rm -rf $${RESPONSEFILE})

$(call gb_SrsPartTarget_get_dep_target,%) : $(SRCDIR)/% $(gb_Helper_MISCDUMMY)
    $(call gb_Helper_abbreviate_dirs,\
        mkdir -p $(dir $@) && \
        echo '$(call gb_SrsPartTarget_get_target,$*) : $(gb_Helper_PHONY)' > $@)

$(call gb_SrsPartTarget_get_dep_target,%) :
    $(error unable to find resource definition file $(SRCDIR)/$*)

gb_SrsPartTarget_SrsPartTarget =


# SrsTarget class

gb_SrsTarget_DEFAULTDEFS := $(gb_GLOBALDEFS)

.PHONY : $(call gb_SrsTarget_get_clean_target,%)
$(call gb_SrsTarget_get_clean_target,%) :
    $(call gb_Helper_announce,Cleaning up srs $* ...)
    -$(call gb_Helper_abbreviate_dirs,\
        rm -f $(call gb_SrsTarget_get_target,$*) \
            $(call gb_SrsTarget_get_dep_target,$*) \
            $(foreach part,$(PARTS),$(call gb_SrsPartTarget_get_target,$(part))) \
            $(foreach part,$(PARTS),$(call gb_SrsPartTarget_get_dep_target,$(part))))


define gb_SrsTarget__command_dep
$(call gb_Helper_announce,Collecting dependencies for srs $(2) ...)
$(call gb_Helper_abbreviate_dirs,\
    mkdir -p $(dir $(1)) && \
    cat $(3) > $(1))
endef

$(call gb_SrsTarget_get_target,%) :
    $(call gb_SrsTarget__command_dep,$(call gb_SrsTarget_get_dep_target,$*),$*,$(foreach part,$(PARTS),$(call gb_SrsPartTarget_get_dep_target,$(part))))
    $(call gb_Helper_announce,Processing srs $* ...)
    $(call gb_Helper_abbreviate_dirs,\
        mkdir -p $(dir $@) && \
        cat $^ > $@)

$(call gb_SrsTarget_get_dep_target,%) :
    $(call gb_SrsTarget__command_dep,$@,$*,$^)

define gb_SrsTarget_SrsTarget
$(call gb_SrsTarget_get_target,$(1)) : DEFS := $(gb_SrsTarget_DEFAULTDEFS)
$(call gb_SrsTarget_get_target,$(1)) : INCLUDE := $(SOLARINC)
$(call gb_SrsTarget_get_clean_target,$(1)) : PARTS :=
$(call gb_SrsTarget_get_target,$(1)) : PARTS :=
ifeq ($(gb_FULLDEPS),$(true))
include $(call gb_SrsTarget_get_dep_target,$(1))
endif

endef

define gb_SrsTarget_set_defs
$(call gb_SrsTarget_get_target,$(1)) : DEFS := $(2)
$(call gb_SrsTarget_get_dep_target,$(1)) : DEFS := $(2)

endef

define gb_SrsTarget_set_include
$(call gb_SrsTarget_get_target,$(1)) : INCLUDE := $(2)
$(call gb_SrsTarget_get_dep_target,$(1)) : INCLUDE := $(2)

endef

define gb_SrsTarget_add_file
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_SrsTarget_get_dep_target,$(1)) : $(call gb_SrsPartTarget_get_dep_target,$(2))
endif
$(call gb_SrsTarget_get_target,$(1)) : $(call gb_SrsPartTarget_get_target,$(2))
$(call gb_SrsTarget_get_clean_target,$(1)) : PARTS += $(2)
$(call gb_SrsTarget_get_target,$(1)) : PARTS += $(2)

endef

define gb_SrsTarget_add_files
$(foreach file,$(2),$(call gb_SrsTarget_add_file,$(1),$(file)))

endef


# ResTarget

gb_ResTarget_RSCTARGET := $(gb_SrsPartTarget_RSCTARGET)
gb_ResTarget_RSCCOMMAND := $(gb_SrsPartTarget_RSCCOMMAND)

$(call gb_ResTarget_get_clean_target,%) :
    $(call gb_Helper_announce,Cleaning up resource $* ...)
    $(call gb_Helper_abbreviate_dirs,\
        rm -f $(OUTDIR)/bin/$*.res $(call gb_ResTarget_get_target,$*))

$(call gb_ResTarget_get_target,%) : $(gb_Helper_MISCDUMMY) | $(gb_ResTarget_RSCTARGET)
    $(call gb_Helper_announce,Building resource $@ ...)
    $(call gb_Helper_abbreviate_dirs_native,\
        mkdir -p $(dir $@) $(OUTDIR)/bin \
            $(dir $(call gb_ResTarget_get_imagelist_target,$(1))) && \
        RESPONSEFILE=`$(gb_MKTEMP) $(gb_Helper_MISC)` && \
        echo "-r -p \
            -lg$(LANGUAGE) \
            -fs=$(OUTDIR)/bin/$(LIBRARY)$(LANGUAGE).res \
            -lip=$(SRCDIR)/default_images/$(RESLOCATION)/imglst/$(LANGUAGE) \
            -lip=$(SRCDIR)/default_images/$(RESLOCATION)/imglst \
            -lip=$(SRCDIR)/default_images/$(RESLOCATION)/res/$(LANGUAGE) \
            -lip=$(SRCDIR)/default_images/$(RESLOCATION)/res \
            -lip=$(SRCDIR)/default_images/$(RESLOCATION) \
            -lip=$(SRCDIR)/default_images/res/$(LANGUAGE) \
            -lip=$(SRCDIR)/default_images/res \
            -subMODULE=$(SRCDIR)/default_images \
            -subGLOBALRES=$(SRCDIR)/default_images/res \
            -oil=$(dir $(call gb_ResTarget_get_imagelist_target,$(1))) \
            -ft=$@ \
            $(filter-out $(gb_Helper_MISCDUMMY),$^)" > $${RESPONSEFILE} && \
        $(gb_ResTarget_RSCCOMMAND) @$${RESPONSEFILE} && \
        rm -f $${RESPONSEFILE})

define gb_ResTarget_ResTarget
$(call gb_ResTarget_get_target,$(1)) : LIBRARY = $(2)
$(call gb_ResTarget_get_target,$(1)) : LANGUAGE = $(3)
$(call gb_ResTarget_get_target,$(1)) : RESLOCATION = $(2)
$(call gb_AllLangResTarget_get_target,$(2)) : $(call gb_ResTarget_get_target,$(1))
$(call gb_AllLangResTarget_get_clean_target,$(2)) : $(call gb_ResTarget_get_clean_target,$(1))
$(call gb_ResTarget_get_imagelist_target,$(1)) : $(call gb_ResTarget_get_target,$(1))

endef

define gb_ResTarget_add_file
$(call gb_ResTarget_get_target,$(1)) : $(2)

endef

define gb_ResTarget_add_one_srs
$(call gb_ResTarget_add_file,$(1),$(call gb_SrsTarget_get_target,$(2)))
$(call gb_ResTarget_get_clean_target,$(1)) : $(call gb_SrsTarget_get_clean_target,$(2))

endef

define gb_ResTarget_add_files
$(foreach file,$(2),\
    $(call gb_ResTarget_add_file,$(1),$(file)))

endef

define gb_ResTarget_add_srs
$(foreach srs,$(2),\
    $(call gb_ResTarget_add_one_srs,$(1),$(srs)))

endef

define gb_ResTarget_set_reslocation
$(call gb_ResTarget_get_target,$(1)) : RESLOCATION = $(2)

endef


# AllLangResTarget

gb_AllLangResTarget_LANGS := en-US

define gb_AllLangResTarget_set_langs
gb_AllLangResTarget_LANGS := $(1)
endef

$(call gb_AllLangResTarget_get_clean_target,%) :
    $(call gb_Helper_abbreviate_dirs,\
        rm -f $(call gb_AllLangResTarget_get_target,$*))

$(call gb_AllLangResTarget_get_target,%) :
    $(call gb_Helper_abbreviate_dirs,\
        mkdir -p $(dir $@) && touch $@)

define gb_AllLangResTarget_AllLangResTarget
$(call gb_ResTarget_ResTarget,$(1)en-US,$(1),en-US)
endef

define gb_AllLangResTarget_add_one_srs
$(foreach lang,$(gb_AllLangResTarget_LANGS),\
    $(call gb_ResTarget_add_one_srs,$(1)$(lang),$(2)))

endef

define gb_AllLangResTarget_add_srs
$(foreach srs,$(2),\
    $(call gb_AllLangResTarget_add_one_srs,$(1),$(srs)))

endef

define gb_AllLangResTarget_set_reslocation
$(foreach lang,$(gb_AllLangResTarget_LANGS),\
    $(call gb_ResTarget_set_reslocation,$(1)$(lang),$(2)))

endef

# vim: set noet sw=4 ts=4:
