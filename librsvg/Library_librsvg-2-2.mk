# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,librsvg-2-2))

$(eval $(call gb_Library_use_unpacked,librsvg-2-2,rsvg))

$(eval $(call gb_Library_use_externals,librsvg-2-2,\
	gobject \
	gio \
	glib \
	gthread \
	cairo \
	libxml2 \
	croco \
	pango \
	gsf \
	pixbuf \
))

# Generate a import library
$(eval $(call gb_Library_add_ldflags,librsvg-2-2,\
	-EXPORT:GetVersionInfo \
))

$(eval $(call gb_Library_add_defs,librsvg-2-2,\
	-DHAVE_GSF \
	-DHAVE_LIBCROCO \
))

$(eval $(call gb_Library_set_warnings_not_errors,librsvg-2-2))

$(eval $(call gb_Library_add_generated_cobjects,librsvg-2-2,\
	UnpackedTarball/rsvg/librsvg-enum-types \
	UnpackedTarball/rsvg/librsvg-features \
	UnpackedTarball/rsvg/rsvg-affine \
	UnpackedTarball/rsvg/rsvg-base-file-util \
	UnpackedTarball/rsvg/rsvg-base \
	UnpackedTarball/rsvg/rsvg-bpath-util \
	UnpackedTarball/rsvg/rsvg-cairo-clip \
	UnpackedTarball/rsvg/rsvg-cairo-draw \
	UnpackedTarball/rsvg/rsvg-cairo-render \
	UnpackedTarball/rsvg/rsvg-cond \
	UnpackedTarball/rsvg/rsvg-convert \
	UnpackedTarball/rsvg/rsvg-css \
	UnpackedTarball/rsvg/rsvg-defs \
	UnpackedTarball/rsvg/rsvg-file-util	\
	UnpackedTarball/rsvg/rsvg-filter \
	UnpackedTarball/rsvg/rsvg-gobject \
	UnpackedTarball/rsvg/rsvg-image \
	UnpackedTarball/rsvg/rsvg-marker \
	UnpackedTarball/rsvg/rsvg-mask \
	UnpackedTarball/rsvg/rsvg-paint-server \
	UnpackedTarball/rsvg/rsvg-path \
	UnpackedTarball/rsvg/rsvg-shapes \
	UnpackedTarball/rsvg/rsvg-structure	\
	UnpackedTarball/rsvg/rsvg-styles \
	UnpackedTarball/rsvg/rsvg-text \
	UnpackedTarball/rsvg/rsvg-xml \
	UnpackedTarball/rsvg/rsvg \
))


# vim: set noet sw=4 ts=4:
