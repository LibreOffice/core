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



#include "vbabutton.hxx"
#include "vbanewfont.hxx"

using namespace com::sun::star;
using namespace ooo::vba;


const static rtl::OUString LABEL( RTL_CONSTASCII_USTRINGPARAM("Label") );
ScVbaButton::ScVbaButton( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, AbstractGeometryAttributes* pGeomHelper ) : ButtonImpl_BASE( xParent, xContext, xControl, xModel, pGeomHelper )
{
}

// Attributes
rtl::OUString SAL_CALL
ScVbaButton::getCaption() throw (css::uno::RuntimeException)
{
    rtl::OUString Label;
    m_xProps->getPropertyValue( LABEL ) >>= Label;
    return Label;
}

void SAL_CALL
ScVbaButton::setCaption( const rtl::OUString& _caption ) throw (::com::sun::star::uno::RuntimeException)
{
    m_xProps->setPropertyValue( LABEL, uno::makeAny( _caption ) );
}

sal_Bool SAL_CALL ScVbaButton::getAutoSize() throw (uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL ScVbaButton::setAutoSize( sal_Bool /*bAutoSize*/ ) throw (uno::RuntimeException)
{
}

sal_Bool SAL_CALL ScVbaButton::getCancel() throw (uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL ScVbaButton::setCancel( sal_Bool /*bCancel*/ ) throw (uno::RuntimeException)
{
}

sal_Bool SAL_CALL ScVbaButton::getDefault() throw (uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL ScVbaButton::setDefault( sal_Bool /*bDefault*/ ) throw (uno::RuntimeException)
{
}

sal_Int32 SAL_CALL ScVbaButton::getBackColor() throw (uno::RuntimeException)
{
    return 0;
}

void SAL_CALL ScVbaButton::setBackColor( sal_Int32 /*nBackColor*/ ) throw (uno::RuntimeException)
{
}

sal_Int32 SAL_CALL ScVbaButton::getForeColor() throw (uno::RuntimeException)
{
    return 0;
}

void SAL_CALL ScVbaButton::setForeColor( sal_Int32 /*nForeColor*/ ) throw (uno::RuntimeException)
{
}

uno::Reference< msforms::XNewFont > SAL_CALL ScVbaButton::getFont() throw (uno::RuntimeException)
{
    return new VbaNewFont( this, mxContext, m_xProps );
}

rtl::OUString&
ScVbaButton::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaButton") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaButton::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.msforms.Button" ) );
    }
    return aServiceNames;
}
