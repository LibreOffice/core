# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Module_Module,vcl))

ifneq ($(ENABLE_WASM_STRIP_PINGUSER),TRUE)
$(eval $(call gb_Module_add_targets,vcl,\
    Package_tipoftheday \
))
endif

$(eval $(call gb_Module_add_targets,vcl,\
    Library_vcl \
    Package_theme_definitions \
    Package_toolbarmode \
    UIConfig_vcl \
    $(if $(filter WNT,$(OS)), \
        Package_opengl_denylist ) \
    $(if $(filter SKIA,$(BUILD_TYPE)), \
        Package_skia_denylist ) \
    $(if $(filter DESKTOP FUZZERS,$(BUILD_TYPE)), \
        StaticLibrary_vclmain \
        $(if $(or $(DISABLE_GUI),$(DISABLE_DYNLOADING)), \
            $(if $(filter EMSCRIPTEN,$(OS)), \
                $(if $(ENABLE_QT5),Executable_vcldemo) \
            ) \
        , \
            $(if $(filter LINUX MACOSX SOLARIS WNT %BSD,$(OS)), \
                Executable_vcldemo \
                Executable_svdemo \
                Executable_minvcl \
                Executable_minweld \
                Executable_svptest \
                Executable_icontest \
                Executable_visualbackendtest \
                Executable_mtfdemo \
            ) \
    )) \
))

ifeq ($(CROSS_COMPILING)$(DISABLE_DYNLOADING),)

$(eval $(call gb_Module_add_targets,vcl,\
    $(if $(filter-out ANDROID iOS WNT,$(OS)), \
        Executable_fftester \
        Executable_listfonts \
        Executable_listglyphs \
        Executable_svpclient) \
))

endif

$(eval $(call gb_Module_add_l10n_targets,vcl,\
    AllLangMoTarget_vcl \
))

ifeq ($(USING_X11),TRUE)
$(eval $(call gb_Module_add_targets,vcl,\
    $(if $(ENABLE_GEN),Library_vclplug_gen) \
    Package_fontunxppds \
    Package_fontunxpsprint \
))
endif

ifneq ($(ENABLE_GTK3),)
$(eval $(call gb_Module_add_targets,vcl,\
    Library_vclplug_gtk3 \
))

ifneq ($(ENABLE_ATSPI_TESTS),)
$(eval $(call gb_Module_add_check_targets,vcl,\
    CppunitTest_vcl_a11y \
    CppunitTest_vcl_gtk3_a11y \
))
endif
endif

ifneq ($(ENABLE_GTK4),)
$(eval $(call gb_Module_add_targets,vcl,\
    Library_vclplug_gtk4 \
))
endif

ifneq ($(ENABLE_KF5),)
$(eval $(call gb_Module_add_targets,vcl,\
    CustomTarget_kf5_moc \
    Library_vclplug_kf5 \
))
endif

ifneq ($(ENABLE_KF6),)
$(eval $(call gb_Module_add_targets,vcl,\
    CustomTarget_kf6_moc \
    Library_vclplug_kf6 \
))
endif


ifneq ($(ENABLE_QT5),)
$(eval $(call gb_Module_add_targets,vcl,\
    CustomTarget_qt5_moc \
    Library_vclplug_qt5 \
))
endif

ifneq ($(ENABLE_QT6),)
$(eval $(call gb_Module_add_targets,vcl,\
    CustomTarget_qt6_moc \
    Library_vclplug_qt6 \
))
endif

ifneq ($(ENABLE_GTK3_KDE5),)
$(eval $(call gb_Module_add_targets,vcl,\
    CustomTarget_gtk3_kde5_moc \
    Library_vclplug_gtk3_kde5 \
    Executable_lo_kde5filepicker \
))
endif

ifeq ($(OS),MACOSX)
$(eval $(call gb_Module_add_targets,vcl,\
    Package_osxres \
    Library_vclplug_osx \
))
endif

ifeq ($(OS),WNT)
$(eval $(call gb_Module_add_targets,vcl,\
    WinResTarget_vcl \
    Library_vclplug_win \
))
endif

ifneq (,$(filter FUZZERS,$(BUILD_TYPE)))
$(eval $(call gb_Module_add_targets,vcl,\
    CustomTarget_nativecore \
    CustomTarget_nativecalc \
    CustomTarget_nativedraw \
    CustomTarget_nativewriter \
    CustomTarget_nativemath \
    StaticLibrary_fuzzer_core \
    StaticLibrary_fuzzer_calc \
    StaticLibrary_fuzzer_draw \
    StaticLibrary_fuzzer_writer \
    StaticLibrary_fuzzer_math \
    Executable_wmffuzzer \
    Executable_jpgfuzzer \
    Executable_giffuzzer \
    Executable_xbmfuzzer \
    Executable_xpmfuzzer \
    Executable_pngfuzzer \
    Executable_bmpfuzzer \
    Executable_svmfuzzer \
    Executable_pcdfuzzer \
    Executable_dxffuzzer \
    Executable_metfuzzer \
    Executable_ppmfuzzer \
    Executable_psdfuzzer \
    Executable_epsfuzzer \
    Executable_pctfuzzer \
    Executable_pcxfuzzer \
    Executable_rasfuzzer \
    Executable_tgafuzzer \
    Executable_tiffuzzer \
    Executable_hwpfuzzer \
    Executable_602fuzzer \
    Executable_lwpfuzzer \
    Executable_olefuzzer \
    Executable_pptfuzzer \
    Executable_rtffuzzer \
    Executable_rtf2pdffuzzer \
    Executable_cgmfuzzer \
    Executable_ww2fuzzer \
    Executable_ww6fuzzer \
    Executable_ww8fuzzer \
    Executable_qpwfuzzer \
    Executable_slkfuzzer \
    Executable_fodtfuzzer \
    Executable_fodt2pdffuzzer \
    Executable_fods2xlsfuzzer \
    Executable_fodsfuzzer \
    Executable_fodpfuzzer \
    Executable_xlsfuzzer \
    Executable_schtmlfuzzer \
    Executable_scrtffuzzer \
    Executable_wksfuzzer \
    Executable_diffuzzer \
    Executable_docxfuzzer \
    Executable_xlsxfuzzer \
    Executable_pptxfuzzer \
    Executable_mmlfuzzer \
    Executable_mtpfuzzer \
    Executable_htmlfuzzer \
    Executable_sftfuzzer \
    Executable_eotfuzzer \
    Executable_dbffuzzer \
    Executable_webpfuzzer \
    Executable_zipfuzzer \
    Executable_svgfuzzer \
))
endif

$(eval $(call gb_Module_add_check_targets,vcl,\
    CppunitTest_vcl_drawmode \
    CppunitTest_vcl_lifecycle \
    CppunitTest_vcl_bitmap_test \
    CppunitTest_vcl_bitmapprocessor_test \
    CppunitTest_vcl_cjk \
    CppunitTest_vcl_graphic_test \
    CppunitTest_vcl_fontcharmap \
    CppunitTest_vcl_font \
    CppunitTest_vcl_fontfeature \
    CppunitTest_vcl_fontmetric \
    CppunitTest_vcl_text \
    CppunitTest_vcl_textlayout \
    CppunitTest_vcl_font_ttf_structure_test \
    CppunitTest_vcl_filters_test \
    CppunitTest_vcl_mnemonic \
    CppunitTest_vcl_outdev \
    CppunitTest_vcl_gradient \
    CppunitTest_vcl_app_test \
    CppunitTest_vcl_jpeg_read_write_test \
    CppunitTest_vcl_svm_test \
    CppunitTest_vcl_errorhandler \
    CppunitTest_vcl_bitmap_render_test \
    CppunitTest_vcl_apitests \
    CppunitTest_vcl_png_test \
    CppunitTest_vcl_widget_definition_reader_test \
    CppunitTest_vcl_backend_test \
    CppunitTest_vcl_blocklistparser_test \
    CppunitTest_vcl_type_serializer_test \
    CppunitTest_vcl_animation \
    $(call gb_Helper_optional, PDFIUM, \
        CppunitTest_vcl_pdfium_library_test) \
    $(if $(filter SKIA,$(BUILD_TYPE)), \
        CppunitTest_vcl_skia) \
    CppunitTest_vcl_filter_igif \
))

ifeq ($(USING_X11),TRUE)
$(eval $(call gb_Module_add_check_targets,vcl,\
    CppunitTest_vcl_timer \
))
endif

ifeq ($(DISABLE_GUI),TRUE)
$(eval $(call gb_Module_add_check_targets,vcl,\
    CppunitTest_vcl_timer \
))
endif

# Is any configuration missing?
ifeq ($(OS),WNT)
$(eval $(call gb_Module_add_check_targets,vcl,\
    CppunitTest_vcl_timer \
))
endif

ifeq ($(OS),MACOSX)
$(eval $(call gb_Module_add_check_targets,vcl,\
    CppunitTest_vcl_timer \
))
endif

# screenshots
$(eval $(call gb_Module_add_screenshot_targets,vcl,\
    CppunitTest_vcl_dialogs_test \
))

ifneq ($(DISPLAY),)
$(eval $(call gb_Module_add_slowcheck_targets,vcl,\
    CppunitTest_vcl_gen \
))
endif

ifneq (,$(filter PDFIUM,$(BUILD_TYPE)))
$(eval $(call gb_Module_add_slowcheck_targets,vcl,\
    CppunitTest_vcl_pdfexport \
    CppunitTest_vcl_pdfexport2 \
    CppunitTest_vcl_pdf_encryption \
    CppunitTest_vcl_filter_ipdf \
))
endif

ifneq ($(filter MORE_FONTS,$(BUILD_TYPE)),)
ifeq ($(OS),LINUX)
$(eval $(call gb_Module_add_slowcheck_targets,vcl,\
    CppunitTest_vcl_unx_generic \
))
endif
endif

# vim: set noet sw=4 ts=4:
