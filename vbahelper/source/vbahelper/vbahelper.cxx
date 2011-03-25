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
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/script/XDefaultProperty.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/awt/PosSize.hpp>

#include <ooo/vba/msforms/XShape.hpp>

#include <comphelper/processfactory.hxx>

#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/app.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/itemset.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/viewfac.hxx>

#include <basic/sbx.hxx>
#include <basic/sbstar.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbmeth.hxx>
#include <rtl/math.hxx>
#include <sfx2/viewsh.hxx>
#include <math.h>
#include <tools/urlobj.hxx>
#include <osl/file.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <vcl/syswin.hxx>
#include <tools/diagnose_ex.h>

#ifndef OOVBA_DLLIMPLEMENTATION
#define OOVBA_DLLIMPLEMENTATION
#endif

#include <vbahelper/vbahelper.hxx>
#include <sfx2/app.hxx>
// #FIXME needs service
//#include "vbashape.hxx"
//#include "unonames.hxx"

using namespace ::com::sun::star;
using namespace ::ooo::vba;

void unoToSbxValue( SbxVariable* pVar, const uno::Any& aValue );

uno::Any sbxToUnoValue( SbxVariable* pVar );


namespace ooo
{
namespace vba
{

namespace { const double factor =  2540.0 / 72.0; }

css::uno::Reference< css::uno::XInterface > createVBAUnoAPIService( SfxObjectShell* pShell, const sal_Char* _pAsciiName ) throw (css::uno::RuntimeException)
{
    OSL_PRECOND( pShell, "createVBAUnoAPIService: no shell!" );
    ::rtl::OUString sVarName( ::rtl::OUString::createFromAscii( _pAsciiName ) );
    return getVBAServiceFactory( pShell )->createInstance( sVarName );
}

// helper method to determine if the view ( calc ) is in print-preview mode
bool isInPrintPreview( SfxViewFrame* pView )
{
    sal_uInt16 nViewNo = SID_VIEWSHELL1 - SID_VIEWSHELL0;
    if ( pView->GetObjectShell()->GetFactory().GetViewFactoryCount() >
nViewNo && !pView->GetObjectShell()->IsInPlaceActive() )
    {
        SfxViewFactory &rViewFactory =
            pView->GetObjectShell()->GetFactory().GetViewFactory(nViewNo);
        if (  pView->GetCurViewId() == rViewFactory.GetOrdinal() )
            return true;
    }
    return false;
}
#if 0
namespace excel // all in this namespace probably can be moved to sc
{


const ::rtl::OUString REPLACE_CELLS_WARNING(  RTL_CONSTASCII_USTRINGPARAM( "ReplaceCellsWarning"));
class PasteCellsWarningReseter
{
private:
    bool bInitialWarningState;
    static uno::Reference< beans::XPropertySet > getGlobalSheetSettings() throw ( uno::RuntimeException )
    {
        static uno::Reference< beans::XPropertySet > xTmpProps( ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );
        static uno::Reference<uno::XComponentContext > xContext( xTmpProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))), uno::UNO_QUERY_THROW );
        static uno::Reference<lang::XMultiComponentFactory > xServiceManager(
                xContext->getServiceManager(), uno::UNO_QUERY_THROW );
        static uno::Reference< beans::XPropertySet > xProps( xServiceManager->createInstanceWithContext( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sheet.GlobalSheetSettings" ) ) ,xContext ), uno::UNO_QUERY_THROW );
        return xProps;
    }

    bool getReplaceCellsWarning() throw ( uno::RuntimeException )
    {
        sal_Bool res = sal_False;
        getGlobalSheetSettings()->getPropertyValue( REPLACE_CELLS_WARNING ) >>= res;
        return ( res == sal_True );
    }

    void setReplaceCellsWarning( bool bState ) throw ( uno::RuntimeException )
    {
        getGlobalSheetSettings()->setPropertyValue( REPLACE_CELLS_WARNING, uno::makeAny( bState ) );
    }
public:
    PasteCellsWarningReseter() throw ( uno::RuntimeException )
    {
        bInitialWarningState = getReplaceCellsWarning();
        if ( bInitialWarningState )
            setReplaceCellsWarning( false );
    }
    ~PasteCellsWarningReseter()
    {
        if ( bInitialWarningState )
        {
            // don't allow dtor to throw
            try
            {
                setReplaceCellsWarning( true );
            }
            catch ( uno::Exception& /*e*/ ){}
        }
    }
};

void
implnPaste()
{
    PasteCellsWarningReseter resetWarningBox;
    ScTabViewShell* pViewShell = getCurrentBestViewShell();
    if ( pViewShell )
    {
        pViewShell->PasteFromSystem();
        pViewShell->CellContentChanged();
    }
}


void
implnCopy()
{
    ScTabViewShell* pViewShell = getCurrentBestViewShell();
    if ( pViewShell )
        pViewShell->CopyToClip(NULL,false,false,true);
}

void
implnCut()
{
    ScTabViewShell* pViewShell =  getCurrentBestViewShell();
    if ( pViewShell )
        pViewShell->CutToClip( NULL, sal_True );
}
void implnPasteSpecial(SfxViewShell* pViewShell, sal_uInt16 nFlags,sal_uInt16 nFunction,sal_Bool bSkipEmpty, sal_Bool bTranspose)
{
    PasteCellsWarningReseter resetWarningBox;
    sal_Bool bAsLink(sal_False), bOtherDoc(sal_False);
    InsCellCmd eMoveMode = INS_NONE;

    if ( !pTabViewShell )
        // none active, try next best
        pTabViewShell = getCurrentBestViewShell();
    if ( pTabViewShell )
    {
        ScViewData* pView = pTabViewShell->GetViewData();
        Window* pWin = ( pView != NULL ) ? pView->GetActiveWin() : NULL;
        if ( pView && pWin )
        {
            if ( bAsLink && bOtherDoc )
                pTabViewShell->PasteFromSystem(0);//SOT_FORMATSTR_ID_LINK
            else
            {
                ScTransferObj* pOwnClip = ScTransferObj::GetOwnClipboard( pWin );
                ScDocument* pDoc = NULL;
                if ( pOwnClip )
                    pDoc = pOwnClip->GetDocument();
                pTabViewShell->PasteFromClip( nFlags, pDoc,
                    nFunction, bSkipEmpty, bTranspose, bAsLink,
                    eMoveMode, IDF_NONE, sal_True );
                pTabViewShell->CellContentChanged();
            }
        }
    }

}

ScDocShell*
getDocShell( css::uno::Reference< css::frame::XModel>& xModel )
{
    uno::Reference< uno::XInterface > xIf( xModel, uno::UNO_QUERY_THROW );
    ScModelObj* pModel = dynamic_cast< ScModelObj* >( xIf.get() );
    ScDocShell* pDocShell = NULL;
    if ( pModel )
        pDocShell = (ScDocShell*)pModel->GetEmbeddedObject();
    return pDocShell;

}

ScTabViewShell*
getBestViewShell(  css::uno::Reference< css::frame::XModel>& xModel )
{
    ScDocShell* pDocShell = getDocShell( xModel );
    if ( pDocShell )
        return pDocShell->GetBestViewShell();
    return NULL;
}

ScTabViewShell*
getCurrentBestViewShell()
{
    uno::Reference< frame::XModel > xModel = getCurrentDocument();
    return getBestViewShell( xModel );
}

SfxViewFrame*
getCurrentViewFrame()
{
    ScTabViewShell* pViewShell = getCurrentBestViewShell();
    if ( pViewShell )
        return pViewShell->GetViewFrame();
    return NULL;
}
};

#endif

uno::Reference< beans::XIntrospectionAccess >
getIntrospectionAccess( const uno::Any& aObject ) throw (uno::RuntimeException)
{
    static uno::Reference< beans::XIntrospection > xIntrospection;
    if( !xIntrospection.is() )
    {
        uno::Reference< lang::XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );
        xIntrospection.set( xFactory->createInstance( rtl::OUString::createFromAscii("com.sun.star.beans.Introspection") ), uno::UNO_QUERY_THROW );
    }
    return xIntrospection->inspect( aObject );
}

uno::Reference< script::XTypeConverter >
getTypeConverter( const uno::Reference< uno::XComponentContext >& xContext ) throw (uno::RuntimeException)
{
    static uno::Reference< script::XTypeConverter > xTypeConv( xContext->getServiceManager()->createInstanceWithContext( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.script.Converter") ), xContext ), uno::UNO_QUERY_THROW );
    return xTypeConv;
}
const uno::Any&
aNULL()
{
     static  uno::Any aNULLL = uno::makeAny( uno::Reference< uno::XInterface >() );
    return aNULLL;
}

void dispatchExecute(SfxViewShell* pViewShell, sal_uInt16 nSlot, SfxCallMode nCall)
{
    SfxViewFrame* pViewFrame = NULL;
    if ( pViewShell )
        pViewFrame = pViewShell->GetViewFrame();
    if ( pViewFrame )
    {
        SfxDispatcher* pDispatcher = pViewFrame->GetDispatcher();
        if( pDispatcher )
        {
            pDispatcher->Execute( nSlot , nCall );
        }
    }
}

void
dispatchRequests( const uno::Reference< frame::XModel>& xModel, const rtl::OUString& aUrl, const uno::Sequence< beans::PropertyValue >& sProps )
{
    util::URL url;
    url.Complete = aUrl;
    rtl::OUString emptyString = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "" ));
    uno::Reference<frame::XController> xController = xModel->getCurrentController();
    uno::Reference<frame::XFrame> xFrame = xController->getFrame();
    uno::Reference<frame::XDispatchProvider> xDispatchProvider (xFrame,uno::UNO_QUERY_THROW);
    try
    {
        uno::Reference< beans::XPropertySet > xProps( ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );
        uno::Reference<uno::XComponentContext > xContext( xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))), uno::UNO_QUERY_THROW  );
        if ( !xContext.is() )
            return;

        uno::Reference<lang::XMultiComponentFactory > xServiceManager = xContext->getServiceManager();
        if ( !xServiceManager.is() )
            return;

        uno::Reference<util::XURLTransformer> xParser( xServiceManager->createInstanceWithContext(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ) ), xContext),
            uno::UNO_QUERY_THROW );
        if (!xParser.is())
            return;
        xParser->parseStrict (url);
    }
    catch ( uno::Exception & /*e*/ )
    {
        return;
    }

    uno::Reference<frame::XDispatch> xDispatcher = xDispatchProvider->queryDispatch(url,emptyString,0);

    uno::Sequence<beans::PropertyValue> dispatchProps(1);

    sal_Int32 nProps = sProps.getLength();
    beans::PropertyValue* pDest = dispatchProps.getArray();
    if ( nProps )
    {
        dispatchProps.realloc( nProps + 1 );
        // need to reaccquire pDest after realloc
        pDest = dispatchProps.getArray();
        const beans::PropertyValue* pSrc = sProps.getConstArray();
        for ( sal_Int32 index=0; index<nProps; ++index, ++pSrc, ++pDest )
            *pDest = *pSrc;
    }

    (*pDest).Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Silent" ));
    (*pDest).Value <<= (sal_Bool)sal_True;

    if (xDispatcher.is())
        xDispatcher->dispatch( url, dispatchProps );
}

void
dispatchRequests( const uno::Reference< frame::XModel>& xModel, const rtl::OUString& aUrl )
{
    uno::Sequence<beans::PropertyValue> dispatchProps;
    dispatchRequests( xModel, aUrl, dispatchProps );
}

uno::Reference< frame::XModel >
getCurrentDoc( const rtl::OUString& sKey ) throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel;
    SbxObject* pBasic = dynamic_cast< SbxObject* > ( SFX_APP()->GetBasic() );
    SbxObject* basicChosen =  pBasic ;
    if ( basicChosen == NULL)
    {
        OSL_TRACE("getModelFromBasic() StarBASIC* is NULL" );
        return xModel;
    }
    SbxObject* p = pBasic;
    SbxObject* pParent = p->GetParent();
    SbxObject* pParentParent = pParent ? pParent->GetParent() : NULL;

    if( pParentParent )
    {
        basicChosen = pParentParent;
    }
    else if( pParent )
    {
        basicChosen = pParent;
    }


    uno::Any aModel;
    SbxVariable *pCompVar = basicChosen->Find(  sKey, SbxCLASS_OBJECT );

    if ( pCompVar )
    {
        aModel = sbxToUnoValue( pCompVar );
        if ( sal_False == ( aModel >>= xModel ) ||
            !xModel.is() )
        {
            throw uno::RuntimeException(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Can't extract model from basic ( its obviously not set yet ) therefore don't know the current documet context") ), uno::Reference< uno::XInterface >() );
        }
        else
        {
            OSL_TRACE("Have model points to url %s",
            ::rtl::OUStringToOString( xModel->getURL(),
                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        }
    }
    else
    {
        OSL_TRACE("Failed to get %s", rtl::OUStringToOString( sKey, RTL_TEXTENCODING_UTF8 ).getStr() );
        throw uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "Can't determine the currently selected document") ),
            uno::Reference< uno::XInterface >() );
    }
    return xModel;
}

 uno::Reference< frame::XModel >
getCurrentDocCtx( const rtl::OUString& ctxName, const uno::Reference< uno::XComponentContext >& xContext ) throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel;
     // try fallback to calling doc
     css::uno::Reference< css::container::XNameAccess > xNameAccess( xContext, css::uno::UNO_QUERY_THROW );
     xModel.set( xNameAccess->getByName( ctxName ), uno::UNO_QUERY_THROW );
     return xModel;
}

uno::Reference< frame::XModel >
getThisExcelDoc( const uno::Reference< uno::XComponentContext >& xContext ) throw (uno::RuntimeException)
{
    return getCurrentDocCtx( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ExcelDocumentContext" ) ), xContext );
}

uno::Reference< frame::XModel >
getThisWordDoc( const uno::Reference< uno::XComponentContext >& xContext ) throw (uno::RuntimeException)
{
    return getCurrentDocCtx( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("WordDocumentContext" ) ), xContext );
}

 uno::Reference< frame::XModel >
getCurrentExcelDoc( const uno::Reference< uno::XComponentContext >& xContext ) throw (uno::RuntimeException)
{
    static const rtl::OUString sThisExcelDoc( RTL_CONSTASCII_USTRINGPARAM("ThisExcelDoc" ) );
    uno::Reference< frame::XModel > xModel;
    try
    {
        xModel = getCurrentDoc( sThisExcelDoc );
    }
    catch( uno::Exception& e )
    {
        xModel = getThisExcelDoc( xContext );
    }
    return xModel;
}

 uno::Reference< frame::XModel >
getCurrentWordDoc( const uno::Reference< uno::XComponentContext >& xContext ) throw (uno::RuntimeException)
{
    static const rtl::OUString sThisWordDoc( RTL_CONSTASCII_USTRINGPARAM("ThisWordDoc" ) );
    uno::Reference< frame::XModel > xModel;
    try
    {
        xModel = getCurrentDoc( sThisWordDoc );
    }
    catch( uno::Exception& e )
    {
        xModel = getThisWordDoc( xContext );
    }
    return xModel;
}

sal_Int32
OORGBToXLRGB( sal_Int32 nCol )
{
        sal_Int32 nAutoBits = nCol;
        nAutoBits &= 0xFF000000;
    sal_Int32 nRed = nCol;
    nRed &= 0x00FF0000;
    nRed >>= 16;
    sal_Int32 nGreen = nCol;
    nGreen &= 0x0000FF00;
    nGreen >>= 8;
    sal_Int32 nBlue = nCol;
    nBlue &= 0x000000FF;
    sal_Int32 nRGB =  ( nAutoBits | (nBlue << 16) | (nGreen << 8) | nRed );
    return nRGB;
}
sal_Int32
XLRGBToOORGB( sal_Int32 nCol )
{
        sal_Int32 nAutoBits = nCol;
        nAutoBits &= 0xFF000000;

    sal_Int32 nBlue = nCol;
    nBlue &= 0x00FF0000;
    nBlue >>= 16;
    sal_Int32 nGreen = nCol;
    nGreen &= 0x0000FF00;
    nGreen >>= 8;
    sal_Int32 nRed = nCol;
    nRed &= 0x000000FF;
    sal_Int32 nRGB =  ( nAutoBits | (nRed << 16) | (nGreen << 8) | nBlue );
    return nRGB;
}
uno::Any
OORGBToXLRGB( const uno::Any& aCol )
{
    sal_Int32 nCol(0);
    aCol >>= nCol;
    nCol = OORGBToXLRGB( nCol );
    return uno::makeAny( nCol );
}
uno::Any
XLRGBToOORGB(  const uno::Any& aCol )
{
    sal_Int32 nCol(0);
    aCol >>= nCol;
    nCol = XLRGBToOORGB( nCol );
    return uno::makeAny( nCol );
}

void PrintOutHelper( SfxViewShell* pViewShell, const uno::Any& From, const uno::Any& To, const uno::Any& Copies, const uno::Any& Preview, const uno::Any& /*ActivePrinter*/, const uno::Any& /*PrintToFile*/, const uno::Any& Collate, const uno::Any& PrToFileName, sal_Bool bUseSelection  )
{
    sal_Int32 nTo = 0;
    sal_Int32 nFrom = 0;
    sal_Int16 nCopies = 1;
    sal_Bool bPreview = sal_False;
    sal_Bool bCollate = sal_False;
    sal_Bool bSelection = bUseSelection;
    From >>= nFrom;
    To >>= nTo;
    Copies >>= nCopies;
    Preview >>= bPreview;
    if ( nCopies > 1 ) // Collate only useful when more that 1 copy
        Collate >>= bCollate;

    rtl::OUString sRange(  RTL_CONSTASCII_USTRINGPARAM( "-" ) );
    rtl::OUString sFileName;

    if (( nFrom || nTo ) )
    {
        if ( nFrom )
            sRange = ( ::rtl::OUString::valueOf( nFrom ) + sRange );
        if ( nTo )
            sRange += ::rtl::OUString::valueOf( nTo );
    }

    if (  PrToFileName.getValue() )
    {
        PrToFileName >>= sFileName;
    }
    SfxViewFrame* pViewFrame = NULL;
    if ( pViewShell )
        pViewFrame = pViewShell->GetViewFrame();
    if ( pViewFrame )
    {
        SfxAllItemSet aArgs( SFX_APP()->GetPool() );

        SfxBoolItem sfxCollate( SID_PRINT_COLLATE, bCollate );
        aArgs.Put( sfxCollate, sfxCollate.Which() );
        SfxInt16Item sfxCopies( SID_PRINT_COPIES, nCopies );
        aArgs.Put( sfxCopies, sfxCopies.Which() );
        if ( sFileName.getLength() )
        {
            SfxStringItem sfxFileName( SID_FILE_NAME, sFileName);
            aArgs.Put( sfxFileName, sfxFileName.Which() );

        }
        if (  sRange.getLength() )
        {
            SfxStringItem sfxRange( SID_PRINT_PAGES, sRange );
            aArgs.Put( sfxRange, sfxRange.Which() );
        }
        SfxBoolItem sfxSelection( SID_SELECTION, bSelection );
        aArgs.Put( sfxSelection, sfxSelection.Which() );
        SfxBoolItem sfxAsync( SID_ASYNCHRON, sal_False );
        aArgs.Put( sfxAsync, sfxAsync.Which() );
        SfxDispatcher* pDispatcher = pViewFrame->GetDispatcher();

        if ( pDispatcher )
        {
            if ( bPreview )
            {
                if ( !pViewFrame->GetFrame().IsInPlace() )
                {
                    // #TODO is this necessary ( calc specific )
//                  SC_MOD()->InputEnterHandler();
                    pViewFrame->GetDispatcher()->Execute( SID_VIEWSHELL1, SFX_CALLMODE_SYNCHRON );
                    while ( isInPrintPreview( pViewFrame ) )
                        Application::Yield();
                }
            }
            else
                pDispatcher->Execute( (sal_uInt16)SID_PRINTDOC, (SfxCallMode)SFX_CALLMODE_SYNCHRON, aArgs );
        }

    }

    // #FIXME #TODO
    // 1 ActivePrinter ( how/can we switch a printer via API? )
    // 2 PrintToFile ( ms behaviour if this option is specified but no
    //   filename supplied 'PrToFileName' then the user will be prompted )
    // 3 Need to check behaviour of Selected sheets with range ( e.g. From & To
    //    values ) in oOO these options are mutually exclusive
    // 4 There is a pop up to do with transparent objects in the print source
    //   should be able to disable that via configuration for the duration
    //   of this method
}

 void PrintPreviewHelper( const css::uno::Any& /*EnableChanges*/, SfxViewShell* pViewShell )
{
    dispatchExecute( pViewShell, SID_VIEWSHELL1 );
}

sal_Int32 extractIntFromAny( const uno::Any& rAny ) throw (uno::RuntimeException)
{
    switch( rAny.getValueType().getTypeClass() )
    {
        case uno::TypeClass_FLOAT:
            return static_cast< sal_Int32 >( rAny.get< float >() );
        case uno::TypeClass_DOUBLE:
            return static_cast< sal_Int32 >( rAny.get< double >() );
        case uno::TypeClass_BYTE:
        case uno::TypeClass_SHORT:
        case uno::TypeClass_LONG:
            return rAny.get< sal_Int32 >();
        default:;
    }
    throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Invalid type, cannot convert to integer." ) ), 0 );
}

sal_Int32 extractIntFromAny( const uno::Any& rAny, sal_Int32 nDefault ) throw (uno::RuntimeException)
{
    return rAny.hasValue() ? extractIntFromAny( rAny ) : nDefault;
}

bool extractBoolFromAny( const uno::Any& rAny ) throw (uno::RuntimeException)
{
    switch( rAny.getValueType().getTypeClass() )
    {
        case uno::TypeClass_FLOAT:
            return rAny.get< float >() != 0.0;
        case uno::TypeClass_DOUBLE:
            return rAny.get< double >() != 0.0;
        case uno::TypeClass_BYTE:
        case uno::TypeClass_SHORT:
        case uno::TypeClass_LONG:
            return rAny.get< sal_Int32 >() != 0;
        case uno::TypeClass_HYPER:
            return rAny.get< sal_Int64 >() != 0;
        default:;
    }
    throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Invalid type, cannot convert to boolean." ) ), 0 );
}

bool extractBoolFromAny( const uno::Any& rAny, bool bDefault ) throw (uno::RuntimeException)
{
    return rAny.hasValue() ? extractBoolFromAny( rAny ) : bDefault;
}

::rtl::OUString extractStringFromAny( const uno::Any& rAny, bool bUppercaseBool ) throw (uno::RuntimeException)
{
    switch( rAny.getValueType().getTypeClass() )
    {
        case uno::TypeClass_STRING:
            return rAny.get< ::rtl::OUString >();
        case uno::TypeClass_BOOLEAN:
            return bUppercaseBool ?
                (rAny.get< bool >() ? ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TRUE" ) ) : ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FALSE" ) )) :
                ::rtl::OUString::valueOf( (sal_Bool)rAny.get< bool >() );
        case uno::TypeClass_FLOAT:
            return ::rtl::OUString::valueOf( rAny.get< float >() );
        case uno::TypeClass_DOUBLE:
            return ::rtl::OUString::valueOf( rAny.get< double >() );
        case uno::TypeClass_BYTE:
        case uno::TypeClass_SHORT:
        case uno::TypeClass_LONG:
            return ::rtl::OUString::valueOf( rAny.get< sal_Int32 >() );
        case uno::TypeClass_HYPER:
            return ::rtl::OUString::valueOf( rAny.get< sal_Int64 >() );
        default:;
    }
    throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Invalid type, cannot convert to string." ) ), 0 );
}

::rtl::OUString extractStringFromAny( const uno::Any& rAny, const ::rtl::OUString& rDefault, bool bUppercaseBool ) throw (uno::RuntimeException)
{
    return rAny.hasValue() ? extractStringFromAny( rAny, bUppercaseBool ) : rDefault;
}

rtl::OUString getAnyAsString( const uno::Any& pvargItem ) throw ( uno::RuntimeException )
{
    return extractStringFromAny( pvargItem );
}


rtl::OUString
ContainerUtilities::getUniqueName( const uno::Sequence< ::rtl::OUString >&  _slist, const rtl::OUString& _sElementName, const ::rtl::OUString& _sSuffixSeparator)
{
    return getUniqueName(_slist, _sElementName, _sSuffixSeparator, sal_Int32(2));
}

rtl::OUString
ContainerUtilities::getUniqueName( const uno::Sequence< rtl::OUString >& _slist, const rtl::OUString _sElementName, const rtl::OUString& _sSuffixSeparator, sal_Int32 _nStartSuffix)
{
    sal_Int32 a = _nStartSuffix;
    rtl::OUString scompname = _sElementName;
    bool bElementexists = true;
    sal_Int32 nLen = _slist.getLength();
    if ( nLen == 0 )
        return _sElementName;

    while (bElementexists == true)
    {
        for (sal_Int32 i = 0; i < nLen; i++)
        {
            if (FieldInList(_slist, scompname) == -1)
            {
                return scompname;
            }
        }
        scompname = _sElementName + _sSuffixSeparator + rtl::OUString::valueOf( a++ );
    }
    return rtl::OUString();
}

sal_Int32
ContainerUtilities::FieldInList( const uno::Sequence< rtl::OUString >& SearchList, const rtl::OUString& SearchString )
{
    sal_Int32 FieldLen = SearchList.getLength();
    sal_Int32 retvalue = -1;
    for (sal_Int32 i = 0; i < FieldLen; i++)
    {
        // I wonder why comparing lexicographically is done
        // when its a match is whats interesting?
        //if (SearchList[i].compareTo(SearchString) == 0)
        if ( SearchList[i].equals( SearchString ) )
        {
            retvalue = i;
            break;
        }
    }
    return retvalue;

}
bool NeedEsc(sal_Unicode cCode)
{
    String sEsc(RTL_CONSTASCII_USTRINGPARAM(".^$+\\|{}()"));
    return (STRING_NOTFOUND != sEsc.Search(cCode));
}

rtl::OUString VBAToRegexp(const rtl::OUString &rIn, bool bForLike )
{
    rtl::OUStringBuffer sResult;
    const sal_Unicode *start = rIn.getStr();
    const sal_Unicode *end = start + rIn.getLength();

    int seenright = 0;
    if ( bForLike )
        sResult.append(static_cast<sal_Unicode>('^'));

    while (start < end)
    {
        switch (*start)
        {
            case '?':
                sResult.append(static_cast<sal_Unicode>('.'));
                start++;
                break;
            case '*':
                sResult.append(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".*")));
                start++;
                break;
            case '#':
                sResult.append(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("[0-9]")));
                start++;
                break;
            case '~':
                sResult.append(static_cast<sal_Unicode>('\\'));
                sResult.append(*(++start));
                start++;
                break;
                // dump the ~ and escape the next characture
            case ']':
                sResult.append(static_cast<sal_Unicode>('\\'));
                sResult.append(*start++);
                break;
            case '[':
                sResult.append(*start++);
                seenright = 0;
                while (start < end && !seenright)
                {
                    switch (*start)
                    {
                        case '[':
                        case '?':
                        case '*':
                        sResult.append(static_cast<sal_Unicode>('\\'));
                        sResult.append(*start);
                            break;
                        case ']':
                        sResult.append(*start);
                            seenright = 1;
                            break;
                        case '!':
                            sResult.append(static_cast<sal_Unicode>('^'));
                            break;
                        default:
                        if (NeedEsc(*start))
                            sResult.append(static_cast<sal_Unicode>('\\'));
                        sResult.append(*start);
                            break;
                    }
                    start++;
                }
                break;
            default:
                if (NeedEsc(*start))
                    sResult.append(static_cast<sal_Unicode>('\\'));
                sResult.append(*start++);
        }
    }

    if ( bForLike )
        sResult.append(static_cast<sal_Unicode>('$'));

    return sResult.makeStringAndClear( );
}

double getPixelTo100thMillimeterConversionFactor( const css::uno::Reference< css::awt::XDevice >& xDevice, sal_Bool bVertical)
{
    double fConvertFactor = 1.0;
    if( bVertical )
    {
        fConvertFactor = xDevice->getInfo().PixelPerMeterY/100000;
    }
    else
    {
        fConvertFactor = xDevice->getInfo().PixelPerMeterX/100000;
    }
    return fConvertFactor;
}

double PointsToPixels( const css::uno::Reference< css::awt::XDevice >& xDevice, double fPoints, sal_Bool bVertical)
{
    double fConvertFactor = getPixelTo100thMillimeterConversionFactor( xDevice, bVertical );
    return PointsToHmm( fPoints ) * fConvertFactor;
}
double PixelsToPoints( const css::uno::Reference< css::awt::XDevice >& xDevice, double fPixels, sal_Bool bVertical)
{
    double fConvertFactor = getPixelTo100thMillimeterConversionFactor( xDevice, bVertical );
    return HmmToPoints( fPixels/fConvertFactor );
}

sal_Int32 PointsToHmm( double fPoints )
{
    return static_cast<sal_Int32>( fPoints * factor + 0.5 );
}

double HmmToPoints( sal_Int32 nHmm )
{
    return nHmm / factor;
}

ConcreteXShapeGeometryAttributes::ConcreteXShapeGeometryAttributes( const css::uno::Reference< css::uno::XComponentContext >& /*xContext*/, const css::uno::Reference< css::drawing::XShape >& xShape )
{
    m_pShapeHelper.reset( new ShapeHelper( xShape ) );
}
ConcreteXShapeGeometryAttributes::~ConcreteXShapeGeometryAttributes()
{
}

sal_Int32 getPointerStyle( const uno::Reference< frame::XModel >& xModel )
{

    sal_Int32 nPointerStyle( POINTER_ARROW );
    try
    {
        const uno::Reference< frame::XController > xController( xModel->getCurrentController(),     uno::UNO_SET_THROW );
        const uno::Reference< frame::XFrame >      xFrame     ( xController->getFrame(),        uno::UNO_SET_THROW );
        const uno::Reference< awt::XWindow >       xWindow    ( xFrame->getContainerWindow(),   uno::UNO_SET_THROW );
        // why the heck isn't there an XWindowPeer::getPointer, but a setPointer only?
        const Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow )
            nPointerStyle = pWindow->GetSystemWindow()->GetPointer().GetStyle();
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return nPointerStyle;
}

// #FIXME this method looks wrong, shouldn't it just affect calc *or* writer
// document/frame/window(s) but not both ( and depending on what api called
// this )
void setCursorHelper( const uno::Reference< frame::XModel >& xModel, const Pointer& rPointer, sal_Bool bOverWrite )
{
    ::std::vector< uno::Reference< frame::XController > > aControllers;

    uno::Reference< frame::XModel2 > xModel2( xModel, uno::UNO_QUERY );
    if ( xModel2.is() )
    {
        const uno::Reference< container::XEnumeration > xEnumControllers( xModel2->getControllers(), uno::UNO_SET_THROW );
        while ( xEnumControllers->hasMoreElements() )
        {
            const uno::Reference< frame::XController > xController( xEnumControllers->nextElement(), uno::UNO_QUERY_THROW );
            aControllers.push_back( xController );
        }
    }
    else
    {
        if ( xModel.is() )
        {
            const uno::Reference< frame::XController > xController( xModel->getCurrentController(), uno::UNO_SET_THROW );
            aControllers.push_back( xController );
        }
    }

    for (   ::std::vector< uno::Reference< frame::XController > >::const_iterator controller = aControllers.begin();
            controller != aControllers.end();
            ++controller
        )
    {
        const uno::Reference< frame::XFrame >      xFrame     ( (*controller)->getFrame(),       uno::UNO_SET_THROW   );
        const uno::Reference< awt::XWindow >       xWindow    ( xFrame->getContainerWindow(),    uno::UNO_SET_THROW   );

        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        OSL_ENSURE( pWindow, "ScVbaApplication::setCursor: no window!" );
        if ( !pWindow )
            continue;

        pWindow->GetSystemWindow()->SetPointer( rPointer );
        pWindow->GetSystemWindow()->EnableChildPointerOverwrite( bOverWrite );
    }
}

void setDefaultPropByIntrospection( const uno::Any& aObj, const uno::Any& aValue  ) throw ( uno::RuntimeException )
{
    uno::Reference< beans::XIntrospectionAccess > xUnoAccess( getIntrospectionAccess( aObj ) );

    // #MAYBE #FIXME sort of a bit of a hack,
    uno::Reference<  script::XDefaultProperty > xDflt( aObj, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xPropSet;

    if ( xUnoAccess.is() )
        xPropSet.set( xUnoAccess->queryAdapter( ::getCppuType( (const uno::Reference< beans::XPropertySet > *)0 ) ), uno::UNO_QUERY);

    if ( xPropSet.is() )
        xPropSet->setPropertyValue( xDflt->getDefaultPropertyName(), aValue );
    else
        throw uno::RuntimeException();
}

uno::Any getPropertyValue( const uno::Sequence< beans::PropertyValue >& aProp, const rtl::OUString& aName )
{
    uno::Any result;
    for ( sal_Int32 i = 0; i < aProp.getLength(); i++ )
    {
        if ( aProp[i].Name.equals(aName) )
        {
            aProp[i].Value >>= result;
            return result;
        }
    }
    return result;
}

sal_Bool setPropertyValue( uno::Sequence< beans::PropertyValue >& aProp, const rtl::OUString& aName, const uno::Any& aValue )
{
    for ( sal_Int32 i = 0; i < aProp.getLength(); i++ )
    {
        if ( aProp[i].Name.equals(aName) )
        {
            aProp[i].Value = aValue;
            return sal_True;
        }
    }
    return sal_False;
}

// ====UserFormGeomentryHelper====
//---------------------------------------------
UserFormGeometryHelper::UserFormGeometryHelper(
        const uno::Reference< uno::XComponentContext >& /*xContext*/,
        const uno::Reference< awt::XControl >& xControl,
        double fOffsetX, double fOffsetY ) :
    mfOffsetX( fOffsetX ),
    mfOffsetY( fOffsetY ),
    mbDialog( uno::Reference< awt::XDialog >( xControl, uno::UNO_QUERY ).is() )
{
    if ( !xControl.is() )
        throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "No control is provided!" ) ),
                                     uno::Reference< uno::XInterface >() );

    mxWindow.set( xControl->getPeer(), uno::UNO_QUERY_THROW );
    mxModelProps.set( xControl->getModel(), uno::UNO_QUERY_THROW );
    mxUnitConv.set( mxWindow, uno::UNO_QUERY_THROW );
}

double UserFormGeometryHelper::getLeft() const
{
    return implGetPos( false );
}

void UserFormGeometryHelper::setLeft( double fLeft )
{
    implSetPos( fLeft, false );
}

double UserFormGeometryHelper::getTop() const
{
    return implGetPos( true );
}

void UserFormGeometryHelper::setTop( double fTop )
{
    implSetPos( fTop, true );
}

double UserFormGeometryHelper::getWidth() const
{
    return implGetSize( false, true );
}

void UserFormGeometryHelper::setWidth( double fWidth )
{
    implSetSize( fWidth, false, true );
}

double UserFormGeometryHelper::getHeight() const
{
    return implGetSize( true, true );
}

void UserFormGeometryHelper::setHeight( double fHeight )
{
    implSetSize( fHeight, true, true );
}

double UserFormGeometryHelper::getInnerWidth() const
{
    return implGetSize( false, false );
}

void UserFormGeometryHelper::setInnerWidth( double fWidth )
{
    implSetSize( fWidth, false, false );
}

double UserFormGeometryHelper::getInnerHeight() const
{
    return implGetSize( true, false );
}

void UserFormGeometryHelper::setInnerHeight( double fHeight )
{
    implSetSize( fHeight, true, false );
}

double UserFormGeometryHelper::getOffsetX() const
{
    return mfOffsetX;
}

double UserFormGeometryHelper::getOffsetY() const
{
    return mfOffsetY;
}

// ----------------------------------------------------------------------------

static const ::rtl::OUString saPosXName( RTL_CONSTASCII_USTRINGPARAM( "PositionX" ) );
static const ::rtl::OUString saPosYName( RTL_CONSTASCII_USTRINGPARAM( "PositionY" ) );
static const ::rtl::OUString saWidthName( RTL_CONSTASCII_USTRINGPARAM( "Width" ) );
static const ::rtl::OUString saHeightName( RTL_CONSTASCII_USTRINGPARAM( "Height" ) );

double UserFormGeometryHelper::implGetPos( bool bPosY ) const
{
    sal_Int32 nPosAppFont = mxModelProps->getPropertyValue( bPosY ? saPosYName : saPosXName ).get< sal_Int32 >();
    // appfont to pixel
    awt::Point aPosPixel = mxUnitConv->convertPointToPixel( awt::Point( nPosAppFont, nPosAppFont ), util::MeasureUnit::APPFONT );
    // pixel to VBA points
    awt::Point aPosPoint = mxUnitConv->convertPointToLogic( aPosPixel, util::MeasureUnit::POINT );
    return bPosY ? (aPosPoint.Y - mfOffsetY) : (aPosPoint.X - mfOffsetX);
}

void UserFormGeometryHelper::implSetPos( double fPos, bool bPosY )
{
    // convert passed VBA points to pixels
    sal_Int32 nPosPixel = static_cast< sal_Int32 >( fPos + (bPosY ? mfOffsetY : mfOffsetX) );
    awt::Point aPosPixel = mxUnitConv->convertPointToPixel( awt::Point( nPosPixel, nPosPixel ), util::MeasureUnit::POINT );
    // pixel to appfont
    awt::Point aPosAppFont = mxUnitConv->convertPointToLogic( aPosPixel, util::MeasureUnit::APPFONT );
    mxModelProps->setPropertyValue( bPosY ? saPosYName : saPosXName, uno::Any( bPosY ? aPosAppFont.Y : aPosAppFont.X ) );
}

double UserFormGeometryHelper::implGetSize( bool bHeight, bool bOuter ) const
{
    sal_Int32 nSizeAppFont = mxModelProps->getPropertyValue( bHeight ? saHeightName : saWidthName ).get< sal_Int32 >();
    // appfont to pixel
    awt::Size aSizePixel = mxUnitConv->convertSizeToPixel( awt::Size( nSizeAppFont, nSizeAppFont ), util::MeasureUnit::APPFONT );

    /*  The VBA symbols 'Width' and 'Height' return the outer size including
        window decoration (in difference to the symbols 'InnerWidth' and
        'InnerHeight'), but the window API returns the inner size. */
    if( mbDialog && bOuter )
    {
        if( const Window* pWindow = VCLUnoHelper::GetWindow( mxWindow ) )
        {
            Rectangle aOuterRect = pWindow->GetWindowExtentsRelative( NULL );
            aSizePixel = awt::Size( aOuterRect.getWidth(), aOuterRect.getHeight() );
        }
    }

    // pixel to VBA points
    awt::Size aSizePoint = mxUnitConv->convertSizeToLogic( aSizePixel, util::MeasureUnit::POINT );
    return bHeight ? aSizePoint.Height : aSizePoint.Width;
}

void UserFormGeometryHelper::implSetSize( double fSize, bool bHeight, bool bOuter )
{
    // convert passed VBA points to pixels
    sal_Int32 nSize = static_cast< sal_Int32 >( fSize );
    awt::Size aSizePixel = mxUnitConv->convertSizeToPixel( awt::Size( nSize, nSize ), util::MeasureUnit::POINT );

    /*  The VBA symbols 'Width' and 'Height' set the outer size (in difference
        to the symbols 'InnerWidth' and 'InnerHeight'), but the dialog model
        expects the inner size. We have to remove the window extents from the
        pixel height to get the same result. */
    if ( mbDialog && bOuter )
    {
        if( const Window* pWindow = VCLUnoHelper::GetWindow( mxWindow ) )
        {
            Rectangle aOuterRect = pWindow->GetWindowExtentsRelative( NULL );
            if( !aOuterRect.IsEmpty() )
            {
                awt::Rectangle aInnerRect = mxWindow->getPosSize();
                sal_Int32 nDecorWidth = aOuterRect.getWidth() - aInnerRect.Width;
                sal_Int32 nDecorHeight = aOuterRect.getHeight() - aInnerRect.Height;
                aSizePixel.Width = ::std::max< sal_Int32 >( aSizePixel.Width - nDecorWidth, 1 );
                aSizePixel.Height = ::std::max< sal_Int32 >( aSizePixel.Height - nDecorHeight, 1 );
            }
        }
    }

    awt::Size aSizeAppFont = mxUnitConv->convertSizeToLogic( aSizePixel, util::MeasureUnit::APPFONT );
    mxModelProps->setPropertyValue( bHeight ? saHeightName : saWidthName, uno::Any( bHeight ? aSizeAppFont.Height : aSizeAppFont.Width ) );
}

// ============================================================================

    double ConcreteXShapeGeometryAttributes::getLeft() const
    {
        return m_pShapeHelper->getLeft();
    }
    void ConcreteXShapeGeometryAttributes::setLeft( double nLeft )
    {
        m_pShapeHelper->setLeft( nLeft );
    }
    double ConcreteXShapeGeometryAttributes::getTop() const
    {
        return m_pShapeHelper->getTop();
    }
    void ConcreteXShapeGeometryAttributes::setTop( double nTop )
    {
        m_pShapeHelper->setTop( nTop );
    }

    double ConcreteXShapeGeometryAttributes::getHeight() const
    {
        return m_pShapeHelper->getHeight();
    }
    void ConcreteXShapeGeometryAttributes::setHeight( double nHeight )
    {
        m_pShapeHelper->setHeight( nHeight );
    }
    double ConcreteXShapeGeometryAttributes::getWidth() const
    {
        return m_pShapeHelper->getWidth();
    }
    void ConcreteXShapeGeometryAttributes::setWidth( double nWidth)
    {
        m_pShapeHelper->setWidth( nWidth );
    }


    ShapeHelper::ShapeHelper( const css::uno::Reference< css::drawing::XShape >& _xShape) throw (css::script::BasicErrorException ) : xShape( _xShape )
    {
        if( !xShape.is() )
            throw css::uno::RuntimeException( rtl::OUString::createFromAscii("No valid shape for helper"), css::uno::Reference< css::uno::XInterface >() );
    }

    double ShapeHelper::getHeight() const
    {
            return  Millimeter::getInPoints(xShape->getSize().Height);
        }


        void ShapeHelper::setHeight(double _fheight) throw ( css::script::BasicErrorException )
    {
        try
        {
            css::awt::Size aSize = xShape->getSize();
            aSize.Height = Millimeter::getInHundredthsOfOneMillimeter(_fheight);
            xShape->setSize(aSize);
        }
        catch ( css::uno::Exception& /*e*/)
        {
            throw css::script::BasicErrorException( rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
            }
    }


    double ShapeHelper::getWidth() const
    {
        return Millimeter::getInPoints(xShape->getSize().Width);
        }

    void ShapeHelper::setWidth(double _fWidth) throw ( css::script::BasicErrorException )
    {
        try
        {
            css::awt::Size aSize = xShape->getSize();
            aSize.Width = Millimeter::getInHundredthsOfOneMillimeter(_fWidth);
            xShape->setSize(aSize);
        }
        catch (css::uno::Exception& /*e*/)
        {
            throw css::script::BasicErrorException( rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
        }
    }


    double ShapeHelper::getLeft() const
    {
        return Millimeter::getInPoints(xShape->getPosition().X);
    }


    void ShapeHelper::setLeft(double _fLeft)
    {
        css::awt::Point aPoint = xShape->getPosition();
        aPoint.X = Millimeter::getInHundredthsOfOneMillimeter(_fLeft);
        xShape->setPosition(aPoint);
    }


    double ShapeHelper::getTop() const
    {
            return Millimeter::getInPoints(xShape->getPosition().Y);
    }


    void ShapeHelper::setTop(double _fTop)
    {
        css::awt::Point aPoint = xShape->getPosition();
        aPoint.Y = Millimeter::getInHundredthsOfOneMillimeter(_fTop);
        xShape->setPosition(aPoint);
    }

    void DebugHelper::exception( const rtl::OUString&  DetailedMessage, const css::uno::Exception& ex,  int err, const rtl::OUString& /*additionalArgument*/ ) throw( css::script::BasicErrorException )
    {
        // #TODO #FIXME ( do we want to support additionalArg here )
        throw css::script::BasicErrorException( DetailedMessage.concat( rtl::OUString::createFromAscii(" ") ).concat( ex.Message ), css::uno::Reference< css::uno::XInterface >(), err, rtl::OUString() );
    }

    void DebugHelper::exception( int err,  const rtl::OUString& additionalArgument ) throw( css::script::BasicErrorException )
    {
        exception( rtl::OUString(), css::uno::Exception(), err, additionalArgument );
    }
    void DebugHelper::exception( css::uno::Exception& ex ) throw( css::script::BasicErrorException )
    {
        exception( rtl::OUString(), ex, SbERR_INTERNAL_ERROR, rtl::OUString() );
    }

    Millimeter::Millimeter():m_nMillimeter(0) {}

    Millimeter::Millimeter(double mm):m_nMillimeter(mm) {}

    void Millimeter::set(double mm) { m_nMillimeter = mm; }
    void Millimeter::setInPoints(double points)
    {
        m_nMillimeter = points * factor / 100.0;
    }

    void Millimeter::setInHundredthsOfOneMillimeter(double hmm)
    {
        m_nMillimeter = hmm / 100;
    }

    double Millimeter::get()
    {
        return m_nMillimeter;
    }
    double Millimeter::getInHundredthsOfOneMillimeter()
    {
        return m_nMillimeter * 100;
    }
    double Millimeter::getInPoints()
    {
        return m_nMillimeter / factor * 100.0;
    }

    sal_Int32 Millimeter::getInHundredthsOfOneMillimeter(double points)
    {
        sal_Int32 mm = static_cast<sal_Int32>(points * factor);
        return mm;
    }

    double Millimeter::getInPoints(int _hmm)
    {
        double points = double( static_cast<double>(_hmm) / factor);
        return points;
    }

uno::Reference< XHelperInterface > getVBADocument( const uno::Reference< frame::XModel >& xModel )
{
    uno::Reference< XHelperInterface > xIf;
    try
    {
        uno::Reference< beans::XPropertySet > xDocProps( xModel, uno::UNO_QUERY_THROW );
        ::rtl::OUString aCodeName;
        xDocProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CodeName" ) ) ) >>= aCodeName;
        xIf = getUnoDocModule( aCodeName, getSfxObjShell( xModel ) );
    }
    catch( uno::Exception& )
    {
    }
    return xIf;
}

uno::Reference< XHelperInterface > getUnoDocModule( const String& aModName, SfxObjectShell* pShell )
{
    uno::Reference< XHelperInterface > xIf;
    if ( pShell )
    {
        rtl::OUString sProj( RTL_CONSTASCII_USTRINGPARAM("Standard") );
        BasicManager* pBasMgr = pShell->GetBasicManager();
        if ( pBasMgr && pBasMgr->GetName().Len() )
            sProj = pBasMgr->GetName();
        if( StarBASIC* pBasic = pShell->GetBasicManager()->GetLib( sProj ) )
            if( SbModule* pMod = pBasic->FindModule( aModName ) )
                xIf.set( pMod->GetUnoModule(), uno::UNO_QUERY );
    }
    return xIf;
}

SfxObjectShell* getSfxObjShell( const uno::Reference< frame::XModel >& xModel ) throw (uno::RuntimeException)
{
    SfxObjectShell* pFoundShell = NULL;
    if ( xModel.is() )
    {
        uno::Reference< lang::XUnoTunnel >  xObjShellTunnel( xModel, uno::UNO_QUERY_THROW );
        pFoundShell = reinterpret_cast<SfxObjectShell*>( xObjShellTunnel->getSomething(SfxObjectShell::getUnoTunnelId()));
    }
    if ( !pFoundShell )
        throw uno::RuntimeException();
    return pFoundShell;
}

} // openoffice
} //org
