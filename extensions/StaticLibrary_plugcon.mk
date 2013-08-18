# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

ifeq ($(OS),WNT)
$(error only for UNX)
endif

$(eval $(call gb_StaticLibrary_StaticLibrary,plugcon))

$(eval $(call gb_StaticLibrary_use_externals,plugcon,\
    boost_headers \
    npapi_headers \
))

$(eval $(call gb_StaticLibrary_set_include,plugcon,\
	$$(INCLUDE) \
	-I$(SRCDIR)/extensions/source/plugin/inc \
))

$(eval $(call gb_StaticLibrary_use_api,plugcon,\
	offapi \
	udkapi \
))

ifeq ($(ENABLE_JAVA),TRUE)
$(eval $(call gb_StaticLibrary_add_defs,plugcon,\
	-DOJI \
))

endif # ENABLE_JAVA=YES

ifeq ($(SYSTEM_MOZILLA),YES)
$(eval $(call gb_StaticLibrary_add_defs,plugcon,\
	-DSYSTEM_MOZILLA \
))
endif # SYSTEM_MOZILLA=YES

ifeq ($(ENABLE_GTK),TRUE)
$(eval $(call gb_StaticLibrary_add_defs,plugcon,\
	-DENABLE_GTK \
))

$(eval $(call gb_StaticLibrary_use_external,plugcon,gtk))

# Gtk uses cairo, and if we build internal cairo, the linker could
# pick up our cairo libs before they're fully ready, causing problems
# (e.g. using our libpixman and system libcairo). Depend on cairo
# to delay build until cairo is done.
ifeq ($(SYSTEM_CAIRO),NO)
$(eval $(call gb_StaticLibrary_use_external,plugcon,cairo))
endif
endif # ENABLE_GTK=TRUE

$(eval $(call gb_StaticLibrary_add_exception_objects,plugcon,\
	extensions/source/plugin/unx/mediator \
	extensions/source/plugin/unx/plugcon \
))

# vim:set noet sw=4 ts=4:

