# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/check))

$(eval $(call gb_CustomTarget_register_targets,odk/check,\
	checkbin \
	checkinc \
	checkidl \
))

odk_PLATFORM := $(if $(filter WNT,$(OS)),\
	$(if $(filter GCC,$(COM)),mingw,windows),\
	$(if $(filter SOLARIS,$(OS)),\
	$(if $(filter SPARC,$(CPUNAME)),solsparc,\
	$(if $(filter U,$(CPU)),solsparc64,solintel)),\
	$(if $(filter LINUX,$(OS)),linux,\
	$(if $(filter NETBSD,$(OS)),netbsd,\
	$(if $(filter FREEBSD,$(OS)),freebsd,\
	$(if $(filter DRAGONFLY,$(OS)),dragonfly,\
	$(if $(filter MACOSX,$(OS)),macosx,\
	$(if $(filter AIX,$(OS)),aix))))))))

$(call gb_CustomTarget_get_workdir,odk/check)/checkbin: $(SRCDIR)/odk/util/check.pl $(call gb_CustomTarget_get_target,odk/odkcommon)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CHK,1)
	touch $@
	$(PERL) $< $(call gb_CustomTarget_get_workdir,odk/odkcommon) \
		$(odk_PLATFORM) "$(gb_Exectable_EXT)" $@
	cat $@

$(call gb_CustomTarget_get_workdir,odk/check)/checkidl: $(SRCDIR)/odk/util/checkdiff.pl $(call gb_CustomTarget_get_target,odk/odkcommon)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CHK,1)
	touch $@
	diff -br $(call gb_CustomTarget_get_workdir,odk/odkcommon) $(OUTDIR)/idl \
		2>&1 | $(PERL) $< $@
	cat $@

$(call gb_CustomTarget_get_workdir,odk/check)/checkinc: $(SRCDIR)/odk/util/checkdiff.pl $(call gb_CustomTarget_get_target,odk/odkcommon)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CHK,1)
	touch $@
	diff -br $(call gb_CustomTarget_get_workdir,odk/odkcommon)/include/osl \	$(OUTDIR)/inc/osl 2>&1 | $(PERL) $< $@
	diff -br $(call gb_CustomTarget_get_workdir,odk/odkcommon)/include/sal \	$(OUTDIR)/inc/sal 2>&1 | $(PERL) $< $@
	diff -br $(call gb_CustomTarget_get_workdir,odk/odkcommon)/include/store \	$(OUTDIR)/inc/store 2>&1 | $(PERL) $< $@
	diff -br $(call gb_CustomTarget_get_workdir,odk/odkcommon)/include/typelib \	$(OUTDIR)/inc/typelib 2>&1 | $(PERL) $< $@
	diff -br $(call gb_CustomTarget_get_workdir,odk/odkcommon)/include/uno \	$(OUTDIR)/inc/uno 2>&1 | $(PERL) $< $@
	diff -br $(call gb_CustomTarget_get_workdir,odk/odkcommon)/include/bridges \	$(OUTDIR)/inc/bridges 2>&1 | $(PERL) $< $@
	diff -br $(call gb_CustomTarget_get_workdir,odk/odkcommon)/include/cppu \	$(OUTDIR)/inc/cppu 2>&1 | $(PERL) $< $@
	diff -br $(call gb_CustomTarget_get_workdir,odk/odkcommon)/include/com/sun/star/uno \	$(OUTDIR)/inc/com/sun/star/uno 2>&1 | $(PERL) $< $@
	diff -br $(call gb_CustomTarget_get_workdir,odk/odkcommon)/include/cppuhelper \	$(OUTDIR)/inc/cppuhelper 2>&1 | $(PERL) $< $@
	diff -br $(call gb_CustomTarget_get_workdir,odk/odkcommon)/include/osl \	$(OUTDIR)/inc/osl 2>&1 | $(PERL) $< $@
	cat $@

# vim: set noet sw=4 ts=4:
