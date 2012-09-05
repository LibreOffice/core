# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
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
# [ Copyright (C) 2011 Red Hat, Inc., Michael Stahl <mstahl@redhat.com> (initial developer) ]
# [ Copyright (C) 2011 Peter Foley <pefoley2@verizon.net> ]
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
#

# TODO: do we really need 2 of these?
$(eval $(call gb_Library_Library,oleautobridge2))

$(eval $(call gb_Library_add_defs,oleautobridge2,\
	 -DOWNGUID \
))

$(eval $(call gb_Library_use_sdk_api,oleautobridge2))

$(eval $(call gb_Library_set_include,oleautobridge2,\
	-I$(SRCDIR)/extensions/source/ole \
	$(foreach inc,$(ATL_INCLUDE),-I$(inc)) \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_libraries,oleautobridge2,\
	comphelper \
	cppuhelper \
	cppu \
	sal \
	advapi32 \
	ole32 \
	oleaut32 \
	uuid \
))

ifeq ($(COM),MSC)
ifneq ($(USE_DEBUG_RUNTIME),)
$(eval $(call gb_Library_add_libs,oleautobridge2,\
	$(ATL_LIB)/atlsd.lib \
))
else
$(eval $(call gb_Library_add_libs,oleautobridge2,\
	$(ATL_LIB)/atls.lib \
))
endif
endif

$(WORKDIR)/CustomTarget/extensions/source/ole/%.cxx: $(SRCDIR)/extensions/source/ole/%.cxx
	mkdir -p $(dir $@)
	cp $< $@

$(eval $(call gb_Library_add_generated_exception_objects,oleautobridge2,\
	CustomTarget/extensions/source/ole/jscriptclasses \
	CustomTarget/extensions/source/ole/ole2uno \
	CustomTarget/extensions/source/ole/oledll \
	CustomTarget/extensions/source/ole/oleobjw \
	CustomTarget/extensions/source/ole/olethread \
	CustomTarget/extensions/source/ole/servprov \
	CustomTarget/extensions/source/ole/servreg \
	CustomTarget/extensions/source/ole/unoobjw \
	CustomTarget/extensions/source/ole/unotypewrapper \
	CustomTarget/extensions/source/ole/windata \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
