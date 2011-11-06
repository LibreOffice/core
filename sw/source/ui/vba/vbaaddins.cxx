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


#include "vbaaddins.hxx"
#include "vbaaddin.hxx"
#include <cppuhelper/implbase3.hxx>
#include <unotools/pathoptions.hxx>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

uno::Reference< container::XIndexAccess > lcl_getAddinCollection( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext )
{
    XNamedObjectCollectionHelper< word::XAddin >::XNamedVec maAddins;

    // first get the autoload addins in the directory STARTUP
    uno::Reference< lang::XMultiComponentFactory > xMCF( xContext->getServiceManager(), uno::UNO_QUERY_THROW );
    uno::Reference< ucb::XSimpleFileAccess > xSFA( xMCF->createInstanceWithContext( rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ), xContext), uno::UNO_QUERY_THROW );
    SvtPathOptions aPathOpt;
    // FIXME: temporary the STARTUP path is located in $OO/basic3.1/program/addin
    String aAddinPath = aPathOpt.GetAddinPath();
    OSL_TRACE("lcl_getAddinCollection: %s", rtl::OUStringToOString( aAddinPath, RTL_TEXTENCODING_UTF8 ).getStr() );
    if( xSFA->isFolder( aAddinPath ) )
    {
        uno::Sequence< rtl::OUString > sEntries = xSFA->getFolderContents( aAddinPath, sal_False );
        sal_Int32 nEntry = sEntries.getLength();
        for( sal_Int32 index = 0; index < nEntry; ++index )
        {
            rtl::OUString sUrl = sEntries[ index ];
            if( !xSFA->isFolder( sUrl ) && sUrl.endsWithIgnoreAsciiCaseAsciiL( ".dot", 4 ) )
            {
                maAddins.push_back( uno::Reference< word::XAddin >( new SwVbaAddin( xParent, xContext, sUrl, sal_True ) ) );
            }
        }
    }

    // TODO: second get the customize addins in the org.openoffice.Office.Writer/GlobalTemplateList

    uno::Reference< container::XIndexAccess > xAddinsAccess( new XNamedObjectCollectionHelper< word::XAddin >( maAddins ) );
    return xAddinsAccess;
}

SwVbaAddins::SwVbaAddins( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext ) throw (uno::RuntimeException): SwVbaAddins_BASE( xParent, xContext, lcl_getAddinCollection( xParent,xContext ) )
{
}
// XEnumerationAccess
uno::Type
SwVbaAddins::getElementType() throw (uno::RuntimeException)
{
    return word::XAddin::static_type(0);
}
uno::Reference< container::XEnumeration >
SwVbaAddins::createEnumeration() throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumerationAccess > xEnumerationAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return xEnumerationAccess->createEnumeration();
}

uno::Any
SwVbaAddins::createCollectionObject( const css::uno::Any& aSource )
{
    return aSource;
}

rtl::OUString&
SwVbaAddins::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaAddins") );
    return sImplName;
}

css::uno::Sequence<rtl::OUString>
SwVbaAddins::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Addins") );
    }
    return sNames;
}
