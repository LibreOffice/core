/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <osl/thread.h>
#include <sal/log.hxx>
#include <comphelper/processfactory.hxx>

#include <xmloff/xmlscripti.hxx>
#include <facreg.hxx>
#include "sdxmlimp_impl.hxx"
#include "ximpbody.hxx"

#include <xmloff/xmlmetai.hxx>
#include "ximpstyl.hxx"
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/DocumentSettingsContext.hxx>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include "sdpropls.hxx"
#include <xmloff/xmlexppr.hxx>
#include <xmloff/xmlerror.hxx>
#include <xmloff/settingsstore.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include <com/sun/star/style/XStyle.hpp>

#include <xmloff/XMLFontStylesContext.hxx>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

class SdXMLBodyContext_Impl : public SvXMLImportContext
{
    SdXMLImport& GetSdImport() { return static_cast<SdXMLImport&>(GetImport()); }

public:

    SdXMLBodyContext_Impl( SdXMLImport& rImport, sal_uInt16 nPrfx,
                const OUString& rLName,
                const uno::Reference< xml::sax::XAttributeList > & xAttrList );

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
                const OUString& rLocalName,
                const uno::Reference< xml::sax::XAttributeList > & xAttrList ) override;
};

SdXMLBodyContext_Impl::SdXMLBodyContext_Impl( SdXMLImport& rImport,
                sal_uInt16 nPrfx, const OUString& rLName,
                const uno::Reference< xml::sax::XAttributeList > & ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
}

SvXMLImportContextRef SdXMLBodyContext_Impl::CreateChildContext(
        sal_uInt16 /*nPrefix*/,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    return GetSdImport().CreateBodyContext(rLocalName, xAttrList);
}

// NB: virtually inherit so we can multiply inherit properly
//     in SdXMLFlatDocContext_Impl
class SdXMLDocContext_Impl : public virtual SvXMLImportContext
{
protected:
    SdXMLImport& GetSdImport() { return static_cast<SdXMLImport&>(GetImport()); }

public:
    SdXMLDocContext_Impl( SdXMLImport& rImport );

    virtual SvXMLImportContextRef CreateChildContext(sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference<xml::sax::XAttributeList>& xAttrList) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL characters( const OUString& /*aChars*/ ) override {}

    virtual void SAL_CALL startFastElement( sal_Int32 /*nElement*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ ) override {}

    virtual void SAL_CALL endFastElement( sal_Int32 /*nElement*/ ) override {}
};

SdXMLDocContext_Impl::SdXMLDocContext_Impl(
    SdXMLImport& rImport )
:   SvXMLImportContext(rImport)
{
}

SvXMLImportContextRef SdXMLDocContext_Impl::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    SvXMLImportContextRef xContext;

    const SvXMLTokenMap& rTokenMap = GetSdImport().GetDocElemTokenMap();
    switch(rTokenMap.Get(nPrefix, rLocalName))
    {
        case XML_TOK_DOC_FONTDECLS:
        {
            xContext = GetSdImport().CreateFontDeclsContext( rLocalName, xAttrList );
            break;
        }
        case XML_TOK_DOC_SETTINGS:
        {
            if( GetImport().getImportFlags() & SvXMLImportFlags::SETTINGS )
            {
                xContext = new XMLDocumentSettingsContext(GetImport(), nPrefix, rLocalName, xAttrList );
            }
            break;
        }
        case XML_TOK_DOC_STYLES:
        {
            if( GetImport().getImportFlags() & SvXMLImportFlags::STYLES )
            {
                // office:styles inside office:document
                xContext = GetSdImport().CreateStylesContext(rLocalName, xAttrList);
            }
            break;
        }
        case XML_TOK_DOC_AUTOSTYLES:
        {
            if( GetImport().getImportFlags() & SvXMLImportFlags::AUTOSTYLES )
            {
                // office:automatic-styles inside office:document
                xContext = GetSdImport().CreateAutoStylesContext(rLocalName, xAttrList);
            }
            break;
        }
        case XML_TOK_DOC_MASTERSTYLES:
        {
            if( GetImport().getImportFlags() & SvXMLImportFlags::MASTERSTYLES )
            {
                // office:master-styles inside office:document
                xContext = GetSdImport().CreateMasterStylesContext(rLocalName, xAttrList);
            }
            break;
        }
        case XML_TOK_DOC_META:
        {
            SAL_INFO("xmloff.draw", "XML_TOK_DOC_META: should not have come here, maybe document is invalid?");
            break;
        }
        case XML_TOK_DOC_SCRIPT:
        {
            if( GetImport().getImportFlags() & SvXMLImportFlags::SCRIPTS )
            {
                // office:script inside office:document
                xContext = GetSdImport().CreateScriptContext( rLocalName );
            }
            break;
        }
        case XML_TOK_DOC_BODY:
        {
            if( GetImport().getImportFlags() & SvXMLImportFlags::CONTENT )
            {
                // office:body inside office:document
                xContext = new SdXMLBodyContext_Impl(GetSdImport(),nPrefix,
                                                     rLocalName, xAttrList);
            }
            break;
        }
    }

    // call parent when no own context was created
    if (!xContext)
        xContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return xContext;
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL SdXMLDocContext_Impl::createFastChildContext(
    sal_Int32 /*nElement*/, const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    return new SvXMLImportContext( GetImport() );
}

// context for flat file xml format
class SdXMLFlatDocContext_Impl
    : public SdXMLDocContext_Impl, public SvXMLMetaDocumentContext
{
public:
    SdXMLFlatDocContext_Impl( SdXMLImport& i_rImport,
        const uno::Reference<document::XDocumentProperties>& i_xDocProps );

    virtual void SAL_CALL startFastElement( sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;

    virtual void SAL_CALL characters( const OUString& aChars ) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

SdXMLFlatDocContext_Impl::SdXMLFlatDocContext_Impl( SdXMLImport& i_rImport,
        const uno::Reference<document::XDocumentProperties>& i_xDocProps) :
    SvXMLImportContext(i_rImport),
    SdXMLDocContext_Impl(i_rImport),
    SvXMLMetaDocumentContext(i_rImport, i_xDocProps)
{
}

void SAL_CALL SdXMLFlatDocContext_Impl::startFastElement( sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLMetaDocumentContext::startFastElement(nElement, xAttrList);
}

void SAL_CALL SdXMLFlatDocContext_Impl::endFastElement( sal_Int32 nElement )
{
    SvXMLMetaDocumentContext::endFastElement(nElement);
}

void SAL_CALL SdXMLFlatDocContext_Impl::characters( const OUString& rChars )
{
    SvXMLMetaDocumentContext::characters(rChars);
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL SdXMLFlatDocContext_Impl::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    // behave like meta base class iff we encounter office:meta
    if ( nElement == XML_ELEMENT( OFFICE, XML_META ) ) {
        return SvXMLMetaDocumentContext::createFastChildContext(
                    nElement, xAttrList );
    } else {
        return SdXMLDocContext_Impl::createFastChildContext(
                    nElement, xAttrList );
    }
}

#define SERVICE(classname,servicename,implementationname,draw,flags)\
uno::Sequence< OUString > classname##_getSupportedServiceNames() throw()\
{\
    return uno::Sequence< OUString > { servicename };\
}\
OUString classname##_getImplementationName() throw()\
{\
    return OUString( implementationname );\
}\
uno::Reference< uno::XInterface > classname##_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr)\
{\
    return static_cast<cppu::OWeakObject*>(new SdXMLImport( comphelper::getComponentContext(rSMgr), implementationname, draw, flags )); \
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Impress_XMLOasisImporter_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(
        new SdXMLImport(pCtx, "XMLImpressImportOasis", false, SvXMLImportFlags::ALL));
}

SERVICE( XMLDrawImportOasis, "com.sun.star.comp.Draw.XMLOasisImporter", "XMLDrawImportOasis", true, SvXMLImportFlags::ALL )

SERVICE( XMLImpressStylesImportOasis, "com.sun.star.comp.Impress.XMLOasisStylesImporter", "XMLImpressStylesImportOasis", false, SvXMLImportFlags::STYLES|SvXMLImportFlags::AUTOSTYLES|SvXMLImportFlags::MASTERSTYLES )
SERVICE( XMLDrawStylesImportOasis, "com.sun.star.comp.Draw.XMLOasisStylesImporter", "XMLImpressStylesImportOasis", true, SvXMLImportFlags::STYLES|SvXMLImportFlags::AUTOSTYLES|SvXMLImportFlags::MASTERSTYLES )

SERVICE( XMLImpressContentImportOasis, "com.sun.star.comp.Impress.XMLOasisContentImporter", "XMLImpressContentImportOasis", false, SvXMLImportFlags::AUTOSTYLES|SvXMLImportFlags::CONTENT|SvXMLImportFlags::SCRIPTS|SvXMLImportFlags::FONTDECLS )
SERVICE( XMLDrawContentImportOasis, "com.sun.star.comp.Draw.XMLOasisContentImporter", "XMLImpressContentImportOasis", true, SvXMLImportFlags::AUTOSTYLES|SvXMLImportFlags::CONTENT|SvXMLImportFlags::SCRIPTS|SvXMLImportFlags::FONTDECLS )

SERVICE( XMLImpressMetaImportOasis, "com.sun.star.comp.Impress.XMLOasisMetaImporter", "XMLImpressMetaImportOasis", false, SvXMLImportFlags::META )
SERVICE( XMLDrawMetaImportOasis, "com.sun.star.comp.Draw.XMLOasisMetaImporter", "XMLImpressMetaImportOasis", true, SvXMLImportFlags::META )

SERVICE( XMLImpressSettingsImportOasis, "com.sun.star.comp.Impress.XMLOasisSettingsImporter", "XMLImpressSettingsImportOasis", false, SvXMLImportFlags::SETTINGS )
SERVICE( XMLDrawSettingsImportOasis, "com.sun.star.comp.Draw.XMLOasisSettingsImporter", "XMLImpressSettingsImportOasis", true, SvXMLImportFlags::SETTINGS )

SdXMLImport::SdXMLImport(
    const css::uno::Reference< css::uno::XComponentContext >& xContext,
    OUString const & implementationName,
    bool bIsDraw, SvXMLImportFlags nImportFlags )
:   SvXMLImport( xContext, implementationName, nImportFlags ),
    mnNewPageCount(0),
    mnNewMasterPageCount(0),
    mbIsDraw(bIsDraw),
    mbLoadDoc(true),
    mbPreview(false)
{
    // add namespaces
    GetNamespaceMap().Add(
        GetXMLToken(XML_NP_PRESENTATION),
        GetXMLToken(XML_N_PRESENTATION),
        XML_NAMESPACE_PRESENTATION);

    GetNamespaceMap().Add(
        GetXMLToken(XML_NP_SMIL),
        GetXMLToken(XML_N_SMIL_COMPAT),
        XML_NAMESPACE_SMIL);

    GetNamespaceMap().Add(
        GetXMLToken(XML_NP_ANIMATION),
        GetXMLToken(XML_N_ANIMATION),
        XML_NAMESPACE_ANIMATION);
}

// XImporter
void SAL_CALL SdXMLImport::setTargetDocument( const uno::Reference< lang::XComponent >& xDoc )
{
    SvXMLImport::setTargetDocument( xDoc );

    uno::Reference< lang::XServiceInfo > xDocServices( GetModel(), uno::UNO_QUERY );
    if( !xDocServices.is() )
        throw lang::IllegalArgumentException();

    mbIsDraw = !xDocServices->supportsService("com.sun.star.presentation.PresentationDocument");

    // prepare access to styles
    uno::Reference< style::XStyleFamiliesSupplier > xFamSup( GetModel(), uno::UNO_QUERY );
    if(xFamSup.is())
        mxDocStyleFamilies = xFamSup->getStyleFamilies();

    // prepare access to master pages
    uno::Reference < drawing::XMasterPagesSupplier > xMasterPagesSupplier(GetModel(), uno::UNO_QUERY);
    if(xMasterPagesSupplier.is())
        mxDocMasterPages.set(xMasterPagesSupplier->getMasterPages(), css::uno::UNO_QUERY);

    // prepare access to draw pages
    uno::Reference <drawing::XDrawPagesSupplier> xDrawPagesSupplier(GetModel(), uno::UNO_QUERY);
    if(!xDrawPagesSupplier.is())
        throw lang::IllegalArgumentException();

    mxDocDrawPages.set(xDrawPagesSupplier->getDrawPages(), css::uno::UNO_QUERY);
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

    uno::Reference< lang::XMultiServiceFactory > xFac( GetModel(), uno::UNO_QUERY );
    if( xFac.is() )
    {
        uno::Sequence< OUString > sSNS( xFac->getAvailableServiceNames() );
        sal_Int32 n = sSNS.getLength();
        const OUString* pSNS( sSNS.getConstArray() );
        while( --n > 0 )
        {
            if( (*pSNS++) == "com.sun.star.drawing.TableShape" )
            {
                mbIsTableShapeSupported = true;
                break;
            }
        }
    }
}

// XInitialization
void SAL_CALL SdXMLImport::initialize( const uno::Sequence< uno::Any >& aArguments )
{
    SvXMLImport::initialize( aArguments );

    uno::Reference< beans::XPropertySet > xInfoSet( getImportInfo() );
    if( xInfoSet.is() )
    {
        uno::Reference< beans::XPropertySetInfo > xInfoSetInfo( xInfoSet->getPropertySetInfo() );

        if( xInfoSetInfo->hasPropertyByName( gsPageLayouts ) )
            xInfoSet->getPropertyValue( gsPageLayouts ) >>= mxPageLayouts;

        if( xInfoSetInfo->hasPropertyByName( gsPreview ) )
            xInfoSet->getPropertyValue( gsPreview ) >>= mbPreview;

        OUString const sOrganizerMode(
            "OrganizerMode");
        if (xInfoSetInfo->hasPropertyByName(sOrganizerMode))
        {
            bool bStyleOnly(false);
            if (xInfoSet->getPropertyValue(sOrganizerMode) >>= bStyleOnly)
            {
                mbLoadDoc = !bStyleOnly;
            }
        }
    }
}

const SvXMLTokenMap& SdXMLImport::GetDocElemTokenMap()
{
    if(!mpDocElemTokenMap)
    {
        static const SvXMLTokenMapEntry aDocElemTokenMap[] =
{
    { XML_NAMESPACE_OFFICE, XML_FONT_FACE_DECLS,    XML_TOK_DOC_FONTDECLS       },
    { XML_NAMESPACE_OFFICE, XML_STYLES,             XML_TOK_DOC_STYLES          },
    { XML_NAMESPACE_OFFICE, XML_AUTOMATIC_STYLES,   XML_TOK_DOC_AUTOSTYLES      },
    { XML_NAMESPACE_OFFICE, XML_MASTER_STYLES,      XML_TOK_DOC_MASTERSTYLES    },
    { XML_NAMESPACE_OFFICE, XML_META,               XML_TOK_DOC_META            },
    { XML_NAMESPACE_OFFICE, XML_SCRIPTS,            XML_TOK_DOC_SCRIPT          },
    { XML_NAMESPACE_OFFICE, XML_BODY,               XML_TOK_DOC_BODY            },
    { XML_NAMESPACE_OFFICE, XML_SETTINGS,           XML_TOK_DOC_SETTINGS        },
    XML_TOKEN_MAP_END
};

        mpDocElemTokenMap = std::make_unique<SvXMLTokenMap>(aDocElemTokenMap);
    }

    return *mpDocElemTokenMap;
}

const SvXMLTokenMap& SdXMLImport::GetBodyElemTokenMap()
{
    if(!mpBodyElemTokenMap)
    {
        static const SvXMLTokenMapEntry aBodyElemTokenMap[] =
        {
            { XML_NAMESPACE_DRAW,   XML_PAGE,               XML_TOK_BODY_PAGE   },
            { XML_NAMESPACE_PRESENTATION, XML_SETTINGS,     XML_TOK_BODY_SETTINGS   },
            { XML_NAMESPACE_PRESENTATION, XML_HEADER_DECL,  XML_TOK_BODY_HEADER_DECL    },
            { XML_NAMESPACE_PRESENTATION, XML_FOOTER_DECL,  XML_TOK_BODY_FOOTER_DECL    },
            { XML_NAMESPACE_PRESENTATION, XML_DATE_TIME_DECL,XML_TOK_BODY_DATE_TIME_DECL    },

            XML_TOKEN_MAP_END
        };

        mpBodyElemTokenMap = std::make_unique<SvXMLTokenMap>(aBodyElemTokenMap);
    }

    return *mpBodyElemTokenMap;
}

const SvXMLTokenMap& SdXMLImport::GetStylesElemTokenMap()
{
    if(!mpStylesElemTokenMap)
    {
        static const SvXMLTokenMapEntry aStylesElemTokenMap[] =
        {
            { XML_NAMESPACE_STYLE,  XML_PAGE_LAYOUT,                XML_TOK_STYLES_PAGE_MASTER              },
            { XML_NAMESPACE_STYLE,  XML_PRESENTATION_PAGE_LAYOUT,   XML_TOK_STYLES_PRESENTATION_PAGE_LAYOUT },
            { XML_NAMESPACE_STYLE,  XML_STYLE,                      XML_TOK_STYLES_STYLE    },
            XML_TOKEN_MAP_END
        };

        mpStylesElemTokenMap = std::make_unique<SvXMLTokenMap>(aStylesElemTokenMap);
    }

    return *mpStylesElemTokenMap;
}

const SvXMLTokenMap& SdXMLImport::GetMasterPageElemTokenMap()
{
    if(!mpMasterPageElemTokenMap)
    {
        static const SvXMLTokenMapEntry aMasterPageElemTokenMap[] =
        {
            { XML_NAMESPACE_STYLE,          XML_STYLE,      XML_TOK_MASTERPAGE_STYLE    },
            { XML_NAMESPACE_PRESENTATION,   XML_NOTES,      XML_TOK_MASTERPAGE_NOTES    },
            XML_TOKEN_MAP_END
        };

        mpMasterPageElemTokenMap = std::make_unique<SvXMLTokenMap>(aMasterPageElemTokenMap);
    }

    return *mpMasterPageElemTokenMap;
}

const SvXMLTokenMap& SdXMLImport::GetMasterPageAttrTokenMap()
{
    if(!mpMasterPageAttrTokenMap)
    {
        static const SvXMLTokenMapEntry aMasterPageAttrTokenMap[] =
        {
            { XML_NAMESPACE_STYLE,  XML_NAME,                       XML_TOK_MASTERPAGE_NAME },
            { XML_NAMESPACE_STYLE,  XML_DISPLAY_NAME,               XML_TOK_MASTERPAGE_DISPLAY_NAME },
            { XML_NAMESPACE_STYLE,  XML_PAGE_LAYOUT_NAME,           XML_TOK_MASTERPAGE_PAGE_MASTER_NAME },
            { XML_NAMESPACE_DRAW,   XML_STYLE_NAME,                 XML_TOK_MASTERPAGE_STYLE_NAME       },
            { XML_NAMESPACE_PRESENTATION,   XML_PRESENTATION_PAGE_LAYOUT_NAME,  XML_TOK_MASTERPAGE_PAGE_LAYOUT_NAME },
            { XML_NAMESPACE_PRESENTATION,   XML_USE_HEADER_NAME,                XML_TOK_MASTERPAGE_USE_HEADER_NAME  },
            { XML_NAMESPACE_PRESENTATION,   XML_USE_FOOTER_NAME,                XML_TOK_MASTERPAGE_USE_FOOTER_NAME  },
            { XML_NAMESPACE_PRESENTATION,   XML_USE_DATE_TIME_NAME,             XML_TOK_MASTERPAGE_USE_DATE_TIME_NAME   },
            XML_TOKEN_MAP_END
        };

        mpMasterPageAttrTokenMap = std::make_unique<SvXMLTokenMap>(aMasterPageAttrTokenMap);
    }

    return *mpMasterPageAttrTokenMap;
}

const SvXMLTokenMap& SdXMLImport::GetPageMasterAttrTokenMap()
{
    if(!mpPageMasterAttrTokenMap)
    {
        static const SvXMLTokenMapEntry aPageMasterAttrTokenMap[] =
        {
            { XML_NAMESPACE_STYLE,          XML_NAME,               XML_TOK_PAGEMASTER_NAME                 },
            XML_TOKEN_MAP_END
        };

        mpPageMasterAttrTokenMap = std::make_unique<SvXMLTokenMap>(aPageMasterAttrTokenMap);
    }

    return *mpPageMasterAttrTokenMap;
}

const SvXMLTokenMap& SdXMLImport::GetPageMasterStyleAttrTokenMap()
{
    if(!mpPageMasterStyleAttrTokenMap)
    {
        static const SvXMLTokenMapEntry aPageMasterStyleAttrTokenMap[] =
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

        mpPageMasterStyleAttrTokenMap = std::make_unique<SvXMLTokenMap>(aPageMasterStyleAttrTokenMap);
    }

    return *mpPageMasterStyleAttrTokenMap;
}

const SvXMLTokenMap& SdXMLImport::GetDrawPageAttrTokenMap()
{
    if(!mpDrawPageAttrTokenMap)
    {
        static const SvXMLTokenMapEntry aDrawPageAttrTokenMap[] =
        {
            { XML_NAMESPACE_DRAW,           XML_NAME,                           XML_TOK_DRAWPAGE_NAME               },
            { XML_NAMESPACE_DRAW,           XML_STYLE_NAME,                     XML_TOK_DRAWPAGE_STYLE_NAME         },
            { XML_NAMESPACE_DRAW,           XML_MASTER_PAGE_NAME,               XML_TOK_DRAWPAGE_MASTER_PAGE_NAME   },
            { XML_NAMESPACE_PRESENTATION,   XML_PRESENTATION_PAGE_LAYOUT_NAME,  XML_TOK_DRAWPAGE_PAGE_LAYOUT_NAME   },
            { XML_NAMESPACE_DRAW,           XML_ID,                             XML_TOK_DRAWPAGE_DRAWID                 },
            { XML_NAMESPACE_XML,            XML_ID,                             XML_TOK_DRAWPAGE_XMLID                  },
            { XML_NAMESPACE_XLINK,          XML_HREF,                           XML_TOK_DRAWPAGE_HREF               },
            { XML_NAMESPACE_PRESENTATION,   XML_USE_HEADER_NAME,                XML_TOK_DRAWPAGE_USE_HEADER_NAME    },
            { XML_NAMESPACE_PRESENTATION,   XML_USE_FOOTER_NAME,                XML_TOK_DRAWPAGE_USE_FOOTER_NAME    },
            { XML_NAMESPACE_PRESENTATION,   XML_USE_DATE_TIME_NAME,             XML_TOK_DRAWPAGE_USE_DATE_TIME_NAME },

            XML_TOKEN_MAP_END
        };

        mpDrawPageAttrTokenMap = std::make_unique<SvXMLTokenMap>(aDrawPageAttrTokenMap);
    }

    return *mpDrawPageAttrTokenMap;
}

const SvXMLTokenMap& SdXMLImport::GetDrawPageElemTokenMap()
{
    if(!mpDrawPageElemTokenMap)
    {
        static const SvXMLTokenMapEntry aDrawPageElemTokenMap[] =
        {
            { XML_NAMESPACE_PRESENTATION,   XML_NOTES,              XML_TOK_DRAWPAGE_NOTES      },
            { XML_NAMESPACE_ANIMATION,      XML_PAR,                XML_TOK_DRAWPAGE_PAR        },
            { XML_NAMESPACE_ANIMATION,      XML_SEQ,                XML_TOK_DRAWPAGE_SEQ        },
            { XML_NAMESPACE_DRAW,           XML_LAYER_SET,          XML_TOK_DRAWPAGE_LAYER_SET  },
            XML_TOKEN_MAP_END
        };

        mpDrawPageElemTokenMap = std::make_unique<SvXMLTokenMap>(aDrawPageElemTokenMap);
    }

    return *mpDrawPageElemTokenMap;
}

const SvXMLTokenMap& SdXMLImport::GetPresentationPlaceholderAttrTokenMap()
{
    if(!mpPresentationPlaceholderAttrTokenMap)
    {
        static const SvXMLTokenMapEntry aPresentationPlaceholderAttrTokenMap[] =
        {
            { XML_NAMESPACE_PRESENTATION,   XML_OBJECT,     XML_TOK_PRESENTATIONPLACEHOLDER_OBJECTNAME  },
            { XML_NAMESPACE_SVG,            XML_X,          XML_TOK_PRESENTATIONPLACEHOLDER_X           },
            { XML_NAMESPACE_SVG,            XML_Y,          XML_TOK_PRESENTATIONPLACEHOLDER_Y           },
            { XML_NAMESPACE_SVG,            XML_WIDTH,      XML_TOK_PRESENTATIONPLACEHOLDER_WIDTH       },
            { XML_NAMESPACE_SVG,            XML_HEIGHT,     XML_TOK_PRESENTATIONPLACEHOLDER_HEIGHT      },
            XML_TOKEN_MAP_END
        };

        mpPresentationPlaceholderAttrTokenMap = std::make_unique<SvXMLTokenMap>(aPresentationPlaceholderAttrTokenMap);
    }

    return *mpPresentationPlaceholderAttrTokenMap;
}

SvXMLImportContext *SdXMLImport::CreateFastContext( sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = nullptr;

    switch (nElement)
    {
        case XML_ELEMENT( OFFICE, XML_DOCUMENT_STYLES ):
        case XML_ELEMENT( OFFICE, XML_DOCUMENT_CONTENT ):
        case XML_ELEMENT( OFFICE, XML_DOCUMENT_SETTINGS ):
            pContext = new SdXMLDocContext_Impl(*this);
        break;
        case XML_ELEMENT( OFFICE, XML_DOCUMENT_META ):
            pContext = CreateMetaContext(nElement, xAttrList);
        break;
        case XML_ELEMENT( OFFICE, XML_DOCUMENT ):
        {
            uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                GetModel(), uno::UNO_QUERY_THROW);
            // flat OpenDocument file format
            pContext = new SdXMLFlatDocContext_Impl( *this, xDPS->getDocumentProperties());
        }
        break;
        default:
            pContext = SvXMLImport::CreateFastContext(nElement, xAttrList);
    }
    return pContext;
}

SvXMLImportContext *SdXMLImport::CreateMetaContext(const sal_Int32 /*nElement*/,
    const uno::Reference<xml::sax::XFastAttributeList>&)
{
    SvXMLImportContext* pContext = nullptr;

    if (getImportFlags() & SvXMLImportFlags::META)
    {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> const xDocProps(
            !mbLoadDoc ? nullptr : xDPS->getDocumentProperties());
        pContext = new SvXMLMetaDocumentContext(*this, xDocProps);
    }

    if(!pContext)
    {
        pContext = new SvXMLImportContext(*this);
    }

    return pContext;
}

SvXMLImportContext *SdXMLImport::CreateBodyContext(const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>&)
{
    SvXMLImportContext* pContext = new SdXMLBodyContext(*this, rLocalName);
    return pContext;
}

SvXMLStylesContext *SdXMLImport::CreateStylesContext(const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    if(GetShapeImport()->GetStylesContext())
        return GetShapeImport()->GetStylesContext();

    GetShapeImport()->SetStylesContext(new SdXMLStylesContext(
        *this, rLocalName, xAttrList, false));

    return GetShapeImport()->GetStylesContext();
}

SvXMLStylesContext *SdXMLImport::CreateAutoStylesContext(const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    if(GetShapeImport()->GetAutoStylesContext())
        return GetShapeImport()->GetAutoStylesContext();

    GetShapeImport()->SetAutoStylesContext(new SdXMLStylesContext(
        *this, rLocalName, xAttrList, true));

    return GetShapeImport()->GetAutoStylesContext();
}

SvXMLImportContext* SdXMLImport::CreateMasterStylesContext(const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>&)
{
    if (!mxMasterStylesContext.is())
        mxMasterStylesContext.set(new SdXMLMasterStylesContext(*this, rLocalName));
    return mxMasterStylesContext.get();
}

SvXMLImportContext *SdXMLImport::CreateFontDeclsContext(const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    XMLFontStylesContext *pFSContext =
            new XMLFontStylesContext( *this, XML_NAMESPACE_OFFICE,
                                      rLocalName, xAttrList,
                                      osl_getThreadTextEncoding() );
    SetFontDecls( pFSContext );
    return pFSContext;
}

SvXMLImportContext *SdXMLImport::CreateScriptContext(
                                       const OUString& rLocalName )
{
    SvXMLImportContext *pContext = new XMLScriptContext( *this, rLocalName, GetModel() );
    return pContext;
}

void SdXMLImport::SetViewSettings(const css::uno::Sequence<css::beans::PropertyValue>& aViewProps)
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

        if ( rName == "VisibleAreaTop" )
        {
            rValue >>= aVisArea.Y;
        }
        else if ( rName == "VisibleAreaLeft" )
        {
            rValue >>= aVisArea.X;
        }
        else if ( rName == "VisibleAreaWidth" )
        {
            rValue >>= aVisArea.Width;
        }
        else if ( rName == "VisibleAreaHeight" )
        {
            rValue >>= aVisArea.Height;
        }

        pValues++;
    }

    try
    {
        xPropSet->setPropertyValue("VisibleArea", uno::makeAny( aVisArea )  );
    }
    catch(const css::uno::Exception&)
    {
/* #i79978# since old documents may contain invalid view settings, this is nothing to worry the user about.
        uno::Sequence<OUString> aSeq(0);
        SetError( XMLERROR_FLAG_WARNING | XMLERROR_API, aSeq, e.Message, NULL );
*/
    }
}

void SdXMLImport::SetConfigurationSettings(const css::uno::Sequence<css::beans::PropertyValue>& aConfigProps)
{
    uno::Reference< lang::XMultiServiceFactory > xFac( GetModel(), uno::UNO_QUERY );
    if( !xFac.is() )
        return;

    uno::Reference< beans::XPropertySet > xProps( xFac->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY );
    if( !xProps.is() )
        return;

    uno::Reference< beans::XPropertySetInfo > xInfo( xProps->getPropertySetInfo() );
    if( !xInfo.is() )
        return;

    sal_Int32 nCount = aConfigProps.getLength();
    const beans::PropertyValue* pValues = aConfigProps.getConstArray();

    DocumentSettingsSerializer *pFilter;
    pFilter = dynamic_cast<DocumentSettingsSerializer *>(xProps.get());
    uno::Sequence<beans::PropertyValue> aFiltered;
    if( pFilter )
    {
        aFiltered = pFilter->filterStreamsFromStorage( GetDocumentBase(), GetSourceStorage(), aConfigProps );
        nCount = aFiltered.getLength();
        pValues = aFiltered.getConstArray();
    }

    while( nCount-- )
    {
        try
        {
            const OUString& rProperty = pValues->Name;
            if( xInfo->hasPropertyByName( rProperty ) )
                xProps->setPropertyValue( rProperty, pValues->Value );
        }
        catch(const uno::Exception&)
        {
            SAL_INFO("xmloff.draw",  "#SdXMLImport::SetConfigurationSettings: Exception!" );
        }

        pValues++;
    }
}

// #80365# override this method to read and use the hint value from the
// written meta information. If no info is found, guess 10 draw objects
//void SdXMLImport::SetStatisticAttributes(const uno::Reference<xml::sax::XAttributeList>& xAttrList)
void SdXMLImport::SetStatistics(
        const uno::Sequence<beans::NamedValue> & i_rStats)
{
    static const char* s_stats[] =
        { "ObjectCount", nullptr };

    SvXMLImport::SetStatistics(i_rStats);

    sal_uInt32 nCount(10);
    for (sal_Int32 i = 0; i < i_rStats.getLength(); ++i) {
        for (const char** pStat = s_stats; *pStat != nullptr; ++pStat) {
            if (i_rStats[i].Name.equalsAscii(*pStat)) {
                sal_Int32 val = 0;
                if (i_rStats[i].Value >>= val) {
                    nCount = val;
                } else {
                    SAL_WARN("xmloff.draw", "SdXMLImport::SetStatistics: invalid entry");
                }
            }
        }
    }

    if(nCount)
    {
        GetProgressBarHelper()->SetReference(nCount);
        GetProgressBarHelper()->SetValue(0);
    }
}

void SdXMLImport::AddHeaderDecl( const OUString& rName, const OUString& rText )
{
    if( !rName.isEmpty() && !rText.isEmpty() )
        maHeaderDeclsMap[rName] = rText;
}

void SdXMLImport::AddFooterDecl( const OUString& rName, const OUString& rText )
{
    if( !rName.isEmpty() && !rText.isEmpty() )
        maFooterDeclsMap[rName] = rText;
}

void SdXMLImport::AddDateTimeDecl( const OUString& rName, const OUString& rText, bool bFixed, const OUString& rDateTimeFormat )
{
    if( !rName.isEmpty() && (!rText.isEmpty() || !bFixed) )
    {
        DateTimeDeclContextImpl aDecl;
        aDecl.maStrText = rText;
        aDecl.mbFixed = bFixed;
        aDecl.maStrDateTimeFormat = rDateTimeFormat;
        maDateTimeDeclsMap[rName] = aDecl;
    }
}

OUString SdXMLImport::GetHeaderDecl( const OUString& rName ) const
{
    OUString aRet;
    HeaderFooterDeclMap::const_iterator aIter( maHeaderDeclsMap.find( rName ) );
    if( aIter != maHeaderDeclsMap.end() )
        aRet = (*aIter).second;

    return aRet;
}

OUString SdXMLImport::GetFooterDecl( const OUString& rName ) const
{
    OUString aRet;
    HeaderFooterDeclMap::const_iterator aIter( maFooterDeclsMap.find( rName ) );
    if( aIter != maFooterDeclsMap.end() )
        aRet = (*aIter).second;

    return aRet;
}

OUString SdXMLImport::GetDateTimeDecl( const OUString& rName, bool& rbFixed, OUString& rDateTimeFormat )
{
    DateTimeDeclContextImpl aDecl;

    DateTimeDeclMap::const_iterator aIter( maDateTimeDeclsMap.find( rName ) );
    if( aIter != maDateTimeDeclsMap.end() )
        aDecl = (*aIter).second;

    rbFixed = aDecl.mbFixed;
    rDateTimeFormat = aDecl.maStrDateTimeFormat;
    return aDecl.maStrText;
}

void SdXMLImport::NotifyEmbeddedFontRead()
{
    uno::Reference< lang::XMultiServiceFactory > xFac( GetModel(), uno::UNO_QUERY );
    if( xFac.is() )
    {
        uno::Reference< beans::XPropertySet > xProps( xFac->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY );
        if( xProps.is() )
            xProps->setPropertyValue("EmbedFonts", uno::makeAny( true ) );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
