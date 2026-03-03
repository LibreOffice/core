# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,zxing))

$(eval $(call gb_StaticLibrary_use_unpacked,zxing,zxing))

$(eval $(call gb_StaticLibrary_set_precompiled_header,zxing,external/zxing/inc/pch/precompiled_zxing))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,zxing,cpp))

$(eval $(call gb_StaticLibrary_use_external,zxing,icu_headers))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,zxing))

$(eval $(call gb_StaticLibrary_set_include,zxing,\
	-I$(gb_UnpackedTarball_workdir)/zxing/core/src/ \
	-I$(gb_UnpackedTarball_workdir)/zxing/core/src/libzint/ \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_cxxflags,zxing,\
	-DZXING_INTERNAL \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,zxing,\
	UnpackedTarball/zxing/core/src/Barcode \
	UnpackedTarball/zxing/core/src/BarcodeFormat \
	UnpackedTarball/zxing/core/src/BinaryBitmap \
	UnpackedTarball/zxing/core/src/BitArray \
	UnpackedTarball/zxing/core/src/BitMatrix \
	UnpackedTarball/zxing/core/src/BitMatrixIO \
	UnpackedTarball/zxing/core/src/BitSource \
	UnpackedTarball/zxing/core/src/CharacterSet \
	UnpackedTarball/zxing/core/src/ConcentricFinder \
	UnpackedTarball/zxing/core/src/Content \
	UnpackedTarball/zxing/core/src/CreateBarcode \
	UnpackedTarball/zxing/core/src/ECI \
	UnpackedTarball/zxing/core/src/Error \
	UnpackedTarball/zxing/core/src/GTIN \
	UnpackedTarball/zxing/core/src/GenericGF \
	UnpackedTarball/zxing/core/src/GenericGFPoly \
	UnpackedTarball/zxing/core/src/GlobalHistogramBinarizer \
	UnpackedTarball/zxing/core/src/GridSampler \
	UnpackedTarball/zxing/core/src/HRI \
	UnpackedTarball/zxing/core/src/HybridBinarizer \
	UnpackedTarball/zxing/core/src/JSON \
	UnpackedTarball/zxing/core/src/MultiFormatReader \
	UnpackedTarball/zxing/core/src/MultiFormatWriter \
	UnpackedTarball/zxing/core/src/PerspectiveTransform \
	UnpackedTarball/zxing/core/src/ReadBarcode \
	UnpackedTarball/zxing/core/src/ReedSolomonDecoder \
	UnpackedTarball/zxing/core/src/ReedSolomonEncoder \
	UnpackedTarball/zxing/core/src/ResultPoint \
	UnpackedTarball/zxing/core/src/TextDecoder \
	UnpackedTarball/zxing/core/src/TextEncoder \
	UnpackedTarball/zxing/core/src/Utf \
	UnpackedTarball/zxing/core/src/WhiteRectDetector \
	UnpackedTarball/zxing/core/src/WriteBarcode \
	UnpackedTarball/zxing/core/src/ZXingC \
	UnpackedTarball/zxing/core/src/ZXingCpp \
	UnpackedTarball/zxing/core/src/maxicode/MCBitMatrixParser \
	UnpackedTarball/zxing/core/src/maxicode/MCDecoder \
	UnpackedTarball/zxing/core/src/maxicode/MCReader \
	UnpackedTarball/zxing/core/src/oned/ODCodabarReader \
	UnpackedTarball/zxing/core/src/oned/ODCodabarWriter \
	UnpackedTarball/zxing/core/src/oned/ODCode128Patterns \
	UnpackedTarball/zxing/core/src/oned/ODCode128Reader \
	UnpackedTarball/zxing/core/src/oned/ODCode128Writer \
	UnpackedTarball/zxing/core/src/oned/ODCode39Reader \
	UnpackedTarball/zxing/core/src/oned/ODCode39Writer \
	UnpackedTarball/zxing/core/src/oned/ODCode93Reader \
	UnpackedTarball/zxing/core/src/oned/ODCode93Writer \
	UnpackedTarball/zxing/core/src/oned/ODDXFilmEdgeReader \
	UnpackedTarball/zxing/core/src/oned/ODDataBarCommon \
	UnpackedTarball/zxing/core/src/oned/ODDataBarExpandedBitDecoder \
	UnpackedTarball/zxing/core/src/oned/ODDataBarExpandedReader \
	UnpackedTarball/zxing/core/src/oned/ODDataBarLimitedReader \
	UnpackedTarball/zxing/core/src/oned/ODDataBarReader \
	UnpackedTarball/zxing/core/src/oned/ODEAN13Writer \
	UnpackedTarball/zxing/core/src/oned/ODEAN8Writer \
	UnpackedTarball/zxing/core/src/oned/ODITFReader \
	UnpackedTarball/zxing/core/src/oned/ODITFWriter \
	UnpackedTarball/zxing/core/src/oned/ODMultiUPCEANReader \
	UnpackedTarball/zxing/core/src/oned/ODReader \
	UnpackedTarball/zxing/core/src/oned/ODUPCAWriter \
	UnpackedTarball/zxing/core/src/oned/ODUPCEANCommon \
	UnpackedTarball/zxing/core/src/oned/ODUPCEWriter \
	UnpackedTarball/zxing/core/src/oned/ODWriterHelper \
	UnpackedTarball/zxing/core/src/qrcode/QRBitMatrixParser \
	UnpackedTarball/zxing/core/src/qrcode/QRCodecMode \
	UnpackedTarball/zxing/core/src/qrcode/QRDataBlock \
	UnpackedTarball/zxing/core/src/qrcode/QRDecoder \
	UnpackedTarball/zxing/core/src/qrcode/QRDetector \
	UnpackedTarball/zxing/core/src/qrcode/QREncoder \
	UnpackedTarball/zxing/core/src/qrcode/QRErrorCorrectionLevel \
	UnpackedTarball/zxing/core/src/qrcode/QRFormatInformation \
	UnpackedTarball/zxing/core/src/qrcode/QRMaskUtil \
	UnpackedTarball/zxing/core/src/qrcode/QRMatrixUtil \
	UnpackedTarball/zxing/core/src/qrcode/QRReader \
	UnpackedTarball/zxing/core/src/qrcode/QRVersion \
	UnpackedTarball/zxing/core/src/qrcode/QRWriter \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,zxing,\
	UnpackedTarball/zxing/core/src/libzint/2of5inter_based \
	UnpackedTarball/zxing/core/src/libzint/2of5inter \
	UnpackedTarball/zxing/core/src/libzint/codabar \
	UnpackedTarball/zxing/core/src/libzint/code128 \
	UnpackedTarball/zxing/core/src/libzint/code \
	UnpackedTarball/zxing/core/src/libzint/common \
	UnpackedTarball/zxing/core/src/libzint/dxfilmedge \
	UnpackedTarball/zxing/core/src/libzint/eci \
	UnpackedTarball/zxing/core/src/libzint/filemem \
	UnpackedTarball/zxing/core/src/libzint/general_field \
	UnpackedTarball/zxing/core/src/libzint/gs1 \
	UnpackedTarball/zxing/core/src/libzint/large \
	UnpackedTarball/zxing/core/src/libzint/library \
	UnpackedTarball/zxing/core/src/libzint/medical \
	UnpackedTarball/zxing/core/src/libzint/output \
	UnpackedTarball/zxing/core/src/libzint/qr \
	UnpackedTarball/zxing/core/src/libzint/raster \
	UnpackedTarball/zxing/core/src/libzint/reedsol \
	UnpackedTarball/zxing/core/src/libzint/rss \
	UnpackedTarball/zxing/core/src/libzint/stubs \
	UnpackedTarball/zxing/core/src/libzint/svg \
	UnpackedTarball/zxing/core/src/libzint/upcean \
	UnpackedTarball/zxing/core/src/libzint/vector \
))

# vim: set noet sw=4 ts=4:
