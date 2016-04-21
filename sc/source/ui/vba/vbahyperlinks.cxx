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

#include "vbahyperlinks.hxx"
#include <algorithm>
#include <vector>
#include <cppuhelper/implbase.hxx>
#include <ooo/vba/office/MsoHyperlinkType.hpp>
#include "rangelst.hxx"
#include "vbahyperlink.hxx"
#include "vbarange.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

namespace {

/** Returns true, if every range of rxInner is contained in any range of rScOuter. */
bool lclContains( const ScRangeList& rScOuter, const uno::Reference< excel::XRange >& rxInner ) throw (uno::RuntimeException)
{
    const ScRangeList& rScInner = ScVbaRange::getScRangeList( rxInner );
    if( rScInner.empty() || rScOuter.empty() )
        throw uno::RuntimeException("Empty range objects" );

    for( size_t nIndex = 0, nCount = rScInner.size(); nIndex < nCount; ++nIndex )
        if( !rScOuter.In( *rScInner[ nIndex ] ) )
            return false;
    return true;
}

/** Functor to decide whether the anchors of two Hyperlink objects are equal. */
struct EqualAnchorFunctor
{
    uno::Reference< excel::XRange > mxAnchorRange;
    uno::Reference< msforms::XShape > mxAnchorShape;
    sal_Int32 mnType;
    explicit EqualAnchorFunctor( const uno::Reference< excel::XHyperlink >& rxHlink ) throw (uno::RuntimeException);
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
            return (rScRanges1.size() == 1) && (rScRanges2.size() == 1) && (*rScRanges1[ 0 ] == *rScRanges2[ 0 ]);
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

namespace detail {

class ScVbaHlinkContainer : public ::cppu::WeakImplHelper< container::XIndexAccess >
{
public:
    explicit ScVbaHlinkContainer() throw (uno::RuntimeException);
    explicit ScVbaHlinkContainer( const ScVbaHlinkContainerRef& rxSheetContainer, const ScRangeList& rScRanges ) throw (uno::RuntimeException);
    virtual ~ScVbaHlinkContainer();

    /** Inserts the passed hyperlink into the collection. Will remove a
        Hyperlink object with the same anchor as the passed Hyperlink object. */
    void insertHyperlink( const uno::Reference< excel::XHyperlink >& rxHlink ) throw (uno::RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw (uno::RuntimeException, std::exception) override;
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw (uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw (uno::RuntimeException, std::exception) override;

private:
    typedef ::std::vector< uno::Reference< excel::XHyperlink > > HyperlinkVector;
    HyperlinkVector     maHlinks;
};

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

sal_Int32 SAL_CALL ScVbaHlinkContainer::getCount() throw (uno::RuntimeException, std::exception)
{
    return static_cast< sal_Int32 >( maHlinks.size() );
}

uno::Any SAL_CALL ScVbaHlinkContainer::getByIndex( sal_Int32 nIndex )
        throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    if( (0 <= nIndex) && (nIndex < getCount()) )
        return uno::Any( maHlinks[ static_cast< size_t >( nIndex ) ] );
    throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScVbaHlinkContainer::getElementType() throw (uno::RuntimeException, std::exception)
{
    return cppu::UnoType<excel::XHyperlink>::get();
}

sal_Bool SAL_CALL ScVbaHlinkContainer::hasElements() throw (uno::RuntimeException, std::exception)
{
    return !maHlinks.empty();
}

ScVbaHlinkContainerMember::ScVbaHlinkContainerMember( ScVbaHlinkContainer* pContainer ) :
    mxContainer( pContainer )
{
}

ScVbaHlinkContainerMember::~ScVbaHlinkContainerMember()
{
}

} // namespace detail

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
    const uno::Any& rScreenTip, const uno::Any& rTextToDisplay ) throw (uno::RuntimeException, std::exception)
{
    /*  If this Hyperlinks object has been created from a Range object, the
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

void SAL_CALL ScVbaHyperlinks::Delete() throw (uno::RuntimeException, std::exception)
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
    return cppu::UnoType<excel::XHyperlink>::get();
}

// ScVbaCollectionBase --------------------------------------------------------

uno::Any ScVbaHyperlinks::createCollectionObject( const uno::Any& rSource )
{
    // container stores XHyperlink objects, just return the passed object
    return rSource;
}

// XHelperInterface -----------------------------------------------------------

VBAHELPER_IMPL_XHELPERINTERFACE( ScVbaHyperlinks, "ooo.vba.excel.Hyperlinks" )

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
