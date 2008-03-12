/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tablecolumn.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:02:46 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <com/sun/star/lang/DisposedException.hpp>

#include "tablecolumn.hxx"
#include "tableundo.hxx"
#include "svx/svdmodel.hxx"
#include "svx/svdotable.hxx"

// -----------------------------------------------------------------------------

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::beans;

// -----------------------------------------------------------------------------

namespace sdr { namespace table {

const sal_Int32 Property_Width = 0;
const sal_Int32 Property_OptimalWidth = 1;
const sal_Int32 Property_IsVisible = 2;
const sal_Int32 Property_IsStartOfNewPage = 3;

// -----------------------------------------------------------------------------
// TableRow
// -----------------------------------------------------------------------------

TableColumn::TableColumn( const TableModelRef& xTableModel, sal_Int32 nColumn )
: TableColumnBase( getStaticPropertySetInfo() )
, mxTableModel( xTableModel )
, mnColumn( nColumn )
, mnWidth( 0 )
, mbOptimalWidth( sal_True )
, mbIsVisible( sal_True )
, mbIsStartOfNewPage( sal_False )
{
}

// -----------------------------------------------------------------------------

TableColumn::~TableColumn()
{
}

// -----------------------------------------------------------------------------

void TableColumn::dispose()
{
    mxTableModel.clear();
}

// -----------------------------------------------------------------------------

void TableColumn::throwIfDisposed() const throw (::com::sun::star::uno::RuntimeException)
{
    if( !mxTableModel.is() )
        throw DisposedException();
}

// -----------------------------------------------------------------------------

TableColumn& TableColumn::operator=( const TableColumn& r )
{
    mnWidth = r.mnWidth;
    mbOptimalWidth = r.mbOptimalWidth;
    mbIsVisible = r.mbIsVisible;
    mbIsStartOfNewPage = r.mbIsStartOfNewPage;

    return *this;
}

// -----------------------------------------------------------------------------
// XCellRange
// -----------------------------------------------------------------------------

Reference< XCell > SAL_CALL TableColumn::getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow ) throw (IndexOutOfBoundsException, RuntimeException)
{
    throwIfDisposed();
    if( nColumn != 0 )
        throw IndexOutOfBoundsException();

    return mxTableModel->getCellByPosition( mnColumn, nRow );
}

// -----------------------------------------------------------------------------

Reference< XCellRange > SAL_CALL TableColumn::getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom ) throw (IndexOutOfBoundsException, RuntimeException)
{
    throwIfDisposed();
    if( (nTop >= 0 ) && (nLeft == 0) && (nBottom >= nTop) && (nRight == 0)  )
    {
        return mxTableModel->getCellRangeByPosition( mnColumn, nTop, mnColumn, nBottom );
    }
    throw IndexOutOfBoundsException();
}

// -----------------------------------------------------------------------------

Reference< XCellRange > SAL_CALL TableColumn::getCellRangeByName( const OUString& /*aRange*/ ) throw (RuntimeException)
{
    return Reference< XCellRange >();
}

// -----------------------------------------------------------------------------
// XNamed
// -----------------------------------------------------------------------------

OUString SAL_CALL TableColumn::getName() throw (RuntimeException)
{
    return maName;
}

// -----------------------------------------------------------------------------

void SAL_CALL TableColumn::setName( const OUString& aName ) throw (RuntimeException)
{
    maName = aName;
}

// -----------------------------------------------------------------------------
// XFastPropertySet
// -----------------------------------------------------------------------------

void SAL_CALL TableColumn::setFastPropertyValue( sal_Int32 nHandle, const Any& aValue ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, RuntimeException)
{
    bool bOk = false;
    bool bChange = false;

    TableColumnUndo* pUndo = 0;
    if( mxTableModel.is() && mxTableModel->getSdrTableObj() && mxTableModel->getSdrTableObj()->IsInserted() )
    {
        TableColumnRef xThis( this );
        pUndo = new TableColumnUndo( xThis );
    }

    switch( nHandle )
    {
    case Property_Width:
        {
            sal_Int32 nWidth = mnWidth;
            bOk = aValue >>= nWidth;
            if( bOk && (nWidth != mnWidth) )
            {
                mnWidth = nWidth;
                mbOptimalWidth = mnWidth == 0;
                bChange = true;
            }
            break;
        }
    case Property_OptimalWidth:
        {
            sal_Bool bOptimalWidth = mbOptimalWidth;
            bOk = aValue >>= bOptimalWidth;
            if( bOk && (mbOptimalWidth != bOptimalWidth) )
            {
                mbOptimalWidth = bOptimalWidth;
                if( bOptimalWidth )
                    mnWidth = 0;
                bChange = true;
            }
            break;
        }
    case Property_IsVisible:
        {
            sal_Bool bIsVisible = mbIsVisible;
            bOk = aValue >>= bIsVisible;
            if( bOk && (mbIsVisible != bIsVisible) )
            {
                mbIsVisible = bIsVisible;
                bChange = true;
            }
            break;
        }

    case Property_IsStartOfNewPage:
        {
            sal_Bool bIsStartOfNewPage = mbIsStartOfNewPage;
            bOk = aValue >>= bIsStartOfNewPage;
            if( bOk && (mbIsStartOfNewPage != bIsStartOfNewPage) )
            {
                mbIsStartOfNewPage = bIsStartOfNewPage;
                bChange = true;
            }
            break;
        }
    default:
        throw UnknownPropertyException();
    }
    if( !bOk )
        throw IllegalArgumentException();

    if( bChange )
    {
        if( pUndo )
        {
            SdrModel* pModel = mxTableModel->getSdrTableObj()->GetModel();
            if( pModel )
            {
                pModel->AddUndo( pUndo );
                pUndo = 0;
            }
        }
        mxTableModel->setModified(sal_True);
    }

    if( pUndo )
        delete pUndo;
}

// -----------------------------------------------------------------------------

Any SAL_CALL TableColumn::getFastPropertyValue( sal_Int32 nHandle ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    switch( nHandle )
    {
    case Property_Width:            return Any( mnWidth );
    case Property_OptimalWidth:     return Any( mbOptimalWidth );
    case Property_IsVisible:        return Any( mbIsVisible );
    case Property_IsStartOfNewPage: return Any( mbIsStartOfNewPage );
    default:                        throw UnknownPropertyException();
    }
}

// -----------------------------------------------------------------------------

rtl::Reference< ::comphelper::FastPropertySetInfo > TableColumn::getStaticPropertySetInfo()
{
    static rtl::Reference< ::comphelper::FastPropertySetInfo > xInfo;
    if( !xInfo.is() )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !xInfo.is() )
        {
            comphelper::PropertyVector aProperties(6);

            aProperties[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Width" ) );
            aProperties[0].Handle = Property_Width;
            aProperties[0].Type = ::getCppuType((const sal_Int32*)0);
            aProperties[0].Attributes = 0;

            aProperties[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "OptimalWidth" ) );
            aProperties[1].Handle = Property_OptimalWidth;
            aProperties[1].Type = ::getBooleanCppuType();
            aProperties[1].Attributes = 0;

            aProperties[2].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "IsVisible" ) );
            aProperties[2].Handle = Property_IsVisible;
            aProperties[2].Type = ::getBooleanCppuType();
            aProperties[2].Attributes = 0;

            aProperties[3].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "IsStartOfNewPage" ) );
            aProperties[3].Handle = Property_IsStartOfNewPage;
            aProperties[3].Type = ::getBooleanCppuType();
            aProperties[3].Attributes = 0;

            aProperties[4].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Size" ) );
            aProperties[4].Handle = Property_Width;
            aProperties[4].Type = ::getCppuType((const sal_Int32*)0);
            aProperties[4].Attributes = 0;

            aProperties[5].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "OptimalSize" ) );
            aProperties[5].Handle = Property_OptimalWidth;
            aProperties[5].Type = ::getBooleanCppuType();
            aProperties[5].Attributes = 0;

            xInfo.set( new ::comphelper::FastPropertySetInfo(aProperties) );
        }
    }

    return xInfo;
}

// -----------------------------------------------------------------------------

} }
