/*************************************************************************
 *
 *  $RCSfile: sdxmlimp.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-04 16:10:26 $
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

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _SD_XMLVIEWSETTINGSCONTEXT_HXX
#include "viewcontext.hxx"
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////

static __FAR_DATA SvXMLTokenMapEntry aDocElemTokenMap[] =
{
    { XML_NAMESPACE_OFFICE, sXML_styles,            XML_TOK_DOC_STYLES          },
    { XML_NAMESPACE_OFFICE, sXML_automatic_styles,  XML_TOK_DOC_AUTOSTYLES      },
    { XML_NAMESPACE_OFFICE, sXML_master_styles,     XML_TOK_DOC_MASTERSTYLES    },
    { XML_NAMESPACE_OFFICE, sXML_meta,              XML_TOK_DOC_META            },
    { XML_NAMESPACE_OFFICE, sXML_script,            XML_TOK_DOC_SCRIPT          },
    { XML_NAMESPACE_OFFICE, sXML_body,              XML_TOK_DOC_BODY            },
    { XML_NAMESPACE_DRAW,   sXML_view_settings,     XML_TOK_DOC_VIEWSETTINGS    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aBodyElemTokenMap[] =
{
    { XML_NAMESPACE_DRAW,   sXML_page,              XML_TOK_BODY_PAGE   },
    { XML_NAMESPACE_PRESENTATION, sXML_settings,    XML_TOK_BODY_SETTINGS   },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aStylesElemTokenMap[] =
{
    { XML_NAMESPACE_STYLE,  sXML_page_master,               XML_TOK_STYLES_PAGE_MASTER              },
    { XML_NAMESPACE_STYLE,  sXML_presentation_page_layout,  XML_TOK_STYLES_PRESENTATION_PAGE_LAYOUT },
    { XML_NAMESPACE_STYLE,  sXML_style,                     XML_TOK_STYLES_STYLE    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aMasterPageElemTokenMap[] =
{
    { XML_NAMESPACE_STYLE,          sXML_style,     XML_TOK_MASTERPAGE_STYLE    },
    { XML_NAMESPACE_PRESENTATION,   sXML_notes,     XML_TOK_MASTERPAGE_NOTES    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aMasterPageAttrTokenMap[] =
{
    { XML_NAMESPACE_STYLE,  sXML_name,              XML_TOK_MASTERPAGE_NAME },
    { XML_NAMESPACE_STYLE,  sXML_page_master_name,  XML_TOK_MASTERPAGE_PAGE_MASTER_NAME },
    { XML_NAMESPACE_DRAW,   sXML_style_name,        XML_TOK_MASTERPAGE_STYLE_NAME       },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aPageMasterAttrTokenMap[] =
{
    { XML_NAMESPACE_STYLE,          sXML_name,              XML_TOK_PAGEMASTER_NAME                 },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aPageMasterStyleAttrTokenMap[] =
{
    { XML_NAMESPACE_FO,             sXML_margin_top,        XML_TOK_PAGEMASTERSTYLE_MARGIN_TOP          },
    { XML_NAMESPACE_FO,             sXML_margin_bottom,     XML_TOK_PAGEMASTERSTYLE_MARGIN_BOTTOM       },
    { XML_NAMESPACE_FO,             sXML_margin_left,       XML_TOK_PAGEMASTERSTYLE_MARGIN_LEFT         },
    { XML_NAMESPACE_FO,             sXML_margin_right,      XML_TOK_PAGEMASTERSTYLE_MARGIN_RIGHT        },
    { XML_NAMESPACE_FO,             sXML_page_width,        XML_TOK_PAGEMASTERSTYLE_PAGE_WIDTH          },
    { XML_NAMESPACE_FO,             sXML_page_height,       XML_TOK_PAGEMASTERSTYLE_PAGE_HEIGHT         },
    { XML_NAMESPACE_STYLE,          sXML_print_orientation, XML_TOK_PAGEMASTERSTYLE_PAGE_ORIENTATION    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDrawPageAttrTokenMap[] =
{
    { XML_NAMESPACE_DRAW,           sXML_name,                          XML_TOK_DRAWPAGE_NAME               },
    { XML_NAMESPACE_DRAW,           sXML_style_name,                    XML_TOK_DRAWPAGE_STYLE_NAME         },
    { XML_NAMESPACE_DRAW,           sXML_master_page_name,              XML_TOK_DRAWPAGE_MASTER_PAGE_NAME   },
    { XML_NAMESPACE_PRESENTATION,   sXML_presentation_page_layout_name, XML_TOK_DRAWPAGE_PAGE_LAYOUT_NAME   },
    { XML_NAMESPACE_DRAW,           sXML_id,                            XML_TOK_DRAWPAGE_ID                 },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDrawPageElemTokenMap[] =
{
    { XML_NAMESPACE_PRESENTATION,   sXML_notes,             XML_TOK_DRAWPAGE_NOTES      },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aPresentationPlaceholderAttrTokenMap[] =
{
    { XML_NAMESPACE_PRESENTATION,   sXML_object,    XML_TOK_PRESENTATIONPLACEHOLDER_OBJECTNAME  },
    { XML_NAMESPACE_SVG,            sXML_x,         XML_TOK_PRESENTATIONPLACEHOLDER_X           },
    { XML_NAMESPACE_SVG,            sXML_y,         XML_TOK_PRESENTATIONPLACEHOLDER_Y           },
    { XML_NAMESPACE_SVG,            sXML_width,     XML_TOK_PRESENTATIONPLACEHOLDER_WIDTH       },
    { XML_NAMESPACE_SVG,            sXML_height,    XML_TOK_PRESENTATIONPLACEHOLDER_HEIGHT      },
    XML_TOKEN_MAP_END
};

//////////////////////////////////////////////////////////////////////////////
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
        case XML_TOK_DOC_VIEWSETTINGS:
        {
            if( GetImport().getImportFlags() & IMPORT_SETTINGS )
            {
                // draw:view-settings inside office:document
                pContext = GetSdImport().CreateViewSettingsContext(rLocalName, xAttrList);
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
                pContext = GetSdImport().CreateBodyContext(rLocalName, xAttrList);
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

uno::Sequence< OUString > SAL_CALL SdImpressXMLImport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Impress.XMLImporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SdImpressXMLImport_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdXMLImport.Impress" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SdImpressXMLImport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SdXMLImport( sal_False );
}

uno::Sequence< OUString > SAL_CALL SdDrawXMLImport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Draw.XMLImporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SdDrawXMLImport_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdXMLImport.Draw" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SdDrawXMLImport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SdXMLImport( sal_True );
}

//////////////////////////////////////////////////////////////////////////////

uno::Sequence< OUString > SAL_CALL SdImpressXMLImport_Style_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Impress.XMLStylesImporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SdImpressXMLImport_Style_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdXMLImport.Impress.Styles" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SdImpressXMLImport_Style_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SdXMLImport( sal_False, IMPORT_STYLES|IMPORT_AUTOSTYLES|IMPORT_MASTERSTYLES );
}

uno::Sequence< OUString > SAL_CALL SdDrawXMLImport_Style_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Draw.XMLStylesImporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SdDrawXMLImport_Style_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdXMLImport.Draw.Styles" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SdDrawXMLImport_Style_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SdXMLImport( sal_True, IMPORT_STYLES|IMPORT_AUTOSTYLES|IMPORT_MASTERSTYLES );
}

//////////////////////////////////////////////////////////////////////////////

uno::Sequence< OUString > SAL_CALL SdImpressXMLImport_Content_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Impress.XMLContentImporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SdImpressXMLImport_Content_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdXMLImport.Impress.Content" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SdImpressXMLImport_Content_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SdXMLImport( sal_False, IMPORT_AUTOSTYLES|IMPORT_CONTENT|IMPORT_SCRIPTS|IMPORT_SETTINGS|IMPORT_FONTDECLS );
}

uno::Sequence< OUString > SAL_CALL SdDrawXMLImport_Content_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Draw.XMLContentImporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SdDrawXMLImport_Content_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdXMLImport.Draw.Content" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SdDrawXMLImport_Content_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SdXMLImport( sal_True, IMPORT_AUTOSTYLES|IMPORT_CONTENT|IMPORT_SCRIPTS|IMPORT_SETTINGS|IMPORT_FONTDECLS );
}

//////////////////////////////////////////////////////////////////////////////

uno::Sequence< OUString > SAL_CALL SdImpressXMLImport_Meta_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Impress.XMLMetaImporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SdImpressXMLImport_Meta_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdXMLImport.Impress.Meta" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SdImpressXMLImport_Meta_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SdXMLImport( sal_False, IMPORT_META );
}

uno::Sequence< OUString > SAL_CALL SdDrawXMLImport_Meta_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Draw.XMLMetaImporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SdDrawXMLImport_Meta_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdXMLImport.Draw.Meta" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SdDrawXMLImport_Meta_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SdXMLImport( sal_True, IMPORT_META );
}

//////////////////////////////////////////////////////////////////////////////

SdXMLImport::SdXMLImport( sal_Bool bIsDraw, sal_uInt16 nImportFlags )
:   SvXMLImport( nImportFlags ),
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
    mbIsDraw(bIsDraw)
{
    // add namespaces
    GetNamespaceMap().AddAtIndex(
        XML_NAMESPACE_PRESENTATION, sXML_np_presentation, sXML_n_presentation, XML_NAMESPACE_PRESENTATION);
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
    if(!xFamSup.is())
        throw lang::IllegalArgumentException();

    mxDocStyleFamilies = xFamSup->getStyleFamilies();
    if(!mxDocStyleFamilies.is())
        throw lang::IllegalArgumentException();

    // prepare access to master pages
    uno::Reference < drawing::XMasterPagesSupplier > xMasterPagesSupplier(GetModel(), uno::UNO_QUERY);
    if(!xMasterPagesSupplier.is())
        throw lang::IllegalArgumentException();

    mxDocMasterPages = mxDocMasterPages.query( xMasterPagesSupplier->getMasterPages() );
    if(!mxDocMasterPages.is())
        throw lang::IllegalArgumentException();

    // prepare access to draw pages
    uno::Reference <drawing::XDrawPagesSupplier> xDrawPagesSupplier(GetModel(), uno::UNO_QUERY);
    if(!xDrawPagesSupplier.is())
        throw lang::IllegalArgumentException();

    mxDocDrawPages = mxDocDrawPages.query( xDrawPagesSupplier->getDrawPages() );
    if(!mxDocDrawPages.is())
        throw lang::IllegalArgumentException();
}

//////////////////////////////////////////////////////////////////////////////

SdXMLImport::~SdXMLImport()
{
    // stop progress view
    if(mxStatusIndicator.is())
    {
        mxStatusIndicator->end();
        mxStatusIndicator->reset();
    }

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
        ( 0 == rLocalName.compareToAscii(sXML_document) ||
          0 == rLocalName.compareToAscii(sXML_document_meta) ||
          0 == rLocalName.compareToAscii(sXML_document_styles) ||
          0 == rLocalName.compareToAscii(sXML_document_content) ))
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

SvXMLImportContext* SdXMLImport::CreateViewSettingsContext(const rtl::OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList)
{
    return new SdXMLViewSettingsContext( *this, XML_NAMESPACE_DRAW, rLocalName, xAttrList );
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

