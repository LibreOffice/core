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

$(eval $(call gb_Library_Library,resourcemodel))

$(eval $(call gb_Library_use_custom_headers,resourcemodel,writerfilter/source))

$(eval $(call gb_Library_set_include,resourcemodel,\
    $$(INCLUDE) \
    -I$(SRCDIR)/writerfilter/inc \
))

include $(SRCDIR)/writerfilter/debug_setup.mk

$(eval $(call gb_Library_add_defs,resourcemodel,\
	-DWRITERFILTER_RESOURCEMODEL_DLLIMPLEMENTATION \
	$(writerfilter_debug_flags) \
))

$(eval $(call gb_Library_use_api,resourcemodel,\
    offapi \
    udkapi \
))

$(eval $(call gb_Library_use_libraries,resourcemodel,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_use_external,resourcemodel,libxml2))

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


ifneq ($(COM)-$(OS)-$(CPUNAME),GCC-LINUX-POWERPC64)
#Apparently some compilers, according to the original .mk this was converted
#from, require this to be noopt or they fail to compile it, probably good to
#revisit that and narrow this down to where it's necessary
$(eval $(call gb_Library_add_generated_cxxobjects,resourcemodel,\
    CustomTarget/writerfilter/source/qnametostr \
	, $(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) \
))
else
#Ironically, on RHEL-6 PPC64 with no-opt the output is too large for the
#toolchain, "Error: operand out of range", but it build fine with
#normal flags
$(eval $(call gb_Library_add_generated_cxxobjects,resourcemodel,\
    CustomTarget/writerfilter/source/qnametostr \
	, $(gb_LinkTarget_EXCEPTIONFLAGS) \
))
endif

# vim: set noet sw=4 ts=4:
