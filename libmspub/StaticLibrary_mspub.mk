# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,mspublib))

$(eval $(call gb_StaticLibrary_use_unpacked,mspublib,mspub))

$(eval $(call gb_StaticLibrary_use_package,mspublib,libmspub_inc))

$(eval $(call gb_StaticLibrary_use_externals,mspublib,\
	wpd \
	wpg \
	zlib \
))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,mspublib))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,mspublib,cpp))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,mspublib,\
	UnpackedTarball/mspub/src/lib/ColorReference \
	UnpackedTarball/mspub/src/lib/Dash \
	UnpackedTarball/mspub/src/lib/Fill \
	UnpackedTarball/mspub/src/lib/MSPUBCollector \
	UnpackedTarball/mspub/src/lib/MSPUBDocument \
	UnpackedTarball/mspub/src/lib/MSPUBParser2k \
	UnpackedTarball/mspub/src/lib/MSPUBParser97 \
	UnpackedTarball/mspub/src/lib/MSPUBParser \
	UnpackedTarball/mspub/src/lib/MSPUBStringVector \
	UnpackedTarball/mspub/src/lib/MSPUBSVGGenerator \
	UnpackedTarball/mspub/src/lib/PolygonUtils \
	UnpackedTarball/mspub/src/lib/Shadow \
	UnpackedTarball/mspub/src/lib/ShapeGroupElement \
	UnpackedTarball/mspub/src/lib/VectorTransformation2D \
	UnpackedTarball/mspub/src/lib/libmspub_utils \
))

# vim: set noet sw=4 ts=4:
