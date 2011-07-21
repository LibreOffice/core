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

$(eval $(call gb_CppunitTest_CppunitTest,slideshow))

$(eval $(call gb_CppunitTest_add_package_headers,slideshow,sd_qa_unit))

$(eval $(call gb_CppunitTest_set_include,slideshow,\
    $$(INCLUDE) \
    -I$(realpath $(SRCDIR)/slideshow/inc/pch) \
    -I$(realpath $(SRCDIR)/slideshow/source/inc) \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_CppunitTest_set_defs,slideshow,\
    $$(DEFS) \
    $(if $(filter TRUE,$(VERBOSE)),-DVERBOSE) \
))

ifneq ($(strip $(debug)$(DEBUG)),)
$(eval $(call gb_CppunitTest_set_defs,slideshow,\
    $$(DEFS) \
    -DBOOST_SP_ENABLE_DEBUG_HOOKS \
))
endif

$(eval $(call gb_CppunitTest_add_api,slideshow,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_add_library_objects,slideshow,slideshow))

$(eval $(call gb_CppunitTest_add_linked_libs,slideshow,\
    avmedia \
    basegfx \
    canvastools \
    comphelper \
    cppcanvas \
    cppu \
    cppuhelper \
    sal \
    svt \
    tl \
    utl \
    vcl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_CppunitTest_add_exception_objects,slideshow,\
    slideshow/test/slidetest \
    slideshow/test/testshape \
    slideshow/test/testview \
    slideshow/test/views \
))

# vim: set noet sw=4:
