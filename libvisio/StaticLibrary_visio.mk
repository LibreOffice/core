# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,visiolib))

$(eval $(call gb_StaticLibrary_use_unpacked,visiolib,visio))

$(eval $(call gb_StaticLibrary_use_package,visiolib,libvisio_inc))

$(eval $(call gb_StaticLibrary_use_externals,visiolib,\
	wpd \
	wpg \
))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,visiolib,cpp))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,visiolib,\
	UnpackedTarball/visio/src/lib/libvisio_utils \
	UnpackedTarball/visio/src/lib/VisioDocument \
	UnpackedTarball/visio/src/lib/VSD11Parser \
	UnpackedTarball/visio/src/lib/VSD6Parser \
	UnpackedTarball/visio/src/lib/VSDInternalStream \
	UnpackedTarball/visio/src/lib/VSDStringVector \
	UnpackedTarball/visio/src/lib/VSDSVGGenerator \
	UnpackedTarball/visio/src/lib/VSDXCharacterList \
	UnpackedTarball/visio/src/lib/VSDXContentCollector \
	UnpackedTarball/visio/src/lib/VSDXFieldList \
	UnpackedTarball/visio/src/lib/VSDXGeometryList \
	UnpackedTarball/visio/src/lib/VSDXOutputElementList \
	UnpackedTarball/visio/src/lib/VSDXPages \
	UnpackedTarball/visio/src/lib/VSDXParagraphList \
	UnpackedTarball/visio/src/lib/VSDXParser \
	UnpackedTarball/visio/src/lib/VSDXShapeList \
	UnpackedTarball/visio/src/lib/VSDXStencils \
	UnpackedTarball/visio/src/lib/VSDXStyles \
	UnpackedTarball/visio/src/lib/VSDXStylesCollector \
))

# vim: set noet sw=4 ts=4:
