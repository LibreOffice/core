/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaapplication.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2007-10-09 15:28:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include <stdio.h>


#include<com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include<com/sun/star/view/XSelectionSupplier.hpp>
#include<org/openoffice/excel/XlCalculation.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <org/openoffice/excel/XlMousePointer.hpp>

#include "vbaapplication.hxx"
#include "vbaworkbooks.hxx"
#include "vbaworkbook.hxx"
#include "vbaworksheets.hxx"
#include "vbarange.hxx"
#include "vbawsfunction.hxx"
#include "vbadialogs.hxx"
#include "vbawindow.hxx"
#include "vbawindows.hxx"
#include "vbaglobals.hxx"
#include "tabvwsh.hxx"
#include "gridwin.hxx"

//start test includes
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/app.hxx>

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <tools/diagnose_ex.h>

#include <docuno.hxx>

#include <basic/sbx.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbuno.hxx>
#include <basic/sbmeth.hxx>
//end test includes

using namespace ::org::openoffice;
using namespace ::com::sun::star;

#define EXCELVERSION "11.0"

class ActiveWorkbook : public ScVbaWorkbook
{
protected:
    virtual uno::Reference< frame::XModel > getModel()
    {
        return getCurrentDocument();
    }
public:
    ActiveWorkbook( uno::Reference< uno::XComponentContext >& xContext) : ScVbaWorkbook(  xContext ){}
};

ScVbaApplication::ScVbaApplication( uno::Reference<uno::XComponentContext >& xContext ): m_xContext( xContext ), m_xCalculation( excel::XlCalculation::xlCalculationAutomatic )
{
}

ScVbaApplication::~ScVbaApplication()
{
}


uno::Reference< excel::XWorkbook >
ScVbaApplication::getActiveWorkbook() throw (uno::RuntimeException)
{
    return new ActiveWorkbook( m_xContext );
}
uno::Reference< excel::XWorkbook > SAL_CALL
ScVbaApplication::getThisWorkbook() throw (uno::RuntimeException)
{
    return getActiveWorkbook();
}

uno::Reference< excel::XRange >
ScVbaApplication::getSelection() throw (uno::RuntimeException)
{
    uno::Reference< table::XCellRange > xRange( getCurrentDocument()->getCurrentSelection(), ::uno::UNO_QUERY);
    if ( !xRange.is() )
    {
        uno::Reference< sheet::XSheetCellRangeContainer > xRanges( getCurrentDocument()->getCurrentSelection(), ::uno::UNO_QUERY);
        if ( xRanges.is() )
            return uno::Reference< excel::XRange >( new ScVbaRange( m_xContext, xRanges ) );

    }
    return uno::Reference< excel::XRange >( new ScVbaRange( m_xContext, xRange ) );
}

uno::Reference< excel::XRange >
ScVbaApplication::getActiveCell() throw (uno::RuntimeException )
{
    uno::Reference< sheet::XSpreadsheetView > xView( getCurrentDocument()->getCurrentController(), uno::UNO_QUERY_THROW );
    uno::Reference< table::XCellRange > xRange( xView->getActiveSheet(), ::uno::UNO_QUERY_THROW);
    ScTabViewShell* pViewShell = getCurrentBestViewShell();
    if ( !pViewShell )
        throw uno::RuntimeException( rtl::OUString::createFromAscii("No ViewShell available"), uno::Reference< uno::XInterface >() );
    ScViewData* pTabView = pViewShell->GetViewData();
    if ( !pTabView )
        throw uno::RuntimeException( rtl::OUString::createFromAscii("No ViewData available"), uno::Reference< uno::XInterface >() );

    sal_Int32 nCursorX = pTabView->GetCurX();
    sal_Int32 nCursorY = pTabView->GetCurY();

    return uno::Reference< excel::XRange >( new ScVbaRange( m_xContext, xRange->getCellRangeByPosition( nCursorX, nCursorY,
                                        nCursorX, nCursorY ) ) );
}

sal_Bool
ScVbaApplication::getScreenUpdating() throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
    return !xModel->hasControllersLocked();
}

void
ScVbaApplication::setScreenUpdating(sal_Bool bUpdate) throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
    if (bUpdate)
        xModel->unlockControllers();
    else
        xModel->lockControllers();
}

sal_Bool
ScVbaApplication::getDisplayStatusBar() throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
    uno::Reference< frame::XFrame > xFrame( xModel->getCurrentController()->getFrame(), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xProps( xFrame, uno::UNO_QUERY_THROW );

    if( xProps.is() ){
        uno::Reference< frame::XLayoutManager > xLayoutManager( xProps->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LayoutManager")) ), uno::UNO_QUERY_THROW );
        rtl::OUString url(RTL_CONSTASCII_USTRINGPARAM( "private:resource/statusbar/statusbar" ));
        if( xLayoutManager.is() && xLayoutManager->isElementVisible( url ) ){
            return sal_True;
        }
    }
    return sal_False;
}

void
ScVbaApplication::setDisplayStatusBar(sal_Bool bDisplayStatusBar) throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
    uno::Reference< frame::XFrame > xFrame( xModel->getCurrentController()->getFrame(), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xProps( xFrame, uno::UNO_QUERY_THROW );

    if( xProps.is() ){
        uno::Reference< frame::XLayoutManager > xLayoutManager( xProps->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LayoutManager")) ), uno::UNO_QUERY_THROW );
        rtl::OUString url(RTL_CONSTASCII_USTRINGPARAM( "private:resource/statusbar/statusbar" ));
        if( xLayoutManager.is() ){
            if( bDisplayStatusBar && !xLayoutManager->isElementVisible( url ) ){
                if( !xLayoutManager->showElement( url ) )
                    xLayoutManager->createElement( url );
                return;
            }
            else if( !bDisplayStatusBar && xLayoutManager->isElementVisible( url ) ){
                xLayoutManager->hideElement( url );
                return;
            }
        }
    }
    return;
}

uno::Any SAL_CALL
ScVbaApplication::Workbooks( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    uno::Reference< vba::XCollection > xWorkBooks( new ScVbaWorkbooks( m_xContext ) );
    if (  aIndex.getValueTypeClass() == uno::TypeClass_VOID )
    {
        // void then somebody did Workbooks.something in vba
        return uno::Any( xWorkBooks );
    }

    return uno::Any ( xWorkBooks->Item( aIndex ) );
}

uno::Any SAL_CALL
ScVbaApplication::Worksheets( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    uno::Reference< excel::XWorkbook > xWorkbook( getActiveWorkbook(), uno::UNO_QUERY );
        uno::Any result;
    if ( xWorkbook.is() )
        result  = xWorkbook->Worksheets( aIndex );

    else
        // Fixme - check if this is reasonable/desired behavior
        throw uno::RuntimeException( rtl::OUString::createFromAscii(
            "No ActiveWorkBook available" ), uno::Reference< uno::XInterface >() );

    return result;
}

uno::Any SAL_CALL
ScVbaApplication::WorksheetFunction( ) throw (::com::sun::star::uno::RuntimeException)
{
        return uno::makeAny( uno::Reference< script::XInvocation >( new ScVbaWSFunction(m_xContext) ) );
}

uno::Any SAL_CALL
ScVbaApplication::Evaluate( const ::rtl::OUString& Name ) throw (uno::RuntimeException)
{
    // #TODO Evaluate allows other things to be evaluated, e.g. functions
    // I think ( like SIN(3) etc. ) need to investigate that
    // named Ranges also? e.g. [MyRange] if so need a list of named ranges
    uno::Any aVoid;
    return uno::Any( getActiveWorkbook()->getActiveSheet()->Range( uno::Any( Name ), aVoid ) );
}

uno::Any
ScVbaApplication::Dialogs( const uno::Any &aIndex ) throw (uno::RuntimeException)
{
    uno::Reference< excel::XDialogs > xDialogs( new ScVbaDialogs( m_xContext ) );
    if( !aIndex.hasValue() )
        return uno::Any( xDialogs );
    return uno::Any( xDialogs->Item( aIndex ) );
}

uno::Reference< excel::XWindow > SAL_CALL
ScVbaApplication::getActiveWindow() throw (uno::RuntimeException)
{
    return new ScVbaWindow( m_xContext, getCurrentDocument() );
}

uno::Any SAL_CALL
ScVbaApplication::getCutCopyMode() throw (uno::RuntimeException)
{
    //# FIXME TODO, implementation
    uno::Any result;
    result <<= sal_False;
    return result;
}

::rtl::OUString
ScVbaApplication::getVersion() throw (uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(EXCELVERSION));
}

void SAL_CALL
ScVbaApplication::setCutCopyMode( const uno::Any& /*_cutcopymode*/ ) throw (uno::RuntimeException)
{
    //# FIXME TODO, implementation
}

uno::Any SAL_CALL
ScVbaApplication::getStatusBar() throw (uno::RuntimeException)
{
    return uno::makeAny( !getDisplayStatusBar() );
}

void SAL_CALL
ScVbaApplication::setStatusBar( const uno::Any& _statusbar ) throw (uno::RuntimeException)
{
    rtl::OUString sText;
    sal_Bool bDefault = sal_False;
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
    uno::Reference< task::XStatusIndicatorSupplier > xStatusIndicatorSupplier( xModel->getCurrentController(), uno::UNO_QUERY_THROW );
    uno::Reference< task::XStatusIndicator > xStatusIndicator( xStatusIndicatorSupplier->getStatusIndicator(), uno::UNO_QUERY_THROW );
    if( _statusbar >>= sText )
    {
        setDisplayStatusBar( sal_True );
        xStatusIndicator->start( sText, 100 );
        //xStatusIndicator->setText( sText );
    }
    else if( _statusbar >>= bDefault )
    {
        if( bDefault == sal_False )
        {
            xStatusIndicator->end();
            setDisplayStatusBar( sal_True );
        }
    }
    else
        throw uno::RuntimeException( rtl::OUString::createFromAscii( "Invalid prarameter. It should be a string or False" ),
            uno::Reference< uno::XInterface >() );
}

double SAL_CALL
ScVbaApplication::CountA( const uno::Any& arg1 ) throw (uno::RuntimeException)
{
    double result = 0;
    uno::Reference< script::XInvocation > xInvoc( WorksheetFunction(), uno::UNO_QUERY_THROW );
    if  ( xInvoc.is() )
    {
        static rtl::OUString FunctionName( RTL_CONSTASCII_USTRINGPARAM("CountA" ) );
        uno::Sequence< uno::Any > Params(1);
        Params[0] = arg1;
        uno::Sequence< sal_Int16 > OutParamIndex;
        uno::Sequence< uno::Any > OutParam;
        xInvoc->invoke( FunctionName, Params, OutParamIndex, OutParam ) >>= result;
    }
    return result;
}

::sal_Int32 SAL_CALL
ScVbaApplication::getCalculation() throw (css::uno::RuntimeException)
{
    return m_xCalculation;
}

void SAL_CALL
ScVbaApplication::setCalculation( ::sal_Int32 _calculation ) throw (css::uno::RuntimeException)
{
    m_xCalculation = _calculation;
}

uno::Any SAL_CALL
ScVbaApplication::Windows( const uno::Any& aIndex  ) throw (uno::RuntimeException)
{
    uno::Reference< vba::XCollection >  xWindows = ScVbaWindows::Windows( m_xContext );
    if ( aIndex.getValueTypeClass() == uno::TypeClass_VOID )
        return uno::Any( xWindows );
    return uno::Any( xWindows->Item( aIndex ) );
}
void SAL_CALL
ScVbaApplication::wait( double time ) throw (css::uno::RuntimeException)
{
    StarBASIC* pBasic = SFX_APP()->GetBasic();
    SFX_APP()->EnterBasicCall();
    SbxArrayRef aArgs = new SbxArray;
    SbxVariableRef aRef = new SbxVariable;
    aRef->PutDouble( time );
    aArgs->Put(  aRef, 1 );
    SbMethod* pMeth = (SbMethod*)pBasic->GetRtl()->Find( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("WaitUntil") ), SbxCLASS_METHOD );

    if ( pMeth )
    {
        pMeth->SetParameters( aArgs );
        SbxVariableRef refTemp = pMeth;
        // forces a broadcast
        SbxVariableRef pNew = new  SbxMethod( *((SbxMethod*)pMeth));
    }
    SFX_APP()->LeaveBasicCall();

}

uno::Any SAL_CALL
ScVbaApplication::Range( const uno::Any& Cell1, const uno::Any& Cell2 ) throw (uno::RuntimeException)
{
    uno::Reference< excel::XRange > xVbRange = ScVbaRange::ApplicationRange( m_xContext, Cell1, Cell2 );
    return uno::makeAny( xVbRange );
}

uno::Reference< excel::XWorksheet > SAL_CALL
ScVbaApplication::getActiveSheet() throw (uno::RuntimeException)
{
    uno::Reference< excel::XWorksheet > result;
    uno::Reference< excel::XWorkbook > xWorkbook( getActiveWorkbook(), uno::UNO_QUERY );
    if ( xWorkbook.is() )
    {
        uno::Reference< excel::XWorksheet > xWorksheet(
            xWorkbook->getActiveSheet(), uno::UNO_QUERY );
        if ( xWorksheet.is() )
        {
            result = xWorksheet;
        }
    }

    if ( !result.is() )
    {
        // Fixme - check if this is reasonable/desired behavior
        throw uno::RuntimeException( rtl::OUString::createFromAscii(
            "No activeSheet available" ), uno::Reference< uno::XInterface >() );
    }
    return result;

}

/*******************************************************************************
 *  In msdn:
 *  Reference   Optional Variant. The destination. Can be a Range
 *  object, a string that contains a cell reference in R1C1-style notation,
 *  or a string that contains a Visual Basic procedure name.
 *  Scroll   Optional Variant. True to scrol, False to not scroll through
 *  the window. The default is False.
 *  Parser is split to three parts, Range, R1C1 string and procedure name.
 *  by test excel, it seems Scroll no effect. ???
*******************************************************************************/
void SAL_CALL
ScVbaApplication::GoTo( const uno::Any& Reference, const uno::Any& Scroll ) throw (uno::RuntimeException)
{
    //test Scroll is a boolean
    sal_Bool bScroll = sal_False;
    //R1C1-style string or a string of procedure name.

    if( Scroll.hasValue() )
    {
        sal_Bool aScroll = sal_False;
        if( Scroll >>= aScroll )
        {
            bScroll = aScroll;
        }
        else
            throw uno::RuntimeException( rtl::OUString::createFromAscii( "sencond parameter should be boolean" ),
                    uno::Reference< uno::XInterface >() );
    }

    rtl::OUString sRangeName;
    if( Reference >>= sRangeName )
    {
        uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
        uno::Reference< sheet::XSpreadsheetView > xSpreadsheet(
                xModel->getCurrentController(), uno::UNO_QUERY_THROW );
        uno::Reference< sheet::XSpreadsheet > xDoc = xSpreadsheet->getActiveSheet();

        ScTabViewShell* pShell = getCurrentBestViewShell();
        ScGridWindow* gridWindow = (ScGridWindow*)pShell->GetWindow();
        try
        {
            uno::Reference< table::XCellRange > xRange = ScVbaRange::getCellRangeForName( sRangeName, xDoc, ScAddress::CONV_XL_R1C1 );
            ScVbaRange* pRange = new ScVbaRange( m_xContext, xRange );
            uno::Reference< excel::XRange > xVbaSheetRange( pRange );
            if( bScroll )
            {
                xVbaSheetRange->Select();
                uno::Reference< excel::XWindow >  xWindow = getActiveWindow();
                ScSplitPos eWhich = pShell->GetViewData()->GetActivePart();
                sal_Int32 nValueX = pShell->GetViewData()->GetPosX(WhichH(eWhich));
                sal_Int32 nValueY = pShell->GetViewData()->GetPosY(WhichV(eWhich));
                xWindow->SmallScroll( uno::makeAny( (sal_Int16)(xVbaSheetRange->getRow() - 1) ),
                         uno::makeAny( (sal_Int16)nValueY ),
                         uno::makeAny( (sal_Int16)(xVbaSheetRange->getColumn() - 1)  ),
                         uno::makeAny( (sal_Int16)nValueX ) );
                gridWindow->GrabFocus();
            }
            else
            {
                xVbaSheetRange->Select();
                gridWindow->GrabFocus();
            }
        }
        catch( uno::RuntimeException )
        {
            //maybe this should be a procedure name
            //TODO for procedure name
            //browse::XBrowseNodeFactory is a singlton. OUString::createFromAscii( "/singletons/com.sun.star.script.browse.theBrowseNodeFactory")
            //and the createView( browse::BrowseNodeFactoryViewTypes::MACROSELECTOR ) to get a root browse::XBrowseNode.
            //for query XInvocation interface.
            //but how to directly get the XInvocation?
            throw uno::RuntimeException( rtl::OUString::createFromAscii( "invalid reference for range name, it should be procedure name" ),
                    uno::Reference< uno::XInterface >() );
        }
        return;
    }
    uno::Reference< excel::XRange > xRange;
    if( Reference >>= xRange )
    {
        uno::Reference< excel::XRange > xVbaRange( Reference, uno::UNO_QUERY );
        ScTabViewShell* pShell = getCurrentBestViewShell();
        ScGridWindow* gridWindow = (ScGridWindow*)pShell->GetWindow();
        if ( xVbaRange.is() )
        {
            //TODO bScroll should be using, In this time, it doesenot have effection
            if( bScroll )
            {
                xVbaRange->Select();
                uno::Reference< excel::XWindow >  xWindow = getActiveWindow();
                ScSplitPos eWhich = pShell->GetViewData()->GetActivePart();
                sal_Int32 nValueX = pShell->GetViewData()->GetPosX(WhichH(eWhich));
                sal_Int32 nValueY = pShell->GetViewData()->GetPosY(WhichV(eWhich));
                xWindow->SmallScroll( uno::makeAny( (sal_Int16)(xVbaRange->getRow() - 1) ),
                         uno::makeAny( (sal_Int16)nValueY ),
                         uno::makeAny( (sal_Int16)(xVbaRange->getColumn() - 1)  ),
                         uno::makeAny( (sal_Int16)nValueX ) );
                gridWindow->GrabFocus();
            }
            else
            {
                xVbaRange->Select();
                gridWindow->GrabFocus();
            }
        }
        return;
    }
    throw uno::RuntimeException( rtl::OUString::createFromAscii( "invalid reference or name" ),
            uno::Reference< uno::XInterface >() );
}

sal_Int32 SAL_CALL
ScVbaApplication::getCursor() throw (uno::RuntimeException)
{
    sal_Int32 nPointerStyle( POINTER_ARROW );
    try
    {
        const uno::Reference< frame::XModel >      xWorkingDoc( SfxObjectShell::GetWorkingDocument(), uno::UNO_SET_THROW );
        const uno::Reference< frame::XController > xController( xWorkingDoc->getCurrentController(),  uno::UNO_SET_THROW );
        const uno::Reference< frame::XFrame >      xFrame     ( xController->getFrame(),              uno::UNO_SET_THROW );
        const uno::Reference< awt::XWindow >       xWindow    ( xFrame->getContainerWindow(),         uno::UNO_SET_THROW );
        // why the heck isn't there an XWindowPeer::getPointer, but a setPointer only?
        const Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow )
            nPointerStyle = pWindow->GetSystemWindow()->GetPointer().GetStyle();
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    switch( nPointerStyle )
    {
        case POINTER_ARROW:
            return excel::XlMousePointer::xlNorthwestArrow;
        case POINTER_NULL:
            return excel::XlMousePointer::xlDefault;
        case POINTER_WAIT:
            return excel::XlMousePointer::xlWait;
        case POINTER_TEXT:
            return excel::XlMousePointer::xlIBeam;
        default:
            return excel::XlMousePointer::xlDefault;
    }
}

void SAL_CALL
ScVbaApplication::setCursor( sal_Int32 _cursor ) throw (uno::RuntimeException)
{
    try
    {
        const uno::Reference< frame::XModel2 >          xWorkingDoc     ( SfxObjectShell::GetWorkingDocument(), uno::UNO_QUERY_THROW );
        const uno::Reference< container::XEnumeration > xEnumControllers( xWorkingDoc->getControllers(),        uno::UNO_SET_THROW   );
        while ( xEnumControllers->hasMoreElements() )
        {
            const uno::Reference< frame::XController > xController( xEnumControllers->nextElement(), uno::UNO_QUERY_THROW );
            const uno::Reference< frame::XFrame >      xFrame     ( xController->getFrame(),         uno::UNO_SET_THROW   );
            const uno::Reference< awt::XWindow >       xWindow    ( xFrame->getContainerWindow(),    uno::UNO_SET_THROW   );

            Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
            OSL_ENSURE( pWindow, "ScVbaApplication::setCursor: no window!" );
            if ( !pWindow )
                continue;

            switch( _cursor )
            {
                case excel::XlMousePointer::xlNorthwestArrow:
                {
                    const Pointer& rPointer( POINTER_ARROW );
                    pWindow->GetSystemWindow()->SetPointer( rPointer );
                    pWindow->GetSystemWindow()->EnableChildPointerOverwrite( sal_False );
                    break;
                }
                case excel::XlMousePointer::xlWait:
                case excel::XlMousePointer::xlIBeam:
                {
                    const Pointer& rPointer( static_cast< PointerStyle >( _cursor ) );
                    //It will set the edit window, toobar and statusbar's mouse pointer.
                    pWindow->GetSystemWindow()->SetPointer( rPointer );
                    pWindow->GetSystemWindow()->EnableChildPointerOverwrite( sal_True );
                    //It only set the edit window's mouse pointer
                    //pWindow->.SetPointer( rPointer );
                    //pWindow->.EnableChildPointerOverwrite( sal_True );
                    //printf("\nset Cursor...%d\n", pWindow->.GetType());
                    break;
                }
                case excel::XlMousePointer::xlDefault:
                {
                    const Pointer& rPointer( POINTER_NULL );
                    pWindow->GetSystemWindow()->SetPointer( rPointer );
                    pWindow->GetSystemWindow()->EnableChildPointerOverwrite( sal_False );
                    break;
                }
                default:
                    throw uno::RuntimeException( rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM("Unknown value for Cursor pointer")), uno::Reference< uno::XInterface >() );
                    // TODO: isn't this a flaw in the API? It should be allowed to throw an
                    // IllegalArgumentException, or so
            }
        }
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}
