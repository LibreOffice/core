/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <string.h>

#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <osl/mutex.hxx>
#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/XMLFontStylesContext.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnumfi.hxx>
#include "XMLEventImportHelper.hxx"
#include "XMLStarBasicContextFactory.hxx"
#include "XMLScriptContextFactory.hxx"
#include "StyleMap.hxx"
#include <xmloff/ProgressBarHelper.hxx>
#include "xmloff/xmlerror.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/document/XBinaryStreamResolver.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/xml/sax/XLocator.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <comphelper/namecontainer.hxx>
#include <comphelper/servicehelper.hxx>
#include <rtl/logfile.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/storagehelper.hxx>
#include <unotools/fontcvt.hxx>

#include <com/sun/star/rdf/XMetadatable.hpp>
#include <com/sun/star/rdf/XRepositorySupplier.hpp>
#include "RDFaImportHelper.hxx"

#define LOGFILE_AUTHOR "unknown"

using ::com::sun::star::beans::XPropertySetInfo;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::xmloff::token;

sal_Char const sXML_np__office[] = "_office";
sal_Char const sXML_np__office_ext[] = "_office_ooo";
sal_Char const sXML_np__ooo[] = "_ooo";
sal_Char const sXML_np__ooow[] = "_ooow";
sal_Char const sXML_np__oooc[] = "_oooc";
sal_Char const sXML_np__of[] = "_of";
sal_Char const sXML_np__style[] = "_style";
sal_Char const sXML_np__text[] = "_text";
sal_Char const sXML_np__table[] = "_table";
sal_Char const sXML_np__table_ext[] = "_table_ooo";
sal_Char const sXML_np__draw[] = "_draw";
sal_Char const sXML_np__draw_ext[] = "_draw_ooo";
sal_Char const sXML_np__dr3d[] = "_dr3d";
sal_Char const sXML_np__fo[] = "_fo";
sal_Char const sXML_np__xlink[] = "_xlink";
sal_Char const sXML_np__dc[] = "_dc";
sal_Char const sXML_np__dom[] = "_dom";
sal_Char const sXML_np__meta[] = "_meta";
sal_Char const sXML_np__number[] = "_number";
sal_Char const sXML_np__svg[] = "_svg";
sal_Char const sXML_np__chart[] = "_chart";
sal_Char const sXML_np__math[] = "_math";
sal_Char const sXML_np__form[] = "_form";
sal_Char const sXML_np__script[] = "_script";
sal_Char const sXML_np__config[] = "_config";
sal_Char const sXML_np__db[] = "_db";
sal_Char const sXML_np__xforms[] = "_xforms";
sal_Char const sXML_np__formx[] = "_formx";
sal_Char const sXML_np__xsd[] = "_xsd";
sal_Char const sXML_np__xsi[] = "_xsi";
sal_Char const sXML_np__field[] = "_field";
sal_Char const sXML_np__xhtml[] = "_xhtml";
sal_Char const sXML_np__css3text[] = "_css3text";

sal_Char const sXML_np__fo_old[] = "__fo";
sal_Char const sXML_np__xlink_old[] = "__xlink";
sal_Char const sXML_np__office_old[] = "__office";
sal_Char const sXML_np__style_old[] = "__style";
sal_Char const sXML_np__text_old[] = "__text";
sal_Char const sXML_np__table_old[] = "__table";
sal_Char const sXML_np__meta_old[] = "__meta";



class SvXMLImportEventListener : public cppu::WeakImplHelper1<
                            com::sun::star::lang::XEventListener >
{
private:
    SvXMLImport*    pImport;

public:
                            SvXMLImportEventListener(SvXMLImport* pImport);
    virtual                 ~SvXMLImportEventListener();

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
void SAL_CALL SvXMLImportEventListener::disposing( const lang::EventObject& )
    throw(uno::RuntimeException)
{
    if (pImport)
    {
        pImport->DisposingModel();
        pImport = NULL;
    }
}

//==============================================================================

class SAL_DLLPRIVATE SvXMLImport_Impl
{
public:
    FontToSubsFontConverter hBatsFontConv;
    FontToSubsFontConverter hMathFontConv;

    bool mbOwnGraphicResolver;
    bool mbOwnEmbeddedResolver;
    INetURLObject aBaseURL;
    INetURLObject aDocBase;

    /// name of stream in package, e.g., "content.xml"
    ::rtl::OUString mStreamName;

    ::rtl::OUString aODFVersion;

    // Boolean, indicating that position attributes
    // of shapes are given in horizontal left-to-right layout. This is the case
    // for the OpenOffice.org file format. (#i28749#)
    sal_Bool mbShapePositionInHoriL2R;
    sal_Bool mbTextDocInOOoFileFormat;

    const uno::Reference< uno::XComponentContext > mxComponentContext;

    uno::Reference< embed::XStorage > mxSourceStorage;

    std::auto_ptr< xmloff::RDFaImportHelper > mpRDFaHelper;

    SvXMLImport_Impl() :
        hBatsFontConv( 0 ), hMathFontConv( 0 ),
        mbOwnGraphicResolver( false ),
        mbOwnEmbeddedResolver( false ),
        mStreamName(),
        // Convert drawing object positions from OOo file format to OASIS (#i28749#)
        mbShapePositionInHoriL2R( sal_False ),
        mbTextDocInOOoFileFormat( sal_False ),
        mxComponentContext( ::comphelper::getProcessComponentContext() ),
        mpRDFaHelper() // lazy
    {
        OSL_ENSURE(mxComponentContext.is(), "SvXMLImport: no ComponentContext");
        if (!mxComponentContext.is()) throw uno::RuntimeException();
    }

    ~SvXMLImport_Impl()
    {
        if( hBatsFontConv )
            DestroyFontToSubsFontConverter( hBatsFontConv );
        if( hMathFontConv )
            DestroyFontToSubsFontConverter( hMathFontConv );
    }

    ::comphelper::UnoInterfaceToUniqueIdentifierMapper  maInterfaceToIdentifierMapper;
};

class SvXMLImportContexts_Impl : public std::vector<SvXMLImportContext *> {};

SvXMLImportContext *SvXMLImport::CreateContext( sal_uInt16 nPrefix,
                                         const OUString& rLocalName,
                                         const uno::Reference< xml::sax::XAttributeList >& )
{
    return new SvXMLImportContext( *this, nPrefix, rLocalName );
}

void SvXMLImport::_InitCtor()
{
    if( mnImportFlags != 0 )
    {
        // implicit "xml" namespace prefix
        mpNamespaceMap->Add( GetXMLToken(XML_XML), GetXMLToken(XML_N_XML),
                            XML_NAMESPACE_XML );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__office ) ),
                            GetXMLToken(XML_N_OFFICE),
                            XML_NAMESPACE_OFFICE );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__office_ext ) ),
                            GetXMLToken(XML_N_OFFICE_EXT),
                            XML_NAMESPACE_OFFICE_EXT );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__ooo ) ), GetXMLToken(XML_N_OOO), XML_NAMESPACE_OOO );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__style) ),
                            GetXMLToken(XML_N_STYLE),
                            XML_NAMESPACE_STYLE );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__text) ),
                            GetXMLToken(XML_N_TEXT),
                            XML_NAMESPACE_TEXT );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__table ) ),
                            GetXMLToken(XML_N_TABLE),
                            XML_NAMESPACE_TABLE );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__table_ext ) ),
                            GetXMLToken(XML_N_TABLE_EXT),
                            XML_NAMESPACE_TABLE_EXT );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__draw ) ),
                            GetXMLToken(XML_N_DRAW),
                            XML_NAMESPACE_DRAW );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__draw_ext ) ),
                            GetXMLToken(XML_N_DRAW_EXT),
                            XML_NAMESPACE_DRAW_EXT );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM (sXML_np__dr3d ) ),
                            GetXMLToken(XML_N_DR3D),
                            XML_NAMESPACE_DR3D );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__fo) ),
                            GetXMLToken(XML_N_FO_COMPAT),
                            XML_NAMESPACE_FO );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__xlink) ),
                            GetXMLToken(XML_N_XLINK),
                            XML_NAMESPACE_XLINK );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__dc) ),
                            GetXMLToken(XML_N_DC),
                            XML_NAMESPACE_DC );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__dom ) ),
                            GetXMLToken(XML_N_DOM),
                            XML_NAMESPACE_DOM );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__meta) ),
                            GetXMLToken(XML_N_META),
                            XML_NAMESPACE_META );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__number) ),
                            GetXMLToken(XML_N_NUMBER),
                            XML_NAMESPACE_NUMBER );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__svg) ),
                            GetXMLToken(XML_N_SVG_COMPAT),
                            XML_NAMESPACE_SVG );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__chart) ),
                            GetXMLToken(XML_N_CHART),
                            XML_NAMESPACE_CHART );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__math) ),
                            GetXMLToken(XML_N_MATH),
                            XML_NAMESPACE_MATH );
        mpNamespaceMap->Add(OUString(RTL_CONSTASCII_USTRINGPARAM( sXML_np__form )),
                            GetXMLToken(XML_N_FORM),
                            XML_NAMESPACE_FORM );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__script) ),
                            GetXMLToken(XML_N_SCRIPT),
                            XML_NAMESPACE_SCRIPT );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__config) ),
                            GetXMLToken(XML_N_CONFIG),
                            XML_NAMESPACE_CONFIG );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__xforms) ),
                            GetXMLToken(XML_N_XFORMS_1_0),
                            XML_NAMESPACE_XFORMS );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__formx) ),
                            GetXMLToken( XML_N_FORMX ),
                            XML_NAMESPACE_FORMX );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__xsd) ),
                            GetXMLToken(XML_N_XSD),
                            XML_NAMESPACE_XSD );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__xsi) ),
                            GetXMLToken(XML_N_XSI),
                            XML_NAMESPACE_XFORMS );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__ooow ) ), GetXMLToken(XML_N_OOOW), XML_NAMESPACE_OOOW );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__oooc ) ), GetXMLToken(XML_N_OOOC), XML_NAMESPACE_OOOC );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__field ) ), GetXMLToken(XML_N_FIELD), XML_NAMESPACE_FIELD );
        mpNamespaceMap->Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__of ) ),
                            GetXMLToken(XML_N_OF), XML_NAMESPACE_OF );
        mpNamespaceMap->Add(
            OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__xhtml ) ),
            GetXMLToken(XML_N_XHTML), XML_NAMESPACE_XHTML );
        mpNamespaceMap->Add(
            OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_np__css3text ) ),
            GetXMLToken(XML_N_CSS3TEXT), XML_NAMESPACE_CSS3TEXT );

        mpNamespaceMap->Add( OUString("_calc_libo"), GetXMLToken(XML_N_CALC_EXT), XML_NAMESPACE_CALC_EXT);
    }

    msPackageProtocol = OUString( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.Package:" ) );

    if (mxNumberFormatsSupplier.is())
        mpNumImport = new SvXMLNumFmtHelper(mxNumberFormatsSupplier, getServiceFactory());

    if (mxModel.is() && !mxEventListener.is())
    {
        mxEventListener.set(new SvXMLImportEventListener(this));
        mxModel->addEventListener(mxEventListener);
    }

    ::comphelper::UnoInterfaceToUniqueIdentifierMapper  maInterfaceToIdentifierMapper;

}

// #110680#
SvXMLImport::SvXMLImport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    sal_uInt16 nImportFlags ) throw ()
:   mpImpl( new SvXMLImport_Impl() ),
    mpNamespaceMap( new SvXMLNamespaceMap ),

    mpUnitConv( new SvXMLUnitConverter( xServiceFactory,
                util::MeasureUnit::MM_100TH, util::MeasureUnit::MM_100TH) ),

    mpContexts( new SvXMLImportContexts_Impl ),
    mpNumImport( NULL ),
    mpProgressBarHelper( NULL ),
    mpEventImportHelper( NULL ),
    mpXMLErrors( NULL ),
    mpStyleMap(0),
    mnImportFlags( nImportFlags ),
    mnErrorFlags(0),
    // #110680#
    mxServiceFactory(xServiceFactory),
    mbIsFormsSupported( sal_True ),
    mbIsTableShapeSupported( false ),
    mbIsGraphicLoadOnDemandSupported( true )
{
    DBG_ASSERT( mxServiceFactory.is(), "got no service manager" );
    _InitCtor();
}

SvXMLImport::~SvXMLImport() throw ()
{
    delete mpXMLErrors;
    delete mpNamespaceMap;
    delete mpUnitConv;
    delete mpEventImportHelper;
    if( mpContexts )
    {
        while( !mpContexts->empty() )
        {
            SvXMLImportContext *pContext = mpContexts->back();
            mpContexts->pop_back();
            if( pContext )
                pContext->ReleaseRef();
        }
        delete mpContexts;
    }

    //  #i9518# the import component might not be deleted until after the document has been closed,
    //  so the stuff that accesses the document has been moved to endDocument.

    //  pNumImport is allocated in the ctor, so it must also be deleted here in case the component
    //  is created and deleted without actually importing.
    delete mpNumImport;
    delete mpProgressBarHelper;

    xmloff::token::ResetTokens();

    if( mpImpl )
        delete mpImpl;

    if (mxEventListener.is() && mxModel.is())
        mxModel->removeEventListener(mxEventListener);
}

namespace
{
    class theSvXMLImportUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSvXMLImportUnoTunnelId> {};
}

// XUnoTunnel & co
const uno::Sequence< sal_Int8 > & SvXMLImport::getUnoTunnelId() throw()
{
    return theSvXMLImportUnoTunnelId::get().getSeq();
}

// XUnoTunnel
sal_Int64 SAL_CALL SvXMLImport::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw( uno::RuntimeException )
{
    if( rId.getLength() == 16 && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                                         rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_uIntPtr>(this));
    }
    return 0;
}

void SAL_CALL SvXMLImport::startDocument( void )
    throw( xml::sax::SAXException, uno::RuntimeException )
{
    RTL_LOGFILE_TRACE_AUTHOR( "xmloff", LOGFILE_AUTHOR, "{ SvXMLImport::startDocument" );

    if( !mxGraphicResolver.is() || !mxEmbeddedResolver.is() )
    {
        Reference< lang::XMultiServiceFactory > xFactory( mxModel,  UNO_QUERY );
        if( xFactory.is() )
        {
            try
            {
                if( !mxGraphicResolver.is() )
                {
                    mxGraphicResolver = Reference< XGraphicObjectResolver >::query(
                        xFactory->createInstance(
                            OUString(RTL_CONSTASCII_USTRINGPARAM(
                                // #99870# Import... instead of Export...
                                "com.sun.star.document.ImportGraphicObjectResolver"))));
                    mpImpl->mbOwnGraphicResolver = mxGraphicResolver.is();
                }

                if( !mxEmbeddedResolver.is() )
                {
                    mxEmbeddedResolver = Reference< XEmbeddedObjectResolver >::query(
                        xFactory->createInstance(
                            OUString(RTL_CONSTASCII_USTRINGPARAM(
                                // #99870# Import... instead of Export...
                                "com.sun.star.document.ImportEmbeddedObjectResolver"))));
                    mpImpl->mbOwnEmbeddedResolver = mxEmbeddedResolver.is();
                }
            }
            catch( com::sun::star::uno::Exception& )
            {
            }
        }
    }
}

void SAL_CALL SvXMLImport::endDocument( void )
    throw( xml::sax::SAXException, uno::RuntimeException)
{
    RTL_LOGFILE_TRACE_AUTHOR( "xmloff", LOGFILE_AUTHOR, "} SvXMLImport::startDocument" );

    //  #i9518# All the stuff that accesses the document has to be done here, not in the dtor,
    //  because the SvXMLImport dtor might not be called until after the document has been closed.

    if (mpImpl->mpRDFaHelper.get())
    {
        const uno::Reference<rdf::XRepositorySupplier> xRS(mxModel,
            uno::UNO_QUERY);
        if (xRS.is())
        {
            mpImpl->mpRDFaHelper->InsertRDFa( xRS );
        }
    }

    if (mpNumImport)
    {
        delete mpNumImport;
        mpNumImport = NULL;
    }
    if (mxImportInfo.is())
    {
        uno::Reference< beans::XPropertySetInfo > xPropertySetInfo = mxImportInfo->getPropertySetInfo();
        if (xPropertySetInfo.is())
        {
            if (mpProgressBarHelper)
            {
                OUString sProgressMax(RTL_CONSTASCII_USTRINGPARAM(XML_PROGRESSMAX));
                OUString sProgressCurrent(RTL_CONSTASCII_USTRINGPARAM(XML_PROGRESSCURRENT));
                OUString sRepeat(RTL_CONSTASCII_USTRINGPARAM(XML_PROGRESSREPEAT));
                if (xPropertySetInfo->hasPropertyByName(sProgressMax) &&
                    xPropertySetInfo->hasPropertyByName(sProgressCurrent))
                {
                    sal_Int32 nProgressMax(mpProgressBarHelper->GetReference());
                    sal_Int32 nProgressCurrent(mpProgressBarHelper->GetValue());
                    uno::Any aAny;
                    aAny <<= nProgressMax;
                    mxImportInfo->setPropertyValue(sProgressMax, aAny);
                    aAny <<= nProgressCurrent;
                    mxImportInfo->setPropertyValue(sProgressCurrent, aAny);
                }
                if (xPropertySetInfo->hasPropertyByName(sRepeat))
                    mxImportInfo->setPropertyValue(sRepeat, cppu::bool2any(mpProgressBarHelper->GetRepeat()));
                // pProgressBarHelper is deleted in dtor
            }
            OUString sNumberStyles(RTL_CONSTASCII_USTRINGPARAM(XML_NUMBERSTYLES));
            if (mxNumberStyles.is() && xPropertySetInfo->hasPropertyByName(sNumberStyles))
            {
                uno::Any aAny;
                aAny <<= mxNumberStyles;
                mxImportInfo->setPropertyValue(sNumberStyles, aAny);
            }
        }
    }

    if( mxFontDecls.Is() )
        ((SvXMLStylesContext *)&mxFontDecls)->Clear();
    if( mxStyles.Is() )
        ((SvXMLStylesContext *)&mxStyles)->Clear();
    if( mxAutoStyles.Is() )
        ((SvXMLStylesContext *)&mxAutoStyles)->Clear();
    if( mxMasterStyles.Is() )
        ((SvXMLStylesContext *)&mxMasterStyles)->Clear();

    // possible form-layer related knittings which can only be done when
    // the whole document exists
    if ( mxFormImport.is() )
        mxFormImport->documentDone();

    //  The shape import helper does the z-order sorting in the dtor,
    //  so it must be deleted here, too.
    mxShapeImport = NULL;

    if( mpImpl->mbOwnGraphicResolver )
    {
        Reference< lang::XComponent > xComp( mxGraphicResolver, UNO_QUERY );
        xComp->dispose();
    }

    if( mpImpl->mbOwnEmbeddedResolver )
    {
        Reference< lang::XComponent > xComp( mxEmbeddedResolver, UNO_QUERY );
        xComp->dispose();
    }
    if( mpStyleMap )
    {
        mpStyleMap->release();
        mpStyleMap = 0;
    }

    if ( mpXMLErrors != NULL )
    {
        mpXMLErrors->ThrowErrorAsSAXException( XMLERROR_FLAG_SEVERE );
    }
}

void SAL_CALL SvXMLImport::startElement( const OUString& rName,
                                         const uno::Reference< xml::sax::XAttributeList >& xAttrList )
    throw(xml::sax::SAXException, uno::RuntimeException)
{
    SvXMLNamespaceMap *pRewindMap = 0;

    // Process namespace attributes. This must happen before creating the
    // context, because namespace decaration apply to the element name itself.
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        if ( rAttrName == "office:version" )
        {
            mpImpl->aODFVersion = xAttrList->getValueByIndex( i );

            // the ODF version in content.xml and manifest.xml must be the same starting from ODF1.2
            if ( mpImpl->mStreamName == "content.xml" && !IsODFVersionConsistent( mpImpl->aODFVersion ) )
            {
                throw xml::sax::SAXException(
                        ::rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM( "Inconsistent ODF versions in content.xml and manifest.xml!" ) ),
                        uno::Reference< uno::XInterface >(),
                        uno::makeAny(
                            packages::zip::ZipIOException(
                                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                    "Inconsistent ODF versions in content.xml and manifest.xml!" ) ),
                                Reference< XInterface >() ) ) );
            }
        }
        else if( ( rAttrName.getLength() >= 5 ) &&
            ( rAttrName.compareTo( GetXMLToken(XML_XMLNS), 5 ) == 0 ) &&
            ( rAttrName.getLength() == 5 || ':' == rAttrName[5] ) )
        {
            if( !pRewindMap )
            {
                pRewindMap = mpNamespaceMap;
                mpNamespaceMap = new SvXMLNamespaceMap( *mpNamespaceMap );
            }
            const OUString& rAttrValue = xAttrList->getValueByIndex( i );

            OUString aPrefix( ( rAttrName.getLength() == 5 )
                                 ? OUString()
                                 : rAttrName.copy( 6 ) );
            // Add namespace, but only if it is known.
            sal_uInt16 nKey = mpNamespaceMap->AddIfKnown( aPrefix, rAttrValue );
            // If namespace is unknown, try to match a name with similar
            // TC Id an version
            if( XML_NAMESPACE_UNKNOWN == nKey  )
            {
                OUString aTestName( rAttrValue );
                if( SvXMLNamespaceMap::NormalizeURI( aTestName ) )
                    nKey = mpNamespaceMap->AddIfKnown( aPrefix, aTestName );
            }
            // If that namespace is not known, too, add it as unknown
            if( XML_NAMESPACE_UNKNOWN == nKey  )
                mpNamespaceMap->Add( aPrefix, rAttrValue );

        }
    }

    // Get element's namespace and local name.
    OUString aLocalName;
    sal_uInt16 nPrefix =
        mpNamespaceMap->GetKeyByAttrName( rName, &aLocalName );

    // If there are contexts already, call a CreateChildContext at the topmost
    // context. Otherwise, create a default context.
    SvXMLImportContext *pContext;
    sal_uInt16 nCount = mpContexts->size();
    if( nCount > 0 )
    {
        pContext = (*mpContexts)[nCount - 1]->CreateChildContext( nPrefix,
                                                                 aLocalName,
                                                                 xAttrList );
        DBG_ASSERT( pContext && pContext->GetPrefix() == nPrefix,
                "SvXMLImport::startElement: created context has wrong prefix" );
    }
    else
    {
#ifdef TIMELOG
        // If we do profiling, we want a trace message for the first element
        // in order to identify the stream.
        rtl::OString aString(rtl::OUStringToOString(rName, RTL_TEXTENCODING_ASCII_US));
        RTL_LOGFILE_TRACE_AUTHOR1( "xmloff", LOGFILE_AUTHOR,
                                   "SvXMLImport::StartElement( \"%s\", ... )",
                                   aString.getStr() );
#endif

        pContext = CreateContext( nPrefix, aLocalName, xAttrList );
        if( (nPrefix & XML_NAMESPACE_UNKNOWN_FLAG) != 0 &&
            IS_TYPE( SvXMLImportContext, pContext ) )
        {
            OUString aMsg( RTL_CONSTASCII_USTRINGPARAM( "Root element unknown" ) );
            Reference<xml::sax::XLocator> xDummyLocator;
            Sequence < OUString > aParams(1);
            aParams.getArray()[0] = rName;

            SetError( XMLERROR_FLAG_SEVERE|XMLERROR_UNKNOWN_ROOT,
                      aParams, aMsg, xDummyLocator );
        }
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
    mpContexts->push_back( pContext );
}

void SAL_CALL SvXMLImport::endElement( const OUString&
#ifdef DBG_UTIL
rName
#endif
)
    throw(xml::sax::SAXException, uno::RuntimeException)
{
    sal_uInt16 nCount = mpContexts->size();
    DBG_ASSERT( nCount, "SvXMLImport::endElement: no context left" );
    if( nCount > 0 )
    {
        // Get topmost context and remove it from the stack.
        SvXMLImportContext *pContext = mpContexts->back();
        mpContexts->pop_back();

#ifdef DBG_UTIL
        // Non product only: check if endElement call matches startELement call.
        OUString aLocalName;
        sal_uInt16 nPrefix =
            mpNamespaceMap->GetKeyByAttrName( rName, &aLocalName );
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
            delete mpNamespaceMap;
            mpNamespaceMap = pRewindMap;
        }
    }
}

void SAL_CALL SvXMLImport::characters( const OUString& rChars )
    throw(xml::sax::SAXException, uno::RuntimeException)
{
    if( !mpContexts->empty() )
    {
        mpContexts->back()->Characters( rChars );
    }
}

void SAL_CALL SvXMLImport::ignorableWhitespace( const OUString& )
    throw(xml::sax::SAXException, uno::RuntimeException)
{
}

void SAL_CALL SvXMLImport::processingInstruction( const OUString&,
                                       const OUString& )
    throw(xml::sax::SAXException, uno::RuntimeException)
{
}

void SAL_CALL SvXMLImport::setDocumentLocator( const uno::Reference< xml::sax::XLocator >& rLocator )
    throw(xml::sax::SAXException, uno::RuntimeException)
{
    mxLocator = rLocator;
}

// XExtendedDocumentHandler
void SAL_CALL SvXMLImport::startCDATA( void ) throw(xml::sax::SAXException, uno::RuntimeException)
{
}

void SAL_CALL SvXMLImport::endCDATA( void ) throw(uno::RuntimeException)
{
}

void SAL_CALL SvXMLImport::comment( const OUString& )
    throw(xml::sax::SAXException, uno::RuntimeException)
{
}

void SAL_CALL SvXMLImport::allowLineBreak( void )
    throw(xml::sax::SAXException, uno::RuntimeException)
{
}

void SAL_CALL SvXMLImport::unknown( const OUString& )
    throw(xml::sax::SAXException, uno::RuntimeException)
{
}

void SvXMLImport::SetStatistics(const uno::Sequence< beans::NamedValue> &)
{
    GetProgressBarHelper()->SetRepeat(sal_False);
    GetProgressBarHelper()->SetReference(0);
}

///////////////////////////////////////////////////////////////////////

// XImporter
void SAL_CALL SvXMLImport::setTargetDocument( const uno::Reference< lang::XComponent >& xDoc )
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    mxModel = uno::Reference< frame::XModel >::query( xDoc );
    if( !mxModel.is() )
        throw lang::IllegalArgumentException();
    if (mxModel.is() && !mxEventListener.is())
    {
        mxEventListener.set(new SvXMLImportEventListener(this));
        mxModel->addEventListener(mxEventListener);
    }

    DBG_ASSERT( !mpNumImport, "number format import already exists." );
    if( mpNumImport )
    {
        delete mpNumImport;
        mpNumImport = 0;
    }
}

// XFilter
sal_Bool SAL_CALL SvXMLImport::filter( const uno::Sequence< beans::PropertyValue >& )
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
            mxGraphicResolver = xTmpGraphicResolver;

        uno::Reference<document::XEmbeddedObjectResolver> xTmpObjectResolver(
            xValue, UNO_QUERY );
        if( xTmpObjectResolver.is() )
            mxEmbeddedResolver = xTmpObjectResolver;

        uno::Reference<beans::XPropertySet> xTmpPropSet( xValue, UNO_QUERY );
        if( xTmpPropSet.is() )
        {
            mxImportInfo = xTmpPropSet;
            uno::Reference< beans::XPropertySetInfo > xPropertySetInfo = mxImportInfo->getPropertySetInfo();
            if (xPropertySetInfo.is())
            {
                OUString sPropName(RTL_CONSTASCII_USTRINGPARAM(XML_NUMBERSTYLES));
                if (xPropertySetInfo->hasPropertyByName(sPropName))
                {
                    uno::Any aAny = mxImportInfo->getPropertyValue(sPropName);
                    aAny >>= mxNumberStyles;
                }

                sPropName = OUString( RTL_CONSTASCII_USTRINGPARAM("PrivateData" ) );
                if (xPropertySetInfo->hasPropertyByName(sPropName))
                {
                    Reference < XInterface > xIfc;
                    uno::Any aAny = mxImportInfo->getPropertyValue(sPropName);
                    aAny >>= xIfc;

                    StyleMap *pSMap = StyleMap::getImplementation( xIfc );
                    if( pSMap )
                    {
                        mpStyleMap = pSMap;
                        mpStyleMap->acquire();
                    }
                }
                OUString sBaseURI;
                sPropName = OUString( RTL_CONSTASCII_USTRINGPARAM("BaseURI" ) );
                if (xPropertySetInfo->hasPropertyByName(sPropName))
                {
                    uno::Any aAny = mxImportInfo->getPropertyValue(sPropName);
                    aAny >>= sBaseURI;
                    mpImpl->aBaseURL.SetURL( sBaseURI );
                    mpImpl->aDocBase.SetURL( sBaseURI );
                }
                OUString sRelPath;
                sPropName = OUString( RTL_CONSTASCII_USTRINGPARAM("StreamRelPath" ) );
                if( xPropertySetInfo->hasPropertyByName(sPropName) )
                {
                    uno::Any aAny = mxImportInfo->getPropertyValue(sPropName);
                    aAny >>= sRelPath;
                }
                OUString sName;
                sPropName = OUString( RTL_CONSTASCII_USTRINGPARAM("StreamName" ) );
                if( xPropertySetInfo->hasPropertyByName(sPropName) )
                {
                    uno::Any aAny = mxImportInfo->getPropertyValue(sPropName);
                    aAny >>= sName;
                }
                if( !sBaseURI.isEmpty() && !sName.isEmpty() )
                {
                    if( !sRelPath.isEmpty() )
                        mpImpl->aBaseURL.insertName( sRelPath );
                    mpImpl->aBaseURL.insertName( sName );
                }
                mpImpl->mStreamName = sName; // Note: may be empty (XSLT)
                // Retrieve property <ShapePositionInHoriL2R> (#i28749#)
                sPropName = OUString( RTL_CONSTASCII_USTRINGPARAM("ShapePositionInHoriL2R" ) );
                if( xPropertySetInfo->hasPropertyByName(sPropName) )
                {
                    uno::Any aAny = mxImportInfo->getPropertyValue(sPropName);
                    aAny >>= (mpImpl->mbShapePositionInHoriL2R);
                }
                sPropName = OUString( RTL_CONSTASCII_USTRINGPARAM("TextDocInOOoFileFormat" ) );
                if( xPropertySetInfo->hasPropertyByName(sPropName) )
                {
                    uno::Any aAny = mxImportInfo->getPropertyValue(sPropName);
                    aAny >>= (mpImpl->mbTextDocInOOoFileFormat);
                }

                sPropName = OUString( RTL_CONSTASCII_USTRINGPARAM("SourceStorage" ) );
                if( xPropertySetInfo->hasPropertyByName(sPropName) )
                    mxImportInfo->getPropertyValue(sPropName) >>= mpImpl->mxSourceStorage;
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
    aSeq[0] = OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.ImportFilter"));
    aSeq[1] = OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.XMLImportFilter"));
    return aSeq;
}

///////////////////////////////////////////////////////////////////////

XMLTextImportHelper* SvXMLImport::CreateTextImport()
{
    return new XMLTextImportHelper( mxModel, *this );
}

XMLShapeImportHelper* SvXMLImport::CreateShapeImport()
{
    return new XMLShapeImportHelper( *this, mxModel );
}

SchXMLImportHelper* SvXMLImport::CreateChartImport()
{
    return new SchXMLImportHelper();
}

::xmloff::OFormLayerXMLImport* SvXMLImport::CreateFormImport()
{
    return new ::xmloff::OFormLayerXMLImport(*this);
}


///////////////////////////////////////////////////////////////////////////////
//
// Get or create fill/line/lineend-style-helper
//

const Reference< container::XNameContainer > & SvXMLImport::GetGradientHelper()
{
    if( !mxGradientHelper.is() )
    {
        if( mxModel.is() )
        {
            Reference< lang::XMultiServiceFactory > xServiceFact( mxModel, UNO_QUERY);
            if( xServiceFact.is() )
            {
                try
                {
                    mxGradientHelper =  Reference< container::XNameContainer >( xServiceFact->createInstance(
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.GradientTable" ) ) ), UNO_QUERY);
                }
                catch( lang::ServiceNotRegisteredException& )
                {}
            }
        }
    }

    return mxGradientHelper;
}

const Reference< container::XNameContainer > & SvXMLImport::GetHatchHelper()
{
    if( !mxHatchHelper.is() )
    {
        if( mxModel.is() )
        {
            Reference< lang::XMultiServiceFactory > xServiceFact( mxModel, UNO_QUERY);
            if( xServiceFact.is() )
            {
                try
                {
                    mxHatchHelper =  Reference< container::XNameContainer >( xServiceFact->createInstance(
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.HatchTable" ) ) ), UNO_QUERY);
                }
                catch( lang::ServiceNotRegisteredException& )
                {}
            }
        }
    }

    return mxHatchHelper;
}

const Reference< container::XNameContainer > & SvXMLImport::GetBitmapHelper()
{
    if( !mxBitmapHelper.is() )
    {
        if( mxModel.is() )
        {
            Reference< lang::XMultiServiceFactory > xServiceFact( mxModel, UNO_QUERY);
            if( xServiceFact.is() )
            {
                try
                {
                    mxBitmapHelper =  Reference< container::XNameContainer >( xServiceFact->createInstance(
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.BitmapTable" ) ) ), UNO_QUERY);
                }
                catch( lang::ServiceNotRegisteredException& )
                {}
            }
        }
    }

    return mxBitmapHelper;
}

const Reference< container::XNameContainer > & SvXMLImport::GetTransGradientHelper()
{
    if( !mxTransGradientHelper.is() )
    {
        if( mxModel.is() )
        {
            Reference< lang::XMultiServiceFactory > xServiceFact( mxModel, UNO_QUERY);
            if( xServiceFact.is() )
            {
                try
                {
                    mxTransGradientHelper =  Reference< container::XNameContainer >( xServiceFact->createInstance(
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.TransparencyGradientTable" ) ) ), UNO_QUERY);
                }
                catch( lang::ServiceNotRegisteredException& )
                {}
            }
        }
    }

    return mxTransGradientHelper;
}

const Reference< container::XNameContainer > & SvXMLImport::GetMarkerHelper()
{
    if( !mxMarkerHelper.is() )
    {
        if( mxModel.is() )
        {
            Reference< lang::XMultiServiceFactory > xServiceFact( mxModel, UNO_QUERY);
            if( xServiceFact.is() )
            {
                try
                {
                    mxMarkerHelper =  Reference< container::XNameContainer >( xServiceFact->createInstance(
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.MarkerTable" ) ) ), UNO_QUERY);
                }
                catch( lang::ServiceNotRegisteredException& )
                {}
            }
        }
    }

    return mxMarkerHelper;
}

const Reference< container::XNameContainer > & SvXMLImport::GetDashHelper()
{
    if( !mxDashHelper.is() )
    {
        if( mxModel.is() )
        {
            Reference< lang::XMultiServiceFactory > xServiceFact( mxModel, UNO_QUERY);
            if( xServiceFact.is() )
            {
                try
                {
                    mxDashHelper =  Reference< container::XNameContainer >( xServiceFact->createInstance(
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.DashTable" ) ) ), UNO_QUERY);
                }
                catch( lang::ServiceNotRegisteredException& )
                {}
            }
        }
    }

    return mxDashHelper;
}

sal_Bool SvXMLImport::IsPackageURL( const ::rtl::OUString& rURL ) const
{

    // if, and only if, only parts are imported, then we're in a package
    const sal_uInt32 nTest = IMPORT_META|IMPORT_STYLES|IMPORT_CONTENT|IMPORT_SETTINGS;
    if( (mnImportFlags & nTest) == nTest )
        return sal_False;

    // Some quick tests: Some may rely on the package structure!
    sal_Int32 nLen = rURL.getLength();
    if( (nLen > 0 && '/' == rURL[0]) )
        // RFC2396 net_path or abs_path
        return sal_False;
    else if( nLen > 1 && '.' == rURL[0] )
    {
        if( '.' == rURL[1] )
            // ../: We are never going up one level, so we know
            // it's not an external URI
            return sal_False;
        else if( '/' == rURL[1] )
            // we are remaining on a level, so it's an package URI
            return sal_True;
    }

    // Now check for a RFC2396 schema
    sal_Int32 nPos = 1;
    while( nPos < nLen )
    {
        switch( rURL[nPos] )
        {
        case '/':
            // a relative path segement
            return sal_True;
        case ':':
            // a schema
            return sal_False;
        default:
            break;
            // we don't care about any other characters
        }
        ++nPos;
    }

    return sal_True;
}

::rtl::OUString SvXMLImport::ResolveGraphicObjectURL( const ::rtl::OUString& rURL,
                                                      sal_Bool bLoadOnDemand )
{
    ::rtl::OUString sRet;

    if( IsPackageURL( rURL ) )
    {
        if( !bLoadOnDemand && mxGraphicResolver.is() )
        {
            ::rtl::OUString     aTmp( msPackageProtocol );
            aTmp += rURL;
            sRet = mxGraphicResolver->resolveGraphicObjectURL( aTmp );
        }

        if( sRet.isEmpty() )
        {
            sRet = msPackageProtocol;
            sRet += rURL;
        }
    }

    if( sRet.isEmpty() )
        sRet = GetAbsoluteReference( rURL );

    return sRet;
}

Reference< XOutputStream > SvXMLImport::GetStreamForGraphicObjectURLFromBase64()
{
    Reference< XOutputStream > xOStm;
    Reference< document::XBinaryStreamResolver > xStmResolver( mxGraphicResolver, UNO_QUERY );

    if( xStmResolver.is() )
        xOStm = xStmResolver->createOutputStream();

    return xOStm;
}

::rtl::OUString SvXMLImport::ResolveGraphicObjectURLFromBase64(
                                 const Reference < XOutputStream >& rOut )
{
    OUString sURL;
    Reference< document::XBinaryStreamResolver > xStmResolver( mxGraphicResolver, UNO_QUERY );
    if( xStmResolver.is() )
        sURL = xStmResolver->resolveOutputStream( rOut );

    return sURL;
}

::rtl::OUString SvXMLImport::ResolveEmbeddedObjectURL(
                                    const ::rtl::OUString& rURL,
                                    const ::rtl::OUString& rClassId )
{
    ::rtl::OUString sRet;

    if( IsPackageURL( rURL ) )
    {
        if ( mxEmbeddedResolver.is() )
        {
            OUString sURL( rURL );
            if( !rClassId.isEmpty() )
            {
                sURL += OUString( sal_Unicode('!') );
                sURL += rClassId;
            }
            sRet = mxEmbeddedResolver->resolveEmbeddedObjectURL( sURL );
        }
    }
    else
        sRet = GetAbsoluteReference( rURL );

    return sRet;
}

Reference< embed::XStorage > SvXMLImport::GetSourceStorage()
{
    return mpImpl->mxSourceStorage;
}

Reference < XOutputStream >
        SvXMLImport::GetStreamForEmbeddedObjectURLFromBase64()
{
    Reference < XOutputStream > xOLEStream;

    if( mxEmbeddedResolver.is() )
    {
        Reference< XNameAccess > xNA( mxEmbeddedResolver, UNO_QUERY );
        if( xNA.is() )
        {
            OUString aURL( RTL_CONSTASCII_USTRINGPARAM( "Obj12345678" ) );
            Any aAny = xNA->getByName( aURL );
            aAny >>= xOLEStream;
        }
    }

    return xOLEStream;
}

::rtl::OUString SvXMLImport::ResolveEmbeddedObjectURLFromBase64()
{
    ::rtl::OUString sRet;

    if( mxEmbeddedResolver.is() )
    {
        OUString aURL( RTL_CONSTASCII_USTRINGPARAM( "Obj12345678" ) );
        sRet = mxEmbeddedResolver->resolveEmbeddedObjectURL( aURL );
    }

    return sRet;
}

void SvXMLImport::AddStyleDisplayName( sal_uInt16 nFamily,
                                       const OUString& rName,
                                       const OUString& rDisplayName )
{
    if( !mpStyleMap )
    {
        mpStyleMap = new StyleMap;
        mpStyleMap->acquire();
        if( mxImportInfo.is() )
        {
            OUString sPrivateData(
                    RTL_CONSTASCII_USTRINGPARAM("PrivateData" ) );
            Reference< beans::XPropertySetInfo > xPropertySetInfo =
                mxImportInfo->getPropertySetInfo();
            if( xPropertySetInfo.is() &&
                xPropertySetInfo->hasPropertyByName(sPrivateData) )
            {
                Reference < XInterface > xIfc(
                        static_cast< XUnoTunnel *>( mpStyleMap ) );
                Any aAny;
                aAny <<= xIfc;
                mxImportInfo->setPropertyValue( sPrivateData, aAny );
            }
        }
    }

    StyleMap::key_type aKey( nFamily, rName );
    StyleMap::value_type aValue( aKey, rDisplayName );
    ::std::pair<StyleMap::iterator,bool> aRes( mpStyleMap->insert( aValue ) );
    SAL_WARN_IF( !aRes.second,
                 "xmloff",
                 "duplicate style name" );

}

OUString SvXMLImport::GetStyleDisplayName( sal_uInt16 nFamily,
                                           const OUString& rName ) const
{
    OUString sName( rName );
    if( mpStyleMap && !rName.isEmpty() )
    {
        StyleMap::key_type aKey( nFamily, rName );
        StyleMap::const_iterator aIter = mpStyleMap->find( aKey );
        if( aIter != mpStyleMap->end() )
            sName = (*aIter).second;
    }
    return sName;
}

void SvXMLImport::SetViewSettings(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>&)
{
}

void SvXMLImport::SetConfigurationSettings(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>&)
{
}

void SvXMLImport::SetDocumentSpecificSettings(const ::rtl::OUString& _rSettingsGroupName, const uno::Sequence<beans::PropertyValue>& _rSettings)
{
    (void)_rSettingsGroupName;
    (void)_rSettings;
}

ProgressBarHelper*  SvXMLImport::GetProgressBarHelper()
{
    if (!mpProgressBarHelper)
    {
        mpProgressBarHelper = new ProgressBarHelper(mxStatusIndicator, sal_False);

        if (mxImportInfo.is())
        {
            uno::Reference< beans::XPropertySetInfo > xPropertySetInfo = mxImportInfo->getPropertySetInfo();
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
                    aAny = mxImportInfo->getPropertyValue(sProgressRange);
                    if (aAny >>= nProgressRange)
                        mpProgressBarHelper->SetRange(nProgressRange);
                    aAny = mxImportInfo->getPropertyValue(sProgressMax);
                    if (aAny >>= nProgressMax)
                        mpProgressBarHelper->SetReference(nProgressMax);
                    aAny = mxImportInfo->getPropertyValue(sProgressCurrent);
                    if (aAny >>= nProgressCurrent)
                        mpProgressBarHelper->SetValue(nProgressCurrent);
                }
                if (xPropertySetInfo->hasPropertyByName(sRepeat))
                {
                    uno::Any aAny = mxImportInfo->getPropertyValue(sRepeat);
                    if (aAny.getValueType() == getBooleanCppuType())
                        mpProgressBarHelper->SetRepeat(::cppu::any2bool(aAny));
                    else {
                        SAL_WARN( "xmloff.core", "why is it no boolean?" );
                    }
                }
            }
        }
    }
    return mpProgressBarHelper;
}

void SvXMLImport::AddNumberStyle(sal_Int32 nKey, const OUString& rName)
{
    if (!mxNumberStyles.is())
        mxNumberStyles = uno::Reference< container::XNameContainer >( comphelper::NameContainer_createInstance( ::getCppuType((const sal_Int32*)0)) );
    if (mxNumberStyles.is())
    {
        uno::Any aAny;
        aAny <<= nKey;
        try
        {
            mxNumberStyles->insertByName(rName, aAny);
        }
        catch ( uno::Exception& )
        {
            OSL_FAIL("Numberformat could not be inserted");
        }
    }
    else {
        OSL_FAIL("not possible to create NameContainer");
    }
}

XMLEventImportHelper& SvXMLImport::GetEventImport()
{
    if (!mpEventImportHelper)
    {
        // construct event helper and register StarBasic handler and standard
        // event tables
        mpEventImportHelper = new XMLEventImportHelper();
        OUString sStarBasic(GetXMLToken(XML_STARBASIC));
        mpEventImportHelper->RegisterFactory(sStarBasic,
                                            new XMLStarBasicContextFactory());
        OUString sScript(GetXMLToken(XML_SCRIPT));
        mpEventImportHelper->RegisterFactory(sScript,
                                            new XMLScriptContextFactory());
        mpEventImportHelper->AddTranslationTable(aStandardEventTable);

        // register StarBasic event handler with capitalized spelling
        OUString sStarBasicCap(RTL_CONSTASCII_USTRINGPARAM("StarBasic"));
        mpEventImportHelper->RegisterFactory(sStarBasicCap,
                                            new XMLStarBasicContextFactory());
    }

    return *mpEventImportHelper;
}

void SvXMLImport::SetFontDecls( XMLFontStylesContext *pFontDecls )
{
    mxFontDecls = pFontDecls;
    GetTextImport()->SetFontDecls( pFontDecls );
}

void SvXMLImport::SetStyles( SvXMLStylesContext *pStyles )
{
    mxStyles = pStyles;
}

void SvXMLImport::SetAutoStyles( SvXMLStylesContext *pAutoStyles )
{
    if (pAutoStyles && mxNumberStyles.is() && (mnImportFlags & IMPORT_CONTENT) )
    {
        uno::Reference<xml::sax::XAttributeList> xAttrList;
        uno::Sequence< ::rtl::OUString > aNames = mxNumberStyles->getElementNames();
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
                    aAny = mxNumberStyles->getByName(*pNames);
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
    mxAutoStyles = pAutoStyles;
    GetTextImport()->SetAutoStyles( pAutoStyles );
    GetShapeImport()->SetAutoStylesContext( pAutoStyles );
    GetChartImport()->SetAutoStylesContext( pAutoStyles );
    GetFormImport()->setAutoStyleContext( pAutoStyles );
}

void SvXMLImport::SetMasterStyles( SvXMLStylesContext *pMasterStyles )
{
    mxMasterStyles = pMasterStyles;
}

XMLFontStylesContext *SvXMLImport::GetFontDecls()
{
    return (XMLFontStylesContext *)&mxFontDecls;
}

SvXMLStylesContext *SvXMLImport::GetStyles()
{
    return (SvXMLStylesContext *)&mxStyles;
}

SvXMLStylesContext *SvXMLImport::GetAutoStyles()
{
    return (SvXMLStylesContext *)&mxAutoStyles;
}

const XMLFontStylesContext *SvXMLImport::GetFontDecls() const
{
    return (const XMLFontStylesContext *)&mxFontDecls;
}

const SvXMLStylesContext *SvXMLImport::GetStyles() const
{
    return (const SvXMLStylesContext *)&mxStyles;
}

const SvXMLStylesContext *SvXMLImport::GetAutoStyles() const
{
    return (const SvXMLStylesContext *)&mxAutoStyles;
}

OUString SvXMLImport::GetAbsoluteReference(const OUString& rValue) const
{
    if( rValue.isEmpty() || rValue[0] == '#' )
        return rValue;

    INetURLObject aAbsURL;
    if( mpImpl->aBaseURL.GetNewAbsURL( rValue, &aAbsURL ) )
        return aAbsURL.GetMainURL( INetURLObject::DECODE_TO_IURI );
    else
        return rValue;
}

sal_Bool SvXMLImport::IsODFVersionConsistent( const ::rtl::OUString& aODFVersion )
{
    // the check returns sal_False only if the storage version could be retrieved
    sal_Bool bResult = sal_True;

    if ( !aODFVersion.isEmpty() && aODFVersion.compareTo( ODFVER_012_TEXT ) >= 0 )
    {
        // check the consistency only for the ODF1.2 and later ( according to content.xml )
        // manifest.xml might have no version, it should be checked here and the correct version should be set
        try
        {
            uno::Reference< document::XStorageBasedDocument > xDoc( mxModel, uno::UNO_QUERY_THROW );
            uno::Reference< embed::XStorage > xStor = xDoc->getDocumentStorage();
            uno::Reference< beans::XPropertySet > xStorProps( xStor, uno::UNO_QUERY_THROW );

            // the check should be done only for OASIS format
            ::rtl::OUString aMediaType;
            xStorProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ) ) >>= aMediaType;
            if ( ::comphelper::OStorageHelper::GetXStorageFormat( xStor ) >= SOFFICE_FILEFORMAT_8 )
            {
                sal_Bool bRepairPackage = sal_False;
                try
                {
                    xStorProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "RepairPackage" ) ) )
                        >>= bRepairPackage;
                } catch ( uno::Exception& )
                {}

                // check only if not in Repair mode
                if ( !bRepairPackage )
                {
                    ::rtl::OUString aStorVersion;
                    xStorProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Version" ) ) )
                        >>= aStorVersion;

                    // if the storage version is set in manifest.xml, it must be the same as in content.xml
                    // if not, set it explicitly to be used further ( it will work even for readonly storage )
                    // This workaround is not nice, but I see no other way to handle it, since there are
                    // ODF1.2 documents without version in manifest.xml
                    if ( !aStorVersion.isEmpty() )
                        bResult = aODFVersion.equals( aStorVersion );
                    else
                        xStorProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Version" ) ),
                                                      uno::makeAny( aODFVersion ) );

                    if ( bResult )
                    {
                        sal_Bool bInconsistent = sal_False;
                        xStorProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsInconsistent" ) ) )
                            >>= bInconsistent;
                        bResult = !bInconsistent;
                    }
                }
            }
        }
        catch( uno::Exception& )
        {}
    }

    return bResult;
}

void SvXMLImport::_CreateNumberFormatsSupplier()
{
    DBG_ASSERT( !mxNumberFormatsSupplier.is(),
                "number formats supplier already exists!" );
    if(mxModel.is())
        mxNumberFormatsSupplier =
            uno::Reference< util::XNumberFormatsSupplier> (mxModel, uno::UNO_QUERY);
}


void SvXMLImport::_CreateDataStylesImport()
{
    DBG_ASSERT( mpNumImport == NULL, "data styles import already exists!" );
    uno::Reference<util::XNumberFormatsSupplier> xNum =
        GetNumberFormatsSupplier();
    if ( xNum.is() )
        mpNumImport = new SvXMLNumFmtHelper(xNum, getServiceFactory());
}


sal_Unicode SvXMLImport::ConvStarBatsCharToStarSymbol( sal_Unicode c )
{
    sal_Unicode cNew = c;
    if( !mpImpl->hBatsFontConv )
    {
        OUString sStarBats( RTL_CONSTASCII_USTRINGPARAM( "StarBats" ) );
        mpImpl->hBatsFontConv = CreateFontToSubsFontConverter( sStarBats,
                 FONTTOSUBSFONT_IMPORT|FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS );
        OSL_ENSURE( mpImpl->hBatsFontConv, "Got no symbol font converter" );
    }
    if( mpImpl->hBatsFontConv )
    {
        cNew = ConvertFontToSubsFontChar( mpImpl->hBatsFontConv, c );
    }

    return cNew;
}

sal_Unicode SvXMLImport::ConvStarMathCharToStarSymbol( sal_Unicode c )
{
    sal_Unicode cNew = c;
    if( !mpImpl->hMathFontConv )
    {
        OUString sStarMath( RTL_CONSTASCII_USTRINGPARAM( "StarMath" ) );
        mpImpl->hMathFontConv = CreateFontToSubsFontConverter( sStarMath,
                 FONTTOSUBSFONT_IMPORT|FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS );
        OSL_ENSURE( mpImpl->hMathFontConv, "Got no symbol font converter" );
    }
    if( mpImpl->hMathFontConv )
    {
        cNew = ConvertFontToSubsFontChar( mpImpl->hMathFontConv, c );
    }

    return cNew;
}



void SvXMLImport::SetError(
    sal_Int32 nId,
    const Sequence<OUString>& rMsgParams,
    const OUString& rExceptionMessage,
    const Reference<xml::sax::XLocator>& rLocator )
{
    // maintain error flags
    if ( ( nId & XMLERROR_FLAG_ERROR ) != 0 )
        mnErrorFlags |= ERROR_ERROR_OCCURRED;
    if ( ( nId & XMLERROR_FLAG_WARNING ) != 0 )
        mnErrorFlags |= ERROR_WARNING_OCCURRED;
    if ( ( nId & XMLERROR_FLAG_SEVERE ) != 0 )
        mnErrorFlags |= ERROR_DO_NOTHING;

    // create error list on demand
    if ( mpXMLErrors == NULL )
        mpXMLErrors = new XMLErrors();

    // save error information
    // use document locator (if none supplied)
    mpXMLErrors->AddRecord( nId, rMsgParams, rExceptionMessage,
                           rLocator.is() ? rLocator : mxLocator );
}

void SvXMLImport::SetError(
    sal_Int32 nId,
    const Sequence<OUString>& rMsgParams)
{
    OUString sEmpty;
    SetError( nId, rMsgParams, sEmpty, NULL );
}

void SvXMLImport::SetError(
    sal_Int32 nId)
{
    Sequence<OUString> aSeq(0);
    SetError( nId, aSeq );
}

void SvXMLImport::SetError(
    sal_Int32 nId,
    const OUString& rMsg1)
{
    Sequence<OUString> aSeq(1);
    OUString* pSeq = aSeq.getArray();
    pSeq[0] = rMsg1;
    SetError( nId, aSeq );
}

void SvXMLImport::SetError(
    sal_Int32 nId,
    const OUString& rMsg1,
    const OUString& rMsg2)
{
    Sequence<OUString> aSeq(2);
    OUString* pSeq = aSeq.getArray();
    pSeq[0] = rMsg1;
    pSeq[1] = rMsg2;
    SetError( nId, aSeq );
}

void SvXMLImport::DisposingModel()
{
    if( mxFontDecls.Is() )
        ((SvXMLStylesContext *)&mxFontDecls)->Clear();
    if( mxStyles.Is() )
        ((SvXMLStylesContext *)&mxStyles)->Clear();
    if( mxAutoStyles.Is() )
        ((SvXMLStylesContext *)&mxAutoStyles)->Clear();
    if( mxMasterStyles.Is() )
        ((SvXMLStylesContext *)&mxMasterStyles)->Clear();

    mxModel.set(0);
    mxEventListener.set(NULL);
}

::comphelper::UnoInterfaceToUniqueIdentifierMapper& SvXMLImport::getInterfaceToIdentifierMapper()
{
    return mpImpl->maInterfaceToIdentifierMapper;
}

::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > SvXMLImport::getServiceFactory()
{
    // #110680#
    return mxServiceFactory;
}

uno::Reference< uno::XComponentContext >
SvXMLImport::GetComponentContext() const
{
    return mpImpl->mxComponentContext;
}

::rtl::OUString SvXMLImport::GetBaseURL() const
{
    return mpImpl->aBaseURL.GetMainURL( INetURLObject::NO_DECODE );
}

::rtl::OUString SvXMLImport::GetDocumentBase() const
{
    return mpImpl->aDocBase.GetMainURL( INetURLObject::NO_DECODE );
}

::rtl::OUString SvXMLImport::GetStreamName() const
{
    return mpImpl->mStreamName;
}

// Convert drawing object positions from OOo file format to OASIS (#i28749#)
sal_Bool SvXMLImport::IsShapePositionInHoriL2R() const
{
    return mpImpl->mbShapePositionInHoriL2R;
}

sal_Bool SvXMLImport::IsTextDocInOOoFileFormat() const
{
    return mpImpl->mbTextDocInOOoFileFormat;
}

void SvXMLImport::initXForms()
{
    // dummy method; to be implemented by derived classes supporting XForms
}

bool SvXMLImport::getBuildIds( sal_Int32& rUPD, sal_Int32& rBuild ) const
{
    bool bRet = false;
    if( mxImportInfo.is() ) try
    {
        const OUString aPropName(RTL_CONSTASCII_USTRINGPARAM("BuildId"));
        Reference< XPropertySetInfo > xSetInfo( mxImportInfo->getPropertySetInfo() );
        if( xSetInfo.is() && xSetInfo->hasPropertyByName( aPropName ) )
        {
            OUString aBuildId;
            mxImportInfo->getPropertyValue( aPropName ) >>= aBuildId;
            if( !aBuildId.isEmpty() )
            {
                sal_Int32 nIndex = aBuildId.indexOf('$');
                if( nIndex != -1 )
                {
                    rUPD = aBuildId.copy( 0, nIndex ).toInt32();
                    rBuild = aBuildId.copy( nIndex+1 ).toInt32();
                    bRet = true;
                }
            }
        }
    }
    catch( Exception& )
    {
    }
    return bRet;
}

sal_uInt16 SvXMLImport::getGeneratorVersion() const
{
    sal_Int32 nUPD, nBuild;

    if( getBuildIds( nUPD, nBuild ) )
    {
        if( nUPD == 680 )
            return OOo_2x;

        if( nUPD >= 640 && nUPD <= 645 )
            return OOo_1x;
    }

    return OOo_Current;
}

bool SvXMLImport::isGraphicLoadOnDemandSupported() const
{
    return mbIsGraphicLoadOnDemandSupported;
}

::rtl::OUString SvXMLImport::GetODFVersion() const
{
    return mpImpl->aODFVersion;
}

// xml:id for RDF metadata
void SvXMLImport::SetXmlId(uno::Reference<uno::XInterface> const & i_xIfc,
    ::rtl::OUString const & i_rXmlId)
{
    if (!i_rXmlId.isEmpty()) {
        try {
            const uno::Reference<rdf::XMetadatable> xMeta(i_xIfc,
                uno::UNO_QUERY);
//FIXME: not yet
            if (xMeta.is()) {
                const beans::StringPair mdref( GetStreamName(), i_rXmlId );
                try {
                    xMeta->setMetadataReference(mdref);
                } catch (lang::IllegalArgumentException &) {
                    // probably duplicate; ignore
                    OSL_TRACE("SvXMLImport::SetXmlId: cannot set xml:id");
                }
            }
        } catch (uno::Exception &) {
            OSL_FAIL("SvXMLImport::SetXmlId: exception?");
        }
    }
}

SAL_DLLPRIVATE ::xmloff::RDFaImportHelper &
SvXMLImport::GetRDFaImportHelper()
{
    if (!mpImpl->mpRDFaHelper.get())
    {
        mpImpl->mpRDFaHelper.reset( new ::xmloff::RDFaImportHelper(*this) );
    }
    return *mpImpl->mpRDFaHelper;
}

void
SvXMLImport::AddRDFa(uno::Reference<rdf::XMetadatable> i_xObject,
    ::rtl::OUString const & i_rAbout,
    ::rtl::OUString const & i_rProperty,
    ::rtl::OUString const & i_rContent,
    ::rtl::OUString const & i_rDatatype)
{
    // N.B.: we only get called if i_xObject had xhtml:about attribute
    // (an empty attribute value is valid)
    ::xmloff::RDFaImportHelper & rRDFaHelper( GetRDFaImportHelper() );
    rRDFaHelper.ParseAndAddRDFa(i_xObject,
        i_rAbout, i_rProperty, i_rContent, i_rDatatype);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
