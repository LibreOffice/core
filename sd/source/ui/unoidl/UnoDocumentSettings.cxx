/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <vector>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <cppuhelper/implbase3.hxx>
#include <comphelper/propertysethelper.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <tools/urlobj.hxx>
#include <svx/xtable.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
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

#define MAP_LEN(x) x, sizeof(x)-1

using namespace ::comphelper;
using namespace ::osl;
using ::rtl::OUString;
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
    class DocumentSettings : public WeakImplHelper3< XPropertySet, XMultiPropertySet, XServiceInfo >,
                             public comphelper::PropertySetHelper,
                             public DocumentSettingsSerializer
    {
    public:
        DocumentSettings( SdXImpressDocument* pModel );
        virtual ~DocumentSettings() throw();

        // XInterface
        virtual Any SAL_CALL queryInterface( const Type& aType ) throw (RuntimeException);
        virtual void SAL_CALL acquire(  ) throw ();
        virtual void SAL_CALL release(  ) throw ();

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // XMultiPropertySet
        virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);

        // DocumentSettingsSerializer cf. xmloff
        virtual uno::Sequence<beans::PropertyValue>
                filterStreamsFromStorage(const uno::Reference< embed::XStorage > &xStorage,
                                         const uno::Sequence<beans::PropertyValue>& aConfigProps );
        virtual uno::Sequence<beans::PropertyValue>
                filterStreamsToStorage(const uno::Reference< embed::XStorage > &xStorage,
                                       const uno::Sequence<beans::PropertyValue>& aConfigProps );

    protected:
        virtual void _setPropertyValues( const comphelper::PropertyMapEntry** ppEntries, const ::com::sun::star::uno::Any* pValues ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
        virtual void _getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, ::com::sun::star::uno::Any* pValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException );

    private:
        bool LoadList( XPropertyListType t, const rtl::OUString &rPath,
                       const uno::Reference< embed::XStorage > &xStorage );
        void AssignURL( XPropertyListType t, const Any* pValue, bool *pOk, bool *pChanged );
        void ExtractURL( XPropertyListType t, Any* pValue );
        Reference< XModel >     mxModel;
        SdXImpressDocument*     mpModel;
    };

    Reference< XInterface > SAL_CALL DocumentSettings_createInstance( SdXImpressDocument* pModel )
        throw( Exception )
    {
        DBG_ASSERT( pModel, "I need a model for the DocumentSettings!" );
        return (XWeak*)new DocumentSettings( pModel );
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
    ,HANDLE_LOAD_READONLY, HANDLE_SAVE_VERSION
    ,HANDLE_SLIDESPERHANDOUT, HANDLE_HANDOUTHORIZONTAL
};

#define MID_PRINTER 1

    PropertySetInfo* createSettingsInfoImpl( sal_Bool bIsDraw )
    {
        static PropertyMapEntry aImpressSettingsInfoMap[] =
        {
            { MAP_LEN("IsPrintDrawing"),        HANDLE_PRINTDRAWING,        &::getBooleanCppuType(),                0,  MID_PRINTER },
            { MAP_LEN("IsPrintNotes"),          HANDLE_PRINTNOTES,          &::getBooleanCppuType(),                0,  MID_PRINTER },
            { MAP_LEN("IsPrintHandout"),        HANDLE_PRINTHANDOUT,        &::getBooleanCppuType(),                0,  MID_PRINTER },
            { MAP_LEN("IsPrintOutline"),        HANDLE_PRINTOUTLINE,        &::getBooleanCppuType(),                0,  MID_PRINTER },
            { MAP_LEN("SlidesPerHandout"),      HANDLE_SLIDESPERHANDOUT,    &::getCppuType((const sal_Int16*)0),    0,  MID_PRINTER },
            { MAP_LEN("HandoutsHorizontal"),    HANDLE_HANDOUTHORIZONTAL,   &::getBooleanCppuType(),                0,  MID_PRINTER },
            { NULL, 0, 0, NULL, 0, 0 }
        };

        static PropertyMapEntry aDrawSettingsInfoMap[] =
        {
            { MAP_LEN("MeasureUnit"),           HANDLE_MEASUREUNIT,         &::getCppuType((const sal_Int16*)0),    0,  0 },
            { MAP_LEN("ScaleNumerator"),        HANDLE_SCALE_NUM,           &::getCppuType((const sal_Int32*)0),    0,  0 },
            { MAP_LEN("ScaleDenominator"),      HANDLE_SCALE_DOM,           &::getCppuType((const sal_Int32*)0),    0,  0 },
            { NULL, 0, 0, NULL, 0, 0 }
        };

        static PropertyMapEntry aCommonSettingsInfoMap[] =
        {
            { MAP_LEN("DefaultTabStop"),        HANDLE_TABSTOP,             &::getCppuType((const sal_Int32*)0),    0,  0 },
            { MAP_LEN("PrinterName"),           HANDLE_PRINTERNAME,         &::getCppuType((const OUString*)0),     0,  0 },
            { MAP_LEN("PrinterSetup"),          HANDLE_PRINTERJOB,          &::getCppuType((const uno::Sequence < sal_Int8 > *)0),  0, MID_PRINTER },

            { MAP_LEN("IsPrintPageName"),       HANDLE_PRINTPAGENAME,       &::getBooleanCppuType(),                0,  MID_PRINTER },
            { MAP_LEN("IsPrintDate"),           HANDLE_PRINTDATE,           &::getBooleanCppuType(),                0,  MID_PRINTER },
            { MAP_LEN("IsPrintTime"),           HANDLE_PRINTTIME,           &::getBooleanCppuType(),                0,  MID_PRINTER },
            { MAP_LEN("IsPrintHiddenPages"),    HANDLE_PRINTHIDENPAGES,     &::getBooleanCppuType(),                0,  MID_PRINTER },
            { MAP_LEN("IsPrintFitPage"),        HANDLE_PRINTFITPAGE,        &::getBooleanCppuType(),                0,  MID_PRINTER },
            { MAP_LEN("IsPrintTilePage"),       HANDLE_PRINTTILEPAGE,       &::getBooleanCppuType(),                0,  MID_PRINTER },
            { MAP_LEN("IsPrintBooklet"),        HANDLE_PRINTBOOKLET,        &::getBooleanCppuType(),                0,  MID_PRINTER },
            { MAP_LEN("IsPrintBookletFront"),   HANDLE_PRINTBOOKLETFRONT,   &::getBooleanCppuType(),                0,  MID_PRINTER },
            { MAP_LEN("IsPrintBookletBack"),    HANDLE_PRINTBOOKLETBACK,    &::getBooleanCppuType(),                0,  MID_PRINTER },
            { MAP_LEN("PrintQuality"),          HANDLE_PRINTQUALITY,        &::getCppuType((const sal_Int32*)0),    0,  MID_PRINTER },
            { MAP_LEN("ColorTableURL"),         HANDLE_COLORTABLEURL,       &::getCppuType((const OUString*)0),     0,  0 },
            { MAP_LEN("DashTableURL"),          HANDLE_DASHTABLEURL,        &::getCppuType((const OUString*)0),     0,  0 },
            { MAP_LEN("LineEndTableURL"),       HANDLE_LINEENDTABLEURL,     &::getCppuType((const OUString*)0),     0,  0 },
            { MAP_LEN("HatchTableURL"),         HANDLE_HATCHTABLEURL,       &::getCppuType((const OUString*)0),     0,  0 },
            { MAP_LEN("GradientTableURL"),      HANDLE_GRADIENTTABLEURL,    &::getCppuType((const OUString*)0),     0,  0 },
            { MAP_LEN("BitmapTableURL"),        HANDLE_BITMAPTABLEURL,      &::getCppuType((const OUString*)0),     0,  0 },

            { MAP_LEN("ForbiddenCharacters"),   HANDLE_FORBIDDENCHARS,      &::getCppuType((const Reference< XForbiddenCharacters >*)0),    0, 0 },
            { MAP_LEN("ApplyUserData"),         HANDLE_APPLYUSERDATA,       &::getBooleanCppuType(),                0,  0 },

            { MAP_LEN("PageNumberFormat"),      HANDLE_PAGENUMFMT,          &::getCppuType((const sal_Int32*)0),    0,  0 },
            { MAP_LEN("ParagraphSummation"),    HANDLE_PARAGRAPHSUMMATION,  &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("CharacterCompressionType"),HANDLE_CHARCOMPRESS,      &::getCppuType((sal_Int16*)0),          0,  0 },
            { MAP_LEN("IsKernAsianPunctuation"),HANDLE_ASIANPUNCT,          &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("UpdateFromTemplate"),    HANDLE_UPDATEFROMTEMPLATE,  &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("PrinterIndependentLayout"),HANDLE_PRINTER_INDEPENDENT_LAYOUT,&::getCppuType((const sal_Int16*)0), 0,  0 },
            // --> #i33095#
            { MAP_LEN("LoadReadonly"),          HANDLE_LOAD_READONLY,       &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("SaveVersionOnClose"),    HANDLE_SAVE_VERSION,        &::getBooleanCppuType(),                0,  0 },
            { NULL, 0, 0, NULL, 0, 0 }
        };

        PropertySetInfo* pInfo = new PropertySetInfo( aCommonSettingsInfoMap );
        pInfo->add( bIsDraw ? aDrawSettingsInfoMap : aImpressSettingsInfoMap );

        return pInfo;
    }
}

using namespace ::sd;

DocumentSettings::DocumentSettings( SdXImpressDocument* pModel )
:   PropertySetHelper( createSettingsInfoImpl( !pModel->IsImpressDocument() ) ),
    mxModel( pModel ),
    mpModel( pModel )
{
}

DocumentSettings::~DocumentSettings() throw()
{
}

bool DocumentSettings::LoadList( XPropertyListType t, const rtl::OUString &rInPath,
                                 const uno::Reference< embed::XStorage > &xStorage )
{
    SdDrawDocument* pDoc = mpModel->GetDoc();

    sal_Int32 nSlash = rInPath.lastIndexOf('/');
    rtl::OUString aPath, aName;
    if (nSlash < -1)
        aName = rInPath;
    else {
        aName = rInPath.copy( nSlash + 1 );
        aPath = rInPath.copy( 0, nSlash );
    }

    XPropertyListRef pList = XPropertyList::CreatePropertyList(
        t, aPath, (XOutdevItemPool*)&pDoc->GetPool() );
    pList->SetName( aName );

    if( pList->LoadFrom( xStorage, rInPath ) )
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

    if( LoadList( t, aURL, uno::Reference< embed::XStorage >() ) )
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

static XPropertyListType getTypeOfName( const rtl::OUString &aName )
{
    for( size_t i = 0; i < SAL_N_ELEMENTS( aURLPropertyNames ); i++ ) {
        if( aName.equalsAscii( aURLPropertyNames[i].pName ) )
            return aURLPropertyNames[i].t;
    }
    return (XPropertyListType) -1;
}

static rtl::OUString getNameOfType( XPropertyListType t )
{
    for( size_t i = 0; i < SAL_N_ELEMENTS( aURLPropertyNames ); i++ ) {
        if( t == aURLPropertyNames[i].t )
            return rtl::OUString( aURLPropertyNames[i].pName,
                                  strlen( aURLPropertyNames[i].pName ) - 3,
                                  RTL_TEXTENCODING_UTF8 );
    }
    return rtl::OUString();
}

uno::Sequence<beans::PropertyValue>
        DocumentSettings::filterStreamsFromStorage(
                const uno::Reference< embed::XStorage > &xStorage,
                const uno::Sequence<beans::PropertyValue>& aConfigProps )
{
    uno::Sequence<beans::PropertyValue> aRet( aConfigProps.getLength() );
    int nRet = 0;
    for( sal_Int32 i = 0; i < aConfigProps.getLength(); i++ )
    {
        XPropertyListType t = getTypeOfName( aConfigProps[i].Name );
        if (t < 0)
            aRet[nRet++] = aConfigProps[i];
        else
        {
            rtl::OUString aURL;
            aConfigProps[i].Value >>= aURL;
            LoadList( t, aURL, xStorage );
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
    SdDrawDocument* pDoc = mpModel->GetDoc();
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
                    rtl::OUString aValue;
                    aRet[i].Value >>= aValue;

                    rtl::OUStringBuffer aName( getNameOfType( t ) );
                    rtl::OUString aResult;
                    if( pList->SaveTo( xSubStorage, aName.makeStringAndClear(), &aResult ) )
                    {
                        rtl::OUString aRealPath( "Settings/" );
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
//                 rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
    }

    return aRet;
}

void DocumentSettings::_setPropertyValues( const PropertyMapEntry** ppEntries, const Any* pValues ) throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException )
{
    ::SolarMutexGuard aGuard;

    SdDrawDocument* pDoc = mpModel->GetDoc();
    ::sd::DrawDocShell* pDocSh = mpModel->GetDocShell();
    if( NULL == pDoc || NULL == pDocSh )
        throw UnknownPropertyException();

    sal_Bool bValue = sal_False;
    bool bOk, bChanged = false, bOptionsChanged = false;

    SdOptionsPrintItem aOptionsPrintItem( ATTR_OPTIONS_PRINT );

    SfxPrinter* pPrinter = pDocSh->GetPrinter( sal_False );
    if( pPrinter )
    {
        SdOptionsPrintItem* pPrinterOptions = NULL;
        if(pPrinter->GetOptions().GetItemState( ATTR_OPTIONS_PRINT, sal_False, (const SfxPoolItem**) &pPrinterOptions) == SFX_ITEM_SET)
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
                    sal_Bool bApplyUserData = sal_False;
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
                        short nFieldUnit;
                        if( SvxMeasureUnitToFieldUnit( nValue, nFieldUnit ) )
                        {
                            pDoc->SetUIUnit((FieldUnit)nFieldUnit );
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
                        if( !aPrinterName.isEmpty() && pDocSh->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
                        {
                            SfxPrinter *pTempPrinter = pDocSh->GetPrinter( sal_True );
                            if (pTempPrinter)
                            {
                                SfxPrinter *pNewPrinter = new SfxPrinter( pTempPrinter->GetOptions().Clone(), aPrinterName );
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
                            SvMemoryStream aStream (aSequence.getArray(), nSize, STREAM_READ );
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
                sal_Bool bIsSummationOfParagraphs = sal_False;
                if ( *pValues >>= bIsSummationOfParagraphs )
                {
                    bOk = true;
                    bChanged = true;
                    if ( pDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS )
                    {
                        sal_uInt32 nSum = bIsSummationOfParagraphs ? EE_CNTRL_ULSPACESUMMATION : 0;
                        sal_uInt32 nCntrl;

                        pDoc->SetSummationOfParagraphs( bIsSummationOfParagraphs );
                        SdDrawDocument* pDocument = pDocSh->GetDoc();
                        SdrOutliner& rOutl = pDocument->GetDrawOutliner();
                        nCntrl = rOutl.GetControlWord() &~ EE_CNTRL_ULSPACESUMMATION;
                        rOutl.SetControlWord( nCntrl | nSum );
                        ::sd::Outliner* pOutl = pDocument->GetOutliner( sal_False );
                        if( pOutl )
                        {
                            nCntrl = pOutl->GetControlWord() &~ EE_CNTRL_ULSPACESUMMATION;
                            pOutl->SetControlWord( nCntrl | nSum );
                        }
                        pOutl = pDocument->GetInternalOutliner( sal_False );
                        if( pOutl )
                        {
                            nCntrl = pOutl->GetControlWord() &~ EE_CNTRL_ULSPACESUMMATION;
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
                    ::sd::Outliner* pOutl = pDocument->GetOutliner( sal_False );
                    if( pOutl )
                    {
                        pOutl->SetAsianCompressionMode( (sal_uInt16)nCharCompressType );
                    }
                    pOutl = pDocument->GetInternalOutliner( sal_False );
                    if( pOutl )
                    {
                        pOutl->SetAsianCompressionMode( (sal_uInt16)nCharCompressType );
                    }
                }
                break;

            }
            case HANDLE_ASIANPUNCT:
            {
                sal_Bool bAsianPunct = sal_False;
                if( *pValues >>= bAsianPunct )
                {
                    bOk = true;

                    pDoc->SetKernAsianPunctuation( bAsianPunct );
                    SdDrawDocument* pDocument = pDocSh->GetDoc();
                    SdrOutliner& rOutl = pDocument->GetDrawOutliner();
                    rOutl.SetKernAsianPunctuation( bAsianPunct );
                    ::sd::Outliner* pOutl = pDocument->GetOutliner( sal_False );
                    if( pOutl )
                    {
                        pOutl->SetKernAsianPunctuation( bAsianPunct );
                    }
                    pOutl = pDocument->GetInternalOutliner( sal_False );
                    if( pOutl )
                    {
                        pOutl->SetKernAsianPunctuation( bAsianPunct );
                    }
                }
                break;

            }
            case HANDLE_UPDATEFROMTEMPLATE:
            {
                sal_Bool value = sal_False;
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
                sal_Bool bNewValue = sal_False;
                if ( *pValues >>= bNewValue )
                {
                    bChanged = ( pDocSh->IsLoadReadonly() != bNewValue );
                    pDocSh->SetLoadReadonly( bNewValue );
                    bOk = true;
                }
            }
            break;

            case HANDLE_SAVE_VERSION:
            {
                sal_Bool bNewValue = sal_False;
                if ( *pValues >>= bNewValue )
                {
                    bChanged = ( pDocSh->IsSaveVersionOnClose() != bNewValue );
                    pDocSh->SetSaveVersionOnClose( bNewValue );
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
            pPrinter = pDocSh->GetPrinter( sal_True );
        SfxItemSet aNewOptions( pPrinter->GetOptions() );
        aNewOptions.Put( aOptionsPrintItem );
        pPrinter->SetOptions( aNewOptions );
    }

    if( bChanged || bOptionsChanged )
        mpModel->SetModified( sal_True );
}

void DocumentSettings::ExtractURL( XPropertyListType t, Any* pValue )
{
    XPropertyListRef pList = mpModel->GetDoc()->GetPropertyList( t );
    if( !pList.is() )
        return;

    INetURLObject aPathURL( pList->GetPath() );
    aPathURL.insertName( pList->GetName() );
    aPathURL.setExtension( pList->GetDefaultExt() );
    OUString aPath( aPathURL.GetMainURL( INetURLObject::NO_DECODE ) );
    *pValue <<= aPath;
}

void DocumentSettings::_getPropertyValues( const PropertyMapEntry** ppEntries, Any* pValue ) throw(UnknownPropertyException, WrappedTargetException )
{
    ::SolarMutexGuard aGuard;

    SdDrawDocument* pDoc = mpModel->GetDoc();
    ::sd::DrawDocShell* pDocSh = mpModel->GetDocShell();
    if( NULL == pDoc || NULL == pDocSh )
        throw UnknownPropertyException();

    SdOptionsPrintItem aOptionsPrintItem( ATTR_OPTIONS_PRINT );

    SfxPrinter* pPrinter = pDocSh->GetPrinter( sal_False );
    if( pPrinter )
    {
        SdOptionsPrintItem* pPrinterOptions = NULL;
        if(pPrinter->GetOptions().GetItemState( ATTR_OPTIONS_PRINT, sal_False, (const SfxPoolItem**) &pPrinterOptions) == SFX_ITEM_SET)
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
                *pValue <<= mpModel->getForbiddenCharsTable();
                break;
            case HANDLE_APPLYUSERDATA:
                *pValue <<= pDocSh->IsUseUserData();
                break;
            case HANDLE_PRINTDRAWING:
                *pValue <<= (sal_Bool)aPrintOpts.IsDraw();
                break;
            case HANDLE_PRINTNOTES:
                *pValue <<= (sal_Bool)aPrintOpts.IsNotes();
                break;
            case HANDLE_PRINTHANDOUT:
                *pValue <<= (sal_Bool)aPrintOpts.IsHandout();
                break;
            case HANDLE_PRINTOUTLINE:
                *pValue <<= (sal_Bool)aPrintOpts.IsOutline();
                break;
            case HANDLE_SLIDESPERHANDOUT:
                *pValue <<= (sal_Int16)aPrintOpts.GetHandoutPages();
                break;
            case HANDLE_HANDOUTHORIZONTAL:
                *pValue <<= (sal_Bool)aPrintOpts.IsHandoutHorizontal();
                break;
            case HANDLE_PRINTPAGENAME:
                *pValue <<= (sal_Bool)aPrintOpts.IsPagename();
                break;
            case HANDLE_PRINTDATE:
                *pValue <<= (sal_Bool)aPrintOpts.IsDate();
                break;
            case HANDLE_PRINTTIME:
                *pValue <<= (sal_Bool)aPrintOpts.IsTime();
                break;
            case HANDLE_PRINTHIDENPAGES:
                *pValue <<= (sal_Bool)aPrintOpts.IsHiddenPages();
                break;
            case HANDLE_PRINTFITPAGE:
                *pValue <<= (sal_Bool)aPrintOpts.IsPagesize();
                break;
            case HANDLE_PRINTTILEPAGE:
                *pValue <<= (sal_Bool)aPrintOpts.IsPagetile();
                break;
            case HANDLE_PRINTBOOKLET:
                *pValue <<= (sal_Bool)aPrintOpts.IsBooklet();
                break;
            case HANDLE_PRINTBOOKLETFRONT:
                *pValue <<= (sal_Bool)aPrintOpts.IsFrontPage();
                break;
            case HANDLE_PRINTBOOKLETBACK:
                *pValue <<= (sal_Bool)aPrintOpts.IsBackPage();
                break;
            case HANDLE_PRINTQUALITY:
                *pValue <<= (sal_Int32)aPrintOpts.GetOutputQuality();
                break;
            case HANDLE_MEASUREUNIT:
                {
                    short nMeasure;
                    SvxFieldUnitToMeasureUnit( (const short)pDoc->GetUIUnit(), nMeasure );
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
                    SfxPrinter *pTempPrinter = pDocSh->GetPrinter( sal_False );
                    *pValue <<= pTempPrinter ? OUString ( pTempPrinter->GetName()) : OUString();
                }
                break;
            case HANDLE_PRINTERJOB:
                {
                    SfxPrinter *pTempPrinter = pDocSh->GetPrinter( sal_False );
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
                sal_Bool bIsSummationOfParagraphs = pDoc->IsSummationOfParagraphs();
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
                *pValue <<= (sal_Bool)pDoc->IsKernAsianPunctuation();
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

            case HANDLE_SAVE_VERSION:
            {
                *pValue <<= pDocSh->IsSaveVersionOnClose();
            }
            break;

            default:
                throw UnknownPropertyException();
        }
    }
}

// XInterface
Any SAL_CALL DocumentSettings::queryInterface( const Type& aType ) throw (RuntimeException)
{
    return WeakImplHelper3< XPropertySet, XMultiPropertySet, XServiceInfo >::queryInterface( aType );
}

void SAL_CALL DocumentSettings::acquire(  ) throw ()
{
    WeakImplHelper3< XPropertySet, XMultiPropertySet, XServiceInfo >::acquire();
}

void SAL_CALL DocumentSettings::release(  ) throw ()
{
    WeakImplHelper3< XPropertySet, XMultiPropertySet, XServiceInfo >::release();
}


// XPropertySet
Reference< XPropertySetInfo > SAL_CALL DocumentSettings::getPropertySetInfo(  ) throw(RuntimeException)
{
    return PropertySetHelper::getPropertySetInfo();
}

void SAL_CALL DocumentSettings::setPropertyValue( const OUString& aPropertyName, const Any& aValue ) throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    PropertySetHelper::setPropertyValue( aPropertyName, aValue );
}

Any SAL_CALL DocumentSettings::getPropertyValue( const OUString& PropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    return PropertySetHelper::getPropertyValue( PropertyName );
}

void SAL_CALL DocumentSettings::addPropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    PropertySetHelper::addPropertyChangeListener( aPropertyName, xListener );
}

void SAL_CALL DocumentSettings::removePropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    PropertySetHelper::removePropertyChangeListener( aPropertyName, aListener );
}

void SAL_CALL DocumentSettings::addVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    PropertySetHelper::addVetoableChangeListener( PropertyName, aListener );
}

void SAL_CALL DocumentSettings::removeVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    PropertySetHelper::removeVetoableChangeListener( PropertyName, aListener );
}

// XMultiPropertySet
void SAL_CALL DocumentSettings::setPropertyValues( const Sequence< OUString >& aPropertyNames, const Sequence< Any >& aValues ) throw(PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    PropertySetHelper::setPropertyValues( aPropertyNames, aValues );
}

Sequence< Any > SAL_CALL DocumentSettings::getPropertyValues( const Sequence< OUString >& aPropertyNames ) throw(RuntimeException)
{
    return PropertySetHelper::getPropertyValues( aPropertyNames );
}

void SAL_CALL DocumentSettings::addPropertiesChangeListener( const Sequence< OUString >& aPropertyNames, const Reference< XPropertiesChangeListener >& xListener ) throw(RuntimeException)
{
    PropertySetHelper::addPropertiesChangeListener( aPropertyNames, xListener );
}

void SAL_CALL DocumentSettings::removePropertiesChangeListener( const Reference< XPropertiesChangeListener >& xListener ) throw(RuntimeException)
{
    PropertySetHelper::removePropertiesChangeListener( xListener );
}

void SAL_CALL DocumentSettings::firePropertiesChangeEvent( const Sequence< OUString >& aPropertyNames, const Reference< XPropertiesChangeListener >& xListener ) throw(RuntimeException)
{
    PropertySetHelper::firePropertiesChangeEvent( aPropertyNames, xListener );
}

// XServiceInfo
OUString SAL_CALL DocumentSettings::getImplementationName(  )
    throw(RuntimeException)
{
    return OUString( "com.sun.star.comp.Draw.DocumentSettings" );
}

sal_Bool SAL_CALL DocumentSettings::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    const Sequence< OUString > aSeq( getSupportedServiceNames() );
    sal_Int32 nCount = aSeq.getLength();
    const OUString* pServices = aSeq.getConstArray();
    while( nCount-- )
    {
        if( *pServices++ == ServiceName )
            return sal_True;
    }

    return sal_True;
}

Sequence< OUString > SAL_CALL DocumentSettings::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    Sequence< OUString > aSeq( 2 );
    aSeq[0] = "com.sun.star.document.Settings" ;
    if( mpModel->IsImpressDocument() )
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
