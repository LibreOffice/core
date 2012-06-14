# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
# Norbert Thiebaud <nthiebaud@gmail.com> (C) 2010, All Rights Reserved.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,smd))

$(eval $(call gb_Library_add_precompiled_header,smd,$(SRCDIR)/starmath/inc/pch/precompiled_starmath))

$(eval $(call gb_Library_set_componentfile,smd,starmath/util/smd))

$(eval $(call gb_Library_set_include,smd,\
        -I$(SRCDIR)/starmath/inc/pch \
        -I$(SRCDIR)/starmath/inc \
        -I$(WORKDIR)/Misc/sm/ \
        $$(INCLUDE) \
        -I$(OUTDIR)/inc/offuh \
        -I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_set_defs,smd,\
        $$(DEFS) \
))

$(eval $(call gb_Library_add_linked_libs,smd,\
        cppu \
        cppuhelper \
        sal \
        sfx \
        sot \
        svl \
        svt \
        tl \
        ucbhelper \
        vcl \
))

$(eval $(call gb_Library_add_exception_objects,smd,\
	starmath/source/detreg \
	starmath/source/smdetect \
	starmath/source/eqnolefilehdr \
))
