# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,skia))

$(eval $(call gb_Library_set_warnings_disabled,skia))

$(eval $(call gb_Library_use_unpacked,skia,skia))

#$(eval $(call gb_Library_set_precompiled_header,skia,$(SRCDIR)/external/skia/inc/pch/precompiled_skia))

$(eval $(call gb_Library_add_defs,skia,\
	-DSK_GAMMA_SRGB \
	-DSK_GAMMA_APPLY_TO_A8 \
	-DSK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1 \
	-DSK_SCALAR_IS_FLOAT \
	-DSK_CAN_USE_FLOAT \
	-DSK_BUILD_FOR_UNIX \
	-DSK_USE_POSIX_THREADS \
	-DSK_RELEASE \
	-DGR_RELEASE=1 \
	-DNDEBUG \
	-DSKIA_DLL \
	-DSK_HAS_JPEG_LIBRARY=1 \
	-DSK_HAS_PNG_LIBRARY=1 \
))

# TODO
$(eval $(call gb_Library_add_defs,skia,\
    -DSK_USER_CONFIG_HEADER="<$(SRCDIR)/external/skia/configs/SkUserConfig.h>" \
))

# TODO
$(eval $(call gb_Library_add_defs,skia,\
	-DSK_SUPPORT_GPU=1 \
	-DSK_GL=1 \
	-DSK_VULKAN=1 \
))

$(eval $(call gb_Library_use_externals,skia,\
	freetype \
	fontconfig \
	zlib \
	libjpeg \
	libpng \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,skia,\
	-lm \
	-ldl \
	-lGLX \
))
endif

$(eval $(call gb_Library_use_libraries,skia,\
    sal \
))

$(eval $(call gb_Library_add_cxxflags,skia, \
	-mssse3 \
))

$(eval $(call gb_Library_set_include,skia,\
	$$(INCLUDE) \
	-I$(call gb_UnpackedTarball_get_dir,skia) \
	-I$(call gb_UnpackedTarball_get_dir,skia)/include/third_party/skcms/ \
	-I$(call gb_UnpackedTarball_get_dir,skia)/third_party/vulkanmemoryallocator/ \
))

$(eval $(call gb_Library_add_exception_objects,skia,\
	external/skia/source/SkMemory_malloc \
))

$(eval $(call gb_Library_set_generated_cxx_suffix,skia,cpp))

$(eval $(call gb_Library_add_generated_exception_objects,skia,\
       UnpackedTarball/skia/src/codec/SkAndroidCodecAdapter \
       UnpackedTarball/skia/src/codec/SkAndroidCodec \
       UnpackedTarball/skia/src/codec/SkBmpBaseCodec \
       UnpackedTarball/skia/src/codec/SkBmpCodec \
       UnpackedTarball/skia/src/codec/SkBmpMaskCodec \
       UnpackedTarball/skia/src/codec/SkBmpRLECodec \
       UnpackedTarball/skia/src/codec/SkBmpStandardCodec \
       UnpackedTarball/skia/src/codec/SkCodec \
       UnpackedTarball/skia/src/codec/SkCodecImageGenerator \
       UnpackedTarball/skia/src/codec/SkColorTable \
       UnpackedTarball/skia/src/codec/SkEncodedInfo \
       UnpackedTarball/skia/src/codec/SkGifCodec \
       UnpackedTarball/skia/src/codec/SkIcoCodec \
       UnpackedTarball/skia/src/codec/SkJpegCodec \
       UnpackedTarball/skia/src/codec/SkJpegDecoderMgr \
       UnpackedTarball/skia/src/codec/SkJpegUtility \
       UnpackedTarball/skia/src/codec/SkMasks \
       UnpackedTarball/skia/src/codec/SkMaskSwizzler \
       UnpackedTarball/skia/src/codec/SkOrientationMarker \
       UnpackedTarball/skia/src/codec/SkPngCodec \
       UnpackedTarball/skia/src/codec/SkSampledCodec \
       UnpackedTarball/skia/src/codec/SkSampler \
       UnpackedTarball/skia/src/codec/SkStreamBuffer \
       UnpackedTarball/skia/src/codec/SkSwizzler \
       UnpackedTarball/skia/src/codec/SkWbmpCodec \
       UnpackedTarball/skia/src/core/SkAAClip \
       UnpackedTarball/skia/src/core/SkAlphaRuns \
       UnpackedTarball/skia/src/core/SkAnalyticEdge \
       UnpackedTarball/skia/src/core/SkAnnotation \
       UnpackedTarball/skia/src/core/SkArenaAlloc \
       UnpackedTarball/skia/src/core/SkATrace \
       UnpackedTarball/skia/src/core/SkAutoPixmapStorage \
       UnpackedTarball/skia/src/core/SkBBHFactory \
       UnpackedTarball/skia/src/core/SkBigPicture \
       UnpackedTarball/skia/src/core/SkBitmapCache \
       UnpackedTarball/skia/src/core/SkBitmapController \
       UnpackedTarball/skia/src/core/SkBitmap \
       UnpackedTarball/skia/src/core/SkBitmapDevice \
       UnpackedTarball/skia/src/core/SkBitmapProcState \
       UnpackedTarball/skia/src/core/SkBitmapProcState_matrixProcs \
       UnpackedTarball/skia/src/core/SkBitmapProvider \
       UnpackedTarball/skia/src/core/SkBlendMode \
       UnpackedTarball/skia/src/core/SkBlitRow_D32 \
       UnpackedTarball/skia/src/core/SkBlitter_ARGB32 \
       UnpackedTarball/skia/src/core/SkBlitter_A8 \
       UnpackedTarball/skia/src/core/SkBlitter \
       UnpackedTarball/skia/src/core/SkBlitter_RGB565 \
       UnpackedTarball/skia/src/core/SkBlitter_Sprite \
       UnpackedTarball/skia/src/core/SkBlurMask \
       UnpackedTarball/skia/src/core/SkBlurMF \
       UnpackedTarball/skia/src/core/SkBuffer \
       UnpackedTarball/skia/src/core/SkCachedData \
       UnpackedTarball/skia/src/core/SkCanvas \
       UnpackedTarball/skia/src/core/SkCanvasPriv \
       UnpackedTarball/skia/src/core/SkClipStack \
       UnpackedTarball/skia/src/core/SkClipStackDevice \
       UnpackedTarball/skia/src/core/SkColor \
       UnpackedTarball/skia/src/core/SkColorFilter \
       UnpackedTarball/skia/src/core/SkColorFilter_Matrix \
       UnpackedTarball/skia/src/core/SkColorSpace \
       UnpackedTarball/skia/src/core/SkColorSpaceXformSteps \
       UnpackedTarball/skia/src/core/SkContourMeasure \
       UnpackedTarball/skia/src/core/SkConvertPixels \
       UnpackedTarball/skia/src/core/SkCpu \
       UnpackedTarball/skia/src/core/SkCubicClipper \
       UnpackedTarball/skia/src/core/SkCubicMap \
       UnpackedTarball/skia/src/core/SkData \
       UnpackedTarball/skia/src/core/SkDataTable \
       UnpackedTarball/skia/src/core/SkDebug \
       UnpackedTarball/skia/src/core/SkDeferredDisplayList \
       UnpackedTarball/skia/src/core/SkDeferredDisplayListRecorder \
       UnpackedTarball/skia/src/core/SkDeque \
       UnpackedTarball/skia/src/core/SkDescriptor \
       UnpackedTarball/skia/src/core/SkDevice \
       UnpackedTarball/skia/src/core/SkDistanceFieldGen \
       UnpackedTarball/skia/src/core/SkDocument \
       UnpackedTarball/skia/src/core/SkDrawable \
       UnpackedTarball/skia/src/core/SkDraw \
       UnpackedTarball/skia/src/core/SkDrawLooper \
       UnpackedTarball/skia/src/core/SkDrawShadowInfo \
       UnpackedTarball/skia/src/core/SkDraw_text \
       UnpackedTarball/skia/src/core/SkDraw_vertices \
       UnpackedTarball/skia/src/core/SkEdgeBuilder \
       UnpackedTarball/skia/src/core/SkEdgeClipper \
       UnpackedTarball/skia/src/core/SkEdge \
       UnpackedTarball/skia/src/core/SkExecutor \
       UnpackedTarball/skia/src/core/SkFlattenable \
       UnpackedTarball/skia/src/core/SkFont \
       UnpackedTarball/skia/src/core/SkFontDescriptor \
       UnpackedTarball/skia/src/core/SkFontLCDConfig \
       UnpackedTarball/skia/src/core/SkFontMgr \
       UnpackedTarball/skia/src/core/SkFontStream \
       UnpackedTarball/skia/src/core/SkForceCPlusPlusLinking \
       UnpackedTarball/skia/src/core/SkGaussFilter \
       UnpackedTarball/skia/src/core/SkGeometry \
       UnpackedTarball/skia/src/core/SkGlobalInitialization_core \
       UnpackedTarball/skia/src/core/SkGlyph \
       UnpackedTarball/skia/src/core/SkGlyphRun \
       UnpackedTarball/skia/src/core/SkGlyphRunPainter \
       UnpackedTarball/skia/src/core/SkGraphics \
       UnpackedTarball/skia/src/core/SkHalf \
       UnpackedTarball/skia/src/core/SkICC \
       UnpackedTarball/skia/src/core/SkImageFilterCache \
       UnpackedTarball/skia/src/core/SkImageFilter \
       UnpackedTarball/skia/src/core/SkImageGenerator \
       UnpackedTarball/skia/src/core/SkImageInfo \
       UnpackedTarball/skia/src/core/SkLatticeIter \
       UnpackedTarball/skia/src/core/SkLineClipper \
       UnpackedTarball/skia/src/core/SkLocalMatrixImageFilter \
       UnpackedTarball/skia/src/core/SkMallocPixelRef \
       UnpackedTarball/skia/src/core/SkMaskBlurFilter \
       UnpackedTarball/skia/src/core/SkMaskCache \
       UnpackedTarball/skia/src/core/SkMask \
       UnpackedTarball/skia/src/core/SkMaskFilter \
       UnpackedTarball/skia/src/core/SkMaskGamma \
       UnpackedTarball/skia/src/core/SkMath \
       UnpackedTarball/skia/src/core/SkMatrix \
       UnpackedTarball/skia/src/core/SkMatrixImageFilter \
       UnpackedTarball/skia/src/core/SkMatrix44 \
       UnpackedTarball/skia/src/core/SkMD5 \
       UnpackedTarball/skia/src/core/SkMiniRecorder \
       UnpackedTarball/skia/src/core/SkMipMap \
       UnpackedTarball/skia/src/core/SkModeColorFilter \
       UnpackedTarball/skia/src/core/SkMultiPictureDraw \
       UnpackedTarball/skia/src/core/SkNormalFlatSource \
       UnpackedTarball/skia/src/core/SkNormalMapSource \
       UnpackedTarball/skia/src/core/SkNormalSource \
       UnpackedTarball/skia/src/core/SkOpts \
       UnpackedTarball/skia/src/core/SkOverdrawCanvas \
       UnpackedTarball/skia/src/core/SkPaint \
       UnpackedTarball/skia/src/core/SkPaintPriv \
       UnpackedTarball/skia/src/core/SkPath \
       UnpackedTarball/skia/src/core/SkPathEffect \
       UnpackedTarball/skia/src/core/SkPathMeasure \
       UnpackedTarball/skia/src/core/SkPathRef \
       UnpackedTarball/skia/src/core/SkPath_serial \
       UnpackedTarball/skia/src/core/SkPicture \
       UnpackedTarball/skia/src/core/SkPictureData \
       UnpackedTarball/skia/src/core/SkPictureFlat \
       UnpackedTarball/skia/src/core/SkPictureImageGenerator \
       UnpackedTarball/skia/src/core/SkPicturePlayback \
       UnpackedTarball/skia/src/core/SkPictureRecord \
       UnpackedTarball/skia/src/core/SkPictureRecorder \
       UnpackedTarball/skia/src/core/SkPixelRef \
       UnpackedTarball/skia/src/core/SkPixmap \
       UnpackedTarball/skia/src/core/SkPoint \
       UnpackedTarball/skia/src/core/SkPoint3 \
       UnpackedTarball/skia/src/core/SkPromiseImageTexture \
       UnpackedTarball/skia/src/core/SkPtrRecorder \
       UnpackedTarball/skia/src/core/SkQuadClipper \
       UnpackedTarball/skia/src/core/SkRasterClip \
       UnpackedTarball/skia/src/core/SkRasterPipelineBlitter \
       UnpackedTarball/skia/src/core/SkRasterPipeline \
       UnpackedTarball/skia/src/core/SkReadBuffer \
       UnpackedTarball/skia/src/core/SkRecord \
       UnpackedTarball/skia/src/core/SkRecordDraw \
       UnpackedTarball/skia/src/core/SkRecordedDrawable \
       UnpackedTarball/skia/src/core/SkRecorder \
       UnpackedTarball/skia/src/core/SkRecordOpts \
       UnpackedTarball/skia/src/core/SkRecords \
       UnpackedTarball/skia/src/core/SkRect \
       UnpackedTarball/skia/src/core/SkRegion \
       UnpackedTarball/skia/src/core/SkRegion_path \
       UnpackedTarball/skia/src/core/SkRemoteGlyphCache \
       UnpackedTarball/skia/src/core/SkResourceCache \
       UnpackedTarball/skia/src/core/SkRRect \
       UnpackedTarball/skia/src/core/SkRTree \
       UnpackedTarball/skia/src/core/SkRWBuffer \
       UnpackedTarball/skia/src/core/SkScalar \
       UnpackedTarball/skia/src/core/SkScalerContext \
       UnpackedTarball/skia/src/core/SkScan_AAAPath \
       UnpackedTarball/skia/src/core/SkScan_Antihair \
       UnpackedTarball/skia/src/core/SkScan_AntiPath \
       UnpackedTarball/skia/src/core/SkScan \
       UnpackedTarball/skia/src/core/SkScan_Hairline \
       UnpackedTarball/skia/src/core/SkScan_Path \
       UnpackedTarball/skia/src/core/SkSemaphore \
       UnpackedTarball/skia/src/core/SkSharedMutex \
       UnpackedTarball/skia/src/core/SkSpecialImage \
       UnpackedTarball/skia/src/core/SkSpecialSurface \
       UnpackedTarball/skia/src/core/SkSpinlock \
       UnpackedTarball/skia/src/core/SkSpriteBlitter_ARGB32 \
       UnpackedTarball/skia/src/core/SkSpriteBlitter_RGB565 \
       UnpackedTarball/skia/src/core/SkStream \
       UnpackedTarball/skia/src/core/SkStrikeCache \
       UnpackedTarball/skia/src/core/SkStrike \
       UnpackedTarball/skia/src/core/SkStrikeSpec \
       UnpackedTarball/skia/src/core/SkString \
       UnpackedTarball/skia/src/core/SkStringUtils \
       UnpackedTarball/skia/src/core/SkStroke \
       UnpackedTarball/skia/src/core/SkStrokeRec \
       UnpackedTarball/skia/src/core/SkStrokerPriv \
       UnpackedTarball/skia/src/core/SkSurfaceCharacterization \
       UnpackedTarball/skia/src/core/SkSwizzle \
       UnpackedTarball/skia/src/core/SkTaskGroup \
       UnpackedTarball/skia/src/core/SkTextBlob \
       UnpackedTarball/skia/src/core/SkThreadID \
       UnpackedTarball/skia/src/core/SkTime \
       UnpackedTarball/skia/src/core/SkTLS \
       UnpackedTarball/skia/src/core/SkTSearch \
       UnpackedTarball/skia/src/core/SkTypefaceCache \
       UnpackedTarball/skia/src/core/SkTypeface \
       UnpackedTarball/skia/src/core/SkTypeface_remote \
       UnpackedTarball/skia/src/core/SkUnPreMultiply \
       UnpackedTarball/skia/src/core/SkUtilsArm \
       UnpackedTarball/skia/src/core/SkUtils \
       UnpackedTarball/skia/src/core/SkVertices \
       UnpackedTarball/skia/src/core/SkVertState \
       UnpackedTarball/skia/src/core/SkVM \
       UnpackedTarball/skia/src/core/SkWriteBuffer \
       UnpackedTarball/skia/src/core/SkWriter32 \
       UnpackedTarball/skia/src/core/SkXfermode \
       UnpackedTarball/skia/src/core/SkXfermodeInterpretation \
       UnpackedTarball/skia/src/core/SkYUVASizeInfo \
       UnpackedTarball/skia/src/core/SkYUVMath \
       UnpackedTarball/skia/src/core/SkYUVPlanesCache \
       UnpackedTarball/skia/src/c/sk_effects \
       UnpackedTarball/skia/src/c/sk_imageinfo \
       UnpackedTarball/skia/src/c/sk_paint \
       UnpackedTarball/skia/src/c/sk_surface \
       UnpackedTarball/skia/src/effects/imagefilters/SkAlphaThresholdFilter \
       UnpackedTarball/skia/src/effects/imagefilters/SkArithmeticImageFilter \
       UnpackedTarball/skia/src/effects/imagefilters/SkBlurImageFilter \
       UnpackedTarball/skia/src/effects/imagefilters/SkColorFilterImageFilter \
       UnpackedTarball/skia/src/effects/imagefilters/SkComposeImageFilter \
       UnpackedTarball/skia/src/effects/imagefilters/SkDisplacementMapEffect \
       UnpackedTarball/skia/src/effects/imagefilters/SkDropShadowImageFilter \
       UnpackedTarball/skia/src/effects/imagefilters/SkImageSource \
       UnpackedTarball/skia/src/effects/imagefilters/SkLightingImageFilter \
       UnpackedTarball/skia/src/effects/imagefilters/SkMagnifierImageFilter \
       UnpackedTarball/skia/src/effects/imagefilters/SkMatrixConvolutionImageFilter \
       UnpackedTarball/skia/src/effects/imagefilters/SkMergeImageFilter \
       UnpackedTarball/skia/src/effects/imagefilters/SkMorphologyImageFilter \
       UnpackedTarball/skia/src/effects/imagefilters/SkOffsetImageFilter \
       UnpackedTarball/skia/src/effects/imagefilters/SkPaintImageFilter \
       UnpackedTarball/skia/src/effects/imagefilters/SkPictureImageFilter \
       UnpackedTarball/skia/src/effects/imagefilters/SkTileImageFilter \
       UnpackedTarball/skia/src/effects/imagefilters/SkXfermodeImageFilter \
       UnpackedTarball/skia/src/effects/SkColorMatrix \
       UnpackedTarball/skia/src/effects/SkColorMatrixFilter \
       UnpackedTarball/skia/src/effects/SkCornerPathEffect \
       UnpackedTarball/skia/src/effects/SkDashPathEffect \
       UnpackedTarball/skia/src/effects/SkDiscretePathEffect \
       UnpackedTarball/skia/src/effects/SkEmbossMask \
       UnpackedTarball/skia/src/effects/SkEmbossMaskFilter \
       UnpackedTarball/skia/src/effects/SkHighContrastFilter \
       UnpackedTarball/skia/src/effects/SkLayerDrawLooper \
       UnpackedTarball/skia/src/effects/SkLumaColorFilter \
       UnpackedTarball/skia/src/effects/SkOpPathEffect \
       UnpackedTarball/skia/src/effects/SkOverdrawColorFilter \
       UnpackedTarball/skia/src/effects/SkPackBits \
       UnpackedTarball/skia/src/effects/SkShaderMaskFilter \
       UnpackedTarball/skia/src/effects/SkTableColorFilter \
       UnpackedTarball/skia/src/effects/SkTableMaskFilter \
       UnpackedTarball/skia/src/effects/SkTrimPathEffect \
       UnpackedTarball/skia/src/effects/Sk1DPathEffect \
       UnpackedTarball/skia/src/effects/Sk2DPathEffect \
       UnpackedTarball/skia/src/fonts/SkFontMgr_indirect \
       UnpackedTarball/skia/src/fonts/SkRemotableFontMgr \
       UnpackedTarball/skia/src/image/SkImage \
       UnpackedTarball/skia/src/image/SkImage_Lazy \
       UnpackedTarball/skia/src/image/SkImage_Raster \
       UnpackedTarball/skia/src/image/SkSurface \
       UnpackedTarball/skia/src/image/SkSurface_Raster \
       UnpackedTarball/skia/src/images/SkImageEncoder \
       UnpackedTarball/skia/src/images/SkJpegEncoder \
       UnpackedTarball/skia/src/images/SkJPEGWriteUtility \
       UnpackedTarball/skia/src/images/SkPngEncoder \
       UnpackedTarball/skia/src/images/SkWebpEncoder \
       UnpackedTarball/skia/src/lazy/SkDiscardableMemoryPool \
       UnpackedTarball/skia/src/pathops/SkAddIntersections \
       UnpackedTarball/skia/src/pathops/SkDConicLineIntersection \
       UnpackedTarball/skia/src/pathops/SkDCubicLineIntersection \
       UnpackedTarball/skia/src/pathops/SkDCubicToQuads \
       UnpackedTarball/skia/src/pathops/SkDLineIntersection \
       UnpackedTarball/skia/src/pathops/SkDQuadLineIntersection \
       UnpackedTarball/skia/src/pathops/SkIntersections \
       UnpackedTarball/skia/src/pathops/SkOpAngle \
       UnpackedTarball/skia/src/pathops/SkOpBuilder \
       UnpackedTarball/skia/src/pathops/SkOpCoincidence \
       UnpackedTarball/skia/src/pathops/SkOpContour \
       UnpackedTarball/skia/src/pathops/SkOpCubicHull \
       UnpackedTarball/skia/src/pathops/SkOpEdgeBuilder \
       UnpackedTarball/skia/src/pathops/SkOpSegment \
       UnpackedTarball/skia/src/pathops/SkOpSpan \
       UnpackedTarball/skia/src/pathops/SkPathOpsAsWinding \
       UnpackedTarball/skia/src/pathops/SkPathOpsCommon \
       UnpackedTarball/skia/src/pathops/SkPathOpsConic \
       UnpackedTarball/skia/src/pathops/SkPathOpsCubic \
       UnpackedTarball/skia/src/pathops/SkPathOpsCurve \
       UnpackedTarball/skia/src/pathops/SkPathOpsDebug \
       UnpackedTarball/skia/src/pathops/SkPathOpsLine \
       UnpackedTarball/skia/src/pathops/SkPathOpsOp \
       UnpackedTarball/skia/src/pathops/SkPathOpsQuad \
       UnpackedTarball/skia/src/pathops/SkPathOpsRect \
       UnpackedTarball/skia/src/pathops/SkPathOpsSimplify \
       UnpackedTarball/skia/src/pathops/SkPathOpsTightBounds \
       UnpackedTarball/skia/src/pathops/SkPathOpsTSect \
       UnpackedTarball/skia/src/pathops/SkPathOpsTypes \
       UnpackedTarball/skia/src/pathops/SkPathOpsWinding \
       UnpackedTarball/skia/src/pathops/SkPathWriter \
       UnpackedTarball/skia/src/pathops/SkReduceOrder \
       UnpackedTarball/skia/src/pdf/SkClusterator \
       UnpackedTarball/skia/src/pdf/SkDeflate \
       UnpackedTarball/skia/src/pdf/SkJpegInfo \
       UnpackedTarball/skia/src/pdf/SkKeyedImage \
       UnpackedTarball/skia/src/pdf/SkPDFBitmap \
       UnpackedTarball/skia/src/pdf/SkPDFDevice \
       UnpackedTarball/skia/src/pdf/SkPDFDocument \
       UnpackedTarball/skia/src/pdf/SkPDFFont \
       UnpackedTarball/skia/src/pdf/SkPDFFormXObject \
       UnpackedTarball/skia/src/pdf/SkPDFGradientShader \
       UnpackedTarball/skia/src/pdf/SkPDFGraphicStackState \
       UnpackedTarball/skia/src/pdf/SkPDFGraphicState \
       UnpackedTarball/skia/src/pdf/SkPDFMakeCIDGlyphWidthsArray \
       UnpackedTarball/skia/src/pdf/SkPDFMakeToUnicodeCmap \
       UnpackedTarball/skia/src/pdf/SkPDFMetadata \
       UnpackedTarball/skia/src/pdf/SkPDFResourceDict \
       UnpackedTarball/skia/src/pdf/SkPDFShader \
       UnpackedTarball/skia/src/pdf/SkPDFSubsetFont \
       UnpackedTarball/skia/src/pdf/SkPDFTag \
       UnpackedTarball/skia/src/pdf/SkPDFTypes \
       UnpackedTarball/skia/src/pdf/SkPDFType1Font \
       UnpackedTarball/skia/src/pdf/SkPDFUtils \
       UnpackedTarball/skia/src/sfnt/SkOTTable_name \
       UnpackedTarball/skia/src/sfnt/SkOTUtils \
       UnpackedTarball/skia/src/shaders/gradients/SkGradientShader \
       UnpackedTarball/skia/src/shaders/gradients/SkLinearGradient \
       UnpackedTarball/skia/src/shaders/gradients/SkRadialGradient \
       UnpackedTarball/skia/src/shaders/gradients/SkSweepGradient \
       UnpackedTarball/skia/src/shaders/gradients/SkTwoPointConicalGradient \
       UnpackedTarball/skia/src/shaders/gradients/Sk4fGradientBase \
       UnpackedTarball/skia/src/shaders/gradients/Sk4fLinearGradient \
       UnpackedTarball/skia/src/shaders/SkBitmapProcShader \
       UnpackedTarball/skia/src/shaders/SkColorFilterShader \
       UnpackedTarball/skia/src/shaders/SkColorShader \
       UnpackedTarball/skia/src/shaders/SkComposeShader \
       UnpackedTarball/skia/src/shaders/SkImageShader \
       UnpackedTarball/skia/src/shaders/SkLightingShader \
       UnpackedTarball/skia/src/shaders/SkLights \
       UnpackedTarball/skia/src/shaders/SkLocalMatrixShader \
       UnpackedTarball/skia/src/shaders/SkPerlinNoiseShader \
       UnpackedTarball/skia/src/shaders/SkPictureShader \
       UnpackedTarball/skia/src/shaders/SkRTShader \
       UnpackedTarball/skia/src/shaders/SkShader \
       UnpackedTarball/skia/src/sksl/ir/SkSLSetting \
       UnpackedTarball/skia/src/sksl/ir/SkSLSymbolTable \
       UnpackedTarball/skia/src/sksl/ir/SkSLType \
       UnpackedTarball/skia/src/sksl/ir/SkSLVariableReference \
       UnpackedTarball/skia/src/sksl/SkSLASTNode \
       UnpackedTarball/skia/src/sksl/SkSLByteCode \
       UnpackedTarball/skia/src/sksl/SkSLByteCodeGenerator \
       UnpackedTarball/skia/src/sksl/SkSLCFGGenerator \
       UnpackedTarball/skia/src/sksl/SkSLCompiler \
       UnpackedTarball/skia/src/sksl/SkSLCPPCodeGenerator \
       UnpackedTarball/skia/src/sksl/SkSLCPPUniformCTypes \
       UnpackedTarball/skia/src/sksl/SkSLGLSLCodeGenerator \
       UnpackedTarball/skia/src/sksl/SkSLHCodeGenerator \
       UnpackedTarball/skia/src/sksl/SkSLIRGenerator \
       UnpackedTarball/skia/src/sksl/SkSLJIT \
       UnpackedTarball/skia/src/sksl/SkSLLexer \
       UnpackedTarball/skia/src/sksl/SkSLMetalCodeGenerator \
       UnpackedTarball/skia/src/sksl/SkSLOutputStream \
       UnpackedTarball/skia/src/sksl/SkSLParser \
       UnpackedTarball/skia/src/sksl/SkSLPipelineStageCodeGenerator \
       UnpackedTarball/skia/src/sksl/SkSLSPIRVCodeGenerator \
       UnpackedTarball/skia/src/sksl/SkSLString \
       UnpackedTarball/skia/src/sksl/SkSLUtil \
       UnpackedTarball/skia/src/utils/SkBase64 \
       UnpackedTarball/skia/src/utils/SkCamera \
       UnpackedTarball/skia/src/utils/SkCanvasStack \
       UnpackedTarball/skia/src/utils/SkCanvasStateUtils \
       UnpackedTarball/skia/src/utils/SkDashPath \
       UnpackedTarball/skia/src/utils/SkEventTracer \
       UnpackedTarball/skia/src/utils/SkFloatToDecimal \
       UnpackedTarball/skia/src/utils/SkFrontBufferedStream \
       UnpackedTarball/skia/src/utils/SkCharToGlyphCache \
       UnpackedTarball/skia/src/utils/SkInterpolator \
       UnpackedTarball/skia/src/utils/SkJSON \
       UnpackedTarball/skia/src/utils/SkJSONWriter \
       UnpackedTarball/skia/src/utils/SkMatrix22 \
       UnpackedTarball/skia/src/utils/SkMultiPictureDocument \
       UnpackedTarball/skia/src/utils/SkNullCanvas \
       UnpackedTarball/skia/src/utils/SkNWayCanvas \
       UnpackedTarball/skia/src/utils/SkOSPath \
       UnpackedTarball/skia/src/utils/SkPaintFilterCanvas \
       UnpackedTarball/skia/src/utils/SkParseColor \
       UnpackedTarball/skia/src/utils/SkParse \
       UnpackedTarball/skia/src/utils/SkParsePath \
       UnpackedTarball/skia/src/utils/SkPatchUtils \
       UnpackedTarball/skia/src/utils/SkPolyUtils \
       UnpackedTarball/skia/src/utils/SkShadowTessellator \
       UnpackedTarball/skia/src/utils/SkShadowUtils \
       UnpackedTarball/skia/src/utils/SkShaperJSONWriter \
       UnpackedTarball/skia/src/utils/SkTextUtils \
       UnpackedTarball/skia/src/utils/SkThreadUtils_pthread \
       UnpackedTarball/skia/src/utils/SkThreadUtils_win \
       UnpackedTarball/skia/src/utils/SkUTF \
       UnpackedTarball/skia/src/utils/SkWhitelistTypefaces \
       UnpackedTarball/skia/src/utils/Sk3D \
       UnpackedTarball/skia/src/xps/SkXPSDevice \
       UnpackedTarball/skia/src/xps/SkXPSDocument \
))

$(eval $(call gb_Library_add_generated_exception_objects,skia,\
       UnpackedTarball/skia/src/opts/SkOpts_avx \
       UnpackedTarball/skia/src/opts/SkOpts_crc32 \
       UnpackedTarball/skia/src/opts/SkOpts_hsw \
       UnpackedTarball/skia/src/opts/SkOpts_sse41 \
       UnpackedTarball/skia/src/opts/SkOpts_sse42 \
       UnpackedTarball/skia/src/opts/SkOpts_ssse3 \
       UnpackedTarball/skia/src/ports/SkDebug_stdio \
       UnpackedTarball/skia/src/ports/SkGlobalInitialization_default \
       UnpackedTarball/skia/src/ports/SkFontHost_FreeType_common \
       UnpackedTarball/skia/src/ports/SkFontHost_FreeType \
       UnpackedTarball/skia/src/ports/SkFontMgr_fontconfig \
       UnpackedTarball/skia/src/ports/SkFontMgr_fontconfig_factory \
       UnpackedTarball/skia/src/ports/SkImageGenerator_none \
       UnpackedTarball/skia/src/ports/SkOSFile_posix \
       UnpackedTarball/skia/src/ports/SkOSFile_stdio \
       UnpackedTarball/skia/src/ports/SkOSLibrary_posix \
       UnpackedTarball/skia/src/ports/SkTLS_pthread \
))

$(eval $(call gb_Library_add_generated_exception_objects,skia,\
       UnpackedTarball/skia/src/core/SkGpuBlurUtils \
       UnpackedTarball/skia/src/gpu/ccpr/GrCCAtlas \
       UnpackedTarball/skia/src/gpu/ccpr/GrCCClipPath \
       UnpackedTarball/skia/src/gpu/ccpr/GrCCClipProcessor \
       UnpackedTarball/skia/src/gpu/ccpr/GrCCConicShader \
       UnpackedTarball/skia/src/gpu/ccpr/GrCCCoverageProcessor \
       UnpackedTarball/skia/src/gpu/ccpr/GrCCCubicShader \
       UnpackedTarball/skia/src/gpu/ccpr/GrCCDrawPathsOp \
       UnpackedTarball/skia/src/gpu/ccpr/GrCCFiller \
       UnpackedTarball/skia/src/gpu/ccpr/GrCCFillGeometry \
       UnpackedTarball/skia/src/gpu/ccpr/GrCCPathCache \
       UnpackedTarball/skia/src/gpu/ccpr/GrCCPathProcessor \
       UnpackedTarball/skia/src/gpu/ccpr/GrCCPerFlushResources \
       UnpackedTarball/skia/src/gpu/ccpr/GrCCQuadraticShader \
       UnpackedTarball/skia/src/gpu/ccpr/GrCCStrokeGeometry \
       UnpackedTarball/skia/src/gpu/ccpr/GrCCStroker \
       UnpackedTarball/skia/src/gpu/ccpr/GrCoverageCountingPathRenderer \
       UnpackedTarball/skia/src/gpu/ccpr/GrGSCoverageProcessor \
       UnpackedTarball/skia/src/gpu/ccpr/GrOctoBounds \
       UnpackedTarball/skia/src/gpu/ccpr/GrSampleMaskProcessor \
       UnpackedTarball/skia/src/gpu/ccpr/GrStencilAtlasOp \
       UnpackedTarball/skia/src/gpu/ccpr/GrVSCoverageProcessor \
       UnpackedTarball/skia/src/gpu/effects/generated/GrAARectEffect \
       UnpackedTarball/skia/src/gpu/effects/generated/GrAlphaThresholdFragmentProcessor \
       UnpackedTarball/skia/src/gpu/effects/generated/GrBlurredEdgeFragmentProcessor \
       UnpackedTarball/skia/src/gpu/effects/generated/GrCircleBlurFragmentProcessor \
       UnpackedTarball/skia/src/gpu/effects/generated/GrCircleEffect \
       UnpackedTarball/skia/src/gpu/effects/generated/GrColorMatrixFragmentProcessor \
       UnpackedTarball/skia/src/gpu/effects/generated/GrComposeLerpEffect \
       UnpackedTarball/skia/src/gpu/effects/generated/GrComposeLerpRedEffect \
       UnpackedTarball/skia/src/gpu/effects/generated/GrConfigConversionEffect \
       UnpackedTarball/skia/src/gpu/effects/generated/GrConstColorProcessor \
       UnpackedTarball/skia/src/gpu/effects/generated/GrEllipseEffect \
       UnpackedTarball/skia/src/gpu/effects/generated/GrLumaColorFilterEffect \
       UnpackedTarball/skia/src/gpu/effects/generated/GrMagnifierEffect \
       UnpackedTarball/skia/src/gpu/effects/generated/GrMixerEffect \
       UnpackedTarball/skia/src/gpu/effects/generated/GrOverrideInputFragmentProcessor \
       UnpackedTarball/skia/src/gpu/effects/generated/GrPremulInputFragmentProcessor \
       UnpackedTarball/skia/src/gpu/effects/generated/GrRectBlurEffect \
       UnpackedTarball/skia/src/gpu/effects/generated/GrRRectBlurEffect \
       UnpackedTarball/skia/src/gpu/effects/generated/GrSimpleTextureEffect \
       UnpackedTarball/skia/src/gpu/effects/GrBezierEffect \
       UnpackedTarball/skia/src/gpu/effects/GrBicubicEffect \
       UnpackedTarball/skia/src/gpu/effects/GrBitmapTextGeoProc \
       UnpackedTarball/skia/src/gpu/effects/GrConvexPolyEffect \
       UnpackedTarball/skia/src/gpu/effects/GrCoverageSetOpXP \
       UnpackedTarball/skia/src/gpu/effects/GrCustomXfermode \
       UnpackedTarball/skia/src/gpu/effects/GrDisableColorXP \
       UnpackedTarball/skia/src/gpu/effects/GrDistanceFieldGeoProc \
       UnpackedTarball/skia/src/gpu/effects/GrGaussianConvolutionFragmentProcessor \
       UnpackedTarball/skia/src/gpu/effects/GrMatrixConvolutionEffect \
       UnpackedTarball/skia/src/gpu/effects/GrOvalEffect \
       UnpackedTarball/skia/src/gpu/effects/GrPorterDuffXferProcessor \
       UnpackedTarball/skia/src/gpu/effects/GrRRectEffect \
       UnpackedTarball/skia/src/gpu/effects/GrShadowGeoProc \
       UnpackedTarball/skia/src/gpu/effects/GrSkSLFP \
       UnpackedTarball/skia/src/gpu/effects/GrSRGBEffect \
       UnpackedTarball/skia/src/gpu/effects/GrTextureDomain \
       UnpackedTarball/skia/src/gpu/effects/GrXfermodeFragmentProcessor \
       UnpackedTarball/skia/src/gpu/effects/GrYUVtoRGBEffect \
       UnpackedTarball/skia/src/gpu/geometry/GrPathUtils \
       UnpackedTarball/skia/src/gpu/geometry/GrQuad \
       UnpackedTarball/skia/src/gpu/geometry/GrQuadUtils \
       UnpackedTarball/skia/src/gpu/geometry/GrShape \
       UnpackedTarball/skia/src/gpu/gl/builders/GrGLProgramBuilder \
       UnpackedTarball/skia/src/gpu/gl/builders/GrGLShaderStringBuilder \
       UnpackedTarball/skia/src/gpu/gl/GrGLAssembleGLESInterfaceAutogen \
       UnpackedTarball/skia/src/gpu/gl/GrGLAssembleGLInterfaceAutogen \
       UnpackedTarball/skia/src/gpu/gl/GrGLAssembleHelpers \
       UnpackedTarball/skia/src/gpu/gl/GrGLAssembleInterface \
       UnpackedTarball/skia/src/gpu/gl/GrGLAssembleWebGLInterfaceAutogen \
       UnpackedTarball/skia/src/gpu/gl/GrGLBuffer \
       UnpackedTarball/skia/src/gpu/gl/GrGLCaps \
       UnpackedTarball/skia/src/gpu/gl/GrGLContext \
       UnpackedTarball/skia/src/gpu/gl/GrGLExtensions \
       UnpackedTarball/skia/src/gpu/gl/GrGLGLSL \
       UnpackedTarball/skia/src/gpu/gl/GrGLGpuCommandBuffer \
       UnpackedTarball/skia/src/gpu/gl/GrGLGpu \
       UnpackedTarball/skia/src/gpu/gl/GrGLGpuProgramCache \
       UnpackedTarball/skia/src/gpu/gl/GrGLInterfaceAutogen \
       UnpackedTarball/skia/src/gpu/gl/GrGLPath \
       UnpackedTarball/skia/src/gpu/gl/GrGLPathRendering \
       UnpackedTarball/skia/src/gpu/gl/GrGLProgram \
       UnpackedTarball/skia/src/gpu/gl/GrGLProgramDataManager \
       UnpackedTarball/skia/src/gpu/gl/GrGLRenderTarget \
       UnpackedTarball/skia/src/gpu/gl/GrGLSemaphore \
       UnpackedTarball/skia/src/gpu/gl/GrGLStencilAttachment \
       UnpackedTarball/skia/src/gpu/gl/GrGLTexture \
       UnpackedTarball/skia/src/gpu/gl/GrGLTextureRenderTarget \
       UnpackedTarball/skia/src/gpu/gl/GrGLTypesPriv \
       UnpackedTarball/skia/src/gpu/gl/GrGLUniformHandler \
       UnpackedTarball/skia/src/gpu/gl/GrGLUtil \
       UnpackedTarball/skia/src/gpu/gl/GrGLVaryingHandler \
       UnpackedTarball/skia/src/gpu/gl/GrGLVertexArray \
       UnpackedTarball/skia/src/gpu/glsl/GrGLSLBlend \
       UnpackedTarball/skia/src/gpu/glsl/GrGLSL \
       UnpackedTarball/skia/src/gpu/glsl/GrGLSLFragmentProcessor \
       UnpackedTarball/skia/src/gpu/glsl/GrGLSLFragmentShaderBuilder \
       UnpackedTarball/skia/src/gpu/glsl/GrGLSLGeometryProcessor \
       UnpackedTarball/skia/src/gpu/glsl/GrGLSLPrimitiveProcessor \
       UnpackedTarball/skia/src/gpu/glsl/GrGLSLProgramBuilder \
       UnpackedTarball/skia/src/gpu/glsl/GrGLSLProgramDataManager \
       UnpackedTarball/skia/src/gpu/glsl/GrGLSLShaderBuilder \
       UnpackedTarball/skia/src/gpu/glsl/GrGLSLUtil \
       UnpackedTarball/skia/src/gpu/glsl/GrGLSLVarying \
       UnpackedTarball/skia/src/gpu/glsl/GrGLSLVertexGeoBuilder \
       UnpackedTarball/skia/src/gpu/glsl/GrGLSLXferProcessor \
       UnpackedTarball/skia/src/gpu/gradients/generated/GrClampedGradientEffect \
       UnpackedTarball/skia/src/gpu/gradients/generated/GrDualIntervalGradientColorizer \
       UnpackedTarball/skia/src/gpu/gradients/generated/GrLinearGradientLayout \
       UnpackedTarball/skia/src/gpu/gradients/generated/GrRadialGradientLayout \
       UnpackedTarball/skia/src/gpu/gradients/generated/GrSingleIntervalGradientColorizer \
       UnpackedTarball/skia/src/gpu/gradients/generated/GrSweepGradientLayout \
       UnpackedTarball/skia/src/gpu/gradients/generated/GrTextureGradientColorizer \
       UnpackedTarball/skia/src/gpu/gradients/generated/GrTiledGradientEffect \
       UnpackedTarball/skia/src/gpu/gradients/generated/GrTwoPointConicalGradientLayout \
       UnpackedTarball/skia/src/gpu/gradients/generated/GrUnrolledBinaryGradientColorizer \
       UnpackedTarball/skia/src/gpu/gradients/GrGradientBitmapCache \
       UnpackedTarball/skia/src/gpu/gradients/GrGradientShader \
       UnpackedTarball/skia/src/gpu/GrAHardwareBufferImageGenerator \
       UnpackedTarball/skia/src/gpu/GrAHardwareBufferUtils \
       UnpackedTarball/skia/src/gpu/GrAuditTrail \
       UnpackedTarball/skia/src/gpu/GrBackendSurface \
       UnpackedTarball/skia/src/gpu/GrBackendTextureImageGenerator \
       UnpackedTarball/skia/src/gpu/GrBitmapTextureMaker \
       UnpackedTarball/skia/src/gpu/GrBlurUtils \
       UnpackedTarball/skia/src/gpu/GrBufferAllocPool \
       UnpackedTarball/skia/src/gpu/GrCaps \
       UnpackedTarball/skia/src/gpu/GrClipStackClip \
       UnpackedTarball/skia/src/gpu/GrColorSpaceInfo \
       UnpackedTarball/skia/src/gpu/GrColorSpaceXform \
       UnpackedTarball/skia/src/gpu/GrContext_Base \
       UnpackedTarball/skia/src/gpu/GrContext \
       UnpackedTarball/skia/src/gpu/GrContextPriv \
       UnpackedTarball/skia/src/gpu/GrContextThreadSafeProxy \
       UnpackedTarball/skia/src/gpu/GrDataUtils \
       UnpackedTarball/skia/src/gpu/GrDDLContext \
       UnpackedTarball/skia/src/gpu/GrDefaultGeoProcFactory \
       UnpackedTarball/skia/src/gpu/GrDeinstantiateProxyTracker \
       UnpackedTarball/skia/src/gpu/GrDistanceFieldGenFromVector \
       UnpackedTarball/skia/src/gpu/GrDrawingManager \
       UnpackedTarball/skia/src/gpu/GrDrawOpAtlas \
       UnpackedTarball/skia/src/gpu/GrDrawOpTest \
       UnpackedTarball/skia/src/gpu/GrDriverBugWorkarounds \
       UnpackedTarball/skia/src/gpu/GrFixedClip \
       UnpackedTarball/skia/src/gpu/GrFragmentProcessor \
       UnpackedTarball/skia/src/gpu/GrGpuBuffer \
       UnpackedTarball/skia/src/gpu/GrGpuCommandBuffer \
       UnpackedTarball/skia/src/gpu/GrGpu \
       UnpackedTarball/skia/src/gpu/GrGpuResource \
       UnpackedTarball/skia/src/gpu/GrImageContext \
       UnpackedTarball/skia/src/gpu/GrImageTextureMaker \
       UnpackedTarball/skia/src/gpu/GrLegacyDirectContext \
       UnpackedTarball/skia/src/gpu/GrMemoryPool \
       UnpackedTarball/skia/src/gpu/GrOnFlushResourceProvider \
       UnpackedTarball/skia/src/gpu/GrOpFlushState \
       UnpackedTarball/skia/src/gpu/GrOpList \
       UnpackedTarball/skia/src/gpu/GrPaint \
       UnpackedTarball/skia/src/gpu/GrPath \
       UnpackedTarball/skia/src/gpu/GrPathProcessor \
       UnpackedTarball/skia/src/gpu/GrPathRenderer \
       UnpackedTarball/skia/src/gpu/GrPathRendererChain \
       UnpackedTarball/skia/src/gpu/GrPathRendering \
       UnpackedTarball/skia/src/gpu/GrPipeline \
       UnpackedTarball/skia/src/gpu/GrPrimitiveProcessor \
       UnpackedTarball/skia/src/gpu/GrProcessorAnalysis \
       UnpackedTarball/skia/src/gpu/GrProcessor \
       UnpackedTarball/skia/src/gpu/GrProcessorSet \
       UnpackedTarball/skia/src/gpu/GrProcessorUnitTest \
       UnpackedTarball/skia/src/gpu/GrProgramDesc \
       UnpackedTarball/skia/src/gpu/GrProxyProvider \
       UnpackedTarball/skia/src/gpu/GrRecordingContext \
       UnpackedTarball/skia/src/gpu/GrRectanizer_pow2 \
       UnpackedTarball/skia/src/gpu/GrRectanizer_skyline \
       UnpackedTarball/skia/src/gpu/GrReducedClip \
       UnpackedTarball/skia/src/gpu/GrRenderTargetContext \
       UnpackedTarball/skia/src/gpu/GrRenderTarget \
       UnpackedTarball/skia/src/gpu/GrRenderTargetOpList \
       UnpackedTarball/skia/src/gpu/GrRenderTargetProxy \
       UnpackedTarball/skia/src/gpu/GrResourceAllocator \
       UnpackedTarball/skia/src/gpu/GrResourceCache \
       UnpackedTarball/skia/src/gpu/GrResourceProvider \
       UnpackedTarball/skia/src/gpu/GrSamplePatternDictionary \
       UnpackedTarball/skia/src/gpu/GrShaderCaps \
       UnpackedTarball/skia/src/gpu/GrShaderUtils \
       UnpackedTarball/skia/src/gpu/GrShaderVar \
       UnpackedTarball/skia/src/gpu/GrSoftwarePathRenderer \
       UnpackedTarball/skia/src/gpu/GrStencilAttachment \
       UnpackedTarball/skia/src/gpu/GrStencilSettings \
       UnpackedTarball/skia/src/gpu/GrStyle \
       UnpackedTarball/skia/src/gpu/GrSurfaceContext \
       UnpackedTarball/skia/src/gpu/GrSurface \
       UnpackedTarball/skia/src/gpu/GrSurfaceProxy \
       UnpackedTarball/skia/src/gpu/GrSwizzle \
       UnpackedTarball/skia/src/gpu/GrSWMaskHelper \
       UnpackedTarball/skia/src/gpu/GrTessellator \
       UnpackedTarball/skia/src/gpu/GrTestUtils \
       UnpackedTarball/skia/src/gpu/GrTextureAdjuster \
       UnpackedTarball/skia/src/gpu/GrTextureContext \
       UnpackedTarball/skia/src/gpu/GrTexture \
       UnpackedTarball/skia/src/gpu/GrTextureMaker \
       UnpackedTarball/skia/src/gpu/GrTextureOpList \
       UnpackedTarball/skia/src/gpu/GrTextureProducer \
       UnpackedTarball/skia/src/gpu/GrTextureProxy \
       UnpackedTarball/skia/src/gpu/GrTextureRenderTargetProxy \
       UnpackedTarball/skia/src/gpu/GrXferProcessor \
       UnpackedTarball/skia/src/gpu/GrYUVProvider \
       UnpackedTarball/skia/src/gpu/mock/GrMockGpu \
       UnpackedTarball/skia/src/gpu/mock/GrMockTypes \
       UnpackedTarball/skia/src/gpu/ops/GrAAConvexPathRenderer \
       UnpackedTarball/skia/src/gpu/ops/GrAAConvexTessellator \
       UnpackedTarball/skia/src/gpu/ops/GrAAHairLinePathRenderer \
       UnpackedTarball/skia/src/gpu/ops/GrAALinearizingConvexPathRenderer \
       UnpackedTarball/skia/src/gpu/ops/GrAtlasTextOp \
       UnpackedTarball/skia/src/gpu/ops/GrClearOp \
       UnpackedTarball/skia/src/gpu/ops/GrClearStencilClipOp \
       UnpackedTarball/skia/src/gpu/ops/GrCopySurfaceOp \
       UnpackedTarball/skia/src/gpu/ops/GrDashLinePathRenderer \
       UnpackedTarball/skia/src/gpu/ops/GrDashOp \
       UnpackedTarball/skia/src/gpu/ops/GrDebugMarkerOp \
       UnpackedTarball/skia/src/gpu/ops/GrDefaultPathRenderer \
       UnpackedTarball/skia/src/gpu/ops/GrDrawableOp \
       UnpackedTarball/skia/src/gpu/ops/GrDrawAtlasOp \
       UnpackedTarball/skia/src/gpu/ops/GrDrawPathOp \
       UnpackedTarball/skia/src/gpu/ops/GrDrawVerticesOp \
       UnpackedTarball/skia/src/gpu/ops/GrFillRectOp \
       UnpackedTarball/skia/src/gpu/ops/GrFillRRectOp \
       UnpackedTarball/skia/src/gpu/ops/GrLatticeOp \
       UnpackedTarball/skia/src/gpu/ops/GrMeshDrawOp \
       UnpackedTarball/skia/src/gpu/ops/GrOp \
       UnpackedTarball/skia/src/gpu/ops/GrOvalOpFactory \
       UnpackedTarball/skia/src/gpu/ops/GrQuadPerEdgeAA \
       UnpackedTarball/skia/src/gpu/ops/GrRegionOp \
       UnpackedTarball/skia/src/gpu/ops/GrSemaphoreOp \
       UnpackedTarball/skia/src/gpu/ops/GrShadowRRectOp \
       UnpackedTarball/skia/src/gpu/ops/GrSimpleMeshDrawOpHelper \
       UnpackedTarball/skia/src/gpu/ops/GrSmallPathRenderer \
       UnpackedTarball/skia/src/gpu/ops/GrStencilAndCoverPathRenderer \
       UnpackedTarball/skia/src/gpu/ops/GrStencilPathOp \
       UnpackedTarball/skia/src/gpu/ops/GrStrokeRectOp \
       UnpackedTarball/skia/src/gpu/ops/GrTessellatingPathRenderer \
       UnpackedTarball/skia/src/gpu/ops/GrTextureOp \
       UnpackedTarball/skia/src/gpu/ops/GrTransferFromOp \
       UnpackedTarball/skia/src/gpu/SkGpuDevice \
       UnpackedTarball/skia/src/gpu/SkGpuDevice_drawTexture \
       UnpackedTarball/skia/src/gpu/SkGr \
       UnpackedTarball/skia/src/gpu/text/GrAtlasManager \
       UnpackedTarball/skia/src/gpu/text/GrDistanceFieldAdjustTable \
       UnpackedTarball/skia/src/gpu/text/GrSDFMaskFilter \
       UnpackedTarball/skia/src/gpu/text/GrStrikeCache \
       UnpackedTarball/skia/src/gpu/text/GrTextBlobCache \
       UnpackedTarball/skia/src/gpu/text/GrTextBlob \
       UnpackedTarball/skia/src/gpu/text/GrTextBlobVertexRegenerator \
       UnpackedTarball/skia/src/gpu/text/GrTextContext \
       UnpackedTarball/skia/src/image/SkImage_GpuBase \
       UnpackedTarball/skia/src/image/SkImage_Gpu \
       UnpackedTarball/skia/src/image/SkImage_GpuYUVA \
       UnpackedTarball/skia/src/image/SkSurface_Gpu \
       UnpackedTarball/skia/src/gpu/gl/glx/GrGLMakeNativeInterface_glx \
       UnpackedTarball/skia/src/gpu/vk/GrVkAMDMemoryAllocator \
       UnpackedTarball/skia/src/gpu/vk/GrVkBuffer \
       UnpackedTarball/skia/src/gpu/vk/GrVkBufferView \
       UnpackedTarball/skia/src/gpu/vk/GrVkCaps \
       UnpackedTarball/skia/src/gpu/vk/GrVkCommandBuffer \
       UnpackedTarball/skia/src/gpu/vk/GrVkCommandPool \
       UnpackedTarball/skia/src/gpu/vk/GrVkDescriptorPool \
       UnpackedTarball/skia/src/gpu/vk/GrVkDescriptorSet \
       UnpackedTarball/skia/src/gpu/vk/GrVkDescriptorSetManager \
       UnpackedTarball/skia/src/gpu/vk/GrVkExtensions \
       UnpackedTarball/skia/src/gpu/vk/GrVkFramebuffer \
       UnpackedTarball/skia/src/gpu/vk/GrVkGpuCommandBuffer \
       UnpackedTarball/skia/src/gpu/vk/GrVkGpu \
       UnpackedTarball/skia/src/gpu/vk/GrVkImage \
       UnpackedTarball/skia/src/gpu/vk/GrVkImageView \
       UnpackedTarball/skia/src/gpu/vk/GrVkIndexBuffer \
       UnpackedTarball/skia/src/gpu/vk/GrVkInterface \
       UnpackedTarball/skia/src/gpu/vk/GrVkMemory \
       UnpackedTarball/skia/src/gpu/vk/GrVkPipeline \
       UnpackedTarball/skia/src/gpu/vk/GrVkPipelineLayout \
       UnpackedTarball/skia/src/gpu/vk/GrVkPipelineStateBuilder \
       UnpackedTarball/skia/src/gpu/vk/GrVkPipelineStateCache \
       UnpackedTarball/skia/src/gpu/vk/GrVkPipelineState \
       UnpackedTarball/skia/src/gpu/vk/GrVkPipelineStateDataManager \
       UnpackedTarball/skia/src/gpu/vk/GrVkRenderPass \
       UnpackedTarball/skia/src/gpu/vk/GrVkRenderTarget \
       UnpackedTarball/skia/src/gpu/vk/GrVkResourceProvider \
       UnpackedTarball/skia/src/gpu/vk/GrVkSampler \
       UnpackedTarball/skia/src/gpu/vk/GrVkSamplerYcbcrConversion \
       UnpackedTarball/skia/src/gpu/vk/GrVkSecondaryCBDrawContext \
       UnpackedTarball/skia/src/gpu/vk/GrVkSemaphore \
       UnpackedTarball/skia/src/gpu/vk/GrVkStencilAttachment \
       UnpackedTarball/skia/src/gpu/vk/GrVkTexture \
       UnpackedTarball/skia/src/gpu/vk/GrVkTextureRenderTarget \
       UnpackedTarball/skia/src/gpu/vk/GrVkTransferBuffer \
       UnpackedTarball/skia/src/gpu/vk/GrVkTypesPriv \
       UnpackedTarball/skia/src/gpu/vk/GrVkUniformBuffer \
       UnpackedTarball/skia/src/gpu/vk/GrVkUniformHandler \
       UnpackedTarball/skia/src/gpu/vk/GrVkUtil \
       UnpackedTarball/skia/src/gpu/vk/GrVkVaryingHandler \
       UnpackedTarball/skia/src/gpu/vk/GrVkVertexBuffer \
))

#       UnpackedTarball/skia/src/android/SkAndroidFrameworkUtils \
#       UnpackedTarball/skia/src/android/SkAnimatedImage \
#       UnpackedTarball/skia/src/android/SkBitmapRegionCodec \
#       UnpackedTarball/skia/src/android/SkBitmapRegionDecoder \

#       UnpackedTarball/skia/src/codec/SkHeifCodec \
#       UnpackedTarball/skia/src/codec/SkRawCodec \
#       UnpackedTarball/skia/src/codec/SkWebpCodec \
#       UnpackedTarball/skia/src/codec/SkWuffsCodec \
#       UnpackedTarball/skia/src/utils/SkAnimCodecPlayer \

#       UnpackedTarball/skia/src/gpu/gl/android/GrGLMakeNativeInterface_android \
#       UnpackedTarball/skia/src/gpu/gl/egl/GrGLMakeNativeInterface_egl \
#       UnpackedTarball/skia/src/gpu/gl/glfw/GrGLMakeNativeInterface_glfw \
#       UnpackedTarball/skia/src/gpu/gl/iOS/GrGLMakeNativeInterface_iOS \
#       UnpackedTarball/skia/src/gpu/gl/mac/GrGLMakeNativeInterface_mac \
#       UnpackedTarball/skia/src/gpu/gl/win/GrGLMakeNativeInterface_win \

#       UnpackedTarball/skia/src/ports/SkDebug_android \
#       UnpackedTarball/skia/src/ports/SkDebug_win \
#       UnpackedTarball/skia/src/ports/SkDiscardableMemory_none \
#       UnpackedTarball/skia/src/ports/SkFontConfigInterface \
#       UnpackedTarball/skia/src/ports/SkFontConfigInterface_direct \
#       UnpackedTarball/skia/src/ports/SkFontConfigInterface_direct_factory \
#       UnpackedTarball/skia/src/ports/SkFontHost_mac \
#       UnpackedTarball/skia/src/ports/SkFontHost_win \
#       UnpackedTarball/skia/src/ports/SkFontMgr_android \
#       UnpackedTarball/skia/src/ports/SkFontMgr_android_factory \
#       UnpackedTarball/skia/src/ports/SkFontMgr_android_parser \
#       UnpackedTarball/skia/src/ports/SkFontMgr_custom \
#       UnpackedTarball/skia/src/ports/SkFontMgr_custom_directory \
#       UnpackedTarball/skia/src/ports/SkFontMgr_custom_directory_factory \
#       UnpackedTarball/skia/src/ports/SkFontMgr_custom_embedded \
#       UnpackedTarball/skia/src/ports/SkFontMgr_custom_embedded_factory \
#       UnpackedTarball/skia/src/ports/SkFontMgr_custom_empty \
#       UnpackedTarball/skia/src/ports/SkFontMgr_custom_empty_factory \
#       UnpackedTarball/skia/src/ports/SkFontMgr_empty_factory \
#       UnpackedTarball/skia/src/ports/SkFontMgr_FontConfigInterface \
#       UnpackedTarball/skia/src/ports/SkFontMgr_FontConfigInterface_factory \
#       UnpackedTarball/skia/src/ports/SkFontMgr_fuchsia \
#       UnpackedTarball/skia/src/ports/SkFontMgr_win_dw \
#       UnpackedTarball/skia/src/ports/SkFontMgr_win_dw_factory \
#       UnpackedTarball/skia/src/ports/SkImageEncoder_CG \
#       UnpackedTarball/skia/src/ports/SkImageEncoder_WIC \
#       UnpackedTarball/skia/src/ports/SkImageGeneratorCG \
#       UnpackedTarball/skia/src/ports/SkImageGenerator_skia \
#       UnpackedTarball/skia/src/ports/SkImageGeneratorWIC \
#       UnpackedTarball/skia/src/ports/SkMemory_malloc \
#       UnpackedTarball/skia/src/ports/SkMemory_mozalloc \
#       UnpackedTarball/skia/src/ports/SkOSFile_win \
#       UnpackedTarball/skia/src/ports/SkOSLibrary_win \
#       UnpackedTarball/skia/src/ports/SkRemotableFontMgr_win_dw \
#       UnpackedTarball/skia/src/ports/SkScalerContext_win_dw \
#       UnpackedTarball/skia/src/ports/SkTLS_none \
#       UnpackedTarball/skia/src/ports/SkTLS_win \
#       UnpackedTarball/skia/src/ports/SkTypeface_win_dw \

#       UnpackedTarball/skia/src/utils/mac/SkCreateCGImageRef \
#       UnpackedTarball/skia/src/utils/mac/SkStream_mac \
#       UnpackedTarball/skia/src/utils/win/SkAutoCoInitialize \
#       UnpackedTarball/skia/src/utils/win/SkDWrite \
#       UnpackedTarball/skia/src/utils/win/SkDWriteFontFileStream \
#       UnpackedTarball/skia/src/utils/win/SkDWriteGeometrySink \
#       UnpackedTarball/skia/src/utils/win/SkHRESULT \
#       UnpackedTarball/skia/src/utils/win/SkIStream \
#       UnpackedTarball/skia/src/utils/win/SkWGL_win \

#       UnpackedTarball/skia/src/core/SkPicture_none \
#       UnpackedTarball/skia/src/gpu/ccpr/GrCoverageCountingPathRenderer_none \
#       UnpackedTarball/skia/src/gpu/gl/GrGLMakeNativeInterface_none \
#       UnpackedTarball/skia/src/gpu/GrPathRendering_none \
#       UnpackedTarball/skia/src/pdf/SkDocument_PDF_None \

#       UnpackedTarball/skia/src/svg/SkSVGCanvas \
#       UnpackedTarball/skia/src/svg/SkSVGDevice \
#       UnpackedTarball/skia/src/xml/SkDOM \
#       UnpackedTarball/skia/src/xml/SkXMLParser \
#       UnpackedTarball/skia/src/xml/SkXMLWriter \

#       UnpackedTarball/skia/src/utils/SkLuaCanvas \
#       UnpackedTarball/skia/src/utils/SkLua \

#       UnpackedTarball/skia/src/gpu/dawn/GrDawnCaps \
#       UnpackedTarball/skia/src/gpu/dawn/GrDawnGpuCommandBuffer \
#       UnpackedTarball/skia/src/gpu/dawn/GrDawnGpu \
#       UnpackedTarball/skia/src/gpu/dawn/GrDawnRenderTarget \
#       UnpackedTarball/skia/src/gpu/dawn/GrDawnUtil \
#       UnpackedTarball/skia/src/sksl/SkSLMain \
#       UnpackedTarball/skia/src/sksl/lex/Main \
#       UnpackedTarball/skia/src/sksl/lex/NFA \
#       UnpackedTarball/skia/src/sksl/lex/RegexNode \
#       UnpackedTarball/skia/src/sksl/lex/RegexParser \

#       UnpackedTarball/skia/src/atlastext/SkAtlasTextContext \
#       UnpackedTarball/skia/src/atlastext/SkAtlasTextTarget \
#       UnpackedTarball/skia/src/atlastext/SkInternalAtlasTextContext \

$(eval $(call gb_Library_add_generated_exception_objects,skia,\
       UnpackedTarball/skia/third_party/gif/SkGifImageReader \
       UnpackedTarball/skia/third_party/skcms/skcms \
       UnpackedTarball/skia/third_party/vulkanmemoryallocator/GrVulkanMemoryAllocator \
))

# vim: set noet sw=4 ts=4:
