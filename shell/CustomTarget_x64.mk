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
	win32/ooofilereader/autostyletag.cxx \
	win32/ooofilereader/basereader.cxx \
	win32/ooofilereader/contentreader.cxx \
	win32/ooofilereader/keywordstag.cxx \
	win32/ooofilereader/metainforeader.cxx \
	win32/ooofilereader/simpletag.cxx \
	win32/shlxthandler/ooofilt/stream_helper.cxx \
	win32/shlxthandler/util/fileextensions.cxx \
	win32/shlxthandler/util/iso8601_converter.cxx \
	win32/shlxthandler/util/registry.cxx \
	win32/shlxthandler/util/utilities.cxx \
	win32/zipfile/zipexcptn.cxx \
	win32/zipfile/zipfile.cxx

SHELL_SHLXTHDL_FILES := \
	win32/shlxthandler/classfactory.cxx \
	win32/shlxthandler/columninfo/columninfo.cxx \
	win32/shlxthandler/infotips/infotips.cxx \
	win32/shlxthandler/propsheets/document_statistic.cxx \
	win32/shlxthandler/propsheets/listviewbuilder.cxx \
	win32/shlxthandler/propsheets/propsheets.cxx \
	win32/shlxthandler/shlxthdl.cxx \
	win32/shlxthandler/thumbviewer/thumbviewer.cxx

SHELL_OOOFILT_FILES := \
    win32/shlxthandler/ooofilt/ooofilt.cxx \
    win32/shlxthandler/ooofilt/propspec.cxx

SHELL_PROPERTYHDL_FILES := \
    win32/shlxthandler/prophdl/propertyhdl.cxx

SHELL_XMLPARSER_FILES := \
	all/xml_parser.cxx

SHELL_SPSUPP_FILES := \
    win32/spsupp/COMOpenDocuments.cxx \
    win32/spsupp/registrar.cxx \
    win32/spsupp/spsuppClassFactory.cxx \
    win32/spsupp/spsuppEditOrRODlg.cxx \
    win32/spsupp/spsuppServ.cxx \
    win32/spsupp/res/resource.h

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
	$(foreach source,$(SHELL_SHLXTHANDLER_COMMON_FILES) $(SHELL_SHLXTHDL_FILES) $(SHELL_OOOFILT_FILES) $(SHELL_PROPERTYHDL_FILES) $(SHELL_XMLPARSER_FILES) $(SHELL_SPSUPP_FILES),\
		$(call gb_CustomTarget_get_workdir,shell/source)/$(source)) \
	$(foreach header,$(SHELL_HEADERS),\
		$(call gb_CustomTarget_get_workdir,shell/source)/$(header).hxx)

$(call gb_CustomTarget_get_workdir,shell/source)/% : $(SRCDIR)/shell/source/%
	mkdir -p $(dir $@) && $(call gb_Deliver_deliver,$<,$@)

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
