/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
