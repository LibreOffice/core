/*************************************************************************
 *
 *  $RCSfile: xmlimp.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: sab $ $Date: 2000-11-16 18:16:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#include <rtl/uuid.h>
#include <rtl/memory.h>


#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_XMLNUMFI_HXX
#include "xmlnumfi.hxx"
#endif

#ifndef _XMLOFF_PROGRESSBARHELPER_HXX
#include "ProgressBarHelper.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_SERVICENOTREGISTEREDEXCEPTION_HDL_
#include <com/sun/star/lang/ServiceNotRegisteredException.hdl>
#endif

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;

sal_Char __READONLY_DATA sXML_np__office[] = "_office";
sal_Char __READONLY_DATA sXML_np__style[] = "_style";
sal_Char __READONLY_DATA sXML_np__text[] = "_text";
sal_Char __READONLY_DATA sXML_np__table[] = "_table";
sal_Char __READONLY_DATA sXML_np__draw[] = "_draw";
sal_Char __READONLY_DATA sXML_np__fo[] = "_fo";
sal_Char __READONLY_DATA sXML_np__xlink[] = "_xlink";
sal_Char __READONLY_DATA sXML_np__dc[] = "_dc";
sal_Char __READONLY_DATA sXML_np__meta[] = "_meta";
sal_Char __READONLY_DATA sXML_np__number[] = "_number";
sal_Char __READONLY_DATA sXML_np__svg[] = "_svg";
sal_Char __READONLY_DATA sXML_np__chart[] = "_chart";
sal_Char __READONLY_DATA sXML_np__math[] = "_math";

sal_Char __READONLY_DATA sXML_np__fo_old[] = "__fo";
sal_Char __READONLY_DATA sXML_np__xlink_old[] = "__xlink";
sal_Char __READONLY_DATA sXML_np__office_old[] = "__office";
sal_Char __READONLY_DATA sXML_np__style_old[] = "__style";
sal_Char __READONLY_DATA sXML_np__text_old[] = "__text";
sal_Char __READONLY_DATA sXML_np__table_old[] = "__table";
sal_Char __READONLY_DATA sXML_np__meta_old[] = "__meta";



typedef SvXMLImportContext *SvXMLImportContextPtr;
SV_DECL_PTRARR( SvXMLImportContexts_Impl, SvXMLImportContextPtr, 20, 5 )
SV_IMPL_PTRARR( SvXMLImportContexts_Impl, SvXMLImportContextPtr )

SvXMLImportContext *SvXMLImport::CreateContext( USHORT nPrefix,
                                         const OUString& rLocalName,
                                         const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    return new SvXMLImportContext( *this, nPrefix, rLocalName );
}

void SvXMLImport::_InitCtor()
{
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_OFFICE_IDX, sXML_np__office,
                               sXML_n_office, XML_NAMESPACE_OFFICE );
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_STYLE_IDX, sXML_np__style,
                               sXML_n_style, XML_NAMESPACE_STYLE );
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_TEXT_IDX, sXML_np__text,
                               sXML_n_text, XML_NAMESPACE_TEXT );
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_TABLE_IDX, sXML_np__table,
                               sXML_n_table, XML_NAMESPACE_TABLE );
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_DRAW_IDX, sXML_np__draw,
                               sXML_n_draw, XML_NAMESPACE_DRAW );
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_FO_IDX, sXML_np__fo,
                               sXML_n_fo, XML_NAMESPACE_FO );
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_XLINK_IDX, sXML_np__xlink,
                               sXML_n_xlink, XML_NAMESPACE_XLINK );
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_DC_IDX, sXML_np__dc,
                               sXML_n_dc, XML_NAMESPACE_DC );
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_META_IDX, sXML_np__meta,
                               sXML_n_meta, XML_NAMESPACE_META );
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_NUMBER_IDX, sXML_np__number,
                                sXML_n_number, XML_NAMESPACE_NUMBER );
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_SVG_IDX, sXML_np__svg,
                               sXML_n_svg, XML_NAMESPACE_SVG );
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_CHART_IDX, sXML_np__chart,
                               sXML_n_chart, XML_NAMESPACE_CHART );
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_MATH_IDX, sXML_np__math,
                               sXML_n_math, XML_NAMESPACE_MATH );


    // namespaces used in the technical preview (SO 5.2)
    pNamespaceMap->AddAtIndex( XML_OLD_NAMESPACE_FO_IDX, sXML_np__fo_old,
                               sXML_n_fo_old, XML_NAMESPACE_FO );
    pNamespaceMap->AddAtIndex( XML_OLD_NAMESPACE_XLINK_IDX, sXML_np__xlink_old,
                               sXML_n_xlink_old, XML_NAMESPACE_XLINK );
    pNamespaceMap->AddAtIndex( XML_OLD_NAMESPACE_OFFICE_IDX, sXML_np__office,
                               sXML_n_office_old, XML_NAMESPACE_OFFICE );
    pNamespaceMap->AddAtIndex( XML_OLD_NAMESPACE_STYLE_IDX, sXML_np__style_old,
                               sXML_n_style_old, XML_NAMESPACE_STYLE );
    pNamespaceMap->AddAtIndex( XML_OLD_NAMESPACE_TEXT_IDX, sXML_np__text_old,
                               sXML_n_text_old, XML_NAMESPACE_TEXT );
    pNamespaceMap->AddAtIndex( XML_OLD_NAMESPACE_TABLE_IDX, sXML_np__table_old,
                               sXML_n_table_old, XML_NAMESPACE_TABLE );
    pNamespaceMap->AddAtIndex( XML_OLD_NAMESPACE_META_IDX, sXML_np__meta_old,
                               sXML_n_meta_old, XML_NAMESPACE_META );
}

SvXMLImport::SvXMLImport() throw () :
    pImpl( 0 ),
    pNamespaceMap( new SvXMLNamespaceMap ),
    pUnitConv( new SvXMLUnitConverter( MAP_100TH_MM, MAP_100TH_MM ) ),
    pContexts( new SvXMLImportContexts_Impl ),
    pNumImport( NULL ),
    pProgressBarHelper( NULL )
{
    _InitCtor();
}

SvXMLImport::SvXMLImport( const Reference< XModel > & rModel ) throw () :
    pImpl( 0 ),
    pNamespaceMap( new SvXMLNamespaceMap ),
    pUnitConv( new SvXMLUnitConverter( MAP_100TH_MM, MAP_100TH_MM ) ),
    pContexts( new SvXMLImportContexts_Impl ),
    pNumImport( NULL ),
    xModel( rModel ),
    xNumberFormatsSupplier (rModel, uno::UNO_QUERY),
    pProgressBarHelper( new ProgressBarHelper(rModel, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("XML Import"))))
{
    _InitCtor();
    if (xNumberFormatsSupplier.is())
        pNumImport = new SvXMLNumFmtHelper(xNumberFormatsSupplier);
}

SvXMLImport::~SvXMLImport() throw ()
{
    delete pNamespaceMap;
    delete pUnitConv;
    delete pContexts;
    if (pNumImport)
        delete pNumImport;
    if (pProgressBarHelper)
        delete pProgressBarHelper;
}

// XUnoTunnel & co
const uno::Sequence< sal_Int8 > & SvXMLImport::getUnoTunnelId() throw()
{
    static uno::Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        Guard< Mutex > aGuard( Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

SvXMLImport* SvXMLImport::getImplementation( uno::Reference< uno::XInterface > xInt ) throw()
{
    uno::Reference< lang::XUnoTunnel > xUT( xInt, uno::UNO_QUERY );
    if( xUT.is() )
        return (SvXMLImport*)xUT->getSomething( SvXMLImport::getUnoTunnelId() );
    else
        return NULL;
}

// XUnoTunnel
sal_Int64 SAL_CALL SvXMLImport::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw( uno::RuntimeException )
{
    if( rId.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                                         rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    return 0;
}

void SAL_CALL SvXMLImport::startDocument( void )
    throw( xml::sax::SAXException, uno::RuntimeException )
{
}

void SAL_CALL SvXMLImport::endDocument( void )
    throw( xml::sax::SAXException, uno::RuntimeException)
{
}

void SAL_CALL SvXMLImport::startElement( const OUString& rName,
                                         const uno::Reference< xml::sax::XAttributeList >& xAttrList )
    throw(xml::sax::SAXException, uno::RuntimeException)
{
    SvXMLNamespaceMap *pRewindMap = 0;

    // Process namespace attributes. This must happen before creating the
    // context, because namespace decaration apply to the element name itself.
    INT16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( INT16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        if( rAttrName.compareToAscii( sXML_xmlns, 5 ) == 0 &&
            ( rAttrName.getLength() == 5 || ':' == rAttrName[5] ) )
        {
            if( !pRewindMap )
            {
                pRewindMap = pNamespaceMap;
                pNamespaceMap = new SvXMLNamespaceMap( *pNamespaceMap );
            }
            const OUString& rAttrValue = xAttrList->getValueByIndex( i );
            if( rAttrName.getLength() == 5 )
            {
//              pNamespaceMap->SetDefault( aName );
            }
            else
            {
                OUString aPrefix( rAttrName.copy( 6 ) );
                pNamespaceMap->Add( aPrefix, rAttrValue );
            }
        }
    }

    // Get element's namespace and local name.
    OUString aLocalName;
    USHORT nPrefix =
        pNamespaceMap->GetKeyByAttrName( rName, &aLocalName );

    // If there are contexts already, call a CreateChildContext at the topmost
    // context. Otherwise, create a default context.
    SvXMLImportContext *pContext;
    USHORT nCount = pContexts->Count();
    if( nCount > 0 )
    {
        pContext = (*pContexts)[nCount - 1]->CreateChildContext( nPrefix,
                                                                 aLocalName,
                                                                 xAttrList );
    }
    else
    {
        pContext = CreateContext( nPrefix, aLocalName, xAttrList );
    }

    DBG_ASSERT( pContext, "SvXMLImport::startElement: missing context" );
    if( !pContext )
        pContext = new SvXMLImportContext( *this, nPrefix, aLocalName );

    pContext->AddRef();

    // Remeber old namespace map.
    if( pRewindMap )
        pContext->SetRewindMap( pRewindMap );

    // Call a startElement at the new context.
    pContext->StartElement( xAttrList );

    // Push context on stack.
    pContexts->Insert( pContext, nCount );
}

void SAL_CALL SvXMLImport::endElement( const OUString& rName )
    throw(xml::sax::SAXException, uno::RuntimeException)
{
    USHORT nCount = pContexts->Count();
    DBG_ASSERT( nCount, "SvXMLImport::endElement: no context left" );
    if( nCount > 0 )
    {
        // Get topmost context and remove it from the stack.
        SvXMLImportContext *pContext = (*pContexts)[nCount-1];
        pContexts->Remove( nCount-1, 1 );

#ifndef PRODUCT
        // Non product only: check if endElement call matches startELement call.
        OUString aLocalName;
        USHORT nPrefix =
            pNamespaceMap->GetKeyByAttrName( rName, &aLocalName );
        DBG_ASSERT( pContext->GetPrefix() == nPrefix,
                "SvXMLImport::endElement: popped context has wrong prefix" );
        DBG_ASSERT( pContext->GetLocalName() == aLocalName,
                "SvXMLImport::endElement: popped context has wrong lname" );
#endif

        // Call a EndElement at the current context.
        pContext->EndElement();

        // Get a namespace map to rewind.
        SvXMLNamespaceMap *pRewindMap = pContext->GetRewindMap();

        // Delete the current context.
        pContext->ReleaseRef();
        pContext = 0;

        // Rewind a namespace map.
        if( pRewindMap )
        {
            delete pNamespaceMap;
            pNamespaceMap = pRewindMap;
        }
    }
}

void SAL_CALL SvXMLImport::characters( const OUString& rChars )
    throw(xml::sax::SAXException, uno::RuntimeException)
{
    USHORT nCount = pContexts->Count();
    if( nCount > 0 )
    {
        (*pContexts)[nCount - 1]->Characters( rChars );
    }
}

void SAL_CALL SvXMLImport::ignorableWhitespace( const OUString& rWhitespaces )
    throw(xml::sax::SAXException, uno::RuntimeException)
{
}

void SAL_CALL SvXMLImport::processingInstruction( const OUString& rTarget,
                                       const OUString& rData )
    throw(xml::sax::SAXException, uno::RuntimeException)
{
}

void SAL_CALL SvXMLImport::setDocumentLocator( const uno::Reference< xml::sax::XLocator >& rLocator )
    throw(xml::sax::SAXException, uno::RuntimeException)
{
    xLocator = rLocator;
}

// XExtendedDocumentHandler
void SAL_CALL SvXMLImport::startCDATA( void ) throw(xml::sax::SAXException, uno::RuntimeException)
{
}

void SAL_CALL SvXMLImport::endCDATA( void ) throw(uno::RuntimeException)
{
}

void SAL_CALL SvXMLImport::comment( const OUString& rComment )
    throw(xml::sax::SAXException, uno::RuntimeException)
{
}

void SAL_CALL SvXMLImport::allowLineBreak( void )
    throw(xml::sax::SAXException, uno::RuntimeException)
{
}

void SAL_CALL SvXMLImport::unknown( const OUString& sString )
    throw(xml::sax::SAXException, uno::RuntimeException)
{
}

void SvXMLImport::SetStatisticAttributes(const uno::Reference< xml::sax::XAttributeList > & xAttribs)
{
}

XMLTextImportHelper* SvXMLImport::CreateTextImport()
{
    return new XMLTextImportHelper( xModel );
}

XMLShapeImportHelper* SvXMLImport::CreateShapeImport()
{
    return new XMLShapeImportHelper( xModel );
}

SchXMLImportHelper* SvXMLImport::CreateChartImport()
{
    return new SchXMLImportHelper();
}

///////////////////////////////////////////////////////////////////////////////
//
// Get or create fill/line/lineend-style-helper
//

const Reference< container::XNameContainer > & SvXMLImport::GetGradientHelper()
{
    if( !xGradientHelper.is() )
    {
        if( xModel.is() )
        {
            Reference< lang::XMultiServiceFactory > xServiceFact( xModel, UNO_QUERY);
            if( xServiceFact.is() )
            {
                try
                {
                    xGradientHelper =  Reference< container::XNameContainer >( xServiceFact->createInstance(
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.GradientTable" ) ) ), UNO_QUERY);
                }
                catch( lang::ServiceNotRegisteredException& )
                {}
            }
        }
    }

    return xGradientHelper;
}

const Reference< container::XNameContainer > & SvXMLImport::GetHatchHelper()
{
    if( !xHatchHelper.is() )
    {
        if( xModel.is() )
        {
            Reference< lang::XMultiServiceFactory > xServiceFact( xModel, UNO_QUERY);
            if( xServiceFact.is() )
            {
                try
                {
                    xHatchHelper =  Reference< container::XNameContainer >( xServiceFact->createInstance(
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.HatchTable" ) ) ), UNO_QUERY);
                }
                catch( lang::ServiceNotRegisteredException& )
                {}
            }
        }
    }

    return xHatchHelper;
}

const Reference< container::XNameContainer > & SvXMLImport::GetBitmapHelper()
{
    if( !xBitmapHelper.is() )
    {
        if( xModel.is() )
        {
            Reference< lang::XMultiServiceFactory > xServiceFact( xModel, UNO_QUERY);
            if( xServiceFact.is() )
            {
                try
                {
                    xBitmapHelper =  Reference< container::XNameContainer >( xServiceFact->createInstance(
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.BitmapTable" ) ) ), UNO_QUERY);
                }
                catch( lang::ServiceNotRegisteredException& )
                {}
            }
        }
    }

    return xBitmapHelper;
}

const Reference< container::XNameContainer > & SvXMLImport::GetTransGradientHelper()
{
    if( !xTransGradientHelper.is() )
    {
        if( xModel.is() )
        {
            Reference< lang::XMultiServiceFactory > xServiceFact( xModel, UNO_QUERY);
            if( xServiceFact.is() )
            {
                try
                {
                    xTransGradientHelper =  Reference< container::XNameContainer >( xServiceFact->createInstance(
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.TransparencyGradientTable" ) ) ), UNO_QUERY);
                }
                catch( lang::ServiceNotRegisteredException& )
                {}
            }
        }
    }

    return xTransGradientHelper;
}

const Reference< container::XNameContainer > & SvXMLImport::GetMarkerHelper()
{
    if( !xMarkerHelper.is() )
    {
        if( xModel.is() )
        {
            Reference< lang::XMultiServiceFactory > xServiceFact( xModel, UNO_QUERY);
            if( xServiceFact.is() )
            {
                try
                {
                    xMarkerHelper =  Reference< container::XNameContainer >( xServiceFact->createInstance(
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.MarkerTable" ) ) ), UNO_QUERY);
                }
                catch( lang::ServiceNotRegisteredException& )
                {}
            }
        }
    }

    return xMarkerHelper;
}

const Reference< container::XNameContainer > & SvXMLImport::GetDashHelper()
{
    if( !xDashHelper.is() )
    {
        if( xModel.is() )
        {
            Reference< lang::XMultiServiceFactory > xServiceFact( xModel, UNO_QUERY);
            if( xServiceFact.is() )
            {
                try
                {
                    xDashHelper =  Reference< container::XNameContainer >( xServiceFact->createInstance(
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.DashTable" ) ) ), UNO_QUERY);
                }
                catch( lang::ServiceNotRegisteredException& )
                {}
            }
        }
    }

    return xDashHelper;
}
