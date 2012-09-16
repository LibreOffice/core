# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
# 

$(eval $(call gb_Package_Package,helpcompiler_inc,$(SRCDIR)/helpcompiler))

$(eval $(call gb_Package_add_file,helpcompiler_inc,inc/helpcompiler/dllapi.h,inc/dllapi.h))
$(eval $(call gb_Package_add_file,helpcompiler_inc,inc/helpcompiler/compilehelp.hxx,inc/compilehelp.hxx))
$(eval $(call gb_Package_add_file,helpcompiler_inc,inc/helpcompiler/HelpCompiler.hxx,inc/HelpCompiler.hxx))
$(eval $(call gb_Package_add_file,helpcompiler_inc,inc/helpcompiler/HelpIndexer.hxx,inc/HelpIndexer.hxx))
$(eval $(call gb_Package_add_file,helpcompiler_inc,inc/helpcompiler/HelpLinker.hxx,inc/HelpLinker.hxx))
$(eval $(call gb_Package_add_file,helpcompiler_inc,inc/helpcompiler/HelpSearch.hxx,inc/HelpSearch.hxx))

# vim: set noet sw=4 ts=4:
