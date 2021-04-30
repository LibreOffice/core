# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,lingucomponent))

ifneq ($(ENABLE_WASM_STRIP_GUESSLANG),TRUE)
$(eval $(call gb_Module_add_targets,lingucomponent,\
	Library_guesslang \
))
endif

ifneq ($(ENABLE_WASM_STRIP_HUNSPELL),TRUE)
$(eval $(call gb_Module_add_targets,lingucomponent,\
	Library_hyphen \
	Library_lnth \
	Library_spell \
	StaticLibrary_ulingu \
))
endif

$(eval $(call gb_Module_add_targets,lingucomponent,\
	$(if $(filter iOS MACOSX,$(OS)),Library_MacOSXSpell) \
	Library_numbertext \
))

# vim: set noet sw=4 ts=4:
