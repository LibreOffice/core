/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <memory>
#include <utility>
#include <vector>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/propertysethelper.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <tools/urlobj.hxx>
#include <svx/xtable.hxx>
#include <vcl/svapp.hxx>

#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include "UnoDocumentSettings.hxx"
#include <unomodel.hxx>

#include <optsitem.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/sfxsids.hrc>
#include <sdattr.hxx>
#include <sdmod.hxx>
#include <ViewShell.hxx>
#include <FrameView.hxx>
#include <Outliner.hxx>
#include <xmloff/settingsstore.hxx>
#include <editeng/editstat.hxx>
#include <svx/unoapi.hxx>

using namespace ::comphelper;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::i18n;

namespace sd
{
    class DocumentSettings : public WeakImplHelper< XPropertySet, XMultiPropertySet, XServiceInfo >,
                             public comphelper::PropertySetHelper,
                             public DocumentSettingsSerializer
    {
    public:
        explicit DocumentSettings( SdXImpressDocument* pModel );

        // XInterface
        virtual Any SAL_CALL queryInterface( const Type& aType ) override;
        virtual void SAL_CALL acquire(  ) throw () override;
        virtual void SAL_CALL release(  ) throw () override;

        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
        virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

        // XMultiPropertySet
        virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) override;
        virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) override;
        virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
        virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
        virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

        // DocumentSettingsSerializer cf. xmloff
        virtual uno::Sequence<beans::PropertyValue>
                filterStreamsFromStorage(OUString const & referer,
                                         const uno::Reference< embed::XStorage > &xStorage,
                                         const uno::Sequence<beans::PropertyValue>& aConfigProps ) override;
        virtual uno::Sequence<beans::PropertyValue>
                filterStreamsToStorage(const uno::Reference< embed::XStorage > &xStorage,
                                       const uno::Sequence<beans::PropertyValue>& aConfigProps ) override;

    protected:
        virtual void _setPropertyValues( const comphelper::PropertyMapEntry** ppEntries, const css::uno::Any* pValues ) override;
        virtual void _getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, css::uno::Any* pValue ) override;

    private:
        bool LoadList( XPropertyListType t, const OUString &rPath,
                       const OUString &rReferer,
                       const uno::Reference< embed::XStorage > &xStorage );
        void AssignURL( XPropertyListType t, const Any* pValue, bool *pOk, bool *pChanged );
        void ExtractURL( XPropertyListType t, Any* pValue );
        rtl::Reference<SdXImpressDocument> mxModel;
    };

    Reference< XInterface > DocumentSettings_createInstance( SdXImpressDocument* pModel )
        throw ()
    {
        DBG_ASSERT( pModel, "I need a model for the DocumentSettings!" );
        return static_cast<XWeak*>(new DocumentSettings( pModel ));
    }

enum SdDocumentSettingsPropertyHandles
{
    HANDLE_PRINTDRAWING, HANDLE_PRINTNOTES, HANDLE_PRINTHANDOUT, HANDLE_PRINTOUTLINE, HANDLE_MEASUREUNIT, HANDLE_SCALE_NUM,
    HANDLE_SCALE_DOM, HANDLE_TABSTOP, HANDLE_PRINTPAGENAME, HANDLE_PRINTDATE, HANDLE_PRINTTIME,
    HANDLE_PRINTHIDENPAGES, HANDLE_PRINTFITPAGE, HANDLE_PRINTTILEPAGE, HANDLE_PRINTBOOKLET, HANDLE_PRINTBOOKLETFRONT,
    HANDLE_PRINTBOOKLETBACK, HANDLE_PRINTQUALITY, HANDLE_COLORTABLEURL, HANDLE_DASHTABLEURL, HANDLE_LINEENDTABLEURL, HANDLE_HATCHTABLEURL,
    HANDLE_GRADIENTTABLEURL, HANDLE_BITMAPTABLEURL, HANDLE_FORBIDDENCHARS, HANDLE_APPLYUSERDATA, HANDLE_SAVETHUMBNAIL, HANDLE_PAGENUMFMT,
    HANDLE_PRINTERNAME, HANDLE_PRINTERJOB, HANDLE_PRINTERPAPERSIZE, HANDLE_PARAGRAPHSUMMATION, HANDLE_CHARCOMPRESS, HANDLE_ASIANPUNCT,
    HANDLE_UPDATEFROMTEMPLATE, HANDLE_PRINTER_INDEPENDENT_LAYOUT
    // #i33095#
    ,HANDLE_LOAD_READONLY, HANDLE_MODIFY_PASSWD, HANDLE_SAVE_VERSION
    ,HANDLE_SLIDESPERHANDOUT, HANDLE_HANDOUTHORIZONTAL,
    HANDLE_EMBED_FONTS, HANDLE_EMBED_USED_FONTS,
    HANDLE_EMBED_LATIN_SCRIPT_FONTS, HANDLE_EMBED_ASIAN_SCRIPT_FONTS, HANDLE_EMBED_COMPLEX_SCRIPT_FONTS,
};

#define MID_PRINTER 1

    static rtl::Reference<PropertySetInfo> createSettingsInfoImpl( bool bIsDraw )
    {
        static PropertyMapEntry const aImpressSettingsInfoMap[] =
        {
            { OUString("IsPrintDrawing"),        HANDLE_PRINTDRAWING,        cppu::UnoType<bool>::get(),                0,  MID_PRINTER },
            { OUString("IsPrintNotes"),          HANDLE_PRINTNOTES,          cppu::UnoType<bool>::get(),                0,  MID_PRINTER },
            { OUString("IsPrintHandout"),        HANDLE_PRINTHANDOUT,        cppu::UnoType<bool>::get(),                0,  MID_PRINTER },
            { OUString("IsPrintOutline"),        HANDLE_PRINTOUTLINE,        cppu::UnoType<bool>::get(),                0,  MID_PRINTER },
            { OUString("SlidesPerHandout"),      HANDLE_SLIDESPERHANDOUT,    ::cppu::UnoType<sal_Int16>::get(),    0,  MID_PRINTER },
            { OUString("HandoutsHorizontal"),    HANDLE_HANDOUTHORIZONTAL,   cppu::UnoType<bool>::get(),                0,  MID_PRINTER },
            { OUString(), 0, css::uno::Type(), 0, 0 }
        };

        static PropertyMapEntry const aDrawSettingsInfoMap[] =
        {
            { OUString("MeasureUnit"),           HANDLE_MEASUREUNIT,         ::cppu::UnoType<sal_Int16>::get(),    0,  0 },
            { OUString("ScaleNumerator"),        HANDLE_SCALE_NUM,           ::cppu::UnoType<sal_Int32>::get(),    0,  0 },
            { OUString("ScaleDenominator"),      HANDLE_SCALE_DOM,           ::cppu::UnoType<sal_Int32>::get(),    0,  0 },
            { OUString(), 0, css::uno::Type(), 0, 0 }
        };

        static PropertyMapEntry const aCommonSettingsInfoMap[] =
        {
            { OUString("DefaultTabStop"),        HANDLE_TABSTOP,             ::cppu::UnoType<sal_Int32>::get(),    0,  0 },
            { OUString("PrinterName"),           HANDLE_PRINTERNAME,         ::cppu::UnoType<OUString>::get(),     0,  0 },
            { OUString("PrinterSetup"),          HANDLE_PRINTERJOB,          cppu::UnoType<uno::Sequence < sal_Int8 >>::get(),  0, MID_PRINTER },
            { OUString("PrinterPaperFromSetup"), HANDLE_PRINTERPAPERSIZE,    cppu::UnoType<bool>::get(),                0,  MID_PRINTER },

            { OUString("IsPrintPageName"),       HANDLE_PRINTPAGENAME,       cppu::UnoType<bool>::get(),                0,  MID_PRINTER },
            { OUString("IsPrintDate"),           HANDLE_PRINTDATE,           cppu::UnoType<bool>::get(),                0,  MID_PRINTER },
            { OUString("IsPrintTime"),           HANDLE_PRINTTIME,           cppu::UnoType<bool>::get(),                0,  MID_PRINTER },
            { OUString("IsPrintHiddenPages"),    HANDLE_PRINTHIDENPAGES,     cppu::UnoType<bool>::get(),                0,  MID_PRINTER },
            { OUString("IsPrintFitPage"),        HANDLE_PRINTFITPAGE,        cppu::UnoType<bool>::get(),                0,  MID_PRINTER },
            { OUString("IsPrintTilePage"),       HANDLE_PRINTTILEPAGE,       cppu::UnoType<bool>::get(),                0,  MID_PRINTER },
            { OUString("IsPrintBooklet"),        HANDLE_PRINTBOOKLET,        cppu::UnoType<bool>::get(),                0,  MID_PRINTER },
            { OUString("IsPrintBookletFront"),   HANDLE_PRINTBOOKLETFRONT,   cppu::UnoType<bool>::get(),                0,  MID_PRINTER },
            { OUString("IsPrintBookletBack"),    HANDLE_PRINTBOOKLETBACK,    cppu::UnoType<bool>::get(),                0,  MID_PRINTER },
            { OUString("PrintQuality"),          HANDLE_PRINTQUALITY,        ::cppu::UnoType<sal_Int32>::get(),    0,  MID_PRINTER },
            { OUString("ColorTableURL"),         HANDLE_COLORTABLEURL,       ::cppu::UnoType<OUString>::get(),     0,  0 },
            { OUString("DashTableURL"),          HANDLE_DASHTABLEURL,        ::cppu::UnoType<OUString>::get(),     0,  0 },
            { OUString("LineEndTableURL"),       HANDLE_LINEENDTABLEURL,     ::cppu::UnoType<OUString>::get(),     0,  0 },
            { OUString("HatchTableURL"),         HANDLE_HATCHTABLEURL,       ::cppu::UnoType<OUString>::get(),     0,  0 },
            { OUString("GradientTableURL"),      HANDLE_GRADIENTTABLEURL,    ::cppu::UnoType<OUString>::get(),     0,  0 },
            { OUString("BitmapTableURL"),        HANDLE_BITMAPTABLEURL,      ::cppu::UnoType<OUString>::get(),     0,  0 },

            { OUString("ForbiddenCharacters"),   HANDLE_FORBIDDENCHARS,      cppu::UnoType<XForbiddenCharacters>::get(),    0, 0 },
            { OUString("ApplyUserData"),         HANDLE_APPLYUSERDATA,       cppu::UnoType<bool>::get(),                0,  0 },
            { OUString("SaveThumbnail"),         HANDLE_SAVETHUMBNAIL,       cppu::UnoType<bool>::get(),                0,  0 },

            { OUString("PageNumberFormat"),      HANDLE_PAGENUMFMT,          ::cppu::UnoType<sal_Int32>::get(),    0,  0 },
            { OUString("ParagraphSummation"),    HANDLE_PARAGRAPHSUMMATION,  cppu::UnoType<bool>::get(),                0,  0 },
            { OUString("CharacterCompressionType"),HANDLE_CHARCOMPRESS,      ::cppu::UnoType<sal_Int16>::get(),          0,  0 },
            { OUString("IsKernAsianPunctuation"),HANDLE_ASIANPUNCT,          cppu::UnoType<bool>::get(),                0,  0 },
            { OUString("UpdateFromTemplate"),    HANDLE_UPDATEFROMTEMPLATE,  cppu::UnoType<bool>::get(),                0,  0 },
            { OUString("PrinterIndependentLayout"),HANDLE_PRINTER_INDEPENDENT_LAYOUT,::cppu::UnoType<sal_Int16>::get(), 0,  0 },
            // --> #i33095#
            { OUString("LoadReadonly"),          HANDLE_LOAD_READONLY,       cppu::UnoType<bool>::get(),                0,  0 },
            { OUString("ModifyPasswordInfo"),    HANDLE_MODIFY_PASSWD,       cppu::UnoType<uno::Sequence < beans::PropertyValue >>::get(),  0,  0 },
            { OUString("SaveVersionOnClose"),    HANDLE_SAVE_VERSION,        cppu::UnoType<bool>::get(),                0,  0 },
            { OUString("EmbedFonts"),              HANDLE_EMBED_FONTS,                cppu::UnoType<bool>::get(), 0,  0 },
            { OUString("EmbedOnlyUsedFonts"),      HANDLE_EMBED_USED_FONTS,           cppu::UnoType<bool>::get(), 0,  0 },
            { OUString("EmbedLatinScriptFonts"),   HANDLE_EMBED_LATIN_SCRIPT_FONTS,   cppu::UnoType<bool>::get(), 0,  0 },
            { OUString("EmbedAsianScriptFonts"),   HANDLE_EMBED_ASIAN_SCRIPT_FONTS,   cppu::UnoType<bool>::get(), 0,  0 },
            { OUString("EmbedComplexScriptFonts"), HANDLE_EMBED_COMPLEX_SCRIPT_FONTS, cppu::UnoType<bool>::get(), 0,  0 },
            { OUString(), 0, css::uno::Type(), 0, 0 }
        };

        rtl::Reference<PropertySetInfo> xInfo = new PropertySetInfo( aCommonSettingsInfoMap );
        xInfo->add( bIsDraw ? aDrawSettingsInfoMap : aImpressSettingsInfoMap );

        return xInfo;
    }
}

using namespace ::sd;

DocumentSettings::DocumentSettings( SdXImpressDocument* pModel )
:   PropertySetHelper( createSettingsInfoImpl( !pModel->IsImpressDocument() ) ),
    mxModel( pModel )
{
}

bool DocumentSettings::LoadList( XPropertyListType t, const OUString &rInPath,
                                 const OUString &rReferer,
                                 const uno::Reference< embed::XStorage > &xStorage )
{
    SdDrawDocument* pDoc = mxModel->GetDoc();

    sal_Int32 nSlash = rInPath.lastIndexOf('/');
    OUString aPath, aName;
    if (nSlash < 0)
        aName = rInPath;
    else {
        aName = rInPath.copy( nSlash + 1 );
        aPath = rInPath.copy( 0, nSlash );
    }

    XPropertyListRef pList = XPropertyList::CreatePropertyList(
        t, aPath, rReferer );
    pList->SetName( aName );

    if( pList->LoadFrom( xStorage, rInPath, rReferer ) )
    {
        pDoc->SetPropertyList( pList );
        return true;
    }

    return false;
}

void DocumentSettings::AssignURL( XPropertyListType t, const Any* pValue,
                                  bool *pOk, bool *pChanged )
{
    OUString aURL;
    if( !( *pValue >>= aURL ) )
        return;

    if( LoadList( t, aURL, ""/*TODO?*/, uno::Reference< embed::XStorage >() ) )
        *pOk = *pChanged = true;
}

static struct {
    const char *pName;
    XPropertyListType t;
} const aURLPropertyNames[] = {
    { "ColorTableURL", XPropertyListType::Color },
    { "DashTableURL", XPropertyListType::Dash },
    { "LineEndTableURL", XPropertyListType::LineEnd },
    { "HatchTableURL", XPropertyListType::Hatch },
    { "GradientTableURL", XPropertyListType::Gradient },
    { "BitmapTableURL", XPropertyListType::Bitmap }
};

static XPropertyListType getTypeOfName( const OUString &aName )
{
    for(const auto & rURLPropertyName : aURLPropertyNames) {
        if( aName.equalsAscii( rURLPropertyName.pName ) )
            return rURLPropertyName.t;
    }
    return XPropertyListType::Unknown;
}

static OUString getNameOfType( XPropertyListType t )
{
    for(const auto & rURLPropertyName : aURLPropertyNames) {
        if( t == rURLPropertyName.t )
            return OUString( rURLPropertyName.pName,
                                  strlen( rURLPropertyName.pName ) - 3,
                                  RTL_TEXTENCODING_ASCII_US );
    }
    return OUString();
}

uno::Sequence<beans::PropertyValue>
        DocumentSettings::filterStreamsFromStorage(
                OUString const & referer,
                const uno::Reference< embed::XStorage > &xStorage,
                const uno::Sequence<beans::PropertyValue>& aConfigProps )
{
    uno::Sequence<beans::PropertyValue> aRet( aConfigProps.getLength() );
    int nRet = 0;
    for( sal_Int32 i = 0; i < aConfigProps.getLength(); i++ )
    {
        XPropertyListType t = getTypeOfName( aConfigProps[i].Name );
        if (t == XPropertyListType::Unknown)
            aRet[nRet++] = aConfigProps[i];
        else
        {
            OUString aURL;
            aConfigProps[i].Value >>= aURL;
            LoadList( t, aURL, referer, xStorage );
        }
    }
    aRet.realloc( nRet );
    return aRet;
}

uno::Sequence<beans::PropertyValue>
        DocumentSettings::filterStreamsToStorage(
                const uno::Reference< embed::XStorage > &xStorage,
                const uno::Sequence<beans::PropertyValue>& aConfigProps )
{
    uno::Sequence<beans::PropertyValue> aRet( aConfigProps.getLength() );

    bool bHasEmbed = false;
    SdDrawDocument* pDoc = mxModel->GetDoc();
    for( size_t i = 0; i < SAL_N_ELEMENTS( aURLPropertyNames ); i++ )
    {
        const XPropertyListRef& pList = pDoc->GetPropertyList( static_cast<XPropertyListType>(i) );
        bHasEmbed = pList.is() && pList->IsEmbedInDocument();
        if( bHasEmbed )
            break;
    }
    if( !bHasEmbed )
        return aConfigProps;

    try {
        // create Settings/ sub storage.
        uno::Reference< embed::XStorage > xSubStorage;
        xSubStorage = xStorage->openStorageElement( "Settings" ,
            embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE );
        if( !xSubStorage.is() )
            return aRet;

        // now populate it
        for( sal_Int32 i = 0; i < aConfigProps.getLength(); i++ )
        {
            XPropertyListType t = getTypeOfName( aConfigProps[i].Name );
            aRet[i] = aConfigProps[i];
            if (t != XPropertyListType::Unknown) {
                const XPropertyListRef& pList = pDoc->GetPropertyList( t );
                if( !pList.is() || !pList->IsEmbedInDocument() )
                    continue; // no change ...
                else
                {
                    // Such specific path construction is grim.

                    OUStringBuffer aName( getNameOfType( t ) );
                    OUString aResult;
                    if( pList->SaveTo( xSubStorage, aName.makeStringAndClear(), &aResult ) )
                    {
                        OUString aRealPath( "Settings/" );
                        aRealPath += aResult;
                        aRet[i].Value <<= aRealPath;
                    }
                }
            }
        }

        // surprisingly difficult to make it really exist
        uno::Reference< embed::XTransactedObject > xTrans( xSubStorage, UNO_QUERY );
        if( xTrans.is() )
            xTrans->commit();
        uno::Reference< lang::XComponent > xComp( xSubStorage, UNO_QUERY );
        if( xComp.is() )
            xSubStorage->dispose();
    } catch (const uno::Exception &) {
//        fprintf (stderr, "saving etc. exception '%s'\n",
//                 OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
    }

    return aRet;
}

// Most of the code reading/writing UNO document settings is the same in
// sd, sc and sw and it is mostly copy-pasted back and forth.
// TODO: Move _setPropertyValues and _getPropertyValues to some shared
// place, at least for the settings that are common to sd, sc and sw
void
DocumentSettings::_setPropertyValues(const PropertyMapEntry** ppEntries,
        const Any* pValues)
{
    ::SolarMutexGuard aGuard;

    SdDrawDocument* pDoc = mxModel->GetDoc();
    ::sd::DrawDocShell* pDocSh = mxModel->GetDocShell();
    if( nullptr == pDoc || nullptr == pDocSh )
    {
        throw RuntimeException("Document or Shell missing",
                static_cast<OWeakObject *>(this));
    }

    bool bValue = false;
    bool bOk, bChanged = false, bOptionsChanged = false;

    SdOptionsPrintItem aOptionsPrintItem;

    VclPtr<SfxPrinter> pPrinter = pDocSh->GetPrinter( false );
    if( pPrinter )
    {
        SdOptionsPrintItem const * pPrinterOptions = nullptr;
        if(pPrinter->GetOptions().GetItemState( ATTR_OPTIONS_PRINT, false, reinterpret_cast<const SfxPoolItem**>(&pPrinterOptions)) == SfxItemState::SET)
            aOptionsPrintItem.GetOptionsPrint() = pPrinterOptions->GetOptionsPrint();
    }
    else
    {
        aOptionsPrintItem.SetOptions( SD_MOD()->GetSdOptions(pDoc->GetDocumentType()) );
    }
    SdOptionsPrint& aPrintOpts = aOptionsPrintItem.GetOptionsPrint();

    for( ; *ppEntries; ppEntries++, pValues++ )
    {
        bOk = false;

        switch( (*ppEntries)->mnHandle )
        {
            case HANDLE_COLORTABLEURL:
                AssignURL( XPropertyListType::Color, pValues, &bOk, &bChanged );
                break;

            case HANDLE_DASHTABLEURL:
                AssignURL( XPropertyListType::Dash, pValues, &bOk, &bChanged );
                break;

            case HANDLE_LINEENDTABLEURL:
                AssignURL( XPropertyListType::LineEnd, pValues, &bOk, &bChanged );
                break;

            case HANDLE_HATCHTABLEURL:
                AssignURL( XPropertyListType::Hatch, pValues, &bOk, &bChanged );
                break;

            case HANDLE_GRADIENTTABLEURL:
                AssignURL( XPropertyListType::Gradient, pValues, &bOk, &bChanged );
                break;

            case HANDLE_BITMAPTABLEURL:
                AssignURL( XPropertyListType::Bitmap, pValues, &bOk, &bChanged );
                break;

            case HANDLE_FORBIDDENCHARS:
                bOk = true;
                break;

            case HANDLE_APPLYUSERDATA:
                {
                    bool bApplyUserData = false;
                    if( *pValues >>= bApplyUserData )
                    {
                        bChanged = ( bApplyUserData != pDocSh->IsUseUserData() );
                        pDocSh->SetUseUserData( bApplyUserData );
                        bOk = true;
                    }
                }
                break;
            case HANDLE_SAVETHUMBNAIL:
                {
                    bool bSaveThumbnail = false;
                    if (*pValues >>= bSaveThumbnail)
                    {
                         bChanged = (bSaveThumbnail != pDocSh->IsUseThumbnailSave());
                         pDocSh->SetUseThumbnailSave(bSaveThumbnail);
                         bOk = true;
                    }
                }
                break;

            case HANDLE_PRINTDRAWING:
                if( *pValues >>= bValue )
                {
                    if( aPrintOpts.IsDraw() != bValue )
                    {
                        aPrintOpts.SetDraw( bValue );
                        bOptionsChanged = true;
                    }

                    bOk = true;
                }
                break;
            case HANDLE_PRINTNOTES:
                if( *pValues >>= bValue )
                {
                    if( aPrintOpts.IsNotes() != bValue )
                    {
                        aPrintOpts.SetNotes( bValue );
                        bOptionsChanged = true;
                    }

                    bOk = true;
                }
                break;
            case HANDLE_PRINTHANDOUT:
                if( *pValues >>= bValue )
                {
                    if( aPrintOpts.IsHandout() != bValue)
                    {
                        aPrintOpts.SetHandout( bValue );
                        bOptionsChanged = true;
                    }

                    bOk = true;
                }
                break;
            case HANDLE_PRINTOUTLINE:
                if( *pValues >>= bValue )
                {
                    if( aPrintOpts.IsOutline() != bValue)
                    {
                        aPrintOpts.SetOutline( bValue );
                        bOptionsChanged = true;
                    }
                    bOk = true;
                }
                break;
            case HANDLE_SLIDESPERHANDOUT:
                {
                    sal_Int16 nValue = 0;
                    if( (*pValues >>= nValue) && (nValue >= 1) && (nValue <= 9) )
                    {
                        if( static_cast<sal_Int16>( aPrintOpts.GetHandoutPages() ) != nValue )
                        {
                            aPrintOpts.SetHandoutPages( static_cast< sal_uInt16 >( nValue ) );
                            bOptionsChanged = true;
                        }
                        bOk = true;
                    }
                }
                break;
            case HANDLE_HANDOUTHORIZONTAL:
                if( *pValues >>= bValue )
                {
                    if( aPrintOpts.IsHandoutHorizontal() != bValue )
                    {
                        aPrintOpts.SetHandoutHorizontal( bValue );
                        bOptionsChanged = true;
                    }
                    bOk = true;
                }
                break;

            case HANDLE_PRINTPAGENAME:
                if( *pValues >>= bValue )
                {
                    if( aPrintOpts.IsPagename() != bValue)
                    {
                        aPrintOpts.SetPagename( bValue );
                        bOptionsChanged = true;
                    }
                    bOk = true;
                }
                break;
            case HANDLE_PRINTDATE:
                if( *pValues >>= bValue )
                {
                    if( aPrintOpts.IsDate() != bValue)
                    {
                        aPrintOpts.SetDate( bValue );
                        bOptionsChanged = true;
                    }
                    bOk = true;
                }
                break;
            case HANDLE_PRINTTIME:
                if( *pValues >>= bValue )
                {
                    if( aPrintOpts.IsDate() != bValue)
                    {
                        aPrintOpts.SetTime( bValue );
                        bOptionsChanged = true;
                    }
                    bOk = true;
                }
                break;
            case HANDLE_PRINTHIDENPAGES:
                if( *pValues >>= bValue )
                {
                    if( aPrintOpts.IsHiddenPages() != bValue)
                    {
                        aPrintOpts.SetHiddenPages( bValue );
                        bOptionsChanged = true;
                    }
                    bOk = true;
                }
                break;
            case HANDLE_PRINTFITPAGE:
                if( *pValues >>= bValue )
                {
                    if( aPrintOpts.IsPagesize() != bValue)
                    {
                        aPrintOpts.SetPagesize( bValue );
                        bOptionsChanged = true;
                    }
                    bOk = true;
                }
                break;
            case HANDLE_PRINTTILEPAGE:
                if( *pValues >>= bValue )
                {
                    if( aPrintOpts.IsPagetile() != bValue)
                    {
                        aPrintOpts.SetPagetile( bValue );
                        bOptionsChanged = true;
                    }
                    bOk = true;
                }
                break;
            case HANDLE_PRINTBOOKLET:
                if( *pValues >>= bValue )
                {
                    if( aPrintOpts.IsBooklet() != bValue)
                    {
                        aPrintOpts.SetBooklet( bValue );
                        bOptionsChanged = true;
                    }
                    bOk = true;
                }
                break;
            case HANDLE_PRINTBOOKLETFRONT:
                if( *pValues >>= bValue )
                {
                    if( aPrintOpts.IsFrontPage() != bValue)
                    {
                        aPrintOpts.SetFrontPage( bValue );
                        bOptionsChanged = true;
                    }
                    bOk = true;
                }
                break;
            case HANDLE_PRINTBOOKLETBACK:
                if( *pValues >>= bValue )
                {
                    if( aPrintOpts.IsBackPage() != bValue)
                    {
                        aPrintOpts.SetBackPage( bValue );
                        bOptionsChanged = true;
                    }
                    bOk = true;
                }
                break;
            case HANDLE_PRINTQUALITY:
                {
                    sal_Int32 nValue = 0;
                    if( *pValues >>= nValue )
                    {
                        if( aPrintOpts.GetOutputQuality() != nValue)
                        {
                            aPrintOpts.SetOutputQuality( static_cast<sal_uInt16>(nValue) );
                            bOptionsChanged = true;
                        }
                        bOk = true;
                    }
                }
                break;
            case HANDLE_MEASUREUNIT:
                {
                    sal_Int16 nValue = 0;
                    if( *pValues >>= nValue )
                    {
                        FieldUnit nFieldUnit;
                        if( SvxMeasureUnitToFieldUnit( nValue, nFieldUnit ) )
                        {
                            pDoc->SetUIUnit( nFieldUnit );
                            bOk = true;
                        }
                    }
                }
                break;
            case HANDLE_SCALE_NUM:
                {
                    sal_Int32 nValue = 0;
                    if( *pValues >>= nValue )
                    {
                        Fraction aFract( nValue, pDoc->GetUIScale().GetDenominator() );
                        pDoc->SetUIScale( aFract );
                        bOk = true;
                        bChanged = true;
                    }
                }
                break;
            case HANDLE_SCALE_DOM:
                {
                    sal_Int32 nValue = 0;
                    if( *pValues >>= nValue )
                    {
                        Fraction aFract( pDoc->GetUIScale().GetNumerator(), nValue );
                        pDoc->SetUIScale( aFract );
                        bOk = true;
                        bChanged = true;
                    }
                }
                break;

            case HANDLE_TABSTOP:
                {
                    sal_Int32 nValue = 0;
                    if( (*pValues >>= nValue) && (nValue >= 0) )
                    {
                        pDoc->SetDefaultTabulator(static_cast<sal_uInt16>(nValue));
                        bOk = true;
                        bChanged = true;
                    }
                }
                break;
            case HANDLE_PAGENUMFMT:
                {
                    sal_Int32 nValue = 0;
                    if( (*pValues >>= nValue ) && (nValue >= css::style::NumberingType::CHARS_UPPER_LETTER ) && (nValue <= css::style::NumberingType::PAGE_DESCRIPTOR) )
                    {
                        pDoc->SetPageNumType(static_cast<SvxNumType>(nValue));
                        bOk = true;
                        bChanged = true;
                    }
                }
                break;
            case HANDLE_PRINTERNAME:
                {
                    OUString aPrinterName;
                    if( *pValues >>= aPrinterName )
                    {
                        bOk = true;
                        if( !aPrinterName.isEmpty() && pDocSh->GetCreateMode() != SfxObjectCreateMode::EMBEDDED )
                        {
                            SfxPrinter *pTempPrinter = pDocSh->GetPrinter( true );
                            if (pTempPrinter)
                            {
                                VclPtr<SfxPrinter> pNewPrinter = VclPtr<SfxPrinter>::Create( pTempPrinter->GetOptions().Clone(), aPrinterName );
                                pDocSh->SetPrinter( pNewPrinter );
                            }
                        }
                    }
                }
                break;
            case HANDLE_PRINTERJOB:
                {
                    Sequence < sal_Int8 > aSequence;
                    if ( *pValues >>= aSequence )
                    {
                        bOk = true;
                        sal_uInt32 nSize = aSequence.getLength();
                        if( nSize )
                        {
                            SvMemoryStream aStream (aSequence.getArray(), nSize, StreamMode::READ );
                            aStream.Seek ( STREAM_SEEK_TO_BEGIN );
                            std::unique_ptr<SfxItemSet> pItemSet;

                            bool bPreferPrinterPapersize = false;
                            if( pPrinter )
                            {
                                pItemSet = pPrinter->GetOptions().Clone();
                                bPreferPrinterPapersize = pPrinter->GetPrinterSettingsPreferred();
                            }
                            else
                            {
                                pItemSet = std::make_unique<SfxItemSet>(pDoc->GetPool(),
                                            svl::Items<SID_PRINTER_NOTFOUND_WARN,  SID_PRINTER_NOTFOUND_WARN,
                                            SID_PRINTER_CHANGESTODOC,   SID_PRINTER_CHANGESTODOC,
                                            ATTR_OPTIONS_PRINT,         ATTR_OPTIONS_PRINT>{} );
                            }

                            pPrinter = SfxPrinter::Create ( aStream, std::move(pItemSet) );
                            pPrinter->SetPrinterSettingsPreferred( bPreferPrinterPapersize );

                            MapMode aMM (pPrinter->GetMapMode());
                            aMM.SetMapUnit(MapUnit::Map100thMM);
                            pPrinter->SetMapMode(aMM);

                            pDocSh->SetPrinter( pPrinter );

                            pPrinter = nullptr;
                        }
                    }
                }
                break;

            case HANDLE_PRINTERPAPERSIZE:
                {
                    bool bPreferPrinterPapersize;
                    if( *pValues >>= bPreferPrinterPapersize )
                    {
                        bOk = true;
                        if( pDocSh->GetCreateMode() != SfxObjectCreateMode::EMBEDDED )
                        {
                            SfxPrinter *pTempPrinter = pDocSh->GetPrinter( true );
                            if (pTempPrinter)
                                pTempPrinter->SetPrinterSettingsPreferred( bPreferPrinterPapersize );
                        }
                    }
                }
                break;

            case HANDLE_PARAGRAPHSUMMATION :
            {
                bool bIsSummationOfParagraphs = false;
                if ( *pValues >>= bIsSummationOfParagraphs )
                {
                    bOk = true;
                    bChanged = true;
                    if ( pDoc->GetDocumentType() == DocumentType::Impress )
                    {
                        EEControlBits nSum = bIsSummationOfParagraphs ? EEControlBits::ULSPACESUMMATION : EEControlBits::NONE;
                        EEControlBits nCntrl;

                        pDoc->SetSummationOfParagraphs( bIsSummationOfParagraphs );
                        SdDrawDocument* pDocument = pDocSh->GetDoc();
                        SdrOutliner& rOutl = pDocument->GetDrawOutliner();
                        nCntrl = rOutl.GetControlWord() &~ EEControlBits::ULSPACESUMMATION;
                        rOutl.SetControlWord( nCntrl | nSum );
                        SdOutliner* pOutl = pDocument->GetOutliner( false );
                        if( pOutl )
                        {
                            nCntrl = pOutl->GetControlWord() &~ EEControlBits::ULSPACESUMMATION;
                            pOutl->SetControlWord( nCntrl | nSum );
                        }
                        pOutl = pDocument->GetInternalOutliner( false );
                        if( pOutl )
                        {
                            nCntrl = pOutl->GetControlWord() &~ EEControlBits::ULSPACESUMMATION;
                            pOutl->SetControlWord( nCntrl | nSum );
                        }
                    }
                }
            }
            break;

            case HANDLE_CHARCOMPRESS:
            {
                sal_Int16 nCharCompressType = 0;
                if( *pValues >>= nCharCompressType )
                {
                    bOk = true;

                    pDoc->SetCharCompressType( static_cast<CharCompressType>(nCharCompressType) );
                    SdDrawDocument* pDocument = pDocSh->GetDoc();
                    SdrOutliner& rOutl = pDocument->GetDrawOutliner();
                    rOutl.SetAsianCompressionMode( static_cast<CharCompressType>(nCharCompressType) );
                    SdOutliner* pOutl = pDocument->GetOutliner( false );
                    if( pOutl )
                    {
                        pOutl->SetAsianCompressionMode( static_cast<CharCompressType>(nCharCompressType) );
                    }
                    pOutl = pDocument->GetInternalOutliner( false );
                    if( pOutl )
                    {
                        pOutl->SetAsianCompressionMode( static_cast<CharCompressType>(nCharCompressType) );
                    }
                }
                break;

            }
            case HANDLE_ASIANPUNCT:
            {
                bool bAsianPunct = false;
                if( *pValues >>= bAsianPunct )
                {
                    bOk = true;

                    pDoc->SetKernAsianPunctuation( bAsianPunct );
                    SdDrawDocument* pDocument = pDocSh->GetDoc();
                    SdrOutliner& rOutl = pDocument->GetDrawOutliner();
                    rOutl.SetKernAsianPunctuation( bAsianPunct );
                    SdOutliner* pOutl = pDocument->GetOutliner( false );
                    if( pOutl )
                    {
                        pOutl->SetKernAsianPunctuation( bAsianPunct );
                    }
                    pOutl = pDocument->GetInternalOutliner( false );
                    if( pOutl )
                    {
                        pOutl->SetKernAsianPunctuation( bAsianPunct );
                    }
                }
                break;

            }
            case HANDLE_UPDATEFROMTEMPLATE:
            {
                bool value = false;
                if( *pValues >>= value )
                {
                    bChanged = ( value != pDocSh->IsQueryLoadTemplate() );
                    pDocSh->SetQueryLoadTemplate( value );
                    bOk = true;
                }
            }
            break;

            case HANDLE_PRINTER_INDEPENDENT_LAYOUT:
            {
                // Just propagate the new printer independent layout mode to
                // the document and determine it really differs from the old
                // one.
                sal_Int16 nOldValue =
                    static_cast<sal_Int16>(pDoc->GetPrinterIndependentLayout ());
                sal_Int16 nValue = 0;
                if (*pValues >>= nValue)
                {
                    pDoc->SetPrinterIndependentLayout (nValue);
                    bChanged = (nValue != nOldValue);
                    bOk = true;
                }
            }
            break;

            // --> #i33095#
            case HANDLE_LOAD_READONLY:
            {
                bool bNewValue = false;
                if ( *pValues >>= bNewValue )
                {
                    bChanged = ( pDocSh->IsLoadReadonly() != bNewValue );
                    pDocSh->SetLoadReadonly( bNewValue );
                    bOk = true;
                }
            }
            break;

            case HANDLE_MODIFY_PASSWD:
            {
                uno::Sequence< beans::PropertyValue > aInfo;
                if ( !( *pValues >>= aInfo ) )
                    throw lang::IllegalArgumentException(
                        "Value of type Sequence<PropertyValue> expected!",
                        uno::Reference< uno::XInterface >(),
                        2 );

                if ( !pDocSh->SetModifyPasswordInfo( aInfo ) )
                    throw beans::PropertyVetoException(
                        "The hash is not allowed to be changed now!" );

            }
            break;

            case HANDLE_SAVE_VERSION:
            {
                bool bNewValue = false;
                if ( *pValues >>= bNewValue )
                {
                    bChanged = ( pDocSh->IsSaveVersionOnClose() != bNewValue );
                    pDocSh->SetSaveVersionOnClose( bNewValue );
                    bOk = true;
                }
            }
            break;

            case HANDLE_EMBED_FONTS:
            {
                if (pValues->has<bool>())
                {
                    bool bNewValue = pValues->get<bool>();
                    bChanged = (pDoc->IsEmbedFonts() != bNewValue);
                    pDoc->SetEmbedFonts(bNewValue);
                    bOk = true;
                }
            }
            break;

            case HANDLE_EMBED_USED_FONTS:
            {
                if (pValues->has<bool>())
                {
                    bool bNewValue = pValues->get<bool>();
                    bChanged = (pDoc->IsEmbedUsedFontsOnly() != bNewValue);
                    pDoc->SetEmbedUsedFontsOnly(bNewValue);
                    bOk = true;
                }
            }
            break;

            case HANDLE_EMBED_LATIN_SCRIPT_FONTS:
            {
                if (pValues->has<bool>())
                {
                    bool bNewValue = pValues->get<bool>();
                    bChanged = (pDoc->IsEmbedFontScriptLatin() != bNewValue);
                    pDoc->SetEmbedFontScriptLatin(bNewValue);
                    bOk = true;
                }
            }
            break;

            case HANDLE_EMBED_ASIAN_SCRIPT_FONTS:
            {
                if (pValues->has<bool>())
                {
                    bool bNewValue = pValues->get<bool>();
                    bChanged = (pDoc->IsEmbedFontScriptAsian() != bNewValue);
                    pDoc->SetEmbedFontScriptAsian(bNewValue);
                    bOk = true;
                }
            }
            break;

            case HANDLE_EMBED_COMPLEX_SCRIPT_FONTS:
            {
                if (pValues->has<bool>())
                {
                    bool bNewValue = pValues->get<bool>();
                    bChanged = (pDoc->IsEmbedFontScriptComplex() != bNewValue);
                    pDoc->SetEmbedFontScriptComplex(bNewValue);
                    bOk = true;
                }
            }
            break;

            default:
                throw UnknownPropertyException( OUString::number((*ppEntries)->mnHandle), static_cast<cppu::OWeakObject*>(this));
        }

        if( !bOk )
            throw IllegalArgumentException();
    }

    if( bOptionsChanged )
    {
        if( !pPrinter )
            pPrinter = pDocSh->GetPrinter( true );
        SfxItemSet aNewOptions( pPrinter->GetOptions() );
        aNewOptions.Put( aOptionsPrintItem );
        pPrinter->SetOptions( aNewOptions );
    }

    if( bChanged || bOptionsChanged )
        mxModel->SetModified();
}

void DocumentSettings::ExtractURL( XPropertyListType t, Any* pValue )
{
    XPropertyListRef pList = mxModel->GetDoc()->GetPropertyList( t );
    if( !pList.is() )
        return;

    INetURLObject aPathURL( pList->GetPath() );
    aPathURL.insertName( pList->GetName() );
    aPathURL.setExtension( pList->GetDefaultExt() );
    OUString aPath( aPathURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
    *pValue <<= aPath;
}

void
DocumentSettings::_getPropertyValues(
        const PropertyMapEntry** ppEntries, Any* pValue)
{
    ::SolarMutexGuard aGuard;

    SdDrawDocument* pDoc = mxModel->GetDoc();
    ::sd::DrawDocShell* pDocSh = mxModel->GetDocShell();
    if( nullptr == pDoc || nullptr == pDocSh )
    {
        throw RuntimeException("Document or Shell missing",
                static_cast<OWeakObject *>(this));
    }

    SdOptionsPrintItem aOptionsPrintItem;

    SfxPrinter* pPrinter = pDocSh->GetPrinter( false );
    if( pPrinter )
    {
        SdOptionsPrintItem const * pPrinterOptions = nullptr;
        if(pPrinter->GetOptions().GetItemState( ATTR_OPTIONS_PRINT, false, reinterpret_cast<const SfxPoolItem**>(&pPrinterOptions)) == SfxItemState::SET)
            aOptionsPrintItem.GetOptionsPrint() = pPrinterOptions->GetOptionsPrint();
    }
    else
    {
        aOptionsPrintItem.SetOptions( SD_MOD()->GetSdOptions(pDoc->GetDocumentType()) );
    }
    SdOptionsPrint& aPrintOpts = aOptionsPrintItem.GetOptionsPrint();

    for( ; *ppEntries; ppEntries++, pValue++ )
    {
        switch( (*ppEntries)->mnHandle )
        {
            case HANDLE_COLORTABLEURL:
                ExtractURL( XPropertyListType::Color, pValue );
                break;
            case HANDLE_DASHTABLEURL:
                ExtractURL( XPropertyListType::Dash, pValue );
                break;
            case HANDLE_LINEENDTABLEURL:
                ExtractURL( XPropertyListType::LineEnd, pValue );
                break;
            case HANDLE_HATCHTABLEURL:
                ExtractURL( XPropertyListType::Hatch, pValue );
                break;
            case HANDLE_GRADIENTTABLEURL:
                ExtractURL( XPropertyListType::Gradient, pValue );
                break;
            case HANDLE_BITMAPTABLEURL:
                ExtractURL( XPropertyListType::Bitmap, pValue );
                break;
            case HANDLE_FORBIDDENCHARS:
                *pValue <<= mxModel->getForbiddenCharsTable();
                break;
            case HANDLE_APPLYUSERDATA:
                *pValue <<= pDocSh->IsUseUserData();
                break;
            case HANDLE_SAVETHUMBNAIL:
                *pValue <<= pDocSh->IsUseThumbnailSave();
                break;
            case HANDLE_PRINTDRAWING:
                *pValue <<= aPrintOpts.IsDraw();
                break;
            case HANDLE_PRINTNOTES:
                *pValue <<= aPrintOpts.IsNotes();
                break;
            case HANDLE_PRINTHANDOUT:
                *pValue <<= aPrintOpts.IsHandout();
                break;
            case HANDLE_PRINTOUTLINE:
                *pValue <<= aPrintOpts.IsOutline();
                break;
            case HANDLE_SLIDESPERHANDOUT:
                *pValue <<= static_cast<sal_Int16>(aPrintOpts.GetHandoutPages());
                break;
            case HANDLE_HANDOUTHORIZONTAL:
                *pValue <<= aPrintOpts.IsHandoutHorizontal();
                break;
            case HANDLE_PRINTPAGENAME:
                *pValue <<= aPrintOpts.IsPagename();
                break;
            case HANDLE_PRINTDATE:
                *pValue <<= aPrintOpts.IsDate();
                break;
            case HANDLE_PRINTTIME:
                *pValue <<= aPrintOpts.IsTime();
                break;
            case HANDLE_PRINTHIDENPAGES:
                *pValue <<= aPrintOpts.IsHiddenPages();
                break;
            case HANDLE_PRINTFITPAGE:
                *pValue <<= aPrintOpts.IsPagesize();
                break;
            case HANDLE_PRINTTILEPAGE:
                *pValue <<= aPrintOpts.IsPagetile();
                break;
            case HANDLE_PRINTBOOKLET:
                *pValue <<= aPrintOpts.IsBooklet();
                break;
            case HANDLE_PRINTBOOKLETFRONT:
                *pValue <<= aPrintOpts.IsFrontPage();
                break;
            case HANDLE_PRINTBOOKLETBACK:
                *pValue <<= aPrintOpts.IsBackPage();
                break;
            case HANDLE_PRINTQUALITY:
                *pValue <<= static_cast<sal_Int32>(aPrintOpts.GetOutputQuality());
                break;
            case HANDLE_MEASUREUNIT:
                {
                    short nMeasure;
                    SvxFieldUnitToMeasureUnit( pDoc->GetUIUnit(), nMeasure );
                    *pValue <<= static_cast<sal_Int16>(nMeasure);
                }
                break;
            case HANDLE_SCALE_NUM:
                *pValue <<= pDoc->GetUIScale().GetNumerator();
                break;
            case HANDLE_SCALE_DOM:
                *pValue <<= pDoc->GetUIScale().GetDenominator();
                break;
            case HANDLE_TABSTOP:
                *pValue <<= static_cast<sal_Int32>(pDoc->GetDefaultTabulator());
                break;
            case HANDLE_PAGENUMFMT:
                *pValue <<= static_cast<sal_Int32>(pDoc->GetPageNumType());
                break;
            case HANDLE_PRINTERNAME:
                {
                    SfxPrinter *pTempPrinter = pDocSh->GetPrinter( false );
                    *pValue <<= pTempPrinter ? pTempPrinter->GetName() : OUString();
                }
                break;
            case HANDLE_PRINTERJOB:
                {
                    SfxPrinter *pTempPrinter = pDocSh->GetPrinter( false );
                    if (pTempPrinter)
                    {
                        SvMemoryStream aStream;
                        pTempPrinter->Store( aStream );
                        *pValue <<= Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aStream.GetData() ),
                                                        aStream.TellEnd() );
                    }
                    else
                    {
                        Sequence < sal_Int8 > aSequence;
                        *pValue <<= aSequence;
                    }
                }
                break;

            case HANDLE_PRINTERPAPERSIZE:
                {
                    SfxPrinter *pTempPrinter = pDocSh->GetPrinter( false );
                    *pValue <<= pTempPrinter && pTempPrinter->GetPrinterSettingsPreferred();
                }
                break;

            case HANDLE_PARAGRAPHSUMMATION :
            {
                bool bIsSummationOfParagraphs = pDoc->IsSummationOfParagraphs();
                *pValue <<= bIsSummationOfParagraphs;
            }
            break;

            case HANDLE_CHARCOMPRESS:
            {
                *pValue <<= static_cast<sal_Int16>(pDoc->GetCharCompressType());
                break;
            }

            case HANDLE_ASIANPUNCT:
            {
                *pValue <<= pDoc->IsKernAsianPunctuation();
                break;
            }

            case HANDLE_UPDATEFROMTEMPLATE:
            {
                *pValue <<= pDocSh->IsQueryLoadTemplate();
            }
            break;

            case HANDLE_PRINTER_INDEPENDENT_LAYOUT:
            {
                sal_Int16 nPrinterIndependentLayout =
                    static_cast<sal_Int16>(pDoc->GetPrinterIndependentLayout());
                *pValue <<= nPrinterIndependentLayout;
            }
            break;

            // --> #i33095#
            case HANDLE_LOAD_READONLY:
            {
                *pValue <<= pDocSh->IsLoadReadonly();
            }
            break;

            case HANDLE_MODIFY_PASSWD:
            {
                *pValue <<= pDocSh->GetModifyPasswordInfo();
            }
            break;

            case HANDLE_SAVE_VERSION:
            {
                *pValue <<= pDocSh->IsSaveVersionOnClose();
            }
            break;

            case HANDLE_EMBED_FONTS:
            {
                *pValue <<= pDoc->IsEmbedFonts();
            }
            break;

            case HANDLE_EMBED_USED_FONTS:
            {
                *pValue <<= pDoc->IsEmbedUsedFontsOnly();
            }
            break;

            case HANDLE_EMBED_LATIN_SCRIPT_FONTS:
            {
                *pValue <<= pDoc->IsEmbedFontScriptLatin();
            }
            break;

            case HANDLE_EMBED_ASIAN_SCRIPT_FONTS:
            {
                *pValue <<= pDoc->IsEmbedFontScriptAsian();
            }
            break;

            case HANDLE_EMBED_COMPLEX_SCRIPT_FONTS:
            {
                *pValue <<= pDoc->IsEmbedFontScriptComplex();
            }
            break;

            default:
                throw UnknownPropertyException( OUString::number((*ppEntries)->mnHandle), static_cast<cppu::OWeakObject*>(this));
        }
    }
}

// XInterface
Any SAL_CALL DocumentSettings::queryInterface( const Type& aType )
{
    return WeakImplHelper< XPropertySet, XMultiPropertySet, XServiceInfo >::queryInterface( aType );
}

void SAL_CALL DocumentSettings::acquire(  ) throw ()
{
    WeakImplHelper< XPropertySet, XMultiPropertySet, XServiceInfo >::acquire();
}

void SAL_CALL DocumentSettings::release(  ) throw ()
{
    WeakImplHelper< XPropertySet, XMultiPropertySet, XServiceInfo >::release();
}

// XPropertySet
Reference< XPropertySetInfo > SAL_CALL DocumentSettings::getPropertySetInfo(  )
{
    return PropertySetHelper::getPropertySetInfo();
}

void SAL_CALL DocumentSettings::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
{
    PropertySetHelper::setPropertyValue( aPropertyName, aValue );
}

Any SAL_CALL DocumentSettings::getPropertyValue( const OUString& PropertyName )
{
    return PropertySetHelper::getPropertyValue( PropertyName );
}

void SAL_CALL DocumentSettings::addPropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener )
{
    PropertySetHelper::addPropertyChangeListener( aPropertyName, xListener );
}

void SAL_CALL DocumentSettings::removePropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener )
{
    PropertySetHelper::removePropertyChangeListener( aPropertyName, aListener );
}

void SAL_CALL DocumentSettings::addVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
{
    PropertySetHelper::addVetoableChangeListener( PropertyName, aListener );
}

void SAL_CALL DocumentSettings::removeVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
{
    PropertySetHelper::removeVetoableChangeListener( PropertyName, aListener );
}

// XMultiPropertySet
void SAL_CALL DocumentSettings::setPropertyValues( const Sequence< OUString >& aPropertyNames, const Sequence< Any >& aValues )
{
    PropertySetHelper::setPropertyValues( aPropertyNames, aValues );
}

Sequence< Any > SAL_CALL DocumentSettings::getPropertyValues( const Sequence< OUString >& aPropertyNames )
{
    return PropertySetHelper::getPropertyValues( aPropertyNames );
}

void SAL_CALL DocumentSettings::addPropertiesChangeListener( const Sequence< OUString >& aPropertyNames, const Reference< XPropertiesChangeListener >& xListener )
{
    PropertySetHelper::addPropertiesChangeListener( aPropertyNames, xListener );
}

void SAL_CALL DocumentSettings::removePropertiesChangeListener( const Reference< XPropertiesChangeListener >& xListener )
{
    PropertySetHelper::removePropertiesChangeListener( xListener );
}

void SAL_CALL DocumentSettings::firePropertiesChangeEvent( const Sequence< OUString >& aPropertyNames, const Reference< XPropertiesChangeListener >& xListener )
{
    PropertySetHelper::firePropertiesChangeEvent( aPropertyNames, xListener );
}

// XServiceInfo
OUString SAL_CALL DocumentSettings::getImplementationName(  )
{
    return OUString( "com.sun.star.comp.Draw.DocumentSettings" );
}

sal_Bool SAL_CALL DocumentSettings::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL DocumentSettings::getSupportedServiceNames(  )
{
    Sequence< OUString > aSeq( 2 );
    aSeq[0] = "com.sun.star.document.Settings" ;
    if( mxModel->IsImpressDocument() )
    {
        aSeq[1] = "com.sun.star.presentation.DocumentSettings" ;
    }
    else
    {
        aSeq[1] = "com.sun.star.drawing.DocumentSettings" ;
    }

    return aSeq;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
