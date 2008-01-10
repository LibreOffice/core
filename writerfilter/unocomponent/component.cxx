/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: component.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 12:18:23 $
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

#ifndef INCLUDED_OOXML_SCANNERTESTSERVICE_HXX
#include <debugservices/ooxml/OOXMLTestService.hxx>
#endif

#ifndef INCLUDED_OOXML_ANALYZE_SERVICE_HXX
#include <debugservices/ooxml/OOXMLAnalyzeService.hxx>
#endif

#include <stdio.h>

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
    OOXML_SCANNERTESTSERVICE_COMPONENT_ENTRY, /* debugservices.ooxml.ScannerTestService */
    OOXML_ANALYZESERVICE_COMPONENT_ENTRY, /* debugservices.ooxml.AnalyzeService */
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
    fprintf(stderr, "Loading service: %s: ", implName);

    void * pResult = ::cppu::component_getFactoryHelper(implName, xMgr, xRegistry, s_component_entries );

    fprintf(stderr, "%p\n", pResult);

    return pResult;
}

}
