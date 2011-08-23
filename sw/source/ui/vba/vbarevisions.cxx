/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile:
 * $Revision:
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
#include "vbarevisions.hxx"
#include "vbarevision.hxx"
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/document/XRedlinesSupplier.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

typedef ::cppu::WeakImplHelper1< container::XEnumeration > RevisionEnumeration_BASE;
typedef ::cppu::WeakImplHelper2< container::XIndexAccess, container::XEnumerationAccess > RevisionCollectionHelper_BASE;
typedef std::vector< uno::Reference< beans::XPropertySet > > RevisionMap;

class RedlinesEnumeration : public RevisionEnumeration_BASE
{
    RevisionMap mRevisionMap;
    RevisionMap::iterator mIt;
public:
    RedlinesEnumeration( const RevisionMap& sMap ) : mRevisionMap( sMap ), mIt( mRevisionMap.begin() ) {}
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
    {
        return ( mIt != mRevisionMap.end() );
    }
    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( !hasMoreElements() )
            throw container::NoSuchElementException();
        uno::Reference< beans::XPropertySet > xRevision( *mIt++ );
        return uno::makeAny( xRevision ) ;
    }
};

class RevisionCollectionHelper : public RevisionCollectionHelper_BASE
{
    RevisionMap mRevisionMap;
public:
RevisionCollectionHelper( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XTextRange >& xTextRange ) throw (uno::RuntimeException);

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException) { return  beans::XPropertySet::static_type(0); }
    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException) { return ( mRevisionMap.size() > 0 ); }
    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException) { return mRevisionMap.size(); }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
    {
        if ( Index < 0 || Index >= getCount() )
            throw lang::IndexOutOfBoundsException();

        return uno::makeAny( mRevisionMap[ Index ] );

    }
    // XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) throw (uno::RuntimeException)
    {
        return new RedlinesEnumeration( mRevisionMap );
    }
};

RevisionCollectionHelper::RevisionCollectionHelper( const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XTextRange >& xTextRange ) throw (uno::RuntimeException)
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
class RevisionsEnumeration : public EnumerationHelperImpl
{
    uno::Reference< frame::XModel > m_xModel;
public:
    RevisionsEnumeration( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration,  const uno::Reference< frame::XModel >& xModel  ) throw ( uno::RuntimeException ) : EnumerationHelperImpl( xParent, xContext, xEnumeration ), m_xModel( xModel ) {}

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        uno::Reference< beans::XPropertySet > xRevision( m_xEnumeration->nextElement(), uno::UNO_QUERY_THROW );
        return uno::makeAny( uno::Reference< word::XRevision > ( new SwVbaRevision( m_xParent, m_xContext, m_xModel, xRevision ) ) );
    }

};

SwVbaRevisions::SwVbaRevisions( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XTextRange >& xTextRange ): SwVbaRevisions_BASE( xParent, xContext, new RevisionCollectionHelper( xModel, xTextRange ) ),  mxModel( xModel )
{
}

SwVbaRevisions::SwVbaRevisions( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xModel, const uno::Reference< container::XIndexAccess >& xIndexAccess ): SwVbaRevisions_BASE( xParent, xContext, xIndexAccess ),  mxModel( xModel )
{
}

// XEnumerationAccess
uno::Type
SwVbaRevisions::getElementType() throw (uno::RuntimeException)
{
    return word::XRevision::static_type(0);
}
uno::Reference< container::XEnumeration >
SwVbaRevisions::createEnumeration() throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return new RevisionsEnumeration( this, mxContext, xEnumAccess->createEnumeration(), mxModel );
}

uno::Any
SwVbaRevisions::createCollectionObject( const css::uno::Any& aSource )
{
    uno::Reference< beans::XPropertySet > xRevision( aSource, uno::UNO_QUERY_THROW );
    return uno::makeAny( uno::Reference< word::XRevision > ( new SwVbaRevision( this, mxContext, mxModel, xRevision ) ) );
}

void SAL_CALL SwVbaRevisions::AcceptAll(  ) throw (css::uno::RuntimeException)
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

    std::vector< uno::Reference< word::XRevision > >::iterator it = aRevisions.begin();
    for( ; it != aRevisions.end(); it++ )
    {
        uno::Reference< word::XRevision > xRevision( *it );
        xRevision->Accept();
    }
}

void SAL_CALL SwVbaRevisions::RejectAll(  ) throw (css::uno::RuntimeException)
{
    throw uno::RuntimeException();
}

rtl::OUString&
SwVbaRevisions::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaRevisions") );
    return sImplName;
}

css::uno::Sequence<rtl::OUString>
SwVbaRevisions::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Revisions") );
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
