/*************************************************************************
 *
 *  $RCSfile: sdxmlimp.cxx,v $
 *
 *  $Revision: 1.39 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:09:19 $
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

#pragma hdrstop

#ifndef _XMLOFF_XMLMETAI_HXX
#include "xmlscripti.hxx"
#endif

#ifndef _SDXMLIMP_HXX
#include "sdxmlimp.hxx"
#endif

#ifndef _SDXMLIMP_IMPL_HXX
#include "sdxmlimp_impl.hxx"
#endif

#ifndef _XIMPBODY_HXX
#include "ximpbody.hxx"
#endif

#ifndef _SFX_XMLMETAI_HXX
#include "xmlmetai.hxx"
#endif

#ifndef _XIMPSTYLE_HXX
#include "ximpstyl.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_DOCUMENTSETTINGSCONTEXT_HXX
#include "DocumentSettingsContext.hxx"
#endif

#ifndef _COM_SUN_STAR_FORM_XFORMSSUPPLIER_HPP_
#include <com/sun/star/form/XFormsSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORSUPPLIER_HPP_
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XMASTERPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#endif

#ifndef _SDPROPLS_HXX
#include "sdpropls.hxx"
#endif

#ifndef _XMLOFF_XMLEXPPR_HXX
#include "xmlexppr.hxx"
#endif

#ifndef _XMLOFF_XMLERROR_HXX
#include "xmlerror.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

//////////////////////////////////////////////////////////////////////////////

static __FAR_DATA SvXMLTokenMapEntry aDocElemTokenMap[] =
{
    { XML_NAMESPACE_OFFICE, XML_STYLES,             XML_TOK_DOC_STYLES          },
    { XML_NAMESPACE_OFFICE, XML_AUTOMATIC_STYLES,   XML_TOK_DOC_AUTOSTYLES      },
    { XML_NAMESPACE_OFFICE, XML_MASTER_STYLES,      XML_TOK_DOC_MASTERSTYLES    },
    { XML_NAMESPACE_OFFICE, XML_META,               XML_TOK_DOC_META            },
    { XML_NAMESPACE_OFFICE, XML_SCRIPTS,            XML_TOK_DOC_SCRIPT          },
    { XML_NAMESPACE_OFFICE, XML_BODY,               XML_TOK_DOC_BODY            },
    { XML_NAMESPACE_OFFICE, XML_SETTINGS,           XML_TOK_DOC_SETTINGS        },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aBodyElemTokenMap[] =
{
    { XML_NAMESPACE_DRAW,   XML_PAGE,               XML_TOK_BODY_PAGE   },
    { XML_NAMESPACE_PRESENTATION, XML_SETTINGS,     XML_TOK_BODY_SETTINGS   },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aStylesElemTokenMap[] =
{
    { XML_NAMESPACE_STYLE,  XML_PAGE_LAYOUT,                XML_TOK_STYLES_PAGE_MASTER              },
    { XML_NAMESPACE_STYLE,  XML_PRESENTATION_PAGE_LAYOUT,   XML_TOK_STYLES_PRESENTATION_PAGE_LAYOUT },
    { XML_NAMESPACE_STYLE,  XML_STYLE,                      XML_TOK_STYLES_STYLE    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aMasterPageElemTokenMap[] =
{
    { XML_NAMESPACE_STYLE,          XML_STYLE,      XML_TOK_MASTERPAGE_STYLE    },
    { XML_NAMESPACE_PRESENTATION,   XML_NOTES,      XML_TOK_MASTERPAGE_NOTES    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aMasterPageAttrTokenMap[] =
{
    { XML_NAMESPACE_STYLE,  XML_NAME,                       XML_TOK_MASTERPAGE_NAME },
    { XML_NAMESPACE_STYLE,  XML_PAGE_LAYOUT_NAME,           XML_TOK_MASTERPAGE_PAGE_MASTER_NAME },
    { XML_NAMESPACE_DRAW,   XML_STYLE_NAME,                 XML_TOK_MASTERPAGE_STYLE_NAME       },
    { XML_NAMESPACE_PRESENTATION,   XML_PRESENTATION_PAGE_LAYOUT_NAME,  XML_TOK_MASTERPAGE_PAGE_LAYOUT_NAME },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aPageMasterAttrTokenMap[] =
{
    { XML_NAMESPACE_STYLE,          XML_NAME,               XML_TOK_PAGEMASTER_NAME                 },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aPageMasterStyleAttrTokenMap[] =
{
    { XML_NAMESPACE_FO,             XML_MARGIN_TOP,         XML_TOK_PAGEMASTERSTYLE_MARGIN_TOP          },
    { XML_NAMESPACE_FO,             XML_MARGIN_BOTTOM,      XML_TOK_PAGEMASTERSTYLE_MARGIN_BOTTOM       },
    { XML_NAMESPACE_FO,             XML_MARGIN_LEFT,        XML_TOK_PAGEMASTERSTYLE_MARGIN_LEFT         },
    { XML_NAMESPACE_FO,             XML_MARGIN_RIGHT,       XML_TOK_PAGEMASTERSTYLE_MARGIN_RIGHT        },
    { XML_NAMESPACE_FO,             XML_PAGE_WIDTH,         XML_TOK_PAGEMASTERSTYLE_PAGE_WIDTH          },
    { XML_NAMESPACE_FO,             XML_PAGE_HEIGHT,        XML_TOK_PAGEMASTERSTYLE_PAGE_HEIGHT         },
    { XML_NAMESPACE_STYLE,          XML_PRINT_ORIENTATION,  XML_TOK_PAGEMASTERSTYLE_PAGE_ORIENTATION    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDrawPageAttrTokenMap[] =
{
    { XML_NAMESPACE_DRAW,           XML_NAME,                           XML_TOK_DRAWPAGE_NAME               },
    { XML_NAMESPACE_DRAW,           XML_STYLE_NAME,                     XML_TOK_DRAWPAGE_STYLE_NAME         },
    { XML_NAMESPACE_DRAW,           XML_MASTER_PAGE_NAME,               XML_TOK_DRAWPAGE_MASTER_PAGE_NAME   },
    { XML_NAMESPACE_PRESENTATION,   XML_PRESENTATION_PAGE_LAYOUT_NAME,  XML_TOK_DRAWPAGE_PAGE_LAYOUT_NAME   },
    { XML_NAMESPACE_DRAW,           XML_ID,                             XML_TOK_DRAWPAGE_ID                 },
    { XML_NAMESPACE_XLINK,          XML_HREF,                           XML_TOK_DRAWPAGE_HREF               },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDrawPageElemTokenMap[] =
{
    { XML_NAMESPACE_PRESENTATION,   XML_NOTES,              XML_TOK_DRAWPAGE_NOTES      },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aPresentationPlaceholderAttrTokenMap[] =
{
    { XML_NAMESPACE_PRESENTATION,   XML_OBJECT,     XML_TOK_PRESENTATIONPLACEHOLDER_OBJECTNAME  },
    { XML_NAMESPACE_SVG,            XML_X,          XML_TOK_PRESENTATIONPLACEHOLDER_X           },
    { XML_NAMESPACE_SVG,            XML_Y,          XML_TOK_PRESENTATIONPLACEHOLDER_Y           },
    { XML_NAMESPACE_SVG,            XML_WIDTH,      XML_TOK_PRESENTATIONPLACEHOLDER_WIDTH       },
    { XML_NAMESPACE_SVG,            XML_HEIGHT,     XML_TOK_PRESENTATIONPLACEHOLDER_HEIGHT      },
    XML_TOKEN_MAP_END
};

//////////////////////////////////////////////////////////////////////////////

class SdXMLBodyContext_Impl : public SvXMLImportContext
{
    const SdXMLImport& GetSdImport() const
        { return (const SdXMLImport&)GetImport(); }
    SdXMLImport& GetSdImport() { return (SdXMLImport&)GetImport(); }

public:

    SdXMLBodyContext_Impl( SdXMLImport& rImport, sal_uInt16 nPrfx,
                const OUString& rLName,
                const uno::Reference< xml::sax::XAttributeList > & xAttrList );
    virtual ~SdXMLBodyContext_Impl();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const OUString& rLocalName,
                const uno::Reference< xml::sax::XAttributeList > & xAttrList );
};

SdXMLBodyContext_Impl::SdXMLBodyContext_Impl( SdXMLImport& rImport,
                sal_uInt16 nPrfx, const OUString& rLName,
                const uno::Reference< xml::sax::XAttributeList > & xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
}

SdXMLBodyContext_Impl::~SdXMLBodyContext_Impl()
{
}

SvXMLImportContext *SdXMLBodyContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    return GetSdImport().CreateBodyContext(rLocalName, xAttrList);
}

//////////////////////////////////////////////////////////////////////////////

class SdXMLDocContext_Impl : public SvXMLImportContext
{
    const SdXMLImport& GetSdImport() const { return (const SdXMLImport&)GetImport(); }
    SdXMLImport& GetSdImport() { return (SdXMLImport&)GetImport(); }

public:
    SdXMLDocContext_Impl(
        SdXMLImport& rImport,
        USHORT nPrfx,
        const OUString& rLName,
        const uno::Reference<xml::sax::XAttributeList>& xAttrList);
    virtual ~SdXMLDocContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(USHORT nPrefix,
        const OUString& rLocalName,
        const uno::Reference<xml::sax::XAttributeList>& xAttrList);
};

//////////////////////////////////////////////////////////////////////////////

SdXMLDocContext_Impl::SdXMLDocContext_Impl(
    SdXMLImport& rImport,
    USHORT nPrfx,
    const OUString& rLName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
:   SvXMLImportContext(rImport, nPrfx, rLName)
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLDocContext_Impl::~SdXMLDocContext_Impl()
{
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext *SdXMLDocContext_Impl::CreateChildContext(
    USHORT nPrefix,
    const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    SvXMLImportContext* pContext = 0L;

    const SvXMLTokenMap& rTokenMap = GetSdImport().GetDocElemTokenMap();
    switch(rTokenMap.Get(nPrefix, rLocalName))
    {
        case XML_TOK_DOC_SETTINGS:
        {
            if( GetImport().getImportFlags() & IMPORT_SETTINGS )
            {
                pContext = new XMLDocumentSettingsContext(GetImport(), nPrefix, rLocalName, xAttrList );
            }
            break;
        }
        case XML_TOK_DOC_STYLES:
        {
            if( GetImport().getImportFlags() & IMPORT_STYLES )
            {
                // office:styles inside office:document
                pContext = GetSdImport().CreateStylesContext(rLocalName, xAttrList);
            }
            break;
        }
        case XML_TOK_DOC_AUTOSTYLES:
        {
            if( GetImport().getImportFlags() & IMPORT_AUTOSTYLES )
            {
                // office:automatic-styles inside office:document
                pContext = GetSdImport().CreateAutoStylesContext(rLocalName, xAttrList);
            }
            break;
        }
        case XML_TOK_DOC_MASTERSTYLES:
        {
            if( GetImport().getImportFlags() & IMPORT_MASTERSTYLES )
            {
                // office:master-styles inside office:document
                pContext = GetSdImport().CreateMasterStylesContext(rLocalName, xAttrList);
            }
            break;
        }
        case XML_TOK_DOC_META:
        {
            if( GetImport().getImportFlags() & IMPORT_META )
            {
                // office:meta inside office:document
                pContext = GetSdImport().CreateMetaContext(rLocalName, xAttrList);
            }
            break;
        }
        case XML_TOK_DOC_SCRIPT:
        {
            if( GetImport().getImportFlags() & IMPORT_SCRIPTS )
            {
                // office:script inside office:document
                pContext = GetSdImport().CreateScriptContext( rLocalName );
            }
            break;
        }
        case XML_TOK_DOC_BODY:
        {
            if( GetImport().getImportFlags() & IMPORT_CONTENT )
            {
                // office:body inside office:document
                pContext = new SdXMLBodyContext_Impl(GetSdImport(),nPrefix,
                                                     rLocalName, xAttrList);
            }
            break;
        }
    }

    // call parent when no own context was created
    if(!pContext)
        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}

//////////////////////////////////////////////////////////////////////////////

#define SERVICE(classname,servicename,implementationname,draw,flags)\
uno::Sequence< OUString > SAL_CALL classname##_getSupportedServiceNames() throw()\
{\
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( servicename ) );\
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );\
    return aSeq;\
}\
OUString SAL_CALL classname##_getImplementationName() throw()\
{\
    return OUString( RTL_CONSTASCII_USTRINGPARAM( implementationname ) );\
}\
uno::Reference< uno::XInterface > SAL_CALL classname##_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )\
{\
    return (cppu::OWeakObject*)new SdXMLImport( rSMgr, draw, flags );\
}

SERVICE( XMLImpressImportOasis, "com.sun.star.comp.Impress.XMLOasisImporter", "XMLImpressImportOasis", sal_False, IMPORT_ALL )
SERVICE( XMLDrawImportOasis, "com.sun.star.comp.Draw.XMLOasisImporter", "XMLDrawImportOasis", sal_True, IMPORT_ALL )

SERVICE( XMLImpressStylesImportOasis, "com.sun.star.comp.Impress.XMLOasisStylesImporter", "XMLImpressStylesImportOasis", sal_False, IMPORT_STYLES|IMPORT_AUTOSTYLES|IMPORT_MASTERSTYLES )
SERVICE( XMLDrawStylesImportOasis, "com.sun.star.comp.Draw.XMLOasisStylesImporter", "XMLImpressStylesImportOasis", sal_True, IMPORT_STYLES|IMPORT_AUTOSTYLES|IMPORT_MASTERSTYLES )

SERVICE( XMLImpressContentImportOasis, "com.sun.star.comp.Impress.XMLOasisContentImporter", "XMLImpressContentImportOasis", sal_False, IMPORT_AUTOSTYLES|IMPORT_CONTENT|IMPORT_SCRIPTS|IMPORT_FONTDECLS )
SERVICE( XMLDrawContentImportOasis, "com.sun.star.comp.Draw.XMLOasisContentImporter", "XMLImpressContentImportOasis", sal_True, IMPORT_AUTOSTYLES|IMPORT_CONTENT|IMPORT_SCRIPTS|IMPORT_FONTDECLS )

SERVICE( XMLImpressMetaImportOasis, "com.sun.star.comp.Impress.XMLOasisMetaImporter", "XMLImpressMetaImportOasis", sal_False, IMPORT_META )
SERVICE( XMLDrawMetaImportOasis, "com.sun.star.comp.Draw.XMLOasisMetaImporter", "XMLImpressMetaImportOasis", sal_True, IMPORT_META )

SERVICE( XMLImpressSettingsImportOasis, "com.sun.star.comp.Impress.XMLOasisSettingsImporter", "XMLImpressSettingsImportOasis", sal_False, IMPORT_META )
SERVICE( XMLDrawSettingsImportOasis, "com.sun.star.comp.Draw.XMLOasisSettingsImporter", "XMLImpressSettingsImportOasis", sal_True, IMPORT_META )

//////////////////////////////////////////////////////////////////////////////

// #110680#
SdXMLImport::SdXMLImport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    sal_Bool bIsDraw, sal_uInt16 nImportFlags )
:   SvXMLImport( xServiceFactory, nImportFlags ),
    mpMasterStylesContext(0L),
    mpDocElemTokenMap(0L),
    mpBodyElemTokenMap(0L),
    mpStylesElemTokenMap(0L),
    mpMasterPageElemTokenMap(0L),
    mpMasterPageAttrTokenMap(0L),
    mpPageMasterAttrTokenMap(0L),
    mpPageMasterStyleAttrTokenMap(0L),
    mpDrawPageAttrTokenMap(0L),
    mpDrawPageElemTokenMap(0L),
    mpPresentationPlaceholderAttrTokenMap(0L),
    mnStyleFamilyMask(0),
    mnNewPageCount(0L),
    mnNewMasterPageCount(0L),
    mbLoadDoc(sal_True),
    mbIsDraw(bIsDraw),
    mbPreview(sal_False),
    msPageLayouts( RTL_CONSTASCII_USTRINGPARAM( "PageLayouts" ) ),
    msPreview( RTL_CONSTASCII_USTRINGPARAM( "Preview" ) )
{
    // add namespaces
    GetNamespaceMap().Add(
        GetXMLToken(XML_NP_PRESENTATION),
        GetXMLToken(XML_N_PRESENTATION),
        XML_NAMESPACE_PRESENTATION);
}

// XImporter
void SAL_CALL SdXMLImport::setTargetDocument( const uno::Reference< lang::XComponent >& xDoc )
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    SvXMLImport::setTargetDocument( xDoc );

    uno::Reference< lang::XServiceInfo > xDocServices( GetModel(), uno::UNO_QUERY );
    if( !xDocServices.is() )
        throw lang::IllegalArgumentException();

    mbIsDraw = !xDocServices->supportsService( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.PresentationDocument" ) ) );

    // prepare access to styles
    uno::Reference< style::XStyleFamiliesSupplier > xFamSup( GetModel(), uno::UNO_QUERY );
    if(xFamSup.is())
        mxDocStyleFamilies = xFamSup->getStyleFamilies();

    // prepare access to master pages
    uno::Reference < drawing::XMasterPagesSupplier > xMasterPagesSupplier(GetModel(), uno::UNO_QUERY);
    if(xMasterPagesSupplier.is())
        mxDocMasterPages = mxDocMasterPages.query( xMasterPagesSupplier->getMasterPages() );

    // prepare access to draw pages
    uno::Reference <drawing::XDrawPagesSupplier> xDrawPagesSupplier(GetModel(), uno::UNO_QUERY);
    if(!xDrawPagesSupplier.is())
        throw lang::IllegalArgumentException();

    mxDocDrawPages = mxDocDrawPages.query( xDrawPagesSupplier->getDrawPages() );
    if(!mxDocDrawPages.is())
        throw lang::IllegalArgumentException();

    if( mxDocDrawPages.is() && mxDocDrawPages->getCount() > 0 )
    {
        uno::Reference< form::XFormsSupplier > xFormsSupp;
        mxDocDrawPages->getByIndex(0) >>= xFormsSupp;
        mbIsFormsSupported = xFormsSupp.is();
    }

    // #88546# enable progress bar increments, SdXMLImport is only used for
    // draw/impress import
    GetShapeImport()->enableHandleProgressBar();
}

// XInitialization
void SAL_CALL SdXMLImport::initialize( const uno::Sequence< uno::Any >& aArguments )
    throw( uno::Exception, uno::RuntimeException)
{
    SvXMLImport::initialize( aArguments );

    uno::Reference< beans::XPropertySet > xInfoSet( getImportInfo() );
    if( xInfoSet.is() )
    {
        uno::Reference< beans::XPropertySetInfo > xInfoSetInfo( xInfoSet->getPropertySetInfo() );

        if( xInfoSetInfo->hasPropertyByName( msPageLayouts ) )
            xInfoSet->getPropertyValue( msPageLayouts ) >>= mxPageLayouts;

        if( xInfoSetInfo->hasPropertyByName( msPreview ) )
            xInfoSet->getPropertyValue( msPreview ) >>= mbPreview;
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXMLImport::~SdXMLImport() throw ()
{
// #80365# removed ending of progress bar here, this was an old implementation
// and maybe removed on demand
//  // stop progress view
//  if(mxStatusIndicator.is())
//  {
//      mxStatusIndicator->end();
//      mxStatusIndicator->reset();
//  }

    // Styles or AutoStyles context?
    if(mpMasterStylesContext)
        mpMasterStylesContext->ReleaseRef();

    // delete all token maps
    if(mpDocElemTokenMap)
        delete mpDocElemTokenMap;
    if(mpBodyElemTokenMap)
        delete mpBodyElemTokenMap;
    if(mpStylesElemTokenMap)
        delete mpStylesElemTokenMap;
    if(mpMasterPageElemTokenMap)
        delete mpMasterPageElemTokenMap;
    if(mpMasterPageAttrTokenMap)
        delete mpMasterPageAttrTokenMap;
    if(mpPageMasterAttrTokenMap)
        delete mpPageMasterAttrTokenMap;
    if(mpPageMasterStyleAttrTokenMap)
        delete mpPageMasterStyleAttrTokenMap;
    if(mpDrawPageAttrTokenMap)
        delete mpDrawPageAttrTokenMap;
    if(mpDrawPageElemTokenMap)
        delete mpDrawPageElemTokenMap;
    if(mpPresentationPlaceholderAttrTokenMap)
        delete mpPresentationPlaceholderAttrTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLImport::SetProgress(sal_Int32 nProg)
{
    // set progress view
    if(mxStatusIndicator.is())
        mxStatusIndicator->setValue(nProg);
}

//////////////////////////////////////////////////////////////////////////////

const SvXMLTokenMap& SdXMLImport::GetDocElemTokenMap()
{
    if(!mpDocElemTokenMap)
        mpDocElemTokenMap = new SvXMLTokenMap(aDocElemTokenMap);
    return *mpDocElemTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

const SvXMLTokenMap& SdXMLImport::GetBodyElemTokenMap()
{
    if(!mpBodyElemTokenMap)
        mpBodyElemTokenMap = new SvXMLTokenMap(aBodyElemTokenMap);
    return *mpBodyElemTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

const SvXMLTokenMap& SdXMLImport::GetStylesElemTokenMap()
{
    if(!mpStylesElemTokenMap)
        mpStylesElemTokenMap = new SvXMLTokenMap(aStylesElemTokenMap);
    return *mpStylesElemTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

const SvXMLTokenMap& SdXMLImport::GetMasterPageElemTokenMap()
{
    if(!mpMasterPageElemTokenMap)
        mpMasterPageElemTokenMap = new SvXMLTokenMap(aMasterPageElemTokenMap);
    return *mpMasterPageElemTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

const SvXMLTokenMap& SdXMLImport::GetMasterPageAttrTokenMap()
{
    if(!mpMasterPageAttrTokenMap)
        mpMasterPageAttrTokenMap = new SvXMLTokenMap(aMasterPageAttrTokenMap);
    return *mpMasterPageAttrTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

const SvXMLTokenMap& SdXMLImport::GetPageMasterAttrTokenMap()
{
    if(!mpPageMasterAttrTokenMap)
        mpPageMasterAttrTokenMap = new SvXMLTokenMap(aPageMasterAttrTokenMap);
    return *mpPageMasterAttrTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

const SvXMLTokenMap& SdXMLImport::GetPageMasterStyleAttrTokenMap()
{
    if(!mpPageMasterStyleAttrTokenMap)
        mpPageMasterStyleAttrTokenMap = new SvXMLTokenMap(aPageMasterStyleAttrTokenMap);
    return *mpPageMasterStyleAttrTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

const SvXMLTokenMap& SdXMLImport::GetDrawPageAttrTokenMap()
{
    if(!mpDrawPageAttrTokenMap)
        mpDrawPageAttrTokenMap = new SvXMLTokenMap(aDrawPageAttrTokenMap);
    return *mpDrawPageAttrTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

const SvXMLTokenMap& SdXMLImport::GetDrawPageElemTokenMap()
{
    if(!mpDrawPageElemTokenMap)
        mpDrawPageElemTokenMap = new SvXMLTokenMap(aDrawPageElemTokenMap);
    return *mpDrawPageElemTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

const SvXMLTokenMap& SdXMLImport::GetPresentationPlaceholderAttrTokenMap()
{
    if(!mpPresentationPlaceholderAttrTokenMap)
        mpPresentationPlaceholderAttrTokenMap = new SvXMLTokenMap(aPresentationPlaceholderAttrTokenMap);
    return *mpPresentationPlaceholderAttrTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext *SdXMLImport::CreateContext(USHORT nPrefix,
    const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    SvXMLImportContext* pContext = 0;

    if(XML_NAMESPACE_OFFICE == nPrefix &&
        ( IsXMLToken( rLocalName, XML_DOCUMENT ) ||
          IsXMLToken( rLocalName, XML_DOCUMENT_META ) ||
          IsXMLToken( rLocalName, XML_DOCUMENT_STYLES ) ||
          IsXMLToken( rLocalName, XML_DOCUMENT_CONTENT ) ||
          IsXMLToken( rLocalName, XML_DOCUMENT_SETTINGS )   ))
    {
         pContext = new SdXMLDocContext_Impl(*this, nPrefix, rLocalName, xAttrList);
    }
    else
    {
        pContext = SvXMLImport::CreateContext(nPrefix, rLocalName, xAttrList);
    }

    return pContext;
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext *SdXMLImport::CreateMetaContext(const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    SvXMLImportContext* pContext = 0L;

    if(!IsStylesOnlyMode())
    {
        uno::Reference<document::XDocumentInfoSupplier> xSupp(GetModel(), uno::UNO_QUERY);

        if(xSupp.is())
        {
            pContext = new SfxXMLMetaContext(*this,
                XML_NAMESPACE_OFFICE, rLocalName, GetModel());
        }
    }

    if(!pContext)
    {
        pContext = new SvXMLImportContext(*this, XML_NAMESPACE_OFFICE, rLocalName);
    }

    return pContext;
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext *SdXMLImport::CreateBodyContext(const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    SvXMLImportContext *pContext = 0;
    pContext = new SdXMLBodyContext(*this, XML_NAMESPACE_OFFICE, rLocalName);
    return pContext;
}

//////////////////////////////////////////////////////////////////////////////

SvXMLStylesContext *SdXMLImport::CreateStylesContext(const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    if(GetShapeImport()->GetStylesContext())
        return GetShapeImport()->GetStylesContext();

    GetShapeImport()->SetStylesContext(new SdXMLStylesContext(
        *this, XML_NAMESPACE_OFFICE, rLocalName, xAttrList, FALSE));

    return GetShapeImport()->GetStylesContext();
}

//////////////////////////////////////////////////////////////////////////////

SvXMLStylesContext *SdXMLImport::CreateAutoStylesContext(const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    if(GetShapeImport()->GetAutoStylesContext())
        return GetShapeImport()->GetAutoStylesContext();

    GetShapeImport()->SetAutoStylesContext(new SdXMLStylesContext(
        *this, XML_NAMESPACE_OFFICE, rLocalName, xAttrList, TRUE));

    return GetShapeImport()->GetAutoStylesContext();
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext* SdXMLImport::CreateMasterStylesContext(const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    if(mpMasterStylesContext)
        return mpMasterStylesContext;

    mpMasterStylesContext = new SdXMLMasterStylesContext(
        *this, XML_NAMESPACE_OFFICE, rLocalName);
    mpMasterStylesContext->AddRef();

    return mpMasterStylesContext;
}

//////////////////////////////////////////////////////////////////////////////
// import pool defaults. Parameter contains pool defaults read
// from input data. These data needs to be set at the model.
//
void SdXMLImport::ImportPoolDefaults(const XMLPropStyleContext* pPool)
{
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext *SdXMLImport::CreateScriptContext(
                                       const OUString& rLocalName )
{
    SvXMLImportContext *pContext = 0;

    pContext = new XMLScriptContext( *this,
                                    XML_NAMESPACE_OFFICE, rLocalName,
                                    GetModel() );
    return pContext;
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLImport::setDrawPageId( sal_Int32 nId, uno::Reference< drawing::XDrawPage > xPage )
{
    maDrawPageIds[nId] = xPage;
}

uno::Reference< drawing::XDrawPage > SdXMLImport::getDrawPageForId( sal_Int32 nId )
{
    uno::Reference< drawing::XDrawPage > xPage;

    DrawPageIdMap::iterator aFound( maDrawPageIds.find( nId ) );
    if( aFound != maDrawPageIds.end() )
        xPage = (*aFound).second;

    return xPage;
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLImport::SetViewSettings(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aViewProps)
{
    uno::Reference< beans::XPropertySet > xPropSet( GetModel(), uno::UNO_QUERY );
    if( !xPropSet.is() )
        return;

    awt::Rectangle aVisArea( 0,0, 28000, 21000 );
    sal_Int32 nCount = aViewProps.getLength();

    const beans::PropertyValue* pValues = aViewProps.getConstArray();

    while( nCount-- )
    {
        const OUString& rName = pValues->Name;
        const uno::Any rValue = pValues->Value;

        if( rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("VisibleAreaTop") ) )
        {
            rValue >>= aVisArea.Y;
        }
        else if( rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("VisibleAreaLeft") ) )
        {
            rValue >>= aVisArea.X;
        }
        else if( rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("VisibleAreaWidth") ) )
        {
            rValue >>= aVisArea.Width;
        }
        else if( rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("VisibleAreaHeight") ) )
        {
            rValue >>= aVisArea.Height;
        }

        pValues++;
    }

    try
    {
        xPropSet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "VisibleArea" ) ), uno::makeAny( aVisArea )  );
    }
    catch( com::sun::star::uno::Exception e )
    {
        uno::Sequence<OUString> aSeq(0);
        SetError( XMLERROR_FLAG_WARNING | XMLERROR_API, aSeq, e.Message, NULL );
    }
}

void SdXMLImport::SetConfigurationSettings(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aConfigProps)
{
    uno::Reference< lang::XMultiServiceFactory > xFac( GetModel(), uno::UNO_QUERY );
    if( !xFac.is() )
        return;

    uno::Reference< beans::XPropertySet > xProps( xFac->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.Settings" ) ) ), uno::UNO_QUERY );
    if( !xProps.is() )
        return;

    uno::Reference< beans::XPropertySetInfo > xInfo( xProps->getPropertySetInfo() );
    if( !xInfo.is() )
        return;

    sal_Int32 nCount = aConfigProps.getLength();
    const beans::PropertyValue* pValues = aConfigProps.getConstArray();

    while( nCount-- )
    {
        try
        {
            if( xInfo->hasPropertyByName( pValues->Name ) )
            {
                xProps->setPropertyValue( pValues->Name, pValues->Value );
            }
        }
        catch( uno::Exception& )
        {
            OSL_TRACE( "#SdXMLImport::SetConfigurationSettings: Exception!" );
        }

        pValues++;
    }
}

// #80365# overload this method to read and use the hint value from the
// written meta information. If no info is found, guess 10 draw objects
void SdXMLImport::SetStatisticAttributes(const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    SvXMLImport::SetStatisticAttributes(xAttrList);

    sal_uInt32 nCount(10);
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);

    for(sal_Int16 a(0); a < nAttrCount; a++)
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex(a);
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetNamespaceMap().GetKeyByAttrName(sAttrName, &aLocalName);

        if(nPrefix == XML_NAMESPACE_META)
        {
            rtl::OUString sValue = xAttrList->getValueByIndex(a);
            sal_Int32 nValue(0);

            if(IsXMLToken(aLocalName, XML_OBJECT_COUNT))
            {
                SvXMLUnitConverter::convertNumber(nValue, sValue);
                nCount = nValue;
            }
        }
    }

    if(nCount)
    {
        GetProgressBarHelper()->SetReference(nCount);
        GetProgressBarHelper()->SetValue(0);
    }
}


// XServiceInfo
OUString SAL_CALL SdXMLImport::getImplementationName() throw( uno::RuntimeException )
{
    if( IsDraw())
    {
        // Draw

        switch( getImportFlags())
        {
            case IMPORT_ALL:
                return XMLDrawImportOasis_getImplementationName();
            case (IMPORT_STYLES|IMPORT_AUTOSTYLES|IMPORT_MASTERSTYLES):
                return XMLDrawStylesImportOasis_getImplementationName();
            case (IMPORT_AUTOSTYLES|IMPORT_CONTENT|IMPORT_SCRIPTS|IMPORT_FONTDECLS):
                return XMLDrawContentImportOasis_getImplementationName();
            case IMPORT_META:
                return XMLDrawMetaImportOasis_getImplementationName();
            case IMPORT_SETTINGS:
                return XMLDrawSettingsImportOasis_getImplementationName();
            default:
                return XMLDrawImportOasis_getImplementationName();
        }
    }
    else
    {
        // Impress

        switch( getImportFlags())
        {
            case IMPORT_ALL:
                return XMLImpressImportOasis_getImplementationName();
            case (IMPORT_STYLES|IMPORT_AUTOSTYLES|IMPORT_MASTERSTYLES):
                return XMLImpressStylesImportOasis_getImplementationName();
            case (IMPORT_AUTOSTYLES|IMPORT_CONTENT|IMPORT_SCRIPTS|IMPORT_FONTDECLS):
                return XMLImpressContentImportOasis_getImplementationName();
            case IMPORT_META:
                return XMLImpressMetaImportOasis_getImplementationName();
            case IMPORT_SETTINGS:
                return XMLImpressSettingsImportOasis_getImplementationName();
            default:
                return XMLImpressImportOasis_getImplementationName();
        }
    }
}

// eof
