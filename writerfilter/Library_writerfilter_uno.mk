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
# The Initial Developer of the Original Code is
#       David Tardon, Red Hat Inc. <dtardon@redhat.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
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
    -I$(realpath $(SRCDIR)/writerfilter/inc) \
	$(if $(filter YES,$(SYSTEM_LIBXML)),$(filter -I%,$(LIBXML_CFLAGS))) \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_set_defs,writerfilter_uno,\
	$$(DEFS) \
	-DWRITERFILTER_WRITERFILTER_UNO_DLLIMPLEMENTATION \
	$(if $(filter YES,$(SYSTEM_LIBXML)),$(filter-out -I%,$(LIBXML_CFLAGS))) \
))

$(eval $(call gb_Library_add_api,writerfilter_uno,\
    offapi \
    udkapi \
))

$(eval $(call gb_Library_add_linked_libs,writerfilter_uno,\
    comphelper \
    cppu \
    cppuhelper \
    doctok \
    ooxml \
    resourcemodel \
    sal \
    ucbhelper \
    $(gb_STDLIBS) \
))

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

# vim: set noet ts=4 sw=4:
