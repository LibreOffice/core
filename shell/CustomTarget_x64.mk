# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,shell/source))

SHELL_SHLXTHANDLER_COMMON_FILES := \
	win32/ooofilereader/autostyletag \
	win32/ooofilereader/basereader \
	win32/ooofilereader/contentreader \
	win32/ooofilereader/keywordstag \
	win32/ooofilereader/metainforeader \
	win32/ooofilereader/simpletag \
	win32/shlxthandler/ooofilt/stream_helper \
	win32/shlxthandler/util/fileextensions \
	win32/shlxthandler/util/iso8601_converter \
	win32/shlxthandler/util/registry \
	win32/shlxthandler/util/utilities \
	win32/zipfile/zipexcptn \
	win32/zipfile/zipfile

SHELL_SHLXTHDL_FILES := \
	win32/shlxthandler/classfactory \
	win32/shlxthandler/columninfo/columninfo \
	win32/shlxthandler/infotips/infotips \
	win32/shlxthandler/propsheets/document_statistic \
	win32/shlxthandler/propsheets/listviewbuilder \
	win32/shlxthandler/propsheets/propsheets \
	win32/shlxthandler/shlxthdl \
	win32/shlxthandler/thumbviewer/thumbviewer

SHELL_OOOFILT_FILES := \
    win32/shlxthandler/ooofilt/ooofilt \
    win32/shlxthandler/ooofilt/propspec

SHELL_PROPERTYHDL_FILES := \
    win32/shlxthandler/prophdl/propertyhdl

SHELL_XMLPARSER_FILES := \
	all/xml_parser

SHELL_HEADERS := \
	win32/ooofilereader/autostyletag \
	win32/ooofilereader/dummytag \
	win32/ooofilereader/itag \
	win32/ooofilereader/keywordstag \
	win32/ooofilereader/simpletag \
	win32/shlxthandler/classfactory \
	win32/shlxthandler/ooofilt/ooofilt \
	win32/shlxthandler/ooofilt/propspec \
	win32/shlxthandler/propsheets/document_statistic \
	win32/shlxthandler/propsheets/listviewbuilder \
	win32/zipfile/zipexcptn

$(call gb_CustomTarget_get_target,shell/source) : \
	$(foreach source,$(SHELL_SHLXTHANDLER_COMMON_FILES) $(SHELL_SHLXTHDL_FILES) $(SHELL_OOOFILT_FILES) $(SHELL_PROPERTYHDL_FILES) $(SHELL_XMLPARSER_FILES),\
		$(gb_CustomTarget_workdir)/shell/source/$(source).cxx) \
	$(foreach header,$(SHELL_HEADERS),\
		$(gb_CustomTarget_workdir)/shell/source/$(header).hxx)

$(gb_CustomTarget_workdir)/shell/source/% : $(SRCDIR)/shell/source/%
	mkdir -p $(dir $@) && $(call gb_Deliver_deliver,$<,$@)

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
