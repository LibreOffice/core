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
#       Bjoern Michaelsen, Canonical Ltd. <bjoern.michaelsen@canonical.com>
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

$(eval $(call gb_StaticLibrary_StaticLibrary,toolshelpers))

$(eval $(call gb_StaticLibrary_set_include,toolshelpers,\
    $$(INCLUDE) \
    -I$(realpath $(SRCDIR)/tools/inc) \
    -I$(realpath $(SRCDIR)/tools/inc/pch) \
    -I$(realpath $(SRCDIR)/tools/bootstrp) \
))

$(eval $(call gb_StaticLibrary_add_cxxflags,toolshelpers,\
    -D_TOOLS_STRINGLIST \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,toolshelpers,\
    tools/bootstrp/appdef \
    tools/bootstrp/cppdep \
    tools/bootstrp/prj \
))

ifeq ($(OS),WNT)
ifeq ($(HAVE_GETOPT),YES)
$(eval $(call gb_StaticLibrary_set_cxxflags,toolshelpers,\
    $$(CXXFLAGS) \
    -DHAVE_GETOPT \
))
else
$(eval $(call gb_StaticLibrary_add_linked_libs,toolshelpers,\
    gnu_getopt \
))
endif
endif

# vim: set noet sw=4 ts=4:
