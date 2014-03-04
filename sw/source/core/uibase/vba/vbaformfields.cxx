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

OUString lcl_getFormFieldName( const uno::Reference< text::XFormField >& xFormField )
{
    OUString sName;
    sal_Int32 nCount = xFormField->getParamCount();
    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        if( xFormField->getParamName(i).equalsIgnoreAsciiCase("Name") )
        {
            sName = xFormField->getParamValue(i);
            OSL_TRACE("lcl_getFormFieldName: %s", OUStringToOString( sName, RTL_TEXTENCODING_UTF8 ).getStr() );
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
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException) { return  cppu::UnoType<word::XFormField>::get(); }
    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException) { return getCount() > 0 ; }
    // XNameAcess
    virtual uno::Any SAL_CALL getByName( const OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( !hasByName(aName) )
            throw container::NoSuchElementException();
        uno::Reference< text::XFormField > xFormField( *cachePos, uno::UNO_QUERY_THROW );
        return uno::makeAny( uno::Reference< word::XFormField >( new SwVbaFormField( mxParent, mxContext, mxModel, xFormField ) ) );
    }
    virtual uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (uno::RuntimeException)
    {
        uno::Sequence< OUString > sNames( mxFormFields.size() );
        OUString* pString = sNames.getArray();
        XFormFieldVec::iterator it = mxFormFields.begin();
        XFormFieldVec::iterator it_end = mxFormFields.end();
        for ( ; it != it_end; ++it, ++pString )
        {
            uno::Reference< text::XFormField > xFormField( *cachePos, uno::UNO_QUERY_THROW );
            *pString =  lcl_getFormFieldName( xFormField );
        }
        return sNames;
    }
    virtual ::sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (uno::RuntimeException)
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
    return cppu::UnoType<word::XFormField>::get();
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

OUString
SwVbaFormFields::getServiceImplName()
{
    return OUString("SwVbaFormFields");
}

css::uno::Sequence<OUString>
SwVbaFormFields::getServiceNames()
{
    static uno::Sequence< OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = "ooo.vba.word.FormFields";
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
