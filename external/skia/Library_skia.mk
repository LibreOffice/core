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
#This currently results in all sorts of compile complaints
#$(eval $(call gb_Library_set_clang_precompiled_header,skia,external/skia/inc/pch/precompiled_skia))

$(eval $(call gb_Library_add_defs,skia,\
    -DSKIA_IMPLEMENTATION=1 \
    -DSKIA_DLL \
    -DSK_USER_CONFIG_HEADER="<$(BUILDDIR)/config_host/config_skia.h>" \
    $(if $(filter INTEL,$(CPUNAME)),$(if $(filter WNT,$(OS)),-DSK_CPU_SSE_LEVEL=SK_CPU_SSE_LEVEL_SSE1,-DSK_CPU_SSE_LEVEL=0)) \
    $(if $(filter X86_64,$(CPUNAME)),-DSK_CPU_SSE_LEVEL=SK_CPU_SSE_LEVEL_SSE2) \
))

# SK_DEBUG controls runtime checks and is controlled by config_skia.h and depends on DBG_UTIL.
# This controls whether to build with compiler optimizations, normally yes, --enable-skia=debug
# allows to build non-optimized. We normally wouldn't debug a 3rd-party library, and Skia
# performance is relatively important (it may be the drawing engine used in software mode).
# Some code may be always built with optimizations, even with Skia debug enabled (see
# $(gb_COMPILEROPTFLAGS) usage).
ifeq ($(ENABLE_SKIA_DEBUG),)
$(eval $(call gb_Library_add_cxxflags,skia, \
    $(gb_COMPILEROPTFLAGS) \
    $(PCH_NO_CODEGEN) \
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

# The clang-cl provided with at least VS 2019 16.11.28 is known-broken with -std:c++20:
ifneq ($(filter -std:c++20,$(CXXFLAGS_CXX11)),)
ifeq ($(LO_CLANG_VERSION),120000)
$(eval $(call gb_Library_add_cxxflags,skia, \
    -std:c++17 \
))
endif
endif

$(eval $(call gb_Library_use_system_win32_libs,skia,\
    fontsub \
    ole32 \
    oleaut32 \
    user32 \
    usp10 \
    gdi32 \
))

# cl.exe (and thus clang-cl) likes to emit copies of inline functions even when not needed,
# which means that for e.g. AVX-compiled sources the .o may contain a copy of an inline
# function built using AVX, and the linker may select that copy as the one to keep, thus
# introducing AVX code into generic code. Avoid generating such inlines. The flag currently
# cannot be used for the whole Skia, because code built without the flag cannot use
# libraries built with the flag, so cl.exe-built VCL would have undefined references.
ifeq ($(HAVE_LO_CLANG_DLLEXPORTINLINES),TRUE)
LO_SKIA_AVOID_INLINE_COPIES := -Zc:dllexportInlines-
endif

else ifeq ($(OS),MACOSX)

$(eval $(call gb_Library_use_system_darwin_frameworks,skia,\
    Cocoa \
    Metal \
    QuartzCore \
))

ifneq ($(SKIA_DISABLE_VMA_USE_STL_SHARED_MUTEX),)
# Disable std::shared_mutex usage on MacOSX < 10.12.
$(eval $(call gb_Library_add_defs,skia,\
    -DVMA_USE_STL_SHARED_MUTEX=0 \
))
endif

else
$(eval $(call gb_Library_use_externals,skia,\
    expat \
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
    -I$(call gb_UnpackedTarball_get_dir,skia)/modules/skcms/ \
    -I$(call gb_UnpackedTarball_get_dir,skia)/third_party/vulkanmemoryallocator/ \
    -I$(call gb_UnpackedTarball_get_dir,skia)/include/third_party/vulkan/ \
    -I$(SRCDIR)/external/skia/inc/ \
))

$(eval $(call gb_Library_add_exception_objects,skia,\
    external/skia/source/SkMemory_malloc \
    external/skia/source/skia_compiler \
    external/skia/source/skia_opts \
))

$(eval $(call gb_Library_set_generated_cxx_suffix,skia,cpp))

$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/src/base/SkArenaAlloc \
    UnpackedTarball/skia/src/base/SkBezierCurves \
    UnpackedTarball/skia/src/base/SkBlockAllocator \
    UnpackedTarball/skia/src/base/SkBuffer \
    UnpackedTarball/skia/src/base/SkContainers \
    UnpackedTarball/skia/src/base/SkCubics \
    UnpackedTarball/skia/src/base/SkDeque \
    UnpackedTarball/skia/src/base/SkFloatingPoint \
    UnpackedTarball/skia/src/base/SkHalf \
    UnpackedTarball/skia/src/base/SkMalloc \
    UnpackedTarball/skia/src/base/SkMathPriv \
    UnpackedTarball/skia/src/base/SkQuads \
    UnpackedTarball/skia/src/base/SkSafeMath \
    UnpackedTarball/skia/src/base/SkSemaphore \
    UnpackedTarball/skia/src/base/SkSharedMutex \
    UnpackedTarball/skia/src/base/SkSpinlock \
    UnpackedTarball/skia/src/base/SkTDArray \
    UnpackedTarball/skia/src/base/SkThreadID \
    UnpackedTarball/skia/src/base/SkTSearch \
    UnpackedTarball/skia/src/base/SkUtils \
    UnpackedTarball/skia/src/base/SkUTF \
    UnpackedTarball/skia/src/codec/SkAndroidCodecAdapter \
    UnpackedTarball/skia/src/codec/SkAndroidCodec \
    UnpackedTarball/skia/src/codec/SkBmpBaseCodec \
    UnpackedTarball/skia/src/codec/SkBmpCodec \
    UnpackedTarball/skia/src/codec/SkBmpMaskCodec \
    UnpackedTarball/skia/src/codec/SkBmpRLECodec \
    UnpackedTarball/skia/src/codec/SkBmpStandardCodec \
    UnpackedTarball/skia/src/codec/SkCodec \
    UnpackedTarball/skia/src/codec/SkCodecImageGenerator \
    UnpackedTarball/skia/src/codec/SkColorPalette \
    UnpackedTarball/skia/src/codec/SkEncodedInfo \
    UnpackedTarball/skia/src/codec/SkIcoCodec \
    UnpackedTarball/skia/src/codec/SkMasks \
    UnpackedTarball/skia/src/codec/SkMaskSwizzler \
    UnpackedTarball/skia/src/codec/SkParseEncodedOrigin \
    UnpackedTarball/skia/src/codec/SkPixmapUtils \
    UnpackedTarball/skia/src/codec/SkPngCodec \
    UnpackedTarball/skia/src/codec/SkSampledCodec \
    UnpackedTarball/skia/src/codec/SkSampler \
    UnpackedTarball/skia/src/codec/SkSwizzler \
    UnpackedTarball/skia/src/codec/SkWbmpCodec \
    UnpackedTarball/skia/src/core/SkAAClip \
    UnpackedTarball/skia/src/core/SkAlphaRuns \
    UnpackedTarball/skia/src/core/SkAnalyticEdge \
    UnpackedTarball/skia/src/core/SkAnnotation \
    UnpackedTarball/skia/src/core/SkATrace \
    UnpackedTarball/skia/src/core/SkAutoPixmapStorage \
    UnpackedTarball/skia/src/core/SkBBHFactory \
    UnpackedTarball/skia/src/core/SkBigPicture \
    UnpackedTarball/skia/src/core/SkBitmapCache \
    UnpackedTarball/skia/src/core/SkBitmap \
    UnpackedTarball/skia/src/core/SkBitmapDevice \
    UnpackedTarball/skia/src/core/SkBitmapProcState \
    UnpackedTarball/skia/src/core/SkBitmapProcState_matrixProcs \
    UnpackedTarball/skia/src/core/SkBlendMode \
    UnpackedTarball/skia/src/core/SkBlendModeBlender \
    UnpackedTarball/skia/src/core/SkBlitRow_D32 \
    UnpackedTarball/skia/src/core/SkBlitter_ARGB32 \
    UnpackedTarball/skia/src/core/SkBlitter_A8 \
    UnpackedTarball/skia/src/core/SkBlitter \
    UnpackedTarball/skia/src/core/SkBlitter_Sprite \
    UnpackedTarball/skia/src/core/SkBlurMask \
    UnpackedTarball/skia/src/core/SkBlurMaskFilterImpl \
    UnpackedTarball/skia/src/core/SkCachedData \
    UnpackedTarball/skia/src/core/SkCanvas \
    UnpackedTarball/skia/src/core/SkCanvas_Raster \
    UnpackedTarball/skia/src/core/SkCanvasPriv \
    UnpackedTarball/skia/src/core/SkCapabilities \
    UnpackedTarball/skia/src/core/SkChecksum \
    UnpackedTarball/skia/src/core/SkChromeRemoteGlyphCache \
    UnpackedTarball/skia/src/core/SkClipStack \
    UnpackedTarball/skia/src/core/SkClipStackDevice \
    UnpackedTarball/skia/src/core/SkColor \
    UnpackedTarball/skia/src/core/SkColorFilter \
    UnpackedTarball/skia/src/core/SkColorSpace \
    UnpackedTarball/skia/src/core/SkColorSpaceXformSteps \
    UnpackedTarball/skia/src/core/SkColorTable \
    UnpackedTarball/skia/src/core/SkCompressedDataUtils \
    UnpackedTarball/skia/src/core/SkContourMeasure \
    UnpackedTarball/skia/src/core/SkConvertPixels \
    UnpackedTarball/skia/src/core/SkCpu \
    UnpackedTarball/skia/src/core/SkCubicClipper \
    UnpackedTarball/skia/src/core/SkCubicMap \
    UnpackedTarball/skia/src/core/SkData \
    UnpackedTarball/skia/src/core/SkDataTable \
    UnpackedTarball/skia/src/core/SkDebug \
    UnpackedTarball/skia/src/core/SkDescriptor \
    UnpackedTarball/skia/src/core/SkDevice \
    UnpackedTarball/skia/src/core/SkDistanceFieldGen \
    UnpackedTarball/skia/src/core/SkDocument \
    UnpackedTarball/skia/src/core/SkDrawable \
    UnpackedTarball/skia/src/core/SkDraw \
    UnpackedTarball/skia/src/core/SkDrawBase \
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
    UnpackedTarball/skia/src/core/SkFontMetricsPriv \
    UnpackedTarball/skia/src/core/SkFontMgr \
    UnpackedTarball/skia/src/core/SkFontStream \
    UnpackedTarball/skia/src/core/SkGaussFilter \
    UnpackedTarball/skia/src/core/SkGeometry \
    UnpackedTarball/skia/src/core/SkIDChangeListener \
    UnpackedTarball/skia/src/core/SkGlobalInitialization_core \
    UnpackedTarball/skia/src/core/SkGlyph \
    UnpackedTarball/skia/src/core/SkGlyphRunPainter \
    UnpackedTarball/skia/src/core/SkGraphics \
    UnpackedTarball/skia/src/core/SkImageFilterCache \
    UnpackedTarball/skia/src/core/SkImageFilterTypes \
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
    UnpackedTarball/skia/src/core/SkMatrix \
    UnpackedTarball/skia/src/core/SkMatrixInvert \
    UnpackedTarball/skia/src/core/SkM44 \
    UnpackedTarball/skia/src/core/SkMD5 \
    UnpackedTarball/skia/src/core/SkMesh \
    UnpackedTarball/skia/src/core/SkMipmap \
    UnpackedTarball/skia/src/core/SkMipmapAccessor \
    UnpackedTarball/skia/src/core/SkMipmapBuilder \
    UnpackedTarball/skia/src/core/SkOpts \
    UnpackedTarball/skia/src/core/SkOpts_erms \
    UnpackedTarball/skia/src/core/SkOverdrawCanvas \
    UnpackedTarball/skia/src/core/SkPaint \
    UnpackedTarball/skia/src/core/SkPaintPriv \
    UnpackedTarball/skia/src/core/SkPath \
    UnpackedTarball/skia/src/core/SkPathBuilder \
    UnpackedTarball/skia/src/core/SkPathEffect \
    UnpackedTarball/skia/src/core/SkPathMeasure \
    UnpackedTarball/skia/src/core/SkPathRef \
    UnpackedTarball/skia/src/core/SkPathUtils \
    UnpackedTarball/skia/src/core/SkPath_serial \
    UnpackedTarball/skia/src/core/SkPicture \
    UnpackedTarball/skia/src/core/SkPictureData \
    UnpackedTarball/skia/src/core/SkPictureFlat \
    UnpackedTarball/skia/src/core/SkPicturePlayback \
    UnpackedTarball/skia/src/core/SkPictureRecord \
    UnpackedTarball/skia/src/core/SkPictureRecorder \
    UnpackedTarball/skia/src/core/SkPixelRef \
    UnpackedTarball/skia/src/core/SkPixmap \
    UnpackedTarball/skia/src/core/SkPixmapDraw \
    UnpackedTarball/skia/src/core/SkPoint \
    UnpackedTarball/skia/src/core/SkPoint3 \
    UnpackedTarball/skia/src/core/SkPtrRecorder \
    UnpackedTarball/skia/src/core/SkQuadClipper \
    UnpackedTarball/skia/src/core/SkRasterClip \
    UnpackedTarball/skia/src/core/SkRasterPipelineBlitter \
    UnpackedTarball/skia/src/core/SkRasterPipeline \
    UnpackedTarball/skia/src/core/SkReadBuffer \
    UnpackedTarball/skia/src/core/SkRecord \
    UnpackedTarball/skia/src/core/SkReadPixelsRec \
    UnpackedTarball/skia/src/core/SkRecordDraw \
    UnpackedTarball/skia/src/core/SkRecordedDrawable \
    UnpackedTarball/skia/src/core/SkRecorder \
    UnpackedTarball/skia/src/core/SkRecordOpts \
    UnpackedTarball/skia/src/core/SkRecords \
    UnpackedTarball/skia/src/core/SkRect \
    UnpackedTarball/skia/src/core/SkRegion \
    UnpackedTarball/skia/src/core/SkRegion_path \
    UnpackedTarball/skia/src/core/SkResourceCache \
    UnpackedTarball/skia/src/core/SkRRect \
    UnpackedTarball/skia/src/core/SkRSXform \
    UnpackedTarball/skia/src/core/SkRTree \
    UnpackedTarball/skia/src/core/SkRuntimeBlender \
    UnpackedTarball/skia/src/core/SkRuntimeEffect \
    UnpackedTarball/skia/src/core/SkScalar \
    UnpackedTarball/skia/src/core/SkScalerContext \
    UnpackedTarball/skia/src/core/SkScan_AAAPath \
    UnpackedTarball/skia/src/core/SkScan_Antihair \
    UnpackedTarball/skia/src/core/SkScan_AntiPath \
    UnpackedTarball/skia/src/core/SkScan \
    UnpackedTarball/skia/src/core/SkScan_Hairline \
    UnpackedTarball/skia/src/core/SkScan_Path \
    UnpackedTarball/skia/src/core/SkScan_SAAPath \
    UnpackedTarball/skia/src/core/SkSLTypeShared \
    UnpackedTarball/skia/src/core/SkSpecialImage \
    UnpackedTarball/skia/src/core/SkSpecialSurface \
    UnpackedTarball/skia/src/core/SkSpriteBlitter_ARGB32 \
    UnpackedTarball/skia/src/core/SkStream \
    UnpackedTarball/skia/src/core/SkStrike \
    UnpackedTarball/skia/src/core/SkStrikeCache \
    UnpackedTarball/skia/src/core/SkStrikeSpec \
    UnpackedTarball/skia/src/core/SkString \
    UnpackedTarball/skia/src/core/SkStringUtils \
    UnpackedTarball/skia/src/core/SkStroke \
    UnpackedTarball/skia/src/core/SkStrokeRec \
    UnpackedTarball/skia/src/core/SkStrokerPriv \
    UnpackedTarball/skia/src/core/SkSwizzle \
    UnpackedTarball/skia/src/core/SkTaskGroup \
    UnpackedTarball/skia/src/core/SkTextBlob \
    UnpackedTarball/skia/src/core/SkTextBlobTrace \
    UnpackedTarball/skia/src/core/SkTime \
    UnpackedTarball/skia/src/core/SkTypefaceCache \
    UnpackedTarball/skia/src/core/SkTypeface \
    UnpackedTarball/skia/src/core/SkTypeface_remote \
    UnpackedTarball/skia/src/core/SkUnPreMultiply \
    UnpackedTarball/skia/src/core/SkVertices \
    UnpackedTarball/skia/src/core/SkVertState \
    UnpackedTarball/skia/src/core/SkVM \
    UnpackedTarball/skia/src/core/SkVMBlitter \
    UnpackedTarball/skia/src/core/SkWriteBuffer \
    UnpackedTarball/skia/src/core/SkWritePixelsRec \
    UnpackedTarball/skia/src/core/SkWriter32 \
    UnpackedTarball/skia/src/core/SkYUVAInfo \
    UnpackedTarball/skia/src/core/SkYUVAPixmaps \
    UnpackedTarball/skia/src/core/SkYUVMath \
    UnpackedTarball/skia/src/core/SkYUVPlanesCache \
    UnpackedTarball/skia/src/encode/SkICC \
    UnpackedTarball/skia/src/encode/SkPngEncoderImpl \
    UnpackedTarball/skia/src/encode/SkEncoder \
    UnpackedTarball/skia/src/effects/colorfilters/SkBlendModeColorFilter \
    UnpackedTarball/skia/src/effects/colorfilters/SkColorFilterBase \
    UnpackedTarball/skia/src/effects/colorfilters/SkColorSpaceXformColorFilter \
    UnpackedTarball/skia/src/effects/colorfilters/SkComposeColorFilter \
    UnpackedTarball/skia/src/effects/colorfilters/SkGaussianColorFilter \
    UnpackedTarball/skia/src/effects/colorfilters/SkMatrixColorFilter \
    UnpackedTarball/skia/src/effects/colorfilters/SkRuntimeColorFilter \
    UnpackedTarball/skia/src/effects/colorfilters/SkTableColorFilter \
    UnpackedTarball/skia/src/effects/colorfilters/SkWorkingFormatColorFilter \
    UnpackedTarball/skia/src/effects/imagefilters/SkBlendImageFilter \
    UnpackedTarball/skia/src/effects/imagefilters/SkBlurImageFilter \
    UnpackedTarball/skia/src/effects/imagefilters/SkColorFilterImageFilter \
    UnpackedTarball/skia/src/effects/imagefilters/SkComposeImageFilter \
    UnpackedTarball/skia/src/effects/imagefilters/SkCropImageFilter \
    UnpackedTarball/skia/src/effects/imagefilters/SkDisplacementMapImageFilter \
    UnpackedTarball/skia/src/effects/imagefilters/SkDropShadowImageFilter \
    UnpackedTarball/skia/src/effects/imagefilters/SkImageImageFilter \
    UnpackedTarball/skia/src/effects/imagefilters/SkLightingImageFilter \
    UnpackedTarball/skia/src/effects/imagefilters/SkMagnifierImageFilter \
    UnpackedTarball/skia/src/effects/imagefilters/SkMatrixConvolutionImageFilter \
    UnpackedTarball/skia/src/effects/imagefilters/SkMatrixTransformImageFilter \
    UnpackedTarball/skia/src/effects/imagefilters/SkMergeImageFilter \
    UnpackedTarball/skia/src/effects/imagefilters/SkMorphologyImageFilter \
    UnpackedTarball/skia/src/effects/imagefilters/SkPictureImageFilter \
    UnpackedTarball/skia/src/effects/imagefilters/SkShaderImageFilter \
    UnpackedTarball/skia/src/effects/imagefilters/SkRuntimeImageFilter \
    UnpackedTarball/skia/src/effects/imagefilters/SkTileImageFilter \
    UnpackedTarball/skia/src/effects/SkBlenders \
    UnpackedTarball/skia/src/effects/SkColorMatrix \
    UnpackedTarball/skia/src/effects/SkColorMatrixFilter \
    UnpackedTarball/skia/src/effects/SkCornerPathEffect \
    UnpackedTarball/skia/src/effects/SkDashPathEffect \
    UnpackedTarball/skia/src/effects/SkDiscretePathEffect \
    UnpackedTarball/skia/src/effects/SkEmbossMask \
    UnpackedTarball/skia/src/effects/SkEmbossMaskFilter \
    UnpackedTarball/skia/src/effects/SkHighContrastFilter \
    UnpackedTarball/skia/src/effects/SkLayerDrawLooper \
    UnpackedTarball/skia/src/effects/SkOpPathEffect \
    UnpackedTarball/skia/src/effects/SkShaderMaskFilterImpl \
    UnpackedTarball/skia/src/effects/SkTableMaskFilter \
    UnpackedTarball/skia/src/effects/SkTrimPathEffect \
    UnpackedTarball/skia/src/effects/Sk1DPathEffect \
    UnpackedTarball/skia/src/effects/Sk2DPathEffect \
    UnpackedTarball/skia/src/fonts/SkRemotableFontMgr \
    UnpackedTarball/skia/src/image/SkImage \
    UnpackedTarball/skia/src/image/SkImage_Base \
    UnpackedTarball/skia/src/image/SkImage_Lazy \
    UnpackedTarball/skia/src/image/SkImage_LazyFactories \
    UnpackedTarball/skia/src/image/SkImage_Picture \
    UnpackedTarball/skia/src/image/SkImage_Raster \
    UnpackedTarball/skia/src/image/SkImage_RasterFactories \
    UnpackedTarball/skia/src/image/SkPictureImageGenerator \
    UnpackedTarball/skia/src/image/SkRescaleAndReadPixels \
    UnpackedTarball/skia/src/image/SkSurface \
    UnpackedTarball/skia/src/image/SkSurface_Base\
    UnpackedTarball/skia/src/image/SkSurface_Null \
    UnpackedTarball/skia/src/image/SkSurface_Raster \
    UnpackedTarball/skia/src/image/SkTiledImageUtils \
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
    UnpackedTarball/skia/src/shaders/gradients/SkConicalGradient \
    UnpackedTarball/skia/src/shaders/gradients/SkGradientBaseShader \
    UnpackedTarball/skia/src/shaders/gradients/SkLinearGradient \
    UnpackedTarball/skia/src/shaders/gradients/SkRadialGradient \
    UnpackedTarball/skia/src/shaders/gradients/SkSweepGradient \
    UnpackedTarball/skia/src/shaders/SkBlendShader \
    UnpackedTarball/skia/src/shaders/SkBitmapProcShader \
    UnpackedTarball/skia/src/shaders/SkColorFilterShader \
    UnpackedTarball/skia/src/shaders/SkColorShader \
    UnpackedTarball/skia/src/shaders/SkCoordClampShader \
    UnpackedTarball/skia/src/shaders/SkEmptyShader \
    UnpackedTarball/skia/src/shaders/SkImageShader \
    UnpackedTarball/skia/src/shaders/SkLocalMatrixShader \
    UnpackedTarball/skia/src/shaders/SkPictureShader \
    UnpackedTarball/skia/src/shaders/SkPerlinNoiseShaderImpl \
    UnpackedTarball/skia/src/shaders/SkRuntimeShader \
    UnpackedTarball/skia/src/shaders/SkShader \
    UnpackedTarball/skia/src/shaders/SkShaderBase \
    UnpackedTarball/skia/src/shaders/SkTransformShader \
    UnpackedTarball/skia/src/shaders/SkTriColorShader \
    UnpackedTarball/skia/src/sksl/dsl/DSLExpression \
    UnpackedTarball/skia/src/sksl/dsl/DSLStatement \
    UnpackedTarball/skia/src/sksl/dsl/DSLType \
    UnpackedTarball/skia/src/sksl/ir/SkSLBinaryExpression \
    UnpackedTarball/skia/src/sksl/ir/SkSLBlock \
    UnpackedTarball/skia/src/sksl/ir/SkSLChildCall \
    UnpackedTarball/skia/src/sksl/ir/SkSLConstructor \
    UnpackedTarball/skia/src/sksl/ir/SkSLConstructorArray \
    UnpackedTarball/skia/src/sksl/ir/SkSLConstructorArrayCast \
    UnpackedTarball/skia/src/sksl/ir/SkSLConstructorCompound \
    UnpackedTarball/skia/src/sksl/ir/SkSLConstructorCompoundCast \
    UnpackedTarball/skia/src/sksl/ir/SkSLConstructorDiagonalMatrix \
    UnpackedTarball/skia/src/sksl/ir/SkSLConstructorMatrixResize \
    UnpackedTarball/skia/src/sksl/ir/SkSLConstructorScalarCast \
    UnpackedTarball/skia/src/sksl/ir/SkSLConstructorSplat \
    UnpackedTarball/skia/src/sksl/ir/SkSLConstructorStruct \
    UnpackedTarball/skia/src/sksl/ir/SkSLDiscardStatement \
    UnpackedTarball/skia/src/sksl/ir/SkSLDoStatement \
    UnpackedTarball/skia/src/sksl/ir/SkSLExpression \
    UnpackedTarball/skia/src/sksl/ir/SkSLExpressionStatement \
    UnpackedTarball/skia/src/sksl/ir/SkSLExtension \
    UnpackedTarball/skia/src/sksl/ir/SkSLFieldAccess \
    UnpackedTarball/skia/src/sksl/ir/SkSLForStatement \
    UnpackedTarball/skia/src/sksl/ir/SkSLFunctionCall \
    UnpackedTarball/skia/src/sksl/ir/SkSLFunctionDeclaration \
    UnpackedTarball/skia/src/sksl/ir/SkSLFunctionDefinition \
    UnpackedTarball/skia/src/sksl/ir/SkSLIfStatement \
    UnpackedTarball/skia/src/sksl/ir/SkSLIndexExpression \
    UnpackedTarball/skia/src/sksl/ir/SkSLInterfaceBlock \
    UnpackedTarball/skia/src/sksl/ir/SkSLLayout \
    UnpackedTarball/skia/src/sksl/ir/SkSLLiteral \
    UnpackedTarball/skia/src/sksl/ir/SkSLModifiers \
    UnpackedTarball/skia/src/sksl/ir/SkSLModifiersDeclaration \
    UnpackedTarball/skia/src/sksl/ir/SkSLProgram \
    UnpackedTarball/skia/src/sksl/ir/SkSLPrefixExpression \
    UnpackedTarball/skia/src/sksl/ir/SkSLPostfixExpression \
    UnpackedTarball/skia/src/sksl/ir/SkSLSetting \
    UnpackedTarball/skia/src/sksl/ir/SkSLStructDefinition \
    UnpackedTarball/skia/src/sksl/ir/SkSLSwitchCase \
    UnpackedTarball/skia/src/sksl/ir/SkSLSwitchStatement \
    UnpackedTarball/skia/src/sksl/ir/SkSLSwizzle \
    UnpackedTarball/skia/src/sksl/ir/SkSLSymbolTable \
    UnpackedTarball/skia/src/sksl/ir/SkSLTernaryExpression \
    UnpackedTarball/skia/src/sksl/ir/SkSLType \
    UnpackedTarball/skia/src/sksl/ir/SkSLTypeReference \
    UnpackedTarball/skia/src/sksl/ir/SkSLVarDeclarations \
    UnpackedTarball/skia/src/sksl/ir/SkSLVariable \
    UnpackedTarball/skia/src/sksl/ir/SkSLVariableReference \
    UnpackedTarball/skia/src/sksl/tracing/SkSLTraceHook \
    UnpackedTarball/skia/src/sksl/tracing/SkSLDebugTracePriv \
    UnpackedTarball/skia/src/sksl/tracing/SkSLDebugTracePlayer \
    UnpackedTarball/skia/src/sksl/SkSLAnalysis \
    UnpackedTarball/skia/src/sksl/SkSLBuiltinTypes \
    UnpackedTarball/skia/src/sksl/SkSLCompiler \
    UnpackedTarball/skia/src/sksl/SkSLConstantFolder \
    UnpackedTarball/skia/src/sksl/SkSLContext \
    UnpackedTarball/skia/src/sksl/SkSLErrorReporter \
    UnpackedTarball/skia/src/sksl/SkSLInliner \
    UnpackedTarball/skia/src/sksl/SkSLIntrinsicList \
    UnpackedTarball/skia/src/sksl/SkSLLexer \
    UnpackedTarball/skia/src/sksl/SkSLMangler \
    UnpackedTarball/skia/src/sksl/SkSLModuleLoader \
    UnpackedTarball/skia/src/sksl/SkSLOperator \
    UnpackedTarball/skia/src/sksl/SkSLOutputStream \
    UnpackedTarball/skia/src/sksl/SkSLParser \
    UnpackedTarball/skia/src/sksl/SkSLPool \
    UnpackedTarball/skia/src/sksl/SkSLPosition \
    UnpackedTarball/skia/src/sksl/SkSLSampleUsage \
    UnpackedTarball/skia/src/sksl/SkSLString \
    UnpackedTarball/skia/src/sksl/SkSLThreadContext \
    UnpackedTarball/skia/src/sksl/SkSLUtil \
    UnpackedTarball/skia/src/sksl/analysis/SkSLCanExitWithoutReturningValue \
    UnpackedTarball/skia/src/sksl/analysis/SkSLCheckProgramStructure \
    UnpackedTarball/skia/src/sksl/analysis/SkSLFinalizationChecks \
    UnpackedTarball/skia/src/sksl/analysis/SkSLGetLoopUnrollInfo \
    UnpackedTarball/skia/src/sksl/analysis/SkSLGetReturnComplexity \
    UnpackedTarball/skia/src/sksl/analysis/SkSLHasSideEffects \
    UnpackedTarball/skia/src/sksl/analysis/SkSLIsConstantExpression \
    UnpackedTarball/skia/src/sksl/analysis/SkSLIsSameExpressionTree \
    UnpackedTarball/skia/src/sksl/analysis/SkSLIsTrivialExpression \
    UnpackedTarball/skia/src/sksl/analysis/SkSLProgramUsage \
    UnpackedTarball/skia/src/sksl/analysis/SkSLReturnsInputAlpha \
    UnpackedTarball/skia/src/sksl/analysis/SkSLSymbolTableStackBuilder \
    UnpackedTarball/skia/src/sksl/analysis/SkSLSwitchCaseContainsExit \
    UnpackedTarball/skia/src/sksl/codegen/SkSLGLSLCodeGenerator \
    UnpackedTarball/skia/src/sksl/codegen/SkSLMetalCodeGenerator \
    UnpackedTarball/skia/src/sksl/codegen/SkSLPipelineStageCodeGenerator \
    UnpackedTarball/skia/src/sksl/codegen/SkSLSPIRVCodeGenerator \
    UnpackedTarball/skia/src/sksl/codegen/SkSLSPIRVtoHLSL \
    UnpackedTarball/skia/src/sksl/codegen/SkSLVMCodeGenerator \
    UnpackedTarball/skia/src/sksl/codegen/SkSLWGSLCodeGenerator \
    UnpackedTarball/skia/src/sksl/transform/SkSLAddConstToVarModifiers \
    UnpackedTarball/skia/src/sksl/transform/SkSLEliminateDeadFunctions \
    UnpackedTarball/skia/src/sksl/transform/SkSLEliminateDeadGlobalVariables \
    UnpackedTarball/skia/src/sksl/transform/SkSLEliminateDeadLocalVariables \
    UnpackedTarball/skia/src/sksl/transform/SkSLEliminateEmptyStatements \
    UnpackedTarball/skia/src/sksl/transform/SkSLEliminateUnreachableCode \
    UnpackedTarball/skia/src/sksl/transform/SkSLFindAndDeclareBuiltinFunctions \
    UnpackedTarball/skia/src/sksl/transform/SkSLFindAndDeclareBuiltinVariables \
    UnpackedTarball/skia/src/sksl/transform/SkSLHoistSwitchVarDeclarationsAtTopLevel \
    UnpackedTarball/skia/src/sksl/transform/SkSLRenamePrivateSymbols \
    UnpackedTarball/skia/src/sksl/transform/SkSLReplaceConstVarsWithLiterals \
    UnpackedTarball/skia/src/sksl/transform/SkSLRewriteIndexedSwizzle \
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
    UnpackedTarball/skia/src/utils/SkJSON \
    UnpackedTarball/skia/src/utils/SkJSONWriter \
    UnpackedTarball/skia/src/utils/SkMatrix22 \
    UnpackedTarball/skia/src/utils/SkMultiPictureDocument \
    UnpackedTarball/skia/src/utils/SkNullCanvas \
    UnpackedTarball/skia/src/utils/SkNWayCanvas \
    UnpackedTarball/skia/src/utils/SkOSPath \
    UnpackedTarball/skia/src/utils/SkOrderedFontMgr \
    UnpackedTarball/skia/src/utils/SkPaintFilterCanvas \
    UnpackedTarball/skia/src/utils/SkParseColor \
    UnpackedTarball/skia/src/utils/SkParse \
    UnpackedTarball/skia/src/utils/SkParsePath \
    UnpackedTarball/skia/src/utils/SkPatchUtils \
    UnpackedTarball/skia/src/utils/SkPolyUtils \
    UnpackedTarball/skia/src/utils/SkShaderUtils \
    UnpackedTarball/skia/src/utils/SkShadowTessellator \
    UnpackedTarball/skia/src/utils/SkShadowUtils \
    UnpackedTarball/skia/src/utils/SkTextUtils \
    UnpackedTarball/skia/src/xps/SkXPSDevice \
    UnpackedTarball/skia/src/xps/SkXPSDocument \
))

ifneq ($(SKIA_GPU),)
$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/src/core/SkGpuBlurUtils \
    UnpackedTarball/skia/src/gpu/AtlasTypes \
    UnpackedTarball/skia/src/gpu/Blend \
    UnpackedTarball/skia/src/gpu/BlendFormula \
    UnpackedTarball/skia/src/gpu/DitherUtils \
    UnpackedTarball/skia/src/gpu/RectanizerPow2 \
    UnpackedTarball/skia/src/gpu/RectanizerSkyline \
    UnpackedTarball/skia/src/gpu/ResourceKey \
    UnpackedTarball/skia/src/gpu/ShaderErrorHandler \
    UnpackedTarball/skia/src/gpu/Swizzle \
    UnpackedTarball/skia/src/gpu/TiledTextureUtils \
    UnpackedTarball/skia/src/gpu/ganesh/ClipStack \
    UnpackedTarball/skia/src/gpu/ganesh/Device \
    UnpackedTarball/skia/src/gpu/ganesh/Device_drawTexture \
    UnpackedTarball/skia/src/gpu/ganesh/GrBufferTransferRenderTask \
    UnpackedTarball/skia/src/gpu/ganesh/GrBufferUpdateRenderTask \
    UnpackedTarball/skia/src/gpu/ganesh/GrFragmentProcessors \
    UnpackedTarball/skia/src/gpu/ganesh/GrSurfaceProxyView \
    UnpackedTarball/skia/src/gpu/ganesh/PathRenderer \
    UnpackedTarball/skia/src/gpu/ganesh/PathRendererChain \
    UnpackedTarball/skia/src/gpu/ganesh/StencilMaskHelper \
    UnpackedTarball/skia/src/gpu/ganesh/SurfaceDrawContext \
    UnpackedTarball/skia/src/gpu/ganesh/effects/GrBezierEffect \
    UnpackedTarball/skia/src/gpu/ganesh/effects/GrBicubicEffect \
    UnpackedTarball/skia/src/gpu/ganesh/effects/GrBitmapTextGeoProc \
    UnpackedTarball/skia/src/gpu/ganesh/effects/GrBlendFragmentProcessor \
    UnpackedTarball/skia/src/gpu/ganesh/effects/GrColorTableEffect \
    UnpackedTarball/skia/src/gpu/ganesh/effects/GrConvexPolyEffect \
    UnpackedTarball/skia/src/gpu/ganesh/effects/GrCoverageSetOpXP \
    UnpackedTarball/skia/src/gpu/ganesh/effects/GrCustomXfermode \
    UnpackedTarball/skia/src/gpu/ganesh/effects/GrDisableColorXP \
    UnpackedTarball/skia/src/gpu/ganesh/effects/GrDistanceFieldGeoProc \
    UnpackedTarball/skia/src/gpu/ganesh/effects/GrGaussianConvolutionFragmentProcessor \
    UnpackedTarball/skia/src/gpu/ganesh/effects/GrMatrixConvolutionEffect \
    UnpackedTarball/skia/src/gpu/ganesh/effects/GrMatrixEffect \
    UnpackedTarball/skia/src/gpu/ganesh/effects/GrModulateAtlasCoverageEffect \
    UnpackedTarball/skia/src/gpu/ganesh/effects/GrOvalEffect \
    UnpackedTarball/skia/src/gpu/ganesh/effects/GrPerlinNoise2Effect \
    UnpackedTarball/skia/src/gpu/ganesh/effects/GrPorterDuffXferProcessor \
    UnpackedTarball/skia/src/gpu/ganesh/effects/GrRRectEffect \
    UnpackedTarball/skia/src/gpu/ganesh/effects/GrShadowGeoProc \
    UnpackedTarball/skia/src/gpu/ganesh/effects/GrSkSLFP \
    UnpackedTarball/skia/src/gpu/ganesh/effects/GrTextureEffect \
    UnpackedTarball/skia/src/gpu/ganesh/effects/GrYUVtoRGBEffect \
    UnpackedTarball/skia/src/gpu/ganesh/geometry/GrPathUtils \
    UnpackedTarball/skia/src/gpu/ganesh/geometry/GrQuad \
    UnpackedTarball/skia/src/gpu/ganesh/geometry/GrQuadUtils \
    UnpackedTarball/skia/src/gpu/ganesh/geometry/GrShape \
    UnpackedTarball/skia/src/gpu/ganesh/geometry/GrStyledShape \
    UnpackedTarball/skia/src/gpu/ganesh/glsl/GrGLSLBlend \
    UnpackedTarball/skia/src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder \
    UnpackedTarball/skia/src/gpu/ganesh/glsl/GrGLSLProgramBuilder \
    UnpackedTarball/skia/src/gpu/ganesh/glsl/GrGLSLProgramDataManager \
    UnpackedTarball/skia/src/gpu/ganesh/glsl/GrGLSLShaderBuilder \
    UnpackedTarball/skia/src/gpu/ganesh/glsl/GrGLSLUniformHandler \
    UnpackedTarball/skia/src/gpu/ganesh/glsl/GrGLSLVarying \
    UnpackedTarball/skia/src/gpu/ganesh/glsl/GrGLSLVertexGeoBuilder \
    UnpackedTarball/skia/src/gpu/ganesh/gradients/GrGradientBitmapCache \
    UnpackedTarball/skia/src/gpu/ganesh/gradients/GrGradientShader \
    UnpackedTarball/skia/src/gpu/ganesh/image/GrImageUtils \
    UnpackedTarball/skia/src/gpu/ganesh/image/GrTextureGenerator \
    UnpackedTarball/skia/src/gpu/ganesh/image/SkImage_Ganesh \
    UnpackedTarball/skia/src/gpu/ganesh/image/SkImage_GaneshBase \
    UnpackedTarball/skia/src/gpu/ganesh/image/SkImage_GaneshFactories \
    UnpackedTarball/skia/src/gpu/ganesh/image/SkImage_GaneshYUVA \
    UnpackedTarball/skia/src/gpu/ganesh/image/SkImage_LazyTexture \
    UnpackedTarball/skia/src/gpu/ganesh/image/SkImage_RasterPinnable \
    UnpackedTarball/skia/src/gpu/ganesh/GrAHardwareBufferImageGenerator \
    UnpackedTarball/skia/src/gpu/ganesh/GrAHardwareBufferUtils \
    UnpackedTarball/skia/src/gpu/ganesh/GrAttachment \
    UnpackedTarball/skia/src/gpu/ganesh/GrBackendSemaphore \
    UnpackedTarball/skia/src/gpu/ganesh/GrBackendSurface \
    UnpackedTarball/skia/src/gpu/ganesh/GrBackendTextureImageGenerator \
    UnpackedTarball/skia/src/gpu/ganesh/GrBackendUtils \
    UnpackedTarball/skia/src/gpu/ganesh/GrBufferAllocPool \
    UnpackedTarball/skia/src/gpu/ganesh/GrCaps \
    UnpackedTarball/skia/src/gpu/ganesh/GrClientMappedBufferManager \
    UnpackedTarball/skia/src/gpu/ganesh/GrColorInfo \
    UnpackedTarball/skia/src/gpu/ganesh/GrColorSpaceXform \
    UnpackedTarball/skia/src/gpu/ganesh/GrContext_Base \
    UnpackedTarball/skia/src/gpu/ganesh/GrContextThreadSafeProxy \
    UnpackedTarball/skia/src/gpu/ganesh/GrCopyRenderTask \
    UnpackedTarball/skia/src/gpu/ganesh/GrDataUtils \
    UnpackedTarball/skia/src/gpu/ganesh/GrDDLContext \
    UnpackedTarball/skia/src/gpu/ganesh/GrDDLTask \
    UnpackedTarball/skia/src/gpu/ganesh/GrDefaultGeoProcFactory \
    UnpackedTarball/skia/src/gpu/ganesh/GrDeferredDisplayList \
    UnpackedTarball/skia/src/gpu/ganesh/GrDirectContext \
    UnpackedTarball/skia/src/gpu/ganesh/GrDirectContextPriv \
    UnpackedTarball/skia/src/gpu/ganesh/GrDistanceFieldGenFromVector \
    UnpackedTarball/skia/src/gpu/ganesh/GrDrawingManager \
    UnpackedTarball/skia/src/gpu/ganesh/GrDrawOpAtlas \
    UnpackedTarball/skia/src/gpu/ganesh/GrDriverBugWorkarounds \
    UnpackedTarball/skia/src/gpu/ganesh/GrDynamicAtlas \
    UnpackedTarball/skia/src/gpu/ganesh/GrEagerVertexAllocator \
    UnpackedTarball/skia/src/gpu/ganesh/GrFinishCallbacks \
    UnpackedTarball/skia/src/gpu/ganesh/GrFixedClip \
    UnpackedTarball/skia/src/gpu/ganesh/GrFragmentProcessor \
    UnpackedTarball/skia/src/gpu/ganesh/GrGeometryProcessor \
    UnpackedTarball/skia/src/gpu/ganesh/GrGpu \
    UnpackedTarball/skia/src/gpu/ganesh/GrGpuBuffer \
    UnpackedTarball/skia/src/gpu/ganesh/GrGpuResource \
    UnpackedTarball/skia/src/gpu/ganesh/GrImageContext \
    UnpackedTarball/skia/src/gpu/ganesh/GrImageInfo \
    UnpackedTarball/skia/src/gpu/ganesh/GrManagedResource \
    UnpackedTarball/skia/src/gpu/ganesh/GrMemoryPool \
    UnpackedTarball/skia/src/gpu/ganesh/GrMeshDrawTarget \
    UnpackedTarball/skia/src/gpu/ganesh/GrOnFlushResourceProvider \
    UnpackedTarball/skia/src/gpu/ganesh/GrOpFlushState \
    UnpackedTarball/skia/src/gpu/ganesh/GrOpsRenderPass \
    UnpackedTarball/skia/src/gpu/ganesh/GrPaint \
    UnpackedTarball/skia/src/gpu/ganesh/GrPersistentCacheUtils \
    UnpackedTarball/skia/src/gpu/ganesh/GrPipeline \
    UnpackedTarball/skia/src/gpu/ganesh/GrProcessorAnalysis \
    UnpackedTarball/skia/src/gpu/ganesh/GrProcessor \
    UnpackedTarball/skia/src/gpu/ganesh/GrProcessorSet \
    UnpackedTarball/skia/src/gpu/ganesh/GrProcessorUnitTest \
    UnpackedTarball/skia/src/gpu/ganesh/GrProgramDesc \
    UnpackedTarball/skia/src/gpu/ganesh/GrProgramInfo \
    UnpackedTarball/skia/src/gpu/ganesh/GrPromiseImageTexture \
    UnpackedTarball/skia/src/gpu/ganesh/GrProxyProvider \
    UnpackedTarball/skia/src/gpu/ganesh/GrRecordingContext \
    UnpackedTarball/skia/src/gpu/ganesh/GrRecordingContextPriv \
    UnpackedTarball/skia/src/gpu/ganesh/GrRenderTask \
    UnpackedTarball/skia/src/gpu/ganesh/GrRenderTaskCluster \
    UnpackedTarball/skia/src/gpu/ganesh/GrRenderTarget \
    UnpackedTarball/skia/src/gpu/ganesh/GrRenderTargetProxy \
    UnpackedTarball/skia/src/gpu/ganesh/GrResourceAllocator \
    UnpackedTarball/skia/src/gpu/ganesh/GrResourceCache \
    UnpackedTarball/skia/src/gpu/ganesh/GrResourceProvider \
    UnpackedTarball/skia/src/gpu/ganesh/GrRingBuffer \
    UnpackedTarball/skia/src/gpu/ganesh/GrShaderCaps \
    UnpackedTarball/skia/src/gpu/ganesh/GrShaderVar \
    UnpackedTarball/skia/src/gpu/ganesh/GrSPIRVUniformHandler \
    UnpackedTarball/skia/src/gpu/ganesh/GrSPIRVVaryingHandler \
    UnpackedTarball/skia/src/gpu/ganesh/GrStagingBufferManager \
    UnpackedTarball/skia/src/gpu/ganesh/GrStencilSettings \
    UnpackedTarball/skia/src/gpu/ganesh/GrStyle \
    UnpackedTarball/skia/src/gpu/ganesh/GrSurface \
    UnpackedTarball/skia/src/gpu/ganesh/GrSurfaceCharacterization \
    UnpackedTarball/skia/src/gpu/ganesh/GrSurfaceInfo \
    UnpackedTarball/skia/src/gpu/ganesh/GrSurfaceProxy \
    UnpackedTarball/skia/src/gpu/ganesh/GrSWMaskHelper \
    UnpackedTarball/skia/src/gpu/ganesh/GrTestUtils \
    UnpackedTarball/skia/src/gpu/ganesh/GrUniformDataManager \
    UnpackedTarball/skia/src/gpu/ganesh/GrTexture \
    UnpackedTarball/skia/src/gpu/ganesh/GrTextureProxy \
    UnpackedTarball/skia/src/gpu/ganesh/GrTextureRenderTargetProxy \
    UnpackedTarball/skia/src/gpu/ganesh/GrTextureResolveRenderTask \
    UnpackedTarball/skia/src/gpu/ganesh/GrThreadSafeCache \
    UnpackedTarball/skia/src/gpu/ganesh/GrThreadSafePipelineBuilder \
    UnpackedTarball/skia/src/gpu/ganesh/GrTransferFromRenderTask \
    UnpackedTarball/skia/src/gpu/ganesh/GrUtil \
    UnpackedTarball/skia/src/gpu/ganesh/GrVertexChunkArray \
    UnpackedTarball/skia/src/gpu/ganesh/GrWaitRenderTask \
    UnpackedTarball/skia/src/gpu/ganesh/GrWritePixelsRenderTask \
    UnpackedTarball/skia/src/gpu/ganesh/GrXferProcessor \
    UnpackedTarball/skia/src/gpu/ganesh/GrYUVABackendTextures \
    UnpackedTarball/skia/src/gpu/ganesh/GrYUVATextureProxies \
    UnpackedTarball/skia/src/gpu/ganesh/geometry/GrAAConvexTessellator \
    UnpackedTarball/skia/src/gpu/ganesh/geometry/GrAATriangulator \
    UnpackedTarball/skia/src/gpu/ganesh/geometry/GrTriangulator \
    UnpackedTarball/skia/src/gpu/ganesh/mock/GrMockCaps \
    UnpackedTarball/skia/src/gpu/ganesh/mock/GrMockGpu \
    UnpackedTarball/skia/src/gpu/ganesh/mock/GrMockTypes \
    UnpackedTarball/skia/src/gpu/ganesh/surface/SkSurface_Ganesh \
    UnpackedTarball/skia/src/gpu/ganesh/SkGr \
    UnpackedTarball/skia/src/gpu/ganesh/SurfaceContext \
    UnpackedTarball/skia/src/gpu/ganesh/SurfaceFillContext \
    UnpackedTarball/skia/src/gpu/ganesh/tessellate/GrPathTessellationShader \
    UnpackedTarball/skia/src/gpu/ganesh/tessellate/GrStrokeTessellationShader \
    UnpackedTarball/skia/src/gpu/ganesh/tessellate/GrTessellationShader \
    UnpackedTarball/skia/src/gpu/ganesh/tessellate/PathTessellator \
    UnpackedTarball/skia/src/gpu/ganesh/tessellate/StrokeTessellator \
    UnpackedTarball/skia/src/gpu/ganesh/text/GrAtlasManager \
    UnpackedTarball/skia/src/gpu/PipelineUtils \
    UnpackedTarball/skia/src/gpu/tessellate/FixedCountBufferUtils \
    UnpackedTarball/skia/src/gpu/tessellate/Tessellation \
    UnpackedTarball/skia/src/text/GlyphRun \
    UnpackedTarball/skia/src/text/StrikeForGPU \
    UnpackedTarball/skia/src/text/gpu/DistanceFieldAdjustTable \
    UnpackedTarball/skia/src/text/gpu/GlyphVector \
    UnpackedTarball/skia/src/text/gpu/Slug \
    UnpackedTarball/skia/src/text/gpu/SlugImpl \
    UnpackedTarball/skia/src/text/gpu/StrikeCache \
    UnpackedTarball/skia/src/text/gpu/SubRunContainer \
    UnpackedTarball/skia/src/text/gpu/SubRunAllocator \
    UnpackedTarball/skia/src/text/gpu/SDFMaskFilter \
    UnpackedTarball/skia/src/text/gpu/SDFTControl \
    UnpackedTarball/skia/src/text/gpu/TextBlob \
    UnpackedTarball/skia/src/text/gpu/TextBlobRedrawCoordinator \
    UnpackedTarball/skia/src/text/gpu/VertexFiller \
))

$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/src/gpu/ganesh/GrAuditTrail \
    UnpackedTarball/skia/src/gpu/ganesh/GrBlurUtils \
    UnpackedTarball/skia/src/gpu/ganesh/GrDrawOpTest \
    UnpackedTarball/skia/src/gpu/ganesh/ops/AAConvexPathRenderer \
    UnpackedTarball/skia/src/gpu/ganesh/ops/AAHairLinePathRenderer \
    UnpackedTarball/skia/src/gpu/ganesh/ops/AALinearizingConvexPathRenderer \
    UnpackedTarball/skia/src/gpu/ganesh/ops/AtlasInstancedHelper \
    UnpackedTarball/skia/src/gpu/ganesh/ops/AtlasPathRenderer \
    UnpackedTarball/skia/src/gpu/ganesh/ops/AtlasRenderTask \
    UnpackedTarball/skia/src/gpu/ganesh/ops/AtlasTextOp \
    UnpackedTarball/skia/src/gpu/ganesh/ops/ClearOp \
    UnpackedTarball/skia/src/gpu/ganesh/ops/DashLinePathRenderer \
    UnpackedTarball/skia/src/gpu/ganesh/ops/DashOp \
    UnpackedTarball/skia/src/gpu/ganesh/ops/DefaultPathRenderer \
    UnpackedTarball/skia/src/gpu/ganesh/ops/DrawAtlasOp \
    UnpackedTarball/skia/src/gpu/ganesh/ops/DrawAtlasPathOp \
    UnpackedTarball/skia/src/gpu/ganesh/ops/DrawMeshOp \
    UnpackedTarball/skia/src/gpu/ganesh/ops/DrawableOp \
    UnpackedTarball/skia/src/gpu/ganesh/ops/FillRRectOp \
    UnpackedTarball/skia/src/gpu/ganesh/ops/FillRectOp \
    UnpackedTarball/skia/src/gpu/ganesh/ops/GrMeshDrawOp \
    UnpackedTarball/skia/src/gpu/ganesh/ops/GrOp \
    UnpackedTarball/skia/src/gpu/ganesh/ops/GrOvalOpFactory \
    UnpackedTarball/skia/src/gpu/ganesh/ops/GrSimpleMeshDrawOpHelper \
    UnpackedTarball/skia/src/gpu/ganesh/ops/GrSimpleMeshDrawOpHelperWithStencil \
    UnpackedTarball/skia/src/gpu/ganesh/ops/LatticeOp \
    UnpackedTarball/skia/src/gpu/ganesh/ops/OpsTask \
    UnpackedTarball/skia/src/gpu/ganesh/ops/PathInnerTriangulateOp \
    UnpackedTarball/skia/src/gpu/ganesh/ops/PathStencilCoverOp \
    UnpackedTarball/skia/src/gpu/ganesh/ops/PathTessellateOp \
    UnpackedTarball/skia/src/gpu/ganesh/ops/QuadPerEdgeAA \
    UnpackedTarball/skia/src/gpu/ganesh/ops/RegionOp \
    UnpackedTarball/skia/src/gpu/ganesh/ops/ShadowRRectOp \
    UnpackedTarball/skia/src/gpu/ganesh/ops/SmallPathAtlasMgr \
    UnpackedTarball/skia/src/gpu/ganesh/ops/SmallPathRenderer \
    UnpackedTarball/skia/src/gpu/ganesh/ops/SmallPathShapeData \
    UnpackedTarball/skia/src/gpu/ganesh/ops/SoftwarePathRenderer \
    UnpackedTarball/skia/src/gpu/ganesh/ops/StrokeRectOp \
    UnpackedTarball/skia/src/gpu/ganesh/ops/StrokeTessellateOp \
    UnpackedTarball/skia/src/gpu/ganesh/ops/TessellationPathRenderer \
    UnpackedTarball/skia/src/gpu/ganesh/ops/TextureOp \
    UnpackedTarball/skia/src/gpu/ganesh/ops/TriangulatingPathRenderer \
))

ifeq ($(SKIA_GPU),VULKAN)
$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkBuffer \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkCaps \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkCommandBuffer \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkCommandPool \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkDescriptorPool \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkDescriptorSet \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkDescriptorSetManager \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkFramebuffer \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkGpu \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkImage \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkImageView \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkMSAALoadManager \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkOpsRenderPass \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkPipeline \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkPipelineStateBuilder \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkPipelineStateCache \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkPipelineState \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkPipelineStateDataManager \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkRenderPass \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkRenderTarget \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkResourceProvider \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkSampler \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkSamplerYcbcrConversion \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkSecondaryCBDrawContext \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkSemaphore \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkTexture \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkTextureRenderTarget \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkTypesPriv \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkUniformHandler \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkUtil \
    UnpackedTarball/skia/src/gpu/ganesh/vk/GrVkVaryingHandler \
    UnpackedTarball/skia/src/gpu/vk/VulkanAMDMemoryAllocator \
    UnpackedTarball/skia/src/gpu/vk/VulkanExtensions \
    UnpackedTarball/skia/src/gpu/vk/VulkanInterface \
    UnpackedTarball/skia/src/gpu/vk/VulkanMemory \
))

$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/tools/gpu/vk/VkTestUtils \
    UnpackedTarball/skia/tools/sk_app/VulkanWindowContext \
    UnpackedTarball/skia/third_party/vulkanmemoryallocator/GrVulkanMemoryAllocator \
))

endif
endif

$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/src/ports/SkGlobalInitialization_default \
    UnpackedTarball/skia/src/ports/SkImageGenerator_none \
    UnpackedTarball/skia/src/ports/SkOSFile_stdio \
))

$(eval $(call gb_Library_add_exception_objects,skia,\
    external/skia/source/skia_opts_ssse3, $(CXXFLAGS_INTRINSICS_SSSE3) $(LO_CLANG_CXXFLAGS_INTRINSICS_SSSE3) \
))

$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/src/opts/SkOpts_avx, $(CXXFLAGS_INTRINSICS_AVX) $(LO_CLANG_CXXFLAGS_INTRINSICS_AVX) \
        $(LO_SKIA_AVOID_INLINE_COPIES) \
))
$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/src/opts/SkOpts_hsw, \
        $(CXXFLAGS_INTRINSICS_AVX2) $(CXXFLAGS_INTRINSICS_F16C) $(CXXFLAGS_INTRINSICS_FMA) \
        $(LO_CLANG_CXXFLAGS_INTRINSICS_AVX2) $(LO_CLANG_CXXFLAGS_INTRINSICS_F16C) $(LO_CLANG_CXXFLAGS_INTRINSICS_FMA) \
        $(LO_SKIA_AVOID_INLINE_COPIES) \
))
$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/src/opts/SkOpts_ssse3, $(CXXFLAGS_INTRINSICS_SSSE3) $(LO_CLANG_CXXFLAGS_INTRINSICS_SSSE3) \
        $(LO_SKIA_AVOID_INLINE_COPIES) \
))

$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/src/opts/SkOpts_skx, $(CXXFLAGS_INTRINSICS_AVX512)  $(LO_CLANG_CXXFLAGS_INTRINSICS_AVX512)\
        $(LO_SKIA_AVOID_INLINE_COPIES) \
))

$(eval $(call gb_Library_add_generated_exception_objects,skia,\
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
))

ifeq ($(SKIA_GPU),VULKAN)
$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/tools/sk_app/win/VulkanWindowContext_win \
))
endif

else ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/src/ports/SkDebug_stdio \
    UnpackedTarball/skia/src/ports/SkImageGeneratorCG \
    UnpackedTarball/skia/src/ports/SkFontMgr_mac_ct \
    UnpackedTarball/skia/src/ports/SkFontMgr_mac_ct_factory \
    UnpackedTarball/skia/src/ports/SkScalerContext_mac_ct \
    UnpackedTarball/skia/src/ports/SkTypeface_mac_ct \
    UnpackedTarball/skia/src/ports/SkOSFile_posix \
    UnpackedTarball/skia/src/ports/SkOSLibrary_posix \
    UnpackedTarball/skia/src/utils/mac/SkCTFont \
    UnpackedTarball/skia/src/utils/mac/SkCreateCGImageRef \
))

ifeq ($(SKIA_GPU),METAL)
$(eval $(call gb_Library_add_generated_objcxxobjects,skia,\
    UnpackedTarball/skia/tools/sk_app/MetalWindowContext \
    UnpackedTarball/skia/tools/sk_app/mac/MetalWindowContext_mac \
    UnpackedTarball/skia/tools/sk_app/mac/WindowContextFactory_mac \
))

# Not used, uses OpenGL - UnpackedTarball/skia/tools/sk_app/mac/RasterWindowContext_mac

$(eval $(call gb_Library_add_generated_objcxxobjects,skia,\
    UnpackedTarball/skia/src/gpu/ganesh/mtl/GrMtlAttachment \
    UnpackedTarball/skia/src/gpu/ganesh/mtl/GrMtlBuffer \
    UnpackedTarball/skia/src/gpu/ganesh/mtl/GrMtlCaps \
    UnpackedTarball/skia/src/gpu/ganesh/mtl/GrMtlCommandBuffer \
    UnpackedTarball/skia/src/gpu/ganesh/mtl/GrMtlDepthStencil \
    UnpackedTarball/skia/src/gpu/ganesh/mtl/GrMtlFramebuffer \
    UnpackedTarball/skia/src/gpu/ganesh/mtl/GrMtlGpu \
    UnpackedTarball/skia/src/gpu/ganesh/mtl/GrMtlOpsRenderPass \
    UnpackedTarball/skia/src/gpu/ganesh/mtl/GrMtlPipelineState \
    UnpackedTarball/skia/src/gpu/ganesh/mtl/GrMtlPipelineStateBuilder \
    UnpackedTarball/skia/src/gpu/ganesh/mtl/GrMtlPipelineStateDataManager \
    UnpackedTarball/skia/src/gpu/ganesh/mtl/GrMtlRenderTarget \
    UnpackedTarball/skia/src/gpu/ganesh/mtl/GrMtlResourceProvider \
    UnpackedTarball/skia/src/gpu/ganesh/mtl/GrMtlSampler \
    UnpackedTarball/skia/src/gpu/ganesh/mtl/GrMtlSemaphore \
    UnpackedTarball/skia/src/gpu/ganesh/mtl/GrMtlTexture \
    UnpackedTarball/skia/src/gpu/ganesh/mtl/GrMtlTextureRenderTarget \
    UnpackedTarball/skia/src/gpu/ganesh/mtl/GrMtlTrampoline \
    UnpackedTarball/skia/src/gpu/ganesh/mtl/GrMtlTypesPriv \
    UnpackedTarball/skia/src/gpu/ganesh/mtl/GrMtlUniformHandler \
    UnpackedTarball/skia/src/gpu/ganesh/mtl/GrMtlUtil \
    UnpackedTarball/skia/src/gpu/ganesh/mtl/GrMtlVaryingHandler \
    UnpackedTarball/skia/src/gpu/ganesh/surface/SkSurface_GaneshMtl \
    UnpackedTarball/skia/src/gpu/mtl/MtlUtils \
    , -fobjc-arc \
))
endif

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
))
ifeq ($(SKIA_GPU),VULKAN)
$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/tools/sk_app/unix/VulkanWindowContext_unix \
))
endif

endif

# Skcms code is used by png writer, which is used by SkiaHelper::dump(). Building
# this without optimizations would mean having each pixel of saved images be
# processed by unoptimized code.
$(eval $(call gb_Library_add_generated_exception_objects,skia,\
    UnpackedTarball/skia/modules/skcms/skcms, $(gb_COMPILEROPTFLAGS) \
))

# vim: set noet sw=4 ts=4:
