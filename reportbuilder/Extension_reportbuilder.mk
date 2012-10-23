# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Extension_Extension,report-builder,reportbuilder/util))

$(eval $(call gb_Extension_add_files,report-builder,,\
	$(call gb_Jar_get_outdir_target,reportbuilderwizard) \
	$(call gb_Jar_get_outdir_target,sun-report-builder) \
	$(SRCDIR)/reportbuilder/license/readme_en-US.html \
	$(SRCDIR)/reportbuilder/license/readme_en-US.txt \
	$(SRCDIR)/reportbuilder/util/components.rdb \
))

ifneq ($(SYSTEM_APACHE_COMMONS),YES)
$(eval $(call gb_Extension_add_files,report-builder,,\
	$(OUTDIR)/bin/commons-logging-1.1.1.jar \
))
endif

ifneq ($(SYSTEM_JFREEREPORT),YES)
include $(SRCDIR)/jfreereport/version.mk
$(eval $(call gb_Extension_add_files,report-builder,,\
	$(OUTDIR)/bin/flow-engine-$(FLOW_ENGINE_VERSION).jar \
	$(OUTDIR)/bin/flute-$(FLUTE_VERSION).jar \
	$(OUTDIR)/bin/libbase-$(LIBBASE_VERSION).jar \
	$(OUTDIR)/bin/libfonts-$(LIBFONTS_VERSION).jar \
	$(OUTDIR)/bin/libformula-$(LIBFORMULA_VERSION).jar \
	$(OUTDIR)/bin/liblayout-$(LIBLAYOUT_VERSION).jar \
	$(OUTDIR)/bin/libloader-$(LIBLOADER_VERSION).jar \
	$(OUTDIR)/bin/librepository-$(LIBREPOSITORY_VERSION).jar \
	$(OUTDIR)/bin/libserializer-$(LIBSERIALIZER_VERSION).jar \
	$(OUTDIR)/bin/libxml-$(LIBXML_VERSION).jar \
	$(OUTDIR)/bin/sac.jar \
))
endif

$(eval $(call gb_Extension_add_files,report-builder,images,\
    $(SRCDIR)/icon-themes/galaxy/desktop/res/extension_32.png \
))

$(eval $(call gb_Extension_add_files,report-builder,registry/data/org/openoffice,\
	$(call gb_XcuFile_for_extension,reportbuilder/registry/data/org/openoffice/Setup.xcu) \
))

$(eval $(call gb_Extension_add_files,report-builder,registry/data/org/openoffice/Office,\
	$(call gb_XcuFile_for_extension,reportbuilder/registry/data/org/openoffice/Office/Accelerators.xcu) \
	$(call gb_XcuFile_for_extension,reportbuilder/registry/data/org/openoffice/Office/DataAccess.xcu) \
	$(call gb_XcuFile_for_extension,reportbuilder/registry/data/org/openoffice/Office/Embedding.xcu) \
	$(call gb_XcuFile_for_extension,reportbuilder/registry/data/org/openoffice/Office/ExtendedColorScheme.xcu) \
	$(call gb_XcuDataTarget_get_target,reportbuilder/registry/data/org/openoffice/Office/Paths.xcu) \
	$(call gb_XcuDataTarget_get_target,reportbuilder/registry/data/org/openoffice/Office/ReportDesign.xcu) \
))

$(eval $(call gb_Extension_add_files,report-builder,registry/data/org/openoffice/Office/UI,\
	$(call gb_XcuDataTarget_get_target,reportbuilder/registry/data/org/openoffice/Office/UI/Controller.xcu) \
	$(call gb_XcuFile_for_extension,reportbuilder/registry/data/org/openoffice/Office/UI/DbReportWindowState.xcu) \
	$(call gb_XcuFile_for_extension,reportbuilder/registry/data/org/openoffice/Office/UI/ReportCommands.xcu) \
))

$(eval $(call gb_Extension_add_files,report-builder,registry/data/org/openoffice/TypeDetection,\
	$(call gb_XcuFile_for_extension,reportbuilder/registry/data/org/openoffice/TypeDetection/Filter.xcu) \
	$(call gb_XcuFile_for_extension,reportbuilder/registry/data/org/openoffice/TypeDetection/Types.xcu) \
))

$(eval $(call gb_Extension_add_files,report-builder,registry/schema/org/openoffice/Office,\
	$(call gb_XcsTarget_get_target,reportbuilder/registry/schema/org/openoffice/Office/ReportDesign.xcs) \
))

$(eval $(call gb_Extension_add_files,report-builder,registry/schema/org/openoffice/Office/UI,\
	$(call gb_XcsTarget_get_target,reportbuilder/registry/schema/org/openoffice/Office/UI/DbReportWindowState.xcs) \
	$(call gb_XcsTarget_get_target,reportbuilder/registry/schema/org/openoffice/Office/UI/ReportCommands.xcs) \
))

$(eval $(call gb_Extension_add_files,report-builder,template/en-US/wizard/report,\
	$(SRCDIR)/reportbuilder/template/en-US/wizard/report/default.otr \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
