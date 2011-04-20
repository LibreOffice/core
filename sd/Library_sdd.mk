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

$(eval $(call gb_Library_Library,sdd))

$(eval $(call gb_Library_add_precompiled_header,sdd,$(SRCDIR)/sd/inc/pch/precompiled_sd))

$(eval $(call gb_Library_set_include,sdd,\
    $$(INCLUDE) \
    -I$(realpath $(SRCDIR)/sd/inc) \
    -I$(realpath $(SRCDIR)/sd/inc/pch) \
    -I$(OUTDIR)/inc \
    -I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_add_linked_libs,sdd,\
    cppu \
    cppuhelper \
    sal \
    sfx \
    sot \
    svl \
    svt \
    svxcore \
    tl \
    ucbhelper \
    utl \
    vcl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_set_componentfile,sdd,sd/util/sdd))

$(eval $(call gb_Library_add_exception_objects,sdd,\
    sd/source/ui/unoidl/detreg \
    sd/source/ui/unoidl/sddetect \
))

# vim: set noet ts=4 sw=4:
