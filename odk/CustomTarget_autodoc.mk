# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/docs/common/ref))

$(eval $(call gb_CustomTarget_register_targets,odk/docs/common/ref,\
	autodoc_log.txt \
))

$(call gb_CustomTarget_get_workdir,odk/docs/common/ref)/%.html : $(call gb_CustomTarget_get_workdir,odk/docs/common/ref)/autodoc_log.txt
	touch $@

$(call gb_CustomTarget_get_workdir,odk/docs/common/ref)/autodoc_log.txt : \
		$(SRCDIR)/odk/pack/copying/idl_chapter_refs.txt \
		$(SRCDIR)/odk/docs/common/ref/idl.css \
		$(call gb_Executable_get_runtime_dependencies,autodoc)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),AUD,1)
	$(call gb_Executable_get_command,autodoc) \
		-html $(dir $@) \
		-dvgroot "http://wiki.services.openoffice.org/wiki" \
		-name "LibreOffice $(PRODUCTVERSION) API" \
		-lg idl \
		-dvgfile $< \
		-t $(SRCDIR)/udkapi $(SRCDIR)/offapi \
		> $@


# vim: set noet sw=4 ts=4:
