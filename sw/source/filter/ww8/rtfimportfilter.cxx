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



#include "rtfimportfilter.hxx"
#include "../rtf/swparrtf.hxx"

#include <docsh.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <unotxdoc.hxx>
#include <swerror.h>

#include <cppuhelper/factory.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>

using namespace ::comphelper;
using namespace ::com::sun::star;
using ::rtl::OUString;
using rtl::OUStringToOString;

RtfImportFilter::RtfImportFilter( const uno::Reference< lang::XMultiServiceFactory >& xMSF)  :
    m_xMSF( xMSF )
{
}

RtfImportFilter::~RtfImportFilter()
{
}

sal_Bool RtfImportFilter::filter( const uno::Sequence< beans::PropertyValue >& aDescriptor )
    throw (uno::RuntimeException)
{
    OSL_TRACE("%s", OSL_THIS_FUNC);

    MediaDescriptor aMediaDesc = aDescriptor;
    ::uno::Reference< io::XInputStream > xInputStream =
        aMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_INPUTSTREAM(), uno::Reference< io::XInputStream >() );
    SvStream* pStream = utl::UcbStreamHelper::CreateStream( xInputStream, sal_True );
    if (!pStream)
        return sal_False;

    // get SwDoc*
    uno::Reference< uno::XInterface > xIfc( m_xDstDoc, uno::UNO_QUERY );
    SwXTextDocument *pTxtDoc = dynamic_cast< SwXTextDocument * >( xIfc.get() );
    if (!pTxtDoc)
        return sal_False;
    SwDoc *pDoc = pTxtDoc->GetDocShell()->GetDoc();
    if (!pDoc)
        return sal_False;

    // get SwPaM*
    // NEEDSWORK should we care about partial imports? For now we just import
    // the whole document
    SwPaM aPam( pDoc->GetNodes().GetEndOfContent() );
    aPam.SetMark();
    aPam.Move( fnMoveBackward, fnGoDoc );
    SwPaM *pCurPam = new SwPaM( *aPam.End(), *aPam.Start() );

    String aURL;
    OUString sTemp;
    for ( sal_Int32 i = 0; i < aDescriptor.getLength(); i++ )
    {
        if( aDescriptor[i].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("URL")) )
        {
            aDescriptor[i].Value >>= sTemp;
            aURL = sTemp;
        }
    }

    RtfReader aReader;
    sal_Bool bRet = aReader.Read(pStream, *pDoc, aURL, *pCurPam) == 0;
    delete pStream;
    return bRet;
}


void RtfImportFilter::cancel(  ) throw (uno::RuntimeException)
{
}

void RtfImportFilter::setTargetDocument( const uno::Reference< lang::XComponent >& xDoc )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    m_xDstDoc = xDoc;
}

//////////////////////////////////////////////////////////////////////////
// UNO helpers
//////////////////////////////////////////////////////////////////////////

OUString RtfImport_getImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( IMPL_NAME_RTFIMPORT ) );
}

uno::Sequence< OUString > SAL_CALL RtfImport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.ImportFilter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL RtfImport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    return (cppu::OWeakObject*) new RtfImportFilter( rSMgr );
}

/* vi:set shiftwidth=4 expandtab: */
