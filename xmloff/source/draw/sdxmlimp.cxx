/*************************************************************************
 *
 *  $RCSfile: sdxmlimp.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:03 $
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

#ifndef _SDXMLIMP_HXX
#include "sdxmlimp.hxx"
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
    { XML_NAMESPACE_OFFICE, sXML_body,              XML_TOK_DOC_BODY            },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aBodyElemTokenMap[] =
{
    { XML_NAMESPACE_DRAW,   sXML_page,              XML_TOK_BODY_PAGE   },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aStylesElemTokenMap[] =
{
    { XML_NAMESPACE_STYLE,  sXML_page_master,               XML_TOK_STYLES_PAGE_MASTER              },
    { XML_NAMESPACE_STYLE,  sXML_presentation_page_layout,  XML_TOK_STYLES_PRESENTATION_PAGE_LAYOUT },
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
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aPageMasterAttrTokenMap[] =
{
    { XML_NAMESPACE_STYLE,          sXML_name,              XML_TOK_PAGEMASTER_NAME                 },
    { XML_NAMESPACE_FO,             sXML_margin_top,        XML_TOK_PAGEMASTER_MARGIN_TOP           },
    { XML_NAMESPACE_FO,             sXML_margin_bottom,     XML_TOK_PAGEMASTER_MARGIN_BOTTOM        },
    { XML_NAMESPACE_FO,             sXML_margin_left,       XML_TOK_PAGEMASTER_MARGIN_LEFT          },
    { XML_NAMESPACE_FO,             sXML_margin_right,      XML_TOK_PAGEMASTER_MARGIN_RIGHT         },
    { XML_NAMESPACE_FO,             sXML_page_width,        XML_TOK_PAGEMASTER_PAGE_WIDTH           },
    { XML_NAMESPACE_FO,             sXML_page_height,       XML_TOK_PAGEMASTER_PAGE_HEIGHT          },
    { XML_NAMESPACE_FO,             sXML_page_orientation,  XML_TOK_PAGEMASTER_PAGE_ORIENTATION     },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aDrawPageAttrTokenMap[] =
{
    { XML_NAMESPACE_DRAW,           sXML_name,                          XML_TOK_DRAWPAGE_NAME               },
    { XML_NAMESPACE_DRAW,           sXML_style_name,                    XML_TOK_DRAWPAGE_STYLE_NAME         },
    { XML_NAMESPACE_DRAW,           sXML_master_page_name,              XML_TOK_DRAWPAGE_MASTER_PAGE_NAME   },
    { XML_NAMESPACE_PRESENTATION,   sXML_presentation_page_layout_name, XML_TOK_DRAWPAGE_PAGE_LAYOUT_NAME   },
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
        case XML_TOK_DOC_STYLES:
        {
            // office:styles inside office:document
            pContext = GetSdImport().CreateStylesContext(rLocalName, xAttrList);
            break;
        }
        case XML_TOK_DOC_AUTOSTYLES:
        {
            // office:automatic-styles inside office:document
            pContext = GetSdImport().CreateAutoStylesContext(rLocalName, xAttrList);
            break;
        }
        case XML_TOK_DOC_MASTERSTYLES:
        {
            // office:master-styles inside office:document
            pContext = GetSdImport().CreateMasterStylesContext(rLocalName, xAttrList);
            break;
        }
        case XML_TOK_DOC_META:
        {
            // office:meta inside office:document
            pContext = GetSdImport().CreateMetaContext(rLocalName, xAttrList);
            break;
        }
        case XML_TOK_DOC_BODY:
        {
            // office:body inside office:document
            pContext = GetSdImport().CreateBodyContext(rLocalName, xAttrList);
            break;
        }
    }

    // call parent when no own context was created
    if(!pContext)
        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

SdXMLImport::SdXMLImport(
    uno::Reference<frame::XModel>& rMod,
    BOOL bLDoc,
    UINT16 nStyleFamMask,
    BOOL bShowProgr,
    BOOL bIsDraw)
:   SvXMLImport(rMod),
    mpMasterStylesContext(0L),
    mpDocElemTokenMap(0L),
    mpBodyElemTokenMap(0L),
    mpStylesElemTokenMap(0L),
    mpMasterPageElemTokenMap(0L),
    mpMasterPageAttrTokenMap(0L),
    mpPageMasterAttrTokenMap(0L),
    mpDrawPageAttrTokenMap(0L),
    mpDrawPageElemTokenMap(0L),
    mpPresentationPlaceholderAttrTokenMap(0L),
    mnStyleFamilyMask(nStyleFamMask),
    mnNewPageCount(0L),
    mnNewMasterPageCount(0L),
    mbLoadDoc(bLDoc),
    mbIsDraw(bIsDraw)
{
    // prepare access to styles
    uno::Reference< style::XStyleFamiliesSupplier > xFamSup( GetModel(), uno::UNO_QUERY );
    if(xFamSup.is())
    {
        mxDocStyleFamilies = xFamSup->getStyleFamilies();
    }

    // prepare access to master pages
    uno::Reference < drawing::XMasterPagesSupplier > xMasterPagesSupplier(GetModel(), uno::UNO_QUERY);
    if(xMasterPagesSupplier.is())
    {
        mxDocMasterPages = mxDocMasterPages.query( xMasterPagesSupplier->getMasterPages() );
    }

    // prepare access to draw pages
    uno::Reference <drawing::XDrawPagesSupplier> xDrawPagesSupplier(GetModel(), uno::UNO_QUERY);
    if(xDrawPagesSupplier.is())
    {
        mxDocDrawPages = mxDocDrawPages.query( xDrawPagesSupplier->getDrawPages() );
    }

    // add namespaces
    GetNamespaceMap().AddAtIndex(
        XML_NAMESPACE_PRESENTATION, sXML_np_presentation, sXML_n_presentation, XML_NAMESPACE_PRESENTATION);

    // get status indicator (if intended)
    if(bShowProgr)
    {
        uno::Reference<frame::XController> xController(rMod->getCurrentController());
        if(xController.is())
        {
            uno::Reference<frame::XFrame> xFrame(xController->getFrame());
            if(xFrame.is())
            {
                uno::Reference<task::XStatusIndicatorSupplier> xFactory(xFrame, uno::UNO_QUERY);
                if(xFactory.is())
                {
                    mxStatusIndicator = xFactory->getStatusIndicator();
                }
            }
        }
    }

    // add progress view
    if(mxStatusIndicator.is())
    {
        const OUString aText(RTL_CONSTASCII_USTRINGPARAM("XML Import"));
        mxStatusIndicator->start(aText, 100);
    }
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

    if(XML_NAMESPACE_OFFICE == nPrefix && 0 == rLocalName.compareToAscii(sXML_document))
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


