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

$(eval $(call gb_Library_Library,sm))

$(eval $(call gb_Library_add_sdi_headers,sm,starmath/sdi/smslots))

$(eval $(call gb_Library_add_precompiled_header,sm,$(SRCDIR)/starmath/inc/pch/precompiled_starmath))

$(eval $(call gb_Library_set_componentfile,sm,starmath/util/sm))

$(eval $(call gb_Library_set_include,sm,\
        -I$(SRCDIR)/starmath/inc/pch \
        -I$(SRCDIR)/starmath/inc \
        -I$(WORKDIR)/SdiTarget/starmath/sdi \
        -I$(WORKDIR)/Misc/starmath/ \
        $$(INCLUDE) \
        -I$(OUTDIR)/inc/offuh \
        -I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_set_defs,sm,\
        $$(DEFS) \
        -DSMDLL \
        -DSC_INFO_OSVERSION=\"$(OS)\" \
))

$(eval $(call gb_Library_add_linked_libs,sm,\
        comphelper \
        cppu \
        cppuhelper \
        editeng \
	i18npaper \
        sal \
        sfx \
        sot \
        svl \
        svt \
        svx \
        svxcore \
        tk \
        tl \
        utl \
        vcl \
	ucbhelper \
	xo \
))

$(eval $(call gb_Library_add_exception_objects,sm,\
        starmath/source/accessibility \
        starmath/source/action \
        starmath/source/caret \
        starmath/source/cfgitem \
        starmath/source/config \
        starmath/source/cursor \
        starmath/source/dialog \
        starmath/source/document \
        starmath/source/edit \
        starmath/source/eqnolefilehdr \
        starmath/source/format \
        starmath/source/mathmlexport \
        starmath/source/mathmlimport \
        starmath/source/mathtype \
        starmath/source/node \
        starmath/source/parse \
        starmath/source/rect \
        starmath/source/register \
	starmath/source/smdll \
        starmath/source/smmod \
        starmath/source/symbol \
        starmath/source/toolbox \
        starmath/source/typemap \
        starmath/source/types \
        starmath/source/unodoc \
        starmath/source/unomodel \
        starmath/source/utility \
        starmath/source/view \
        starmath/source/visitors \
))


$(eval $(call gb_SdiTarget_SdiTarget,starmath/sdi/smslots,starmath/sdi/smath))

$(eval $(call gb_SdiTarget_set_include,starmath/sdi/smslots,\
        -I$(SRCDIR)/starmath/inc \
        -I$(SRCDIR)/starmath/sdi \
        $$(INCLUDE) \
        -I$(OUTDIR)/inc \
))
