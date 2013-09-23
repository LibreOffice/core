# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/docs))

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
odk_cpp_PREFIX := $(INSTDIR)/$(SDKDIRNAME)/include/
odk_cpp_DOXY_INPUT := $(SRCDIR)/odk/docs/cpp/main.dox \
	$(SRCDIR)/include/sal/log-areas.dox \
	$(addprefix $(odk_cpp_PREFIX),$(odk_cpp_INCDIRLIST) $(odk_cpp_INCFILELIST))
odk_cpp_DOXY_WORKDIR := $(call gb_CustomTarget_get_workdir,odk/docs/cpp)/ref

$(eval $(call gb_CustomTarget_register_targets,odk/docs,\
	cpp/Doxyfile \
	cpp/doxygen.log \
))

$(call gb_CustomTarget_get_workdir,odk/docs)/cpp/Doxyfile : \
		$(SRCDIR)/odk/docs/cpp/Doxyfile \
		$(gb_Module_CURRENTMAKEFILE)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),SED,1)
	sed -e 's!^INPUT = %$$!INPUT = $(call odk_cygwin_path,$(odk_cpp_DOXY_INPUT))!' \
		-e 's!^OUTPUT_DIRECTORY = %$$!OUTPUT_DIRECTORY = $(call odk_cygwin_path,$(odk_cpp_DOXY_WORKDIR))!' \
		-e 's!^PROJECT_BRIEF = %$$!PROJECT_BRIEF = "$(PRODUCTNAME) $(PRODUCTVERSION) SDK C/C++ API Reference"!' \
		-e 's!^PROJECT_NAME = %$$!PROJECT_NAME = $(PRODUCTNAME)!' \
		-e 's!^QUIET = %$$!QUIET = $(if $(VERBOSE),NO,YES)!' \
		-e 's!^STRIP_FROM_PATH = %$$!STRIP_FROM_PATH = $(call odk_cygwin_path,$(odk_cpp_PREFIX))!' \
		$< > $@

$(call gb_CustomTarget_get_workdir,odk/docs)/cpp/doxygen.log : \
		$(call gb_CustomTarget_get_workdir,odk/docs)/cpp/Doxyfile \
		$(SRCDIR)/include/sal/log-areas.dox \
		$(SRCDIR)/odk/docs/cpp/main.dox \
		$(call gb_PackageSet_get_target,odk_headers)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),GEN,1)
	rm -rf $(odk_cpp_DOXY_WORKDIR)/ && $(DOXYGEN) $< > $@

$(eval $(call gb_CustomTarget_register_targets,odk/docs,\
	idl/Doxyfile \
	idl/doxygen.log \
))

odk_idl_PREFIX := $(SRCDIR)/udkapi/ $(SRCDIR)/offapi/
# note: generated_idl_chapter_refs.idl must be the _last_ input file!
# otherwise spurious references to it will appear in the output
odk_idl_DOXY_INPUT := $(SRCDIR)/odk/docs/idl/main.dox \
	$(addsuffix com,$(odk_idl_PREFIX)) \
	$(SRCDIR)/odk/docs/idl/generated_idl_chapter_refs.idl
odk_idl_DOXY_WORKDIR := $(call gb_CustomTarget_get_workdir,odk/docs/idl)/ref

# don't depend on the IDL files directly but instead on the udkapi/offapi
# which will get rebuilt when any IDL file changes
$(call gb_CustomTarget_get_workdir,odk/docs)/idl/Doxyfile : \
		$(SRCDIR)/odk/docs/idl/Doxyfile \
		$(call gb_UnoApi_get_target,udkapi) \
		$(call gb_UnoApi_get_target,offapi) \
		$(gb_Module_CURRENTMAKEFILE)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),SED,1)
	sed -e 's!^INPUT = %$$!INPUT = $(call odk_cygwin_path,$(odk_idl_DOXY_INPUT))!' \
		-e 's!^OUTPUT_DIRECTORY = %$$!OUTPUT_DIRECTORY = $(call odk_cygwin_path,$(odk_idl_DOXY_WORKDIR))!' \
		-e 's!^PROJECT_BRIEF = %$$!PROJECT_BRIEF = "$(PRODUCTNAME) $(PRODUCTVERSION) SDK API Reference"!' \
		-e 's!^PROJECT_NAME = %$$!PROJECT_NAME = $(PRODUCTNAME)!' \
		-e 's!^QUIET = %$$!QUIET = $(if $(VERBOSE),NO,YES)!' \
		-e 's!^STRIP_FROM_PATH = %$$!STRIP_FROM_PATH = $(call odk_cygwin_path,$(odk_idl_PREFIX))!' \
		-e 's!^SHORT_NAMES = %$$!SHORT_NAMES = $(if $(filter WNT,$(OS)),YES,NO)!' \
		$< > $@

$(call gb_CustomTarget_get_workdir,odk/docs)/idl/doxygen.log : \
		$(call gb_CustomTarget_get_workdir,odk/docs)/idl/Doxyfile \
		$(SRCDIR)/odk/docs/idl/main.dox
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),GEN,1)
	rm -rf $(odk_idl_DOXY_WORKDIR)/ && $(DOXYGEN) $< > $@

# vim: set noet sw=4 ts=4:
