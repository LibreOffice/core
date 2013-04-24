# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,svg2odf))

$(eval $(call gb_Executable_set_targettype_gui,svg2odf,YES))

$(eval $(call gb_Executable_use_external,svg2odf,boost_headers))

$(eval $(call gb_Executable_use_sdk_api,svg2odf))

$(eval $(call gb_Executable_set_include,svg2odf,\
    $$(INCLUDE) \
    -I$(SRCDIR)/filter/inc \
))

$(eval $(call gb_Executable_use_libraries,svg2odf,\
    svgfilter \
	svxcore \
	editeng \
	xo \
	svt \
	vcl \
	svl \
	utl \
	tl \
	sax \
	comphelper \
	basegfx \
	cppuhelper \
	cppu \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_add_exception_objects,svg2odf,\
    filter/source/svg/test/svg2odf \
    filter/source/svg/test/odfserializer \
))

# vim: set ts=4 sw=4 et:
