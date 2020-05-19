/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/stream.hxx>
#include <vcl/FilterConfigItem.hxx>
#include "commonfuzzer.hxx"

#include <config_features.h>
#include <osl/detail/component-mapping.h>

extern "C" {
void * unoxml_component_getFactory( const char* , void* , void* );
void * i18npool_component_getFactory( const char* , void* , void* );
void * ucb_component_getFactory( const char* , void* , void* );
void * emfio_component_getFactory( const char* , void* , void* );

void * com_sun_star_comp_framework_Desktop_get_implementation( void *, void * );
void * com_sun_star_i18n_LocaleDataImpl_get_implementation( void *, void * );
void * com_sun_star_i18n_BreakIterator_Unicode_get_implementation( void *, void * );
void * com_sun_star_i18n_BreakIterator_get_implementation( void *, void * );
void * SfxDocumentMetaData_get_implementation( void *, void * );
void * com_sun_star_i18n_CharacterClassification_Unicode_get_implementation( void *, void * );
void * com_sun_star_i18n_CharacterClassification_get_implementation( void *, void * );
void * com_sun_star_i18n_NativeNumberSupplier_get_implementation( void *, void * );
void * com_sun_star_i18n_NumberFormatCodeMapper_get_implementation( void *, void * );
void * com_sun_star_comp_graphic_GraphicProvider_get_implementation( void *, void * );
void * IndexedPropertyValuesContainer_get_implementation( void *, void * );
void * com_sun_star_comp_uui_UUIInteractionHandler_get_implementation( void *, void * );
void * com_sun_star_comp_comphelper_OPropertyBag( void *, void * );
void * com_sun_star_i18n_Transliteration_get_implementation( void *, void * );
void * com_sun_star_text_DefaultNumberingProvider_get_implementation( void *, void * );
void * com_sun_star_comp_uri_UriReferenceFactory_get_implementation( void *, void * );
}

const lib_to_factory_mapping *
lo_get_factory_map(void)
{
    static lib_to_factory_mapping map[] = {
        { "libunoxmllo.a", unoxml_component_getFactory },
        { "libi18npoollo.a", i18npool_component_getFactory },
        { "libemfiolo.a", emfio_component_getFactory },
        { "libucb1.a", ucb_component_getFactory },
        { 0, 0 }
    };

    return map;
}

const lib_to_constructor_mapping *
lo_get_constructor_map(void)
{
    static lib_to_constructor_mapping map[] = {
        { "com_sun_star_comp_framework_Desktop_get_implementation", com_sun_star_comp_framework_Desktop_get_implementation },
        { "com_sun_star_i18n_LocaleDataImpl_get_implementation", com_sun_star_i18n_LocaleDataImpl_get_implementation },
        { "com_sun_star_i18n_BreakIterator_Unicode_get_implementation", com_sun_star_i18n_BreakIterator_Unicode_get_implementation },
        { "com_sun_star_i18n_BreakIterator_get_implementation", com_sun_star_i18n_BreakIterator_get_implementation },
        { "SfxDocumentMetaData_get_implementation", SfxDocumentMetaData_get_implementation },
        { "com_sun_star_i18n_CharacterClassification_Unicode_get_implementation", com_sun_star_i18n_CharacterClassification_Unicode_get_implementation },
        { "com_sun_star_i18n_CharacterClassification_get_implementation", com_sun_star_i18n_CharacterClassification_get_implementation },
        { "com_sun_star_i18n_NativeNumberSupplier_get_implementation", com_sun_star_i18n_NativeNumberSupplier_get_implementation },
        { "com_sun_star_i18n_NumberFormatCodeMapper_get_implementation", com_sun_star_i18n_NumberFormatCodeMapper_get_implementation },
        { "com_sun_star_comp_graphic_GraphicProvider_get_implementation", com_sun_star_comp_graphic_GraphicProvider_get_implementation },
        { "IndexedPropertyValuesContainer_get_implementation", IndexedPropertyValuesContainer_get_implementation },
        { "com_sun_star_comp_uui_UUIInteractionHandler_get_implementation", com_sun_star_comp_uui_UUIInteractionHandler_get_implementation },
        { "com_sun_star_comp_comphelper_OPropertyBag", com_sun_star_comp_comphelper_OPropertyBag },
        { "com_sun_star_i18n_Transliteration_get_implementation", com_sun_star_i18n_Transliteration_get_implementation },
        { "com_sun_star_text_DefaultNumberingProvider_get_implementation", com_sun_star_text_DefaultNumberingProvider_get_implementation },
        { "com_sun_star_comp_uri_UriReferenceFactory_get_implementation", com_sun_star_comp_uri_UriReferenceFactory_get_implementation},
        { 0, 0 }
    };

    return map;
}

extern "C" void* lo_get_custom_widget_func(const char*)
{
    return nullptr;
}

extern "C" void* SwCreateDialogFactory()
{
    return nullptr;
}

extern "C" bool TestImportWW8(SvStream &rStream);

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    TypicalFuzzerInitialize(argc, argv);
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    SvMemoryStream aStream(const_cast<uint8_t*>(data), size, StreamMode::READ);
    (void)TestImportWW8(aStream);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
