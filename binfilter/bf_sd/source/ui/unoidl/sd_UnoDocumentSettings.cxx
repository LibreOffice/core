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
#include <comphelper/propertysethelper.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <bf_sfx2/docinf.hxx>
#include <tools/urlobj.hxx>
#include <bf_svx/xtable.hxx>
#include <osl/diagnose.h>

#include "drawdoc.hxx"
#include "bf_sd/docshell.hxx"
#include "unomodel.hxx"

#include "optsitem.hxx"
#include <bf_sfx2/printer.hxx>
#include "sdattr.hxx"
#include "bf_sd/frmview.hxx"
#include <sdoutl.hxx>
#include <bf_svx/editstat.hxx>
#include <bf_svx/unoapi.hxx>

namespace binfilter {

#define MAP_LEN(x) x, sizeof(x)-1

using namespace ::comphelper;
using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::vos;
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
                             public comphelper::PropertySetHelper
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
    //  virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);

    protected:
        virtual void _setPropertyValues( const comphelper::PropertyMapEntry** ppEntries, const ::com::sun::star::uno::Any* pValues ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
        virtual void _getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, ::com::sun::star::uno::Any* pValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException );

    private:
        Reference< XModel >		mxModel;
        SdXImpressDocument*		mpModel;
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
};

#define MID_PRINTER 1

    PropertySetInfo* createSettingsInfoImpl( sal_Bool bIsDraw )
    {
        static PropertyMapEntry aImpressSettingsInfoMap[] =
        {
            { MAP_LEN("IsPrintDrawing"),		HANDLE_PRINTDRAWING,		&::getBooleanCppuType(),				0,	MID_PRINTER },
            { MAP_LEN("IsPrintNotes"),			HANDLE_PRINTNOTES,			&::getBooleanCppuType(),				0,	MID_PRINTER },
            { MAP_LEN("IsPrintHandout"),		HANDLE_PRINTHANDOUT,		&::getBooleanCppuType(),				0,	MID_PRINTER },
            { MAP_LEN("IsPrintOutline"),		HANDLE_PRINTOUTLINE,		&::getBooleanCppuType(),				0,	MID_PRINTER },
            { NULL, 0, 0, NULL, 0, 0 }
        };

        static PropertyMapEntry aDrawSettingsInfoMap[] =
        {
            { MAP_LEN("MeasureUnit"),			HANDLE_MEASUREUNIT,			&::getCppuType((const sal_Int16*)0),	0,	0 },
            { MAP_LEN("ScaleNumerator"),		HANDLE_SCALE_NUM,			&::getCppuType((const sal_Int32*)0),	0,	0 },
            { MAP_LEN("ScaleDenominator"),		HANDLE_SCALE_DOM,			&::getCppuType((const sal_Int32*)0),	0,	0 },
            { NULL, 0, 0, NULL, 0, 0 }
        };

        static PropertyMapEntry aCommonSettingsInfoMap[] =
        {
            { MAP_LEN("DefaultTabStop"),		HANDLE_TABSTOP,				&::getCppuType((const sal_Int32*)0),	0,	0 },
            { MAP_LEN("PrinterName"),			HANDLE_PRINTERNAME,			&::getCppuType((const OUString*)0),		0,  0 },
            { MAP_LEN("PrinterSetup"),			HANDLE_PRINTERJOB,			&::getCppuType((const uno::Sequence < sal_Int8 > *)0),	0, MID_PRINTER },

            { MAP_LEN("IsPrintPageName"),		HANDLE_PRINTPAGENAME,		&::getBooleanCppuType(),				0,	MID_PRINTER },
            { MAP_LEN("IsPrintDate"),			HANDLE_PRINTDATE,			&::getBooleanCppuType(),				0,	MID_PRINTER },
            { MAP_LEN("IsPrintTime"),			HANDLE_PRINTTIME,			&::getBooleanCppuType(),				0,	MID_PRINTER },
            { MAP_LEN("IsPrintHiddenPages"),	HANDLE_PRINTHIDENPAGES,		&::getBooleanCppuType(),				0,	MID_PRINTER },
            { MAP_LEN("IsPrintFitPage"),		HANDLE_PRINTFITPAGE,		&::getBooleanCppuType(),				0,	MID_PRINTER },
            { MAP_LEN("IsPrintTilePage"),		HANDLE_PRINTTILEPAGE,		&::getBooleanCppuType(),				0,	MID_PRINTER },
            { MAP_LEN("IsPrintBooklet"),		HANDLE_PRINTBOOKLET,		&::getBooleanCppuType(),				0,	MID_PRINTER },
            { MAP_LEN("IsPrintBookletFront"),	HANDLE_PRINTBOOKLETFRONT,	&::getBooleanCppuType(),				0,	MID_PRINTER },
            { MAP_LEN("IsPrintBookletBack"),	HANDLE_PRINTBOOKLETBACK,	&::getBooleanCppuType(),				0,	MID_PRINTER },
            { MAP_LEN("PrintQuality"),			HANDLE_PRINTQUALITY,		&::getCppuType((const sal_Int32*)0),	0,	MID_PRINTER },
            { MAP_LEN("ColorTableURL"),			HANDLE_COLORTABLEURL,		&::getCppuType((const OUString*)0),		0,	0 },
            { MAP_LEN("DashTableURL"),			HANDLE_DASHTABLEURL,		&::getCppuType((const OUString*)0),		0,	0 },
            { MAP_LEN("LineEndTableURL"),		HANDLE_LINEENDTABLEURL,		&::getCppuType((const OUString*)0),		0,	0 },
            { MAP_LEN("HatchTableURL"),			HANDLE_HATCHTABLEURL,		&::getCppuType((const OUString*)0),		0,	0 },
            { MAP_LEN("GradientTableURL"),		HANDLE_GRADIENTTABLEURL,	&::getCppuType((const OUString*)0),		0,	0 },
            { MAP_LEN("BitmapTableURL"),		HANDLE_BITMAPTABLEURL,		&::getCppuType((const OUString*)0),		0,	0 },

            { MAP_LEN("ForbiddenCharacters"),	HANDLE_FORBIDDENCHARS,		&::getCppuType((const Reference< XForbiddenCharacters >*)0),	0, 0 },
            { MAP_LEN("ApplyUserData"),			HANDLE_APPLYUSERDATA,		&::getBooleanCppuType(),				0,	0 },

            { MAP_LEN("PageNumberFormat"),		HANDLE_PAGENUMFMT,			&::getCppuType((const sal_Int32*)0),	0,  0 },
            { MAP_LEN("ParagraphSummation"),	HANDLE_PARAGRAPHSUMMATION,	&::getBooleanCppuType(),	            0,  0 },
            { MAP_LEN("CharacterCompressionType"),HANDLE_CHARCOMPRESS,		&::getCppuType((sal_Int16*)0),	        0,  0 },
            { MAP_LEN("IsKernAsianPunctuation"),HANDLE_ASIANPUNCT,			&::getBooleanCppuType(),	            0,  0 },
            { MAP_LEN("UpdateFromTemplate"),	HANDLE_UPDATEFROMTEMPLATE,	&::getBooleanCppuType(),	            0,  0 },
            { MAP_LEN("PrinterIndependentLayout"),HANDLE_PRINTER_INDEPENDENT_LAYOUT,&::getCppuType((const sal_Int16*)0), 0,  0 },
            { NULL, 0, 0, NULL, 0, 0 }
        };

        PropertySetInfo* pInfo = new PropertySetInfo( aCommonSettingsInfoMap );
        pInfo->add( bIsDraw ? aDrawSettingsInfoMap : aImpressSettingsInfoMap );

        return pInfo;
    }
}

using namespace ::binfilter::sd;

DocumentSettings::DocumentSettings( SdXImpressDocument* pModel )
:	PropertySetHelper( createSettingsInfoImpl( !pModel->IsImpressDocument() ) ),
    mxModel( pModel ),
    mpModel( pModel )
{
}

DocumentSettings::~DocumentSettings() throw()
{
}

void DocumentSettings::_setPropertyValues( const PropertyMapEntry** ppEntries, const Any* pValues ) throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    SdDrawDocument* pDoc = mpModel->GetDoc();
    SdDrawDocShell* pDocSh = mpModel->GetDocShell();
    if( NULL == pDoc || NULL == pDocSh )
        throw UnknownPropertyException();

    sal_Bool bOk, bChanged = sal_False, bValue;

    SfxPrinter* pPrinter = NULL;
    SdOptionsPrintItem* pPrinterOptions = NULL;

    for( ; *ppEntries; ppEntries++, pValues++ )
    {
        bOk = sal_False;
        if( ((*ppEntries)->mnMemberId == MID_PRINTER) && (pPrinter == NULL) )
        {
            pPrinter = pDocSh->GetPrinter( sal_True );
            if(pPrinter->GetOptions().GetItemState( ATTR_OPTIONS_PRINT, FALSE, (const SfxPoolItem**) &pPrinterOptions) != SFX_ITEM_SET)
            {
                DBG_ERROR( "could not get printer options!" );
                pPrinter = NULL;
                pPrinterOptions = NULL;
                continue;
            }
        }

        switch( (*ppEntries)->mnHandle )
        {
            case HANDLE_COLORTABLEURL:
                {
                    OUString aURLString;
                    if( *pValues >>= aURLString )
                    {
                        INetURLObject aURL( aURLString );
                        INetURLObject aPathURL( aURL ); 
                    
                        aPathURL.removeSegment(); 
                        aPathURL.removeFinalSlash();

                        XColorTable* pColTab = new XColorTable( aPathURL.GetMainURL( INetURLObject::NO_DECODE ), (XOutdevItemPool*)&pDoc->GetPool() );
                        pColTab->SetName( aURL.getName() );
                        if( pColTab->Load() )
                        {
                            pDoc->SetColorTable( pColTab );
                            bOk = sal_True;
                            bChanged = sal_True;
                        }
                    }
                }
                break;
            case HANDLE_DASHTABLEURL:
                {
                    OUString aURLString;
                    if( *pValues >>= aURLString )
                    {
                        INetURLObject aURL( aURLString );
                        INetURLObject aPathURL( aURL ); 
                    
                        aPathURL.removeSegment(); 
                        aPathURL.removeFinalSlash();

                        XDashList* pDashTab = new XDashList( aPathURL.GetMainURL( INetURLObject::NO_DECODE ), (XOutdevItemPool*)&pDoc->GetPool() );
                        pDashTab->SetName( aURL.getName() );
                        if( pDashTab->Load() )
                        {
                            pDoc->SetDashList( pDashTab );
                            bOk = sal_True;
                            bChanged = sal_True;
                        }
                    }
                }
                break;
            case HANDLE_LINEENDTABLEURL:
                {
                    OUString aURLString;
                    if( *pValues >>= aURLString )
                    {
                        INetURLObject aURL( aURLString );
                        INetURLObject aPathURL( aURL ); 
                    
                        aPathURL.removeSegment(); 
                        aPathURL.removeFinalSlash();

                        XLineEndList* pTab = new XLineEndList( aPathURL.GetMainURL( INetURLObject::NO_DECODE ), (XOutdevItemPool*)&pDoc->GetPool() );
                        pTab->SetName( aURL.getName() );
                        if( pTab->Load() )
                        {
                            pDoc->SetLineEndList( pTab );
                            bOk = sal_True;
                            bChanged = sal_True;
                        }
                    }
                }
                break;
            case HANDLE_HATCHTABLEURL:
                {
                    OUString aURLString;
                    if( *pValues >>= aURLString )
                    {
                        INetURLObject aURL( aURLString );
                        INetURLObject aPathURL( aURL ); 
                    
                        aPathURL.removeSegment(); 
                        aPathURL.removeFinalSlash();

                        XHatchList* pTab = new XHatchList( aPathURL.GetMainURL( INetURLObject::NO_DECODE ), (XOutdevItemPool*)&pDoc->GetPool() );
                        pTab->SetName( aURL.getName() );
                        if( pTab->Load() )
                        {
                            pDoc->SetHatchList( pTab );
                            bOk = sal_True;
                            bChanged = sal_True;
                        }
                    }
                }
                break;
            case HANDLE_GRADIENTTABLEURL:
                {
                    OUString aURLString;
                    if( *pValues >>= aURLString )
                    {
                        INetURLObject aURL( aURLString );
                        INetURLObject aPathURL( aURL ); 
                    
                        aPathURL.removeSegment(); 
                        aPathURL.removeFinalSlash();

                        XGradientList* pTab = new XGradientList( aPathURL.GetMainURL( INetURLObject::NO_DECODE ), (XOutdevItemPool*)&pDoc->GetPool() );
                        pTab->SetName( aURL.getName() );
                        if( pTab->Load() )
                        {
                            pDoc->SetGradientList( pTab );
                            bOk = sal_True;
                            bChanged = sal_True;
                        }
                    }
                }
                break;
            case HANDLE_BITMAPTABLEURL:
                {
                    OUString aURLString;
                    if( *pValues >>= aURLString )
                    {
                        INetURLObject aURL( aURLString );
                        INetURLObject aPathURL( aURL ); 
                    
                        aPathURL.removeSegment(); 
                        aPathURL.removeFinalSlash();

                        XBitmapList* pTab = new XBitmapList( aPathURL.GetMainURL( INetURLObject::NO_DECODE ), (XOutdevItemPool*)&pDoc->GetPool() );
                        pTab->SetName( aURL.getName() );
                        if( pTab->Load() )
                        {
                            pDoc->SetBitmapList( pTab );
                            bOk = sal_True;
                            bChanged = sal_True;
                        }
                    }
                }
                break;
            case HANDLE_FORBIDDENCHARS:
                {
                    bOk = sal_True;
                }
                break;
            case HANDLE_APPLYUSERDATA:
                {
                    sal_Bool bApplyUserData;
                    if( *pValues >>= bApplyUserData )
                    {
                        SfxDocumentInfo& rInfo = pDocSh->GetDocInfo();
                        if( rInfo.IsUseUserData() != bApplyUserData )
                        {
                            rInfo.SetUseUserData( bApplyUserData );
                            bChanged = sal_True;
                        }
                        bOk = sal_True;
                    }
                }
                break;
            case HANDLE_PRINTDRAWING:
                if( *pValues >>= bValue )
                {
                    pPrinterOptions->SetDraw( bValue );
                    bOk = sal_True;
                }
                break;
            case HANDLE_PRINTNOTES:
                if( *pValues >>= bValue )
                {
                    pPrinterOptions->SetNotes( bValue );
                    bOk = sal_True;
                }
                break;
            case HANDLE_PRINTHANDOUT:
                if( *pValues >>= bValue )
                {
                    pPrinterOptions->SetHandout( bValue );
                    bOk = sal_True;
                }
                break;
            case HANDLE_PRINTOUTLINE:
                if( *pValues >>= bValue )
                {
                    pPrinterOptions->SetOutline( bValue );
                    bOk = sal_True;
                }
                break;
            case HANDLE_PRINTPAGENAME:
                if( *pValues >>= bValue )
                {
                    pPrinterOptions->SetPagename( bValue );
                    bOk = sal_True;
                }
                break;
            case HANDLE_PRINTDATE:
                if( *pValues >>= bValue )
                {
                    pPrinterOptions->SetDate( bValue );
                    bOk = sal_True;
                }
                break;
            case HANDLE_PRINTTIME:
                if( *pValues >>= bValue )
                {
                    pPrinterOptions->SetTime( bValue );
                    bOk = sal_True;
                }
                break;
            case HANDLE_PRINTHIDENPAGES:
                if( *pValues >>= bValue )
                {
                    pPrinterOptions->SetHiddenPages( bValue );
                    bOk = sal_True;
                }
                break;
            case HANDLE_PRINTFITPAGE:
                if( *pValues >>= bValue )
                {
                    pPrinterOptions->SetPagesize( bValue );
                    bOk = sal_True;
                }
                break;
            case HANDLE_PRINTTILEPAGE:
                if( *pValues >>= bValue )
                {
                    pPrinterOptions->SetPagetile( bValue );
                    bOk = sal_True;
                }
                break;
            case HANDLE_PRINTBOOKLET:
                if( *pValues >>= bValue )
                {
                    pPrinterOptions->SetBooklet( bValue );
                    bOk = sal_True;
                }
                break;
            case HANDLE_PRINTBOOKLETFRONT:
                if( *pValues >>= bValue )
                {
                    pPrinterOptions->SetFrontPage( bValue );
                    bOk = sal_True;
                }
                break;
            case HANDLE_PRINTBOOKLETBACK:
                if( *pValues >>= bValue )
                {
                    pPrinterOptions->SetBackPage( bValue );
                    bOk = sal_True;
                }
                break;
            case HANDLE_PRINTQUALITY:
                {
                    sal_Int32 nValue;
                    if( *pValues >>= nValue )
                    {
                        pPrinterOptions->SetOutputQuality( (sal_uInt16)nValue );
                        bOk = sal_True;
                    }
                }
                break;
            case HANDLE_MEASUREUNIT:
                {
                    sal_Int16 nValue;
                    if( *pValues >>= nValue )
                    {
                        short nFieldUnit;
                        if( SvxMeasureUnitToFieldUnit( nValue, nFieldUnit ) )
                        {
                            pDoc->SetUIUnit((FieldUnit)nFieldUnit );
                            bOk = sal_True;
                        }
                    }
                }
                break;
            case HANDLE_SCALE_NUM:
                {
                    sal_Int32 nValue;
                    if( *pValues >>= nValue )
                    {
                        Fraction aFract( nValue, pDoc->GetUIScale().GetDenominator() );
                        pDoc->SetUIScale( aFract );
                        bOk = sal_True;
                        bChanged = sal_True;
                    }
                }
                break;
            case HANDLE_SCALE_DOM:
                {
                    sal_Int32 nValue;
                    if( *pValues >>= nValue )
                    {
                        Fraction aFract( pDoc->GetUIScale().GetNumerator(), nValue );
                        pDoc->SetUIScale( aFract );
                        bOk = sal_True;
                        bChanged = sal_True;
                    }
                }
                break;

            case HANDLE_TABSTOP:
                {
                    sal_Int32 nValue;
                    if( (*pValues >>= nValue) && (nValue >= 0) )
                    {
                        pDoc->SetDefaultTabulator((sal_uInt16)nValue);
                        bOk = sal_True;
                        bChanged = sal_True;
                    }
                }
                break;
            case HANDLE_PAGENUMFMT:
                {
                    sal_Int32 nValue;
                    if( (*pValues >>= nValue ) && (nValue >= SVX_CHARS_UPPER_LETTER ) && (nValue <= SVX_PAGEDESC) )
                    {
                        pDoc->SetPageNumType((SvxNumType)nValue);
                        bOk = sal_True;
                        bChanged = sal_True;
                    }
                }
                break;
            case HANDLE_PRINTERNAME:
                {
                    OUString aPrinterName;
                    if( *pValues >>= aPrinterName )
                    {
                        bOk = sal_True;
                        SfxPrinter *pPrinter = pDocSh->GetPrinter( sal_True );
                        if (pPrinter)
                        {
                            SfxPrinter *pNewPrinter = new SfxPrinter ( pPrinter->GetOptions().Clone(), aPrinterName );
                            pDocSh->SetPrinter( pNewPrinter );
                        }
                    }
                }
                break;
            case HANDLE_PRINTERJOB:
                {
                    Sequence < sal_Int8 > aSequence;
                    if ( *pValues >>= aSequence )
                    {
                        bOk = sal_True;
                        sal_uInt32 nSize = aSequence.getLength();
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
                                        SID_PRINTER_NOTFOUND_WARN,	SID_PRINTER_NOTFOUND_WARN,
                                        SID_PRINTER_CHANGESTODOC,	SID_PRINTER_CHANGESTODOC,
                                        ATTR_OPTIONS_PRINT, 		ATTR_OPTIONS_PRINT,
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
                break;
                
            case HANDLE_PARAGRAPHSUMMATION :
            {
                sal_Bool bIsSummationOfParagraphs;
                if ( *pValues >>= bIsSummationOfParagraphs )
                {
                    bOk = sal_True;
                    bChanged = sal_True;
                    if ( pDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS )
                    {
                        sal_uInt32 nSum = bIsSummationOfParagraphs ? EE_CNTRL_ULSPACESUMMATION : 0;
                        sal_uInt32 nCntrl;

                        pDoc->SetSummationOfParagraphs( bIsSummationOfParagraphs );
                        SdDrawDocument* pDocument = pDocSh->GetDoc();
                        SdrOutliner& rOutl = pDocument->GetDrawOutliner( FALSE );
                        nCntrl = rOutl.GetControlWord() &~ EE_CNTRL_ULSPACESUMMATION;
                        rOutl.SetControlWord( nCntrl | nSum );
                        SdOutliner* pOutl = pDocument->GetOutliner( FALSE );
                        if( pOutl )
                        {
                            nCntrl = pOutl->GetControlWord() &~ EE_CNTRL_ULSPACESUMMATION;
                            pOutl->SetControlWord( nCntrl | nSum );
                        }
                        pOutl = pDocument->GetInternalOutliner( FALSE );
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
                sal_Int16 nCharCompressType;
                if( *pValues >>= nCharCompressType )
                {
                    bOk = sal_True;

                    pDoc->SetCharCompressType( (UINT16)nCharCompressType );
                    SdDrawDocument* pDocument = pDocSh->GetDoc();
                    SdrOutliner& rOutl = pDocument->GetDrawOutliner( FALSE );
                    rOutl.SetAsianCompressionMode( (UINT16)nCharCompressType );
                    SdOutliner* pOutl = pDocument->GetOutliner( FALSE );
                    if( pOutl )
                    {
                        pOutl->SetAsianCompressionMode( (UINT16)nCharCompressType );
                    }
                    pOutl = pDocument->GetInternalOutliner( FALSE );
                    if( pOutl )
                    {
                        pOutl->SetAsianCompressionMode( (UINT16)nCharCompressType );
                    }
                }
                break;

            }
            case HANDLE_ASIANPUNCT:
            {
                sal_Bool bAsianPunct;
                if( *pValues >>= bAsianPunct )
                {
                    bOk = sal_True;

                    pDoc->SetKernAsianPunctuation( bAsianPunct );
                    SdDrawDocument* pDocument = pDocSh->GetDoc();
                    SdrOutliner& rOutl = pDocument->GetDrawOutliner( FALSE );
                    rOutl.SetKernAsianPunctuation( bAsianPunct );
                    SdOutliner* pOutl = pDocument->GetOutliner( FALSE );
                    if( pOutl )
                    {
                        pOutl->SetKernAsianPunctuation( bAsianPunct );
                    }
                    pOutl = pDocument->GetInternalOutliner( FALSE );
                    if( pOutl )
                    {
                        pOutl->SetKernAsianPunctuation( bAsianPunct );
                    }
                }
                break;

            }
            case HANDLE_UPDATEFROMTEMPLATE:
            {
                sal_Bool bValue;
                if( *pValues >>= bValue )
                {
                    SfxDocumentInfo& rInfo = pDocSh->GetDocInfo();
                    rInfo.SetQueryLoadTemplate( bValue );
                    bOk = sal_True;
                }
            }
            break;

            case HANDLE_PRINTER_INDEPENDENT_LAYOUT:
            {
                // Just propagate the new printer independent layout mode to
                // the document and determine it really differs from the old
                // one.
                sal_Int16 nOldValue = pDoc->GetPrinterIndependentLayout ();
                sal_Int16 nValue;
                if (*pValues >>= nValue)
                {
                    pDoc->SetPrinterIndependentLayout (nValue);
                    bChanged = (nValue != nOldValue);
                    bOk = sal_True;
                }
            }
            break;

            default:
                throw UnknownPropertyException();

        }

        if( !bOk )
            throw IllegalArgumentException();
    }

    if( pPrinter && pPrinterOptions )
    {
        SfxItemSet aNewOptions( pPrinter->GetOptions() );
        SdOptionsPrintItem aOpts( *pPrinterOptions );
        aNewOptions.Put( aOpts );
        pPrinter->SetOptions( aNewOptions );

        SdOptions* pOptions = SD_MOD()->GetSdOptions( pDoc->GetDocumentType() );
        pOptions->SetPrinterOptions( &aOpts );
    }

    if( bChanged )
        mpModel->SetModified( sal_True );
}

void DocumentSettings::_getPropertyValues( const PropertyMapEntry** ppEntries, Any* pValue ) throw(UnknownPropertyException, WrappedTargetException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    SdDrawDocument* pDoc = mpModel->GetDoc();
    SdDrawDocShell* pDocSh = mpModel->GetDocShell();
    if( NULL == pDoc || NULL == pDocSh )
        throw UnknownPropertyException();

    SfxPrinter* pPrinter = NULL;
    SdOptionsPrintItem* pPrinterOptions = NULL;

    for( ; *ppEntries; ppEntries++, pValue++ )
    {
        if( (*ppEntries)->mnMemberId == MID_PRINTER && pPrinter == NULL )
        {
            pPrinter = pDocSh->GetPrinter( sal_True );
            if(pPrinter->GetOptions().GetItemState( ATTR_OPTIONS_PRINT, FALSE, (const SfxPoolItem**) &pPrinterOptions) != SFX_ITEM_SET)
            {
                DBG_ERROR( "could not get printer options!" );
                pPrinter = NULL;
                pPrinterOptions = NULL;
                continue;
            }
        }

        switch( (*ppEntries)->mnHandle )
        {
            case HANDLE_COLORTABLEURL:
                {
                    INetURLObject aPathURL( pDoc->GetColorTable()->GetPath() );
                    aPathURL.insertName( pDoc->GetColorTable()->GetName() );
                    String aExt( RTL_CONSTASCII_USTRINGPARAM("soc") );
                    aPathURL.setExtension( aExt );
                    OUString aPath( aPathURL.GetMainURL( INetURLObject::NO_DECODE ) );
                    *pValue <<= aPath;
                }
                break;
            case HANDLE_DASHTABLEURL:
                {
                    INetURLObject aPathURL( pDoc->GetDashList()->GetPath() );
                    aPathURL.insertName( pDoc->GetDashList()->GetName() );
                    String aExt( RTL_CONSTASCII_USTRINGPARAM("sod") );
                    aPathURL.setExtension( aExt );
                    OUString aPath( aPathURL.GetMainURL( INetURLObject::NO_DECODE ) );
                    *pValue <<= aPath;
                }
                break;
            case HANDLE_LINEENDTABLEURL:
                {
                    INetURLObject aPathURL( pDoc->GetLineEndList()->GetPath() );
                    aPathURL.insertName( pDoc->GetLineEndList()->GetName() );
                    String aExt( RTL_CONSTASCII_USTRINGPARAM("soe") );
                    aPathURL.setExtension( aExt );
                    OUString aPath( aPathURL.GetMainURL( INetURLObject::NO_DECODE ) );
                    *pValue <<= aPath;
                }
                break;
            case HANDLE_HATCHTABLEURL:
                {
                    INetURLObject aPathURL( pDoc->GetHatchList()->GetPath() );
                    aPathURL.insertName( pDoc->GetHatchList()->GetName() );
                    String aExt( RTL_CONSTASCII_USTRINGPARAM("soh") );
                    aPathURL.setExtension( aExt );
                    OUString aPath( aPathURL.GetMainURL( INetURLObject::NO_DECODE ) );
                    *pValue <<= aPath;
                }
                break;
            case HANDLE_GRADIENTTABLEURL:
                {
                    INetURLObject aPathURL( pDoc->GetGradientList()->GetPath() );
                    aPathURL.insertName( pDoc->GetGradientList()->GetName() );
                    String aExt( RTL_CONSTASCII_USTRINGPARAM("sog") );
                    aPathURL.setExtension( aExt );
                    OUString aPath( aPathURL.GetMainURL( INetURLObject::NO_DECODE ) );
                    *pValue <<= aPath;
                }
                break;
            case HANDLE_BITMAPTABLEURL:
                {
                    INetURLObject aPathURL( pDoc->GetBitmapList()->GetPath() );
                    aPathURL.insertName( pDoc->GetBitmapList()->GetName() );
                    String aExt( RTL_CONSTASCII_USTRINGPARAM("sob") );
                    aPathURL.setExtension( aExt );
                    OUString aPath( aPathURL.GetMainURL( INetURLObject::NO_DECODE ) );
                    *pValue <<= aPath;
                }
                break;
            case HANDLE_FORBIDDENCHARS:
                *pValue <<= mpModel->getForbiddenCharsTable();
                break;

            case HANDLE_APPLYUSERDATA:
                *pValue <<= (sal_Bool)pDocSh->GetDocInfo().IsUseUserData();
                break;

            case HANDLE_PRINTDRAWING:
                *pValue <<= (sal_Bool)pPrinterOptions->IsDraw();
                break;
            case HANDLE_PRINTNOTES:
                *pValue <<= (sal_Bool)pPrinterOptions->IsNotes();
                break;
            case HANDLE_PRINTHANDOUT:
                *pValue <<= (sal_Bool)pPrinterOptions->IsHandout();
                break;
            case HANDLE_PRINTOUTLINE:
                *pValue <<= (sal_Bool)pPrinterOptions->IsOutline();
                break;
            case HANDLE_PRINTPAGENAME:
                *pValue <<= (sal_Bool)pPrinterOptions->IsPagename();
                break;
            case HANDLE_PRINTDATE:
                *pValue <<= (sal_Bool)pPrinterOptions->IsDate();
                break;
            case HANDLE_PRINTTIME:
                *pValue <<= (sal_Bool)pPrinterOptions->IsTime();
                break;
            case HANDLE_PRINTHIDENPAGES:
                *pValue <<= (sal_Bool)pPrinterOptions->IsHiddenPages();
                break;
            case HANDLE_PRINTFITPAGE:
                *pValue <<= (sal_Bool)pPrinterOptions->IsPagesize();
                break;
            case HANDLE_PRINTTILEPAGE:
                *pValue <<= (sal_Bool)pPrinterOptions->IsPagetile();
                break;
            case HANDLE_PRINTBOOKLET:
                *pValue <<= (sal_Bool)pPrinterOptions->IsBooklet();
                break;
            case HANDLE_PRINTBOOKLETFRONT:
                *pValue <<= (sal_Bool)pPrinterOptions->IsFrontPage();
                break;
            case HANDLE_PRINTBOOKLETBACK:
                *pValue <<= (sal_Bool)pPrinterOptions->IsBackPage();
                break;
            case HANDLE_PRINTQUALITY:
                *pValue <<= (sal_Int32)pPrinterOptions->GetOutputQuality();
                break;
            case HANDLE_MEASUREUNIT:
                {
                    sal_Int16 nMeasure;
                    SvxFieldUnitToMeasureUnit( pDoc->GetUIUnit(), nMeasure );
                    *pValue <<= nMeasure;
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
                    SfxPrinter *pPrinter = pDocSh->GetPrinter( sal_False );
                    *pValue <<= pPrinter ? OUString ( pPrinter->GetName()) : OUString();
                }
                break;
            case HANDLE_PRINTERJOB:
                {
                    SfxPrinter *pPrinter = pDocSh->GetPrinter( sal_False );
                    if (pPrinter)
                    {
                        SvMemoryStream aStream;
                        pPrinter->Store( aStream );
                        aStream.Seek ( STREAM_SEEK_TO_END );
                        sal_uInt32 nSize = aStream.Tell();
                        aStream.Seek ( STREAM_SEEK_TO_BEGIN );
                        Sequence < sal_Int8 > aSequence ( nSize );
                        memcpy ( aSequence.getArray(), aStream.GetData(), nSize );
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
                SfxDocumentInfo& rInfo = pDocSh->GetDocInfo();
                *pValue <<= (sal_Bool)rInfo.IsQueryLoadTemplate();
            }
            break;

            case HANDLE_PRINTER_INDEPENDENT_LAYOUT:
            {
                sal_Int16 nPrinterIndependentLayout = pDoc->GetPrinterIndependentLayout();
                *pValue <<= nPrinterIndependentLayout;
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
    return OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Draw.DocumentSettings"));
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
    aSeq[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.Settings") );
    if( mpModel->IsImpressDocument() )
    {
        aSeq[1] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.DocumentSettings") );
    }
    else
    {
        aSeq[1] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.DocumentSettings") );
    }

    return aSeq;
}

}
