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

#include "vbahyperlinks.hxx"
#include <algorithm>
#include <vector>
#include <ooo/vba/office/MsoHyperlinkType.hpp>
#include "rangelst.hxx"
#include "vbahyperlink.hxx"
#include "vbarange.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;
using ::rtl::OUString;

// ============================================================================

namespace {

/** Returns true, if every range of rxInner is contained in any range of rScOuter. */
bool lclContains( const ScRangeList& rScOuter, const uno::Reference< excel::XRange >& rxInner ) throw (uno::RuntimeException)
{
    const ScRangeList& rScInner = ScVbaRange::getScRangeList( rxInner );
    if( (rScInner.Count() == 0) || (rScOuter.Count() == 0) )
        throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Empty range objects" ) ), uno::Reference< uno::XInterface >() );

    for( sal_uLong nIndex = 0, nCount = rScInner.Count(); nIndex < nCount; ++nIndex )
        if( !rScOuter.In( *rScInner.GetObject( nIndex ) ) )
            return false;
    return true;
}

// ----------------------------------------------------------------------------

/** Functor to decide whether the anchors of two Hyperlink objects are equal. */
struct EqualAnchorFunctor
{
    uno::Reference< excel::XRange > mxAnchorRange;
    uno::Reference< msforms::XShape > mxAnchorShape;
    sal_Int32 mnType;
    EqualAnchorFunctor( const uno::Reference< excel::XHyperlink >& rxHlink ) throw (uno::RuntimeException);
    bool operator()( const uno::Reference< excel::XHyperlink >& rxHlink ) const throw (uno::RuntimeException);
};

EqualAnchorFunctor::EqualAnchorFunctor( const uno::Reference< excel::XHyperlink >& rxHlink ) throw (uno::RuntimeException) :
    mnType( rxHlink->getType() )
{
    switch( mnType )
    {
        case office::MsoHyperlinkType::msoHyperlinkRange:
            mxAnchorRange.set( rxHlink->getRange(), uno::UNO_QUERY_THROW );
        break;
        case office::MsoHyperlinkType::msoHyperlinkShape:
        case office::MsoHyperlinkType::msoHyperlinkInlineShape:
            mxAnchorShape.set( rxHlink->getShape(), uno::UNO_QUERY_THROW );
        break;
        default:
            throw uno::RuntimeException();
    }
}

bool EqualAnchorFunctor::operator()( const uno::Reference< excel::XHyperlink >& rxHlink ) const throw (uno::RuntimeException)
{
    sal_Int32 nType = rxHlink->getType();
    if( nType != mnType )
        return false;

    switch( nType )
    {
        case office::MsoHyperlinkType::msoHyperlinkRange:
        {
            uno::Reference< excel::XRange > xAnchorRange( rxHlink->getRange(), uno::UNO_QUERY_THROW );
            const ScRangeList& rScRanges1 = ScVbaRange::getScRangeList( xAnchorRange );
            const ScRangeList& rScRanges2 = ScVbaRange::getScRangeList( mxAnchorRange );
            return (rScRanges1.Count() == 1) && (rScRanges2.Count() == 1) && (*rScRanges1.GetObject( 0 ) == *rScRanges2.GetObject( 0 ));
        }
        case office::MsoHyperlinkType::msoHyperlinkShape:
        case office::MsoHyperlinkType::msoHyperlinkInlineShape:
        {
            uno::Reference< msforms::XShape > xAnchorShape( rxHlink->getShape(), uno::UNO_QUERY_THROW );
            return xAnchorShape.get() == mxAnchorShape.get();
        }
        default:
            throw uno::RuntimeException();
    }
}

} // namespace

// ============================================================================

namespace detail {

class ScVbaHlinkContainer : public ::cppu::WeakImplHelper1< container::XIndexAccess >
{
public:
    explicit ScVbaHlinkContainer() throw (uno::RuntimeException);
    explicit ScVbaHlinkContainer( const ScVbaHlinkContainerRef& rxSheetContainer, const ScRangeList& rScRanges ) throw (uno::RuntimeException);
    virtual ~ScVbaHlinkContainer();

    /** Inserts the passed hyperlink into the collection. Will remove a
        Hyperlink object with the same anchor as the passed Hyperlink object. */
    void insertHyperlink( const uno::Reference< excel::XHyperlink >& rxHlink ) throw (uno::RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw (uno::RuntimeException);
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException);

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw (uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw (uno::RuntimeException);

private:
    typedef ::std::vector< uno::Reference< excel::XHyperlink > > HyperlinkVector;
    HyperlinkVector     maHlinks;
};

// ----------------------------------------------------------------------------

ScVbaHlinkContainer::ScVbaHlinkContainer() throw (uno::RuntimeException)
{
    // TODO FIXME: fill with existing hyperlinks
}

ScVbaHlinkContainer::ScVbaHlinkContainer( const ScVbaHlinkContainerRef& rxSheetContainer,
        const ScRangeList& rScRanges ) throw (uno::RuntimeException)
{
    for( sal_Int32 nIndex = 0, nCount = rxSheetContainer->getCount(); nIndex < nCount; ++nIndex )
    {
        uno::Reference< excel::XHyperlink > xHlink( rxSheetContainer->getByIndex( nIndex ), uno::UNO_QUERY_THROW );
        uno::Reference< excel::XRange > xHlinkRange( xHlink->getRange(), uno::UNO_QUERY_THROW );
        if( lclContains( rScRanges, xHlinkRange ) )
            maHlinks.push_back( xHlink );
    }
}

ScVbaHlinkContainer::~ScVbaHlinkContainer()
{
}

void ScVbaHlinkContainer::insertHyperlink( const uno::Reference< excel::XHyperlink >& rxHlink ) throw (uno::RuntimeException)
{
    HyperlinkVector::iterator aIt = ::std::find_if( maHlinks.begin(), maHlinks.end(), EqualAnchorFunctor( rxHlink ) );
    if( aIt == maHlinks.end() )
        maHlinks.push_back( rxHlink );
    else
        *aIt = rxHlink;
}

sal_Int32 SAL_CALL ScVbaHlinkContainer::getCount() throw (uno::RuntimeException)
{
    return static_cast< sal_Int32 >( maHlinks.size() );
}

uno::Any SAL_CALL ScVbaHlinkContainer::getByIndex( sal_Int32 nIndex )
        throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( (0 <= nIndex) && (nIndex < getCount()) )
        return uno::Any( maHlinks[ static_cast< size_t >( nIndex ) ] );
    throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScVbaHlinkContainer::getElementType() throw (uno::RuntimeException)
{
    return excel::XHyperlink::static_type( 0 );
}

sal_Bool SAL_CALL ScVbaHlinkContainer::hasElements() throw (uno::RuntimeException)
{
    return !maHlinks.empty();
}

// ============================================================================

ScVbaHlinkContainerMember::ScVbaHlinkContainerMember( ScVbaHlinkContainer* pContainer ) :
    mxContainer( pContainer )
{
}

ScVbaHlinkContainerMember::~ScVbaHlinkContainerMember()
{
}

} // namespace detail

// ============================================================================

ScVbaHyperlinks::ScVbaHyperlinks( const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext ) throw (uno::RuntimeException) :
    detail::ScVbaHlinkContainerMember( new detail::ScVbaHlinkContainer ),
    ScVbaHyperlinks_BASE( rxParent, rxContext, uno::Reference< container::XIndexAccess >( mxContainer.get() ) )
{
}

ScVbaHyperlinks::ScVbaHyperlinks( const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const ScVbaHyperlinksRef& rxSheetHlinks, const ScRangeList& rScRanges ) throw (uno::RuntimeException) :
    detail::ScVbaHlinkContainerMember( new detail::ScVbaHlinkContainer( rxSheetHlinks->mxContainer, rScRanges ) ),
    ScVbaHyperlinks_BASE( rxParent, rxContext, uno::Reference< container::XIndexAccess >( mxContainer.get() ) ),
    mxSheetHlinks( rxSheetHlinks )
{
}

ScVbaHyperlinks::~ScVbaHyperlinks()
{
}

// XHyperlinks ----------------------------------------------------------------

uno::Reference< excel::XHyperlink > SAL_CALL ScVbaHyperlinks::Add(
    const uno::Any& rAnchor, const uno::Any& rAddress, const uno::Any& rSubAddress,
    const uno::Any& rScreenTip, const uno::Any& rTextToDisplay ) throw (uno::RuntimeException)
{
    /*  If this Hyperlinks object has been craeted from a Range object, the
        call to Add() is passed to the Hyperlinks object of the parent
        worksheet. This container will not be modified (it will not contain the
        inserted hyperlink).
        For details, see documentation in hyperlinks.hxx.
     */
    if( mxSheetHlinks.is() )
        return mxSheetHlinks->Add( rAnchor, rAddress, rSubAddress, rScreenTip, rTextToDisplay );

    // get anchor object (can be a Range or a Shape object)
    uno::Reference< XHelperInterface > xAnchor( rAnchor, uno::UNO_QUERY_THROW );

    /*  Create the Hyperlink object, this tries to insert the hyperlink into
        the spreadsheet document. Parent of the Hyperlink is the anchor object. */
    uno::Reference< excel::XHyperlink > xHlink( new ScVbaHyperlink(
        xAnchor, mxContext, rAddress, rSubAddress, rScreenTip, rTextToDisplay ) );

    /*  If creation of the hyperlink did not throw, insert it into the
        collection. */
    mxContainer->insertHyperlink( xHlink );
    return xHlink;
}

void SAL_CALL ScVbaHyperlinks::Delete() throw (uno::RuntimeException)
{
    // FIXME not implemented
    throw uno::RuntimeException();
}

// XEnumerationAccess ---------------------------------------------------------

uno::Reference< container::XEnumeration > SAL_CALL ScVbaHyperlinks::createEnumeration() throw (uno::RuntimeException)
{
    return new SimpleIndexAccessToEnumeration( m_xIndexAccess );
}

// XElementAccess -------------------------------------------------------------

uno::Type SAL_CALL ScVbaHyperlinks::getElementType() throw (uno::RuntimeException)
{
    return excel::XHyperlink::static_type( 0 );
}

// ScVbaCollectionBase --------------------------------------------------------

uno::Any ScVbaHyperlinks::createCollectionObject( const uno::Any& rSource )
{
    // container stores XHyperlink objects, just return the passed object
    return rSource;
}

// XHelperInterface -----------------------------------------------------------

VBAHELPER_IMPL_XHELPERINTERFACE( ScVbaHyperlinks, "ooo.vba.excel.Hyperlinks" )

// ============================================================================
