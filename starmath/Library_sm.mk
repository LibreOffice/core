#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Library_Library,sm))

$(eval $(call gb_Library_add_sdi_headers,sm,starmath/sdi/smslots))

$(eval $(call gb_Library_add_precompiled_header,sm,$(SRCDIR)/starmath/inc/pch/precompiled_starmath))

$(eval $(call gb_Library_set_componentfile,sm,starmath/util/sm))

$(eval $(call gb_Library_set_include,sm,\
	-I$(SRCDIR)/starmath/inc/pch \
	-I$(SRCDIR)/starmath/inc \
	-I$(WORKDIR)/SdiTarget/starmath/sdi \
	$$(INCLUDE) \
	-I$(OUTDIR)/inc/offuh \
	-I$(OUTDIR)/inc \
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
	stl \
	svl \
	svt \
	svx \
	svxcore \
	tk \
	tl \
	utl \
	vcl \
	xo \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,sm,\
	starmath/source/accessibility \
	starmath/source/action \
	starmath/source/cfgitem \
	starmath/source/config \
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
))

$(eval $(call gb_SdiTarget_SdiTarget,starmath/sdi/smslots,starmath/sdi/smath))

$(eval $(call gb_SdiTarget_set_include,starmath/sdi/smslots,\
	-I$(SRCDIR)/starmath/inc \
	-I$(SRCDIR)/starmath/sdi \
	$$(INCLUDE) \
))

# vim: set noet sw=4 ts=4:
