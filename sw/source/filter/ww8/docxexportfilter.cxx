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



#include "docxexportfilter.hxx"
#include "rtfexportfilter.hxx"
#include "rtfimportfilter.hxx"
#include "docxexport.hxx"

#include <docsh.hxx>
#include <pam.hxx>
#include <unotxdoc.hxx>

#include <cppuhelper/factory.hxx>

using namespace ::comphelper;
using namespace ::com::sun::star;
using ::rtl::OUString;

#define S( x ) OUString( RTL_CONSTASCII_USTRINGPARAM( x ) )

DocxExportFilter::DocxExportFilter( const uno::Reference< lang::XMultiServiceFactory >& rMSF )
    : oox::core::XmlFilterBase( rMSF )
{
}

bool DocxExportFilter::exportDocument()
{
    OSL_TRACE(, "DocxExportFilter::exportDocument()\n" ); // DEBUG remove me

    // get SwDoc*
    uno::Reference< uno::XInterface > xIfc( getModel(), uno::UNO_QUERY );
    SwXTextDocument *pTxtDoc = dynamic_cast< SwXTextDocument * >( xIfc.get() );
    if ( !pTxtDoc )
        return false;

    SwDoc *pDoc = pTxtDoc->GetDocShell()->GetDoc();
    if ( !pDoc )
        return false;

    // get SwPaM*
    // FIXME so far we get SwPaM for the entire document; probably we should
    // be able to output just the selection as well - though no idea how to
    // get the correct SwPaM* then...
    SwPaM aPam( pDoc->GetNodes().GetEndOfContent() );
    aPam.SetMark();
    aPam.Move( fnMoveBackward, fnGoDoc );

    SwPaM *pCurPam = new SwPaM( *aPam.End(), *aPam.Start() );

    // export the document
    // (in a separate block so that it's destructed before the commit)
    {
        DocxExport aExport( this, pDoc, pCurPam, &aPam );
        aExport.ExportDocument( true ); // FIXME support exporting selection only
    }

    commit();

    // delete the pCurPam
    if ( pCurPam )
    {
        while ( pCurPam->GetNext() != pCurPam )
            delete pCurPam->GetNext();
        delete pCurPam;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
// UNO stuff so that the filter is registered
//////////////////////////////////////////////////////////////////////////

#define IMPL_NAME "com.sun.star.comp.Writer.DocxExport"

OUString DocxExport_getImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( IMPL_NAME ) );
}

OUString DocxExportFilter::implGetImplementationName() const
{
    return DocxExport_getImplementationName();
}

uno::Sequence< OUString > SAL_CALL DocxExport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.ExportFilter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL DocxExport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    return (cppu::OWeakObject*) new DocxExportFilter( rSMgr );
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

    if ( rtl_str_compare( pImplName, IMPL_NAME ) == 0 )
    {
        const OUString aServiceName( OUString::createFromAscii( IMPL_NAME ) );

        xFactory = uno::Reference< lang::XSingleServiceFactory >( ::cppu::createSingleFactory(
                    reinterpret_cast< lang::XMultiServiceFactory* >( pServiceManager ),
                    DocxExport_getImplementationName(),
                    DocxExport_createInstance,
                    DocxExport_getSupportedServiceNames() ) );
    } else if ( rtl_str_compare( pImplName, IMPL_NAME_RTFEXPORT ) == 0 ) {
        const OUString aServiceName( OUString::createFromAscii( IMPL_NAME_RTFEXPORT ) );

        xFactory = uno::Reference< lang::XSingleServiceFactory >( ::cppu::createSingleFactory(
                    reinterpret_cast< lang::XMultiServiceFactory* >( pServiceManager ),
                    RtfExport_getImplementationName(),
                    RtfExport_createInstance,
                    RtfExport_getSupportedServiceNames() ) );
    } else if ( rtl_str_compare( pImplName, IMPL_NAME_RTFIMPORT ) == 0 ) {
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

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
