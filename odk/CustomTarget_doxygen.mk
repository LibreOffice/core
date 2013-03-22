# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/odkcommon/docs/cpp/ref))

odkcommon_ZIPLIST += docs/cpp/ref
odkcommon_ZIPDEPS += $(odk_WORKDIR)/docs/cpp/ref/index.html

CPPDOCREFNAME := "$(PRODUCTNAME) $(PRODUCTVERSION) SDK C/C++ API Reference"

odk_INCDIRLIST := sal salhelper rtl osl store typelib uno cppu cppuhelper \
	registry $(if $(filter WNT,$(OS)),systools)
odk_INCFILELIST := com/sun/star/uno/Any.h \
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
DOXY_INPUT := $(SRCDIR)/odk/pack/gendocu/main.dox $(SRCDIR)/sal/inc/sal/log-areas.dox \
	$(addprefix $(OUTDIR)/inc/,$(odk_INCDIRLIST) $(odk_INCFILELIST))
DOXY_INPUT := $(if $(filter WNT,$(OS)),$(shell cygpath -u $(DOXY_INPUT)),$(DOXY_INPUT))
DOXY_WORKDIR := $(if $(filter WNT,$(OS)),$(shell cygpath -u $(odk_WORKDIR)/docs/cpp/ref),$(odk_WORKDIR)/docs/cpp/ref)
DOXY_STRIP_PATH := $(if $(filter WNT,$(OS)),$(shell cygpath -u $(OUTDIR)/inc),$(OUTDIR)/inc)
DOXY_DEPS := $(SRCDIR)/odk/pack/gendocu/Doxyfile \
	$(SRCDIR)/odk/pack/gendocu/main.dox \
	$(SRCDIR)/sal/inc/sal/log-areas.dox \
	$(call gb_Package_get_target,sal_odk_headers) \
	$(call gb_Package_get_target,sal_generated) \
	$(call gb_Package_get_target,salhelper_odk_headers) \
	$(call gb_Package_get_target,cppu_odk_headers) \
	$(call gb_Package_get_target,cppuhelper_odk_headers) \
	$(call gb_Package_get_target,store_odk_headers) \
	$(call gb_Package_get_target,registry_odk_headers)


$(eval $(call gb_CustomTarget_register_target,odk/odkcommon/docs/cpp/ref,index.html))

$(odk_WORKDIR)/docs/cpp/ref/index.html: $(DOXY_DEPS)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),GEN,1)
	sed -e 's!^INPUT = %$$!INPUT = $(DOXY_INPUT)!' \
	-e 's!^OUTPUT_DIRECTORY = %$$!OUTPUT_DIRECTORY = $(DOXY_WORKDIR)!' \
	-e 's!^PROJECT_BRIEF = %$$!PROJECT_BRIEF = $(CPPDOCREFNAME)!' \
	-e 's!^PROJECT_NAME = %$$!PROJECT_NAME = $(PRODUCTNAME)!' \
        -e 's!^QUIET = %$$!QUIET = $(if $(VERBOSE),NO,YES)!' \
        -e 's!^STRIP_FROM_PATH = %$$!STRIP_FROM_PATH = $(DOXY_STRIP_PATH)!' \
	$< > $(odk_WORKDIR)/Doxyfile
	$(DOXYGEN) $(odk_WORKDIR)/Doxyfile > $(odk_WORKDIR)/doxygen.log

# vim: set noet sw=4 ts=4:
