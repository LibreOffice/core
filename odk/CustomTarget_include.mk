# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/odkcommon/include))

include $(SRCDIR)/solenv/inc/udkversion.mk

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
odk_INCLIST := $(subst $(OUTDIR)/inc/,,$(shell find \
	$(foreach dir,$(odk_INCDIRLIST),$(OUTDIR)/inc/$(dir)) -type f)) \
	$(odk_INCFILELIST)

define odk_inc
odkcommon_ZIPLIST += include/$(1)
$(call gb_CustomTarget_get_target,odk/odkcommon/include): $(odk_WORKDIR)/include/$(1)
$(odk_WORKDIR)/include/$(1): $(OUTDIR)/inc/$(1)
	mkdir -p $$(dir $$@)
	$$(call gb_Output_announce,$$(subst $$(WORKDIR)/,,$$@),build,CPY,1)
	cp $$< $$@
endef

$(foreach inc,$(odk_INCLIST),$(eval $(call odk_inc,$(inc))))

odkcommon_ZIPLIST += include/udkversion.mk
$(eval $(call gb_CustomTarget_register_target,odk/odkcommon/include,include/udkversion.mk))
$(odk_WORKDIR)/include/udkversion.mk:
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	echo "#UDK version number" > $@
	echo "#major" >>$@
	echo "UDK_MAJOR=$(UDK_MAJOR)" >> $@
	echo "#minor" >>$@
	echo "UDK_MINOR=$(UDK_MINOR)" >> $@
	echo "#micro" >>$@
	echo "UDK_MICRO=$(UDK_MICRO)" >> $@

# vim: set noet sw=4 ts=4:
