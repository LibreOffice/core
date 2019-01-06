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
#include <vbahelper/helperdecl.hxx>
#include "service.hxx"
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
#include <basic/sberrors.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/implbase.hxx>

#include <docsh.hxx>
#include <tabvwsh.hxx>
#include <docuno.hxx>
#include <sc.hrc>
#include <sfx2/viewfrm.hxx>
#include <vcl/wrkwin.hxx>
#include <unonames.hxx>
#include <markdata.hxx>
#include <unordered_map>

using namespace ::com::sun::star;
using namespace ::ooo::vba;
using namespace ::ooo::vba::excel::XlWindowState;

typedef  std::unordered_map< OUString,
SCTAB > NameIndexHash;

typedef std::vector< uno::Reference< sheet::XSpreadsheet > > Sheets;

typedef ::cppu::WeakImplHelper< container::XEnumerationAccess
    , css::container::XIndexAccess
    , css::container::XNameAccess
    > SelectedSheets_BASE;

class SelectedSheetsEnum : public ::cppu::WeakImplHelper< container::XEnumeration >
{
public:
    uno::Reference< uno::XComponentContext > m_xContext;
    Sheets m_sheets;
    uno::Reference< frame::XModel > m_xModel;
    Sheets::const_iterator m_it;

    /// @throws uno::RuntimeException
    SelectedSheetsEnum( const uno::Reference< uno::XComponentContext >& xContext, const Sheets& sheets, const uno::Reference< frame::XModel >& xModel ) :  m_xContext( xContext ), m_sheets( sheets ), m_xModel( xModel )
    {
        m_it = m_sheets.begin();
    }
    // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override
    {
        return m_it != m_sheets.end();
    }
    virtual uno::Any SAL_CALL nextElement(  ) override
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
            throw uno::RuntimeException("Cannot obtain current document" );
        ScDocShell* pDocShell = static_cast<ScDocShell*>(pModel->GetEmbeddedObject());
        if ( !pDocShell )
            throw uno::RuntimeException("Cannot obtain docshell" );
        ScTabViewShell* pViewShell = excel::getBestViewShell( m_xModel );
        if ( !pViewShell )
            throw uno::RuntimeException("Cannot obtain view shell" );

        SCTAB nTabCount = pDocShell->GetDocument().GetTableCount();
        SCTAB nIndex = 0;
        const ScMarkData& rMarkData = pViewShell->GetViewData().GetMarkData();
        sheets.reserve( nTabCount );
        uno::Reference <sheet::XSpreadsheetDocument> xSpreadSheet( m_xModel, uno::UNO_QUERY_THROW );
        uno::Reference <container::XIndexAccess> xIndex( xSpreadSheet->getSheets(), uno::UNO_QUERY_THROW );
        for (const auto& rTab : rMarkData)
        {
            if (rTab >= nTabCount)
                break;
            uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex( rTab ), uno::UNO_QUERY_THROW );
            uno::Reference< container::XNamed > xNamed( xSheet, uno::UNO_QUERY_THROW );
            sheets.push_back( xSheet );
            namesToIndices[ xNamed->getName() ] = nIndex++;
        }

    }

    //XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) override
    {
        return new SelectedSheetsEnum( m_xContext, sheets, m_xModel  );
    }
    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) override
    {
        return sheets.size();
    }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) override
    {
        if ( Index < 0
        || static_cast< Sheets::size_type >( Index ) >= sheets.size() )
            throw lang::IndexOutOfBoundsException();

        return uno::makeAny( sheets[ Index ] );
    }

    //XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) override
    {
        return cppu::UnoType<excel::XWorksheet>::get();
    }

    virtual sal_Bool SAL_CALL hasElements(  ) override
    {
        return ( !sheets.empty() );
    }

    //XNameAccess
    virtual uno::Any SAL_CALL getByName( const OUString& aName ) override
    {
        NameIndexHash::const_iterator it = namesToIndices.find( aName );
        if ( it == namesToIndices.end() )
            throw container::NoSuchElementException();
        return uno::makeAny( sheets[ it->second ] );

    }

    virtual uno::Sequence< OUString > SAL_CALL getElementNames(  ) override
    {
        return comphelper::mapKeysToSequence( namesToIndices );
    }

    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override
    {
        NameIndexHash::const_iterator it = namesToIndices.find( aName );
        return (it != namesToIndices.end());
    }

};

ScVbaWindow::ScVbaWindow(
        const uno::Reference< XHelperInterface >& xParent,
        const uno::Reference< uno::XComponentContext >& xContext,
        const uno::Reference< frame::XModel >& xModel,
        const uno::Reference< frame::XController >& xController ) :
    WindowImpl_BASE( xParent, xContext, xModel, xController )
{
    init();
}

ScVbaWindow::ScVbaWindow(
        const uno::Sequence< uno::Any >& args,
        const uno::Reference< uno::XComponentContext >& xContext ) :
    WindowImpl_BASE( args, xContext )
{
    init();
}

void
ScVbaWindow::init()
{
    /*  This method is called from the constructor, thus the own refcount is
        still zero. The implementation of ActivePane() uses a UNO reference of
        this (to set this window as parent of the pane object). This requires
        the own refcount to be non-zero, otherwise this instance will be
        destructed immediately! Guard the call to ActivePane() in try/catch to
        not miss the decrementation of the reference count on exception. */
    osl_atomic_increment( &m_refCount );
    try
    {
       m_xPane = ActivePane();
    }
    catch( uno::Exception& )
    {
    }
    osl_atomic_decrement( &m_refCount );
}

uno::Reference< beans::XPropertySet >
ScVbaWindow::getControllerProps()
{
    return uno::Reference< beans::XPropertySet >( getController(), uno::UNO_QUERY_THROW );
}

uno::Reference< beans::XPropertySet >
ScVbaWindow::getFrameProps()
{
    return uno::Reference< beans::XPropertySet >( getController()->getFrame(), uno::UNO_QUERY_THROW );
}

uno::Reference< awt::XDevice >
ScVbaWindow::getDevice()
{
    return uno::Reference< awt::XDevice >( getWindow(), uno::UNO_QUERY_THROW );
}

void
ScVbaWindow::Scroll( const uno::Any& Down, const uno::Any& Up, const uno::Any& ToRight, const uno::Any& ToLeft, bool bLargeScroll )
{
    if( !m_xPane.is() )
        throw uno::RuntimeException();
    if( bLargeScroll )
        m_xPane->LargeScroll( Down, Up, ToRight, ToLeft );
    else
        m_xPane->SmallScroll( Down, Up, ToRight, ToLeft );
}

void SAL_CALL
ScVbaWindow::SmallScroll( const uno::Any& Down, const uno::Any& Up, const uno::Any& ToRight, const uno::Any& ToLeft )
{
    Scroll( Down, Up, ToRight, ToLeft, false );
}

void SAL_CALL
ScVbaWindow::LargeScroll( const uno::Any& Down, const uno::Any& Up, const uno::Any& ToRight, const uno::Any& ToLeft )
{
    Scroll( Down, Up, ToRight, ToLeft, true );
}

uno::Any SAL_CALL
ScVbaWindow::SelectedSheets( const uno::Any& aIndex )
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
ScVbaWindow::ScrollWorkbookTabs( const uno::Any& /*Sheets*/, const uno::Any& /*Position*/ )
{
// #TODO #FIXME need some implementation to scroll through the tabs
// but where is this done?
/*
    sal_Int32 nSheets = 0;
    sal_Int32 nPosition = 0;
    throw uno::RuntimeException("No Implemented" );
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
ScVbaWindow::getCaption()
{
    static const char sCrud[] = " - OpenOffice.org Calc";
    static const sal_Int32 nCrudLen = strlen(sCrud);

    OUString sTitle;
    getFrameProps()->getPropertyValue( SC_UNONAME_TITLE ) >>= sTitle;
    sal_Int32 nCrudIndex = sTitle.indexOf( sCrud );
    // adjust title ( by removing crud )
    // sCrud string present
    if ( nCrudIndex != -1 )
    {
        // and ends with sCrud
        if ( ( nCrudLen + nCrudIndex ) == sTitle.getLength() )
        {
            sTitle = sTitle.copy( 0, nCrudIndex );
            rtl::Reference< ScVbaWorkbook > workbook( new ScVbaWorkbook( uno::Reference< XHelperInterface >( Application(), uno::UNO_QUERY_THROW ), mxContext, m_xModel ) );
            OUString sName = workbook->getName();
            // rather bizarre hack to make sure the name behavior
            // is like XL
            // if the adjusted title == workbook name, use name
            // if the adjusted title != workbook name but ...
            //  name == title + extension ( .csv, ,odt, .xls )
            //  etc. then also use the name

            if ( sTitle != sName )
            {
                // starts with title
                if ( sName.startsWith( sTitle ) )
                    // extension starts immediately after
                    if ( sName.match( ".", sTitle.getLength() ) )
                        sTitle = sName;
            }
        }
    }
    return uno::makeAny( sTitle );
}

void SAL_CALL
ScVbaWindow::setCaption( const uno::Any& _caption )
{
    getFrameProps()->setPropertyValue( SC_UNONAME_TITLE, _caption );
}

uno::Any SAL_CALL
ScVbaWindow::getScrollRow()
{
    sal_Int32 nValue = 0;
    // !! TODO !! get view shell from controller
    ScTabViewShell* pViewShell = excel::getBestViewShell( m_xModel );
    if ( pViewShell )
    {
        ScSplitPos eWhich = pViewShell->GetViewData().GetActivePart();
        nValue = pViewShell->GetViewData().GetPosY(WhichV(eWhich));
    }

    return uno::makeAny( nValue + 1);
}

void SAL_CALL
ScVbaWindow::setScrollRow( const uno::Any& _scrollrow )
{
    // !! TODO !! get view shell from controller
    ScTabViewShell* pViewShell = excel::getBestViewShell( m_xModel );
    if ( pViewShell )
    {
        sal_Int32 scrollRow = 0;
        _scrollrow >>= scrollRow;
        ScSplitPos eWhich = pViewShell->GetViewData().GetActivePart();
        sal_Int32 nOldValue = pViewShell->GetViewData().GetPosY(WhichV(eWhich)) + 1;
        pViewShell->ScrollLines(0, scrollRow - nOldValue);
    }
}

uno::Any SAL_CALL
ScVbaWindow::getScrollColumn()
{
    sal_Int32 nValue = 0;
    // !! TODO !! get view shell from controller
    ScTabViewShell* pViewShell = excel::getBestViewShell( m_xModel );
    if ( pViewShell )
    {
        ScSplitPos eWhich = pViewShell->GetViewData().GetActivePart();
        nValue = pViewShell->GetViewData().GetPosX(WhichH(eWhich));
    }

    return uno::makeAny( nValue + 1);
}

void SAL_CALL
ScVbaWindow::setScrollColumn( const uno::Any& _scrollcolumn )
{
    // !! TODO !! get view shell from controller
    ScTabViewShell* pViewShell = excel::getBestViewShell( m_xModel );
    if ( pViewShell )
    {
        sal_Int32 scrollColumn = 0;
        _scrollcolumn >>= scrollColumn;
        ScSplitPos eWhich = pViewShell->GetViewData().GetActivePart();
        sal_Int32 nOldValue = pViewShell->GetViewData().GetPosX(WhichH(eWhich)) + 1;
        pViewShell->ScrollLines(scrollColumn - nOldValue, 0);
    }
}

uno::Any SAL_CALL
ScVbaWindow::getWindowState()
{
    sal_Int32 nwindowState = xlNormal;
    // !! TODO !! get view shell from controller
    ScTabViewShell* pViewShell = excel::getBestViewShell( m_xModel );
    SfxViewFrame* pViewFrame = pViewShell -> GetViewFrame();
    WorkWindow* pWork = static_cast<WorkWindow*>( pViewFrame->GetFrame().GetSystemWindow() );
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
ScVbaWindow::setWindowState( const uno::Any& _windowstate )
{
    sal_Int32 nwindowState = xlMaximized;
    _windowstate >>= nwindowState;
    // !! TODO !! get view shell from controller
    ScTabViewShell* pViewShell = excel::getBestViewShell( m_xModel );
    SfxViewFrame* pViewFrame = pViewShell -> GetViewFrame();
    WorkWindow* pWork = static_cast<WorkWindow*>( pViewFrame->GetFrame().GetSystemWindow() );
    if ( pWork )
    {
        if ( nwindowState == xlMaximized)
            pWork -> Maximize();
        else if (nwindowState == xlMinimized)
            pWork -> Minimize();
        else if (nwindowState == xlNormal)
            pWork -> Restore();
        else
            throw uno::RuntimeException("Invalid Parameter" );
    }
}

void
ScVbaWindow::Activate()
{
    rtl::Reference<ScVbaWorkbook> workbook( new ScVbaWorkbook( uno::Reference< XHelperInterface >( Application(), uno::UNO_QUERY_THROW ), mxContext, m_xModel ) );

    workbook->Activate();
}

void
ScVbaWindow::Close( const uno::Any& SaveChanges, const uno::Any& FileName, const uno::Any& RouteWorkBook )
{
    rtl::Reference< ScVbaWorkbook > workbook( new ScVbaWorkbook( uno::Reference< XHelperInterface >( Application(), uno::UNO_QUERY_THROW ), mxContext, m_xModel ) );
    workbook->Close(SaveChanges, FileName, RouteWorkBook );
}

uno::Reference< excel::XPane > SAL_CALL
ScVbaWindow::ActivePane()
{
    uno::Reference< sheet::XViewPane > xViewPane( getController(), uno::UNO_QUERY_THROW );
    return new ScVbaPane( this, mxContext, m_xModel, xViewPane );
}

uno::Reference< excel::XRange > SAL_CALL
ScVbaWindow::ActiveCell(  )
{
    uno::Reference< excel::XApplication > xApplication( Application(), uno::UNO_QUERY_THROW );
    return xApplication->getActiveCell();
}

uno::Any SAL_CALL
ScVbaWindow::Selection(  )
{
    uno::Reference< excel::XApplication > xApplication( Application(), uno::UNO_QUERY_THROW );
    return xApplication->getSelection();
}

uno::Reference< excel::XRange > SAL_CALL
ScVbaWindow::RangeSelection()
{
    /*  TODO / FIXME: According to documentation, this method returns the range
        selection even if shapes are selected. */
    return uno::Reference< excel::XRange >( Selection(), uno::UNO_QUERY_THROW );
}

sal_Bool SAL_CALL
ScVbaWindow::getDisplayGridlines()
{
    bool bGrid = true;
    getControllerProps()->getPropertyValue( SC_UNO_SHOWGRID ) >>= bGrid;
    return bGrid;
}

void SAL_CALL
ScVbaWindow::setDisplayGridlines( sal_Bool _displaygridlines )
{
    getControllerProps()->setPropertyValue( SC_UNO_SHOWGRID, uno::makeAny( _displaygridlines ));
}

sal_Bool SAL_CALL
ScVbaWindow::getDisplayHeadings()
{
    bool bHeading = true;
    getControllerProps()->getPropertyValue( SC_UNO_COLROWHDR ) >>= bHeading;
    return bHeading;
}

void SAL_CALL
ScVbaWindow::setDisplayHeadings( sal_Bool _bDisplayHeadings )
{
    getControllerProps()->setPropertyValue( SC_UNO_COLROWHDR, uno::makeAny( _bDisplayHeadings ));
}

sal_Bool SAL_CALL
ScVbaWindow::getDisplayHorizontalScrollBar()
{
    bool bHorizontalScrollBar = true;
    getControllerProps()->getPropertyValue( SC_UNO_HORSCROLL ) >>= bHorizontalScrollBar;
    return bHorizontalScrollBar;
}

void SAL_CALL
ScVbaWindow::setDisplayHorizontalScrollBar( sal_Bool _bDisplayHorizontalScrollBar )
{
    getControllerProps()->setPropertyValue( SC_UNO_HORSCROLL, uno::makeAny( _bDisplayHorizontalScrollBar ));
}

sal_Bool SAL_CALL
ScVbaWindow::getDisplayOutline()
{
    bool bOutline = true;
    getControllerProps()->getPropertyValue( SC_UNO_OUTLSYMB ) >>= bOutline;
    return bOutline;
}

void SAL_CALL
ScVbaWindow::setDisplayOutline( sal_Bool _bDisplayOutline )
{
    getControllerProps()->setPropertyValue( SC_UNO_OUTLSYMB, uno::makeAny( _bDisplayOutline ));
}

sal_Bool SAL_CALL
ScVbaWindow::getDisplayVerticalScrollBar()
{
    bool bVerticalScrollBar = true;
    getControllerProps()->getPropertyValue( SC_UNO_VERTSCROLL ) >>= bVerticalScrollBar;
    return bVerticalScrollBar;
}

void SAL_CALL
ScVbaWindow::setDisplayVerticalScrollBar( sal_Bool _bDisplayVerticalScrollBar )
{
    getControllerProps()->setPropertyValue( SC_UNO_VERTSCROLL, uno::makeAny( _bDisplayVerticalScrollBar ));
}

sal_Bool SAL_CALL
ScVbaWindow::getDisplayWorkbookTabs()
{
    bool bWorkbookTabs = true;
    getControllerProps()->getPropertyValue( SC_UNO_SHEETTABS ) >>= bWorkbookTabs;
    return bWorkbookTabs;
}

void SAL_CALL
ScVbaWindow::setDisplayWorkbookTabs( sal_Bool _bDisplayWorkbookTabs )
{
    getControllerProps()->setPropertyValue( SC_UNO_SHEETTABS, uno::makeAny( _bDisplayWorkbookTabs ));
}

sal_Bool SAL_CALL
ScVbaWindow::getFreezePanes()
{
    uno::Reference< sheet::XViewFreezable > xViewFreezable( getController(), uno::UNO_QUERY_THROW );
    return xViewFreezable->hasFrozenPanes();
}

void SAL_CALL
ScVbaWindow::setFreezePanes( sal_Bool _bFreezePanes )
{
    uno::Reference< sheet::XViewPane > xViewPane( getController(), uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XViewSplitable > xViewSplitable( xViewPane, uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XViewFreezable > xViewFreezable( xViewPane, uno::UNO_QUERY_THROW );
    if( _bFreezePanes )
    {
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
    else
    {
        //remove the freeze panes
        xViewSplitable->splitAtPosition(0,0);
    }
}

sal_Bool SAL_CALL
ScVbaWindow::getSplit()
{
    uno::Reference< sheet::XViewSplitable > xViewSplitable( getController(), uno::UNO_QUERY_THROW );
    return xViewSplitable->getIsWindowSplit();
}

void SAL_CALL
ScVbaWindow::setSplit( sal_Bool _bSplit )
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
        SplitAtDefinedPosition( nColumn-1, nRow-1 );
    }
}

sal_Int32 SAL_CALL
ScVbaWindow::getSplitColumn()
{
    uno::Reference< sheet::XViewSplitable > xViewSplitable( getController(), uno::UNO_QUERY_THROW );
    return xViewSplitable->getSplitColumn();
}

void SAL_CALL
ScVbaWindow::setSplitColumn( sal_Int32 _splitcolumn )
{
    if( getSplitColumn() != _splitcolumn )
    {
        uno::Reference< sheet::XViewFreezable > xViewFreezable( getController(), uno::UNO_QUERY_THROW );
        sal_Int32 nRow = getSplitRow();
        SplitAtDefinedPosition( _splitcolumn, nRow );
    }
}

double SAL_CALL
ScVbaWindow::getSplitHorizontal()
{
    uno::Reference< sheet::XViewSplitable > xViewSplitable( getController(), uno::UNO_QUERY_THROW );
    return PixelsToPoints( getDevice(), xViewSplitable->getSplitHorizontal(), true );
}

void SAL_CALL
ScVbaWindow::setSplitHorizontal( double _splithorizontal )
{
    uno::Reference< sheet::XViewSplitable > xViewSplitable( getController(), uno::UNO_QUERY_THROW );
    double fHoriPixels = PointsToPixels( getDevice(), _splithorizontal, true );
    xViewSplitable->splitAtPosition( static_cast< sal_Int32 >( fHoriPixels ), 0 );
}

sal_Int32 SAL_CALL
ScVbaWindow::getSplitRow()
{
    uno::Reference< sheet::XViewSplitable > xViewSplitable( getController(), uno::UNO_QUERY_THROW );
    return xViewSplitable->getSplitRow();
}

void SAL_CALL
ScVbaWindow::setSplitRow( sal_Int32 _splitrow )
{
    if( getSplitRow() != _splitrow )
    {
        uno::Reference< sheet::XViewFreezable > xViewFreezable( getController(), uno::UNO_QUERY_THROW );
        sal_Int32 nColumn = getSplitColumn();
        SplitAtDefinedPosition( nColumn, _splitrow );
    }
}

double SAL_CALL
ScVbaWindow::getSplitVertical()
{
    uno::Reference< sheet::XViewSplitable > xViewSplitable( getController(), uno::UNO_QUERY_THROW );
    return PixelsToPoints( getDevice(), xViewSplitable->getSplitVertical(), false );
}

void SAL_CALL
ScVbaWindow::setSplitVertical(double _splitvertical )
{
    uno::Reference< sheet::XViewSplitable > xViewSplitable( getController(), uno::UNO_QUERY_THROW );
    double fVertiPixels = PointsToPixels( getDevice(), _splitvertical, false );
    xViewSplitable->splitAtPosition( 0, static_cast<sal_Int32>( fVertiPixels ) );
}

void ScVbaWindow::SplitAtDefinedPosition( sal_Int32 nColumns, sal_Int32 nRows )
{
    uno::Reference< sheet::XViewSplitable > xViewSplitable( getController(), uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XViewFreezable > xViewFreezable( xViewSplitable, uno::UNO_QUERY_THROW );
    // nColumns and nRows means split columns/rows
    if( nColumns == 0 && nRows == 0 )
        return;

    sal_Int32 cellColumn = nColumns + 1;
    sal_Int32 cellRow = nRows + 1;

    ScTabViewShell* pViewShell = excel::getBestViewShell( m_xModel );
    if ( pViewShell )
    {
        //firstly remove the old splitter
        xViewSplitable->splitAtPosition(0,0);

        uno::Reference< excel::XApplication > xApplication( Application(), uno::UNO_QUERY_THROW );
        uno::Reference< excel::XWorksheet > xSheet( xApplication->getActiveSheet(), uno::UNO_QUERY_THROW );
        xSheet->Cells(uno::makeAny(cellRow), uno::makeAny(cellColumn))->Select();

        //pViewShell->FreezeSplitters( FALSE );
        dispatchExecute( pViewShell, SID_WINDOW_SPLIT );
    }
}

uno::Any SAL_CALL
ScVbaWindow::getZoom()
{
    uno::Reference< beans::XPropertySet > xProps = getControllerProps();
    OUString sName( SC_UNO_ZOOMTYPE );
    sal_Int16 nZoomType = view::DocumentZoomType::PAGE_WIDTH;
    xProps->getPropertyValue( sName ) >>= nZoomType;
    if( nZoomType == view::DocumentZoomType::PAGE_WIDTH )
    {
        return uno::makeAny( true );
    }
    else if( nZoomType == view::DocumentZoomType::BY_VALUE )
    {
        sName = SC_UNO_ZOOMVALUE;
        sal_Int16 nZoom = 100;
        xProps->getPropertyValue( sName ) >>= nZoom;
        return uno::makeAny( nZoom );
    }
    return uno::Any();
}

void SAL_CALL ScVbaWindow::setZoom(const uno::Any& _zoom)
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
ScVbaWindow::ActiveSheet(  )
{
    uno::Reference< excel::XApplication > xApplication( Application(), uno::UNO_QUERY_THROW );
    return xApplication->getActiveSheet();
}

uno::Any SAL_CALL
ScVbaWindow::getView()
{
    bool bPageBreak = false;
    sal_Int32 nWindowView = excel::XlWindowView::xlNormalView;

    ScTabViewShell* pViewShell = excel::getBestViewShell( m_xModel );
    if (pViewShell)
        bPageBreak = pViewShell->GetViewData().IsPagebreakMode();

    if( bPageBreak )
        nWindowView = excel::XlWindowView::xlPageBreakPreview;
    else
        nWindowView = excel::XlWindowView::xlNormalView;

    return uno::makeAny( nWindowView );
}

void SAL_CALL
ScVbaWindow::setView( const uno::Any& _view)
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
            DebugHelper::runtimeexception(ERRCODE_BASIC_BAD_PARAMETER);
    }
    // !! TODO !! get view shell from controller
    ScTabViewShell* pViewShell = excel::getBestViewShell( m_xModel );
    if ( pViewShell )
        dispatchExecute( pViewShell, nSlot );
}

uno::Reference< excel::XRange > SAL_CALL
ScVbaWindow::getVisibleRange()
{
    uno::Reference< container::XIndexAccess > xPanesIA( getController(), uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XViewPane > xTopLeftPane( xPanesIA->getByIndex( 0 ), uno::UNO_QUERY_THROW );
    uno::Reference< excel::XPane > xPane( new ScVbaPane( this, mxContext, m_xModel, xTopLeftPane ) );
    return xPane->getVisibleRange();
}

sal_Int32 SAL_CALL
ScVbaWindow::PointsToScreenPixelsX(sal_Int32 _points)
{
    sal_Int32 nHundredthsofOneMillimeters = Millimeter::getInHundredthsOfOneMillimeter( _points );
    double fConvertFactor = getDevice()->getInfo().PixelPerMeterX/100000;
    return static_cast<sal_Int32>(fConvertFactor * nHundredthsofOneMillimeters );
}

sal_Int32 SAL_CALL
ScVbaWindow::PointsToScreenPixelsY(sal_Int32 _points)
{
    sal_Int32 nHundredthsofOneMillimeters = Millimeter::getInHundredthsOfOneMillimeter( _points );
    double fConvertFactor = getDevice()->getInfo().PixelPerMeterY/100000;
    return static_cast<sal_Int32>(fConvertFactor * nHundredthsofOneMillimeters );
}

void SAL_CALL
ScVbaWindow::PrintOut( const css::uno::Any& From, const css::uno::Any&To, const css::uno::Any& Copies, const css::uno::Any& Preview, const css::uno::Any& ActivePrinter, const css::uno::Any& PrintToFile, const css::uno::Any& Collate, const css::uno::Any& PrToFileName )
{
    // need test, print current active sheet
    // !! TODO !! get view shell from controller
    PrintOutHelper( excel::getBestViewShell( m_xModel ), From, To, Copies, Preview, ActivePrinter, PrintToFile, Collate, PrToFileName, true );
}

void SAL_CALL
ScVbaWindow::PrintPreview( const css::uno::Any& EnableChanges )
{
    // need test, print preview current active sheet
    // !! TODO !! get view shell from controller
    PrintPreviewHelper( EnableChanges, excel::getBestViewShell( m_xModel ) );
}

double SAL_CALL ScVbaWindow::getTabRatio()
{
    ScTabViewShell* pViewShell = excel::getBestViewShell( m_xModel );
    if ( pViewShell && pViewShell->GetViewData().GetView() )
    {
        double fRatio = ScTabView::GetRelTabBarWidth();
        if ( fRatio >= 0.0 && fRatio <= 1.0 )
            return fRatio;
    }
    return 0.0;
}

void SAL_CALL ScVbaWindow::setTabRatio( double fRatio )
{
    ScTabViewShell* pViewShell = excel::getBestViewShell( m_xModel );
    if ( pViewShell && pViewShell->GetViewData().GetView() )
    {
        if ( fRatio >= 0.0 && fRatio <= 1.0 )
            pViewShell->GetViewData().GetView()->SetRelTabBarWidth( fRatio );
    }
}

OUString
ScVbaWindow::getServiceImplName()
{
    return OUString("ScVbaWindow");
}

uno::Sequence< OUString >
ScVbaWindow::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.excel.Window"
    };
    return aServiceNames;
}
namespace window
{
namespace sdecl = comphelper::service_decl;
sdecl::vba_service_class_<ScVbaWindow, sdecl::with_args<true> > const serviceImpl;
sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "ScVbaWindow",
    "ooo.vba.excel.Window" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
