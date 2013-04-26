# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/docs/cpp))

odk_cpp_INCDIRLIST := sal salhelper rtl osl typelib uno cppu cppuhelper \
	$(if $(filter WNT,$(OS)),systools)
odk_cpp_INCFILELIST := com/sun/star/uno/Any.h \
	com/sun/star/uno/Any.hxx \
	com/sun/star/uno/genfunc.h \
	com/sun/star/uno/genfunc.hxx \
	com/sun/star/uno/Reference.h \
	com/sun/star/uno/Reference.hxx \
	com/sun/star/uno/Sequence.h \
	com/sun/star/uno/Sequence.hxx \
	com/sun/star/uno/Type.h \
	com/sun/star/uno/Type.hxx

# Cygwin Doxygen needs unix paths
odk_cygwin_path = $(if $(filter WNT,$(OS)),$(shell cygpath -u $(1)),$(1))
odk_cpp_PREFIX := $(INSTDIR)/$(gb_Package_SDKDIRNAME)/include/
odk_cpp_DOXY_INPUT := $(SRCDIR)/odk/pack/gendocu/main.dox $(SRCDIR)/include/sal/log-areas.dox \
	$(addprefix $(odk_cpp_PREFIX),$(odk_cpp_INCDIRLIST) $(odk_cpp_INCFILELIST))
odk_cpp_DOXY_WORKDIR := $(call gb_CustomTarget_get_workdir,odk/docs/cpp)/ref

$(eval $(call gb_CustomTarget_register_targets,odk/docs/cpp,\
	Doxyfile \
	doxygen.log \
))

$(call gb_CustomTarget_get_workdir,odk/docs/cpp)/Doxyfile : \
		$(SRCDIR)/odk/pack/gendocu/Doxyfile \
		$(gb_Module_CURRENTMAKEFILE)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),SED,1)
	sed -e 's!^INPUT = %$$!INPUT = $(call odk_cygwin_path,$(odk_cpp_DOXY_INPUT))!' \
		-e 's!^OUTPUT_DIRECTORY = %$$!OUTPUT_DIRECTORY = $(call odk_cygwin_path,$(odk_cpp_DOXY_WORKDIR))!' \
		-e 's!^PROJECT_BRIEF = %$$!PROJECT_BRIEF = "$(PRODUCTNAME) $(PRODUCTVERSION) SDK C/C++ API Reference"!' \
		-e 's!^PROJECT_NAME = %$$!PROJECT_NAME = $(PRODUCTNAME)!' \
		-e 's!^QUIET = %$$!QUIET = $(if $(VERBOSE),NO,YES)!' \
		-e 's!^STRIP_FROM_PATH = %$$!STRIP_FROM_PATH = $(call odk_cygwin_path,$(odk_cpp_PREFIX))!' \
		$< > $@

$(call gb_CustomTarget_get_workdir,odk/docs/cpp)/doxygen.log : \
		$(call gb_CustomTarget_get_workdir,odk/docs/cpp)/Doxyfile \
		$(SRCDIR)/include/sal/log-areas.dox \
		$(SRCDIR)/odk/pack/gendocu/main.dox \
		$(call gb_PackageSet_get_target,odk_headers)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),GEN,1)
	$(DOXYGEN) $< > $@

# vim: set noet sw=4 ts=4:
