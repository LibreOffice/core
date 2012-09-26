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
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
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

$(eval $(call gb_Library_Library,writerfilter_uno))

$(eval $(call gb_Library_set_include,writerfilter_uno,\
    $$(INCLUDE) \
    -I$(SRCDIR)/writerfilter/inc \
))

$(eval $(call gb_Library_add_defs,writerfilter_uno,\
	-DWRITERFILTER_WRITERFILTER_UNO_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_sdk_api,writerfilter_uno))

$(eval $(call gb_Library_use_libraries,writerfilter_uno,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    writerfilter \
	$(gb_UWINAPI) \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_use_external,writerfilter_uno,libxml2))

$(eval $(call gb_Library_add_exception_objects,writerfilter_uno,\
    writerfilter/unocomponent/component \
    writerfilter/unocomponent/debugservices/doctok/DocTokAnalyzeService \
    writerfilter/unocomponent/debugservices/doctok/DocTokTestService \
    writerfilter/unocomponent/debugservices/ooxml/OOXMLAnalyzeService \
    writerfilter/unocomponent/debugservices/ooxml/OOXMLTestService \
))

# not used
	# writerfilter/unocomponent/debugservices/rtftok/ScannerTestService \
	# writerfilter/unocomponent/debugservices/rtftok/XMLScanner \

# vim: set noet sw=4 ts=4:
