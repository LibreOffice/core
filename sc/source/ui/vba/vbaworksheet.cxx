/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaworksheet.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 16:13:48 $
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
#include <cppuhelper/queryinterface.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/util/XProtectable.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XCalculatable.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XSheetCellRange.hpp>
#include <com/sun/star/sheet/XSheetCellCursor.hpp>
#include <com/sun/star/sheet/XSheetAnnotationsSupplier.hpp>
#include <com/sun/star/sheet/XUsedAreaCursor.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XSheetPastable.hpp>
#include <com/sun/star/sheet/XCellAddressable.hpp>
#include <com/sun/star/sheet/XSheetOutline.hpp>
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/table/XTableChartsSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <tools/string.hxx>

#include <svx/svdouno.hxx>

#include "cellsuno.hxx"
#include "drwlayer.hxx"

#include "vbaoutline.hxx"
#include "vbarange.hxx"
#include "vbacomments.hxx"
#include "vbaworksheet.hxx"
#include "vbachartobjects.hxx"
#include "vbapivottables.hxx"
#include "vbacombobox.hxx"

#define STANDARDWIDTH 2267
#define STANDARDHEIGHT 427
#define DOESNOTEXIST -1
using namespace com::sun::star;
using namespace org::openoffice;
static bool
nameExists( uno::Reference <sheet::XSpreadsheetDocument>& xSpreadDoc, ::rtl::OUString & name, SCTAB& nTab ) throw ( lang::IllegalArgumentException )
{
    if (!xSpreadDoc.is())
        throw lang::IllegalArgumentException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "nameExists() xSpreadDoc is null" ) ), uno::Reference< uno::XInterface  >(), 1 );
    uno::Reference <sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
    uno::Reference <container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
    if ( xIndex.is() )
    {
        SCTAB  nCount = static_cast< SCTAB >( xIndex->getCount() );
        for (SCTAB i=0; i < nCount; i++)
        {
            uno::Reference< sheet::XSpreadsheet > xSheet(xIndex->getByIndex(i), uno::UNO_QUERY);
            uno::Reference< container::XNamed > xNamed( xSheet, uno::UNO_QUERY_THROW );
            if (xNamed->getName() == name)
            {
                nTab = i;
                return true;
            }
        }
    }
    return false;
}

static void getNewSpreadsheetName (rtl::OUString &aNewName, rtl::OUString aOldName, uno::Reference <sheet::XSpreadsheetDocument>& xSpreadDoc )
{
    if (!xSpreadDoc.is())
        throw lang::IllegalArgumentException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "getNewSpreadsheetName() xSpreadDoc is null" ) ), uno::Reference< uno::XInterface  >(), 1 );
    static rtl::OUString aUnderScre( RTL_CONSTASCII_USTRINGPARAM( "_" ) );
    int currentNum =2;
    aNewName = aOldName + aUnderScre+ String::CreateFromInt32(currentNum) ;
    SCTAB nTab = 0;
    while ( nameExists(xSpreadDoc,aNewName, nTab ) )
    {
        aNewName = aOldName + aUnderScre +
        String::CreateFromInt32(++currentNum) ;
    }
}

static void removeAllSheets( uno::Reference <sheet::XSpreadsheetDocument>& xSpreadDoc, rtl::OUString aSheetName)
{
    if (!xSpreadDoc.is())
        throw lang::IllegalArgumentException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "removeAllSheets() xSpreadDoc is null" ) ), uno::Reference< uno::XInterface  >(), 1 );
    uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
    uno::Reference <container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );

    if ( xIndex.is() )
    {
        uno::Reference<container::XNameContainer> xNameContainer(xSheets,uno::UNO_QUERY_THROW);
        for (sal_Int32 i = xIndex->getCount() -1; i>= 1; i--)
        {
            uno::Reference< sheet::XSpreadsheet > xSheet(xIndex->getByIndex(i), uno::UNO_QUERY);
            uno::Reference< container::XNamed > xNamed( xSheet, uno::UNO_QUERY_THROW );
            if (xNamed.is())
            {
                xNameContainer->removeByName(xNamed->getName());
            }
        }

        uno::Reference< sheet::XSpreadsheet > xSheet(xIndex->getByIndex(0), uno::UNO_QUERY);                uno::Reference< container::XNamed > xNamed( xSheet, uno::UNO_QUERY_THROW );
        if (xNamed.is())
        {
            xNamed->setName(aSheetName);
        }
    }
}

static uno::Reference<frame::XModel>
openNewDoc(rtl::OUString aSheetName )
{
    uno::Reference<frame::XModel> xModel;
    try
    {
        uno::Reference<uno::XComponentContext > xContext(  ::cppu::defaultBootstrap_InitialComponentContext(), uno::UNO_QUERY_THROW );
        uno::Reference<lang::XMultiComponentFactory > xServiceManager(
                                        xContext->getServiceManager(), uno::UNO_QUERY_THROW );

        uno::Reference <frame::XComponentLoader > xComponentLoader(
                        xServiceManager->createInstanceWithContext(
                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ) ),
                        xContext ), uno::UNO_QUERY_THROW );

        uno::Reference<lang::XComponent > xComponent( xComponentLoader->loadComponentFromURL(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:factory/scalc" ) ),
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "_blank" ) ), 0,
                uno::Sequence < ::com::sun::star::beans::PropertyValue >() ) );
        uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( xComponent, uno::UNO_QUERY_THROW );
        if ( xSpreadDoc.is() )
        {
            removeAllSheets(xSpreadDoc,aSheetName);
        }
        xModel.set(xSpreadDoc,uno::UNO_QUERY_THROW);
    }
    catch ( ::cppu::BootstrapException & /*e*/ )
    {
    }
    catch ( uno::Exception & /*e*/ )
    {
    }
    return xModel;
}


ScVbaWorksheet::ScVbaWorksheet( const uno::Reference< uno::XComponentContext >& xContext ): m_xContext( xContext )
{
}
ScVbaWorksheet::ScVbaWorksheet(const uno::Reference< uno::XComponentContext >& xContext,
        const uno::Reference< sheet::XSpreadsheet >& xSheet,
        const uno::Reference< frame::XModel >& xModel ) throw (uno::RuntimeException) : m_xContext(xContext), mxSheet( xSheet ), mxModel(xModel)
{
}

::rtl::OUString
ScVbaWorksheet::getName() throw (uno::RuntimeException)
{
    uno::Reference< container::XNamed > xNamed( getSheet(), uno::UNO_QUERY_THROW );
    return xNamed->getName();
}

void
ScVbaWorksheet::setName(const ::rtl::OUString &rName ) throw (uno::RuntimeException)
{
    uno::Reference< container::XNamed > xNamed( getSheet(), uno::UNO_QUERY_THROW );
    xNamed->setName( rName );
}

sal_Bool
ScVbaWorksheet::getVisible() throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps( getSheet(), uno::UNO_QUERY_THROW );
    uno::Any aValue = xProps->getPropertyValue
            (rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsVisible" ) ) );
    sal_Bool bRet = false;
    aValue >>= bRet;
    return bRet;
}

void
ScVbaWorksheet::setVisible( sal_Bool bVisible ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps( getSheet(), uno::UNO_QUERY_THROW );
    uno::Any aValue( bVisible );
    xProps->setPropertyValue
            (rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsVisible" ) ), aValue);
}

uno::Reference< excel::XRange >
ScVbaWorksheet::getUsedRange() throw (uno::RuntimeException)
{
     uno::Reference< sheet::XSheetCellRange > xSheetCellRange(getSheet(), uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XSheetCellCursor > xSheetCellCursor( getSheet()->createCursorByRange( xSheetCellRange ), uno::UNO_QUERY_THROW );
    uno::Reference<sheet::XUsedAreaCursor> xUsedCursor(xSheetCellCursor,uno::UNO_QUERY_THROW);
    xUsedCursor->gotoStartOfUsedArea( false );
    xUsedCursor->gotoEndOfUsedArea( true );
    uno::Reference< table::XCellRange > xRange( xSheetCellCursor, uno::UNO_QUERY);
    return new ScVbaRange(m_xContext, xRange);
}

uno::Reference< excel::XOutline >
ScVbaWorksheet::Outline( ) throw (uno::RuntimeException)
{
    uno::Reference<sheet::XSheetOutline> xOutline(getSheet(),uno::UNO_QUERY_THROW);
    return uno::Reference<excel::XOutline> (new ScVbaOutline(m_xContext, xOutline));
}

sal_Int32
ScVbaWorksheet::getStandardWidth() throw (uno::RuntimeException)
{
    return STANDARDWIDTH ;
}

sal_Int32
ScVbaWorksheet::getStandardHeight() throw (uno::RuntimeException)
{
    return STANDARDHEIGHT;
}

sal_Bool
ScVbaWorksheet::getProtectionMode() throw (uno::RuntimeException)
{
    return false;
}

sal_Bool
ScVbaWorksheet::getProtectContents()throw (uno::RuntimeException)
{
    uno::Reference<util::XProtectable > xProtectable(getSheet(), uno::UNO_QUERY_THROW);
    return xProtectable->isProtected();
}

sal_Bool
ScVbaWorksheet::getProtectDrawingObjects() throw (uno::RuntimeException)
{
    return false;
}

void
ScVbaWorksheet::Activate() throw (uno::RuntimeException)
{
    uno::Reference< sheet::XSpreadsheetView > xSpreadsheet(
            getModel()->getCurrentController(), uno::UNO_QUERY_THROW );
    xSpreadsheet->setActiveSheet(getSheet());
}

void
ScVbaWorksheet::Select() throw (uno::RuntimeException)
{
    Activate();
}

void
ScVbaWorksheet::Move( const uno::Any& Before, const uno::Any& After ) throw (uno::RuntimeException)
{
    rtl::OUString aSheetName;
    uno::Reference<excel::XWorksheet> xSheet;
    rtl::OUString aCurrSheetName =getName();

    if (!(Before >>= xSheet) && !(After >>=xSheet)&& !(Before.hasValue()) && !(After.hasValue()))
    {
        uno::Reference< sheet::XSheetCellCursor > xSheetCellCursor = getSheet()->createCursor( );
        uno::Reference<sheet::XUsedAreaCursor> xUsedCursor(xSheetCellCursor,uno::UNO_QUERY_THROW);
            uno::Reference< table::XCellRange > xRange1( xSheetCellCursor, uno::UNO_QUERY);
        uno::Reference<excel::XRange> xRange =  new ScVbaRange(m_xContext, xRange1);
        if (xRange.is())
            xRange->Select();
        implnCopy();
        uno::Reference<frame::XModel> xModel = openNewDoc(aCurrSheetName);
        if (xModel.is())
        {
            implnPaste();
            Delete();
        }
        return ;
    }

    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( getModel(), uno::UNO_QUERY_THROW );
    SCTAB nDest = 0;
    aSheetName = xSheet->getName();
    bool bSheetExists = nameExists (xSpreadDoc, aSheetName, nDest);
    if ( bSheetExists )
    {
        sal_Bool bAfter = After.hasValue();
        if (bAfter)
            nDest++;
        uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
        xSheets->moveByName(aCurrSheetName,nDest);
    }
}

void
ScVbaWorksheet::Copy( const uno::Any& Before, const uno::Any& After ) throw (uno::RuntimeException)
{
    rtl::OUString aSheetName;
    uno::Reference<excel::XWorksheet> xSheet;
    rtl::OUString aCurrSheetName =getName();
    if (!(Before >>= xSheet) && !(After >>=xSheet)&& !(Before.hasValue()) && !(After.hasValue()))
    {
        uno::Reference< sheet::XSheetCellCursor > xSheetCellCursor = getSheet()->createCursor( );
        uno::Reference<sheet::XUsedAreaCursor> xUsedCursor(xSheetCellCursor,uno::UNO_QUERY_THROW);
            uno::Reference< table::XCellRange > xRange1( xSheetCellCursor, uno::UNO_QUERY);
        uno::Reference<excel::XRange> xRange =  new ScVbaRange(m_xContext, xRange1);
        if (xRange.is())
            xRange->Select();
        implnCopy();
        uno::Reference<frame::XModel> xModel = openNewDoc(aCurrSheetName);
        if (xModel.is())
        {
            implnPaste();
        }
        return;
    }

    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( getModel(), uno::UNO_QUERY );
    SCTAB nDest = 0;
    aSheetName = xSheet->getName();
    bool bSheetExists = nameExists (xSpreadDoc, aSheetName, nDest );

    if ( bSheetExists )
    {
        sal_Bool bAfter = After.hasValue();
        if(bAfter)
              nDest++;
        uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
        getNewSpreadsheetName(aSheetName,aCurrSheetName,xSpreadDoc);
        xSheets->copyByName(aCurrSheetName,aSheetName,nDest);
    }
}


void
ScVbaWorksheet::Paste( const uno::Any& Destination, const uno::Any& /*Link*/ ) throw (uno::RuntimeException)
{
    // #TODO# #FIXME# Link is not used
    uno::Reference<excel::XRange> xRange( Destination, uno::UNO_QUERY );
    if ( xRange.is() )
        xRange->Select();
    implnPaste();
}

void
ScVbaWorksheet::Delete() throw (uno::RuntimeException)
{
    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( getModel(), uno::UNO_QUERY_THROW );
    rtl::OUString aSheetName = getName();
    if ( xSpreadDoc.is() )
    {
        SCTAB nTab = 0;
        if (!nameExists(xSpreadDoc, aSheetName, nTab ))
        {
            return;
        }
        uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
        uno::Reference<container::XNameContainer> xNameContainer(xSheets,uno::UNO_QUERY_THROW);
        xNameContainer->removeByName(aSheetName);
    }
}

uno::Reference< excel::XWorksheet >
ScVbaWorksheet::getSheetAtOffset(SCTAB offset) throw (uno::RuntimeException)
{
    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( getModel(), uno::UNO_QUERY_THROW );
    uno::Reference <sheet::XSpreadsheets> xSheets( xSpreadDoc->getSheets(), uno::UNO_QUERY_THROW );
    uno::Reference <container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY_THROW );

    rtl::OUString aName = getName();
    SCTAB nIdx = 0;
    bool bSheetExists = nameExists (xSpreadDoc, aName, nIdx );

    if ( !bSheetExists )
        return uno::Reference< excel::XWorksheet >();
    nIdx = nIdx + offset;
    uno::Reference< sheet::XSpreadsheet > xSheet(xIndex->getByIndex(nIdx), uno::UNO_QUERY_THROW);
    return new ScVbaWorksheet (m_xContext, xSheet, getModel());
}

uno::Reference< excel::XWorksheet >
ScVbaWorksheet::getNext() throw (uno::RuntimeException)
{
    return getSheetAtOffset(static_cast<SCTAB>(1));
}

uno::Reference< excel::XWorksheet >
ScVbaWorksheet::getPrevious() throw (uno::RuntimeException)
{
    return getSheetAtOffset(-1);
}


void
ScVbaWorksheet::Protect( const uno::Any& Password, const uno::Any& /*DrawingObjects*/, const uno::Any& /*Contents*/, const uno::Any& /*Scenarios*/, const uno::Any& /*UserInterfaceOnly*/ ) throw (uno::RuntimeException)
{
    // #TODO# #FIXME# is there anything we can do witht the unused param
    // can the implementation use anything else here
    uno::Reference<util::XProtectable > xProtectable(getSheet(), uno::UNO_QUERY_THROW);
    ::rtl::OUString aPasswd;
    Password >>= aPasswd;
    xProtectable->protect( aPasswd );
}

void
ScVbaWorksheet::Unprotect( const uno::Any& Password ) throw (uno::RuntimeException)
{
    uno::Reference<util::XProtectable > xProtectable(getSheet(), uno::UNO_QUERY_THROW);
    ::rtl::OUString aPasswd;
    Password >>= aPasswd;
    xProtectable->unprotect( aPasswd );
}

void
ScVbaWorksheet::Calculate() throw (uno::RuntimeException)
{
    uno::Reference <sheet::XCalculatable> xReCalculate(getModel(), uno::UNO_QUERY_THROW);
    xReCalculate->calculate();
}

uno::Reference< excel::XRange >
ScVbaWorksheet::Range( const ::uno::Any& Cell1, const ::uno::Any& Cell2 ) throw (uno::RuntimeException)
{
    uno::Reference< excel::XRange > xSheetRange( new ScVbaRange( m_xContext
, uno::Reference< table::XCellRange >( getSheet(), uno::UNO_QUERY_THROW ) ) );
    return xSheetRange->Range( Cell1, Cell2 );
}

void
ScVbaWorksheet::CheckSpelling( const uno::Any& /*CustomDictionary*/,const uno::Any& /*IgnoreUppercase*/,const uno::Any& /*AlwaysSuggest*/, const uno::Any& /*SpellingLang*/ ) throw (uno::RuntimeException)
{
    // #TODO# #FIXME# unused params above, can we do anything with those
    rtl::OUString url = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:SpellDialog"));
    uno::Reference< frame::XModel > xModel( getModel() );
    dispatchRequests(xModel,url);
}

uno::Reference< excel::XRange >
ScVbaWorksheet::getSheetRange() throw (uno::RuntimeException)
{
    uno::Reference< table::XCellRange > xRange( getSheet(),uno::UNO_QUERY_THROW );
    return uno::Reference< excel::XRange >( new ScVbaRange( m_xContext, xRange ) );
}

// These are hacks - we prolly (somehow) need to inherit
// the vbarange functionality here ...
uno::Reference< excel::XRange >
ScVbaWorksheet::Cells( const ::uno::Any &nRow, const ::uno::Any &nCol )
        throw (uno::RuntimeException)
{
    return getSheetRange()->Cells( nRow, nCol );
}

uno::Reference< excel::XRange >
ScVbaWorksheet::Rows(const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    return getSheetRange()->Rows( aIndex );
}

uno::Reference< excel::XRange >
ScVbaWorksheet::Columns( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    return getSheetRange()->Columns( aIndex );
}

uno::Any SAL_CALL
ScVbaWorksheet::ChartObjects( const uno::Any& Index ) throw (uno::RuntimeException)
{
    if ( !mxCharts.is() )
    {
        uno::Reference< table::XTableChartsSupplier > xChartSupplier( getSheet(), uno::UNO_QUERY_THROW );
        uno::Reference< table::XTableCharts > xTableCharts = xChartSupplier->getCharts();

        mxCharts = new ScVbaChartObjects( m_xContext, xTableCharts );
    }
    if ( Index.hasValue() )
    {
        uno::Reference< vba::XCollection > xColl( mxCharts, uno::UNO_QUERY_THROW );
        return xColl->Item( Index );
    }
    else
        return makeAny( mxCharts );

}

uno::Any SAL_CALL
ScVbaWorksheet::PivotTables( const uno::Any& Index ) throw (uno::RuntimeException)
{
    uno::Reference< css::sheet::XSpreadsheet > xSheet = getSheet();
    uno::Reference< sheet::XDataPilotTablesSupplier > xTables(xSheet, uno::UNO_QUERY_THROW ) ;
    uno::Reference< container::XIndexAccess > xIndexAccess( xTables->getDataPilotTables(), uno::UNO_QUERY_THROW );

    uno::Reference< vba::XCollection > xColl( uno::Reference< excel::XPivotTables > ( new ScVbaPivotTables( m_xContext, xIndexAccess ) ), uno::UNO_QUERY_THROW );
    if ( Index.hasValue() )
        return xColl->Item( Index );
    return makeAny( xColl );
}

uno::Any SAL_CALL
ScVbaWorksheet::Comments( const uno::Any& Index ) throw (uno::RuntimeException)
{
    uno::Reference< css::sheet::XSpreadsheet > xSheet = getSheet();
    uno::Reference< sheet::XSheetAnnotationsSupplier > xAnnosSupp( xSheet, uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XSheetAnnotations > xAnnos( xAnnosSupp->getAnnotations(), uno::UNO_QUERY_THROW );
    uno::Reference< container::XIndexAccess > xIndexAccess( xAnnos, uno::UNO_QUERY_THROW );

    uno::Reference< vba::XCollection > xColl( uno::Reference< excel::XComments > ( new ScVbaComments( m_xContext, xIndexAccess ) ), uno::UNO_QUERY_THROW );
    if ( Index.hasValue() )
        return xColl->Item( Index );
    return makeAny( xColl );
}

uno::Any SAL_CALL
ScVbaWorksheet::Evaluate( const ::rtl::OUString& Name ) throw (uno::RuntimeException)
{
    // #TODO Evaluate allows other things to be evaluated, e.g. functions
    // I think ( like SIN(3) etc. ) need to investigate that
    // named Ranges also? e.g. [MyRange] if so need a list of named ranges
    uno::Any aVoid;
    return uno::Any( Range( uno::Any( Name ), aVoid ) );
}


uno::Reference< beans::XIntrospectionAccess > SAL_CALL
ScVbaWorksheet::getIntrospection(  ) throw (uno::RuntimeException)
{
    return uno::Reference< beans::XIntrospectionAccess >();
}

uno::Any SAL_CALL
ScVbaWorksheet::invoke( const ::rtl::OUString& aFunctionName, const uno::Sequence< uno::Any >& /*aParams*/, uno::Sequence< ::sal_Int16 >& /*aOutParamIndex*/, uno::Sequence< uno::Any >& /*aOutParam*/ ) throw (lang::IllegalArgumentException, script::CannotConvertException, reflection::InvocationTargetException, uno::RuntimeException)
{
    OSL_TRACE("** ScVbaWorksheet::invoke( %s ), will barf",
        rtl::OUStringToOString( aFunctionName, RTL_TEXTENCODING_UTF8 ).getStr() );

    throw uno::RuntimeException(); // unsupported operation
}

void SAL_CALL
ScVbaWorksheet::setValue( const ::rtl::OUString& /*aPropertyName*/, const uno::Any& /*aValue*/ ) throw (beans::UnknownPropertyException, script::CannotConvertException, reflection::InvocationTargetException, uno::RuntimeException)
{
    throw uno::RuntimeException(); // unsupported operation
}
uno::Any SAL_CALL
ScVbaWorksheet::getValue( const ::rtl::OUString& aPropertyName ) throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    uno::Any aProp = getControl( aPropertyName );
    if ( !aProp.hasValue() )
        throw beans::UnknownPropertyException(); // unsupported operation
    // #TODO we need a factory here when we support
    // more control types
    sal_Int32 nClassId = -1;
    uno::Reference< beans::XPropertySet > xProps( aProp, uno::UNO_QUERY_THROW );
    const static rtl::OUString sClassId( RTL_CONSTASCII_USTRINGPARAM("ClassId") );
    xProps->getPropertyValue( sClassId ) >>= nClassId;
    if ( nClassId == form::FormComponentType::COMBOBOX )
    {
        uno::Reference< msforms::XComboBox > xCbx( new ScVbaComboBox( m_xContext, xProps ) );
        return uno::makeAny( xCbx );
    }

    return aProp;
}

::sal_Bool SAL_CALL
ScVbaWorksheet::hasMethod( const ::rtl::OUString& /*aName*/ ) throw (uno::RuntimeException)
{
    return sal_False;
}
::sal_Bool SAL_CALL
ScVbaWorksheet::hasProperty( const ::rtl::OUString& aName ) throw (uno::RuntimeException)
{
    if ( getControl( aName ).hasValue() )
        return sal_True;
    return sal_False;
}
uno::Any
ScVbaWorksheet::getControl( const ::rtl::OUString& sName )
{
    uno::Reference< sheet::XScenarioEnhanced > xIf( getSheet(), uno::UNO_QUERY_THROW );
    ScTableSheetObj* pTab= static_cast< ScTableSheetObj* >( xIf.get() );
    ScDocShell* pShell = NULL;
    if ( pTab )
        pShell = pTab->GetDocShell();
    if ( pShell )
    {
        ScDrawLayer* pDrawLayer = pShell->MakeDrawLayer();
        SCTAB nTab = 0;
        // make GetTab_Impl() public or this class a friend
        const ScRangeList& rRanges = pTab->GetRangeList();
        const ScRange* pFirst = rRanges.GetObject(0);
        if (pFirst)
            nTab = pFirst->aStart.Tab();

        SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
        if ( pPage )
        {
            ULONG nCount = pPage->GetObjCount();
            for ( ULONG index=0; index<nCount; ++index )
            {
                SdrObject* pObj = pPage->GetObj( index );
                if ( pObj )
                {

                    SdrUnoObj* pUnoObj = PTR_CAST(SdrUnoObj, pObj);
                    if ( pUnoObj )
                    {
                        uno::Reference< container::XNamed > xNamed( pUnoObj->GetUnoControlModel(), uno::UNO_QUERY_THROW );
                        if ( sName.equals( xNamed->getName() ) )
                            return uno::makeAny( xNamed );
                    }
                }
            }
        }
    }

    return uno::Any();
}




