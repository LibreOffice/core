/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbawindow.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 16:11:49 $
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
#include "vbawindow.hxx"
#include "vbaworksheets.hxx"
#include "vbaworksheet.hxx"
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <org/openoffice/excel/XlWindowState.hpp>
#include <org/openoffice/excel/Constants.hpp>

#include <docsh.hxx>
#include <tabvwsh.hxx>
#include <docuno.hxx>
#include <sc.hrc>
#include <hash_map>
#include <sfx2/viewfrm.hxx>
#include <sfx2/topfrm.hxx>

using namespace ::com::sun::star;
using namespace ::org::openoffice;
using namespace ::org::openoffice::excel::XlWindowState;

typedef  std::hash_map< rtl::OUString,
SCTAB, ::rtl::OUStringHash,
::std::equal_to< ::rtl::OUString > > NameIndexHash;

typedef std::vector < uno::Reference< sheet::XSpreadsheet > > Sheets;

typedef ::cppu::WeakImplHelper1< container::XEnumeration

> Enumeration_BASE;

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
        return uno::makeAny( uno::Reference< excel::XWorksheet > ( new ScVbaWorksheet( m_xContext, *(m_it++), m_xModel ) ) );
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
        ScTabViewShell* pViewShell = getBestViewShell( m_xModel );
        if ( !pViewShell )
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Cannot obtain view shell" ) ), uno::Reference< uno::XInterface >() );

        SCTAB nTabCount = pDocShell->GetDocument()->GetTableCount();
        uno::Sequence<sal_Int32> aSheets( nTabCount );
        SCTAB nIndex = 0;
        const ScMarkData& rMarkData = pViewShell->GetViewData()->GetMarkData();
        sheets.reserve( nTabCount );
        uno::Reference <sheet::XSpreadsheetDocument> xSpreadSheet( m_xModel, uno::UNO_QUERY_THROW );
        uno::Reference <container::XIndexAccess> xIndex( xSpreadSheet->getSheets(), uno::UNO_QUERY_THROW );
        for ( SCTAB nTab=0; nTab<nTabCount; nTab++ )
        {
            if ( rMarkData.GetTableSelect(nTab) )
            {
                uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex( nTab ), uno::UNO_QUERY_THROW );
                uno::Reference< container::XNamed > xNamed( xSheet, uno::UNO_QUERY_THROW );
                sheets.push_back( xSheet );
                namesToIndices[ xNamed->getName() ] = nIndex++;
            }
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
        return (sheets.size() > 0);
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


void
ScVbaWindow::Scroll( const uno::Any& Down, const uno::Any& Up, const uno::Any& ToRight, const uno::Any& ToLeft, bool bLargeScroll ) throw (uno::RuntimeException)
{
    ScTabViewShell* pViewShell = getBestViewShell( m_xModel );
    if ( !pViewShell )
        return;

    sal_Int16 down = 0;
    sal_Int16 up = 0;
    sal_Int16 toRight = 0;
    sal_Int16 toLeft = 0;
    Down >>= down;
    Up >>= up;
    ToRight >>= toRight;
    ToLeft >>= toLeft;
    uno::Sequence< beans::PropertyValue > args1(2);
    args1[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "By" ) );
    args1[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Sel" ) );
    args1[1].Value <<= false;

    SCROW totalUp = up - down;
    SCCOL totalLeft = toLeft - toRight;

    if ( totalUp != 0 )
    {
        if (bLargeScroll)
            totalUp = totalUp * pViewShell->GetViewData()->VisibleCellsY( SC_SPLIT_BOTTOM );
        pViewShell->ScrollLines(0, -totalUp);
    }

    if ( totalLeft != 0 )
    {
        if (bLargeScroll)
            totalLeft = totalLeft * pViewShell->GetViewData()->VisibleCellsX( SC_SPLIT_LEFT );
        pViewShell->ScrollLines(-totalLeft, 0);
    }

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
    uno::Reference< container::XEnumerationAccess > xEnumAccess( new SelectedSheetsEnumAccess( m_xContext, m_xModel  ) );
    uno::Reference< excel::XWorksheets > xSheets(  new ScVbaWorksheets( m_xContext, xEnumAccess, m_xModel ) );
    if ( aIndex.hasValue() )
    {
        uno::Reference< vba::XCollection > xColl( xSheets, uno::UNO_QUERY_THROW );
        return xColl->Item( aIndex );
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
    throw uno::RuntimeException( rtl::OUString::createFromAscii("No Implemented" ), uno::Reference< uno::XInterface >() );
    sal_Bool bSheets = ( Sheets >>= nSheets );
    sal_Bool bPosition = ( Position >>= nPosition );
    if ( bSheets || bPosition ) // at least one param specified
        if ( bSheets )
            ;// use sheets
        else if ( bPosition )
            ; //use position
*/

}
uno::Reference< beans::XPropertySet >
getPropsFromModel( const uno::Reference< frame::XModel >& xModel )
{
    uno::Reference< frame::XController > xController = xModel->getCurrentController();
    if ( !xController.is() )
        throw uno::RuntimeException( rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM ("No controller for model") ), uno::Reference< uno::XInterface >() );
    return uno::Reference< beans::XPropertySet >(  xController->getFrame(), uno::UNO_QUERY );
}


uno::Any SAL_CALL
ScVbaWindow::getCaption() throw (uno::RuntimeException)
{
    static rtl::OUString sCrud(RTL_CONSTASCII_USTRINGPARAM(" - OpenOffice.org Calc" ) );
    static sal_Int32 nCrudLen = sCrud.getLength();

    uno::Reference< beans::XPropertySet > xProps = getPropsFromModel( m_xModel );
    rtl::OUString sTitle;
    xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ("Title") ) ) >>= sTitle;
    sal_Int32 nCrudIndex = sTitle.indexOf( sCrud );
    // adjust title ( by removing crud )
    // sCrud string present
    if ( nCrudIndex != -1 )
    {
        // and ends with sCrud
        if ( ( nCrudLen + nCrudIndex ) == sTitle.getLength() )
        {
            sTitle = sTitle.copy( 0, nCrudIndex );
            ScVbaWorkbook workbook( m_xContext, m_xModel );
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

    uno::Reference< beans::XPropertySet > xProps = getPropsFromModel( m_xModel );
    xProps->setPropertyValue( rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM ("Title") ) , _caption );
}

uno::Any SAL_CALL
ScVbaWindow::getScrollRow() throw (uno::RuntimeException)
{
    sal_Int32 nValue = 0;
    ScTabViewShell* pViewShell = getBestViewShell( m_xModel );
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
    ScTabViewShell* pViewShell = getBestViewShell( m_xModel );
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
    ScTabViewShell* pViewShell = getBestViewShell( m_xModel );
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
    ScTabViewShell* pViewShell = getBestViewShell( m_xModel );
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
    ScTabViewShell* pViewShell = getBestViewShell( m_xModel );
    SfxViewFrame* pViewFrame = pViewShell -> GetViewFrame();
    SfxTopViewFrame *pTop= PTR_CAST( SfxTopViewFrame, pViewFrame -> GetTopViewFrame() );
    if ( pTop )
    {
    // FOR_UPSTREAM_BUILD
    /*
        WorkWindow* pWork = (WorkWindow*) pTop->GetTopFrame_Impl()->GetTopWindow_Impl();
        if ( pWork )
        {
            if ( pWork -> IsMaximized())
                nwindowState = xlMaximized;
            else if (pWork -> IsMinimized())
                nwindowState = xlMinimized;
        }
    */
    }
    return uno::makeAny( nwindowState );
}

void SAL_CALL
ScVbaWindow::setWindowState( const uno::Any& _windowstate ) throw (uno::RuntimeException)
{
    sal_Int32 nwindowState = xlMaximized;
    _windowstate >>= nwindowState;
    ScTabViewShell* pViewShell = getBestViewShell( m_xModel );
    SfxViewFrame* pViewFrame = pViewShell -> GetViewFrame();
    SfxTopViewFrame *pTop= PTR_CAST( SfxTopViewFrame, pViewFrame -> GetTopViewFrame() );
    if ( pTop )
    {
    // FOR_UPSTREAM_BUILD
    /*
        WorkWindow* pWork = (WorkWindow*) pTop->GetTopFrame_Impl()->GetTopWindow_Impl();
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
    */
    }
}

void
ScVbaWindow::Activate() throw (css::uno::RuntimeException)
{
    ScVbaWorkbook workbook( m_xContext, m_xModel );
    workbook.Activate();
}

void
ScVbaWindow::Close( const uno::Any& SaveChanges, const uno::Any& FileName, const uno::Any& RouteWorkBook ) throw (uno::RuntimeException)
{
    ScVbaWorkbook workbook( m_xContext, m_xModel );
    workbook.Close(SaveChanges, FileName, RouteWorkBook );
}
