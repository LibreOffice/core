/**
  Copyright 2005 Sun Microsystems, Inc.
  */

#ifndef _CPPUHELPTER_FACTORY_
#include <cppuhelper/factory.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_
#include <cppuhelper/implementationentry.hxx>
#endif

#ifndef INCLUDED_RTFTOK_SCANNERTESTSERVICE_HXX
#include <debugservices/rtftok/ScannerTestService.hxx>
#endif

#ifndef INCLUDED_RTFTOK_XMLSCANNER_HXX
#include <debugservices/rtftok/XMLScanner.hxx>
#endif

#ifndef INCLUDED_XXML_FILEXXMLREADER_HXX
#include <debugservices/xxml/FileXXmlReader.hxx>
#endif

#ifndef INCLUDED_XXML_STORAGEXXMLREADER_HXX
#include <debugservices/xxml/StorageXXmlReader.hxx>
#endif

#ifndef INCLUDED_ODSL_ODSLREADER_HXX
#include <debugservices/odsl/ODSLParser.hxx>
#endif

#ifndef INCLUDED_RTFSL_PARSER_HXX
#include <debugservices/rtfsl/RTFSLParser.hxx>
#endif

#ifndef INCLUDED_DOCTOK_SCANNERTESTSERVICE_HXX
#include <debugservices/doctok/DocTokTestService.hxx>
#endif

#ifndef INCLUDED_DOCTOK_ANALYZESERVICE_HXX
#include <debugservices/doctok/DocTokAnalyzeService.hxx>
#endif

using namespace com::sun::star;


extern "C"
{
/* shared lib exports implemented with helpers */
static struct ::cppu::ImplementationEntry s_component_entries [] =
{//uno -l writerfilter.uno.dll -c debugservices.rtftok.ScannerTestService -- a b c
    RTFTOK_SCANNERTESTSERVICE_COMPONENT_ENTRY, /* debugservices.rtftok.ScannerTestService */
    RTFTOK_XMLSCANNER_COMPONENT_ENTRY, /* debugservices.rtftok.XMLScanner */
    XXML_FILEXXMLREADER_COMPONENT_ENTRY, /* debugservices.xxml.FileXXmlReader */
    XXML_STORAGEXXMLREADER_COMPONENT_ENTRY, /* debugservices.xxml.StorageXXmlReader */
    ODSL_ODSLREADER_COMPONENT_ENTRY, /* debugservices.odsl.ODSLReader */
    RTFSL_PARSER_COMPONENT_ENTRY, /* debugservices.rtfsl.RTFSLParser */
    DOCTOK_SCANNERTESTSERVICE_COMPONENT_ENTRY, /* debugservices.doctok.ScannerTestService */
    DOCTOK_ANALYZESERVICE_COMPONENT_ENTRY, /* debugservices.doctok.AnalyzeService */
    { 0, 0, 0, 0, 0, 0 } // terminate with NULL
};


    void SAL_CALL component_getImplementationEnvironment(const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo( ::com::sun::star::lang::XMultiServiceFactory * xMgr, ::com::sun::star::registry::XRegistryKey * xRegistry )
{
    return ::cppu::component_writeInfoHelper( xMgr, xRegistry, s_component_entries );
}


void * SAL_CALL component_getFactory(sal_Char const * implName, ::com::sun::star::lang::XMultiServiceFactory * xMgr, ::com::sun::star::registry::XRegistryKey * xRegistry )
{
    return ::cppu::component_getFactoryHelper(implName, xMgr, xRegistry, s_component_entries );
}

}
