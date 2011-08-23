/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#include <rtl/uuid.h>
#include <rtl/memory.h>

#ifndef _SVARRAY_HXX
#include <bf_svtools/svarray.hxx>
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
#ifndef _XMLOFF_XMLFONTSTYLESCONTEXT_HXX_
#include "XMLFontStylesContext.hxx"
#endif


#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_XMLNUMFI_HXX
#include "xmlnumfi.hxx"
#endif
#ifndef _XMLOFF_XMLEVENTIMPORTHELPER_HXX
#include "XMLEventImportHelper.hxx"
#endif
#ifndef _XMLOFF_XMLSTARBASICCONTEXTFACTORY_HXX
#include "XMLStarBasicContextFactory.hxx"
#endif
#ifndef _XMLOFF_XMLSCRIPTCONTEXTFACTORY_HXX
#include "XMLScriptContextFactory.hxx"
#endif



#ifndef _XMLOFF_XMLERROR_HXX
#include "xmlerror.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_SERVICENOTREGISTEREDEXCEPTION_HPP_
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XBINARYSTREAMRESOLVER_HPP_
#include <com/sun/star/document/XBinaryStreamResolver.hpp>
#endif

#ifndef _COMPHELPER_NAMECONTAINER_HXX_
#include <comphelper/namecontainer.hxx>
#endif

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifdef CONV_STAR_FONTS
#ifndef _VCL_FONTCVT_HXX
#include <unotools/fontcvt.hxx>
#endif
#endif
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::binfilter::xmloff::token;

sal_Char __READONLY_DATA sXML_np__office[] = "_office";
sal_Char __READONLY_DATA sXML_np__style[] = "_style";
sal_Char __READONLY_DATA sXML_np__text[] = "_text";
sal_Char __READONLY_DATA sXML_np__table[] = "_table";
sal_Char __READONLY_DATA sXML_np__draw[] = "_draw";
sal_Char __READONLY_DATA sXML_np__dr3d[] = "_dr3d";
sal_Char __READONLY_DATA sXML_np__fo[] = "_fo";
sal_Char __READONLY_DATA sXML_np__xlink[] = "_xlink";
sal_Char __READONLY_DATA sXML_np__dc[] = "_dc";
sal_Char __READONLY_DATA sXML_np__meta[] = "_meta";
sal_Char __READONLY_DATA sXML_np__number[] = "_number";
sal_Char __READONLY_DATA sXML_np__svg[] = "_svg";
sal_Char __READONLY_DATA sXML_np__chart[] = "_chart";
sal_Char __READONLY_DATA sXML_np__math[] = "_math";
sal_Char __READONLY_DATA sXML_np__script[] = "_script";
sal_Char __READONLY_DATA sXML_np__config[] = "_config";

sal_Char __READONLY_DATA sXML_np__fo_old[] = "__fo";
sal_Char __READONLY_DATA sXML_np__xlink_old[] = "__xlink";
sal_Char __READONLY_DATA sXML_np__office_old[] = "__office";
sal_Char __READONLY_DATA sXML_np__style_old[] = "__style";
sal_Char __READONLY_DATA sXML_np__text_old[] = "__text";
sal_Char __READONLY_DATA sXML_np__table_old[] = "__table";
sal_Char __READONLY_DATA sXML_np__meta_old[] = "__meta";

#define LOGFILE_AUTHOR "mb93740"

#ifdef CONV_STAR_FONTS

class SvXMLImportEventListener : public ::cppu::WeakImplHelper1<
                            ::com::sun::star::lang::XEventListener >
{
private:
    SvXMLImport*	pImport;

public:
                            SvXMLImportEventListener(SvXMLImport* pImport);
    virtual					~SvXMLImportEventListener();

                            // XEventListener
    virtual void SAL_CALL disposing(const lang::EventObject& rEventObject) throw(::com::sun::star::uno::RuntimeException);
};

SvXMLImportEventListener::SvXMLImportEventListener(SvXMLImport* pTempImport)
    : pImport(pTempImport)
{
}

SvXMLImportEventListener::~SvXMLImportEventListener()
{
}

// XEventListener
void SAL_CALL SvXMLImportEventListener::disposing( const lang::EventObject& rEventObject )
    throw(uno::RuntimeException)
{
    if (pImport)
        pImport->DisposingModel();
}

//==============================================================================

class SvXMLImport_Impl
{
public:
    FontToSubsFontConverter hBatsFontConv;
    FontToSubsFontConverter hMathFontConv;

    bool mbOwnGraphicResolver;
    bool mbOwnEmbeddedResolver;

    SvXMLImport_Impl() : hBatsFontConv( 0 ), hMathFontConv( 0 ), mbOwnGraphicResolver( false ), mbOwnEmbeddedResolver( false ) {}
    ~SvXMLImport_Impl()
    {
        if( hBatsFontConv )
            DestroyFontToSubsFontConverter( hBatsFontConv );
        if( hMathFontConv )
            DestroyFontToSubsFontConverter( hMathFontConv );
    }
};
#endif

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
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__office ) ),
                        GetXMLToken(XML_N_OFFICE),
                        XML_NAMESPACE_OFFICE );
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__style) ),
                        GetXMLToken(XML_N_STYLE),
                        XML_NAMESPACE_STYLE );
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__text) ),
                        GetXMLToken(XML_N_TEXT),
                        XML_NAMESPACE_TEXT );
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__table ) ),
                        GetXMLToken(XML_N_TABLE),
                        XML_NAMESPACE_TABLE );
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__draw ) ),
                        GetXMLToken(XML_N_DRAW),
                        XML_NAMESPACE_DRAW );
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM (sXML_np__dr3d ) ),
                        GetXMLToken(XML_N_DR3D),
                        XML_NAMESPACE_DR3D );
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__fo) ),
                        GetXMLToken(XML_N_FO),
                        XML_NAMESPACE_FO );
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__xlink) ),
                        GetXMLToken(XML_N_XLINK),
                        XML_NAMESPACE_XLINK );
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__dc) ),
                        GetXMLToken(XML_N_DC),
                        XML_NAMESPACE_DC );
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__meta) ),
                        GetXMLToken(XML_N_META),
                        XML_NAMESPACE_META );
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__number) ),
                        GetXMLToken(XML_N_NUMBER),
                        XML_NAMESPACE_NUMBER );
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__svg) ),
                        GetXMLToken(XML_N_SVG),
                        XML_NAMESPACE_SVG );
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__chart) ),
                        GetXMLToken(XML_N_CHART),
                        XML_NAMESPACE_CHART );
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__math) ),
                        GetXMLToken(XML_N_MATH),
                        XML_NAMESPACE_MATH );
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_namespace_form) ),
                           GetXMLToken(XML_N_FORM),
                        XML_NAMESPACE_FORM );
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__script) ),
                        GetXMLToken(XML_N_SCRIPT),
                        XML_NAMESPACE_SCRIPT );
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__config) ),
                        GetXMLToken(XML_N_CONFIG),
                        XML_NAMESPACE_CONFIG );

    // namespaces used in the technical preview (SO 5.2)
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__fo_old) ),
                        GetXMLToken(XML_N_FO_OLD),
                        XML_NAMESPACE_FO );
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__xlink_old) ),
                        GetXMLToken(XML_N_XLINK_OLD),
                        XML_NAMESPACE_XLINK );
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__office) ),
                        GetXMLToken(XML_N_OFFICE_OLD),
                        XML_NAMESPACE_OFFICE );
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__style_old) ),
                        GetXMLToken(XML_N_STYLE_OLD),
                        XML_NAMESPACE_STYLE );
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__text_old) ),
                        GetXMLToken(XML_N_TEXT_OLD),
                        XML_NAMESPACE_TEXT );
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__table_old) ),
                        GetXMLToken(XML_N_TABLE_OLD),
                        XML_NAMESPACE_TABLE );
    pNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__meta_old) ),
                        GetXMLToken(XML_N_META_OLD),
                        XML_NAMESPACE_META );
    sPackageProtocol = OUString( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.Package:" ) );

    if (xNumberFormatsSupplier.is())
        pNumImport = new SvXMLNumFmtHelper(xNumberFormatsSupplier, getServiceFactory());

    if (xModel.is() && !pEventListener)
    {
        pEventListener = new SvXMLImportEventListener(this);
        xModel->addEventListener(pEventListener);
    }
}

// #110680#
SvXMLImport::SvXMLImport( 
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    sal_uInt16 nImportFlags ) throw () 
:	pImpl( new SvXMLImport_Impl() ),
    // #110680#
    mxServiceFactory(xServiceFactory),
    pNamespaceMap( new SvXMLNamespaceMap ),

    // #110680#
    // pUnitConv( new SvXMLUnitConverter( MAP_100TH_MM, MAP_100TH_MM ) ),
    pUnitConv( new SvXMLUnitConverter( MAP_100TH_MM, MAP_100TH_MM, getServiceFactory() ) ),

    pContexts( new SvXMLImportContexts_Impl ),
    pNumImport( NULL ),
    pProgressBarHelper( NULL ),
    pEventImportHelper( NULL ),
    pEventListener( NULL ),
    pXMLErrors( NULL ),
    mnImportFlags( nImportFlags ),
    mbIsFormsSupported( sal_True )
{
    DBG_ASSERT( mxServiceFactory.is(), "got no service manager" );
    _InitCtor();
}

SvXMLImport::~SvXMLImport() throw ()
{
    delete pXMLErrors;
    delete pNamespaceMap;
    delete pUnitConv;
    delete pContexts;
    delete pEventImportHelper;
//	delete pImageMapImportHelper;

    //	#i9518# the import component might not be deleted until after the document has been closed,
    //	so the stuff that accesses the document has been moved to endDocument.

    //	pNumImport is allocated in the ctor, so it must also be deleted here in case the component
    //	is created and deleted without actually importing.
    delete pNumImport;
    delete pProgressBarHelper;

    xmloff::token::ResetTokens();

    if( pImpl )
        delete pImpl;

    if (pEventListener && xModel.is())
        xModel->removeEventListener(pEventListener);
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
    RTL_LOGFILE_TRACE_AUTHOR( "xmloff", LOGFILE_AUTHOR, "{ SvXMLImport::startDocument" );

    if( !xGraphicResolver.is() || !xEmbeddedResolver.is() )
    {
        Reference< lang::XMultiServiceFactory > xFactory( xModel,	UNO_QUERY );
        if( xFactory.is() )
        {
            try
            {
                if( !xGraphicResolver.is() )
                {
                    xGraphicResolver = Reference< XGraphicObjectResolver >::query(
                        xFactory->createInstance(
                            OUString(RTL_CONSTASCII_USTRINGPARAM(
                                // #99870# Import... instead of Export...
                                "com.sun.star.document.ImportGraphicObjectResolver"))));
                    pImpl->mbOwnGraphicResolver = xGraphicResolver.is();
                }

                if( !xEmbeddedResolver.is() )
                {
                    xEmbeddedResolver = Reference< XEmbeddedObjectResolver >::query(
                        xFactory->createInstance(
                            OUString(RTL_CONSTASCII_USTRINGPARAM(
                                // #99870# Import... instead of Export...
                                "com.sun.star.document.ImportEmbeddedObjectResolver"))));
                    pImpl->mbOwnEmbeddedResolver = xEmbeddedResolver.is();
                }
            }
            catch( ::com::sun::star::uno::Exception& )
            {
            }
        }
    }
}

void SAL_CALL SvXMLImport::endDocument( void )
    throw( xml::sax::SAXException, uno::RuntimeException)
{
    RTL_LOGFILE_TRACE_AUTHOR( "xmloff", LOGFILE_AUTHOR, "} SvXMLImport::startDocument" );

    //	#i9518# All the stuff that accesses the document has to be done here, not in the dtor,
    //	because the SvXMLImport dtor might not be called until after the document has been closed.

    if (pNumImport)
    {
        delete pNumImport;
        pNumImport = NULL;
    }
    if (xImportInfo.is())
    {
        uno::Reference< beans::XPropertySetInfo > xPropertySetInfo = xImportInfo->getPropertySetInfo();
        if (xPropertySetInfo.is())
        {
            if (pProgressBarHelper)
            {
                OUString sProgressMax(RTL_CONSTASCII_USTRINGPARAM(XML_PROGRESSMAX));
                OUString sProgressCurrent(RTL_CONSTASCII_USTRINGPARAM(XML_PROGRESSCURRENT));
                OUString sRepeat(RTL_CONSTASCII_USTRINGPARAM(XML_PROGRESSREPEAT));
                if (xPropertySetInfo->hasPropertyByName(sProgressMax) &&
                    xPropertySetInfo->hasPropertyByName(sProgressCurrent))
                {
                    sal_Int32 nProgressMax(pProgressBarHelper->GetReference());
                    sal_Int32 nProgressCurrent(pProgressBarHelper->GetValue());
                    uno::Any aAny;
                    aAny <<= nProgressMax;
                    xImportInfo->setPropertyValue(sProgressMax, aAny);
                    aAny <<= nProgressCurrent;
                    xImportInfo->setPropertyValue(sProgressCurrent, aAny);
                }
                if (xPropertySetInfo->hasPropertyByName(sRepeat))
                    xImportInfo->setPropertyValue(sRepeat, ::cppu::bool2any(pProgressBarHelper->GetRepeat()));
                // pProgressBarHelper is deleted in dtor
            }
            OUString sNumberStyles(RTL_CONSTASCII_USTRINGPARAM(XML_NUMBERSTYLES));
            if (xNumberStyles.is() && xPropertySetInfo->hasPropertyByName(sNumberStyles))
            {
                uno::Any aAny;
                aAny <<= xNumberStyles;
                xImportInfo->setPropertyValue(sNumberStyles, aAny);
            }
        }
    }

    if( xFontDecls.Is() )
        ((SvXMLStylesContext *)&xFontDecls)->Clear();
    if( xStyles.Is() )
        ((SvXMLStylesContext *)&xStyles)->Clear();
    if( xAutoStyles.Is() )
        ((SvXMLStylesContext *)&xAutoStyles)->Clear();
    if( xMasterStyles.Is() )
        ((SvXMLStylesContext *)&xMasterStyles)->Clear();

    // possible form-layer related knittings which can only be done when
    // the whole document exists
    if ( mxFormImport.is() )
        mxFormImport->documentDone();

    //	The shape import helper does the z-order sorting in the dtor,
    //	so it must be deleted here, too.
    mxShapeImport = NULL;

    if( pImpl->mbOwnGraphicResolver )
    {
        Reference< lang::XComponent > xComp( xGraphicResolver, UNO_QUERY );
        xComp->dispose();
    }

    if( pImpl->mbOwnEmbeddedResolver )
    {
        Reference< lang::XComponent > xComp( xEmbeddedResolver, UNO_QUERY );
        xComp->dispose();
    }

    if ( pXMLErrors != NULL )
    {
        pXMLErrors->ThrowErrorAsSAXException( XMLERROR_FLAG_SEVERE );
    }
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
        if( ( rAttrName.getLength() >= 5 ) && 
            ( rAttrName.compareToAscii( sXML_xmlns, 5 ) == 0 ) &&
            ( rAttrName.getLength() == 5 || ':' == rAttrName[5] ) )
        {
            if( !pRewindMap )
            {
                pRewindMap = pNamespaceMap;
                pNamespaceMap = new SvXMLNamespaceMap( *pNamespaceMap );
            }
            const OUString& rAttrValue = xAttrList->getValueByIndex( i );

            OUString aPrefix( ( rAttrName.getLength() == 5 ) 
                                 ? OUString() 
                                 : rAttrName.copy( 6 ) );
            pNamespaceMap->Add( aPrefix, rAttrValue );
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
#ifdef TIMELOG
        // If we do profiling, we want a trace message for the first element
        // in order to identify the stream.
        ByteString aString( (String)rName, RTL_TEXTENCODING_ASCII_US );
        RTL_LOGFILE_TRACE_AUTHOR1( "xmloff", LOGFILE_AUTHOR,
                                   "SvXMLImport::StartElement( \"%s\", ... )",
                                   aString.GetBuffer() );
#endif

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

#ifdef DBG_UTIL
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
    GetProgressBarHelper()->SetRepeat(sal_False);
    GetProgressBarHelper()->SetReference(0);
}

///////////////////////////////////////////////////////////////////////

// XImporter
void SAL_CALL SvXMLImport::setTargetDocument( const uno::Reference< lang::XComponent >& xDoc )
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    xModel = uno::Reference< frame::XModel >::query( xDoc );
    if( !xModel.is() )
        throw lang::IllegalArgumentException();
    if (xModel.is() && !pEventListener)
    {
        pEventListener = new SvXMLImportEventListener(this);
        xModel->addEventListener(pEventListener);
    }

    DBG_ASSERT( !pNumImport, "number format import already exists." );
    if( pNumImport )
    {
        delete pNumImport;
        pNumImport = 0;
    }
}

// XFilter
sal_Bool SAL_CALL SvXMLImport::filter( const uno::Sequence< beans::PropertyValue >& aDescriptor ) 
    throw (uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL SvXMLImport::cancel(  ) 
    throw (uno::RuntimeException)
{
}

// XInitialize
void SAL_CALL SvXMLImport::initialize( const uno::Sequence< uno::Any >& aArguments )
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    const sal_Int32 nAnyCount = aArguments.getLength();
    const uno::Any* pAny = aArguments.getConstArray();

    for( sal_Int32 nIndex = 0; nIndex < nAnyCount; nIndex++, pAny++ )
    {
        Reference<XInterface> xValue;
        *pAny >>= xValue;

        uno::Reference<task::XStatusIndicator> xTmpStatusIndicator(
            xValue, UNO_QUERY );
        if( xTmpStatusIndicator.is() )
            mxStatusIndicator = xTmpStatusIndicator;

        uno::Reference<document::XGraphicObjectResolver> xTmpGraphicResolver(
            xValue, UNO_QUERY );
        if( xTmpGraphicResolver.is() )
            xGraphicResolver = xTmpGraphicResolver;

        uno::Reference<document::XEmbeddedObjectResolver> xTmpObjectResolver(
            xValue, UNO_QUERY );
        if( xTmpObjectResolver.is() )
            xEmbeddedResolver = xTmpObjectResolver;

        uno::Reference<beans::XPropertySet> xTmpPropSet( xValue, UNO_QUERY );
        if( xTmpPropSet.is() )
        {
            xImportInfo = xTmpPropSet;
            uno::Reference< beans::XPropertySetInfo > xPropertySetInfo = xImportInfo->getPropertySetInfo();
            if (xPropertySetInfo.is())
            {
                OUString sNumberStyles(RTL_CONSTASCII_USTRINGPARAM(XML_NUMBERSTYLES));
                if (xPropertySetInfo->hasPropertyByName(sNumberStyles))
                {
                    uno::Any aAny = xImportInfo->getPropertyValue(sNumberStyles);
                    aAny >>= xNumberStyles;
                }
            }
        }
    }
}

// XServiceInfo
OUString SAL_CALL SvXMLImport::getImplementationName()
    throw(uno::RuntimeException)
{
    OUString aStr;
    return aStr;
}

sal_Bool SAL_CALL SvXMLImport::supportsService( const OUString& rServiceName )
    throw(::com::sun::star::uno::RuntimeException)
{
    return
        rServiceName.equalsAsciiL(
            "com.sun.star.document.ImportFilter",
            sizeof("com.sun.star.document.ImportFilter")-1 ) ||
        rServiceName.equalsAsciiL(
            "com.sun.star.xml.XMLImportFilter",
            sizeof("com.sun.star.xml.XMLImportFilter")-1);
}

uno::Sequence< OUString > SAL_CALL SvXMLImport::getSupportedServiceNames(  )
    throw(uno::RuntimeException)
{
    uno::Sequence<OUString> aSeq(2);
    OUString* pSeq = aSeq.getArray();
    aSeq[0] = OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.ImportFilter"));
    aSeq[1] = OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.XMLImportFilter"));
    return aSeq;
}

///////////////////////////////////////////////////////////////////////

XMLTextImportHelper* SvXMLImport::CreateTextImport()
{
    return new XMLTextImportHelper( xModel, *this );
}

XMLShapeImportHelper* SvXMLImport::CreateShapeImport()
{
    return new XMLShapeImportHelper( *this, xModel );
}

#ifndef SVX_LIGHT
SchXMLImportHelper* SvXMLImport::CreateChartImport()
{
    return new SchXMLImportHelper();
}
#endif

#ifndef SVX_LIGHT
::binfilter::xmloff::OFormLayerXMLImport* SvXMLImport::CreateFormImport()
{
    return new ::binfilter::xmloff::OFormLayerXMLImport(*this);
}
#endif // #ifndef SVX_LIGHT


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

::rtl::OUString SvXMLImport::ResolveGraphicObjectURL( const ::rtl::OUString& rURL,
                                                      sal_Bool bLoadOnDemand )
{
    ::rtl::OUString sRet;

    if( 0 == rURL.compareTo( ::rtl::OUString( '#' ), 1 ) )
    {
        if( !bLoadOnDemand && xGraphicResolver.is() )
        {
            ::rtl::OUString		aTmp( sPackageProtocol );
            aTmp += rURL.copy( 1 );
            sRet = xGraphicResolver->resolveGraphicObjectURL( aTmp );
        }

        if( !sRet.getLength() )
        {
            sRet = sPackageProtocol;
            sRet += rURL.copy( 1 );
        }
    }

    if( !sRet.getLength() )
        sRet = ::binfilter::StaticBaseUrl::RelToAbs( rURL );

    return sRet;
}

Reference< XOutputStream > SvXMLImport::GetStreamForGraphicObjectURLFromBase64()
{
    Reference< XOutputStream > xOStm;
    Reference< document::XBinaryStreamResolver > xStmResolver( xGraphicResolver, UNO_QUERY );

    if( xStmResolver.is() )
        xOStm = xStmResolver->createOutputStream();

    return xOStm;
}

::rtl::OUString SvXMLImport::ResolveGraphicObjectURLFromBase64(
                                 const Reference < XOutputStream >& rOut )
{
    OUString sURL;
    Reference< document::XBinaryStreamResolver > xStmResolver( xGraphicResolver, UNO_QUERY );
    if( xStmResolver.is() )
        sURL = xStmResolver->resolveOutputStream( rOut );

    return sURL;
}

::rtl::OUString SvXMLImport::ResolveEmbeddedObjectURL(
                                    const ::rtl::OUString& rURL,
                                    const ::rtl::OUString& rClassId )
{
    ::rtl::OUString sRet;

    if( 0 == rURL.compareTo( ::rtl::OUString( '#' ), 1 ) &&
         xEmbeddedResolver.is() )
    {
        OUString sURL( rURL );
        if( rClassId.getLength() )
        {
            sURL += OUString( '!' );
            sURL += rClassId;
        }
        sRet = xEmbeddedResolver->resolveEmbeddedObjectURL( sURL );
    }

    return sRet;
}

Reference < XOutputStream > SvXMLImport::ResolveEmbeddedObjectURLFromBase64(
                                    const ::rtl::OUString& rURL )
{
    Reference < XOutputStream > xOLEStream;

    if( 0 == rURL.compareTo( ::rtl::OUString( '#' ), 1 ) &&
         xEmbeddedResolver.is() )
    {
        Reference< XNameAccess > xNA( xEmbeddedResolver, UNO_QUERY );
        if( xNA.is() )
        {
            Any aAny = xNA->getByName( rURL );
            aAny >>= xOLEStream;
        }
    }

    return xOLEStream;
}

void SvXMLImport::SetViewSettings(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aViewProps)
{
}

void SvXMLImport::SetConfigurationSettings(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aConfigProps)
{
}

ProgressBarHelper*	SvXMLImport::GetProgressBarHelper()
{
    if (!pProgressBarHelper)
    {
        pProgressBarHelper = new ProgressBarHelper(mxStatusIndicator, sal_False);

        if (pProgressBarHelper && xImportInfo.is())
        {
            uno::Reference< beans::XPropertySetInfo > xPropertySetInfo = xImportInfo->getPropertySetInfo();
            if (xPropertySetInfo.is())
            {
                OUString sProgressRange(RTL_CONSTASCII_USTRINGPARAM(XML_PROGRESSRANGE));
                OUString sProgressMax(RTL_CONSTASCII_USTRINGPARAM(XML_PROGRESSMAX));
                OUString sProgressCurrent(RTL_CONSTASCII_USTRINGPARAM(XML_PROGRESSCURRENT));
                OUString sRepeat(RTL_CONSTASCII_USTRINGPARAM(XML_PROGRESSREPEAT));
                if (xPropertySetInfo->hasPropertyByName(sProgressMax) &&
                    xPropertySetInfo->hasPropertyByName(sProgressCurrent) &&
                    xPropertySetInfo->hasPropertyByName(sProgressRange))
                {
                    uno::Any aAny;
                    sal_Int32 nProgressMax(0);
                    sal_Int32 nProgressCurrent(0);
                    sal_Int32 nProgressRange(0);
                    aAny = xImportInfo->getPropertyValue(sProgressRange);
                    if (aAny >>= nProgressRange)
                        pProgressBarHelper->SetRange(nProgressRange);
                    aAny = xImportInfo->getPropertyValue(sProgressMax);
                    if (aAny >>= nProgressMax)
                        pProgressBarHelper->SetReference(nProgressMax);
                    aAny = xImportInfo->getPropertyValue(sProgressCurrent);
                    if (aAny >>= nProgressCurrent)
                        pProgressBarHelper->SetValue(nProgressCurrent);
                }
                if (xPropertySetInfo->hasPropertyByName(sRepeat))
                {
                    uno::Any aAny = xImportInfo->getPropertyValue(sRepeat);
                    if (aAny.getValueType() == getBooleanCppuType())
                        pProgressBarHelper->SetRepeat(::cppu::any2bool(aAny));
                    else
                        DBG_ERRORFILE("why is it no boolean?");
                }
            }
        }
    }
    return pProgressBarHelper;
}

void SvXMLImport::AddNumberStyle(sal_Int32 nKey, const OUString& rName)
{
    if (!xNumberStyles.is())
        xNumberStyles = uno::Reference< container::XNameContainer >( comphelper::NameContainer_createInstance( ::getCppuType((const sal_Int32*)0)) );
    if (xNumberStyles.is())
    {
        uno::Any aAny;
        aAny <<= nKey;
        try
        {
            xNumberStyles->insertByName(rName, aAny);
        }
        catch ( uno::Exception& )
        {
            DBG_ERROR("Numberformat could not be inserted");
        }
    }
    else
        DBG_ERROR("not possible to create NameContainer");
}

XMLEventImportHelper& SvXMLImport::GetEventImport()
{
#ifndef SVX_LIGHT
    if (!pEventImportHelper)
    {
        // construct event helper and register StarBasic handler and standard
        // event tables
        pEventImportHelper = new XMLEventImportHelper();
        OUString sStarBasic(GetXMLToken(XML_STARBASIC));
        pEventImportHelper->RegisterFactory(sStarBasic,
                                            new XMLStarBasicContextFactory());
        OUString sScript(GetXMLToken(XML_SCRIPT));
        pEventImportHelper->RegisterFactory(sScript,
                                            new XMLScriptContextFactory());
        pEventImportHelper->AddTranslationTable(aStandardEventTable);

        // register StarBasic event handler with capitalized spelling
        OUString sStarBasicCap(RTL_CONSTASCII_USTRINGPARAM("StarBasic"));
        pEventImportHelper->RegisterFactory(sStarBasicCap,
                                            new XMLStarBasicContextFactory());
    }
#endif

    return *pEventImportHelper;
}

void SvXMLImport::SetFontDecls( XMLFontStylesContext *pFontDecls )
{
    xFontDecls = pFontDecls;
    GetTextImport()->SetFontDecls( pFontDecls );
}

void SvXMLImport::SetStyles( SvXMLStylesContext *pStyles )
{
    xStyles = pStyles;
}

void SvXMLImport::SetAutoStyles( SvXMLStylesContext *pAutoStyles )
{
    if (pAutoStyles && xNumberStyles.is() && (mnImportFlags & IMPORT_CONTENT) )
    {
        uno::Reference<xml::sax::XAttributeList> xAttrList;
        uno::Sequence< ::rtl::OUString > aNames = xNumberStyles->getElementNames();
        sal_uInt32 nCount(aNames.getLength());
        if (nCount)
        {
            const OUString* pNames = aNames.getConstArray();
            if ( pNames )
            {
                SvXMLStyleContext* pContext;
                uno::Any aAny;
                sal_Int32 nKey(0);
                for (sal_uInt32 i = 0; i < nCount; i++, pNames++)
                {
                    aAny = xNumberStyles->getByName(*pNames);
                    if (aAny >>= nKey)
                    {
                        pContext = new SvXMLNumFormatContext( *this, XML_NAMESPACE_NUMBER,
                                    *pNames, xAttrList, nKey, *pAutoStyles );
                        pAutoStyles->AddStyle(*pContext);
                    }
                }
            }
        }
    }
    xAutoStyles = pAutoStyles;
    GetTextImport()->SetAutoStyles( pAutoStyles );
    GetShapeImport()->SetAutoStylesContext( pAutoStyles );
#ifndef SVX_LIGHT
    GetChartImport()->SetAutoStylesContext( pAutoStyles );
    GetFormImport()->setAutoStyleContext( pAutoStyles );
#endif
}

void SvXMLImport::SetMasterStyles( SvXMLStylesContext *pMasterStyles )
{
    xMasterStyles = pMasterStyles;
}

XMLFontStylesContext *SvXMLImport::GetFontDecls()
{
    return (XMLFontStylesContext *)&xFontDecls;
}

SvXMLStylesContext *SvXMLImport::GetStyles()
{
    return (SvXMLStylesContext *)&xStyles;
}

SvXMLStylesContext *SvXMLImport::GetAutoStyles()
{
    return (SvXMLStylesContext *)&xAutoStyles;
}

const XMLFontStylesContext *SvXMLImport::GetFontDecls() const
{
    return (const XMLFontStylesContext *)&xFontDecls;
}

const SvXMLStylesContext *SvXMLImport::GetStyles() const
{
    return (const SvXMLStylesContext *)&xStyles;
}

const SvXMLStylesContext *SvXMLImport::GetAutoStyles() const
{
    return (const SvXMLStylesContext *)&xAutoStyles;
}

OUString SvXMLImport::GetAbsoluteReference(const OUString& rValue)
{
    return ::binfilter::StaticBaseUrl::RelToAbs( rValue );
}

void SvXMLImport::_CreateNumberFormatsSupplier()
{
    DBG_ASSERT( !xNumberFormatsSupplier.is(),
                "number formats supplier already exists!" );
    if(xModel.is())
        xNumberFormatsSupplier =
            uno::Reference< util::XNumberFormatsSupplier> (xModel, uno::UNO_QUERY);
}


void SvXMLImport::_CreateDataStylesImport()
{
    DBG_ASSERT( pNumImport == NULL, "data styles import already exists!" );
    uno::Reference<util::XNumberFormatsSupplier> xNum =
        GetNumberFormatsSupplier();
    if ( xNum.is() )
        pNumImport = new SvXMLNumFmtHelper(xNum, getServiceFactory());
}


#ifdef CONV_STAR_FONTS
sal_Unicode SvXMLImport::ConvStarBatsCharToStarSymbol( sal_Unicode c )
{
    sal_Unicode cNew = c;
    if( !pImpl->hBatsFontConv )
    {
        OUString sStarBats( RTL_CONSTASCII_USTRINGPARAM( "StarBats" ) );
        pImpl->hBatsFontConv = CreateFontToSubsFontConverter( sStarBats,
                 FONTTOSUBSFONT_IMPORT|FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS );
        OSL_ENSURE( pImpl->hBatsFontConv, "Got no symbol font converter" );
    }
    if( pImpl->hBatsFontConv )
    {
        cNew = ConvertFontToSubsFontChar( pImpl->hBatsFontConv, c );
    }

    return cNew;
}

sal_Unicode SvXMLImport::ConvStarMathCharToStarSymbol( sal_Unicode c )
{
    sal_Unicode cNew = c;
    if( !pImpl->hMathFontConv )
    {
        OUString sStarMath( RTL_CONSTASCII_USTRINGPARAM( "StarMath" ) );
        pImpl->hMathFontConv = CreateFontToSubsFontConverter( sStarMath,
                 FONTTOSUBSFONT_IMPORT|FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS );
        OSL_ENSURE( pImpl->hMathFontConv, "Got no symbol font converter" );
    }
    if( pImpl->hMathFontConv )
    {
        cNew = ConvertFontToSubsFontChar( pImpl->hMathFontConv, c );
    }

    return cNew;
}
#endif



void SvXMLImport::SetError(
    sal_Int32 nId,
    const Sequence<OUString>& rMsgParams,
    const OUString& rExceptionMessage,
    const Reference<xml::sax::XLocator>& rLocator )
{
    // maintain error flags
    if ( ( nId & XMLERROR_FLAG_ERROR ) != 0 )
        mnErrorFlags |= ERROR_ERROR_OCCURED;
    if ( ( nId & XMLERROR_FLAG_WARNING ) != 0 )
        mnErrorFlags |= ERROR_WARNING_OCCURED;
    if ( ( nId & XMLERROR_FLAG_SEVERE ) != 0 )
        mnErrorFlags |= ERROR_DO_NOTHING;

    // create error list on demand
    if ( pXMLErrors == NULL )
        pXMLErrors = new XMLErrors();

    // save error information
    // use document locator (if none supplied)
    pXMLErrors->AddRecord( nId, rMsgParams, rExceptionMessage,
                           rLocator.is() ? rLocator : xLocator );
}

void SvXMLImport::SetError(
    sal_Int32 nId,
    const Sequence<OUString>& rMsgParams)
{
    OUString sEmpty;
    SetError( nId, rMsgParams, sEmpty, NULL );
}

void SvXMLImport::DisposingModel()
{
    if( xFontDecls.Is() )
        ((SvXMLStylesContext *)&xFontDecls)->Clear();
    if( xStyles.Is() )
        ((SvXMLStylesContext *)&xStyles)->Clear();
    if( xAutoStyles.Is() )
        ((SvXMLStylesContext *)&xAutoStyles)->Clear();
    if( xMasterStyles.Is() )
        ((SvXMLStylesContext *)&xMasterStyles)->Clear();

    xModel = 0;
    pEventListener = NULL;
}

::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > SvXMLImport::getServiceFactory()
{
    // #110680#
    return mxServiceFactory;
}

// eof
}//end of namespace binfilter
