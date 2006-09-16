/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: statusbarconfiguration.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 14:31:12 $
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


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#ifndef __FRAMEWORK_CLASSES_STATUSBARCONFIGURATION_HXX_
#include <xml/statusbarconfiguration.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_STATUSBARDOCUMENTHANDLER_HXX_
#include <xml/statusbardocumenthandler.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_SAXNAMESPACEFILTER_HXX_
#include <xml/saxnamespacefilter.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif

#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::container;


namespace framework
{

SV_IMPL_PTRARR( StatusBarDescriptor, StatusBarItemDescriptorPtr);

static Reference< XParser > GetSaxParser(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory
    )
{
    return Reference< XParser >( xServiceFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.xml.sax.Parser" )), UNO_QUERY);
}

static Reference< XDocumentHandler > GetSaxWriter(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory
    )
{
    return Reference< XDocumentHandler >( xServiceFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.xml.sax.Writer" )), UNO_QUERY) ;
}

sal_Bool StatusBarConfiguration::LoadStatusBar(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&,
    SvStream&, StatusBarDescriptor& )
{
    // obsolete - only support linkage of binary filters!
    return sal_True;
}

sal_Bool StatusBarConfiguration::StoreStatusBar(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&,
    SvStream&, const StatusBarDescriptor& )
{
    // obsolete - only support linkage of binary filters!
    return sal_True;
}

sal_Bool StatusBarConfiguration::LoadStatusBar(
    const Reference< XMultiServiceFactory >& xServiceFactory,
    const Reference< XInputStream >& xInputStream,
    const Reference< XIndexContainer >& rStatusbarConfiguration )
{
    Reference< XParser > xParser( GetSaxParser( xServiceFactory ) );

    // connect stream to input stream to the parser
    InputSource aInputSource;
    aInputSource.aInputStream = xInputStream;

    // create namespace filter and set menudocument handler inside to support xml namespaces
    Reference< XDocumentHandler > xDocHandler( new OReadStatusBarDocumentHandler( rStatusbarConfiguration ));
    Reference< XDocumentHandler > xFilter( new SaxNamespaceFilter( xDocHandler ));

    // connect parser and filter
    xParser->setDocumentHandler( xFilter );

    try
    {
        xParser->parseStream( aInputSource );
        return sal_True;
    }
    catch ( RuntimeException& )
    {
        return sal_False;
    }
    catch( SAXException& )
    {
        return sal_False;
    }
    catch( ::com::sun::star::io::IOException& )
    {
        return sal_False;
    }
}

sal_Bool StatusBarConfiguration::StoreStatusBar(
    const Reference< XMultiServiceFactory >& xServiceFactory,
    const Reference< XOutputStream >& xOutputStream,
    const Reference< XIndexAccess >& rStatusbarConfiguration )
{
    Reference< XDocumentHandler > xWriter( GetSaxWriter( xServiceFactory ) );
    Reference< ::com::sun::star::io::XActiveDataSource> xDataSource( xWriter , UNO_QUERY );
    xDataSource->setOutputStream( xOutputStream );

    try
    {
        OWriteStatusBarDocumentHandler aWriteStatusBarDocumentHandler( rStatusbarConfiguration, xWriter );
        aWriteStatusBarDocumentHandler.WriteStatusBarDocument();
        return sal_True;
    }
    catch ( RuntimeException& )
    {
        return sal_False;
    }
    catch ( SAXException& )
    {
        return sal_False;
    }
    catch ( ::com::sun::star::io::IOException& )
    {
        return sal_False;
    }
}
}
