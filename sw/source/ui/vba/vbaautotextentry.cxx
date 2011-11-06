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


#include "vbaautotextentry.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include "vbarange.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaAutoTextEntry::SwVbaAutoTextEntry( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< text::XAutoTextEntry >& xEntry ) throw ( uno::RuntimeException ) :
    SwVbaAutoTextEntry_BASE( rParent, rContext ), mxEntry( xEntry )
{
}

SwVbaAutoTextEntry::~SwVbaAutoTextEntry()
{
}

uno::Reference< word::XRange > SAL_CALL SwVbaAutoTextEntry::Insert( const uno::Reference< word::XRange >& _where, const uno::Any& /*_richtext*/ ) throw ( uno::RuntimeException )
{
    SwVbaRange* pWhere = dynamic_cast<SwVbaRange*>( _where.get() );
    if( pWhere )
    {
        uno::Reference< text::XTextRange > xTextRange = pWhere->getXTextRange();
        xTextRange->setString( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("x") ) ); // set marker
        uno::Reference< text::XTextRange > xEndMarker = xTextRange->getEnd();
        xEndMarker->setString( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("x") ) ); // set marker
        uno::Reference< text::XText > xText = pWhere->getXText();
        mxEntry->applyTo( xEndMarker->getStart() );
        uno::Reference< text::XTextCursor > xTC = xText->createTextCursorByRange( xTextRange->getStart() );
        xTC->goRight( 1, sal_True );
        xTC->setString( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("") ) ); // remove marker
        xEndMarker->setString( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("") ) ); // remove marker
        xTC->gotoRange( xEndMarker, sal_True );
        pWhere->setXTextCursor( xTC );
    }
    return uno::Reference< word::XRange >( pWhere );
}

rtl::OUString&
SwVbaAutoTextEntry::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaAutoTextEntry") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaAutoTextEntry::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.AutoTextEntry" ) );
    }
    return aServiceNames;
}


SwVbaAutoTextEntries::SwVbaAutoTextEntries( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext, const uno::Reference< container::XIndexAccess >& xIndexAccess ) throw (uno::RuntimeException) : SwVbaAutoTextEntries_BASE( xParent, xContext, xIndexAccess ), mxAutoTextEntryAccess( xIndexAccess )
{
}

// XEnumerationAccess
uno::Type
SwVbaAutoTextEntries::getElementType() throw (uno::RuntimeException)
{
    return word::XAutoTextEntry::static_type(0);
}
uno::Reference< container::XEnumeration >
SwVbaAutoTextEntries::createEnumeration() throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumerationAccess > xEnumerationAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return xEnumerationAccess->createEnumeration();
}

uno::Any
SwVbaAutoTextEntries::createCollectionObject( const css::uno::Any& aSource )
{
    uno::Reference< text::XAutoTextEntry > xEntry( aSource, uno::UNO_QUERY_THROW );
    return uno::makeAny( uno::Reference< word::XAutoTextEntry >( new SwVbaAutoTextEntry( this, mxContext, xEntry ) ) );
}

rtl::OUString&
SwVbaAutoTextEntries::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaAutoTextEntries") );
    return sImplName;
}

css::uno::Sequence<rtl::OUString>
SwVbaAutoTextEntries::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.AutoTextEntries") );
    }
    return sNames;
}
