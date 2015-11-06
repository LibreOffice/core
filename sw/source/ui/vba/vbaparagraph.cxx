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
#include "vbaparagraph.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include "vbarange.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaParagraph::SwVbaParagraph( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< text::XTextDocument >& xDocument, const uno::Reference< text::XTextRange >& xTextRange ) throw ( uno::RuntimeException ) :
    SwVbaParagraph_BASE( rParent, rContext ), mxTextDocument( xDocument ), mxTextRange( xTextRange )
{
}

SwVbaParagraph::~SwVbaParagraph()
{
}

uno::Reference< word::XRange > SAL_CALL
SwVbaParagraph::getRange( ) throw ( uno::RuntimeException, std::exception )
{
    return uno::Reference< word::XRange >( new SwVbaRange( this, mxContext, mxTextDocument, mxTextRange->getStart(), mxTextRange->getEnd(), mxTextRange->getText() ) );
}

uno::Any SAL_CALL
SwVbaParagraph::getStyle( ) throw ( uno::RuntimeException, std::exception )
{
    uno::Reference< word::XRange > xRange = getRange();
    return xRange->getStyle();
}

void SAL_CALL
SwVbaParagraph::setStyle( const uno::Any& style ) throw ( uno::RuntimeException, std::exception )
{
    uno::Reference< word::XRange > xRange = getRange();
    xRange->setStyle( style );
}

OUString
SwVbaParagraph::getServiceImplName()
{
    return OUString("SwVbaParagraph");
}

uno::Sequence< OUString >
SwVbaParagraph::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.word.Paragraph";
    }
    return aServiceNames;
}

class ParagraphCollectionHelper : public ::cppu::WeakImplHelper< container::XIndexAccess,
                                                                 container::XEnumerationAccess >
{
private:
    uno::Reference< text::XTextDocument > mxTextDocument;

    uno::Reference< container::XEnumeration > getEnumeration() throw (uno::RuntimeException)
    {
        uno::Reference< container::XEnumerationAccess > xParEnumAccess( mxTextDocument->getText(), uno::UNO_QUERY_THROW );
        return xParEnumAccess->createEnumeration();
    }

public:
    explicit ParagraphCollectionHelper( const uno::Reference< text::XTextDocument >& xDocument ) throw (uno::RuntimeException): mxTextDocument( xDocument )
    {
    }
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException, std::exception) override { return  cppu::UnoType<text::XTextRange>::get(); }
    virtual sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException, std::exception) override { return sal_True; }
    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException, std::exception) override
    {
        sal_Int32 nCount = 0;
        uno::Reference< container::XEnumeration > xParEnum = getEnumeration();
        while( xParEnum->hasMoreElements() )
        {
            uno::Reference< lang::XServiceInfo > xServiceInfo( xParEnum->nextElement(), uno::UNO_QUERY_THROW );
            if( xServiceInfo->supportsService("com.sun.star.text.Paragraph") )
            {
                nCount++;
            }
        }
        return nCount;
    }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception ) override
    {
        if( Index < getCount() )
        {
            sal_Int32 nCount = 0;
            uno::Reference< container::XEnumeration > xParEnum = getEnumeration();
            while( xParEnum->hasMoreElements() )
            {
                uno::Reference< lang::XServiceInfo > xServiceInfo( xParEnum->nextElement(), uno::UNO_QUERY_THROW );
                if( xServiceInfo->supportsService("com.sun.star.text.Paragraph") )
                {
                    if( Index == nCount )
                        return uno::makeAny( xServiceInfo );
                    nCount++;
                }
            }
        }
        throw lang::IndexOutOfBoundsException();
    }
    // XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) throw (uno::RuntimeException, std::exception) override
    {
        return getEnumeration();
    }
};

SwVbaParagraphs::SwVbaParagraphs( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< css::uno::XComponentContext > & xContext, const uno::Reference< text::XTextDocument >& xDocument ) throw (uno::RuntimeException) : SwVbaParagraphs_BASE( xParent, xContext, new ParagraphCollectionHelper( xDocument ) ), mxTextDocument( xDocument )
{
}

// XEnumerationAccess
uno::Type
SwVbaParagraphs::getElementType() throw (uno::RuntimeException)
{
    return cppu::UnoType<word::XParagraph>::get();
}
uno::Reference< container::XEnumeration >
SwVbaParagraphs::createEnumeration() throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumerationAccess > xEnumerationAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return xEnumerationAccess->createEnumeration();
}

uno::Any
SwVbaParagraphs::createCollectionObject( const css::uno::Any& aSource )
{
    uno::Reference< text::XTextRange > xTextRange( aSource, uno::UNO_QUERY_THROW );
    return uno::makeAny( uno::Reference< word::XParagraph >( new SwVbaParagraph( this, mxContext, mxTextDocument, xTextRange ) ) );
}

OUString
SwVbaParagraphs::getServiceImplName()
{
    return OUString("SwVbaParagraphs");
}

css::uno::Sequence<OUString>
SwVbaParagraphs::getServiceNames()
{
    static uno::Sequence< OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = "ooo.vba.word.Paragraphs";
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
