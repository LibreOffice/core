/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbacomments.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:49:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "vbacomments.hxx"

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/sheet/XSheetAnnotation.hpp>

#include "vbaglobals.hxx"

using namespace ::org::openoffice;
using namespace ::com::sun::star;

uno::Any AnnotationToComment( const uno::Any& aSource, uno::Reference< uno::XComponentContext > & xContext )
{
    uno::Reference< sheet::XSheetAnnotation > xAnno( aSource, uno::UNO_QUERY_THROW );
    uno::Reference< container::XChild > xChild( xAnno, uno::UNO_QUERY_THROW );
    uno::Reference< table::XCellRange > xCellRange( xChild->getParent(), uno::UNO_QUERY_THROW );

    // #FIXME needs to find the correct Parent
    return uno::makeAny( uno::Reference< excel::XComment > ( new ScVbaComment( uno::Reference< vba::XHelperInterface >(), xContext, xCellRange ) ) );
}

class CommentEnumeration : public EnumerationHelperImpl
{
public:
    CommentEnumeration( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration ) throw ( uno::RuntimeException ) : EnumerationHelperImpl( xContext, xEnumeration ) {}

    virtual uno::Any SAL_CALL nextElement() throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        return AnnotationToComment( m_xEnumeration->nextElement(),  m_xContext );
    }

};

ScVbaComments::ScVbaComments( const uno::Reference< vba::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< container::XIndexAccess >& xIndexAccess  )
: ScVbaComments_BASE( xParent, xContext, xIndexAccess )
{
}

// public helper functions

uno::Reference< container::XEnumeration >
ScVbaComments::createEnumeration() throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );

    return new CommentEnumeration( mxContext, xEnumAccess->createEnumeration() );
}

uno::Any
ScVbaComments::createCollectionObject( const css::uno::Any& aSource )
{
    return AnnotationToComment( aSource,  mxContext );
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
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.excel.Comments") );
    }
    return sNames;
}
