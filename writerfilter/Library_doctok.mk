# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
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

$(eval $(call gb_Library_Library,doctok))

$(eval $(call gb_Library_set_include,doctok,\
    $$(INCLUDE) \
    -I$(realpath $(SRCDIR)/writerfilter/inc) \
    -I$(realpath $(SRCDIR)/writerfilter/source) \
    -I$(realpath $(SRCDIR)/writerfilter/source/doctok) \
    -I$(WORKDIR)/CustomTarget/writerfilter/source \
    -I$(WORKDIR)/CustomTarget/writerfilter/source/ooxml \
    -I$(WORKDIR)/CustomTarget/writerfilter/source/doctok \
    -I$(OUTDIR)/inc \
))

include $(realpath $(SRCDIR)/writerfilter/debug_setup.mk)

$(eval $(call gb_Library_add_defs,doctok,\
	-DWRITERFILTER_DOCTOK_DLLIMPLEMENTATION \
	$(writerfilter_debug_flags) \
))

$(eval $(call gb_Library_add_api,doctok,\
    offapi \
    udkapi \
))

$(eval $(call gb_Library_add_linked_libs,doctok,\
    cppu \
    cppuhelper \
    resourcemodel \
    sal \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_use_external,doctok,libxml2))

$(eval $(call gb_Library_add_exception_objects,doctok,\
    writerfilter/source/doctok/Dff \
    writerfilter/source/doctok/DffImpl \
    writerfilter/source/doctok/WW8Annotation \
    writerfilter/source/doctok/WW8BinTableImpl \
    writerfilter/source/doctok/WW8Clx \
    writerfilter/source/doctok/WW8CpAndFc \
    writerfilter/source/doctok/WW8DocumentImpl \
    writerfilter/source/doctok/WW8FKPImpl \
    writerfilter/source/doctok/WW8FontTable \
    writerfilter/source/doctok/WW8LFOTable \
    writerfilter/source/doctok/WW8ListTable \
    writerfilter/source/doctok/WW8OutputWithDepth \
    writerfilter/source/doctok/WW8Picture \
    writerfilter/source/doctok/WW8PieceTableImpl \
    writerfilter/source/doctok/WW8PropertySetImpl \
    writerfilter/source/doctok/WW8ResourceModelImpl \
    writerfilter/source/doctok/WW8StreamImpl \
    writerfilter/source/doctok/WW8StructBase \
    writerfilter/source/doctok/WW8Sttbf \
    writerfilter/source/doctok/WW8StyleSheet \
    writerfilter/source/doctok/WW8Table \
    writerfilter/source/doctok/WW8Text \
))

$(eval $(call gb_Library_add_generated_exception_objects,doctok,\
	CustomTarget/writerfilter/source/resources \
))

# vim: set noet sw=4 ts=4:
