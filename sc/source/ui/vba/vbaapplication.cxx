/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaapplication.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:36:34 $
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
#include <com/sun/star/lang/XServiceInfo.hpp>
#include<org/openoffice/excel/XlCalculation.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <com/sun/star/sheet/XCalculatable.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <org/openoffice/excel/XlMousePointer.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>

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
#include "vbanames.hxx"
#include "vbashape.hxx"
#include "sc.hrc"

#include <osl/file.hxx>

#include <sfx2/request.hxx>
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

using namespace ::org::openoffice;
using namespace ::com::sun::star;

// #TODO is this defined somewhere else?
#if ( defined UNX ) || ( defined OS2 ) //unix
#define FILE_PATH_SEPERATOR "/"
#else // windows
#define FILE_PATH_SEPERATOR "\\"
#endif

#define EXCELVERSION "11.0"

class ActiveWorkbook : public ScVbaWorkbook
{
protected:
    virtual uno::Reference< frame::XModel > getModel()
    {
        return getCurrentDocument();
    }
public:
    ActiveWorkbook( const uno::Reference< vba::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext) : ScVbaWorkbook(  xParent, xContext ){}
};

ScVbaApplication::ScVbaApplication( uno::Reference<uno::XComponentContext >& xContext ): ScVbaApplication_BASE( uno::Reference< vba::XHelperInterface >(), xContext ), m_xCalculation( excel::XlCalculation::xlCalculationAutomatic )
{
}

ScVbaApplication::~ScVbaApplication()
{
}



uno::Reference< excel::XWorkbook >
ScVbaApplication::getActiveWorkbook() throw (uno::RuntimeException)
{
    return new ActiveWorkbook( this, mxContext );
}
uno::Reference< excel::XWorkbook > SAL_CALL
ScVbaApplication::getThisWorkbook() throw (uno::RuntimeException)
{
    return getActiveWorkbook();
}

uno::Any SAL_CALL
ScVbaApplication::getSelection() throw (uno::RuntimeException)
{
    uno::Reference< lang::XServiceInfo > xServiceInfo( getCurrentDocument()->getCurrentSelection(), uno::UNO_QUERY_THROW );
    rtl::OUString sImpementaionName = xServiceInfo->getImplementationName();
    if( sImpementaionName.equalsIgnoreAsciiCaseAscii("com.sun.star.drawing.SvxShapeCollection") )
    {
        uno::Reference< drawing::XShapes > xShapes( getCurrentDocument()->getCurrentSelection(), uno::UNO_QUERY_THROW );
        uno::Reference< container::XIndexAccess > xIndexAccess( xShapes, uno::UNO_QUERY_THROW );
        uno::Reference< drawing::XShape > xShape( xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW );
        return uno::makeAny( uno::Reference< msforms::XShape >(new ScVbaShape( this, mxContext, xShape, xShapes, ScVbaShape::getType( xShape ) ) ) );
    }
    else if( xServiceInfo->supportsService( rtl::OUString::createFromAscii("com.sun.star.sheet.SheetCellRange")) ||
             xServiceInfo->supportsService( rtl::OUString::createFromAscii("com.sun.star.sheet.SheetCellRanges")))
    {
        uno::Reference< table::XCellRange > xRange( getCurrentDocument()->getCurrentSelection(), ::uno::UNO_QUERY);
        if ( !xRange.is() )
        {
            uno::Reference< sheet::XSheetCellRangeContainer > xRanges( getCurrentDocument()->getCurrentSelection(), ::uno::UNO_QUERY);
            if ( xRanges.is() )
                return uno::makeAny( uno::Reference< excel::XRange >( new ScVbaRange( this, mxContext, xRanges ) ) );

        }
        return uno::makeAny( uno::Reference< excel::XRange >(new ScVbaRange( this, mxContext, xRange ) ) );
    }
    else
    {
        throw uno::RuntimeException( sImpementaionName + rtl::OUString::createFromAscii(" donot be surpported"), uno::Reference< uno::XInterface >() );
    }
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

    return  new ScVbaRange( this, mxContext, xRange->getCellRangeByPosition( nCursorX, nCursorY, nCursorX, nCursorY ) );
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
    uno::Reference< vba::XCollection > xWorkBooks( new ScVbaWorkbooks( this, mxContext ) );
    if (  aIndex.getValueTypeClass() == uno::TypeClass_VOID )
    {
        // void then somebody did Workbooks.something in vba
        return uno::Any( xWorkBooks );
    }

    return uno::Any ( xWorkBooks->Item( aIndex, uno::Any() ) );
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
        return uno::makeAny( uno::Reference< script::XInvocation >( new ScVbaWSFunction( this, mxContext) ) );
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
    uno::Reference< excel::XDialogs > xDialogs( new ScVbaDialogs( uno::Reference< vba::XHelperInterface >( ScVbaGlobals::getGlobalsImpl( mxContext )->getApplication(), uno::UNO_QUERY_THROW ), mxContext ) );
    if( !aIndex.hasValue() )
        return uno::Any( xDialogs );
    return uno::Any( xDialogs->Item( aIndex ) );
}

uno::Reference< excel::XWindow > SAL_CALL
ScVbaApplication::getActiveWindow() throw (uno::RuntimeException)
{
    // #FIXME sofar can't determine Parent
    return new ScVbaWindow( uno::Reference< vba::XHelperInterface >(), mxContext, getCurrentDocument() );
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
ScVbaApplication::getCalculation() throw (uno::RuntimeException)
{
    uno::Reference<sheet::XCalculatable> xCalc(getCurrentDocument(), uno::UNO_QUERY_THROW);
    if(xCalc->isAutomaticCalculationEnabled())
        return excel::XlCalculation::xlCalculationAutomatic;
    else
        return excel::XlCalculation::xlCalculationManual;
}

void SAL_CALL
ScVbaApplication::setCalculation( ::sal_Int32 _calculation ) throw (uno::RuntimeException)
{
    uno::Reference< sheet::XCalculatable > xCalc(getCurrentDocument(), uno::UNO_QUERY_THROW);
    switch(_calculation)
    {
        case excel::XlCalculation::xlCalculationManual:
            xCalc->enableAutomaticCalculation(sal_False);
            break;
        case excel::XlCalculation::xlCalculationAutomatic:
        case excel::XlCalculation::xlCalculationSemiautomatic:
            xCalc->enableAutomaticCalculation(sal_True);
            break;
    }
}

uno::Any SAL_CALL
ScVbaApplication::Windows( const uno::Any& aIndex  ) throw (uno::RuntimeException)
{
    uno::Reference< vba::XCollection >  xWindows = ScVbaWindows::Windows( mxContext );
    if ( aIndex.getValueTypeClass() == uno::TypeClass_VOID )
        return uno::Any( xWindows );
    return uno::Any( xWindows->Item( aIndex, uno::Any() ) );
}
void SAL_CALL
ScVbaApplication::wait( double time ) throw (uno::RuntimeException)
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
    uno::Reference< excel::XRange > xVbRange = ScVbaRange::ApplicationRange( mxContext, Cell1, Cell2 );
    return uno::makeAny( xVbRange );
}

uno::Any SAL_CALL
ScVbaApplication::Names( ) throw ( uno::RuntimeException )
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XNamedRanges > xNamedRanges( getActiveSheet() , uno::UNO_QUERY_THROW );
    css::uno::Reference< excel::XNames > xNames ( new ScVbaNames( this , mxContext , xNamedRanges , xModel ) );
        return uno::makeAny( xNames );
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
            uno::Reference< excel::XRange > xVbaSheetRange = ScVbaRange::getRangeObjectForName( mxContext, sRangeName, getDocShell( xModel ), ScAddress::CONV_XL_R1C1 );
;
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

namespace
{
    static uno::Reference< frame::XController > lcl_getCurrentController()
    {
        const uno::Reference< frame::XModel > xWorkingDoc( SfxObjectShell::GetCurrentComponent(), uno::UNO_QUERY );
        uno::Reference< frame::XController > xController;
        if ( xWorkingDoc.is() )
            xController.set( xWorkingDoc->getCurrentController(), uno::UNO_SET_THROW );
        else
            xController.set( SfxObjectShell::GetCurrentComponent(), uno::UNO_QUERY_THROW );
        return xController;
    }
}

sal_Int32 SAL_CALL
ScVbaApplication::getCursor() throw (uno::RuntimeException)
{
    sal_Int32 nPointerStyle( POINTER_ARROW );
    try
    {
        const uno::Reference< frame::XController > xController( lcl_getCurrentController(),     uno::UNO_SET_THROW );
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
        ::std::vector< uno::Reference< frame::XController > > aControllers;

        const uno::Reference< frame::XModel2 > xModel2( SfxObjectShell::GetCurrentComponent(), uno::UNO_QUERY );
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
            const uno::Reference< frame::XModel > xModel( SfxObjectShell::GetCurrentComponent(), uno::UNO_QUERY );
            if ( xModel.is() )
            {
                const uno::Reference< frame::XController > xController( xModel->getCurrentController(), uno::UNO_SET_THROW );
                aControllers.push_back( xController );
            }
            else
            {
                const uno::Reference< frame::XController > xController( SfxObjectShell::GetCurrentComponent(), uno::UNO_QUERY_THROW );
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

// #TODO perhaps we should switch the return type depending of the filter
// type, e.g. return Calc for Calc and Excel if its an imported doc
rtl::OUString SAL_CALL
ScVbaApplication::getName() throw (uno::RuntimeException)
{
    static rtl::OUString appName( RTL_CONSTASCII_USTRINGPARAM("Microsoft Excel" ) );
    return appName;
}

// #TODO #FIXME get/setDisplayAlerts are just stub impl
void SAL_CALL
ScVbaApplication::setDisplayAlerts(sal_Bool /*displayAlerts*/) throw (uno::RuntimeException)
{
}

sal_Bool SAL_CALL
ScVbaApplication::getDisplayAlerts() throw (uno::RuntimeException)
{
    return sal_True;
}
void SAL_CALL
ScVbaApplication::Calculate() throw(  script::BasicErrorException , uno::RuntimeException )
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XCalculatable > xCalculatable( getCurrentDocument(), uno::UNO_QUERY_THROW );
    xCalculatable->calculateAll();
}

uno::Reference< beans::XPropertySet > lcl_getPathSettingsService( const uno::Reference< uno::XComponentContext >& xContext ) throw ( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySet >  xPathSettings;
    if ( !xPathSettings.is() )
    {
        uno::Reference< lang::XMultiComponentFactory > xSMgr( xContext->getServiceManager(), uno::UNO_QUERY_THROW );
        xPathSettings.set( xSMgr->createInstanceWithContext(::rtl::OUString::createFromAscii("com.sun.star.util.PathSettings"), xContext), uno::UNO_QUERY_THROW );
    }
    return xPathSettings;
}
rtl::OUString ScVbaApplication::getOfficePath( const rtl::OUString& _sPathType ) throw ( uno::RuntimeException )
{
    rtl::OUString sRetPath;
    uno::Reference< beans::XPropertySet > xProps = lcl_getPathSettingsService( mxContext );
    try
    {
        rtl::OUString sUrl;
         xProps->getPropertyValue( _sPathType ) >>= sUrl;

        // if its a list of paths then use the last one
        sal_Int32 nIndex =  sUrl.lastIndexOf( ';' ) ;
        if ( nIndex > 0 )
            sUrl = sUrl.copy( nIndex + 1 );
        ::osl::File::getSystemPathFromFileURL( sUrl, sRetPath );
    }
    catch (uno::Exception&)
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
    return sRetPath;
}
void SAL_CALL
ScVbaApplication::setDefaultFilePath( const ::rtl::OUString& DefaultFilePath ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps = lcl_getPathSettingsService( mxContext );
    rtl::OUString aURL;
    osl::FileBase::getFileURLFromSystemPath( DefaultFilePath, aURL );
    xProps->setPropertyValue(  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Work")), uno::makeAny( aURL ) );


}

::rtl::OUString SAL_CALL
ScVbaApplication::getDefaultFilePath(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    return getOfficePath( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Work")));
}

::rtl::OUString SAL_CALL
ScVbaApplication::LibraryPath(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    return getOfficePath( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Basic")));
}

::rtl::OUString SAL_CALL
ScVbaApplication::TemplatesPath(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    return getOfficePath( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Template")));
}

::rtl::OUString SAL_CALL
ScVbaApplication::PathSeparator(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    static rtl::OUString sPathSep( RTL_CONSTASCII_USTRINGPARAM( FILE_PATH_SEPERATOR ) );
    return sPathSep;
}

uno::Reference< excel::XRange > SAL_CALL
ScVbaApplication::Intersect( const uno::Reference< excel::XRange >& Arg1, const uno::Reference< excel::XRange >& Arg2, const uno::Any& Arg3, const uno::Any& Arg4, const uno::Any& Arg5, const uno::Any& Arg6, const uno::Any& Arg7, const uno::Any& Arg8, const uno::Any& Arg9, const uno::Any& Arg10, const uno::Any& Arg11, const uno::Any& Arg12, const uno::Any& Arg13, const uno::Any& Arg14, const uno::Any& Arg15, const uno::Any& Arg16, const uno::Any& Arg17, const uno::Any& Arg18, const uno::Any& Arg19, const uno::Any& Arg20, const uno::Any& Arg21, const uno::Any& Arg22, const uno::Any& Arg23, const uno::Any& Arg24, const uno::Any& Arg25, const uno::Any& Arg26, const uno::Any& Arg27, const uno::Any& Arg28, const uno::Any& Arg29, const uno::Any& Arg30 ) throw (script::BasicErrorException, uno::RuntimeException)
{
    std::vector< uno::Reference< excel::XRange > > vRanges;
    if ( !Arg1.is() || !Arg2.is() )
        DebugHelper::exception(SbERR_BAD_PARAMETER, rtl::OUString() );

    vRanges.push_back( Arg1 );
    vRanges.push_back( Arg2 );

    if ( Arg3.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg3, uno::UNO_QUERY_THROW ) );
    if ( Arg4.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg4, uno::UNO_QUERY_THROW ) );
    if ( Arg5.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg5, uno::UNO_QUERY_THROW ) );
    if ( Arg6.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg6, uno::UNO_QUERY_THROW ) );
    if ( Arg7.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg7, uno::UNO_QUERY_THROW ) );
    if ( Arg8.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg8, uno::UNO_QUERY_THROW ) );
    if ( Arg9.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg9, uno::UNO_QUERY_THROW ) );
    if ( Arg10.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg10, uno::UNO_QUERY_THROW ) );
    if ( Arg11.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg11, uno::UNO_QUERY_THROW ) );
    if ( Arg12.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg12, uno::UNO_QUERY_THROW ) );
    if ( Arg13.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg13, uno::UNO_QUERY_THROW ) );
    if ( Arg14.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg14, uno::UNO_QUERY_THROW ) );
    if ( Arg15.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg15, uno::UNO_QUERY_THROW ) );
    if ( Arg16.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg16, uno::UNO_QUERY_THROW ) );
    if ( Arg17.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg17, uno::UNO_QUERY_THROW ) );
    if ( Arg18.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg18, uno::UNO_QUERY_THROW ) );
    if ( Arg19.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg19, uno::UNO_QUERY_THROW ) );
    if ( Arg20.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg20, uno::UNO_QUERY_THROW ) );
    if ( Arg21.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg21, uno::UNO_QUERY_THROW ) );
    if ( Arg22.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg22, uno::UNO_QUERY_THROW ) );
    if ( Arg23.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg23, uno::UNO_QUERY_THROW ) );
    if ( Arg24.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg24, uno::UNO_QUERY_THROW ) );
    if ( Arg25.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg25, uno::UNO_QUERY_THROW ) );
    if ( Arg26.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg26, uno::UNO_QUERY_THROW ) );
    if ( Arg27.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg27, uno::UNO_QUERY_THROW ) );
    if ( Arg28.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg28, uno::UNO_QUERY_THROW ) );
    if ( Arg29.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg29, uno::UNO_QUERY_THROW ) );
    if ( Arg30.hasValue() )
        vRanges.push_back( uno::Reference< excel::XRange >( Arg30, uno::UNO_QUERY_THROW ) );

    std::vector< uno::Reference< excel::XRange > >::iterator it = vRanges.begin();
    std::vector< uno::Reference< excel::XRange > >::iterator it_end = vRanges.end();

    uno::Reference< excel::XRange > xRefRange( *it );
    ++it;
    for ( ; it != it_end; ++it )
    {
        ScVbaRange* pRange = dynamic_cast< ScVbaRange * >( xRefRange.get());
        if ( pRange )
            xRefRange = pRange->intersect( *it );
        if ( !xRefRange.is() )
            return uno::Reference< excel::XRange >();
    }
    return xRefRange;
}

void
ScVbaApplication::Volatile( const uno::Any& aVolatile )  throw ( uno::RuntimeException )
{
    sal_Bool bVolatile = sal_True;
    aVolatile >>= bVolatile;
    return;
/*
    if ( bVolatile )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Volatile - not supported" ) ), uno::Reference< uno::XInterface >() );
    // bVoloatile is false - currently this only would make sense if
    // the autocalculate mode is set to be true.

    // so really this is crap, #TODO try and understand how ( or if )
    // the calculation mode and volatile interoperate
    if ( ! getCalculation() == excel::XlCalculation::xlCalculationAutomatic )
        setCalculation(  excel::XlCalculation::xlCalculationAutomatic );
*/
}

void SAL_CALL
ScVbaApplication::DoEvents() throw ( uno::RuntimeException )
{
}

::sal_Bool SAL_CALL
ScVbaApplication::getDisplayFormulaBar() throw ( css::uno::RuntimeException )
{
    sal_Bool bRes = sal_False;
    ScTabViewShell* pViewShell = getCurrentBestViewShell();
    if ( pViewShell )
    {
        SfxBoolItem sfxFormBar( FID_TOGGLEINPUTLINE);
        SfxAllItemSet reqList(  SFX_APP()->GetPool() );
        reqList.Put( sfxFormBar );

        pViewShell->GetState( reqList );
        const SfxPoolItem *pItem=0;
        if ( reqList.GetItemState( FID_TOGGLEINPUTLINE, sal_False, &pItem ) == SFX_ITEM_SET )
            bRes =   ((SfxBoolItem*)pItem)->GetValue();
    }
    return bRes;
}

void SAL_CALL
ScVbaApplication::setDisplayFormulaBar( ::sal_Bool _displayformulabar ) throw ( css::uno::RuntimeException )
{
    ScTabViewShell* pViewShell = getCurrentBestViewShell();
    if ( pViewShell && ( _displayformulabar !=  getDisplayFormulaBar() ) )
    {
        SfxBoolItem sfxFormBar( FID_TOGGLEINPUTLINE, _displayformulabar);
        SfxAllItemSet reqList(  SFX_APP()->GetPool() );
        SfxRequest aReq( FID_TOGGLEINPUTLINE, 0, reqList );
        pViewShell->Execute( aReq );
    }
}

rtl::OUString&
ScVbaApplication::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaApplication") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaApplication::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.excel.Application" ) );
    }
    return aServiceNames;
}
