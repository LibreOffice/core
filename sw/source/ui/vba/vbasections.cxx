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
#include "vbasections.hxx"
#include "vbasection.hxx"
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include "wordvbahelper.hxx"
#include <cppuhelper/implbase.hxx>
#include <utility>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

typedef std::vector< uno::Reference< beans::XPropertySet > > XSectionVec;

namespace {

class SectionEnumeration : public ::cppu::WeakImplHelper< container::XEnumeration >
{
    XSectionVec mxSections;
    XSectionVec::iterator mIt;

public:
    explicit SectionEnumeration( XSectionVec&& rVec ) : mxSections( std::move(rVec) ), mIt( mxSections.begin() ) {}
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override
    {
        return ( mIt != mxSections.end() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        if ( hasMoreElements() )
            return uno::Any( *mIt++ );
        throw container::NoSuchElementException();
    }
};

// here I regard pagestyle as section
class SectionCollectionHelper : public ::cppu::WeakImplHelper< container::XIndexAccess,
                                                               container::XEnumerationAccess >
{
private:
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< frame::XModel > mxModel;
    XSectionVec mxSections;

public:
    /// @throws uno::RuntimeException
    SectionCollectionHelper( uno::Reference< XHelperInterface >  xParent, uno::Reference< uno::XComponentContext > xContext, uno::Reference< frame::XModel >  xModel ) : mxParent(std::move( xParent )), mxContext(std::move( xContext )), mxModel(std::move( xModel ))
    {
        uno::Reference< style::XStyleFamiliesSupplier > xSytleFamSupp( mxModel, uno::UNO_QUERY_THROW );
        uno::Reference< container::XNameAccess > xSytleFamNames( xSytleFamSupp->getStyleFamilies(), uno::UNO_SET_THROW );
        uno::Reference< container::XIndexAccess > xPageStyles( xSytleFamNames->getByName(u"PageStyles"_ustr), uno::UNO_QUERY_THROW );
        sal_Int32 nCount = xPageStyles->getCount();
        for( sal_Int32 index = 0; index < nCount; ++index )
        {
            uno::Reference< style::XStyle > xStyle( xPageStyles->getByIndex( index ), uno::UNO_QUERY_THROW );
            // only the pagestyles in using are considered
            if( xStyle->isInUse( ) )
            {
                uno::Reference< beans::XPropertySet > xPageProps( xStyle, uno::UNO_QUERY_THROW );
                mxSections.push_back( xPageProps );
            }
        }
    }

    /// @throws uno::RuntimeException
    SectionCollectionHelper( uno::Reference< XHelperInterface >  xParent, uno::Reference< uno::XComponentContext > xContext, uno::Reference< frame::XModel >  xModel, const uno::Reference< text::XTextRange >& xTextRange ) : mxParent(std::move( xParent )), mxContext(std::move( xContext )), mxModel(std::move( xModel ))
    {
        // Hacky implementation of Range.Sections, only support 1 section
        uno::Reference< beans::XPropertySet > xRangeProps( xTextRange, uno::UNO_QUERY_THROW );
        uno::Reference< style::XStyle > xStyle = word::getCurrentPageStyle( mxModel, xRangeProps );
        uno::Reference< beans::XPropertySet > xPageProps( xStyle, uno::UNO_QUERY_THROW );
        mxSections.push_back( xPageProps );
    }

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) override
    {
        return mxSections.size();
    }
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override
    {
        if ( Index < 0 || Index >= getCount() )
            throw css::lang::IndexOutOfBoundsException();

        uno::Reference< beans::XPropertySet > xPageProps( mxSections[ Index ], uno::UNO_SET_THROW );
        return uno::Any( uno::Reference< word::XSection >( new SwVbaSection( mxParent,  mxContext, mxModel, xPageProps ) ) );
    }
    virtual uno::Type SAL_CALL getElementType(  ) override
    {
        return cppu::UnoType<word::XSection>::get();
    }
    virtual sal_Bool SAL_CALL hasElements(  ) override
    {
        return true;
    }
    // XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) override
    {
        return new SectionEnumeration( std::vector(mxSections) );
    }
};

class SectionsEnumWrapper : public EnumerationHelperImpl
{
    uno::Reference< frame::XModel > mxModel;
public:
    /// @throws uno::RuntimeException
    SectionsEnumWrapper( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration,  uno::Reference< frame::XModel >  xModel  ) : EnumerationHelperImpl( xParent, xContext, xEnumeration ), mxModel(std::move( xModel )){}

    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        uno::Reference< beans::XPropertySet > xPageProps( m_xEnumeration->nextElement(), uno::UNO_QUERY_THROW );
        return uno::Any( uno::Reference< word::XSection > ( new SwVbaSection( m_xParent, m_xContext, mxModel, xPageProps ) ) );
    }
};

}

SwVbaSections::SwVbaSections( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xModel ): SwVbaSections_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( new SectionCollectionHelper( xParent, xContext, xModel ) ) ),  mxModel( xModel )
{
}

SwVbaSections::SwVbaSections( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xModel, const uno::Reference< text::XTextRange >& xTextRange ): SwVbaSections_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( new SectionCollectionHelper( xParent, xContext, xModel, xTextRange ) ) ),  mxModel( xModel )
{
}

uno::Any SAL_CALL
SwVbaSections::PageSetup( )
{
    if( m_xIndexAccess->getCount() )
    {
        // check if the first section is our want
        uno::Reference< word::XSection > xSection( m_xIndexAccess->getByIndex( 0 ), uno::UNO_QUERY_THROW );
        return xSection->PageSetup();
    }
    throw uno::RuntimeException(u"There is no section"_ustr );
}

// XEnumerationAccess
uno::Type SAL_CALL
SwVbaSections::getElementType()
{
    return cppu::UnoType<word::XSection>::get();
}

uno::Reference< container::XEnumeration > SAL_CALL
SwVbaSections::createEnumeration()
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return new SectionsEnumWrapper( this, mxContext, xEnumAccess->createEnumeration(), mxModel );
}

uno::Any
SwVbaSections::createCollectionObject( const css::uno::Any& aSource )
{
    return aSource;
}

OUString
SwVbaSections::getServiceImplName()
{
    return u"SwVbaSections"_ustr;
}

css::uno::Sequence<OUString>
SwVbaSections::getServiceNames()
{
    static uno::Sequence< OUString > const sNames
    {
        u"ooo.vba.word.Sections"_ustr
    };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
