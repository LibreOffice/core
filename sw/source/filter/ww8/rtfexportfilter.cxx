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



#include <rtfexportfilter.hxx>
#include <rtfexport.hxx>
#include <rtfimportfilter.hxx>

#include <docsh.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <unotxdoc.hxx>

#include <cppuhelper/factory.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <unotools/ucbstreamhelper.hxx>

using namespace ::comphelper;
using namespace ::com::sun::star;
using ::rtl::OUString;

RtfExportFilter::RtfExportFilter( const uno::Reference< lang::XMultiServiceFactory >& xMSF)  :
    m_xMSF( xMSF )
{
}

RtfExportFilter::~RtfExportFilter()
{
}

sal_Bool RtfExportFilter::filter( const uno::Sequence< beans::PropertyValue >& aDescriptor )
    throw (uno::RuntimeException)
{
    OSL_TRACE("%s", OSL_THIS_FUNC);

    MediaDescriptor aMediaDesc = aDescriptor;
    ::uno::Reference< io::XStream > xStream =
        aMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_STREAMFOROUTPUT(), uno::Reference< io::XStream >() );
    m_pStream = utl::UcbStreamHelper::CreateStream( xStream, sal_True );
    m_aWriter.SetStream(m_pStream);

    // get SwDoc*
    uno::Reference< uno::XInterface > xIfc( m_xSrcDoc, uno::UNO_QUERY );
    SwXTextDocument *pTxtDoc = dynamic_cast< SwXTextDocument * >( xIfc.get() );
    if ( !pTxtDoc ) {
        return sal_False;
    }

    SwDoc *pDoc = pTxtDoc->GetDocShell()->GetDoc();
    if ( !pDoc ) {
        return sal_False;
    }

    // get SwPaM*
    // we get SwPaM for the entire document; copy&paste is handled internally, not via UNO
    SwPaM aPam( pDoc->GetNodes().GetEndOfContent() );
    aPam.SetMark();
    aPam.Move( fnMoveBackward, fnGoDoc );

    SwPaM *pCurPam = new SwPaM( *aPam.End(), *aPam.Start() );

    // export the document
    // (in a separate block so that it's destructed before the commit)
    {
        RtfExport aExport( this, pDoc, pCurPam, &aPam, NULL );
        aExport.ExportDocument( true );
    }

    // delete the pCurPam
    if ( pCurPam )
    {
        while ( pCurPam->GetNext() != pCurPam )
            delete pCurPam->GetNext();
        delete pCurPam;
    }
    delete m_pStream;

    return sal_True;
}


void RtfExportFilter::cancel(  ) throw (uno::RuntimeException)
{
}

void RtfExportFilter::setSourceDocument( const uno::Reference< lang::XComponent >& xDoc )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    m_xSrcDoc = xDoc;
}

//////////////////////////////////////////////////////////////////////////
// UNO helpers
//////////////////////////////////////////////////////////////////////////

OUString RtfExport_getImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( IMPL_NAME_RTFEXPORT ) );
}

uno::Sequence< OUString > SAL_CALL RtfExport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.ExportFilter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL RtfExport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    return (cppu::OWeakObject*) new RtfExportFilter( rSMgr );
}
#ifdef __cplusplus
extern "C"
{
#endif

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

// ------------------------
// - component_getFactory -
// ------------------------

SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* /* pRegistryKey */ )
{
    OSL_TRACE("%s, pImplName is '%s'", OSL_THIS_FUNC, pImplName);
    uno::Reference< lang::XSingleServiceFactory > xFactory;
    void* pRet = 0;

    if ( rtl_str_compare( pImplName, IMPL_NAME_RTFEXPORT ) == 0 ) {
        const OUString aServiceName( OUString::createFromAscii( IMPL_NAME_RTFEXPORT ) );

        xFactory = uno::Reference< lang::XSingleServiceFactory >( ::cppu::createSingleFactory(
                    reinterpret_cast< lang::XMultiServiceFactory* >( pServiceManager ),
                    RtfExport_getImplementationName(),
                    RtfExport_createInstance,
                    RtfExport_getSupportedServiceNames() ) );
    }
    else if ( rtl_str_compare( pImplName, IMPL_NAME_RTFIMPORT ) == 0 ) {
        const OUString aServiceName( OUString::createFromAscii( IMPL_NAME_RTFIMPORT ) );

        xFactory = uno::Reference< lang::XSingleServiceFactory >( ::cppu::createSingleFactory(
                    reinterpret_cast< lang::XMultiServiceFactory* >( pServiceManager ),
                    RtfImport_getImplementationName(),
                    RtfImport_createInstance,
                    RtfImport_getSupportedServiceNames() ) );
    }

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

#ifdef __cplusplus
}
#endif

/* vi:set shiftwidth=4 expandtab: */
