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
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "UnoDocumentSettings.hxx"
#include "unomodel.hxx"

#include "optsitem.hxx"
#include <sfx2/printer.hxx>
#include "sdattr.hxx"
#include "../inc/ViewShell.hxx"
#include "../inc/FrameView.hxx"
#include "Outliner.hxx"
#include <xmloff/settingsstore.hxx>
#include <editeng/editstat.hxx>
#include <svx/unoapi.hxx>

using namespace ::comphelper;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::i18n;

namespace sd
{
    class DocumentSettings : public WeakImplHelper< XPropertySet, XMultiPropertySet, XServiceInfo >,
                             public comphelper::PropertySetHelper,
                             public DocumentSettingsSerializer
    {
    public:
        DocumentSettings( SdXImpressDocument* pModel );
        virtual ~DocumentSettings() throw();

        // XInterface
        virtual Any SAL_CALL queryInterface( const Type& aType ) throw (RuntimeException, std::exception) override;
        virtual void SAL_CALL acquire(  ) throw () override;
        virtual void SAL_CALL release(  ) throw () override;

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XMultiPropertySet
        virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException, std::exception) override;
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException, std::exception) override;

        // DocumentSettingsSerializer cf. xmloff
        virtual uno::Sequence<beans::PropertyValue>
                filterStreamsFromStorage(OUString const & referer,
                                         const uno::Reference< embed::XStorage > &xStorage,
                                         const uno::Sequence<beans::PropertyValue>& aConfigProps ) override;
        virtual uno::Sequence<beans::PropertyValue>
                filterStreamsToStorage(const uno::Reference< embed::XStorage > &xStorage,
                                       const uno::Sequence<beans::PropertyValue>& aConfigProps ) override;

    protected:
        virtual void _setPropertyValues( const comphelper::PropertyMapEntry** ppEntries, const ::com::sun::star::uno::Any* pValues ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, RuntimeException, std::exception ) override;
        virtual void _getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, ::com::sun::star::uno::Any* pValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, RuntimeException, std::exception ) override;

    private:
        bool LoadList( XPropertyListType t, const OUString &rPath,
                       const OUString &rReferer,
                       const uno::Reference< embed::XStorage > &xStorage );
        void AssignURL( XPropertyListType t, const Any* pValue, bool *pOk, bool *pChanged );
        void ExtractURL( XPropertyListType t, Any* pValue );
        rtl::Reference<SdXImpressDocument> mxModel;
    };

    Reference< XInterface > SAL_CALL DocumentSettings_createInstance( SdXImpressDocument* pModel )
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
    HANDLE_GRADIENTTABLEURL, HANDLE_BITMAPTABLEURL, HANDLE_FORBIDDENCHARS, HANDLE_APPLYUSERDATA, HANDLE_PAGENUMFMT,
    HANDLE_PRINTERNAME, HANDLE_PRINTERJOB, HANDLE_PARAGRAPHSUMMATION, HANDLE_CHARCOMPRESS, HANDLE_ASIANPUNCT, HANDLE_UPDATEFROMTEMPLATE,
    HANDLE_PRINTER_INDEPENDENT_LAYOUT
    // #i33095#
    ,HANDLE_LOAD_READONLY, HANDLE_MODIFY_PASSWD, HANDLE_SAVE_VERSION
    ,HANDLE_SLIDESPERHANDOUT, HANDLE_HANDOUTHORIZONTAL, HANDLE_EMBED_FONTS
};

#define MID_PRINTER 1

    PropertySetInfo * createSettingsInfoImpl( bool bIsDraw )
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
            { OUString("EmbedFonts"),            HANDLE_EMBED_FONTS,         cppu::UnoType<bool>::get(),                0,  0 },
            { OUString(), 0, css::uno::Type(), 0, 0 }
        };

        PropertySetInfo* pInfo = new PropertySetInfo( aCommonSettingsInfoMap );
        pInfo->add( bIsDraw ? aDrawSettingsInfoMap : aImpressSettingsInfoMap );

        return pInfo;
    }
}

using namespace ::sd;

DocumentSettings::DocumentSettings( SdXImpressDocument* pModel )
:   PropertySetHelper( createSettingsInfoImpl( !pModel->IsImpressDocument() ) ),
    mxModel( pModel )
{
}

DocumentSettings::~DocumentSettings() throw()
{
}

bool DocumentSettings::LoadList( XPropertyListType t, const OUString &rInPath,
                                 const OUString &rReferer,
                                 const uno::Reference< embed::XStorage > &xStorage )
{
    SdDrawDocument* pDoc = mxModel->GetDoc();

    sal_Int32 nSlash = rInPath.lastIndexOf('/');
    OUString aPath, aName;
    if (nSlash < -1)
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
    if( !(bool)( *pValue >>= aURL ) )
        return;

    if( LoadList( t, aURL, ""/*TODO?*/, uno::Reference< embed::XStorage >() ) )
        *pOk = *pChanged = true;
}

static struct {
    const char *pName;
    XPropertyListType t;
} aURLPropertyNames[] = {
    { "ColorTableURL", XCOLOR_LIST },
    { "DashTableURL", XDASH_LIST },
    { "LineEndTableURL", XLINE_END_LIST },
    { "HatchTableURL", XHATCH_LIST },
    { "GradientTableURL", XGRADIENT_LIST },
    { "BitmapTableURL", XBITMAP_LIST }
};

static XPropertyListType getTypeOfName( const OUString &aName )
{
    for( size_t i = 0; i < SAL_N_ELEMENTS( aURLPropertyNames ); i++ ) {
        if( aName.equalsAscii( aURLPropertyNames[i].pName ) )
            return aURLPropertyNames[i].t;
    }
    return UNKNOWN_XPROPERTYLISTTYPE;
}

static OUString getNameOfType( XPropertyListType t )
{
    for( size_t i = 0; i < SAL_N_ELEMENTS( aURLPropertyNames ); i++ ) {
        if( t == aURLPropertyNames[i].t )
            return OUString( aURLPropertyNames[i].pName,
                                  strlen( aURLPropertyNames[i].pName ) - 3,
                                  RTL_TEXTENCODING_UTF8 );
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
        if (t == UNKNOWN_XPROPERTYLISTTYPE)
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
        XPropertyListRef pList = pDoc->GetPropertyList( (XPropertyListType) i );
        if( ( bHasEmbed = pList.is() && pList->IsEmbedInDocument() ) )
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
            if (t >= 0) {
                XPropertyListRef pList = pDoc->GetPropertyList( t );
                if( !pList.is() || !pList->IsEmbedInDocument() )
                    continue; // no change ...
                else
                {
                    // Such specific path construction is grim.
                    OUString aValue;
                    aRet[i].Value >>= aValue;

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
    } catch (const uno::Exception &e) {
        (void)e;
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
throw (UnknownPropertyException, PropertyVetoException,
    IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;

    SdDrawDocument* pDoc = mxModel->GetDoc();
    ::sd::DrawDocShell* pDocSh = mxModel->GetDocShell();
    if( NULL == pDoc || NULL == pDocSh )
    {
        throw RuntimeException("Document or Shell missing",
                static_cast<OWeakObject *>(this));
    }

    bool bValue = false;
    bool bOk, bChanged = false, bOptionsChanged = false;

    SdOptionsPrintItem aOptionsPrintItem( ATTR_OPTIONS_PRINT );

    SfxPrinter* pPrinter = pDocSh->GetPrinter( false );
    if( pPrinter )
    {
        SdOptionsPrintItem const * pPrinterOptions = NULL;
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
                AssignURL( XCOLOR_LIST, pValues, &bOk, &bChanged );
                break;

            case HANDLE_DASHTABLEURL:
                AssignURL( XDASH_LIST, pValues, &bOk, &bChanged );
                break;

            case HANDLE_LINEENDTABLEURL:
                AssignURL( XLINE_END_LIST, pValues, &bOk, &bChanged );
                break;

            case HANDLE_HATCHTABLEURL:
                AssignURL( XHATCH_LIST, pValues, &bOk, &bChanged );
                break;

            case HANDLE_GRADIENTTABLEURL:
                AssignURL( XGRADIENT_LIST, pValues, &bOk, &bChanged );
                break;

            case HANDLE_BITMAPTABLEURL:
                AssignURL( XBITMAP_LIST, pValues, &bOk, &bChanged );
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
                            aPrintOpts.SetOutputQuality( (sal_uInt16)nValue );
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
                        pDoc->SetDefaultTabulator((sal_uInt16)nValue);
                        bOk = true;
                        bChanged = true;
                    }
                }
                break;
            case HANDLE_PAGENUMFMT:
                {
                    sal_Int32 nValue = 0;
                    if( (*pValues >>= nValue ) && (nValue >= SVX_CHARS_UPPER_LETTER ) && (nValue <= SVX_PAGEDESC) )
                    {
                        pDoc->SetPageNumType((SvxNumType)nValue);
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
                            SfxItemSet* pItemSet;

                            if( pPrinter )
                            {
                                pItemSet = pPrinter->GetOptions().Clone();
                            }
                            else
                            {
                                pItemSet = new SfxItemSet(pDoc->GetPool(),
                                            SID_PRINTER_NOTFOUND_WARN,  SID_PRINTER_NOTFOUND_WARN,
                                            SID_PRINTER_CHANGESTODOC,   SID_PRINTER_CHANGESTODOC,
                                            ATTR_OPTIONS_PRINT,         ATTR_OPTIONS_PRINT,
                                            0 );
                            }

                            pPrinter = SfxPrinter::Create ( aStream, pItemSet );

                            MapMode aMM (pPrinter->GetMapMode());
                            aMM.SetMapUnit(MAP_100TH_MM);
                            pPrinter->SetMapMode(aMM);

                            pDocSh->SetPrinter( pPrinter );

                            pPrinter = NULL;
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
                    if ( pDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS )
                    {
                        EEControlBits nSum = bIsSummationOfParagraphs ? EEControlBits::ULSPACESUMMATION : EEControlBits::NONE;
                        EEControlBits nCntrl;

                        pDoc->SetSummationOfParagraphs( bIsSummationOfParagraphs );
                        SdDrawDocument* pDocument = pDocSh->GetDoc();
                        SdrOutliner& rOutl = pDocument->GetDrawOutliner();
                        nCntrl = rOutl.GetControlWord() &~ EEControlBits::ULSPACESUMMATION;
                        rOutl.SetControlWord( nCntrl | nSum );
                        ::sd::Outliner* pOutl = pDocument->GetOutliner( false );
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

                    pDoc->SetCharCompressType( (sal_uInt16)nCharCompressType );
                    SdDrawDocument* pDocument = pDocSh->GetDoc();
                    SdrOutliner& rOutl = pDocument->GetDrawOutliner();
                    rOutl.SetAsianCompressionMode( (sal_uInt16)nCharCompressType );
                    ::sd::Outliner* pOutl = pDocument->GetOutliner( false );
                    if( pOutl )
                    {
                        pOutl->SetAsianCompressionMode( (sal_uInt16)nCharCompressType );
                    }
                    pOutl = pDocument->GetInternalOutliner( false );
                    if( pOutl )
                    {
                        pOutl->SetAsianCompressionMode( (sal_uInt16)nCharCompressType );
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
                    ::sd::Outliner* pOutl = pDocument->GetOutliner( false );
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
                    (sal_Int16)pDoc->GetPrinterIndependentLayout ();
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
                        OUString( "Value of type Sequence<PropertyValue> expected!" ),
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
                bool bNewValue = false;
                if ( *pValues >>= bNewValue )
                {
                    bChanged = ( pDoc->IsUsingEmbededFonts() != bNewValue );
                    pDoc->SetIsUsingEmbededFonts( bNewValue );
                    bOk = true;
                }
            }
            break;

            default:
                throw UnknownPropertyException();
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
    OUString aPath( aPathURL.GetMainURL( INetURLObject::NO_DECODE ) );
    *pValue <<= aPath;
}

void
DocumentSettings::_getPropertyValues(
        const PropertyMapEntry** ppEntries, Any* pValue)
throw (UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;

    SdDrawDocument* pDoc = mxModel->GetDoc();
    ::sd::DrawDocShell* pDocSh = mxModel->GetDocShell();
    if( NULL == pDoc || NULL == pDocSh )
    {
        throw RuntimeException("Document or Shell missing",
                static_cast<OWeakObject *>(this));
    }

    SdOptionsPrintItem aOptionsPrintItem( ATTR_OPTIONS_PRINT );

    SfxPrinter* pPrinter = pDocSh->GetPrinter( false );
    if( pPrinter )
    {
        SdOptionsPrintItem const * pPrinterOptions = NULL;
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
                ExtractURL( XCOLOR_LIST, pValue );
                break;
            case HANDLE_DASHTABLEURL:
                ExtractURL( XDASH_LIST, pValue );
                break;
            case HANDLE_LINEENDTABLEURL:
                ExtractURL( XLINE_END_LIST, pValue );
                break;
            case HANDLE_HATCHTABLEURL:
                ExtractURL( XHATCH_LIST, pValue );
                break;
            case HANDLE_GRADIENTTABLEURL:
                ExtractURL( XGRADIENT_LIST, pValue );
                break;
            case HANDLE_BITMAPTABLEURL:
                ExtractURL( XBITMAP_LIST, pValue );
                break;
            case HANDLE_FORBIDDENCHARS:
                *pValue <<= mxModel->getForbiddenCharsTable();
                break;
            case HANDLE_APPLYUSERDATA:
                *pValue <<= pDocSh->IsUseUserData();
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
                *pValue <<= (sal_Int16)aPrintOpts.GetHandoutPages();
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
                *pValue <<= (sal_Int32)aPrintOpts.GetOutputQuality();
                break;
            case HANDLE_MEASUREUNIT:
                {
                    short nMeasure;
                    SvxFieldUnitToMeasureUnit( pDoc->GetUIUnit(), nMeasure );
                    *pValue <<= (sal_Int16)nMeasure;
                }
                break;
            case HANDLE_SCALE_NUM:
                *pValue <<= (sal_Int32)pDoc->GetUIScale().GetNumerator();
                break;
            case HANDLE_SCALE_DOM:
                *pValue <<= (sal_Int32)pDoc->GetUIScale().GetDenominator();
                break;
            case HANDLE_TABSTOP:
                *pValue <<= (sal_Int32)pDoc->GetDefaultTabulator();
                break;
            case HANDLE_PAGENUMFMT:
                *pValue <<= (sal_Int32)pDoc->GetPageNumType();
                break;
            case HANDLE_PRINTERNAME:
                {
                    SfxPrinter *pTempPrinter = pDocSh->GetPrinter( false );
                    *pValue <<= pTempPrinter ? OUString ( pTempPrinter->GetName()) : OUString();
                }
                break;
            case HANDLE_PRINTERJOB:
                {
                    SfxPrinter *pTempPrinter = pDocSh->GetPrinter( false );
                    if (pTempPrinter)
                    {
                        SvMemoryStream aStream;
                        pTempPrinter->Store( aStream );
                        aStream.Seek ( STREAM_SEEK_TO_END );
                        sal_uInt32 nSize = aStream.Tell();
                        aStream.Seek ( STREAM_SEEK_TO_BEGIN );
                        Sequence < sal_Int8 > aSequence ( nSize );
                        memcpy ( aSequence.getArray(), aStream.GetData(), nSize );
                        *pValue <<= aSequence;
                    }
                    else
                    {
                        Sequence < sal_Int8 > aSequence;
                        *pValue <<= aSequence;
                    }
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
                *pValue <<= (sal_Int16)pDoc->GetCharCompressType();
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
                    (sal_Int16)pDoc->GetPrinterIndependentLayout();
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
                *pValue <<= pDoc->IsUsingEmbededFonts();
            }
            break;

            default:
                throw UnknownPropertyException();
        }
    }
}

// XInterface
Any SAL_CALL DocumentSettings::queryInterface( const Type& aType ) throw (RuntimeException, std::exception)
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
Reference< XPropertySetInfo > SAL_CALL DocumentSettings::getPropertySetInfo(  ) throw(RuntimeException, std::exception)
{
    return PropertySetHelper::getPropertySetInfo();
}

void SAL_CALL DocumentSettings::setPropertyValue( const OUString& aPropertyName, const Any& aValue ) throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception)
{
    PropertySetHelper::setPropertyValue( aPropertyName, aValue );
}

Any SAL_CALL DocumentSettings::getPropertyValue( const OUString& PropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    return PropertySetHelper::getPropertyValue( PropertyName );
}

void SAL_CALL DocumentSettings::addPropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    PropertySetHelper::addPropertyChangeListener( aPropertyName, xListener );
}

void SAL_CALL DocumentSettings::removePropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    PropertySetHelper::removePropertyChangeListener( aPropertyName, aListener );
}

void SAL_CALL DocumentSettings::addVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    PropertySetHelper::addVetoableChangeListener( PropertyName, aListener );
}

void SAL_CALL DocumentSettings::removeVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    PropertySetHelper::removeVetoableChangeListener( PropertyName, aListener );
}

// XMultiPropertySet
void SAL_CALL DocumentSettings::setPropertyValues( const Sequence< OUString >& aPropertyNames, const Sequence< Any >& aValues ) throw(PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception)
{
    PropertySetHelper::setPropertyValues( aPropertyNames, aValues );
}

Sequence< Any > SAL_CALL DocumentSettings::getPropertyValues( const Sequence< OUString >& aPropertyNames ) throw(RuntimeException, std::exception)
{
    return PropertySetHelper::getPropertyValues( aPropertyNames );
}

void SAL_CALL DocumentSettings::addPropertiesChangeListener( const Sequence< OUString >& aPropertyNames, const Reference< XPropertiesChangeListener >& xListener ) throw(RuntimeException, std::exception)
{
    PropertySetHelper::addPropertiesChangeListener( aPropertyNames, xListener );
}

void SAL_CALL DocumentSettings::removePropertiesChangeListener( const Reference< XPropertiesChangeListener >& xListener ) throw(RuntimeException, std::exception)
{
    PropertySetHelper::removePropertiesChangeListener( xListener );
}

void SAL_CALL DocumentSettings::firePropertiesChangeEvent( const Sequence< OUString >& aPropertyNames, const Reference< XPropertiesChangeListener >& xListener ) throw(RuntimeException, std::exception)
{
    PropertySetHelper::firePropertiesChangeEvent( aPropertyNames, xListener );
}

// XServiceInfo
OUString SAL_CALL DocumentSettings::getImplementationName(  )
    throw(RuntimeException, std::exception)
{
    return OUString( "com.sun.star.comp.Draw.DocumentSettings" );
}

sal_Bool SAL_CALL DocumentSettings::supportsService( const OUString& ServiceName )
    throw(RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL DocumentSettings::getSupportedServiceNames(  )
    throw(RuntimeException, std::exception)
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
