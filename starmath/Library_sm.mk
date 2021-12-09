# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,sm))

$(eval $(call gb_Library_add_sdi_headers,sm,starmath/sdi/smslots))

$(eval $(call gb_Library_set_componentfile,sm,starmath/util/sm,services))

$(eval $(call gb_Library_set_precompiled_header,sm,starmath/inc/pch/precompiled_sm))

$(eval $(call gb_Library_set_include,sm,\
        -I$(SRCDIR)/starmath/inc \
        -I$(SRCDIR)/starmath/inc/mathml \
        -I$(WORKDIR)/SdiTarget/starmath/sdi \
        $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,sm,\
	-DSM_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_externals,sm, \
    boost_headers \
    icu_headers \
))

$(eval $(call gb_Library_use_custom_headers,sm,\
	officecfg/registry \
	oox/generated \
))

$(eval $(call gb_Library_use_sdk_api,sm))

$(eval $(call gb_Library_use_libraries,sm,\
        comphelper \
        cppu \
        cppuhelper \
        editeng \
        i18nutil \
		i18nlangtag \
        msfilter \
        oox \
        sal \
        salhelper \
        sax \
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
        xo \
))

$(eval $(call gb_Library_add_exception_objects,sm,\
        starmath/source/AccessibleSmElement \
        starmath/source/AccessibleSmElementsControl \
        starmath/source/ElementsDockingWindow \
        starmath/source/accessibility \
        starmath/source/action \
        starmath/source/caret \
        starmath/source/cfgitem \
        starmath/source/cursor \
        starmath/source/dialog \
        starmath/source/document \
        starmath/source/edit \
        starmath/source/smediteng \
        starmath/source/format \
        starmath/source/mathtype \
        starmath/source/node \
        starmath/source/ooxmlexport \
        starmath/source/ooxmlimport \
        starmath/source/rtfexport \
        starmath/source/parsebase \
        starmath/source/parse \
        starmath/source/parse5 \
        starmath/source/rect \
        starmath/source/smdll \
        starmath/source/smmod \
        starmath/source/symbol \
        starmath/source/tmpdevice \
        starmath/source/typemap \
        starmath/source/uiobject \
        starmath/source/unodoc \
        starmath/source/unofilter \
        starmath/source/unomodel \
        starmath/source/utility \
        starmath/source/view \
        starmath/source/visitors \
        starmath/source/wordexportbase \
        starmath/source/mathml/xparsmlbase \
        starmath/source/mathml/mathmlattr \
        starmath/source/mathml/mathmlexport \
        starmath/source/mathml/mathmlimport \
        starmath/source/mathml/mathmlMo \
        starmath/source/mathml/export \
        starmath/source/mathml/import \
        starmath/source/mathml/iterator \
        starmath/source/mathml/attribute \
        starmath/source/mathml/element \
        starmath/source/mathml/def \
        starmath/source/mathml/starmathdatabase \
))


$(eval $(call gb_SdiTarget_SdiTarget,starmath/sdi/smslots,starmath/sdi/smath))

$(eval $(call gb_SdiTarget_set_include,starmath/sdi/smslots,\
        -I$(SRCDIR)/starmath/inc \
        -I$(SRCDIR)/starmath/sdi \
        -I$(SRCDIR)/svx/sdi \
        -I$(SRCDIR)/sfx2/sdi \
        $$(INCLUDE) \
))

# vim: set noet sw=4 ts=4:
