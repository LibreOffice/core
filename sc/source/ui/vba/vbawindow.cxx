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
#include <vbahelper/helperdecl.hxx>
#include "vbawindow.hxx"
#include "vbaworksheets.hxx"
#include "vbaworksheet.hxx"
#include "vbaglobals.hxx"
#include "vbapane.hxx"
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/view/DocumentZoomType.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <ooo/vba/excel/XlWindowState.hpp>
#include <ooo/vba/excel/XlWindowView.hpp>
#include <ooo/vba/excel/Constants.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/PosSize.hpp>

#include <docsh.hxx>
#include <tabvwsh.hxx>
#include <docuno.hxx>
#include <sc.hrc>
#include <boost/unordered_map.hpp>
#include <sfx2/viewfrm.hxx>
#include <vcl/wrkwin.hxx>
#include "unonames.hxx"
#include "markdata.hxx"

using namespace ::com::sun::star;
using namespace ::ooo::vba;
using namespace ::ooo::vba::excel::XlWindowState;

typedef  boost::unordered_map< rtl::OUString,
SCTAB, ::rtl::OUStringHash,
::std::equal_to< ::rtl::OUString > > NameIndexHash;

typedef std::vector< uno::Reference< sheet::XSpreadsheet > > Sheets;

typedef ::cppu::WeakImplHelper1< container::XEnumeration > Enumeration_BASE;

typedef ::cppu::WeakImplHelper3< container::XEnumerationAccess
    , com::sun::star::container::XIndexAccess
    , com::sun::star::container::XNameAccess
    > SelectedSheets_BASE;


class SelectedSheetsEnum : public Enumeration_BASE
{
public:
    uno::Reference< uno::XComponentContext > m_xContext;
    Sheets m_sheets;
    uno::Reference< frame::XModel > m_xModel;
    Sheets::const_iterator m_it;

    SelectedSheetsEnum( const uno::Reference< uno::XComponentContext >& xContext, const Sheets& sheets, const uno::Reference< frame::XModel >& xModel ) throw ( uno::RuntimeException ) :  m_xContext( xContext ), m_sheets( sheets ), m_xModel( xModel )
    {
        m_it = m_sheets.begin();
    }
    // XEnumeration
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
    {
        return m_it != m_sheets.end();
    }
    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( !hasMoreElements() )
        {
            throw container::NoSuchElementException();
        }
        // #FIXME needs ThisWorkbook as parent
        return uno::makeAny( uno::Reference< excel::XWorksheet > ( new ScVbaWorksheet( uno::Reference< XHelperInterface >(), m_xContext, *(m_it++), m_xModel ) ) );
    }


};

class SelectedSheetsEnumAccess : public SelectedSheets_BASE
{
    uno::Reference< uno::XComponentContext > m_xContext;
    NameIndexHash namesToIndices;
    Sheets sheets;
    uno::Reference< frame::XModel > m_xModel;
public:
    SelectedSheetsEnumAccess( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xModel ):m_xContext( xContext ), m_xModel( xModel )
    {
        ScModelObj* pModel = static_cast< ScModelObj* >( m_xModel.get() );
        if ( !pModel )
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Cannot obtain current document" ) ), uno::Reference< uno::XInterface >() );
        ScDocShell* pDocShell = (ScDocShell*)pModel->GetEmbeddedObject();
        if ( !pDocShell )
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Cannot obtain docshell" ) ), uno::Reference< uno::XInterface >() );
        ScTabViewShell* pViewShell = excel::getBestViewShell( m_xModel );
        if ( !pViewShell )
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Cannot obtain view shell" ) ), uno::Reference< uno::XInterface >() );

        SCTAB nTabCount = pDocShell->GetDocument()->GetTableCount();
        uno::Sequence<sal_Int32> aSheets( nTabCount );
        SCTAB nIndex = 0;
        const ScMarkData& rMarkData = pViewShell->GetViewData()->GetMarkData();
        sheets.reserve( nTabCount );
        uno::Reference <sheet::XSpreadsheetDocument> xSpreadSheet( m_xModel, uno::UNO_QUERY_THROW );
        uno::Reference <container::XIndexAccess> xIndex( xSpreadSheet->getSheets(), uno::UNO_QUERY_THROW );
        ScMarkData::const_iterator itr = rMarkData.begin(), itrEnd = rMarkData.end();
        for (; itr != itrEnd && *itr < nTabCount; ++itr)
        {
            uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex( *itr ), uno::UNO_QUERY_THROW );
            uno::Reference< container::XNamed > xNamed( xSheet, uno::UNO_QUERY_THROW );
            sheets.push_back( xSheet );
            namesToIndices[ xNamed->getName() ] = nIndex++;
        }

    }

    //XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) throw (uno::RuntimeException)
    {
        return new SelectedSheetsEnum( m_xContext, sheets, m_xModel  );
    }
    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException)
    {
        return sheets.size();
    }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw ( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( Index < 0
        || static_cast< Sheets::size_type >( Index ) >= sheets.size() )
            throw lang::IndexOutOfBoundsException();

        return uno::makeAny( sheets[ Index ] );
    }

    //XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException)
    {
        return excel::XWorksheet::static_type(0);
    }

    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException)
    {
        return ( !sheets.empty() );
    }

    //XNameAccess
    virtual uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        NameIndexHash::const_iterator it = namesToIndices.find( aName );
        if ( it == namesToIndices.end() )
            throw container::NoSuchElementException();
        return uno::makeAny( sheets[ it->second ] );

    }

    virtual uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (uno::RuntimeException)
    {
        uno::Sequence< ::rtl::OUString > names( namesToIndices.size() );
        ::rtl::OUString* pString = names.getArray();
        NameIndexHash::const_iterator it = namesToIndices.begin();
        NameIndexHash::const_iterator it_end = namesToIndices.end();
        for ( ; it != it_end; ++it, ++pString )
            *pString = it->first;
        return names;
    }

    virtual ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (uno::RuntimeException)
    {
        NameIndexHash::const_iterator it = namesToIndices.find( aName );
        return (it != namesToIndices.end());
    }


};

ScVbaWindow::ScVbaWindow(
        const uno::Reference< XHelperInterface >& xParent,
        const uno::Reference< uno::XComponentContext >& xContext,
        const uno::Reference< frame::XModel >& xModel,
        const uno::Reference< frame::XController >& xController ) throw (uno::RuntimeException) :
    WindowImpl_BASE( xParent, xContext, xModel, xController )
{
    init();
}

ScVbaWindow::ScVbaWindow(
        const uno::Sequence< uno::Any >& args,
        const uno::Reference< uno::XComponentContext >& xContext ) throw (uno::RuntimeException) :
    WindowImpl_BASE( args, xContext )
{
    init();
}

void
ScVbaWindow::init()
{
    /*  This method is called from the constructor, thus the own refcount is
        still zero. The implementation of ActivePane() uses a UNO reference of
        this (to set this window as parent of the pane obejct). This requires
        the own refcount to be non-zero, otherwise this instance will be
        desctructed immediately! Guard the call to ActivePane() in try/catch to
        not miss the decrementation of the reference count on exception. */
    osl_incrementInterlockedCount( &m_refCount );
    try
    {
       m_xPane = ActivePane();
    }
    catch( uno::Exception& )
    {
    }
    osl_decrementInterlockedCount( &m_refCount );
}

uno::Reference< beans::XPropertySet >
ScVbaWindow::getControllerProps() throw (uno::RuntimeException)
{
    return uno::Reference< beans::XPropertySet >( getController(), uno::UNO_QUERY_THROW );
}

uno::Reference< beans::XPropertySet >
ScVbaWindow::getFrameProps() throw (uno::RuntimeException)
{
    return uno::Reference< beans::XPropertySet >( getController()->getFrame(), uno::UNO_QUERY_THROW );
}

uno::Reference< awt::XDevice >
ScVbaWindow::getDevice() throw (uno::RuntimeException)
{
    return uno::Reference< awt::XDevice >( getWindow(), uno::UNO_QUERY_THROW );
}

void
ScVbaWindow::Scroll( const uno::Any& Down, const uno::Any& Up, const uno::Any& ToRight, const uno::Any& ToLeft, bool bLargeScroll ) throw (uno::RuntimeException)
{
    if( !m_xPane.is() )
        throw uno::RuntimeException();
    if( bLargeScroll )
        m_xPane->LargeScroll( Down, Up, ToRight, ToLeft );
    else
        m_xPane->SmallScroll( Down, Up, ToRight, ToLeft );
}

void SAL_CALL
ScVbaWindow::SmallScroll( const uno::Any& Down, const uno::Any& Up, const uno::Any& ToRight, const uno::Any& ToLeft ) throw (uno::RuntimeException)
{
    Scroll( Down, Up, ToRight, ToLeft );
}

void SAL_CALL
ScVbaWindow::LargeScroll( const uno::Any& Down, const uno::Any& Up, const uno::Any& ToRight, const uno::Any& ToLeft ) throw (uno::RuntimeException)
{
    Scroll( Down, Up, ToRight, ToLeft, true );
}

uno::Any SAL_CALL
ScVbaWindow::SelectedSheets( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( new SelectedSheetsEnumAccess( mxContext, m_xModel ) );
    // #FIXME needs a workbook as a parent
    uno::Reference< excel::XWorksheets > xSheets(  new ScVbaWorksheets( uno::Reference< XHelperInterface >(), mxContext, xEnumAccess, m_xModel ) );
    if ( aIndex.hasValue() )
    {
        uno::Reference< XCollection > xColl( xSheets, uno::UNO_QUERY_THROW );
        return xColl->Item( aIndex, uno::Any() );
    }
    return uno::makeAny( xSheets );
}

void SAL_CALL
ScVbaWindow::ScrollWorkbookTabs( const uno::Any& /*Sheets*/, const uno::Any& /*Position*/ ) throw (uno::RuntimeException)
{
// #TODO #FIXME need some implementation to scroll through the tabs
// but where is this done?
/*
    sal_Int32 nSheets = 0;
    sal_Int32 nPosition = 0;
    throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("No Implemented")), uno::Reference< uno::XInterface >() );
    sal_Bool bSheets = ( Sheets >>= nSheets );
    sal_Bool bPosition = ( Position >>= nPosition );
    if ( bSheets || bPosition ) // at least one param specified
        if ( bSheets )
            ;// use sheets
        else if ( bPosition )
            ; //use position
*/

}

uno::Any SAL_CALL
ScVbaWindow::getCaption() throw (uno::RuntimeException)
{
    static rtl::OUString sCrud(RTL_CONSTASCII_USTRINGPARAM(" - OpenOffice.org Calc" ) );
    static sal_Int32 nCrudLen = sCrud.getLength();

    rtl::OUString sTitle;
    getFrameProps()->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SC_UNONAME_TITLE ) ) ) >>= sTitle;
    sal_Int32 nCrudIndex = sTitle.indexOf( sCrud );
    // adjust title ( by removing crud )
    // sCrud string present
    if ( nCrudIndex != -1 )
    {
        // and ends with sCrud
        if ( ( nCrudLen + nCrudIndex ) == sTitle.getLength() )
        {
            sTitle = sTitle.copy( 0, nCrudIndex );
            ScVbaWorkbook workbook( uno::Reference< XHelperInterface >( Application(), uno::UNO_QUERY_THROW ), mxContext, m_xModel );
            rtl::OUString sName = workbook.getName();
            // rather bizare hack to make sure the name behavior
            // is like XL
            // if the adjusted title == workbook name, use name
            // if the adjusted title != workbook name but ...
            //  name == title + extension ( .csv, ,odt, .xls )
            //  etc. then also use the name

            if ( !sTitle.equals( sName ) )
            {
                static rtl::OUString sDot( RTL_CONSTASCII_USTRINGPARAM(".") );
                // starts with title
                if ( sName.indexOf( sTitle ) == 0 )
                    // extention starts immediately after
                    if ( sName.match( sDot, sTitle.getLength() ) )
                        sTitle = sName;
            }
        }
    }
    return uno::makeAny( sTitle );
}

void SAL_CALL
ScVbaWindow::setCaption( const uno::Any& _caption ) throw (uno::RuntimeException)
{
    getFrameProps()->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_TITLE ) ), _caption );
}

uno::Any SAL_CALL
ScVbaWindow::getScrollRow() throw (uno::RuntimeException)
{
    sal_Int32 nValue = 0;
    // !! TODO !! get view shell from controller
    ScTabViewShell* pViewShell = excel::getBestViewShell( m_xModel );
    if ( pViewShell )
    {
        ScSplitPos eWhich = pViewShell->GetViewData()->GetActivePart();
        nValue = pViewShell->GetViewData()->GetPosY(WhichV(eWhich));
    }

    return uno::makeAny( nValue + 1);
}

void SAL_CALL
ScVbaWindow::setScrollRow( const uno::Any& _scrollrow ) throw (uno::RuntimeException)
{
    // !! TODO !! get view shell from controller
    ScTabViewShell* pViewShell = excel::getBestViewShell( m_xModel );
    if ( pViewShell )
    {
        sal_Int32 scrollRow = 0;
        _scrollrow >>= scrollRow;
        ScSplitPos eWhich = pViewShell->GetViewData()->GetActivePart();
        sal_Int32 nOldValue = pViewShell->GetViewData()->GetPosY(WhichV(eWhich)) + 1;
        pViewShell->ScrollLines(0, scrollRow - nOldValue);
    }
}

uno::Any SAL_CALL
ScVbaWindow::getScrollColumn() throw (uno::RuntimeException)
{
    sal_Int32 nValue = 0;
    // !! TODO !! get view shell from controller
    ScTabViewShell* pViewShell = excel::getBestViewShell( m_xModel );
    if ( pViewShell )
    {
        ScSplitPos eWhich = pViewShell->GetViewData()->GetActivePart();
        nValue = pViewShell->GetViewData()->GetPosX(WhichH(eWhich));
    }

    return uno::makeAny( nValue + 1);
}

void SAL_CALL
ScVbaWindow::setScrollColumn( const uno::Any& _scrollcolumn ) throw (uno::RuntimeException)
{
    // !! TODO !! get view shell from controller
    ScTabViewShell* pViewShell = excel::getBestViewShell( m_xModel );
    if ( pViewShell )
    {
        sal_Int32 scrollColumn = 0;
        _scrollcolumn >>= scrollColumn;
        ScSplitPos eWhich = pViewShell->GetViewData()->GetActivePart();
        sal_Int32 nOldValue = pViewShell->GetViewData()->GetPosX(WhichH(eWhich)) + 1;
        pViewShell->ScrollLines(scrollColumn - nOldValue, 0);
    }
}

uno::Any SAL_CALL
ScVbaWindow::getWindowState() throw (uno::RuntimeException)
{
    sal_Int32 nwindowState = xlNormal;
    // !! TODO !! get view shell from controller
    ScTabViewShell* pViewShell = excel::getBestViewShell( m_xModel );
    SfxViewFrame* pViewFrame = pViewShell -> GetViewFrame();
    WorkWindow* pWork = (WorkWindow*) pViewFrame->GetFrame().GetSystemWindow();
    if ( pWork )
    {
        if ( pWork -> IsMaximized())
            nwindowState = xlMaximized;
        else if (pWork -> IsMinimized())
            nwindowState = xlMinimized;
    }
    return uno::makeAny( nwindowState );
}

void SAL_CALL
ScVbaWindow::setWindowState( const uno::Any& _windowstate ) throw (uno::RuntimeException)
{
    sal_Int32 nwindowState = xlMaximized;
    _windowstate >>= nwindowState;
    // !! TODO !! get view shell from controller
    ScTabViewShell* pViewShell = excel::getBestViewShell( m_xModel );
    SfxViewFrame* pViewFrame = pViewShell -> GetViewFrame();
    WorkWindow* pWork = (WorkWindow*) pViewFrame->GetFrame().GetSystemWindow();
    if ( pWork )
    {
        if ( nwindowState == xlMaximized)
            pWork -> Maximize();
        else if (nwindowState == xlMinimized)
            pWork -> Minimize();
        else if (nwindowState == xlNormal)
            pWork -> Restore();
        else
            throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Invalid Parameter" ) ), uno::Reference< uno::XInterface >() );
    }
}

void
ScVbaWindow::Activate() throw (css::uno::RuntimeException)
{
    ScVbaWorkbook workbook( uno::Reference< XHelperInterface >( Application(), uno::UNO_QUERY_THROW ), mxContext, m_xModel );

    workbook.Activate();
}

void
ScVbaWindow::Close( const uno::Any& SaveChanges, const uno::Any& FileName, const uno::Any& RouteWorkBook ) throw (uno::RuntimeException)
{
    ScVbaWorkbook workbook( uno::Reference< XHelperInterface >( Application(), uno::UNO_QUERY_THROW ), mxContext, m_xModel );
    workbook.Close(SaveChanges, FileName, RouteWorkBook );
}

uno::Reference< excel::XPane > SAL_CALL
ScVbaWindow::ActivePane() throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Reference< sheet::XViewPane > xViewPane( getController(), uno::UNO_QUERY_THROW );
    return new ScVbaPane( this, mxContext, m_xModel, xViewPane );
}

uno::Reference< excel::XRange > SAL_CALL
ScVbaWindow::ActiveCell(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Reference< excel::XApplication > xApplication( Application(), uno::UNO_QUERY_THROW );
    return xApplication->getActiveCell();
}

uno::Any SAL_CALL
ScVbaWindow::Selection(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Reference< excel::XApplication > xApplication( Application(), uno::UNO_QUERY_THROW );
    return xApplication->getSelection();
}

uno::Reference< excel::XRange > SAL_CALL
ScVbaWindow::RangeSelection() throw (script::BasicErrorException, uno::RuntimeException)
{
    /*  TODO / FIXME: According to documentation, this method returns the range
        selection even if shapes are selected. */
    return uno::Reference< excel::XRange >( Selection(), uno::UNO_QUERY_THROW );
}

::sal_Bool SAL_CALL
ScVbaWindow::getDisplayGridlines() throw (uno::RuntimeException)
{
    rtl::OUString sName( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_SHOWGRID ) );
    sal_Bool bGrid = sal_True;
    getControllerProps()->getPropertyValue( sName ) >>= bGrid;
    return bGrid;
}


void SAL_CALL
ScVbaWindow::setDisplayGridlines( ::sal_Bool _displaygridlines ) throw (uno::RuntimeException)
{
    rtl::OUString sName( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_SHOWGRID ) );
    getControllerProps()->setPropertyValue( sName, uno::makeAny( _displaygridlines ));
}

::sal_Bool SAL_CALL
ScVbaWindow::getDisplayHeadings() throw (uno::RuntimeException)
{
    rtl::OUString sName( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_COLROWHDR ) );
    sal_Bool bHeading = sal_True;
    getControllerProps()->getPropertyValue( sName ) >>= bHeading;
    return bHeading;
}

void SAL_CALL
ScVbaWindow::setDisplayHeadings( ::sal_Bool _bDisplayHeadings ) throw (uno::RuntimeException)
{
    rtl::OUString sName( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_COLROWHDR ) );
    getControllerProps()->setPropertyValue( sName, uno::makeAny( _bDisplayHeadings ));
}

::sal_Bool SAL_CALL
ScVbaWindow::getDisplayHorizontalScrollBar() throw (uno::RuntimeException)
{
    rtl::OUString sName( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_HORSCROLL ) );
    sal_Bool bHorizontalScrollBar = sal_True;
    getControllerProps()->getPropertyValue( sName ) >>= bHorizontalScrollBar;
    return bHorizontalScrollBar;
}

void SAL_CALL
ScVbaWindow::setDisplayHorizontalScrollBar( ::sal_Bool _bDisplayHorizontalScrollBar ) throw (uno::RuntimeException)
{
    rtl::OUString sName( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_HORSCROLL ) );
    getControllerProps()->setPropertyValue( sName, uno::makeAny( _bDisplayHorizontalScrollBar ));
}

::sal_Bool SAL_CALL
ScVbaWindow::getDisplayOutline() throw (uno::RuntimeException)
{
    rtl::OUString sName( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_OUTLSYMB ) );
    sal_Bool bOutline = sal_True;
    getControllerProps()->getPropertyValue( sName ) >>= bOutline;
    return bOutline;
}

void SAL_CALL
ScVbaWindow::setDisplayOutline( ::sal_Bool _bDisplayOutline ) throw (uno::RuntimeException)
{
    rtl::OUString sName( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_OUTLSYMB ) );
    getControllerProps()->setPropertyValue( sName, uno::makeAny( _bDisplayOutline ));
}

::sal_Bool SAL_CALL
ScVbaWindow::getDisplayVerticalScrollBar() throw (uno::RuntimeException)
{
    rtl::OUString sName( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_VERTSCROLL ) );
    sal_Bool bVerticalScrollBar = sal_True;
    getControllerProps()->getPropertyValue( sName ) >>= bVerticalScrollBar;
    return bVerticalScrollBar;
}

void SAL_CALL
ScVbaWindow::setDisplayVerticalScrollBar( ::sal_Bool _bDisplayVerticalScrollBar ) throw (uno::RuntimeException)
{
    rtl::OUString sName( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_VERTSCROLL ) );
    getControllerProps()->setPropertyValue( sName, uno::makeAny( _bDisplayVerticalScrollBar ));
}

::sal_Bool SAL_CALL
ScVbaWindow::getDisplayWorkbookTabs() throw (uno::RuntimeException)
{
    rtl::OUString sName( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_SHEETTABS ) );
    sal_Bool bWorkbookTabs = sal_True;
    getControllerProps()->getPropertyValue( sName ) >>= bWorkbookTabs;
    return bWorkbookTabs;
}

void SAL_CALL
ScVbaWindow::setDisplayWorkbookTabs( ::sal_Bool _bDisplayWorkbookTabs ) throw (uno::RuntimeException)
{
    rtl::OUString sName( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_SHEETTABS ) );
    getControllerProps()->setPropertyValue( sName, uno::makeAny( _bDisplayWorkbookTabs ));
}

::sal_Bool SAL_CALL
ScVbaWindow::getFreezePanes() throw (uno::RuntimeException)
{
    uno::Reference< sheet::XViewFreezable > xViewFreezable( getController(), uno::UNO_QUERY_THROW );
    return xViewFreezable->hasFrozenPanes();
}

void SAL_CALL
ScVbaWindow::setFreezePanes( ::sal_Bool /*_bFreezePanes*/ ) throw (uno::RuntimeException)
{
    uno::Reference< sheet::XViewPane > xViewPane( getController(), uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XViewSplitable > xViewSplitable( xViewPane, uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XViewFreezable > xViewFreezable( xViewPane, uno::UNO_QUERY_THROW );
    if( xViewSplitable->getIsWindowSplit() )
    {
        // if there is a split we freeze at the split
        sal_Int32 nColumn = getSplitColumn();
        sal_Int32 nRow = getSplitRow();
        xViewFreezable->freezeAtPosition( nColumn, nRow );
    }
    else
    {
        // otherwise we freeze in the center of the visible sheet
        table::CellRangeAddress aCellRangeAddress = xViewPane->getVisibleRange();
        sal_Int32 nColumn = aCellRangeAddress.StartColumn + (( aCellRangeAddress.EndColumn - aCellRangeAddress.StartColumn )/2 );
        sal_Int32 nRow = aCellRangeAddress.StartRow + (( aCellRangeAddress.EndRow - aCellRangeAddress.StartRow )/2 );
        xViewFreezable->freezeAtPosition( nColumn, nRow );
    }
}

::sal_Bool SAL_CALL
ScVbaWindow::getSplit() throw (uno::RuntimeException)
{
    uno::Reference< sheet::XViewSplitable > xViewSplitable( getController(), uno::UNO_QUERY_THROW );
    return xViewSplitable->getIsWindowSplit();
}

void SAL_CALL
ScVbaWindow::setSplit( ::sal_Bool _bSplit ) throw (uno::RuntimeException)
{
    if( !_bSplit )
    {
        uno::Reference< sheet::XViewSplitable > xViewSplitable( getController(), uno::UNO_QUERY_THROW );
        xViewSplitable->splitAtPosition(0,0);
    }
    else
    {
        uno::Reference< sheet::XViewFreezable > xViewFreezable( getController(), uno::UNO_QUERY_THROW );
        uno::Reference< excel::XRange > xRange = ActiveCell();
        sal_Int32 nRow = xRange->getRow();
        sal_Int32 nColumn = xRange->getColumn();
        xViewFreezable->freezeAtPosition( nColumn-1, nRow-1 );
        SplitAtDefinedPosition( sal_True );
    }
}

sal_Int32 SAL_CALL
ScVbaWindow::getSplitColumn() throw (uno::RuntimeException)
{
    uno::Reference< sheet::XViewSplitable > xViewSplitable( getController(), uno::UNO_QUERY_THROW );
    return xViewSplitable->getSplitColumn();
}

void SAL_CALL
ScVbaWindow::setSplitColumn( sal_Int32 _splitcolumn ) throw (uno::RuntimeException)
{
    if( getSplitColumn() != _splitcolumn )
    {
        uno::Reference< sheet::XViewFreezable > xViewFreezable( getController(), uno::UNO_QUERY_THROW );
        sal_Bool bFrozen = getFreezePanes();
        sal_Int32 nRow = getSplitRow();
        xViewFreezable->freezeAtPosition( _splitcolumn, nRow );
        SplitAtDefinedPosition( !bFrozen );
    }
}

double SAL_CALL
ScVbaWindow::getSplitHorizontal() throw (uno::RuntimeException)
{
    uno::Reference< sheet::XViewSplitable > xViewSplitable( getController(), uno::UNO_QUERY_THROW );
    return PixelsToPoints( getDevice(), xViewSplitable->getSplitHorizontal(), sal_True );
}

void SAL_CALL
ScVbaWindow::setSplitHorizontal( double _splithorizontal ) throw (uno::RuntimeException)
{
    uno::Reference< sheet::XViewSplitable > xViewSplitable( getController(), uno::UNO_QUERY_THROW );
    double fHoriPixels = PointsToPixels( getDevice(), _splithorizontal, sal_True );
    xViewSplitable->splitAtPosition( static_cast< sal_Int32 >( fHoriPixels ), 0 );
}

sal_Int32 SAL_CALL
ScVbaWindow::getSplitRow() throw (uno::RuntimeException)
{
    uno::Reference< sheet::XViewSplitable > xViewSplitable( getController(), uno::UNO_QUERY_THROW );
    sal_Int32 nValue = xViewSplitable->getSplitRow();
    return nValue ? nValue - 1 : nValue;
}

void SAL_CALL
ScVbaWindow::setSplitRow( sal_Int32 _splitrow ) throw (uno::RuntimeException)
{
    if( getSplitRow() != _splitrow )
    {
        uno::Reference< sheet::XViewFreezable > xViewFreezable( getController(), uno::UNO_QUERY_THROW );
        sal_Bool bFrozen = getFreezePanes();
        sal_Int32 nColumn = getSplitColumn();
        xViewFreezable->freezeAtPosition( nColumn , _splitrow );
        SplitAtDefinedPosition( !bFrozen );
    }
}

double SAL_CALL
ScVbaWindow::getSplitVertical() throw (uno::RuntimeException)
{
    uno::Reference< sheet::XViewSplitable > xViewSplitable( getController(), uno::UNO_QUERY_THROW );
    return PixelsToPoints( getDevice(), xViewSplitable->getSplitVertical(), sal_False );
}

void SAL_CALL
ScVbaWindow::setSplitVertical(double _splitvertical ) throw (uno::RuntimeException)
{
    uno::Reference< sheet::XViewSplitable > xViewSplitable( getController(), uno::UNO_QUERY_THROW );
    double fVertiPixels = PointsToPixels( getDevice(), _splitvertical, sal_False );
    xViewSplitable->splitAtPosition( 0, static_cast<sal_Int32>( fVertiPixels ) );
}

void ScVbaWindow::SplitAtDefinedPosition(sal_Bool _bUnFreezePane)
{
    uno::Reference< sheet::XViewSplitable > xViewSplitable( getController(), uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XViewFreezable > xViewFreezable( xViewSplitable, uno::UNO_QUERY_THROW );
    sal_Int32 nVertSplit = xViewSplitable->getSplitVertical();
    sal_Int32 nHoriSplit = xViewSplitable->getSplitHorizontal();
    if( _bUnFreezePane )
        xViewFreezable->freezeAtPosition(0,0);
    xViewSplitable->splitAtPosition(nHoriSplit, nVertSplit);
}

uno::Any SAL_CALL
ScVbaWindow::getZoom() throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps = getControllerProps();
    rtl::OUString sName( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_ZOOMTYPE ) );
    sal_Int16 nZoomType = view::DocumentZoomType::PAGE_WIDTH;
    xProps->getPropertyValue( sName ) >>= nZoomType;
    if( nZoomType == view::DocumentZoomType::PAGE_WIDTH )
    {
        return uno::makeAny( sal_True );
    }
    else if( nZoomType == view::DocumentZoomType::BY_VALUE )
    {
        sName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ZOOMVALUE));
        sal_Int16 nZoom = 100;
        xProps->getPropertyValue( sName ) >>= nZoom;
        return uno::makeAny( nZoom );
    }
    return uno::Any();
}

void SAL_CALL
ScVbaWindow::setZoom( const uno::Any& _zoom ) throw (uno::RuntimeException)
{
    sal_Int16 nZoom = 100;
    _zoom >>= nZoom;
    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( m_xModel, uno::UNO_QUERY_THROW );
    uno::Reference< excel::XWorksheet > xActiveSheet = ActiveSheet();
    SCTAB nTab = 0;
    if ( !ScVbaWorksheets::nameExists (xSpreadDoc, xActiveSheet->getName(), nTab) )
        throw uno::RuntimeException();
    std::vector< SCTAB > vTabs;
    vTabs.push_back( nTab );
    excel::implSetZoom( m_xModel, nZoom, vTabs );
}

uno::Reference< excel::XWorksheet > SAL_CALL
ScVbaWindow::ActiveSheet(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Reference< excel::XApplication > xApplication( Application(), uno::UNO_QUERY_THROW );
    return xApplication->getActiveSheet();
}

uno::Any SAL_CALL
ScVbaWindow::getView() throw (uno::RuntimeException)
{
    // not supported now
    sal_Int32 nWindowView = excel::XlWindowView::xlNormalView;
    return uno::makeAny( nWindowView );
}

void SAL_CALL
ScVbaWindow::setView( const uno::Any& _view) throw (uno::RuntimeException)
{
    sal_Int32 nWindowView = excel::XlWindowView::xlNormalView;
    _view >>= nWindowView;
    sal_uInt16 nSlot = FID_NORMALVIEWMODE;
    switch ( nWindowView )
    {
        case excel::XlWindowView::xlNormalView:
            nSlot = FID_NORMALVIEWMODE;
            break;
        case excel::XlWindowView::xlPageBreakPreview:
            nSlot = FID_PAGEBREAKMODE;
            break;
        default:
            DebugHelper::exception(SbERR_BAD_PARAMETER, rtl::OUString() );
    }
    // !! TODO !! get view shell from controller
    ScTabViewShell* pViewShell = excel::getBestViewShell( m_xModel );
    if ( pViewShell )
        dispatchExecute( pViewShell, nSlot );
}

uno::Reference< excel::XRange > SAL_CALL
ScVbaWindow::getVisibleRange() throw (uno::RuntimeException)
{
    uno::Reference< container::XIndexAccess > xPanesIA( getController(), uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XViewPane > xTopLeftPane( xPanesIA->getByIndex( 0 ), uno::UNO_QUERY_THROW );
    uno::Reference< excel::XPane > xPane( new ScVbaPane( this, mxContext, m_xModel, xTopLeftPane ) );
    return xPane->getVisibleRange();
}

sal_Int32 SAL_CALL
ScVbaWindow::PointsToScreenPixelsX(sal_Int32 _points) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    sal_Int32 nHundredthsofOneMillimeters = Millimeter::getInHundredthsOfOneMillimeter( _points );
    double fConvertFactor = (getDevice()->getInfo().PixelPerMeterX/100000);
    return static_cast<sal_Int32>(fConvertFactor * nHundredthsofOneMillimeters );
}

sal_Int32 SAL_CALL
ScVbaWindow::PointsToScreenPixelsY(sal_Int32 _points) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    sal_Int32 nHundredthsofOneMillimeters = Millimeter::getInHundredthsOfOneMillimeter( _points );
    double fConvertFactor = (getDevice()->getInfo().PixelPerMeterY/100000);
    return static_cast<sal_Int32>(fConvertFactor * nHundredthsofOneMillimeters );
}

void SAL_CALL
ScVbaWindow::PrintOut( const css::uno::Any& From, const css::uno::Any&To, const css::uno::Any& Copies, const css::uno::Any& Preview, const css::uno::Any& ActivePrinter, const css::uno::Any& PrintToFile, const css::uno::Any& Collate, const css::uno::Any& PrToFileName ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    // need test, print current active sheet
    // !! TODO !! get view shell from controller
    PrintOutHelper( excel::getBestViewShell( m_xModel ), From, To, Copies, Preview, ActivePrinter, PrintToFile, Collate, PrToFileName, sal_True );
}

void SAL_CALL
ScVbaWindow::PrintPreview( const css::uno::Any& EnableChanges ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    // need test, print preview current active sheet
    // !! TODO !! get view shell from controller
    PrintPreviewHelper( EnableChanges, excel::getBestViewShell( m_xModel ) );
}

double SAL_CALL ScVbaWindow::getTabRatio() throw (css::uno::RuntimeException)
{
    ScTabViewShell* pViewShell = excel::getBestViewShell( m_xModel );
    if ( pViewShell && pViewShell->GetViewData() && pViewShell->GetViewData()->GetView() )
    {
        double fRatio = pViewShell->GetViewData()->GetView()->GetRelTabBarWidth();
        if ( fRatio >= 0.0 && fRatio <= 1.0 )
            return fRatio;
    }
    return 0.0;
}

void SAL_CALL ScVbaWindow::setTabRatio( double fRatio ) throw (css::uno::RuntimeException)
{
    ScTabViewShell* pViewShell = excel::getBestViewShell( m_xModel );
    if ( pViewShell && pViewShell->GetViewData() && pViewShell->GetViewData()->GetView() )
    {
        if ( fRatio >= 0.0 && fRatio <= 1.0 )
            pViewShell->GetViewData()->GetView()->SetRelTabBarWidth( fRatio );
    }
}

rtl::OUString
ScVbaWindow::getServiceImplName()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScVbaWindow"));
}

uno::Sequence< rtl::OUString >
ScVbaWindow::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.Window" ) );
    }
    return aServiceNames;
}
namespace window
{
namespace sdecl = comphelper::service_decl;
sdecl::vba_service_class_<ScVbaWindow, sdecl::with_args<true> > serviceImpl;
extern sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "ScVbaWindow",
    "ooo.vba.excel.Window" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
