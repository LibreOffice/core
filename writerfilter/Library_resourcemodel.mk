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

$(eval $(call gb_Library_Library,resourcemodel))

$(eval $(call gb_Library_set_include,resourcemodel,\
    $$(INCLUDE) \
    -I$(realpath $(SRCDIR)/writerfilter/inc) \
    -I$(WORKDIR)/CustomTarget/writerfilter/source \
    $(if $(filter YES,$(SYSTEM_LIBXML)),$(filter -I%,$(LIBXML_CFLAGS))) \
    -I$(OUTDIR)/inc \
))

include $(SRCDIR)/writerfilter/debug_setup.mk

$(eval $(call gb_Library_add_defs,resourcemodel,\
	-DWRITERFILTER_RESOURCEMODEL_DLLIMPLEMENTATION \
	$(writerfilter_debug_flags) \
    $(if $(filter YES,$(SYSTEM_LIBXML)),$(filter-out -I%,$(LIBXML_CFLAGS))) \
))

$(eval $(call gb_Library_add_api,resourcemodel,\
    offapi \
    udkapi \
))

$(eval $(call gb_Library_add_linked_libs,resourcemodel,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    xml2 \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,resourcemodel,\
    writerfilter/source/resourcemodel/Fraction \
    writerfilter/source/resourcemodel/LoggedResources \
    writerfilter/source/resourcemodel/Protocol \
    writerfilter/source/resourcemodel/resourcemodel \
    writerfilter/source/resourcemodel/ResourceModelHelper \
    writerfilter/source/resourcemodel/TagLogger \
    writerfilter/source/resourcemodel/util \
    writerfilter/source/resourcemodel/WW8Analyzer \
    writerfilter/source/resourcemodel/XPathLogger \
))

$(eval $(call gb_Library_add_generated_exception_objects,resourcemodel,\
    CustomTarget/writerfilter/source/sprmcodetostr \
))

$(eval $(call gb_Library_add_generated_cxxobjects,resourcemodel,\
    CustomTarget/writerfilter/source/qnametostr \
	, $(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) \
))

# vim: set noet ts=4 sw=4:
