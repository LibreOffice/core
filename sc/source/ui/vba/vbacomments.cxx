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
#include "vbacomments.hxx"

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/sheet/XSheetAnnotation.hpp>

#include "vbaglobals.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

uno::Any AnnotationToComment( const uno::Any& aSource, uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xModel )
{
    uno::Reference< sheet::XSheetAnnotation > xAnno( aSource, uno::UNO_QUERY_THROW );
    uno::Reference< container::XChild > xChild( xAnno, uno::UNO_QUERY_THROW );
    uno::Reference< table::XCellRange > xCellRange( xChild->getParent(), uno::UNO_QUERY_THROW );

    // #FIXME needs to find the correct Parent
    return uno::makeAny( uno::Reference< excel::XComment > (
        new ScVbaComment( uno::Reference< XHelperInterface >(), xContext, xModel, xCellRange ) ) );
}

class CommentEnumeration : public EnumerationHelperImpl
{
    css::uno::Reference< css::frame::XModel > mxModel;
public:
    CommentEnumeration(
            const uno::Reference< XHelperInterface >& xParent,
            const uno::Reference< uno::XComponentContext >& xContext,
            const uno::Reference< container::XEnumeration >& xEnumeration,
            const uno::Reference< frame::XModel >& xModel ) throw ( uno::RuntimeException ) :
        EnumerationHelperImpl( xParent, xContext, xEnumeration ),
        mxModel( xModel, uno::UNO_SET_THROW )
    {}

    virtual uno::Any SAL_CALL nextElement() throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        return AnnotationToComment( m_xEnumeration->nextElement(), m_xContext, mxModel );
    }

};

ScVbaComments::ScVbaComments(
        const uno::Reference< XHelperInterface >& xParent,
        const uno::Reference< uno::XComponentContext > & xContext,
        const uno::Reference< frame::XModel >& xModel,
        const uno::Reference< container::XIndexAccess >& xIndexAccess  ) :
    ScVbaComments_BASE( xParent, xContext, xIndexAccess ),
    mxModel( xModel, uno::UNO_SET_THROW )
{
}

// public helper functions

uno::Reference< container::XEnumeration >
ScVbaComments::createEnumeration() throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return new CommentEnumeration( mxParent, mxContext, xEnumAccess->createEnumeration(), mxModel );
}

uno::Any
ScVbaComments::createCollectionObject( const css::uno::Any& aSource )
{
    return AnnotationToComment( aSource,  mxContext, mxModel );
}

uno::Type
ScVbaComments::getElementType() throw (uno::RuntimeException)
{
    return excel::XComment::static_type(0);
}

rtl::OUString&
ScVbaComments::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaComments") );
    return sImplName;
}

css::uno::Sequence<rtl::OUString>
ScVbaComments::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.Comments") );
    }
    return sNames;
}
