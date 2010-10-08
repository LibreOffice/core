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
#include "vbasections.hxx"
#include "vbasection.hxx"
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <docsh.hxx>
#include <doc.hxx>
#include "wordvbahelper.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

typedef ::cppu::WeakImplHelper1< container::XEnumeration > SectionEnumeration_BASE;
typedef ::cppu::WeakImplHelper2< container::XIndexAccess, container::XEnumerationAccess > SectionCollectionHelper_Base;
typedef std::vector< uno::Reference< beans::XPropertySet > > XSectionVec;

class SectionEnumeration : public SectionEnumeration_BASE
{
    XSectionVec mxSections;
    XSectionVec::iterator mIt;

public:
    SectionEnumeration( const XSectionVec& rVec ) : mxSections( rVec ), mIt( mxSections.begin() ) {}
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
    {
        return ( mIt != mxSections.end() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( hasMoreElements() )
            return uno::makeAny( *mIt++ );
        throw container::NoSuchElementException();
    }
};

// here I regard pagestyle as section
class SectionCollectionHelper : public SectionCollectionHelper_Base
{
private:
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< frame::XModel > mxModel;
    XSectionVec mxSections;

public:
    SectionCollectionHelper( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xModel ) throw (uno::RuntimeException) : mxParent( xParent ), mxContext( xContext ), mxModel( xModel )
    {
        uno::Reference< style::XStyleFamiliesSupplier > xSytleFamSupp( mxModel, uno::UNO_QUERY_THROW );
        uno::Reference< container::XNameAccess > xSytleFamNames( xSytleFamSupp->getStyleFamilies(), uno::UNO_QUERY_THROW );
        uno::Reference< container::XIndexAccess > xPageStyles( xSytleFamNames->getByName( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PageStyles") ) ), uno::UNO_QUERY_THROW );
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

    ~SectionCollectionHelper(){}

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException)
    {
        return mxSections.size();
    }
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( Index < 0 || Index >= getCount() )
            throw css::lang::IndexOutOfBoundsException();

        uno::Reference< beans::XPropertySet > xPageProps( mxSections[ Index ], uno::UNO_QUERY_THROW );
        return uno::makeAny( uno::Reference< word::XSection >( new SwVbaSection( mxParent,  mxContext, mxModel, xPageProps ) ) );
    }
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException)
    {
        return word::XSection::static_type(0);
    }
    virtual sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException)
    {
        return sal_True;
    }
    // XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) throw (uno::RuntimeException)
    {
        return new SectionEnumeration( mxSections );
    }
};

class SectionsEnumWrapper : public EnumerationHelperImpl
{
    uno::Reference< frame::XModel > mxModel;
public:
    SectionsEnumWrapper( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration,  const uno::Reference< frame::XModel >& xModel  ) throw ( uno::RuntimeException ) : EnumerationHelperImpl( xParent, xContext, xEnumeration ), mxModel( xModel ){}

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        uno::Reference< beans::XPropertySet > xPageProps( m_xEnumeration->nextElement(), uno::UNO_QUERY_THROW );
        return uno::makeAny( uno::Reference< word::XSection > ( new SwVbaSection( m_xParent, m_xContext, mxModel, xPageProps ) ) );
    }
};

SwVbaSections::SwVbaSections( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xModel ): SwVbaSections_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( new SectionCollectionHelper( xParent, xContext, xModel ) ) ),  mxModel( xModel )
{
}

uno::Any SAL_CALL
SwVbaSections::PageSetup( ) throw (uno::RuntimeException)
{
    if( m_xIndexAccess->getCount() )
    {
        // check if the first section is our want
        uno::Reference< word::XSection > xSection( m_xIndexAccess->getByIndex( 0 ), uno::UNO_QUERY_THROW );
        return xSection->PageSetup();
    }
    throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("There is no section") ), uno::Reference< uno::XInterface >() );
}

// XEnumerationAccess
uno::Type SAL_CALL
SwVbaSections::getElementType() throw (uno::RuntimeException)
{
    return word::XSection::static_type(0);
}

uno::Reference< container::XEnumeration > SAL_CALL
SwVbaSections::createEnumeration() throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return new SectionsEnumWrapper( this, mxContext, xEnumAccess->createEnumeration(), mxModel );
}

uno::Any
SwVbaSections::createCollectionObject( const css::uno::Any& aSource )
{
    return aSource;
}

rtl::OUString&
SwVbaSections::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaSections") );
    return sImplName;
}

css::uno::Sequence<rtl::OUString>
SwVbaSections::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Sections") );
    }
    return sNames;
}
