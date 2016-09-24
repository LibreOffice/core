# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/autotext/user))

extras_AUTOTEXTUSER_AUTOTEXTS := \
	mytexts \


extras_AUTOTEXTUSER_XMLFILES := \
	mytexts/BlockList.xml \
	mytexts/META-INF/manifest.xml \


extras_AUTOTEXTUSER_MIMETYPEFILES := \
	mytexts/mimetype \


ifneq ($(sort $(foreach file,$(extras_AUTOTEXTUSER_XMLFILES),$(firstword $(subst /, ,$(file))))),$(sort $(extras_AUTOTEXTUSER_AUTOTEXTS)))
$(call gb_Output_error,defined user autotext do not match existing directories)
endif

$(call gb_CustomTarget_get_target,extras/source/autotext/user) : \
	$(foreach atexts,$(extras_AUTOTEXTUSER_AUTOTEXTS),$(call gb_CustomTarget_get_workdir,extras/source/autotext/user)/$(atexts).bau)

$(call gb_CustomTarget_get_workdir,extras/source/autotext/user)/%/mimetype : $(SRCDIR)/extras/source/autotext/%/mimetype
	$(call gb_Output_announce,$*/mimetype,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/autotext/user)/%.xml : $(SRCDIR)/extras/source/autotext/%.xml \
		| $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,$*.xml,$(true),XSL,1)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<

$(call gb_CustomTarget_get_workdir,extras/source/autotext/user)/%.bau :
	$(call gb_Output_announce,$*.bau,$(true),ZIP,2)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(EXTRAS_AUTOTEXTUSER_DIR) && \
		zip -qrX --filesync --must-match $@ $(EXTRAS_AUTOTEXTUSER_FILES) \
	)

define extras_Autotextuser_make_file_deps
$(call gb_CustomTarget_get_workdir,$(1)/user)/$(2) : $(SRCDIR)/$(1)/$(2) \
	| $(dir $(call gb_CustomTarget_get_workdir,$(1)/user)/$(2)).dir

endef

define extras_Autotextuser_make_zip_deps
$(call gb_CustomTarget_get_workdir,$(1)/user)/$(2) : \
	$(addprefix $(call gb_CustomTarget_get_workdir,$(1)/user)/,$(filter $(3)/%,$(extras_AUTOTEXTUSER_MIMETYPEFILES) $(extras_AUTOTEXTUSER_XMLFILES))) \
	| $(dir $(call gb_CustomTarget_get_workdir,$(1)/user)/$(2)).dir

$(call gb_CustomTarget_get_workdir,$(1)/user)/$(2) : \
	EXTRAS_AUTOTEXTUSER_FILES := $(foreach file,$(filter $(3)/%,$(extras_AUTOTEXTUSER_MIMETYPEFILES) $(extras_AUTOTEXTUSER_XMLFILES)),$(subst $(3)/,,$(file)))
$(call gb_CustomTarget_get_workdir,$(1)/user)/$(2) : \
	EXTRAS_AUTOTEXTUSER_DIR := $(call gb_CustomTarget_get_workdir,$(1)/user)/$(3)

endef

$(eval $(foreach file,$(extras_AUTOTEXTUSER_MIMETYPEFILES) $(extras_AUTOTEXTUSER_XMLFILES),\
	$(call extras_Autotextuser_make_file_deps,extras/source/autotext,$(file)) \
))

$(eval $(foreach atexts,$(extras_AUTOTEXTUSER_AUTOTEXTS),\
	$(call extras_Autotextuser_make_zip_deps,extras/source/autotext,$(atexts).bau,$(atexts)) \
))

# vim: set noet sw=4 ts=4:
