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

#include <stdio.h>

#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <ooo/vba/excel/XlCalculation.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <com/sun/star/sheet/XCalculatable.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <ooo/vba/excel/XlMousePointer.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <ooo/vba/XExecutableDialog.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include<ooo/vba/XCommandBars.hpp>

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
#include "vbamenubars.hxx"
#include "tabvwsh.hxx"
#include "gridwin.hxx"
#include "vbanames.hxx"
#include <vbahelper/vbashape.hxx>
#include "vbatextboxshape.hxx"
#include "vbaassistant.hxx"
#include "sc.hrc"
#include "macromgr.hxx"
#include "defaultsoptions.hxx"

#include <osl/file.hxx>
#include <rtl/instance.hxx>

#include <sfx2/request.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/app.hxx>

#include <comphelper/processfactory.hxx>

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>

#include <docuno.hxx>

#include <basic/sbx.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbuno.hxx>
#include <basic/sbmeth.hxx>

#include "convuno.hxx"
#include "cellsuno.hxx"
#include "miscuno.hxx"
#include "unonames.hxx"
#include "docsh.hxx"
#include <vbahelper/helperdecl.hxx>
#include "excelvbahelper.hxx"

#include <basic/sbmod.hxx>
#include <basic/sbxobj.hxx>

#include "viewutil.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_QUERY;
using ::rtl::OUString;

// #TODO is this defined somewhere else?
#if ( defined UNX ) //unix
#define FILE_PATH_SEPERATOR "/"
#else // windows
#define FILE_PATH_SEPERATOR "\\"
#endif

class ActiveWorkbook : public ScVbaWorkbook
{
protected:
    virtual uno::Reference< frame::XModel > getModel()
    {
        return getCurrentExcelDoc(mxContext);
    }
public:
    ActiveWorkbook( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext) : ScVbaWorkbook(  xParent, xContext ){}
};

// ============================================================================

/** Global application settings shared by all open workbooks. */
struct ScVbaAppSettings
{
    sal_Int32 mnCalculation;
    sal_Bool mbDisplayAlerts;
    sal_Bool mbEnableEvents;
    sal_Bool mbExcel4Menus;
    sal_Bool mbDisplayNoteIndicator;
    sal_Bool mbShowWindowsInTaskbar;
    explicit ScVbaAppSettings();
};

ScVbaAppSettings::ScVbaAppSettings() :
    mnCalculation( excel::XlCalculation::xlCalculationAutomatic ),
    mbDisplayAlerts( sal_True ),
    mbEnableEvents( sal_True ),
    mbExcel4Menus( sal_False ),
    mbDisplayNoteIndicator( sal_True ),
    mbShowWindowsInTaskbar( sal_True )
{
}

struct ScVbaStaticAppSettings : public ::rtl::Static< ScVbaAppSettings, ScVbaStaticAppSettings > {};

// ============================================================================

ScVbaApplication::ScVbaApplication( const uno::Reference<uno::XComponentContext >& xContext ) :
    ScVbaApplication_BASE( xContext ),
    mrAppSettings( ScVbaStaticAppSettings::get() )
{
}

ScVbaApplication::~ScVbaApplication()
{
}

/*static*/ bool ScVbaApplication::getDocumentEventsEnabled()
{
    return ScVbaStaticAppSettings::get().mbEnableEvents;
}

SfxObjectShell* ScVbaApplication::GetDocShell( const uno::Reference< frame::XModel >& xModel ) throw (uno::RuntimeException)
{
    return static_cast< SfxObjectShell* >( excel::getDocShell( xModel ) );
}

::rtl::OUString SAL_CALL
ScVbaApplication::getExactName( const ::rtl::OUString& aApproximateName ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XExactName > xWSF( new ScVbaWSFunction( this, mxContext ) );
    return xWSF->getExactName( aApproximateName );
}

uno::Reference< beans::XIntrospectionAccess > SAL_CALL
ScVbaApplication::getIntrospection() throw(css::uno::RuntimeException)
{
    uno::Reference< script::XInvocation > xWSF( new ScVbaWSFunction( this, mxContext ) );
    return xWSF->getIntrospection();
}

uno::Any SAL_CALL
ScVbaApplication::invoke( const ::rtl::OUString& FunctionName, const uno::Sequence< uno::Any >& Params, uno::Sequence< sal_Int16 >& OutParamIndex, uno::Sequence< uno::Any >& OutParam) throw(lang::IllegalArgumentException, script::CannotConvertException, reflection::InvocationTargetException, uno::RuntimeException)
{
    /*  When calling the functions directly at the Application object, no runtime
        errors are thrown, but the error is inserted into the return value. */
    uno::Any aAny;
    try
    {
        uno::Reference< script::XInvocation > xWSF( new ScVbaWSFunction( this, mxContext ) );
        aAny = xWSF->invoke( FunctionName, Params, OutParamIndex, OutParam );
    }
    catch (const uno::Exception&)
    {
        aAny <<= script::BasicErrorException( ::rtl::OUString(), uno::Reference< uno::XInterface >(), 1000, ::rtl::OUString() );
    }
    return aAny;
}

void SAL_CALL
ScVbaApplication::setValue( const ::rtl::OUString& PropertyName, const uno::Any& Value ) throw(beans::UnknownPropertyException, script::CannotConvertException, reflection::InvocationTargetException, uno::RuntimeException)
{
    uno::Reference< script::XInvocation > xWSF( new ScVbaWSFunction( this, mxContext ) );
    xWSF->setValue( PropertyName, Value );
}

uno::Any SAL_CALL
ScVbaApplication::getValue( const ::rtl::OUString& PropertyName ) throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    uno::Reference< script::XInvocation > xWSF( new ScVbaWSFunction( this, mxContext ) );
    return xWSF->getValue( PropertyName );
}

sal_Bool SAL_CALL
ScVbaApplication::hasMethod( const ::rtl::OUString& Name ) throw(uno::RuntimeException)
{
    uno::Reference< script::XInvocation > xWSF( new ScVbaWSFunction( this, mxContext ) );
    return xWSF->hasMethod( Name );
}

sal_Bool SAL_CALL
ScVbaApplication::hasProperty( const ::rtl::OUString& Name ) throw(uno::RuntimeException)
{
    uno::Reference< script::XInvocation > xWSF( new ScVbaWSFunction( this, mxContext ) );
    return xWSF->hasProperty( Name );
}

uno::Reference< excel::XWorkbook >
ScVbaApplication::getActiveWorkbook() throw (uno::RuntimeException)
{
    uno::Reference< excel::XWorkbook > xWrkbk;
    ScDocShell* pShell = excel::getDocShell( getCurrentExcelDoc( mxContext ) );
    if ( pShell )
    {
        String aName;
        if ( pShell->GetDocument() )
        {
            aName = pShell->GetDocument()->GetCodeName();
            xWrkbk.set( getUnoDocModule(  aName, pShell ), uno::UNO_QUERY );
            // fallback ( e.g. it's possible a new document was created via the api )
            // in that case the document will not have the appropriate Document Modules
            // #TODO #FIXME ( needs to be fixes as part of providing support for an overall document
            // vba mode etc. )
            if ( !xWrkbk.is() )
                return new ActiveWorkbook( this, mxContext );
        }
    }
    return xWrkbk;
}

uno::Reference< excel::XWorkbook > SAL_CALL
ScVbaApplication::getThisWorkbook() throw (uno::RuntimeException)
{
    uno::Reference< excel::XWorkbook > xWrkbk;
    ScDocShell* pShell = excel::getDocShell( getThisExcelDoc( mxContext ) );
    if ( pShell )
    {
        String aName;
        if ( pShell->GetDocument() )
        {
            aName = pShell->GetDocument()->GetCodeName();
            xWrkbk.set( getUnoDocModule( aName, pShell ), uno::UNO_QUERY );
            // fallback ( e.g. it's possible a new document was created via the api )
            // in that case the document will not have the appropriate Document Modules
            // #TODO #FIXME ( needs to be fixes as part of providing support for an overall document
            // vba mode etc. )
            if ( !xWrkbk.is() )
                return new ActiveWorkbook( this, mxContext );
        }
    }
    return xWrkbk;
}

uno::Reference< XAssistant > SAL_CALL
ScVbaApplication::getAssistant() throw (uno::RuntimeException)
{
    return uno::Reference< XAssistant >( new ScVbaAssistant( this, mxContext ) );
}

uno::Any SAL_CALL
ScVbaApplication::getSelection() throw (uno::RuntimeException)
{
    OSL_TRACE("** ScVbaApplication::getSelection() ** ");
    uno::Reference< frame::XModel > xModel( getCurrentDocument() );

    Reference< view::XSelectionSupplier > xSelSupp( xModel->getCurrentController(), UNO_QUERY_THROW );
    Reference< beans::XPropertySet > xPropSet( xSelSupp, UNO_QUERY_THROW );
    OUString aPropName( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_FILTERED_RANGE_SELECTION ) );
    uno::Any aOldVal = xPropSet->getPropertyValue( aPropName );
    uno::Any any;
    any <<= false;
    xPropSet->setPropertyValue( aPropName, any );
    uno::Reference< uno::XInterface > aSelection = ScUnoHelpFunctions::AnyToInterface(
        xSelSupp->getSelection() );
    xPropSet->setPropertyValue( aPropName, aOldVal );

    if (!aSelection.is())
    {
        throw uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("failed to obtain current selection")),
            uno::Reference< uno::XInterface >() );
    }

    uno::Reference< lang::XServiceInfo > xServiceInfo( aSelection, uno::UNO_QUERY_THROW );
    rtl::OUString sImplementationName = xServiceInfo->getImplementationName();

    if( sImplementationName.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.SvxShapeCollection")) )
    {
        uno::Reference< drawing::XShapes > xShapes( aSelection, uno::UNO_QUERY_THROW );
        uno::Reference< container::XIndexAccess > xIndexAccess( xShapes, uno::UNO_QUERY_THROW );
        uno::Reference< drawing::XShape > xShape( xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW );
    // if ScVbaShape::getType( xShape ) == office::MsoShapeType::msoAutoShape
    // and the uno object implements the com.sun.star.drawing.Text service
    // return a textboxshape object
    if ( ScVbaShape::getType( xShape ) == office::MsoShapeType::msoAutoShape )
    {
        uno::Reference< lang::XServiceInfo > xShapeServiceInfo( xShape, uno::UNO_QUERY_THROW );
        if ( xShapeServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.Text" ) ) )  )
        {
                return uno::makeAny( uno::Reference< msforms::XTextBoxShape >(new ScVbaTextBoxShape( mxContext, xShape, xShapes, xModel ) ) );
        }
    }
        return uno::makeAny( uno::Reference< msforms::XShape >(new ScVbaShape( this, mxContext, xShape, xShapes, xModel, ScVbaShape::getType( xShape ) ) ) );
    }
    else if( xServiceInfo->supportsService( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.SheetCellRange")) ) ||
             xServiceInfo->supportsService( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.SheetCellRanges")) ) )
    {
        uno::Reference< table::XCellRange > xRange( aSelection, ::uno::UNO_QUERY);
        if ( !xRange.is() )
        {
            uno::Reference< sheet::XSheetCellRangeContainer > xRanges( aSelection, ::uno::UNO_QUERY);
            if ( xRanges.is() )
                return uno::makeAny( uno::Reference< excel::XRange >( new ScVbaRange( excel::getUnoSheetModuleObj( xRanges ), mxContext, xRanges ) ) );

        }
        return uno::makeAny( uno::Reference< excel::XRange >(new ScVbaRange( excel::getUnoSheetModuleObj( xRange ), mxContext, xRange ) ) );
    }
    else
    {
        throw uno::RuntimeException( sImplementationName + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
              " not supported")), uno::Reference< uno::XInterface >() );
    }
}

uno::Reference< excel::XRange >
ScVbaApplication::getActiveCell() throw (uno::RuntimeException )
{
    uno::Reference< sheet::XSpreadsheetView > xView( getCurrentDocument()->getCurrentController(), uno::UNO_QUERY_THROW );
    uno::Reference< table::XCellRange > xRange( xView->getActiveSheet(), ::uno::UNO_QUERY_THROW);
    ScTabViewShell* pViewShell = excel::getCurrentBestViewShell(mxContext);
    if ( !pViewShell )
        throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("No ViewShell available")), uno::Reference< uno::XInterface >() );
    ScViewData* pTabView = pViewShell->GetViewData();
    if ( !pTabView )
        throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("No ViewData available")), uno::Reference< uno::XInterface >() );

    sal_Int32 nCursorX = pTabView->GetCurX();
    sal_Int32 nCursorY = pTabView->GetCurY();

    uno::Reference< XHelperInterface > xParent( excel::getUnoSheetModuleObj( xRange ), uno::UNO_QUERY_THROW );
    return new ScVbaRange( xParent, mxContext, xRange->getCellRangeByPosition( nCursorX, nCursorY, nCursorX, nCursorY ) );
}

uno::Any SAL_CALL
ScVbaApplication::Workbooks( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    uno::Reference< XCollection > xWorkBooks( new ScVbaWorkbooks( this, mxContext ) );
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
        throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No ActiveWorkBook available" )),
            uno::Reference< uno::XInterface >() );

    return result;
}

uno::Any SAL_CALL
ScVbaApplication::WorksheetFunction( ) throw (::com::sun::star::uno::RuntimeException)
{
    return uno::makeAny( uno::Reference< script::XInvocation >( new ScVbaWSFunction( this, mxContext ) ) );
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
    uno::Reference< excel::XDialogs > xDialogs( new ScVbaDialogs( uno::Reference< XHelperInterface >( this ), mxContext, getCurrentDocument() ) );
    if( !aIndex.hasValue() )
        return uno::Any( xDialogs );
    return uno::Any( xDialogs->Item( aIndex ) );
}

uno::Reference< excel::XWindow > SAL_CALL
ScVbaApplication::getActiveWindow() throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel = getCurrentDocument();
    uno::Reference< frame::XController > xController( xModel->getCurrentController(), uno::UNO_SET_THROW );
    uno::Reference< XHelperInterface > xParent( getActiveWorkbook(), uno::UNO_QUERY_THROW );
    uno::Reference< excel::XWindow > xWin( new ScVbaWindow( xParent, mxContext, xModel, xController ) );
    return xWin;
}

uno::Any SAL_CALL
ScVbaApplication::getCutCopyMode() throw (uno::RuntimeException)
{
    //# FIXME TODO, implementation
    uno::Any result;
    result <<= sal_False;
    return result;
}

void SAL_CALL
ScVbaApplication::setCutCopyMode( const uno::Any& /* _cutcopymode */ ) throw (uno::RuntimeException)
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
    sal_Bool bDefault = false;
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
    uno::Reference< task::XStatusIndicatorSupplier > xStatusIndicatorSupplier( xModel->getCurrentController(), uno::UNO_QUERY_THROW );
    uno::Reference< task::XStatusIndicator > xStatusIndicator( xStatusIndicatorSupplier->getStatusIndicator(), uno::UNO_QUERY_THROW );
    if( _statusbar >>= sText )
    {
        setDisplayStatusBar( sal_True );
        if ( !sText.isEmpty() )
            xStatusIndicator->start( sText, 100 );
        else
            xStatusIndicator->end();        // restore normal state for empty text
    }
    else if( _statusbar >>= bDefault )
    {
        if( bDefault == false )
        {
            xStatusIndicator->end();
            setDisplayStatusBar( sal_True );
        }
    }
    else
        throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Invalid prarameter. It should be a string or False" )),
            uno::Reference< uno::XInterface >() );
}

::sal_Int32 SAL_CALL
ScVbaApplication::getCalculation() throw (uno::RuntimeException)
{
    // TODO: in Excel, this is an application-wide setting
    uno::Reference<sheet::XCalculatable> xCalc(getCurrentDocument(), uno::UNO_QUERY_THROW);
    if(xCalc->isAutomaticCalculationEnabled())
        return excel::XlCalculation::xlCalculationAutomatic;
    else
        return excel::XlCalculation::xlCalculationManual;
}

void SAL_CALL
ScVbaApplication::setCalculation( ::sal_Int32 _calculation ) throw (uno::RuntimeException)
{
    // TODO: in Excel, this is an application-wide setting
    uno::Reference< sheet::XCalculatable > xCalc(getCurrentDocument(), uno::UNO_QUERY_THROW);
    switch(_calculation)
    {
        case excel::XlCalculation::xlCalculationManual:
            xCalc->enableAutomaticCalculation(false);
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
    uno::Reference< excel::XWindows >  xWindows( new ScVbaWindows( this, mxContext ) );
    if ( aIndex.getValueTypeClass() == uno::TypeClass_VOID )
        return uno::Any( xWindows );
    return uno::Any( xWindows->Item( aIndex, uno::Any() ) );
}
void SAL_CALL
ScVbaApplication::wait( double time ) throw (uno::RuntimeException)
{
    StarBASIC* pBasic = SFX_APP()->GetBasic();
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
}

uno::Any SAL_CALL
ScVbaApplication::Range( const uno::Any& Cell1, const uno::Any& Cell2 ) throw (uno::RuntimeException)
{
    uno::Reference< excel::XRange > xVbRange = ScVbaRange::ApplicationRange( mxContext, Cell1, Cell2 );
    return uno::makeAny( xVbRange );
}

uno::Any SAL_CALL
ScVbaApplication::Names( const css::uno::Any& aIndex ) throw ( uno::RuntimeException )
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xPropertySet( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XNamedRanges > xNamedRanges( xPropertySet->getPropertyValue(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "NamedRanges" )) ), uno::UNO_QUERY_THROW );

    css::uno::Reference< excel::XNames > xNames ( new ScVbaNames( this , mxContext , xNamedRanges , xModel ) );
    if (  aIndex.getValueTypeClass() == uno::TypeClass_VOID )
    {
        return uno::Any( xNames );
}
    return uno::Any( xNames->Item( aIndex, uno::Any() ) );
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
        throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No activeSheet available" )),
            uno::Reference< uno::XInterface >() );
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
    sal_Bool bScroll = false;
    //R1C1-style string or a string of procedure name.

    if( Scroll.hasValue() )
    {
        sal_Bool aScroll = false;
        if( Scroll >>= aScroll )
        {
            bScroll = aScroll;
        }
        else
            throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "second parameter should be boolean" )),
                    uno::Reference< uno::XInterface >() );
    }

    rtl::OUString sRangeName;
    if( Reference >>= sRangeName )
    {
        uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
        uno::Reference< sheet::XSpreadsheetView > xSpreadsheet(
                xModel->getCurrentController(), uno::UNO_QUERY_THROW );
        uno::Reference< sheet::XSpreadsheet > xDoc = xSpreadsheet->getActiveSheet();

        ScTabViewShell* pShell = excel::getCurrentBestViewShell( mxContext );
        ScGridWindow* gridWindow = (ScGridWindow*)pShell->GetWindow();
        try
        {
            uno::Reference< excel::XRange > xVbaSheetRange = ScVbaRange::getRangeObjectForName(
                mxContext, sRangeName, excel::getDocShell( xModel ), formula::FormulaGrammar::CONV_XL_R1C1 );

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
        catch (const uno::RuntimeException&)
        {
            //maybe this should be a procedure name
            //TODO for procedure name
            //browse::XBrowseNodeFactory is a singlton. OUString(RTL_CONSTASCII_USTRINGPARAM( "/singletons/com.sun.star.script.browse.theBrowseNodeFactory"))
            //and the createView( browse::BrowseNodeFactoryViewTypes::MACROSELECTOR ) to get a root browse::XBrowseNode.
            //for query XInvocation interface.
            //but how to directly get the XInvocation?
            throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "invalid reference for range name, it should be procedure name" )),
                    uno::Reference< uno::XInterface >() );
        }
        return;
    }
    uno::Reference< excel::XRange > xRange;
    if( Reference >>= xRange )
    {
        uno::Reference< excel::XRange > xVbaRange( Reference, uno::UNO_QUERY );
        ScTabViewShell* pShell = excel::getCurrentBestViewShell( mxContext );
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
    throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "invalid reference or name" )),
            uno::Reference< uno::XInterface >() );
}

sal_Int32 SAL_CALL
ScVbaApplication::getCursor() throw (uno::RuntimeException)
{
    sal_Int32 nPointerStyle =  getPointerStyle(getCurrentDocument());

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
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
        switch( _cursor )
        {
            case excel::XlMousePointer::xlNorthwestArrow:
            {
                const Pointer& rPointer( POINTER_ARROW );
                setCursorHelper( xModel, rPointer, false );
                break;
            }
            case excel::XlMousePointer::xlWait:
            case excel::XlMousePointer::xlIBeam:
            {
                const Pointer& rPointer( static_cast< PointerStyle >( _cursor ) );
                //It will set the edit window, toobar and statusbar's mouse pointer.
                setCursorHelper( xModel, rPointer, sal_True );
                break;
            }
            case excel::XlMousePointer::xlDefault:
            {
                const Pointer& rPointer( POINTER_NULL );
                setCursorHelper( xModel, rPointer, false );
                break;
            }
            default:
                throw uno::RuntimeException( rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM("Unknown value for Cursor pointer")), uno::Reference< uno::XInterface >() );
                // TODO: isn't this a flaw in the API? It should be allowed to throw an
                // IllegalArgumentException, or so
        }
    }
    catch (const uno::Exception&)
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
// here just the status of the switch is set
// the function that throws an error message needs to
// evaluate this switch in order to know whether it has to disable the
// error message thrown by OpenOffice

void SAL_CALL
ScVbaApplication::setDisplayAlerts(sal_Bool displayAlerts) throw (uno::RuntimeException)
{
    mrAppSettings.mbDisplayAlerts = displayAlerts;
}

sal_Bool SAL_CALL
ScVbaApplication::getDisplayAlerts() throw (uno::RuntimeException)
{
    return mrAppSettings.mbDisplayAlerts;
}

void SAL_CALL
ScVbaApplication::setEnableEvents(sal_Bool bEnable) throw (uno::RuntimeException)
{
    mrAppSettings.mbEnableEvents = bEnable;
}

sal_Bool SAL_CALL
ScVbaApplication::getEnableEvents() throw (uno::RuntimeException)
{
    return mrAppSettings.mbEnableEvents;
}

sal_Bool SAL_CALL
ScVbaApplication::getDisplayFullScreen()  throw (uno::RuntimeException)
{
    SfxViewShell* pShell  = excel::getCurrentBestViewShell( mxContext );
    if ( pShell )
        return ScViewUtil::IsFullScreen( *pShell );
    return sal_False;
}

void SAL_CALL
ScVbaApplication::setDisplayFullScreen( sal_Bool bSet )  throw (uno::RuntimeException)
{
    // #FIXME calling  ScViewUtil::SetFullScreen( *pShell, bSet );
    // directly results in a strange crash, using dispatch instead
    if ( bSet != getDisplayFullScreen() )
        dispatchRequests( getCurrentDocument(), rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(".uno:FullScreen") ) );
}

sal_Bool SAL_CALL
ScVbaApplication::getDisplayScrollBars()  throw (uno::RuntimeException)
{
    ScTabViewShell* pShell  = excel::getCurrentBestViewShell( mxContext );
    if ( pShell )
    {
        return ( pShell->GetViewData()->IsHScrollMode() && pShell->GetViewData()->IsVScrollMode() );
    }
    return true;
}

void SAL_CALL
ScVbaApplication::setDisplayScrollBars( sal_Bool bSet )  throw (uno::RuntimeException)
{
    // use uno here as it does all he repainting etc. magic
    uno::Reference< sheet::XSpreadsheetView > xView( getCurrentDocument()->getCurrentController(), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xProps( xView, uno::UNO_QUERY );
    xProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HasVerticalScrollBar") ), uno::makeAny( bSet ) );
    xProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HasHorizontalScrollBar") ), uno::makeAny( bSet ) );
}

sal_Bool SAL_CALL
ScVbaApplication::getDisplayExcel4Menus() throw (css::uno::RuntimeException)
{
    return mrAppSettings.mbExcel4Menus;
}

void SAL_CALL
ScVbaApplication::setDisplayExcel4Menus( sal_Bool bSet ) throw (css::uno::RuntimeException)
{
    mrAppSettings.mbExcel4Menus = bSet;
}

sal_Bool SAL_CALL
ScVbaApplication::getDisplayNoteIndicator() throw (css::uno::RuntimeException)
{
    return mrAppSettings.mbDisplayNoteIndicator;
}

void SAL_CALL
ScVbaApplication::setDisplayNoteIndicator( sal_Bool bSet ) throw (css::uno::RuntimeException)
{
    mrAppSettings.mbDisplayNoteIndicator = bSet;
}

sal_Bool SAL_CALL
ScVbaApplication::getShowWindowsInTaskbar() throw (css::uno::RuntimeException)
{
    return mrAppSettings.mbShowWindowsInTaskbar;
}

void SAL_CALL
ScVbaApplication::setShowWindowsInTaskbar( sal_Bool bSet ) throw (css::uno::RuntimeException)
{
    mrAppSettings.mbShowWindowsInTaskbar = bSet;
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
        xPathSettings.set( xSMgr->createInstanceWithContext( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.PathSettings")), xContext ), uno::UNO_QUERY_THROW );
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
    catch (const uno::Exception&)
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

// ----------------------------------------------------------------------------
// Helpers for Intersect and Union

namespace {

typedef ::std::list< ScRange > ListOfScRange;

/** Appends all ranges of a VBA Range object in the passed Any to the list of ranges. */
void lclAddToListOfScRange( ListOfScRange& rList, const uno::Any& rArg )
        throw (script::BasicErrorException, uno::RuntimeException)
{
    if( rArg.hasValue() )
    {
        uno::Reference< excel::XRange > xRange( rArg, uno::UNO_QUERY_THROW );
        uno::Reference< XCollection > xCol( xRange->Areas( uno::Any() ), uno::UNO_QUERY_THROW );
        for( sal_Int32 nIdx = 1, nCount = xCol->getCount(); nIdx <= nCount; ++nIdx )
        {
            uno::Reference< excel::XRange > xAreaRange( xCol->Item( uno::Any( nIdx ), uno::Any() ), uno::UNO_QUERY_THROW );
            uno::Reference< sheet::XCellRangeAddressable > xAddressable( xAreaRange->getCellRange(), uno::UNO_QUERY_THROW );
            ScRange aScRange;
            ScUnoConversion::FillScRange( aScRange, xAddressable->getRangeAddress() );
            rList.push_back( aScRange );
        }
    }
}

/** Returns true, if the passed ranges can be expressed by a single range. The
    new range will be contained in r1 then, the range r2 can be removed. */
bool lclTryJoin( ScRange& r1, const ScRange& r2 )
{
    // 1) r2 is completely inside r1
    if( r1.In( r2 ) )
        return true;

    // 2) r1 is completely inside r2
    if( r2.In( r1 ) )
    {
        r1 = r2;
        return true;
    }

    SCCOL n1L = r1.aStart.Col();
    SCCOL n1R = r1.aEnd.Col();
    SCROW n1T = r1.aStart.Row();
    SCROW n1B = r1.aEnd.Row();
    SCCOL n2L = r2.aStart.Col();
    SCCOL n2R = r2.aEnd.Col();
    SCROW n2T = r2.aStart.Row();
    SCROW n2B = r2.aEnd.Row();

    // 3) r1 and r2 have equal upper and lower border
    if( (n1T == n2T) && (n1B == n2B) )
    {
        // check that r1 overlaps or touches r2
        if( ((n1L < n2L) && (n2L - 1 <= n1R)) || ((n2L < n1L) && (n1L - 1 <= n2R)) )
        {
            r1.aStart.SetCol( ::std::min( n1L, n2L ) );
            r1.aEnd.SetCol( ::std::max( n1R, n2R ) );
            return true;
        }
        return false;
    }

    // 4) r1 and r2 have equal left and right border
    if( (n1L == n2L) && (n1R == n2R) )
    {
        // check that r1 overlaps or touches r2
        if( ((n1T < n2T) && (n2T + 1 <= n1B)) || ((n2T < n1T) && (n1T + 1 <= n2B)) )
        {
            r1.aStart.SetRow( ::std::min( n1T, n2T ) );
            r1.aEnd.SetRow( ::std::max( n1B, n2B ) );
            return true;
        }
        return false;
    }

    // 5) cannot join these ranges
    return false;
}

/** Strips out ranges that are contained by other ranges, joins ranges that can be joined
    together (aligned borders, e.g. A4:D10 and B4:E10 would be combined to A4:E10. */
void lclJoinRanges( ListOfScRange& rList )
{
    ListOfScRange::iterator aOuterIt = rList.begin();
    while( aOuterIt != rList.end() )
    {
        bool bAnyErased = false;    // true = any range erased from rList
        ListOfScRange::iterator aInnerIt = rList.begin();
        while( aInnerIt != rList.end() )
        {
            bool bInnerErased = false;   // true = aInnerIt erased from rList
            // do not compare a range with itself
            if( (aOuterIt != aInnerIt) && lclTryJoin( *aOuterIt, *aInnerIt ) )
            {
                // aOuterIt points to joined range, aInnerIt will be removed
                aInnerIt = rList.erase( aInnerIt );
                bInnerErased = bAnyErased = true;
            }
            /*  If aInnerIt has been erased from rList, it already points to
                the next element (return value of list::erase()). */
            if( !bInnerErased )
                ++aInnerIt;
        }
        // if any range has been erased, repeat outer loop with the same range
        if( !bAnyErased )
            ++aOuterIt;
    }
}

/** Intersects the passed list with all ranges of a VBA Range object in the passed Any. */
void lclIntersectRanges( ListOfScRange& rList, const uno::Any& rArg )
        throw (script::BasicErrorException, uno::RuntimeException)
{
    // extract the ranges from the passed argument, will throw on invalid data
    ListOfScRange aList2;
    lclAddToListOfScRange( aList2, rArg );
    // do nothing, if the passed list is already empty
    if( !rList.empty() && !aList2.empty() )
    {
        // save original list in a local
        ListOfScRange aList1;
        aList1.swap( rList );
        // join ranges from passed argument
        lclJoinRanges( aList2 );
        // calculate intersection of the ranges in both lists
        for( ListOfScRange::const_iterator aOuterIt = aList1.begin(), aOuterEnd = aList1.end(); aOuterIt != aOuterEnd; ++aOuterIt )
        {
            for( ListOfScRange::const_iterator aInnerIt = aList2.begin(), aInnerEnd = aList2.end(); aInnerIt != aInnerEnd; ++aInnerIt )
            {
                if( aOuterIt->Intersects( *aInnerIt ) )
                {
                    ScRange aIsectRange(
                        Max( aOuterIt->aStart.Col(), aInnerIt->aStart.Col() ),
                        Max( aOuterIt->aStart.Row(), aInnerIt->aStart.Row() ),
                        Max( aOuterIt->aStart.Tab(), aInnerIt->aStart.Tab() ),
                        Min( aOuterIt->aEnd.Col(),   aInnerIt->aEnd.Col() ),
                        Min( aOuterIt->aEnd.Row(),   aInnerIt->aEnd.Row() ),
                        Min( aOuterIt->aEnd.Tab(),   aInnerIt->aEnd.Tab() ) );
                    rList.push_back( aIsectRange );
                }
            }
        }
        // again, join the result ranges
        lclJoinRanges( rList );
    }
}

/** Creates a VBA Range object from the passed list of ranges. */
uno::Reference< excel::XRange > lclCreateVbaRange(
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< frame::XModel >& rxModel,
        const ListOfScRange& rList ) throw (uno::RuntimeException)
{
    ScDocShell* pDocShell = excel::getDocShell( rxModel );
    if( !pDocShell ) throw uno::RuntimeException();

    ScRangeList aCellRanges;
    for( ListOfScRange::const_iterator aIt = rList.begin(), aEnd = rList.end(); aIt != aEnd; ++aIt )
        aCellRanges.Append( *aIt );

    if( aCellRanges.size() == 1 )
    {
        uno::Reference< table::XCellRange > xRange( new ScCellRangeObj( pDocShell, *aCellRanges.front() ) );
        return new ScVbaRange( excel::getUnoSheetModuleObj( xRange ), rxContext, xRange );
    }
    if( aCellRanges.size() > 1 )
    {
        uno::Reference< sheet::XSheetCellRangeContainer > xRanges( new ScCellRangesObj( pDocShell, aCellRanges ) );
        return new ScVbaRange( excel::getUnoSheetModuleObj( xRanges ), rxContext, xRanges );
    }
    return 0;
}

} // namespace

// ----------------------------------------------------------------------------

uno::Reference< excel::XRange > SAL_CALL ScVbaApplication::Intersect(
        const uno::Reference< excel::XRange >& rArg1, const uno::Reference< excel::XRange >& rArg2,
        const uno::Any& rArg3, const uno::Any& rArg4, const uno::Any& rArg5, const uno::Any& rArg6,
        const uno::Any& rArg7, const uno::Any& rArg8, const uno::Any& rArg9, const uno::Any& rArg10,
        const uno::Any& rArg11, const uno::Any& rArg12, const uno::Any& rArg13, const uno::Any& rArg14,
        const uno::Any& rArg15, const uno::Any& rArg16, const uno::Any& rArg17, const uno::Any& rArg18,
        const uno::Any& rArg19, const uno::Any& rArg20, const uno::Any& rArg21, const uno::Any& rArg22,
        const uno::Any& rArg23, const uno::Any& rArg24, const uno::Any& rArg25, const uno::Any& rArg26,
        const uno::Any& rArg27, const uno::Any& rArg28, const uno::Any& rArg29, const uno::Any& rArg30 )
        throw (script::BasicErrorException, uno::RuntimeException)
{
    if( !rArg1.is() || !rArg2.is() )
        DebugHelper::exception( SbERR_BAD_PARAMETER, rtl::OUString() );

    // initialize the result list with 1st parameter, join its ranges together
    ListOfScRange aList;
    lclAddToListOfScRange( aList, uno::Any( rArg1 ) );
    lclJoinRanges( aList );

    // process all other parameters, this updates the list with intersection
    lclIntersectRanges( aList, uno::Any( rArg2 ) );
    lclIntersectRanges( aList, rArg3 );
    lclIntersectRanges( aList, rArg4 );
    lclIntersectRanges( aList, rArg5 );
    lclIntersectRanges( aList, rArg6 );
    lclIntersectRanges( aList, rArg7 );
    lclIntersectRanges( aList, rArg8 );
    lclIntersectRanges( aList, rArg9 );
    lclIntersectRanges( aList, rArg10 );
    lclIntersectRanges( aList, rArg11 );
    lclIntersectRanges( aList, rArg12 );
    lclIntersectRanges( aList, rArg13 );
    lclIntersectRanges( aList, rArg14 );
    lclIntersectRanges( aList, rArg15 );
    lclIntersectRanges( aList, rArg16 );
    lclIntersectRanges( aList, rArg17 );
    lclIntersectRanges( aList, rArg18 );
    lclIntersectRanges( aList, rArg19 );
    lclIntersectRanges( aList, rArg20 );
    lclIntersectRanges( aList, rArg21 );
    lclIntersectRanges( aList, rArg22 );
    lclIntersectRanges( aList, rArg23 );
    lclIntersectRanges( aList, rArg24 );
    lclIntersectRanges( aList, rArg25 );
    lclIntersectRanges( aList, rArg26 );
    lclIntersectRanges( aList, rArg27 );
    lclIntersectRanges( aList, rArg28 );
    lclIntersectRanges( aList, rArg29 );
    lclIntersectRanges( aList, rArg30 );

    // create the VBA Range object
    return lclCreateVbaRange( mxContext, getCurrentDocument(), aList );
}

uno::Reference< excel::XRange > SAL_CALL ScVbaApplication::Union(
        const uno::Reference< excel::XRange >& rArg1, const uno::Reference< excel::XRange >& rArg2,
        const uno::Any& rArg3, const uno::Any& rArg4, const uno::Any& rArg5, const uno::Any& rArg6,
        const uno::Any& rArg7, const uno::Any& rArg8, const uno::Any& rArg9, const uno::Any& rArg10,
        const uno::Any& rArg11, const uno::Any& rArg12, const uno::Any& rArg13, const uno::Any& rArg14,
        const uno::Any& rArg15, const uno::Any& rArg16, const uno::Any& rArg17, const uno::Any& rArg18,
        const uno::Any& rArg19, const uno::Any& rArg20, const uno::Any& rArg21, const uno::Any& rArg22,
        const uno::Any& rArg23, const uno::Any& rArg24, const uno::Any& rArg25, const uno::Any& rArg26,
        const uno::Any& rArg27, const uno::Any& rArg28, const uno::Any& rArg29, const uno::Any& rArg30 )
        throw (script::BasicErrorException, uno::RuntimeException)
{
    if( !rArg1.is() || !rArg2.is() )
        DebugHelper::exception( SbERR_BAD_PARAMETER, rtl::OUString() );

    ListOfScRange aList;
    lclAddToListOfScRange( aList, uno::Any( rArg1 ) );
    lclAddToListOfScRange( aList, uno::Any( rArg2 ) );
    lclAddToListOfScRange( aList, rArg3 );
    lclAddToListOfScRange( aList, rArg4 );
    lclAddToListOfScRange( aList, rArg5 );
    lclAddToListOfScRange( aList, rArg6 );
    lclAddToListOfScRange( aList, rArg7 );
    lclAddToListOfScRange( aList, rArg8 );
    lclAddToListOfScRange( aList, rArg9 );
    lclAddToListOfScRange( aList, rArg10 );
    lclAddToListOfScRange( aList, rArg11 );
    lclAddToListOfScRange( aList, rArg12 );
    lclAddToListOfScRange( aList, rArg13 );
    lclAddToListOfScRange( aList, rArg14 );
    lclAddToListOfScRange( aList, rArg15 );
    lclAddToListOfScRange( aList, rArg16 );
    lclAddToListOfScRange( aList, rArg17 );
    lclAddToListOfScRange( aList, rArg18 );
    lclAddToListOfScRange( aList, rArg19 );
    lclAddToListOfScRange( aList, rArg20 );
    lclAddToListOfScRange( aList, rArg21 );
    lclAddToListOfScRange( aList, rArg22 );
    lclAddToListOfScRange( aList, rArg23 );
    lclAddToListOfScRange( aList, rArg24 );
    lclAddToListOfScRange( aList, rArg25 );
    lclAddToListOfScRange( aList, rArg26 );
    lclAddToListOfScRange( aList, rArg27 );
    lclAddToListOfScRange( aList, rArg28 );
    lclAddToListOfScRange( aList, rArg29 );
    lclAddToListOfScRange( aList, rArg30 );

    // simply join together all ranges as much as possible, strip out covered ranges etc.
    lclJoinRanges( aList );

    // create the VBA Range object
    return lclCreateVbaRange( mxContext, getCurrentDocument(), aList );
}

void
ScVbaApplication::Volatile( const uno::Any& aVolatile )  throw ( uno::RuntimeException )
{
    sal_Bool bVolatile = sal_True;
    aVolatile >>= bVolatile;
    SbMethod* pMeth = StarBASIC::GetActiveMethod();
    if ( pMeth )
    {
        OSL_TRACE("ScVbaApplication::Volatile() In method ->%s<-", rtl::OUStringToOString( pMeth->GetName(), RTL_TEXTENCODING_UTF8 ).getStr() );
        uno::Reference< frame::XModel > xModel( getCurrentDocument() );
        ScDocument* pDoc = excel::getDocShell( xModel )->GetDocument();
        pDoc->GetMacroManager()->SetUserFuncVolatile( pMeth->GetName(), bVolatile);
    }

// this is bound to break when loading the document
    return;
}

::sal_Bool SAL_CALL
ScVbaApplication::getDisplayFormulaBar() throw ( css::uno::RuntimeException )
{
    sal_Bool bRes = false;
    ScTabViewShell* pViewShell = excel::getCurrentBestViewShell( mxContext );
    if ( pViewShell )
    {
        SfxBoolItem sfxFormBar( FID_TOGGLEINPUTLINE);
        SfxAllItemSet reqList(  SFX_APP()->GetPool() );
        reqList.Put( sfxFormBar );

        pViewShell->GetState( reqList );
        const SfxPoolItem *pItem=0;
        if ( reqList.GetItemState( FID_TOGGLEINPUTLINE, false, &pItem ) == SFX_ITEM_SET )
            bRes =   ((SfxBoolItem*)pItem)->GetValue();
    }
    return bRes;
}

void SAL_CALL
ScVbaApplication::setDisplayFormulaBar( ::sal_Bool _displayformulabar ) throw ( css::uno::RuntimeException )
{
    ScTabViewShell* pViewShell = excel::getCurrentBestViewShell( mxContext );
    if ( pViewShell && ( _displayformulabar !=  getDisplayFormulaBar() ) )
    {
        SfxBoolItem sfxFormBar( FID_TOGGLEINPUTLINE, _displayformulabar);
        SfxAllItemSet reqList(  SFX_APP()->GetPool() );
        SfxRequest aReq( FID_TOGGLEINPUTLINE, 0, reqList );
        pViewShell->Execute( aReq );
    }
}

uno::Any SAL_CALL
ScVbaApplication::Caller( const uno::Any& /*aIndex*/ ) throw ( uno::RuntimeException )
{
    StarBASIC* pBasic = SFX_APP()->GetBasic();
    SbMethod* pMeth = (SbMethod*)pBasic->GetRtl()->Find( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FuncCaller") ), SbxCLASS_METHOD );
    uno::Any aRet;
    if ( pMeth )
    {
        SbxVariableRef refTemp = pMeth;
        // forces a broadcast
        SbxVariableRef pNew = new  SbxMethod( *((SbxMethod*)pMeth));
                OSL_TRACE("pNew has type %d and string value %s", pNew->GetType(), rtl::OUStringToOString( pNew->GetString(), RTL_TEXTENCODING_UTF8 ).getStr() );
        aRet = sbxToUnoValue( pNew );
    }
    return aRet;
}

uno::Reference< frame::XModel >
ScVbaApplication::getCurrentDocument() throw (css::uno::RuntimeException)
{
    return getCurrentExcelDoc(mxContext);
}

uno::Any SAL_CALL
ScVbaApplication::MenuBars( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    uno::Reference< XCommandBars > xCommandBars( CommandBars( uno::Any() ), uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xMenuBars( new ScVbaMenuBars( this, mxContext, xCommandBars ) );
    if (  aIndex.hasValue() )
    {
        return uno::Any ( xMenuBars->Item( aIndex, uno::Any() ) );
    }

    return uno::Any( xMenuBars );
}

rtl::OUString
ScVbaApplication::getServiceImplName()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScVbaApplication"));
}

uno::Sequence< rtl::OUString >
ScVbaApplication::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.Application" ) );
    }
    return aServiceNames;
}

namespace application
{
namespace sdecl = comphelper::service_decl;
sdecl::vba_service_class_<ScVbaApplication, sdecl::with_args<false> > serviceImpl;
extern sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "ScVbaApplication",
    "ooo.vba.excel.Application" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
