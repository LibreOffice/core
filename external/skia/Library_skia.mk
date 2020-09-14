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

$(eval $(call gb_Library_use_clang,skia))
$(eval $(call gb_Library_set_clang_precompiled_header,skia,external/skia/inc/pch/precompiled_skia))

$(eval $(call gb_Library_add_defs,skia,\
    -DSKIA_IMPLEMENTATION=1 \
    -DSKIA_DLL \
    -DSK_USER_CONFIG_HEADER="<$(BUILDDIR)/config_host/config_skia.h>" \
))

# SK_DEBUG controls runtime checks and is controlled by config_skia.h and depends on DBG_UTIL.
# This controls whether to build with compiler optimizations, normally yes, --enable-skia=debug
# allows to build non-optimized. We normally wouldn't debug a 3rd-party library, and Skia
# performance is relatively important (it may be the drawing engine used in software mode).
ifeq ($(ENABLE_SKIA_DEBUG),)
$(eval $(call gb_Library_add_cxxflags,skia, \
    $(gb_COMPILEROPTFLAGS) \
    $(PCH_NO_MODULES_CODEGEN) \
))
endif

ifeq ($(OS),WNT)
# Skia can be built with or without UNICODE set, in LO sources we explicitly use the *W unicode
# variants, so build Skia with UNICODE to make it also use the *W variants.
$(eval $(call gb_Library_add_defs,skia,\
    -DUNICODE -D_UNICODE \
))
ifneq ($(gb_ENABLE_PCH),)
$(eval $(call gb_Library_add_cxxflags,skia, \
    -FIsrc/utils/win/SkDWriteNTDDI_VERSION.h \
))
endif

$(eval $(call gb_Library_use_system_win32_libs,skia,\
    fontsub \
    ole32 \
    oleaut32 \
    user32 \
    usp10 \
    gdi32 \
))
else
$(eval $(call gb_Library_use_externals,skia,\
    freetype \
    fontconfig \
))
endif

# we don't enable jpeg for skia, but it has incorrect #ifdef's in places
$(eval $(call gb_Library_use_externals,skia,\
    zlib \
    libjpeg \
    libpng \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,skia,\
    -lm \
    -ldl \
    -lX11-xcb \
    -lX11 \
))
endif

$(eval $(call gb_Library_use_libraries,skia,\
    sal \
))

$(eval $(call gb_Library_set_include,skia,\
    $$(INCLUDE) \
    -I$(call gb_UnpackedTarball_get_dir,skia) \
    -I$(call gb_UnpackedTarball_get_dir,skia)/include/third_party/skcms/ \
    -I$(call gb_UnpackedTarball_get_dir,skia)/third_party/vulkanmemoryallocator/ \
    -I$(call gb_UnpackedTarball_get_dir,skia)/include/third_party/vulkan/ \
    -I$(SRCDIR)/external/skia/inc/ \
))

$(eval $(call gb_Library_add_exception_objects,skia,\
    external/skia/source/SkMemory_malloc \
    external/skia/source/skia_compiler \
    external/skia/source/skia_opts \
))

$(eval $(call gb_Library_add_exception_objects,skia,\
    external/skia/source/skia_opts_ssse3, $(CXXFLAGS_INTRINSICS_SSSE3) $(CLANG_CXXFLAGS_INTRINSICS_SSSE3) \
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
    UnpackedTarball/skia/src/codec/SkIcoCodec \
    UnpackedTarball/skia/src/codec/SkMasks \
    UnpackedTarball/skia/src/codec/SkMaskSwizzler \
    UnpackedTarball/skia/src/codec/SkParseEncodedOrigin \
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
    UnpackedTarball/skia/src/core/SkCompressedDataUtils \
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
    UnpackedTarball/skia/src/core/SkDraw_atlas \
    UnpackedTarball/skia/src/core/SkDraw_text \
    UnpackedTarball/skia/src/core/SkDraw_vertices \
    UnpackedTarball/skia/src/core/SkEdgeBuilder \
    UnpackedTarball/skia/src/core/SkEdgeClipper \
    UnpackedTarball/skia/src/core/SkEdge \
    UnpackedTarball/skia/src/core/SkExecutor \
    UnpackedTarball/skia/src/core/SkFlattenable \
    UnpackedTarball/skia/src/core/SkFont \
    UnpackedTarball/skia/src/core/SkFont_serial \
    UnpackedTarball/skia/src/core/SkFontDescriptor \
    UnpackedTarball/skia/src/core/SkFontLCDConfig \
    UnpackedTarball/skia/src/core/SkFontMgr \
    UnpackedTarball/skia/src/core/SkFontStream \
    UnpackedTarball/skia/src/core/SkGaussFilter \
    UnpackedTarball/skia/src/core/SkGeometry \
    UnpackedTarball/skia/src/core/SkIDChangeListener \
    UnpackedTarball/skia/src/core/SkGlobalInitialization_core \
    UnpackedTarball/skia/src/core/SkGlyph \
    UnpackedTarball/skia/src/core/SkGlyphBuffer \
    UnpackedTarball/skia/src/core/SkGlyphRun \
    UnpackedTarball/skia/src/core/SkGlyphRunPainter \
    UnpackedTarball/skia/src/core/SkGraphics \
    UnpackedTarball/skia/src/core/SkHalf \
    UnpackedTarball/skia/src/core/SkICC \
    UnpackedTarball/skia/src/core/SkImageFilterCache \
    UnpackedTarball/skia/src/core/SkImageFilterTypes \
    UnpackedTarball/skia/src/core/SkImageFilter \
    UnpackedTarball/skia/src/core/SkImageGenerator \
    UnpackedTarball/skia/src/core/SkImageInfo \
    UnpackedTarball/skia/src/core/SkLatticeIter \
    UnpackedTarball/skia/src/core/SkLineClipper \
    UnpackedTarball/skia/src/core/SkLegacyGpuBlurUtils \
    UnpackedTarball/skia/src/core/SkLocalMatrixImageFilter \
    UnpackedTarball/skia/src/core/SkMalloc \
    UnpackedTarball/skia/src/core/SkMallocPixelRef \
    UnpackedTarball/skia/src/core/SkMarkerStack \
    UnpackedTarball/skia/src/core/SkMaskBlurFilter \
    UnpackedTarball/skia/src/core/SkMaskCache \
    UnpackedTarball/skia/src/core/SkMask \
    UnpackedTarball/skia/src/core/SkMaskFilter \
    UnpackedTarball/skia/src/core/SkMaskGamma \
    UnpackedTarball/skia/src/core/SkMath \
    UnpackedTarball/skia/src/core/SkMatrix \
    UnpackedTarball/skia/src/core/SkMatrixImageFilter \
    UnpackedTarball/skia/src/core/SkMatrix44 \
    UnpackedTarball/skia/src/core/SkM44 \
    UnpackedTarball/skia/src/core/SkMD5 \
    UnpackedTarball/skia/src/core/SkMiniRecorder \
    UnpackedTarball/skia/src/core/SkMipmap \
    UnpackedTarball/skia/src/core/SkModeColorFilter \
    UnpackedTarball/skia/src/core/SkOpts \
    UnpackedTarball/skia/src/core/SkOverdrawCanvas \
    UnpackedTarball/skia/src/core/SkPaint \
    UnpackedTarball/skia/src/core/SkPaintPriv \
    UnpackedTarball/skia/src/core/SkPath \
    UnpackedTarball/skia/src/core/SkPathBuilder \
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
    UnpackedTarball/skia/src/core/SkRuntimeEffect \
    UnpackedTarball/skia/src/core/SkRWBuffer \
    UnpackedTarball/skia/src/core/SkScalar \
    UnpackedTarball/skia/src/core/SkScalerCache \
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
    UnpackedTarball/skia/src/core/SkStrikeForGPU \
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
    UnpackedTarball/skia/src/core/SkTextBlobTrace \
    UnpackedTarball/skia/src/core/SkThreadID \
    UnpackedTarball/skia/src/core/SkTime \
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
    UnpackedTarball/skia/src/core/SkVMBlitter \
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
    UnpackedTarball/skia/src/effects/imagefilters/SkImageFilters \
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
    UnpackedTarball/skia/src/fonts/SkRemotableFontMgr \
    UnpackedTarball/skia/src/image/SkImage \
    UnpackedTarball/skia/src/image/SkImage_Lazy \
    UnpackedTarball/skia/src/image/SkImage_Raster \
    UnpackedTarball/skia/src/image/SkRescaleAndReadPixels \
    UnpackedTarball/skia/src/image/SkSurface \
    UnpackedTarball/skia/src/image/SkSurface_Raster \
    UnpackedTarball/skia/src/images/SkImageEncoder \
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
    UnpackedTarball/skia/src/shaders/SkLocalMatrixShader \
    UnpackedTarball/skia/src/shaders/SkPerlinNoiseShader \
    UnpackedTarball/skia/src/shaders/SkPictureShader \
    UnpackedTarball/skia/src/shaders/SkShader \
    UnpackedTarball/skia/src/sksl/ir/SkSLSetting \
    UnpackedTarball/skia/src/sksl/ir/SkSLSymbolTable \
    UnpackedTarball/skia/src/sksl/ir/SkSLType \
    UnpackedTarball/skia/src/sksl/ir/SkSLVariableReference \
    UnpackedTarball/skia/src/sksl/SkSLASTNode \
    UnpackedTarball/skia/src/sksl/SkSLAnalysis \
    UnpackedTarball/skia/src/sksl/SkSLByteCode \
    UnpackedTarball/skia/src/sksl/SkSLByteCodeGenerator \
    UnpackedTarball/skia/src/sksl/SkSLCFGGenerator \
    UnpackedTarball/skia/src/sksl/SkSLCompiler \
    UnpackedTarball/skia/src/sksl/SkSLCPPCodeGenerator \
    UnpackedTarball/skia/src/sksl/SkSLCPPUniformCTypes \
    UnpackedTarball/skia/src/sksl/SkSLDehydrator \
    UnpackedTarball/skia/src/sksl/SkSLGLSLCodeGenerator \
    UnpackedTarball/skia/src/sksl/SkSLHCodeGenerator \
    UnpackedTarball/skia/src/sksl/SkSLIRGenerator \
    UnpackedTarball/skia/src/sksl/SkSLLexer \
    UnpackedTarball/skia/src/sksl/SkSLMetalCodeGenerator \
    UnpackedTarball/skia/src/sksl/SkSLOutputStream \
    UnpackedTarball/skia/src/sksl/SkSLParser \
    UnpackedTarball/skia/src/sksl/SkSLPipelineStageCodeGenerator \
    UnpackedTarball/skia/src/sksl/SkSLRehydrator \
    UnpackedTarball/skia/src/sksl/SkSLSampleUsage \
    UnpackedTarball/skia/src/sksl/SkSLSPIRVCodeGenerator \
    UnpackedTarball/skia/src/sksl/SkSLSPIRVtoHLSL \
    UnpackedTarball/skia/src/sksl/SkSLSectionAndParameterHelper \
    UnpackedTarball/skia/src/sksl/SkSLString \
    UnpackedTarball/skia/src/sksl/SkSLUtil \
    UnpackedTarball/skia/src/utils/SkBase64 \
    UnpackedTarball/skia/src/utils/SkCamera \
    UnpackedTarball/skia/src/utils/SkCanvasStack \
    UnpackedTarball/skia/src/utils/SkCanvasStateUtils \
    UnpackedTarball/skia/src/utils/SkDashPath \
    UnpackedTarball/skia/src/utils/SkEventTracer \
    UnpackedTarball/skia/src/utils/SkFloatToDecimal \
    UnpackedTarball/skia/src/utils/SkCharToGlyphCache \
    UnpackedTarball/skia/src/utils/SkClipStackUtils \
    UnpackedTarball/skia/src/utils/SkCustomTypeface \
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
    UnpackedTarball/skia/src/xps/SkXPSDevice \
    UnpackedTarball/skia/src/xps/SkXPSDocument \
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
    UnpackedTarball/skia/src/gpu/effects/generated/GrArithmeticProcessor \
    UnpackedTarball/skia/src/gpu/effects/generated/GrBlurredEdgeFragmentProcessor \
    UnpackedTarball/skia/src/gpu/effects/generated/GrCircleBlurFragmentProcessor \
    UnpackedTarball/skia/src/gpu/effects/generated/GrCircleEffect \
    UnpackedTarball/skia/src/gpu/effects/generated/GrClampFragmentProcessor \
    UnpackedTarball/skia/src/gpu/effects/generated/GrColorMatrixFragmentProcessor \
    UnpackedTarball/skia/src/gpu/effects/generated/GrComposeLerpEffect \
    UnpackedTarball/skia/src/gpu/effects/generated/GrConfigConversionEffect \
    UnpackedTarball/skia/src/gpu/effects/generated/GrConstColorProcessor \
    UnpackedTarball/skia/src/gpu/effects/generated/GrDitherEffect \
    UnpackedTarball/skia/src/gpu/effects/generated/GrEllipseEffect \
    UnpackedTarball/skia/src/gpu/effects/generated/GrDeviceSpaceEffect \
    UnpackedTarball/skia/src/gpu/effects/generated/GrHSLToRGBFilterEffect \
    UnpackedTarball/skia/src/gpu/effects/generated/GrHighContrastFilterEffect \
    UnpackedTarball/skia/src/gpu/effects/generated/GrLumaColorFilterEffect \
    UnpackedTarball/skia/src/gpu/effects/generated/GrMagnifierEffect \
    UnpackedTarball/skia/src/gpu/effects/generated/GrMixerEffect \
    UnpackedTarball/skia/src/gpu/effects/generated/GrOverrideInputFragmentProcessor \
    UnpackedTarball/skia/src/gpu/effects/generated/GrRectBlurEffect \
    UnpackedTarball/skia/src/gpu/effects/generated/GrRGBToHSLFilterEffect \
    UnpackedTarball/skia/src/gpu/effects/generated/GrRRectBlurEffect \
    UnpackedTarball/skia/src/gpu/effects/GrBezierEffect \
    UnpackedTarball/skia/src/gpu/effects/GrBicubicEffect \
    UnpackedTarball/skia/src/gpu/effects/GrBitmapTextGeoProc \
    UnpackedTarball/skia/src/gpu/effects/GrBlendFragmentProcessor \
    UnpackedTarball/skia/src/gpu/effects/GrConvexPolyEffect \
    UnpackedTarball/skia/src/gpu/effects/GrCoverageSetOpXP \
    UnpackedTarball/skia/src/gpu/effects/GrCustomXfermode \
    UnpackedTarball/skia/src/gpu/effects/GrDisableColorXP \
    UnpackedTarball/skia/src/gpu/effects/GrDistanceFieldGeoProc \
    UnpackedTarball/skia/src/gpu/effects/GrGaussianConvolutionFragmentProcessor \
    UnpackedTarball/skia/src/gpu/effects/GrMatrixConvolutionEffect \
    UnpackedTarball/skia/src/gpu/effects/GrMatrixEffect \
    UnpackedTarball/skia/src/gpu/effects/GrOvalEffect \
    UnpackedTarball/skia/src/gpu/effects/GrPorterDuffXferProcessor \
    UnpackedTarball/skia/src/gpu/effects/GrRRectEffect \
    UnpackedTarball/skia/src/gpu/effects/GrShadowGeoProc \
    UnpackedTarball/skia/src/gpu/effects/GrSkSLFP \
    UnpackedTarball/skia/src/gpu/effects/GrTextureEffect \
    UnpackedTarball/skia/src/gpu/effects/GrYUVtoRGBEffect \
    UnpackedTarball/skia/src/gpu/geometry/GrPathUtils \
    UnpackedTarball/skia/src/gpu/geometry/GrQuad \
    UnpackedTarball/skia/src/gpu/geometry/GrQuadUtils \
    UnpackedTarball/skia/src/gpu/geometry/GrShape \
    UnpackedTarball/skia/src/gpu/geometry/GrStyledShape \
    UnpackedTarball/skia/src/gpu/glsl/GrGLSLBlend \
    UnpackedTarball/skia/src/gpu/glsl/GrGLSL \
    UnpackedTarball/skia/src/gpu/glsl/GrGLSLFragmentProcessor \
    UnpackedTarball/skia/src/gpu/glsl/GrGLSLFragmentShaderBuilder \
    UnpackedTarball/skia/src/gpu/glsl/GrGLSLGeometryProcessor \
    UnpackedTarball/skia/src/gpu/glsl/GrGLSLPrimitiveProcessor \
    UnpackedTarball/skia/src/gpu/glsl/GrGLSLProgramBuilder \
    UnpackedTarball/skia/src/gpu/glsl/GrGLSLProgramDataManager \
    UnpackedTarball/skia/src/gpu/glsl/GrGLSLShaderBuilder \
    UnpackedTarball/skia/src/gpu/glsl/GrGLSLUniformHandler \
    UnpackedTarball/skia/src/gpu/glsl/GrGLSLVarying \
    UnpackedTarball/skia/src/gpu/glsl/GrGLSLVertexGeoBuilder \
    UnpackedTarball/skia/src/gpu/glsl/GrGLSLXferProcessor \
    UnpackedTarball/skia/src/gpu/gradients/generated/GrClampedGradientEffect \
    UnpackedTarball/skia/src/gpu/gradients/generated/GrDualIntervalGradientColorizer \
    UnpackedTarball/skia/src/gpu/gradients/generated/GrLinearGradientLayout \
    UnpackedTarball/skia/src/gpu/gradients/generated/GrRadialGradientLayout \
    UnpackedTarball/skia/src/gpu/gradients/generated/GrSingleIntervalGradientColorizer \
    UnpackedTarball/skia/src/gpu/gradients/generated/GrSweepGradientLayout \
    UnpackedTarball/skia/src/gpu/gradients/generated/GrTiledGradientEffect \
    UnpackedTarball/skia/src/gpu/gradients/generated/GrTwoPointConicalGradientLayout \
    UnpackedTarball/skia/src/gpu/gradients/generated/GrUnrolledBinaryGradientColorizer \
    UnpackedTarball/skia/src/gpu/gradients/GrGradientBitmapCache \
    UnpackedTarball/skia/src/gpu/gradients/GrGradientShader \
    UnpackedTarball/skia/src/gpu/GrAHardwareBufferImageGenerator \
    UnpackedTarball/skia/src/gpu/GrAHardwareBufferUtils \
    UnpackedTarball/skia/src/gpu/GrAuditTrail \
    UnpackedTarball/skia/src/gpu/GrBackendSemaphore \
    UnpackedTarball/skia/src/gpu/GrBackendSurface \
    UnpackedTarball/skia/src/gpu/GrBackendTextureImageGenerator \
    UnpackedTarball/skia/src/gpu/GrBitmapTextureMaker \
    UnpackedTarball/skia/src/gpu/GrBlockAllocator \
    UnpackedTarball/skia/src/gpu/GrBlurUtils \
    UnpackedTarball/skia/src/gpu/GrBufferAllocPool \
    UnpackedTarball/skia/src/gpu/GrCaps \
    UnpackedTarball/skia/src/gpu/GrClientMappedBufferManager \
    UnpackedTarball/skia/src/gpu/GrClipStackClip \
    UnpackedTarball/skia/src/gpu/GrColorInfo \
    UnpackedTarball/skia/src/gpu/GrColorSpaceXform \
    UnpackedTarball/skia/src/gpu/GrContext_Base \
    UnpackedTarball/skia/src/gpu/GrContext \
    UnpackedTarball/skia/src/gpu/GrContextPriv \
    UnpackedTarball/skia/src/gpu/GrContextThreadSafeProxy \
    UnpackedTarball/skia/src/gpu/GrCopyRenderTask \
    UnpackedTarball/skia/src/gpu/GrDataUtils \
    UnpackedTarball/skia/src/gpu/GrDDLContext \
    UnpackedTarball/skia/src/gpu/GrDefaultGeoProcFactory \
    UnpackedTarball/skia/src/gpu/GrDirectContext \
    UnpackedTarball/skia/src/gpu/GrDistanceFieldGenFromVector \
    UnpackedTarball/skia/src/gpu/GrDrawingManager \
    UnpackedTarball/skia/src/gpu/GrDrawOpAtlas \
    UnpackedTarball/skia/src/gpu/GrDrawOpTest \
    UnpackedTarball/skia/src/gpu/GrDriverBugWorkarounds \
    UnpackedTarball/skia/src/gpu/GrDynamicAtlas \
    UnpackedTarball/skia/src/gpu/GrFinishCallbacks \
    UnpackedTarball/skia/src/gpu/GrFixedClip \
    UnpackedTarball/skia/src/gpu/GrFragmentProcessor \
    UnpackedTarball/skia/src/gpu/GrGpu \
    UnpackedTarball/skia/src/gpu/GrGpuBuffer \
    UnpackedTarball/skia/src/gpu/GrGpuResource \
    UnpackedTarball/skia/src/gpu/GrImageContext \
    UnpackedTarball/skia/src/gpu/GrImageTextureMaker \
    UnpackedTarball/skia/src/gpu/GrManagedResource \
    UnpackedTarball/skia/src/gpu/GrMemoryPool \
    UnpackedTarball/skia/src/gpu/GrOnFlushResourceProvider \
    UnpackedTarball/skia/src/gpu/GrOpFlushState \
    UnpackedTarball/skia/src/gpu/GrOpsRenderPass \
    UnpackedTarball/skia/src/gpu/GrOpsTask \
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
    UnpackedTarball/skia/src/gpu/GrProgramInfo \
    UnpackedTarball/skia/src/gpu/GrProxyProvider \
    UnpackedTarball/skia/src/gpu/GrRecordingContext \
    UnpackedTarball/skia/src/gpu/GrRecordingContextPriv \
    UnpackedTarball/skia/src/gpu/GrRectanizerPow2 \
    UnpackedTarball/skia/src/gpu/GrRectanizerSkyline \
    UnpackedTarball/skia/src/gpu/GrRenderTask \
    UnpackedTarball/skia/src/gpu/GrReducedClip \
    UnpackedTarball/skia/src/gpu/GrRenderTargetContext \
    UnpackedTarball/skia/src/gpu/GrRenderTarget \
    UnpackedTarball/skia/src/gpu/GrRenderTargetProxy \
    UnpackedTarball/skia/src/gpu/GrResourceAllocator \
    UnpackedTarball/skia/src/gpu/GrResourceCache \
    UnpackedTarball/skia/src/gpu/GrResourceProvider \
    UnpackedTarball/skia/src/gpu/GrRingBuffer \
    UnpackedTarball/skia/src/gpu/GrSamplePatternDictionary \
    UnpackedTarball/skia/src/gpu/GrShaderCaps \
    UnpackedTarball/skia/src/gpu/GrShaderUtils \
    UnpackedTarball/skia/src/gpu/GrShaderVar \
    UnpackedTarball/skia/src/gpu/GrSoftwarePathRenderer \
    UnpackedTarball/skia/src/gpu/GrSPIRVUniformHandler \
    UnpackedTarball/skia/src/gpu/GrSPIRVVaryingHandler \
    UnpackedTarball/skia/src/gpu/GrStagingBufferManager \
    UnpackedTarball/skia/src/gpu/GrStencilAttachment \
    UnpackedTarball/skia/src/gpu/GrStencilMaskHelper \
    UnpackedTarball/skia/src/gpu/GrStencilSettings \
    UnpackedTarball/skia/src/gpu/GrStyle \
    UnpackedTarball/skia/src/gpu/GrSurfaceContext \
    UnpackedTarball/skia/src/gpu/GrSurface \
    UnpackedTarball/skia/src/gpu/GrSurfaceProxy \
    UnpackedTarball/skia/src/gpu/GrSwizzle \
    UnpackedTarball/skia/src/gpu/GrSWMaskHelper \
    UnpackedTarball/skia/src/gpu/GrTestUtils \
    UnpackedTarball/skia/src/gpu/GrTriangulator \
    UnpackedTarball/skia/src/gpu/GrUniformDataManager \
    UnpackedTarball/skia/src/gpu/GrTextureAdjuster \
    UnpackedTarball/skia/src/gpu/GrTexture \
    UnpackedTarball/skia/src/gpu/GrTextureMaker \
    UnpackedTarball/skia/src/gpu/GrTextureProducer \
    UnpackedTarball/skia/src/gpu/GrTextureProxy \
    UnpackedTarball/skia/src/gpu/GrTextureRenderTargetProxy \
    UnpackedTarball/skia/src/gpu/GrTextureResolveRenderTask \
    UnpackedTarball/skia/src/gpu/GrTransferFromRenderTask \
    UnpackedTarball/skia/src/gpu/GrUtil \
    UnpackedTarball/skia/src/gpu/GrWaitRenderTask \
    UnpackedTarball/skia/src/gpu/GrXferProcessor \
    UnpackedTarball/skia/src/gpu/mock/GrMockCaps \
    UnpackedTarball/skia/src/gpu/mock/GrMockGpu \
    UnpackedTarball/skia/src/gpu/mock/GrMockTypes \
    UnpackedTarball/skia/src/gpu/ops/GrAAConvexPathRenderer \
    UnpackedTarball/skia/src/gpu/ops/GrAAConvexTessellator \
    UnpackedTarball/skia/src/gpu/ops/GrAAHairLinePathRenderer \
    UnpackedTarball/skia/src/gpu/ops/GrAALinearizingConvexPathRenderer \
    UnpackedTarball/skia/src/gpu/ops/GrAtlasTextOp \
    UnpackedTarball/skia/src/gpu/ops/GrClearOp \
    UnpackedTarball/skia/src/gpu/ops/GrDashLinePathRenderer \
    UnpackedTarball/skia/src/gpu/ops/GrDashOp \
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
    UnpackedTarball/skia/src/gpu/ops/GrShadowRRectOp \
    UnpackedTarball/skia/src/gpu/ops/GrSimpleMeshDrawOpHelper \
    UnpackedTarball/skia/src/gpu/ops/GrSimpleMeshDrawOpHelperWithStencil \
    UnpackedTarball/skia/src/gpu/ops/GrSmallPathAtlasMgr \
    UnpackedTarball/skia/src/gpu/ops/GrSmallPathRenderer \
    UnpackedTarball/skia/src/gpu/ops/GrStencilAndCoverPathRenderer \
    UnpackedTarball/skia/src/gpu/ops/GrStencilPathOp \
    UnpackedTarball/skia/src/gpu/ops/GrSmallPathShapeData \
    UnpackedTarball/skia/src/gpu/ops/GrStrokeRectOp \
    UnpackedTarball/skia/src/gpu/ops/GrTriangulatingPathRenderer \
    UnpackedTarball/skia/src/gpu/ops/GrTextureOp \
    UnpackedTarball/skia/src/gpu/SkGpuDevice \
    UnpackedTarball/skia/src/gpu/SkGpuDevice_drawTexture \
    UnpackedTarball/skia/src/gpu/SkGr \
    UnpackedTarball/skia/src/gpu/tessellate/GrDrawAtlasPathOp \
    UnpackedTarball/skia/src/gpu/tessellate/GrFillPathShader \
    UnpackedTarball/skia/src/gpu/tessellate/GrPathTessellateOp \
    UnpackedTarball/skia/src/gpu/tessellate/GrStencilPathShader \
    UnpackedTarball/skia/src/gpu/tessellate/GrStrokePatchBuilder \
    UnpackedTarball/skia/src/gpu/tessellate/GrStrokeTessellateOp \
    UnpackedTarball/skia/src/gpu/tessellate/GrStrokeTessellateShader \
    UnpackedTarball/skia/src/gpu/tessellate/GrTessellationPathRenderer \
    UnpackedTarball/skia/src/gpu/text/GrAtlasManager \
    UnpackedTarball/skia/src/gpu/text/GrDistanceFieldAdjustTable \
    UnpackedTarball/skia/src/gpu/text/GrSDFMaskFilter \
    UnpackedTarball/skia/src/gpu/text/GrSDFTOptions \
    UnpackedTarball/skia/src/gpu/text/GrStrikeCache \
    UnpackedTarball/skia/src/gpu/text/GrTextBlobCache \
    UnpackedTarball/skia/src/gpu/text/GrTextBlob \
    UnpackedTarball/skia/src/image/SkImage_GpuBase \
    UnpackedTarball/skia/src/image/SkImage_Gpu \
    UnpackedTarball/skia/src/image/SkImage_GpuYUVA \
    UnpackedTarball/skia/src/image/SkSurface_Gpu \
    UnpackedTarball/skia/src/gpu/vk/GrVkAMDMemoryAllocator \
    UnpackedTarball/skia/src/gpu/vk/GrVkBuffer \
    UnpackedTarball/skia/src/gpu/vk/GrVkCaps \
    UnpackedTarball/skia/src/gpu/vk/GrVkCommandBuffer \
    UnpackedTarball/skia/src/gpu/vk/GrVkCommandPool \
    UnpackedTarball/skia/src/gpu/vk/GrVkDescriptorPool \
    UnpackedTarball/skia/src/gpu/vk/GrVkDescriptorSet \
    UnpackedTarball/skia/src/gpu/vk/GrVkDescriptorSetManager \
    UnpackedTarball/skia/src/gpu/vk/GrVkExtensions \
    UnpackedTarball/skia/src/gpu/vk/GrVkFramebuffer \
    UnpackedTarball/skia/src/gpu/vk/GrVkGpu \
    UnpackedTarball/skia/src/gpu/vk/GrVkImage \
    UnpackedTarball/skia/src/gpu/vk/GrVkImageView \
    UnpackedTarball/skia/src/gpu/vk/GrVkInterface \
    UnpackedTarball/skia/src/gpu/vk/GrVkMemory \
    UnpackedTarball/skia/src/gpu/vk/GrVkMeshBuffer \
    UnpackedTarball/skia/src/gpu/vk/GrVkOpsRenderPass \
    UnpackedTarball/skia/src/gpu/vk/GrVkPipeline \
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
))

$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/src/ports/SkGlobalInitialization_default \
    UnpackedTarball/skia/src/ports/SkImageGenerator_none \
    UnpackedTarball/skia/src/ports/SkOSFile_stdio \
))

$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/src/opts/SkOpts_avx, $(CXXFLAGS_INTRINSICS_AVX) $(CLANG_CXXFLAGS_INTRINSICS_AVX) \
))
$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/src/opts/SkOpts_hsw, \
        $(CXXFLAGS_INTRINSICS_AVX2) $(CXXFLAGS_INTRINSICS_F16C) $(CXXFLAGS_INTRINSICS_FMA) \
        $(CLANG_CXXFLAGS_INTRINSICS_AVX2) $(CLANG_CXXFLAGS_INTRINSICS_F16C) $(CLANG_CXXFLAGS_INTRINSICS_FMA) \
))
$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/src/opts/SkOpts_sse41, $(CXXFLAGS_INTRINSICS_SSE41) $(CLANG_CXXFLAGS_INTRINSICS_SSE41) \
))
$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/src/opts/SkOpts_sse42, $(CXXFLAGS_INTRINSICS_SSE42) $(CLANG_CXXFLAGS_INTRINSICS_SSE42) \
))
$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/src/opts/SkOpts_ssse3, $(CXXFLAGS_INTRINSICS_SSSE3) $(CLANG_CXXFLAGS_INTRINSICS_SSSE3) \
))
$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/src/opts/SkOpts_crc32 \
))

$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/src/opts/SkOpts_skx, $(CXXFLAGS_INTRINSICS_AVX512)  $(CLANG_CXXFLAGS_INTRINSICS_AVX512)\
))

$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/tools/gpu/vk/VkTestUtils \
    UnpackedTarball/skia/tools/sk_app/VulkanWindowContext \
    UnpackedTarball/skia/tools/sk_app/WindowContext \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/src/ports/SkDebug_win \
    UnpackedTarball/skia/src/ports/SkFontHost_win \
    UnpackedTarball/skia/src/fonts/SkFontMgr_indirect \
    UnpackedTarball/skia/src/ports/SkFontMgr_win_dw \
    UnpackedTarball/skia/src/ports/SkFontMgr_win_dw_factory \
    UnpackedTarball/skia/src/ports/SkOSFile_win \
    UnpackedTarball/skia/src/ports/SkOSLibrary_win \
    UnpackedTarball/skia/src/ports/SkScalerContext_win_dw \
    UnpackedTarball/skia/src/ports/SkTypeface_win_dw \
    UnpackedTarball/skia/src/utils/win/SkAutoCoInitialize \
    UnpackedTarball/skia/src/utils/win/SkDWrite \
    UnpackedTarball/skia/src/utils/win/SkDWriteFontFileStream \
    UnpackedTarball/skia/src/utils/win/SkDWriteGeometrySink \
    UnpackedTarball/skia/src/utils/win/SkHRESULT \
    UnpackedTarball/skia/src/utils/win/SkIStream \
))

$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/tools/sk_app/win/RasterWindowContext_win \
    UnpackedTarball/skia/tools/sk_app/win/VulkanWindowContext_win \
))
else
$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/src/ports/SkDebug_stdio \
    UnpackedTarball/skia/src/ports/SkFontConfigInterface \
    UnpackedTarball/skia/src/ports/SkFontConfigInterface_direct \
    UnpackedTarball/skia/src/ports/SkFontConfigInterface_direct_factory \
    UnpackedTarball/skia/src/ports/SkFontHost_FreeType_common \
    UnpackedTarball/skia/src/ports/SkFontHost_FreeType \
    UnpackedTarball/skia/src/ports/SkFontMgr_FontConfigInterface \
    UnpackedTarball/skia/src/ports/SkFontMgr_fontconfig \
    UnpackedTarball/skia/src/ports/SkFontMgr_fontconfig_factory \
    UnpackedTarball/skia/src/ports/SkOSFile_posix \
    UnpackedTarball/skia/src/ports/SkOSLibrary_posix \
))

$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/tools/sk_app/unix/RasterWindowContext_unix \
    UnpackedTarball/skia/tools/sk_app/unix/VulkanWindowContext_unix \
))
endif

$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/third_party/skcms/skcms \
    UnpackedTarball/skia/third_party/vulkanmemoryallocator/GrVulkanMemoryAllocator \
))

# vim: set noet sw=4 ts=4:
