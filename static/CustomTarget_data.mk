# vim: set noet sw=4 ts=4:
# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,static/data))

/home/jmg/Development/libreoffice/git_emsdk/upstream/emscripten/tools/file_packager instdir/program/soffice.data --use-preload-plugins --preload instdir/program/*rc instdir/program/*.rdb instdir/program/services/services.rdb instdir/program/resource/* instdir/program/types/* instdir/presets/  --js-output=instdir/program/soffice.data.js --separate-metadata

gb_objcopy_bfdname := $(shell objdump -a $(call gb_GenCxxObject_get_target,CustomTarget/static/component_maps) | sed -ne 's/^.*format //p')

static_WORKDIR := $(call gb_CustomTarget_get_workdir,static)
postprocess_WORKDIR := $(call gb_CustomTarget_get_workdir,postprocess)

$(call gb_CustomTarget_get_target,static/components): \
    $(static_WORKDIR)/component_maps.cxx \

gb_GenCxxObject_get_target
static_WORKDIR := $(call gb_CustomTarget_get_workdir,static)
postprocess_WORKDIR := $(call gb_CustomTarget_get_workdir,postprocess)

gb_GenCxxObject_get_target
gb_GenCxxObject_get_dwo_target
CustomTarget/static/component_maps

define gb_static_components_create_component_map
TEMPFILE=`$(gb_MKTEMP)` && \
$(call gb_Helper_abbreviate_dirs, \
    $(call gb_ExternalExecutable_get_command,python) $(1) \
        $(if $(ENABLE_SERVICES_RDB_FROM_BUILD), \
            -c $(postprocess_WORKDIR)/services_constructors.list, \
            -g core -g writer -g desktop \
        ) \
) > $$TEMPFILE && \
$(call gb_Helper_replace_if_different_and_touch,$${TEMPFILE},$(2))

endef



$(call gb_CustomTarget_get_target,static/data): \
    $(static_WORKDIR)/soffice.data.js.metadata \
    $(static_WORKDIR)/soffice.data \

$(static_WORKDIR)/component_maps.cxx: \
    $(SRCDIR)/solenv/bin/native-code.py \
    | $(static_WORKDIR)/.dir
        $(call gb_Output_announce,$(subst $(BUILDDIR)/,,$@),$(true),GEN,2)
	objcopy -I binary -B i386 -O elf64-x86-64 \
  --rename-section .data=.mydata,readonly,contents src dst.o
	objdump -a $(call gb_GenCxxObject_get_target,CustomTarget/static/component_maps) | sed -ne 's/^.*format //p'
        $(call gb_static_components_create_component_map,$<,$@)

# vim: set noet sw=4:
