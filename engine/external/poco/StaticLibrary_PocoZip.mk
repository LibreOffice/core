# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,PocoZip))

$(eval $(call gb_StaticLibrary_use_unpacked,PocoZip,poco))

# keep the default std::vector ABI - the non-dbgutil online server links this
$(eval $(call gb_StaticLibrary_add_defs,PocoZip,\
    -U_GLIBCXX_DEBUG \
))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,PocoZip,cpp))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,PocoZip,\
    UnpackedTarball/poco/Zip/src/Add \
    UnpackedTarball/poco/Zip/src/AutoDetectStream \
    UnpackedTarball/poco/Zip/src/Compress \
    UnpackedTarball/poco/Zip/src/Decompress \
    UnpackedTarball/poco/Zip/src/Delete \
    UnpackedTarball/poco/Zip/src/Keep \
    UnpackedTarball/poco/Zip/src/ParseCallback \
    UnpackedTarball/poco/Zip/src/PartialStream \
    UnpackedTarball/poco/Zip/src/Rename \
    UnpackedTarball/poco/Zip/src/Replace \
    UnpackedTarball/poco/Zip/src/SkipCallback \
    UnpackedTarball/poco/Zip/src/ZipArchive \
    UnpackedTarball/poco/Zip/src/ZipArchiveInfo \
    UnpackedTarball/poco/Zip/src/ZipCommon \
    UnpackedTarball/poco/Zip/src/ZipDataInfo \
    UnpackedTarball/poco/Zip/src/ZipException \
    UnpackedTarball/poco/Zip/src/ZipFileInfo \
    UnpackedTarball/poco/Zip/src/ZipLocalFileHeader \
    UnpackedTarball/poco/Zip/src/ZipManipulator \
    UnpackedTarball/poco/Zip/src/ZipOperation \
    UnpackedTarball/poco/Zip/src/ZipStream \
    UnpackedTarball/poco/Zip/src/ZipUtil \
))

$(eval $(call gb_StaticLibrary_add_defs,PocoZip,\
    -DPOCO_STATIC \
    -DPOCO_NO_AUTOMATIC_LIBS \
    -UIOS \
))

$(eval $(call gb_StaticLibrary_set_include,PocoZip,\
    -I$(gb_UnpackedTarball_workdir)/poco/include \
    -I$(gb_UnpackedTarball_workdir)/poco/Zip/include/Poco/Zip \
    $$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,PocoZip))

# vim: set noet sw=4 ts=4:
