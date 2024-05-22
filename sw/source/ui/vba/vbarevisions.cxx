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
#include "vbarevisions.hxx"
#include "vbarevision.hxx"
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XRedlinesSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <utility>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

typedef std::vector< uno::Reference< beans::XPropertySet > > RevisionMap;

namespace {

class RedlinesEnumeration : public ::cppu::WeakImplHelper< container::XEnumeration >
{
    RevisionMap mRevisionMap;
    RevisionMap::iterator mIt;
public:
    explicit RedlinesEnumeration( RevisionMap&& sMap ) : mRevisionMap( std::move(sMap) ), mIt( mRevisionMap.begin() ) {}
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override
    {
        return ( mIt != mRevisionMap.end() );
    }
    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        if ( !hasMoreElements() )
            throw container::NoSuchElementException();
        uno::Reference< beans::XPropertySet > xRevision( *mIt++ );
        return uno::Any( xRevision ) ;
    }
};

class RevisionCollectionHelper : public ::cppu::WeakImplHelper< container::XIndexAccess,
                                                                container::XEnumerationAccess >
{
    RevisionMap mRevisionMap;
public:
/// @throws css::uno::RuntimeException
RevisionCollectionHelper( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XTextRange >& xTextRange );

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) override { return  cppu::UnoType<beans::XPropertySet>::get(); }
    virtual sal_Bool SAL_CALL hasElements(  ) override { return ( !mRevisionMap.empty() ); }
    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) override { return mRevisionMap.size(); }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) override
    {
        if ( Index < 0 || Index >= getCount() )
            throw lang::IndexOutOfBoundsException();

        return uno::Any( mRevisionMap[ Index ] );

    }
    // XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) override
    {
        return new RedlinesEnumeration( std::vector(mRevisionMap) );
    }
};

}

RevisionCollectionHelper::RevisionCollectionHelper( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XTextRange >& xTextRange )
    {
        uno::Reference< text::XTextRangeCompare > xTRC( xTextRange->getText(), uno::UNO_QUERY_THROW );
        uno::Reference< document::XRedlinesSupplier > xRedlinesSupp( xModel, uno::UNO_QUERY_THROW );
        uno::Reference< container::XIndexAccess > xRedlines( xRedlinesSupp->getRedlines(), uno::UNO_QUERY_THROW );
        sal_Int32 nCount = xRedlines->getCount();
        for( sal_Int32 index = 0; index < nCount; index++ )
        {
            uno::Reference< text::XTextRange > xRedlineRange( xRedlines->getByIndex( index ), uno::UNO_QUERY_THROW );
            if( xTRC->compareRegionStarts( xTextRange, xRedlineRange ) >= 0 && xTRC->compareRegionEnds( xTextRange, xRedlineRange ) <= 0 )
            {
                uno::Reference< beans::XPropertySet > xRedlineProps( xRedlineRange, uno::UNO_QUERY_THROW );
                mRevisionMap.push_back( xRedlineProps );
            }
        }
    }

namespace {

class RevisionsEnumeration : public EnumerationHelperImpl
{
    uno::Reference< frame::XModel > m_xModel;
public:
    /// @throws uno::RuntimeException
    RevisionsEnumeration( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration,  uno::Reference< frame::XModel >  xModel  ) : EnumerationHelperImpl( xParent, xContext, xEnumeration ), m_xModel(std::move( xModel )) {}

    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        uno::Reference< beans::XPropertySet > xRevision( m_xEnumeration->nextElement(), uno::UNO_QUERY_THROW );
        return uno::Any( uno::Reference< word::XRevision > ( new SwVbaRevision( m_xParent, m_xContext, m_xModel, xRevision ) ) );
    }

};

}

SwVbaRevisions::SwVbaRevisions( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XTextRange >& xTextRange ): SwVbaRevisions_BASE( xParent, xContext, new RevisionCollectionHelper( xModel, xTextRange ) ),  mxModel( xModel )
{
}

SwVbaRevisions::SwVbaRevisions( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, uno::Reference< frame::XModel >  xModel, const uno::Reference< container::XIndexAccess >& xIndexAccess ): SwVbaRevisions_BASE( xParent, xContext, xIndexAccess ),  mxModel(std::move( xModel ))
{
}

// XEnumerationAccess
uno::Type
SwVbaRevisions::getElementType()
{
    return cppu::UnoType<word::XRevision>::get();
}
uno::Reference< container::XEnumeration >
SwVbaRevisions::createEnumeration()
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return new RevisionsEnumeration( this, mxContext, xEnumAccess->createEnumeration(), mxModel );
}

uno::Any
SwVbaRevisions::createCollectionObject( const css::uno::Any& aSource )
{
    uno::Reference< beans::XPropertySet > xRevision( aSource, uno::UNO_QUERY_THROW );
    return uno::Any( uno::Reference< word::XRevision > ( new SwVbaRevision( this, mxContext, mxModel, xRevision ) ) );
}

void SAL_CALL SwVbaRevisions::AcceptAll(  )
{
    // First we need to put all the redline into a vector, because if the redline is accepted,
    // it will auto delete in the document.
    std::vector< uno::Reference< word::XRevision > > aRevisions;
    uno::Reference< container::XEnumeration > xEnumeration = createEnumeration();
    while( xEnumeration->hasMoreElements() )
    {
        uno::Reference< word::XRevision > xRevision( xEnumeration->nextElement(), uno::UNO_QUERY_THROW );
        aRevisions.push_back( xRevision );
    }

    for( const auto& xRevision : aRevisions )
        xRevision->Accept();
}

void SAL_CALL SwVbaRevisions::RejectAll(  )
{
    throw uno::RuntimeException();
}

OUString
SwVbaRevisions::getServiceImplName()
{
    return u"SwVbaRevisions"_ustr;
}

css::uno::Sequence<OUString>
SwVbaRevisions::getServiceNames()
{
    static uno::Sequence< OUString > const sNames
    {
        u"ooo.vba.word.Revisions"_ustr
    };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
