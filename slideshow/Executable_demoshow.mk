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

$(eval $(call gb_Executable_Executable,demoshow))

$(eval $(call gb_Executable_add_package_headers,demoshow,sd_qa_unit))

$(eval $(call gb_Executable_set_include,demoshow,\
    $$(INCLUDE) \
    -I$(realpath $(SRCDIR)/slideshow/inc/pch) \
    -I$(realpath $(SRCDIR)/slideshow/source/inc) \
    -I$(OUTDIR)/inc \
    -I$(OUTDIR)/inc/offuh \
))

ifeq ($(strip $(VERBOSE)),TRUE)
$(eval $(call gb_Executable_set_defs,demoshow,\
    $$(DEFS) \
    -DVERBOSE \
))
endif

ifneq ($(strip $(debug)$(DEBUG)),)
$(eval $(call gb_Executable_set_defs,demoshow,\
    $$(DEFS) \
    -DBOOST_SP_ENABLE_DEBUG_HOOKS \
))
endif

$(eval $(call gb_Executable_add_linked_libs,demoshow,\
    basegfx \
    comphelper \
    cppcanvas \
    cppu \
    cppuhelper \
    sal \
    tl \
    ucbhelper \
    vcl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Executable_add_exception_objects,demoshow,\
    slideshow/test/demoshow \
))

# vim: set noet sw=4:
