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

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,zxing,cpp))

$(eval $(call gb_StaticLibrary_use_external,zxing,icu_headers))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,zxing))

$(eval $(call gb_StaticLibrary_set_include,zxing,\
	-I$(call gb_UnpackedTarball_get_dir,zxing/core/src/) \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,zxing,\
	UnpackedTarball/zxing/core/src/aztec/AZReader \
	UnpackedTarball/zxing/core/src/aztec/AZToken \
	UnpackedTarball/zxing/core/src/aztec/AZEncoder \
	UnpackedTarball/zxing/core/src/aztec/AZDecoder \
	UnpackedTarball/zxing/core/src/aztec/AZDetector \
	UnpackedTarball/zxing/core/src/aztec/AZHighLevelEncoder \
	UnpackedTarball/zxing/core/src/aztec/AZWriter \
	UnpackedTarball/zxing/core/src/BarcodeFormat \
	UnpackedTarball/zxing/core/src/BitArray \
	UnpackedTarball/zxing/core/src/BitMatrixIO \
	UnpackedTarball/zxing/core/src/BitMatrix \
	UnpackedTarball/zxing/core/src/BitSource \
	UnpackedTarball/zxing/core/src/textcodec/Big5TextDecoder \
	UnpackedTarball/zxing/core/src/textcodec/Big5TextEncoder \
	UnpackedTarball/zxing/core/src/textcodec/Big5MapTable \
	UnpackedTarball/zxing/core/src/CharacterSetECI \
	UnpackedTarball/zxing/core/src/DecodeHints \
	UnpackedTarball/zxing/core/src/DecodeStatus \
	UnpackedTarball/zxing/core/src/datamatrix/DMWriter \
	UnpackedTarball/zxing/core/src/datamatrix/DMDefaultPlacement \
	UnpackedTarball/zxing/core/src/datamatrix/DMDetector \
	UnpackedTarball/zxing/core/src/datamatrix/DMVersion \
	UnpackedTarball/zxing/core/src/datamatrix/DMSymbolInfo \
	UnpackedTarball/zxing/core/src/datamatrix/DMDecoder \
	UnpackedTarball/zxing/core/src/datamatrix/DMBitMatrixParser \
	UnpackedTarball/zxing/core/src/datamatrix/DMReader \
	UnpackedTarball/zxing/core/src/datamatrix/DMDataBlock \
	UnpackedTarball/zxing/core/src/datamatrix/DMECEncoder \
	UnpackedTarball/zxing/core/src/datamatrix/DMHighLevelEncoder \
	UnpackedTarball/zxing/core/src/GlobalHistogramBinarizer \
	UnpackedTarball/zxing/core/src/GenericGF \
	UnpackedTarball/zxing/core/src/GridSampler \
	UnpackedTarball/zxing/core/src/GenericLuminanceSource \
	UnpackedTarball/zxing/core/src/GenericGFPoly \
	UnpackedTarball/zxing/core/src/textcodec/GBTextEncoder \
	UnpackedTarball/zxing/core/src/textcodec/GBTextDecoder \
	UnpackedTarball/zxing/core/src/HybridBinarizer \
	UnpackedTarball/zxing/core/src/textcodec/JPTextEncoder \
	UnpackedTarball/zxing/core/src/textcodec/JPTextDecoder \
	UnpackedTarball/zxing/core/src/textcodec/KRHangulMapping \
	UnpackedTarball/zxing/core/src/textcodec/KRTextEncoder \
	UnpackedTarball/zxing/core/src/textcodec/KRTextDecoder \
	UnpackedTarball/zxing/core/src/LuminanceSource \
	UnpackedTarball/zxing/core/src/MultiFormatWriter \
	UnpackedTarball/zxing/core/src/MultiFormatReader \
	UnpackedTarball/zxing/core/src/maxicode/MCDecoder \
	UnpackedTarball/zxing/core/src/maxicode/MCBitMatrixParser \
	UnpackedTarball/zxing/core/src/maxicode/MCReader \
	UnpackedTarball/zxing/core/src/oned/ODUPCEANExtensionSupport \
	UnpackedTarball/zxing/core/src/oned/ODUPCEWriter \
	UnpackedTarball/zxing/core/src/oned/ODEAN8Writer \
	UnpackedTarball/zxing/core/src/oned/ODWriterHelper \
	UnpackedTarball/zxing/core/src/oned/ODITFWriter \
	UnpackedTarball/zxing/core/src/oned/ODITFReader \
	UnpackedTarball/zxing/core/src/oned/ODCode39Reader \
	UnpackedTarball/zxing/core/src/oned/ODRSSExpandedReader \
	UnpackedTarball/zxing/core/src/oned/ODCode128Reader \
	UnpackedTarball/zxing/core/src/oned/ODEAN13Writer \
	UnpackedTarball/zxing/core/src/oned/ODEANManufacturerOrgSupport \
	UnpackedTarball/zxing/core/src/oned/ODMultiUPCEANReader \
	UnpackedTarball/zxing/core/src/oned/ODUPCEANReader \
	UnpackedTarball/zxing/core/src/oned/ODCodabarWriter \
	UnpackedTarball/zxing/core/src/oned/ODCode128Writer \
	UnpackedTarball/zxing/core/src/oned/ODCode93Reader \
	UnpackedTarball/zxing/core/src/oned/ODCodabarReader \
	UnpackedTarball/zxing/core/src/oned/ODCode39Writer \
	UnpackedTarball/zxing/core/src/oned/ODUPCEANCommon \
	UnpackedTarball/zxing/core/src/oned/ODRowReader \
	UnpackedTarball/zxing/core/src/oned/ODReader \
	UnpackedTarball/zxing/core/src/oned/ODCode93Writer \
	UnpackedTarball/zxing/core/src/oned/ODUPCEReader \
	UnpackedTarball/zxing/core/src/oned/ODCode128Patterns \
	UnpackedTarball/zxing/core/src/oned/ODEAN13Reader \
	UnpackedTarball/zxing/core/src/oned/ODUPCAWriter \
	UnpackedTarball/zxing/core/src/oned/ODRSS14Reader \
	UnpackedTarball/zxing/core/src/oned/ODUPCAReader \
	UnpackedTarball/zxing/core/src/oned/ODEAN8Reader \
	UnpackedTarball/zxing/core/src/PerspectiveTransform \
	UnpackedTarball/zxing/core/src/pdf417/PDFCodewordDecoder \
	UnpackedTarball/zxing/core/src/pdf417/PDFHighLevelEncoder \
	UnpackedTarball/zxing/core/src/pdf417/PDFDetectionResultColumn \
	UnpackedTarball/zxing/core/src/pdf417/PDFReader \
	UnpackedTarball/zxing/core/src/pdf417/PDFBoundingBox \
	UnpackedTarball/zxing/core/src/pdf417/PDFScanningDecoder \
	UnpackedTarball/zxing/core/src/pdf417/PDFModulusGF \
	UnpackedTarball/zxing/core/src/pdf417/PDFEncoder \
	UnpackedTarball/zxing/core/src/pdf417/PDFDecodedBitStreamParser \
	UnpackedTarball/zxing/core/src/pdf417/PDFWriter \
	UnpackedTarball/zxing/core/src/pdf417/PDFDetectionResult \
	UnpackedTarball/zxing/core/src/pdf417/PDFModulusPoly \
	UnpackedTarball/zxing/core/src/pdf417/PDFDetector \
	UnpackedTarball/zxing/core/src/pdf417/PDFBarcodeValue \
	UnpackedTarball/zxing/core/src/qrcode/QRFinderPatternFinder \
	UnpackedTarball/zxing/core/src/qrcode/QRDecoder \
	UnpackedTarball/zxing/core/src/qrcode/QRReader \
	UnpackedTarball/zxing/core/src/qrcode/QRErrorCorrectionLevel \
	UnpackedTarball/zxing/core/src/qrcode/QRAlignmentPattern \
	UnpackedTarball/zxing/core/src/qrcode/QRFormatInformation \
	UnpackedTarball/zxing/core/src/qrcode/QREncoder \
	UnpackedTarball/zxing/core/src/qrcode/QRFinderPattern \
	UnpackedTarball/zxing/core/src/qrcode/QRMaskUtil \
	UnpackedTarball/zxing/core/src/qrcode/QRWriter \
	UnpackedTarball/zxing/core/src/qrcode/QRDataMask \
	UnpackedTarball/zxing/core/src/qrcode/QRMatrixUtil \
	UnpackedTarball/zxing/core/src/qrcode/QRDetector \
	UnpackedTarball/zxing/core/src/qrcode/QRVersion \
	UnpackedTarball/zxing/core/src/qrcode/QRCodecMode \
	UnpackedTarball/zxing/core/src/qrcode/QRAlignmentPatternFinder \
	UnpackedTarball/zxing/core/src/qrcode/QRDataBlock \
	UnpackedTarball/zxing/core/src/qrcode/QRBitMatrixParser \
	UnpackedTarball/zxing/core/src/Result \
	UnpackedTarball/zxing/core/src/ReadBarcode \
	UnpackedTarball/zxing/core/src/ResultPoint \
	UnpackedTarball/zxing/core/src/ResultMetadata \
	UnpackedTarball/zxing/core/src/ReedSolomonDecoder \
	UnpackedTarball/zxing/core/src/ReedSolomonEncoder \
	UnpackedTarball/zxing/core/src/TextDecoder \
	UnpackedTarball/zxing/core/src/TextEncoder \
	UnpackedTarball/zxing/core/src/TextUtfEncoding \
	UnpackedTarball/zxing/core/src/WhiteRectDetector \
	UnpackedTarball/zxing/core/src/ZXBigInteger \
))

# vim: set noet sw=4 ts=4: