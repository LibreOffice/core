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
		$(call gb_CustomTarget_get_workdir,shell/source)/$(source).cxx) \
	$(foreach header,$(SHELL_HEADERS),\
		$(call gb_CustomTarget_get_workdir,shell/source)/$(header).hxx)

$(call gb_CustomTarget_get_workdir,shell/source)/% : $(SRCDIR)/shell/source/%
	mkdir -p $(dir $@) && $(call gb_Deliver_deliver,$<,$@)

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
