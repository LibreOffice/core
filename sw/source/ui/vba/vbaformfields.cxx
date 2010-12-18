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
#include "vbaformfields.hxx"
#include "vbaformfield.hxx"
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include "wordvbahelper.hxx"
#include <cppuhelper/implbase3.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

typedef std::vector< uno::Reference< text::XFormField > > XFormFieldVec;
typedef ::cppu::WeakImplHelper1< container::XEnumeration > FormFiledEnumeration_BASE;
typedef ::cppu::WeakImplHelper3< container::XNameAccess, container::XIndexAccess, container::XEnumerationAccess > FormFieldCollectionHelper_BASE;

rtl::OUString lcl_getFormFieldName( const uno::Reference< text::XFormField >& xFormField )
{
    rtl::OUString sName;
    sal_Int32 nCount = xFormField->getParamCount();
    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        if( xFormField->getParamName(i).equalsIgnoreAsciiCaseAscii( "Name" ) )
        {
            sName = xFormField->getParamValue(i);
            OSL_TRACE("lcl_getFormFieldName: %s", rtl::OUStringToOString( sName, RTL_TEXTENCODING_UTF8 ).getStr() );
            break;
        }
    }
    return sName;
}

class FormFieldsEnumeration : public FormFiledEnumeration_BASE
{
private:
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< frame::XModel > mxModel;
    XFormFieldVec mxFormFields;
    XFormFieldVec::iterator cachePos;
public:
    FormFieldsEnumeration( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xModel, const XFormFieldVec& xFormFiels ) throw (uno::RuntimeException) : mxParent( xParent ), mxContext( xContext ), mxModel( xModel ), mxFormFields( xFormFiels ), cachePos( mxFormFields.begin() )
    {
    }
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
    {
        return ( cachePos != mxFormFields.end() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( !hasMoreElements() )
            throw container::NoSuchElementException();
        uno::Reference< text::XFormField > xFormField( *cachePos++ );
        return uno::makeAny( uno::Reference< word::XFormField > ( new SwVbaFormField( mxParent, mxContext, mxModel, xFormField ) ) );
    }

};

class FormFieldCollectionHelper : public FormFieldCollectionHelper_BASE
{
private:
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< frame::XModel > mxModel;
    XFormFieldVec mxFormFields;
    XFormFieldVec::iterator cachePos;
public:
    FormFieldCollectionHelper( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xModel ) throw (uno::RuntimeException) : mxParent( xParent ), mxContext( xContext ), mxModel( xModel )
    {
        uno::Reference< text::XBookmarksSupplier > xBookmarksSupplier( xModel,uno::UNO_QUERY_THROW );
        uno::Reference< container::XIndexAccess > xIndexAccess( xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY_THROW );
        sal_Int32 nCount = xIndexAccess->getCount();
        for( sal_Int32 i = 0; i < nCount; i++ )
        {
            uno::Reference< text::XFormField > xFormField( xIndexAccess->getByIndex( i ), uno::UNO_QUERY );
            if( xFormField.is() )
                mxFormFields.push_back( xFormField );
        }
        cachePos = mxFormFields.begin();
    }
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException) { return  word::XFormField::static_type(0); }
    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException) { return getCount() > 0 ; }
    // XNameAcess
    virtual uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( !hasByName(aName) )
            throw container::NoSuchElementException();
        uno::Reference< text::XFormField > xFormField( *cachePos, uno::UNO_QUERY_THROW );
        return uno::makeAny( uno::Reference< word::XFormField >( new SwVbaFormField( mxParent, mxContext, mxModel, xFormField ) ) );
    }
    virtual uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (uno::RuntimeException)
    {
        uno::Sequence< rtl::OUString > sNames( mxFormFields.size() );
        rtl::OUString* pString = sNames.getArray();
        XFormFieldVec::iterator it = mxFormFields.begin();
        XFormFieldVec::iterator it_end = mxFormFields.end();
        for ( ; it != it_end; ++it, ++pString )
        {
            uno::Reference< text::XFormField > xFormField( *cachePos, uno::UNO_QUERY_THROW );
            *pString =  lcl_getFormFieldName( xFormField );
        }
        return sNames;
    }
    virtual ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (uno::RuntimeException)
    {
        cachePos = mxFormFields.begin();
        XFormFieldVec::iterator it_end = mxFormFields.end();
        for ( ; cachePos != it_end; ++cachePos )
        {
            //uno::Reference< container::XNamed > xName( *cachePos, uno::UNO_QUERY_THROW );
            uno::Reference< text::XFormField > xFormField( *cachePos, uno::UNO_QUERY_THROW );
            if ( aName.equalsIgnoreAsciiCase( lcl_getFormFieldName( xFormField )) )
                break;
        }
        return ( cachePos != it_end );
    }
    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException)
    {
        return mxFormFields.size();
    }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
    {
        if ( Index < 0 || Index >= getCount() )
            throw lang::IndexOutOfBoundsException();
        return uno::makeAny( uno::Reference< word::XFormField >( new SwVbaFormField( mxParent, mxContext, mxModel, mxFormFields[ Index ] ) ) );
    }
    // XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) throw (uno::RuntimeException)
    {
        return new FormFieldsEnumeration( mxParent, mxContext, mxModel, mxFormFields );
    }
};

SwVbaFormFields::SwVbaFormFields( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xModel ): SwVbaFormFields_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( new FormFieldCollectionHelper( xParent, xContext, xModel ) ) ), mxModel( xModel )
{
}
// XEnumerationAccess
uno::Type
SwVbaFormFields::getElementType() throw (uno::RuntimeException)
{
    return word::XFormField::static_type(0);
}
uno::Reference< container::XEnumeration >
SwVbaFormFields::createEnumeration() throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return xEnumAccess->createEnumeration();
}

uno::Any
SwVbaFormFields::createCollectionObject( const css::uno::Any& aSource )
{
    return aSource;
}

rtl::OUString&
SwVbaFormFields::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaFormFields") );
    return sImplName;
}

css::uno::Sequence<rtl::OUString>
SwVbaFormFields::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.FormFields") );
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
