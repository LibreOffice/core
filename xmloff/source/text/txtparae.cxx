/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <tools/debug.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/types.h>
#include <vector>
#include <list>
#include <boost/unordered_map.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XNumberingRulesSupplier.hpp>
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/document/XRedlinesSupplier.hpp>
#include <com/sun/star/text/XBookmarksSupplier.hpp>
#include <com/sun/star/text/XFormField.hpp>
#include <com/sun/star/text/XTextSection.hpp>
#include <com/sun/star/text/SectionFileLink.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/text/XTextShapesSupplier.hpp>
#include <com/sun/star/style/XAutoStylesSupplier.hpp>
#include <com/sun/star/style/XAutoStyleFamily.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/util/DateTime.hpp>

#include <sax/tools/converter.hxx>

#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlaustp.hxx>
#include <xmloff/families.hxx>
#include "txtexppr.hxx"
#include <xmloff/xmlnumfe.hxx>
#include <xmloff/xmlnume.hxx>
#include <xmloff/xmluconv.hxx>
#include "XMLAnchorTypePropHdl.hxx"
#include "xexptran.hxx"
#include <xmloff/ProgressBarHelper.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlexp.hxx>
#include "txtflde.hxx"
#include <xmloff/txtprmap.hxx>
#include "XMLImageMapExport.hxx"
#include "XMLTextNumRuleInfo.hxx"
#include <xmloff/XMLTextListAutoStylePool.hxx>
#include <xmloff/txtparae.hxx>
#include "XMLSectionExport.hxx"
#include "XMLIndexMarkExport.hxx"
#include <xmloff/XMLEventExport.hxx>
#include "XMLRedlineExport.hxx"
#include "MultiPropertySetHelper.hxx"
#include <xmloff/formlayerexport.hxx>
#include "XMLTextCharStyleNamesElementExport.hxx"
#include <xmloff/odffields.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <txtlists.hxx>
#include <com/sun/star/rdf/XMetadatable.hpp>

using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::frame;
using namespace ::xmloff;
using namespace ::xmloff::token;

namespace
{
    class TextContentSet
    {
        public:
            typedef Reference<XTextContent> text_content_ref_t;
            typedef list<text_content_ref_t> contents_t;
            typedef back_insert_iterator<contents_t> inserter_t;
            typedef contents_t::const_iterator const_iterator_t;

            inserter_t getInserter()
                { return back_insert_iterator<contents_t>(m_vTextContents); };
            const_iterator_t getBegin() const
                { return m_vTextContents.begin(); };
            const_iterator_t getEnd() const
                { return m_vTextContents.end(); };

        private:
            contents_t m_vTextContents;
    };

    struct FrameRefHash
        : public unary_function<Reference<XTextFrame>, size_t>
    {
        size_t operator()(const Reference<XTextFrame> xFrame) const
            { return sal::static_int_cast<size_t>(reinterpret_cast<sal_uIntPtr>(xFrame.get())); }
    };

    static bool lcl_TextContentsUnfiltered(const Reference<XTextContent>&)
        { return true; };

    static bool lcl_ShapeFilter(const Reference<XTextContent>& xTxtContent)
    {
        static const OUString sTextFrameService("com.sun.star.text.TextFrame");
        static const OUString sTextGraphicService("com.sun.star.text.TextGraphicObject");
        static const OUString sTextEmbeddedService("com.sun.star.text.TextEmbeddedObject");
        Reference<XShape> xShape(xTxtContent, UNO_QUERY);
        if(!xShape.is())
            return false;
        Reference<XServiceInfo> xServiceInfo(xTxtContent, UNO_QUERY);
        if(xServiceInfo->supportsService(sTextFrameService) ||
            xServiceInfo->supportsService(sTextGraphicService) ||
            xServiceInfo->supportsService(sTextEmbeddedService) )
            return false;
        return true;
    };

    class BoundFrames
    {
        public:
            typedef bool (*filter_t)(const Reference<XTextContent>&);
            BoundFrames(
                const Reference<XEnumerationAccess> xEnumAccess,
                const filter_t& rFilter)
                : m_xEnumAccess(xEnumAccess)
            {
                Fill(rFilter);
            };
            BoundFrames()
                {};
            const TextContentSet* GetPageBoundContents() const
                { return &m_vPageBounds; };
            const TextContentSet* GetFrameBoundContents(const Reference<XTextFrame>& rParentFrame) const
            {
                framebound_map_t::const_iterator it = m_vFrameBoundsOf.find(rParentFrame);
                if(it == m_vFrameBoundsOf.end())
                    return NULL;
                return &(it->second);
            };
            Reference<XEnumeration> createEnumeration() const
            {
                if(!m_xEnumAccess.is())
                    return Reference<XEnumeration>();
                return m_xEnumAccess->createEnumeration();
            };

        private:
            typedef boost::unordered_map<
                Reference<XTextFrame>,
                TextContentSet,
                FrameRefHash> framebound_map_t;
            TextContentSet m_vPageBounds;
            framebound_map_t m_vFrameBoundsOf;
            const Reference<XEnumerationAccess> m_xEnumAccess;
            void Fill(const filter_t& rFilter);
    };

    class FieldParamExporter
    {
        public:
            FieldParamExporter(SvXMLExport* const pExport, Reference<XNameContainer> xFieldParams)
                : m_pExport(pExport)
                , m_xFieldParams(xFieldParams)
                { };
            void Export();

        private:
            SvXMLExport* const m_pExport;
            const Reference<XNameContainer> m_xFieldParams;

            void ExportParameter(const OUString& sKey, const OUString& sValue);
    };
}

namespace xmloff
{
    class BoundFrameSets
    {
        public:
            BoundFrameSets(const Reference<XInterface> xModel);
            const BoundFrames* GetTexts() const
                { return m_pTexts.get(); };
            const BoundFrames* GetGraphics() const
                { return m_pGraphics.get(); };
            const BoundFrames* GetEmbeddeds() const
                { return m_pEmbeddeds.get(); };
            const BoundFrames* GetShapes() const
                { return m_pShapes.get(); };
        private:
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            auto_ptr<BoundFrames> m_pTexts;
            auto_ptr<BoundFrames> m_pGraphics;
            auto_ptr<BoundFrames> m_pEmbeddeds;
            auto_ptr<BoundFrames> m_pShapes;
            SAL_WNODEPRECATED_DECLARATIONS_POP
    };
}

#ifdef DBG_UTIL
static int txtparae_bContainsIllegalCharacters = sal_False;
#endif


//



















static const sal_Char* aParagraphPropertyNamesAuto[] =
{
    "NumberingRules",
    "ParaConditionalStyleName",
    "ParaStyleName",
    NULL
};

enum eParagraphPropertyNamesEnumAuto
{
    NUMBERING_RULES_AUTO = 0,
    PARA_CONDITIONAL_STYLE_NAME_AUTO = 1,
    PARA_STYLE_NAME_AUTO = 2
};

static const sal_Char* aParagraphPropertyNames[] =
{
    "NumberingIsNumber",
    "NumberingStyleName",
    "OutlineLevel",
    "ParaConditionalStyleName",
    "ParaStyleName",
    "TextSection",
    NULL
};

enum eParagraphPropertyNamesEnum
{
    NUMBERING_IS_NUMBER = 0,
    PARA_NUMBERING_STYLENAME = 1,
    PARA_OUTLINE_LEVEL=2,
    PARA_CONDITIONAL_STYLE_NAME = 3,
    PARA_STYLE_NAME = 4,
    TEXT_SECTION = 5
};

void BoundFrames::Fill(const filter_t& rFilter)
{
    if(!m_xEnumAccess.is())
        return;
    const Reference< XEnumeration > xEnum = m_xEnumAccess->createEnumeration();
    if(!xEnum.is())
        return;
    const OUString our_sAnchorType("AnchorType");
    const OUString our_sAnchorFrame("AnchorFrame");
    while(xEnum->hasMoreElements())
    {
        Reference<XPropertySet> xPropSet(xEnum->nextElement(), UNO_QUERY);
        Reference<XTextContent> xTextContent(xPropSet, UNO_QUERY);
        if(!xPropSet.is() || !xTextContent.is())
            continue;
        TextContentAnchorType eAnchor;
        xPropSet->getPropertyValue(our_sAnchorType) >>= eAnchor;
        if(TextContentAnchorType_AT_PAGE != eAnchor && TextContentAnchorType_AT_FRAME != eAnchor)
            continue;
        if(!rFilter(xTextContent))
            continue;

        TextContentSet::inserter_t pInserter = m_vPageBounds.getInserter();
        if(TextContentAnchorType_AT_FRAME == eAnchor)
        {
            Reference<XTextFrame> xAnchorTxtFrame(
                xPropSet->getPropertyValue(our_sAnchorFrame),
                uno::UNO_QUERY);
            pInserter = m_vFrameBoundsOf[xAnchorTxtFrame].getInserter();
        }
        *pInserter++ = xTextContent;
    }
}

BoundFrameSets::BoundFrameSets(const Reference<XInterface> xModel)
    : m_pTexts(new BoundFrames())
    , m_pGraphics(new BoundFrames())
    , m_pEmbeddeds(new BoundFrames())
    , m_pShapes(new BoundFrames())
{
    const Reference<XTextFramesSupplier> xTFS(xModel, UNO_QUERY);
    const Reference<XTextGraphicObjectsSupplier> xGOS(xModel, UNO_QUERY);
    const Reference<XTextEmbeddedObjectsSupplier> xEOS(xModel, UNO_QUERY);
    const Reference<XDrawPageSupplier> xDPS(xModel, UNO_QUERY);
    if(xTFS.is())
        m_pTexts.reset(new BoundFrames(
            Reference<XEnumerationAccess>(xTFS->getTextFrames(), UNO_QUERY),
            &lcl_TextContentsUnfiltered));
    if(xGOS.is())
        m_pGraphics.reset(new BoundFrames(
            Reference<XEnumerationAccess>(xGOS->getGraphicObjects(), UNO_QUERY),
            &lcl_TextContentsUnfiltered));
    if(xEOS.is())
        m_pEmbeddeds.reset(new BoundFrames(
            Reference<XEnumerationAccess>(xEOS->getEmbeddedObjects(), UNO_QUERY),
            &lcl_TextContentsUnfiltered));
    if(xDPS.is())
        m_pShapes.reset(new BoundFrames(
            Reference<XEnumerationAccess>(xDPS->getDrawPage(), UNO_QUERY),
            &lcl_ShapeFilter));
};

void FieldParamExporter::Export()
{
    const Type aStringType = ::getCppuType((OUString*)0);
    const Type aBoolType = ::getCppuType((sal_Bool*)0);
    const Type aSeqType = ::getCppuType((Sequence<OUString>*)0);
    const Type aIntType = ::getCppuType((sal_Int32*)0);
    Sequence<OUString> vParameters(m_xFieldParams->getElementNames());
    for(const OUString* pCurrent = vParameters.begin(); pCurrent != vParameters.end(); ++pCurrent)
    {
        const Any aValue = m_xFieldParams->getByName(*pCurrent);
        const Type aValueType = aValue.getValueType();
        if(aValueType == aStringType)
        {
            OUString sValue;
            aValue >>= sValue;
            ExportParameter(*pCurrent,sValue);

            if ( pCurrent->equalsAscii( ODF_OLE_PARAM ) )
            {
                
                Reference< embed::XStorage > xTargetStg = m_pExport->GetTargetStorage();
                Reference< embed::XStorage > xDstStg = xTargetStg->openStorageElement(
                        OUString("OLELinks"), embed::ElementModes::WRITE );

                if ( !xDstStg->hasByName( sValue ) ) {
                    Reference< XStorageBasedDocument > xStgDoc (
                            m_pExport->GetModel( ), UNO_QUERY );
                    Reference< embed::XStorage > xDocStg = xStgDoc->getDocumentStorage();
                    Reference< embed::XStorage > xOleStg = xDocStg->openStorageElement(
                            OUString("OLELinks"), embed::ElementModes::READ );

                    xOleStg->copyElementTo( sValue, xDstStg, sValue );
                    Reference< embed::XTransactedObject > xTransact( xDstStg, UNO_QUERY );
                    if ( xTransact.is( ) )
                        xTransact->commit( );
                }
            }
        }
        else if(aValueType == aBoolType)
        {
            sal_Bool bValue = false;
            aValue >>= bValue;
            ExportParameter(*pCurrent, (bValue ? OUString("true" ) : OUString("false")) );
        }
        else if(aValueType == aSeqType)
        {
            Sequence<OUString> vValue;
            aValue >>= vValue;
            for(OUString* pSeqCurrent = vValue.begin(); pSeqCurrent != vValue.end(); ++pSeqCurrent)
            {
                ExportParameter(*pCurrent, *pSeqCurrent);
            }
        }
        else if(aValueType == aIntType)
        {
            sal_Int32 nValue = 0;
            aValue >>= nValue;
            ExportParameter(*pCurrent, OUStringBuffer().append(nValue).makeStringAndClear());
        }
    }
}

void FieldParamExporter::ExportParameter(const OUString& sKey, const OUString& sValue)
{
    m_pExport->AddAttribute(XML_NAMESPACE_FIELD, XML_NAME, sKey);
    m_pExport->AddAttribute(XML_NAMESPACE_FIELD, XML_VALUE, sValue);
    m_pExport->StartElement(XML_NAMESPACE_FIELD, XML_PARAM, sal_False);
    m_pExport->EndElement(XML_NAMESPACE_FIELD, XML_PARAM, sal_False);
}

void XMLTextParagraphExport::Add( sal_uInt16 nFamily,
                                  const Reference < XPropertySet > & rPropSet,
                                  const XMLPropertyState** ppAddStates, bool bDontSeek )
{
    UniReference < SvXMLExportPropertyMapper > xPropMapper;
    switch( nFamily )
    {
    case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
        xPropMapper = GetParaPropMapper();
        break;
    case XML_STYLE_FAMILY_TEXT_TEXT:
        xPropMapper = GetTextPropMapper();
        break;
    case XML_STYLE_FAMILY_TEXT_FRAME:
        xPropMapper = GetAutoFramePropMapper();
        break;
    case XML_STYLE_FAMILY_TEXT_SECTION:
        xPropMapper = GetSectionPropMapper();
        break;
    case XML_STYLE_FAMILY_TEXT_RUBY:
        xPropMapper = GetRubyPropMapper();
        break;
    }
    DBG_ASSERT( xPropMapper.is(), "There is the property mapper?" );

    vector< XMLPropertyState > xPropStates =
            xPropMapper->Filter( rPropSet );

    if( ppAddStates )
    {
        while( *ppAddStates )
        {
            xPropStates.push_back( **ppAddStates );
            ppAddStates++;
        }
    }

    if( !xPropStates.empty() )
    {
        Reference< XPropertySetInfo > xPropSetInfo(rPropSet->getPropertySetInfo());
        OUString sParent, sCondParent;
        sal_uInt16 nIgnoreProps = 0;
        switch( nFamily )
        {
        case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
            if( xPropSetInfo->hasPropertyByName( sParaStyleName ) )
            {
                rPropSet->getPropertyValue( sParaStyleName ) >>= sParent;
            }
            if( xPropSetInfo->hasPropertyByName( sParaConditionalStyleName ) )
            {
                rPropSet->getPropertyValue( sParaConditionalStyleName ) >>= sCondParent;
            }
            if( xPropSetInfo->hasPropertyByName( sNumberingRules ) )
            {
                Reference < XIndexReplace > xNumRule(rPropSet->getPropertyValue( sNumberingRules ), uno::UNO_QUERY);
                if( xNumRule.is() && xNumRule->getCount() )
                {
                    Reference < XNamed > xNamed( xNumRule, UNO_QUERY );
                    OUString sName;
                    if( xNamed.is() )
                        sName = xNamed->getName();
                    sal_Bool bAdd = sName.isEmpty();
                    if( !bAdd )
                    {
                        Reference < XPropertySet > xNumPropSet( xNumRule,
                                                                UNO_QUERY );
                        const OUString sIsAutomatic( "IsAutomatic"  );
                        if( xNumPropSet.is() &&
                            xNumPropSet->getPropertySetInfo()
                                       ->hasPropertyByName( sIsAutomatic ) )
                        {
                            bAdd = *(sal_Bool *)xNumPropSet->getPropertyValue( sIsAutomatic ).getValue();
                            
                            const OUString sNumberingIsOutline( "NumberingIsOutline"  );
                            if ( bAdd &&
                                 xNumPropSet->getPropertySetInfo()
                                           ->hasPropertyByName( sNumberingIsOutline ) )
                            {
                                bAdd = !(*(sal_Bool *)xNumPropSet->getPropertyValue( sNumberingIsOutline ).getValue());
                            }
                        }
                        else
                        {
                            bAdd = sal_True;
                        }
                    }
                    if( bAdd )
                        pListAutoPool->Add( xNumRule );
                }
            }
            break;
        case XML_STYLE_FAMILY_TEXT_TEXT:
            {
                
                UniReference< XMLPropertySetMapper > xPM(xPropMapper->getPropertySetMapper());
                for( ::std::vector< XMLPropertyState >::iterator i(xPropStates.begin());
                      nIgnoreProps < 2 && i != xPropStates.end(); )
                {
                    if( i->mnIndex == -1 )
                    {
                        ++i;
                        continue;
                    }

                    switch( xPM->GetEntryContextId(i->mnIndex) )
                    {
                    case CTF_CHAR_STYLE_NAME:
                    case CTF_HYPERLINK_URL:
                        i->mnIndex = -1;
                        nIgnoreProps++;
                        i = xPropStates.erase( i );
                        break;
                    default:
                        ++i;
                        break;
                    }
                }
            }
            break;
        case XML_STYLE_FAMILY_TEXT_FRAME:
            if( xPropSetInfo->hasPropertyByName( sFrameStyleName ) )
            {
                rPropSet->getPropertyValue( sFrameStyleName ) >>= sParent;
            }
            break;
        case XML_STYLE_FAMILY_TEXT_SECTION:
        case XML_STYLE_FAMILY_TEXT_RUBY:
            ; 
            break;
        }
        if( (xPropStates.size() - nIgnoreProps) > 0 )
        {
            GetAutoStylePool().Add( nFamily, sParent, xPropStates, bDontSeek );
            if( !sCondParent.isEmpty() && sParent != sCondParent )
                GetAutoStylePool().Add( nFamily, sCondParent, xPropStates );
        }
    }
}

static bool lcl_validPropState( const XMLPropertyState& rState )
{
    return rState.mnIndex != -1;
}

void XMLTextParagraphExport::Add( sal_uInt16 nFamily,
                                  MultiPropertySetHelper& rPropSetHelper,
                                  const Reference < XPropertySet > & rPropSet,
                                  const XMLPropertyState** ppAddStates)
{
    UniReference < SvXMLExportPropertyMapper > xPropMapper;
    switch( nFamily )
    {
    case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
        xPropMapper = GetParaPropMapper();
        break;
    }
    DBG_ASSERT( xPropMapper.is(), "There is the property mapper?" );

    vector< XMLPropertyState > xPropStates(xPropMapper->Filter( rPropSet ));
    if( ppAddStates )
    {
        while( *ppAddStates )
        {
            xPropStates.push_back( **ppAddStates );
            ++ppAddStates;
        }
    }

    if( rPropSetHelper.hasProperty( NUMBERING_RULES_AUTO ) )
    {
        Reference < XIndexReplace > xNumRule(rPropSetHelper.getValue( NUMBERING_RULES_AUTO,
            rPropSet, true ), uno::UNO_QUERY);
        if( xNumRule.is() && xNumRule->getCount() )
        {
            Reference < XNamed > xNamed( xNumRule, UNO_QUERY );
            OUString sName;
            if( xNamed.is() )
                sName = xNamed->getName();
            sal_Bool bAdd = sName.isEmpty();
            if( !bAdd )
            {
                Reference < XPropertySet > xNumPropSet( xNumRule,
                                                        UNO_QUERY );
                const OUString sIsAutomatic( "IsAutomatic"  );
                if( xNumPropSet.is() &&
                    xNumPropSet->getPropertySetInfo()
                               ->hasPropertyByName( sIsAutomatic ) )
                {
                    bAdd = *(sal_Bool *)xNumPropSet->getPropertyValue( sIsAutomatic ).getValue();
                    
                    const OUString sNumberingIsOutline( "NumberingIsOutline"  );
                    if ( bAdd &&
                         xNumPropSet->getPropertySetInfo()
                                   ->hasPropertyByName( sNumberingIsOutline ) )
                    {
                        bAdd = !(*(sal_Bool *)xNumPropSet->getPropertyValue( sNumberingIsOutline ).getValue());
                    }
                }
                else
                {
                    bAdd = sal_True;
                }
            }
            if( bAdd )
                pListAutoPool->Add( xNumRule );
        }
    }

    if( !xPropStates.empty() )
    {
        OUString sParent, sCondParent;
        switch( nFamily )
        {
        case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
            if( rPropSetHelper.hasProperty( PARA_STYLE_NAME_AUTO ) )
            {
                rPropSetHelper.getValue( PARA_STYLE_NAME_AUTO, rPropSet,
                                                true ) >>= sParent;
            }
            if( rPropSetHelper.hasProperty( PARA_CONDITIONAL_STYLE_NAME_AUTO ) )
            {
                rPropSetHelper.getValue( PARA_CONDITIONAL_STYLE_NAME_AUTO,
                                                 rPropSet, true ) >>= sCondParent;
            }

            break;
        }

        if( find_if( xPropStates.begin(), xPropStates.end(), lcl_validPropState ) != xPropStates.end() )
        {
            GetAutoStylePool().Add( nFamily, sParent, xPropStates );
            if( !sCondParent.isEmpty() && sParent != sCondParent )
                GetAutoStylePool().Add( nFamily, sCondParent, xPropStates );
        }
    }
}

OUString XMLTextParagraphExport::Find(
        sal_uInt16 nFamily,
           const Reference < XPropertySet > & rPropSet,
        const OUString& rParent,
        const XMLPropertyState** ppAddStates) const
{
    OUString sName( rParent );
    UniReference < SvXMLExportPropertyMapper > xPropMapper;
    switch( nFamily )
    {
    case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
        xPropMapper = GetParaPropMapper();
        break;
    case XML_STYLE_FAMILY_TEXT_FRAME:
        xPropMapper = GetAutoFramePropMapper();
        break;
    case XML_STYLE_FAMILY_TEXT_SECTION:
        xPropMapper = GetSectionPropMapper();
        break;
    case XML_STYLE_FAMILY_TEXT_RUBY:
        xPropMapper = GetRubyPropMapper();
        break;
    }
    DBG_ASSERT( xPropMapper.is(), "There is the property mapper?" );
    if( !xPropMapper.is() )
        return sName;
    vector< XMLPropertyState > xPropStates(xPropMapper->Filter( rPropSet ));
    if( ppAddStates )
    {
        while( *ppAddStates )
        {
            xPropStates.push_back( **ppAddStates );
            ++ppAddStates;
        }
    }
    if( find_if( xPropStates.begin(), xPropStates.end(), lcl_validPropState ) != xPropStates.end() )
        sName = GetAutoStylePool().Find( nFamily, sName, xPropStates );

    return sName;
}

OUString XMLTextParagraphExport::FindTextStyleAndHyperlink(
           const Reference < XPropertySet > & rPropSet,
        sal_Bool& rbHyperlink,
        sal_Bool& rbHasCharStyle,
        sal_Bool& rbHasAutoStyle,
        const XMLPropertyState** ppAddStates ) const
{
    UniReference < SvXMLExportPropertyMapper > xPropMapper(GetTextPropMapper());
    vector< XMLPropertyState > xPropStates(xPropMapper->Filter( rPropSet ));

    
    OUString sName;
    rbHyperlink = rbHasCharStyle = rbHasAutoStyle = sal_False;
    sal_uInt16 nIgnoreProps = 0;
    UniReference< XMLPropertySetMapper > xPM(xPropMapper->getPropertySetMapper());
    ::std::vector< XMLPropertyState >::iterator aFirstDel = xPropStates.end();
    ::std::vector< XMLPropertyState >::iterator aSecondDel = xPropStates.end();

    for( ::std::vector< XMLPropertyState >::iterator
            i = xPropStates.begin();
         nIgnoreProps < 2 && i != xPropStates.end();
         ++i )
    {
        if( i->mnIndex == -1 )
            continue;

        switch( xPM->GetEntryContextId(i->mnIndex) )
        {
        case CTF_CHAR_STYLE_NAME:
            i->maValue >>= sName;
            i->mnIndex = -1;
            rbHasCharStyle = !sName.isEmpty();
            if( nIgnoreProps )
                aSecondDel = i;
            else
                aFirstDel = i;
            nIgnoreProps++;
            break;
        case CTF_HYPERLINK_URL:
            rbHyperlink = sal_True;
            i->mnIndex = -1;
            if( nIgnoreProps )
                aSecondDel = i;
            else
                aFirstDel = i;
            nIgnoreProps++;
            break;
        }
    }
    if( ppAddStates )
    {
        while( *ppAddStates )
        {
            xPropStates.push_back( **ppAddStates );
            ppAddStates++;
        }
    }
    if( (xPropStates.size() - nIgnoreProps) > 0L )
    {
        
        
        if ( nIgnoreProps )
        {
            
            
            if( --nIgnoreProps )
                xPropStates.erase( aSecondDel );
            xPropStates.erase( aFirstDel );
        }
        OUString sParent; 
        sName = GetAutoStylePool().Find( XML_STYLE_FAMILY_TEXT_TEXT, sParent, xPropStates );
        
        rbHasAutoStyle = sal_True;
    }

    return sName;
}


void XMLTextParagraphExport::exportListChange(
        const XMLTextNumRuleInfo& rPrevInfo,
        const XMLTextNumRuleInfo& rNextInfo )
{
    
    if ( rPrevInfo.GetLevel() > 0 )
    {
        sal_Int16 nListLevelsToBeClosed = 0;
        if ( !rNextInfo.BelongsToSameList( rPrevInfo ) ||
             rNextInfo.GetLevel() <= 0 )
        {
            
            nListLevelsToBeClosed = rPrevInfo.GetLevel();
        }
        else if ( rPrevInfo.GetLevel() > rNextInfo.GetLevel() )
        {
            
            DBG_ASSERT( rNextInfo.GetLevel() > 0,
                        "<rPrevInfo.GetLevel() > 0> not hold. Serious defect -> please inform OD." );
            nListLevelsToBeClosed = rPrevInfo.GetLevel() - rNextInfo.GetLevel();
        }

        if ( nListLevelsToBeClosed > 0 &&
             pListElements &&
             pListElements->size() >= sal::static_int_cast< sal_uInt32 >( 2 * nListLevelsToBeClosed ) )
        {
            do {
                for(size_t j = 0; j < 2; ++j)
                {
                    OUString aElem(pListElements->back());
                    pListElements->pop_back();
                    GetExport().EndElement(aElem, sal_True);
                }

                
                mpTextListsHelper->PopListFromStack();

                --nListLevelsToBeClosed;
            } while ( nListLevelsToBeClosed > 0 );
        }
    }

    const bool bExportODF =
                ( GetExport().getExportFlags() & EXPORT_OASIS ) != 0;
    const SvtSaveOptions::ODFDefaultVersion eODFDefaultVersion =
                                    GetExport().getDefaultVersion();

    
    if ( rNextInfo.GetLevel() > 0 )
    {
        bool bRootListToBeStarted = false;
        sal_Int16 nListLevelsToBeOpened = 0;
        if ( !rPrevInfo.BelongsToSameList( rNextInfo ) ||
             rPrevInfo.GetLevel() <= 0 )
        {
            
            bRootListToBeStarted = true;
            nListLevelsToBeOpened = rNextInfo.GetLevel();
        }
        else if ( rNextInfo.GetLevel() > rPrevInfo.GetLevel() )
        {
            
            DBG_ASSERT( rPrevInfo.GetLevel() > 0,
                        "<rPrevInfo.GetLevel() > 0> not hold. Serious defect -> please inform OD." );
            nListLevelsToBeOpened = rNextInfo.GetLevel() - rPrevInfo.GetLevel();
        }

        if ( nListLevelsToBeOpened > 0 )
        {
            const OUString sListStyleName( rNextInfo.GetNumRulesName() );
            
            
            const OUString sListId( rNextInfo.GetListId() );
            bool bExportListStyle( true );
            bool bRestartNumberingAtContinuedList( false );
            sal_Int32 nRestartValueForContinuedList( -1 );
            bool bContinueingPreviousSubList = !bRootListToBeStarted &&
                                               rNextInfo.IsContinueingPreviousSubTree();
            do {
                GetExport().CheckAttrList();

                if ( bRootListToBeStarted )
                {
                    if ( !mpTextListsHelper->IsListProcessed( sListId ) )
                    {
                        if ( bExportODF &&
                             eODFDefaultVersion >= SvtSaveOptions::ODFVER_012 &&
                             !sListId.isEmpty() )
                        {
                            /* Property text:id at element <text:list> has to be
                               replaced by property xml:id (#i92221#)
                            */
                            GetExport().AddAttribute( XML_NAMESPACE_XML,
                                                      XML_ID,
                                                      sListId );
                        }
                        mpTextListsHelper->KeepListAsProcessed( sListId,
                                                                sListStyleName,
                                                                OUString() );
                    }
                    else
                    {
                        const OUString sNewListId(
                                        mpTextListsHelper->GenerateNewListId() );
                        if ( bExportODF &&
                             eODFDefaultVersion >= SvtSaveOptions::ODFVER_012 &&
                             !sListId.isEmpty() )
                        {
                            /* Property text:id at element <text:list> has to be
                               replaced by property xml:id (#i92221#)
                            */
                            GetExport().AddAttribute( XML_NAMESPACE_XML,
                                                      XML_ID,
                                                      sNewListId );
                        }

                        const OUString sContinueListId =
                            mpTextListsHelper->GetLastContinuingListId( sListId );
                        
                        
                        mpTextListsHelper->StoreLastContinuingList( sListId,
                                                                    sNewListId );
                        if ( sListStyleName ==
                                mpTextListsHelper->GetListStyleOfLastProcessedList() &&
                             
                             sContinueListId ==
                                mpTextListsHelper->GetLastProcessedListId() &&
                             !rNextInfo.IsRestart() )
                        {
                            GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                                      XML_CONTINUE_NUMBERING,
                                                      XML_TRUE );
                        }
                        else
                        {
                            if ( bExportODF &&
                                 eODFDefaultVersion >= SvtSaveOptions::ODFVER_012 &&
                                 !sListId.isEmpty() )
                            {
                                GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                                          XML_CONTINUE_LIST,
                                                          sContinueListId );
                            }

                            if ( rNextInfo.IsRestart() &&
                                 ( nListLevelsToBeOpened != 1 ||
                                   !rNextInfo.HasStartValue() ) )
                            {
                                bRestartNumberingAtContinuedList = true;
                                nRestartValueForContinuedList =
                                            rNextInfo.GetListLevelStartValue();
                            }
                        }

                        mpTextListsHelper->KeepListAsProcessed( sNewListId,
                                                                sListStyleName,
                                                                sContinueListId );
                    }

                    GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_STYLE_NAME,
                            GetExport().EncodeStyleName( sListStyleName ) );
                    bExportListStyle = false;

                    bRootListToBeStarted = false;
                }
                else if ( bExportListStyle &&
                          !mpTextListsHelper->EqualsToTopListStyleOnStack( sListStyleName ) )
                {
                    GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_STYLE_NAME,
                            GetExport().EncodeStyleName( sListStyleName ) );
                    bExportListStyle = false;

                }
                else
                {
                    
                    if (rNextInfo.IsRestart() && !rNextInfo.HasStartValue())
                    {
                        bRestartNumberingAtContinuedList = true;
                        nRestartValueForContinuedList =
                                        rNextInfo.GetListLevelStartValue();
                    }
                }

                if ( bContinueingPreviousSubList )
                {
                    GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                              XML_CONTINUE_NUMBERING, XML_TRUE );
                    bContinueingPreviousSubList = false;
                }

                enum XMLTokenEnum eLName = XML_LIST;

                OUString aElem(GetExport().GetNamespaceMap().GetQNameByKey(
                                            XML_NAMESPACE_TEXT,
                                            GetXMLToken(eLName) ) );
                GetExport().IgnorableWhitespace();
                GetExport().StartElement(aElem, sal_False);

                if(!pListElements)
                    pListElements = new std::vector<OUString>;
                pListElements->push_back(aElem);

                mpTextListsHelper->PushListOnStack( sListId,
                                                    sListStyleName );

                
                GetExport().CheckAttrList();

                /* Export start value at correct list item (#i97309#) */
                if ( nListLevelsToBeOpened == 1 )
                {
                    if ( rNextInfo.HasStartValue() )
                    {
                        OUStringBuffer aBuffer;
                        aBuffer.append( (sal_Int32)rNextInfo.GetStartValue() );
                        GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_START_VALUE,
                                      aBuffer.makeStringAndClear() );
                    }
                    else if (bRestartNumberingAtContinuedList)
                    {
                        OUStringBuffer aBuffer;
                        aBuffer.append( nRestartValueForContinuedList );
                        GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                                  XML_START_VALUE,
                                                  aBuffer.makeStringAndClear() );
                        bRestartNumberingAtContinuedList = false;
                    }
                }

                eLName = ( rNextInfo.IsNumbered() || nListLevelsToBeOpened > 1 )
                         ? XML_LIST_ITEM
                         : XML_LIST_HEADER;
                aElem = OUString( GetExport().GetNamespaceMap().GetQNameByKey(
                                            XML_NAMESPACE_TEXT,
                                            GetXMLToken(eLName) ) );
                GetExport().IgnorableWhitespace();
                GetExport().StartElement(aElem, sal_False);
                pListElements->push_back(aElem);

                
                if ( GetExport().exportTextNumberElement() &&
                     eLName == XML_LIST_ITEM && nListLevelsToBeOpened == 1 && 
                     !rNextInfo.ListLabelString().isEmpty() )
                {
                    const OUString aTextNumberElem =
                            OUString( GetExport().GetNamespaceMap().GetQNameByKey(
                                      XML_NAMESPACE_TEXT,
                                      GetXMLToken(XML_NUMBER) ) );
                    GetExport().IgnorableWhitespace();
                    GetExport().StartElement( aTextNumberElem, sal_False );
                    GetExport().Characters( rNextInfo.ListLabelString() );
                    GetExport().EndElement( aTextNumberElem, sal_True );
                }
                --nListLevelsToBeOpened;
            } while ( nListLevelsToBeOpened > 0 );
        }
    }

    if ( rNextInfo.GetLevel() > 0 &&
         rNextInfo.IsNumbered() &&
         rPrevInfo.BelongsToSameList( rNextInfo ) &&
         rPrevInfo.GetLevel() >= rNextInfo.GetLevel() )
    {
        
        DBG_ASSERT( pListElements && pListElements->size() >= 2,
                "SwXMLExport::ExportListChange: list elements missing" );

        GetExport().EndElement(pListElements->back(), sal_True );
        pListElements->pop_back();

        
        if ( rNextInfo.IsRestart() && !rNextInfo.HasStartValue() &&
             rNextInfo.GetLevel() != 1 )
        {
            
            GetExport().EndElement(pListElements->back(), sal_True );
            GetExport().IgnorableWhitespace();
            GetExport().StartElement(pListElements->back(), sal_False);
        }

        
        GetExport().CheckAttrList();
        if( rNextInfo.HasStartValue() )
        {
            OUStringBuffer aBuffer;
            aBuffer.append( (sal_Int32)rNextInfo.GetStartValue() );
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_START_VALUE,
                                      aBuffer.makeStringAndClear() );
        }
        
        else if ( rNextInfo.IsRestart() && /*!rNextInfo.HasStartValue() &&*/
                  rNextInfo.GetLevel() == 1 )
        {
            OUStringBuffer aBuffer;
            aBuffer.append( (sal_Int32)rNextInfo.GetListLevelStartValue() );
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_START_VALUE,
                                      aBuffer.makeStringAndClear() );
        }
        if ( ( GetExport().getExportFlags() & EXPORT_OASIS ) != 0 &&
             GetExport().getDefaultVersion() >= SvtSaveOptions::ODFVER_012 )
        {
            const OUString sListStyleName( rNextInfo.GetNumRulesName() );
            if ( !mpTextListsHelper->EqualsToTopListStyleOnStack( sListStyleName ) )
            {
                GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                          XML_STYLE_OVERRIDE,
                                          GetExport().EncodeStyleName( sListStyleName ) );
            }
        }
        OUString aElem( GetExport().GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_TEXT,
                                GetXMLToken(XML_LIST_ITEM) ) );
        GetExport().IgnorableWhitespace();
        GetExport().StartElement(aElem, sal_False );
        pListElements->push_back(aElem);

        
        if ( GetExport().exportTextNumberElement() &&
             !rNextInfo.ListLabelString().isEmpty() )
        {
            const OUString aTextNumberElem =
                    OUString( GetExport().GetNamespaceMap().GetQNameByKey(
                              XML_NAMESPACE_TEXT,
                              GetXMLToken(XML_NUMBER) ) );
            GetExport().IgnorableWhitespace();
            GetExport().StartElement( aTextNumberElem, sal_False );
            GetExport().Characters( rNextInfo.ListLabelString() );
            GetExport().EndElement( aTextNumberElem, sal_True );
        }
    }
}

struct XMLTextParagraphExport::Impl
{
    typedef ::std::map<Reference<XFormField>, sal_Int32> FieldMarkMap_t;
    FieldMarkMap_t m_FieldMarkMap;

    explicit Impl() {}
    sal_Int32 AddFieldMarkStart(Reference<XFormField> const& i_xFieldMark)
    {
        assert(m_FieldMarkMap.find(i_xFieldMark) == m_FieldMarkMap.end());
        sal_Int32 const ret(m_FieldMarkMap.size());
        m_FieldMarkMap.insert(::std::make_pair(i_xFieldMark, ret));
        return ret;
    }
    sal_Int32 GetFieldMarkIndex(Reference<XFormField> const& i_xFieldMark)
    {
        FieldMarkMap_t::const_iterator const it(
                m_FieldMarkMap.find(i_xFieldMark));
        
        
        assert(it != m_FieldMarkMap.end());
        return it->second;
    }
};

XMLTextParagraphExport::XMLTextParagraphExport(
        SvXMLExport& rExp,
        SvXMLAutoStylePoolP & rASP
        ) :
    XMLStyleExport( rExp, OUString(), &rASP ),
    m_pImpl(new Impl),
    rAutoStylePool( rASP ),
    pBoundFrameSets(new BoundFrameSets(GetExport().GetModel())),
    pFieldExport( 0 ),
    pListElements( 0 ),
    pListAutoPool( new XMLTextListAutoStylePool( this->GetExport() ) ),
    pSectionExport( NULL ),
    pIndexMarkExport( NULL ),
    pRedlineExport( NULL ),
    pHeadingStyles( NULL ),
    bProgress( sal_False ),
    bBlock( sal_False ),
    bOpenRuby( sal_False ),
    mpTextListsHelper( 0 ),
    maTextListsHelperStack(),
    sActualSize("ActualSize"),
    
    sTitle("Title"),
    sDescription("Description"),
    sAnchorCharStyleName("AnchorCharStyleName"),
    sAnchorPageNo("AnchorPageNo"),
    sAnchorType("AnchorType"),
    sBeginNotice("BeginNotice"),
    sBookmark("Bookmark"),
    sCategory("Category"),
    sChainNextName("ChainNextName"),
    sCharStyleName("CharStyleName"),
    sCharStyleNames("CharStyleNames"),
    sContourPolyPolygon("ContourPolyPolygon"),
    sDocumentIndex("DocumentIndex"),
    sDocumentIndexMark("DocumentIndexMark"),
    sEndNotice("EndNotice"),
    sFootnote("Footnote"),
    sFootnoteCounting("FootnoteCounting"),
    sFrame("Frame"),
    sFrameHeightAbsolute("FrameHeightAbsolute"),
    sFrameHeightPercent("FrameHeightPercent"),
    sFrameStyleName("FrameStyleName"),
    sFrameWidthAbsolute("FrameWidthAbsolute"),
    sFrameWidthPercent("FrameWidthPercent"),
    sGraphicFilter("GraphicFilter"),
    sGraphicRotation("GraphicRotation"),
    sGraphicURL("GraphicURL"),
    sReplacementGraphicURL("ReplacementGraphicURL"),
    sHeight("Height"),
    sHoriOrient("HoriOrient"),
    sHoriOrientPosition("HoriOrientPosition"),
    sHyperLinkName("HyperLinkName"),
    sHyperLinkTarget("HyperLinkTarget"),
    sHyperLinkURL("HyperLinkURL"),
    sIsAutomaticContour("IsAutomaticContour"),
    sIsCollapsed("IsCollapsed"),
    sIsPixelContour("IsPixelContour"),
    sIsStart("IsStart"),
    sIsSyncHeightToWidth("IsSyncHeightToWidth"),
    sIsSyncWidthToHeight("IsSyncWidthToHeight"),
    sNumberingRules("NumberingRules"),
    sNumberingType("NumberingType"),
    sPageDescName("PageDescName"),
    sPageStyleName("PageStyleName"),
    sParaChapterNumberingLevel("ParaChapterNumberingLevel"),
    sParaConditionalStyleName("ParaConditionalStyleName"),
    sParagraphService("com.sun.star.text.Paragraph"),
    sParaStyleName("ParaStyleName"),
    sPositionEndOfDoc("PositionEndOfDoc"),
    sPrefix("Prefix"),
    sRedline("Redline"),
    sReferenceId("ReferenceId"),
    sReferenceMark("ReferenceMark"),
    sRelativeHeight("RelativeHeight"),
    sRelativeWidth("RelativeWidth"),
    sRuby("Ruby"),
    sRubyAdjust("RubyAdjust"),
    sRubyCharStyleName("RubyCharStyleName"),
    sRubyText("RubyText"),
    sServerMap("ServerMap"),
    sShapeService("com.sun.star.drawing.Shape"),
    sSizeType("SizeType"),
    sSoftPageBreak( "SoftPageBreak"  ),
    sStartAt("StartAt"),
    sSuffix("Suffix"),
    sTableService("com.sun.star.text.TextTable"),
    sText("Text"),
    sTextContentService("com.sun.star.text.TextContent"),
    sTextEmbeddedService("com.sun.star.text.TextEmbeddedObject"),
    sTextEndnoteService("com.sun.star.text.Endnote"),
    sTextField("TextField"),
    sTextFieldService("com.sun.star.text.TextField"),
    sTextFrameService("com.sun.star.text.TextFrame"),
    sTextGraphicService("com.sun.star.text.TextGraphicObject"),
    sTextPortionType("TextPortionType"),
    sTextSection("TextSection"),
    sUnvisitedCharStyleName("UnvisitedCharStyleName"),
    sVertOrient("VertOrient"),
    sVertOrientPosition("VertOrientPosition"),
    sVisitedCharStyleName("VisitedCharStyleName"),
    sWidth("Width"),
    sWidthType( "WidthType"  ),
    sTextFieldStart( "TextFieldStart"  ),
    sTextFieldEnd( "TextFieldEnd"  ),
    sTextFieldStartEnd( "TextFieldStartEnd"  ),
    aCharStyleNamesPropInfoCache( sCharStyleNames )
{
    UniReference < XMLPropertySetMapper > xPropMapper(new XMLTextPropertySetMapper( TEXT_PROP_MAP_PARA, true ));
    xParaPropMapper = new XMLTextExportPropertySetMapper( xPropMapper,
                                                             GetExport() );

    OUString sFamily( GetXMLToken(XML_PARAGRAPH) );
    OUString aPrefix(static_cast<sal_Unicode>('P'));
    rAutoStylePool.AddFamily( XML_STYLE_FAMILY_TEXT_PARAGRAPH, sFamily,
                              xParaPropMapper, aPrefix );

    xPropMapper = new XMLTextPropertySetMapper( TEXT_PROP_MAP_TEXT, true );
    xTextPropMapper = new XMLTextExportPropertySetMapper( xPropMapper,
                                                             GetExport() );
    sFamily = GetXMLToken(XML_TEXT);
    aPrefix = "T";
    rAutoStylePool.AddFamily( XML_STYLE_FAMILY_TEXT_TEXT, sFamily,
                              xTextPropMapper, aPrefix );

    xPropMapper = new XMLTextPropertySetMapper( TEXT_PROP_MAP_AUTO_FRAME, true );
    xAutoFramePropMapper = new XMLTextExportPropertySetMapper( xPropMapper,
                                                                  GetExport() );
    sFamily = XML_STYLE_FAMILY_SD_GRAPHICS_NAME;
    aPrefix = "fr";
    rAutoStylePool.AddFamily( XML_STYLE_FAMILY_TEXT_FRAME, sFamily,
                              xAutoFramePropMapper, aPrefix );

    xPropMapper = new XMLTextPropertySetMapper( TEXT_PROP_MAP_SECTION, true );
    xSectionPropMapper = new XMLTextExportPropertySetMapper( xPropMapper,
                                                             GetExport() );
    sFamily = GetXMLToken( XML_SECTION );
    aPrefix = "Sect" ;
    rAutoStylePool.AddFamily( XML_STYLE_FAMILY_TEXT_SECTION, sFamily,
                              xSectionPropMapper, aPrefix );

    xPropMapper = new XMLTextPropertySetMapper( TEXT_PROP_MAP_RUBY, true );
    xRubyPropMapper = new SvXMLExportPropertyMapper( xPropMapper );
    sFamily = GetXMLToken( XML_RUBY );
    aPrefix = "Ru";
    rAutoStylePool.AddFamily( XML_STYLE_FAMILY_TEXT_RUBY, sFamily,
                              xRubyPropMapper, aPrefix );

    xPropMapper = new XMLTextPropertySetMapper( TEXT_PROP_MAP_FRAME, true );
    xFramePropMapper = new XMLTextExportPropertySetMapper( xPropMapper,
                                                              GetExport() );

    pSectionExport = new XMLSectionExport( rExp, *this );
    pIndexMarkExport = new XMLIndexMarkExport( rExp );

    if( ! IsBlockMode() &&
        Reference<XRedlinesSupplier>( GetExport().GetModel(), UNO_QUERY ).is())
        pRedlineExport = new XMLRedlineExport( rExp );

    
    
    

    
    sal_Int32 nIndex = xTextPropMapper->getPropertySetMapper()->FindEntryIndex(
                                "", XML_NAMESPACE_STYLE,
                                GetXMLToken(XML_TEXT_COMBINE));
    pFieldExport = new XMLTextFieldExport( rExp, new XMLPropertyState( nIndex, uno::makeAny(sal_True) ) );
    PushNewTextListsHelper();
}

XMLTextParagraphExport::~XMLTextParagraphExport()
{
    delete pHeadingStyles;
    delete pRedlineExport;
    delete pIndexMarkExport;
    delete pSectionExport;
    delete pFieldExport;
    delete pListElements;
    delete pListAutoPool;
#ifdef DBG_UTIL
    txtparae_bContainsIllegalCharacters = sal_False;
#endif
    PopTextListsHelper();
    DBG_ASSERT( maTextListsHelperStack.empty(),
                "misusage of text lists helper stack - it is not empty. Serious defect - please inform OD" );
}

SvXMLExportPropertyMapper *XMLTextParagraphExport::CreateShapeExtPropMapper(
        SvXMLExport& rExport )
{
    UniReference < XMLPropertySetMapper > xPropMapper =
        new XMLTextPropertySetMapper( TEXT_PROP_MAP_SHAPE, true );
    return new XMLTextExportPropertySetMapper( xPropMapper, rExport );
}

SvXMLExportPropertyMapper *XMLTextParagraphExport::CreateCharExtPropMapper(
        SvXMLExport& rExport)
{
    XMLPropertySetMapper *pPropMapper =
        new XMLTextPropertySetMapper( TEXT_PROP_MAP_TEXT, true );
    return new XMLTextExportPropertySetMapper( pPropMapper, rExport );
}

SvXMLExportPropertyMapper *XMLTextParagraphExport::CreateParaExtPropMapper(
        SvXMLExport& rExport)
{
    XMLPropertySetMapper *pPropMapper =
        new XMLTextPropertySetMapper( TEXT_PROP_MAP_SHAPE_PARA, true );
    return new XMLTextExportPropertySetMapper( pPropMapper, rExport );
}

SvXMLExportPropertyMapper *XMLTextParagraphExport::CreateParaDefaultExtPropMapper(
        SvXMLExport& rExport)
{
    XMLPropertySetMapper *pPropMapper =
        new XMLTextPropertySetMapper( TEXT_PROP_MAP_TEXT_ADDITIONAL_DEFAULTS, true );
    return new XMLTextExportPropertySetMapper( pPropMapper, rExport );
}

void XMLTextParagraphExport::exportPageFrames( sal_Bool bAutoStyles,
                                               sal_Bool bIsProgress )
{
    const TextContentSet* const pTexts = pBoundFrameSets->GetTexts()->GetPageBoundContents();
    const TextContentSet* const pGraphics = pBoundFrameSets->GetGraphics()->GetPageBoundContents();
    const TextContentSet* const pEmbeddeds = pBoundFrameSets->GetEmbeddeds()->GetPageBoundContents();
    const TextContentSet* const pShapes = pBoundFrameSets->GetShapes()->GetPageBoundContents();
    for(TextContentSet::const_iterator_t it = pTexts->getBegin();
        it != pTexts->getEnd();
        ++it)
        exportTextFrame(*it, bAutoStyles, bIsProgress, sal_True);
    for(TextContentSet::const_iterator_t it = pGraphics->getBegin();
        it != pGraphics->getEnd();
        ++it)
        exportTextGraphic(*it, bAutoStyles);
    for(TextContentSet::const_iterator_t it = pEmbeddeds->getBegin();
        it != pEmbeddeds->getEnd();
        ++it)
        exportTextEmbedded(*it, bAutoStyles);
    for(TextContentSet::const_iterator_t it = pShapes->getBegin();
        it != pShapes->getEnd();
        ++it)
        exportShape(*it, bAutoStyles);
}

void XMLTextParagraphExport::exportFrameFrames(
        sal_Bool bAutoStyles,
        sal_Bool bIsProgress,
        const Reference < XTextFrame > *pParentTxtFrame )
{
    const TextContentSet* const pTexts = pBoundFrameSets->GetTexts()->GetFrameBoundContents(*pParentTxtFrame);
    if(pTexts)
        for(TextContentSet::const_iterator_t it = pTexts->getBegin();
            it != pTexts->getEnd();
            ++it)
            exportTextFrame(*it, bAutoStyles, bIsProgress, sal_True);
    const TextContentSet* const pGraphics = pBoundFrameSets->GetGraphics()->GetFrameBoundContents(*pParentTxtFrame);
    if(pGraphics)
        for(TextContentSet::const_iterator_t it = pGraphics->getBegin();
            it != pGraphics->getEnd();
            ++it)
            exportTextGraphic(*it, bAutoStyles);
    const TextContentSet* const pEmbeddeds = pBoundFrameSets->GetEmbeddeds()->GetFrameBoundContents(*pParentTxtFrame);
    if(pEmbeddeds)
        for(TextContentSet::const_iterator_t it = pEmbeddeds->getBegin();
            it != pEmbeddeds->getEnd();
            ++it)
            exportTextEmbedded(*it, bAutoStyles);
    const TextContentSet* const pShapes = pBoundFrameSets->GetShapes()->GetFrameBoundContents(*pParentTxtFrame);
    if(pShapes)
        for(TextContentSet::const_iterator_t it = pShapes->getBegin();
            it != pShapes->getEnd();
            ++it)
            exportShape(*it, bAutoStyles);
}




static const enum XMLTokenEnum lcl_XmlReferenceElements[] = {
    XML_REFERENCE_MARK, XML_REFERENCE_MARK_START, XML_REFERENCE_MARK_END };
static const enum XMLTokenEnum lcl_XmlBookmarkElements[] = {
    XML_BOOKMARK, XML_BOOKMARK_START, XML_BOOKMARK_END };



bool XMLTextParagraphExport::collectTextAutoStylesOptimized( sal_Bool bIsProgress )
{
    GetExport().GetShapeExport(); 

    const sal_Bool bAutoStyles = sal_True;
    const sal_Bool bExportContent = sal_False;

    
    Reference< XAutoStylesSupplier > xAutoStylesSupp( GetExport().GetModel(), UNO_QUERY );
    if ( xAutoStylesSupp.is() )
    {
        Reference< XAutoStyles > xAutoStyleFamilies = xAutoStylesSupp->getAutoStyles();
        OUString sName;
        sal_uInt16 nFamily;

        for ( int i = 0; i < 3; ++i )
        {
            if ( 0 == i )
            {
                sName = "CharacterStyles" ;
                nFamily = XML_STYLE_FAMILY_TEXT_TEXT;
            }
            else if ( 1 == i )
            {
                sName = "RubyStyles" ;
                nFamily = XML_STYLE_FAMILY_TEXT_RUBY;
            }
            else
            {
                sName = "ParagraphStyles" ;
                nFamily = XML_STYLE_FAMILY_TEXT_PARAGRAPH;
            }

            Any aAny = xAutoStyleFamilies->getByName( sName );
            Reference< XAutoStyleFamily > xAutoStyles = *(Reference<XAutoStyleFamily>*)aAny.getValue();
            Reference < XEnumeration > xAutoStylesEnum( xAutoStyles->createEnumeration() );

            while ( xAutoStylesEnum->hasMoreElements() )
            {
                aAny = xAutoStylesEnum->nextElement();
                Reference< XAutoStyle > xAutoStyle = *(Reference<XAutoStyle>*)aAny.getValue();
                Reference < XPropertySet > xPSet( xAutoStyle, uno::UNO_QUERY );
                Add( nFamily, xPSet, 0, true );
            }
        }
    }

    
    Reference< XTextFieldsSupplier > xTextFieldsSupp( GetExport().GetModel(), UNO_QUERY );
    if ( xTextFieldsSupp.is() )
    {
        Reference< XEnumerationAccess > xTextFields = xTextFieldsSupp->getTextFields();
        Reference < XEnumeration > xTextFieldsEnum( xTextFields->createEnumeration() );

        while ( xTextFieldsEnum->hasMoreElements() )
        {
            Any aAny = xTextFieldsEnum->nextElement();
            Reference< XTextField > xTextField = *(Reference<XTextField>*)aAny.getValue();
            exportTextField( xTextField, bAutoStyles, bIsProgress,
                !xAutoStylesSupp.is() );
            try
            {
                Reference < XPropertySet > xSet( xTextField, UNO_QUERY );
                Reference < XText > xText;
                Any a = xSet->getPropertyValue("TextRange");
                a >>= xText;
                if ( xText.is() )
                {
                    exportText( xText, sal_True, bIsProgress, bExportContent );
                    GetExport().GetTextParagraphExport()
                        ->collectTextAutoStyles( xText );
                }
            }
            catch (Exception&)
            {
            }
        }
    }

    
    Reference<XEnumeration> xTextFramesEnum = pBoundFrameSets->GetTexts()->createEnumeration();
    if(xTextFramesEnum.is())
        while(xTextFramesEnum->hasMoreElements())
        {
            Reference<XTextContent> xTxtCntnt(xTextFramesEnum->nextElement(), UNO_QUERY);
            if(xTxtCntnt.is())
                exportTextFrame(xTxtCntnt, bAutoStyles, bIsProgress, bExportContent, 0);
        }

    
    Reference<XEnumeration> xGraphicsEnum = pBoundFrameSets->GetGraphics()->createEnumeration();
    if(xGraphicsEnum.is())
        while(xGraphicsEnum->hasMoreElements())
        {
            Reference<XTextContent> xTxtCntnt(xGraphicsEnum->nextElement(), UNO_QUERY);
            if(xTxtCntnt.is())
                exportTextGraphic(xTxtCntnt, true, 0);
        }

    
    Reference<XEnumeration> xEmbeddedsEnum = pBoundFrameSets->GetEmbeddeds()->createEnumeration();
    if(xEmbeddedsEnum.is())
        while(xEmbeddedsEnum->hasMoreElements())
        {
            Reference<XTextContent> xTxtCntnt(xEmbeddedsEnum->nextElement(), UNO_QUERY);
            if(xTxtCntnt.is())
                exportTextEmbedded(xTxtCntnt, true, 0);
        }

    
    Reference<XEnumeration> xShapesEnum = pBoundFrameSets->GetShapes()->createEnumeration();
    if(xShapesEnum.is())
        while(xShapesEnum->hasMoreElements())
        {
            Reference<XTextContent> xTxtCntnt(xShapesEnum->nextElement(), UNO_QUERY);
            if(xTxtCntnt.is())
            {
                Reference<XServiceInfo> xServiceInfo(xTxtCntnt, UNO_QUERY);
                if( xServiceInfo->supportsService(sShapeService))
                    exportShape(xTxtCntnt, true, 0);
            }
        }

    sal_Int32 nCount;
    
    Reference< XTextSectionsSupplier > xSectionsSupp( GetExport().GetModel(), UNO_QUERY );
    if ( xSectionsSupp.is() )
    {
        Reference< XIndexAccess > xSections( xSectionsSupp->getTextSections(), UNO_QUERY );
        if ( xSections.is() )
        {
            nCount = xSections->getCount();
            for( sal_Int32 i = 0; i < nCount; ++i )
            {
                Any aAny = xSections->getByIndex( i );
                Reference< XTextSection > xSection = *(Reference<XTextSection>*)aAny.getValue();
                Reference < XPropertySet > xPSet( xSection, uno::UNO_QUERY );
                Add( XML_STYLE_FAMILY_TEXT_SECTION, xPSet );
            }
        }
    }

    
    Reference< XTextTablesSupplier > xTablesSupp( GetExport().GetModel(), UNO_QUERY );
    if ( xTablesSupp.is() )
    {
        Reference< XIndexAccess > xTables( xTablesSupp->getTextTables(), UNO_QUERY );
        if ( xTables.is() )
        {
            nCount = xTables->getCount();
            for( sal_Int32 i = 0; i < nCount; ++i )
            {
                Any aAny = xTables->getByIndex( i );
                Reference< XTextTable > xTable = *(Reference<XTextTable>*)aAny.getValue();
                exportTable( xTable, sal_True, sal_True );
            }
        }
    }

    Reference< XNumberingRulesSupplier > xNumberingRulesSupp( GetExport().GetModel(), UNO_QUERY );
    if ( xNumberingRulesSupp.is() )
    {
        Reference< XIndexAccess > xNumberingRules = xNumberingRulesSupp->getNumberingRules();
        nCount = xNumberingRules->getCount();
        
        const OUString sNumberingIsOutline( "NumberingIsOutline"  );
        for( sal_Int32 i = 0; i < nCount; ++i )
        {
            Reference< XIndexReplace > xNumRule( xNumberingRules->getByIndex( i ), UNO_QUERY );
            if( xNumRule.is() && xNumRule->getCount() )
            {
                Reference < XNamed > xNamed( xNumRule, UNO_QUERY );
                OUString sName;
                if( xNamed.is() )
                    sName = xNamed->getName();
                sal_Bool bAdd = sName.isEmpty();
                if( !bAdd )
                {
                    Reference < XPropertySet > xNumPropSet( xNumRule,
                                                            UNO_QUERY );
                    const OUString sIsAutomatic( "IsAutomatic"  );
                    if( xNumPropSet.is() &&
                        xNumPropSet->getPropertySetInfo()
                                   ->hasPropertyByName( sIsAutomatic ) )
                    {
                        bAdd = *(sal_Bool *)xNumPropSet->getPropertyValue( sIsAutomatic ).getValue();
                        
                        if ( bAdd &&
                             xNumPropSet->getPropertySetInfo()
                                       ->hasPropertyByName( sNumberingIsOutline ) )
                        {
                            bAdd = !(*(sal_Bool *)xNumPropSet->getPropertyValue( sNumberingIsOutline ).getValue());
                        }
                    }
                    else
                    {
                        bAdd = sal_True;
                    }
                }
                if( bAdd )
                    pListAutoPool->Add( xNumRule );
            }
        }
    }

    return true;
}

void XMLTextParagraphExport::exportText(
        const Reference < XText > & rText,
        sal_Bool bAutoStyles,
        sal_Bool bIsProgress,
        sal_Bool bExportParagraph )
{
    if( bAutoStyles )
        GetExport().GetShapeExport(); 
                                      
    Reference < XEnumerationAccess > xEA( rText, UNO_QUERY );
    Reference < XEnumeration > xParaEnum(xEA->createEnumeration());
    Reference < XPropertySet > xPropertySet( rText, UNO_QUERY );
    Reference < XTextSection > xBaseSection;

    
    
    DBG_ASSERT( xParaEnum.is(), "We need a paragraph enumeration" );
    if( ! xParaEnum.is() )
        return;

    sal_Bool bExportLevels = sal_True;

    if (xPropertySet.is())
    {
        Reference < XPropertySetInfo > xInfo ( xPropertySet->getPropertySetInfo() );

        if( xInfo.is() )
        {
            if (xInfo->hasPropertyByName( sTextSection ))
            {
                xPropertySet->getPropertyValue(sTextSection) >>= xBaseSection ;
            }

/* #i35937#
            
            
            if( !bAutoStyles )
            {
                
                
                OUString sHasLevels( "HasLevels" );
                if (xInfo->hasPropertyByName( sHasLevels ) )
                {
                    xPropertySet->getPropertyValue(sHasLevels) >>= bExportLevels;
                }
            }
*/
        }
    }

    
    
    if( !bAutoStyles && (pRedlineExport != NULL) )
        pRedlineExport->ExportStartOrEndRedline( xPropertySet, sal_True );
    exportTextContentEnumeration( xParaEnum, bAutoStyles, xBaseSection,
                                  bIsProgress, bExportParagraph, 0, bExportLevels );
    if( !bAutoStyles && (pRedlineExport != NULL) )
        pRedlineExport->ExportStartOrEndRedline( xPropertySet, sal_False );
}

void XMLTextParagraphExport::exportText(
        const Reference < XText > & rText,
        const Reference < XTextSection > & rBaseSection,
        sal_Bool bAutoStyles,
        sal_Bool bIsProgress,
        sal_Bool bExportParagraph )
{
    if( bAutoStyles )
        GetExport().GetShapeExport(); 
                                      
    Reference < XEnumerationAccess > xEA( rText, UNO_QUERY );
    Reference < XEnumeration > xParaEnum(xEA->createEnumeration());

    
    if( ! xParaEnum.is() )
        return;

    
    
    Reference<XPropertySet> xPropertySet;
    if( !bAutoStyles && (pRedlineExport != NULL) )
    {
        xPropertySet.set(rText, uno::UNO_QUERY );
        pRedlineExport->ExportStartOrEndRedline( xPropertySet, sal_True );
    }
    exportTextContentEnumeration( xParaEnum, bAutoStyles, rBaseSection,
                                  bIsProgress, bExportParagraph );
    if( !bAutoStyles && (pRedlineExport != NULL) )
        pRedlineExport->ExportStartOrEndRedline( xPropertySet, sal_False );
}

sal_Bool XMLTextParagraphExport::exportTextContentEnumeration(
        const Reference < XEnumeration > & rContEnum,
        sal_Bool bAutoStyles,
        const Reference < XTextSection > & rBaseSection,
        sal_Bool bIsProgress,
        sal_Bool bExportParagraph,
        const Reference < XPropertySet > *pRangePropSet,
        sal_Bool bExportLevels )
{
    DBG_ASSERT( rContEnum.is(), "No enumeration to export!" );
    sal_Bool bHasMoreElements = rContEnum->hasMoreElements();
    if( !bHasMoreElements )
        return sal_False;

    XMLTextNumRuleInfo aPrevNumInfo;
    XMLTextNumRuleInfo aNextNumInfo;

    sal_Bool bHasContent = sal_False;
    Reference<XTextSection> xCurrentTextSection(rBaseSection);

    MultiPropertySetHelper aPropSetHelper(
                               bAutoStyles ? aParagraphPropertyNamesAuto :
                                          aParagraphPropertyNames );

    sal_Bool bHoldElement = sal_False;
    Reference < XTextContent > xTxtCntnt;
    while( bHoldElement || bHasMoreElements )
    {
        if (bHoldElement)
        {
            bHoldElement = sal_False;
        }
        else
        {
            xTxtCntnt.set(rContEnum->nextElement(), uno::UNO_QUERY);

            aPropSetHelper.resetValues();

        }

        Reference<XServiceInfo> xServiceInfo( xTxtCntnt, UNO_QUERY );
        if( xServiceInfo->supportsService( sParagraphService ) )
        {
            if( bExportLevels )
            {
                if( bAutoStyles )
                {
                    exportListAndSectionChange( xCurrentTextSection, xTxtCntnt,
                                                aPrevNumInfo, aNextNumInfo,
                                                bAutoStyles );
                }
                else
                {
                    /* Pass list auto style pool to <XMLTextNumRuleInfo> instance
                       Pass info about request to export <text:number> element
                       to <XMLTextNumRuleInfo> instance (#i69627#)
                    */
                    aNextNumInfo.Set( xTxtCntnt,
                                      GetExport().writeOutlineStyleAsNormalListStyle(),
                                      GetListAutoStylePool(),
                                      GetExport().exportTextNumberElement() );

                    exportListAndSectionChange( xCurrentTextSection, aPropSetHelper,
                                                TEXT_SECTION, xTxtCntnt,
                                                aPrevNumInfo, aNextNumInfo,
                                                bAutoStyles );
                }
            }

            
            if (pSectionExport->IsMuteSection(xCurrentTextSection))
            {
                
                if( !bAutoStyles )
                    pSectionExport->ExportMasterDocHeadingDummies();

                while (rContEnum->hasMoreElements() &&
                       pSectionExport->IsInSection( xCurrentTextSection,
                                                    xTxtCntnt, sal_True ))
                {
                    xTxtCntnt.set(rContEnum->nextElement(), uno::UNO_QUERY);
                    aPropSetHelper.resetValues();
                    aNextNumInfo.Reset();
                }
                
                bHoldElement =
                    ! pSectionExport->IsInSection( xCurrentTextSection,
                                                   xTxtCntnt, sal_False );
            }
            else
                exportParagraph( xTxtCntnt, bAutoStyles, bIsProgress,
                                 bExportParagraph, aPropSetHelper );
            bHasContent = sal_True;
        }
        else if( xServiceInfo->supportsService( sTableService ) )
        {
            if( !bAutoStyles )
            {
                aNextNumInfo.Reset();
            }

            exportListAndSectionChange( xCurrentTextSection, xTxtCntnt,
                                        aPrevNumInfo, aNextNumInfo,
                                        bAutoStyles );

            if (! pSectionExport->IsMuteSection(xCurrentTextSection))
            {
                
                if ((! bAutoStyles) && (NULL != pRedlineExport))
                    pRedlineExport->ExportStartOrEndRedline(xTxtCntnt, sal_True);

                exportTable( xTxtCntnt, bAutoStyles, bIsProgress  );

                if ((! bAutoStyles) && (NULL != pRedlineExport))
                    pRedlineExport->ExportStartOrEndRedline(xTxtCntnt, sal_False);
            }
            else if( !bAutoStyles )
            {
                
                pSectionExport->ExportMasterDocHeadingDummies();
            }

            bHasContent = sal_True;
        }
        else if( xServiceInfo->supportsService( sTextFrameService ) )
        {
            exportTextFrame( xTxtCntnt, bAutoStyles, bIsProgress, sal_True, pRangePropSet );
        }
        else if( xServiceInfo->supportsService( sTextGraphicService ) )
        {
            exportTextGraphic( xTxtCntnt, bAutoStyles, pRangePropSet );
        }
        else if( xServiceInfo->supportsService( sTextEmbeddedService ) )
        {
            exportTextEmbedded( xTxtCntnt, bAutoStyles, pRangePropSet );
        }
        else if( xServiceInfo->supportsService( sShapeService ) )
        {
            exportShape( xTxtCntnt, bAutoStyles, pRangePropSet );
        }
        else
        {
            DBG_ASSERT( !xTxtCntnt.is(), "unknown text content" );
        }

        if( !bAutoStyles )
        {
            aPrevNumInfo = aNextNumInfo;
        }

        bHasMoreElements = rContEnum->hasMoreElements();
    }

    if( bExportLevels && bHasContent && !bAutoStyles )
    {
        aNextNumInfo.Reset();

        
        exportListAndSectionChange( xCurrentTextSection, rBaseSection,
                                    aPrevNumInfo, aNextNumInfo,
                                    bAutoStyles );
    }

    return sal_True;
}

void XMLTextParagraphExport::exportParagraph(
        const Reference < XTextContent > & rTextContent,
        sal_Bool bAutoStyles, sal_Bool bIsProgress, sal_Bool bExportParagraph,
        MultiPropertySetHelper& rPropSetHelper)
{
    sal_Int16 nOutlineLevel = -1;

    if( bIsProgress )
    {
        ProgressBarHelper *pProgress = GetExport().GetProgressBarHelper();
        pProgress->SetValue( pProgress->GetValue()+1 );
    }

    
    Reference<XMultiPropertySet> xMultiPropSet( rTextContent, UNO_QUERY );
    Reference<XPropertySet> xPropSet( rTextContent, UNO_QUERY );

    
    if( !rPropSetHelper.checkedProperties() )
        rPropSetHelper.hasProperties( xPropSet->getPropertySetInfo() );






    if( bExportParagraph )
    {
        if( bAutoStyles )
        {
            Add( XML_STYLE_FAMILY_TEXT_PARAGRAPH, rPropSetHelper, xPropSet );
        }
        else
        {
            
            GetExport().AddAttributeXmlId(rTextContent);
            GetExport().AddAttributesRDFa(rTextContent);

            OUString sStyle;
            if( rPropSetHelper.hasProperty( PARA_STYLE_NAME ) )
            {
                if( xMultiPropSet.is() )
                    rPropSetHelper.getValue( PARA_STYLE_NAME,
                                                    xMultiPropSet ) >>= sStyle;
                else
                    rPropSetHelper.getValue( PARA_STYLE_NAME,
                                                    xPropSet ) >>= sStyle;
            }

            if( rTextContent.is() )
            {
                const OUString& rIdentifier = GetExport().getInterfaceToIdentifierMapper().getIdentifier( rTextContent );
                if( !rIdentifier.isEmpty() )
                {
                    
                    
                    
                    
                    uno::Reference<rdf::XMetadatable> const xMeta(rTextContent,
                        uno::UNO_QUERY);
                    OSL_ENSURE(!xMeta.is(), "paragraph that implements "
                        "XMetadatable used in interfaceToIdentifierMapper?");
                    GetExport().AddAttributeIdLegacy(XML_NAMESPACE_TEXT,
                        rIdentifier);
                }
            }

            OUString sAutoStyle( sStyle );
            sAutoStyle = Find( XML_STYLE_FAMILY_TEXT_PARAGRAPH, xPropSet, sStyle );
            if( !sAutoStyle.isEmpty() )
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_STYLE_NAME,
                              GetExport().EncodeStyleName( sAutoStyle ) );

            if( rPropSetHelper.hasProperty( PARA_CONDITIONAL_STYLE_NAME ) )
            {
                OUString sCondStyle;
                if( xMultiPropSet.is() )
                    rPropSetHelper.getValue( PARA_CONDITIONAL_STYLE_NAME,
                                                     xMultiPropSet ) >>= sCondStyle;
                else
                    rPropSetHelper.getValue( PARA_CONDITIONAL_STYLE_NAME,
                                                     xPropSet ) >>= sCondStyle;
                if( sCondStyle != sStyle )
                {
                    sCondStyle = Find( XML_STYLE_FAMILY_TEXT_PARAGRAPH, xPropSet,
                                          sCondStyle );
                    if( !sCondStyle.isEmpty() )
                        GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                                  XML_COND_STYLE_NAME,
                              GetExport().EncodeStyleName( sCondStyle ) );
                }
            }

            if( rPropSetHelper.hasProperty( PARA_OUTLINE_LEVEL ) )
            {
                if( xMultiPropSet.is() )
                    rPropSetHelper.getValue( PARA_OUTLINE_LEVEL,
                                                     xMultiPropSet ) >>= nOutlineLevel;
                else
                    rPropSetHelper.getValue( PARA_OUTLINE_LEVEL,
                                                     xPropSet ) >>= nOutlineLevel;

                if( 0 < nOutlineLevel )
                {
                    OUStringBuffer sTmp;
                    sTmp.append( sal_Int32( nOutlineLevel) );
                    GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                              XML_OUTLINE_LEVEL,
                                  sTmp.makeStringAndClear() );

                    if( rPropSetHelper.hasProperty( NUMBERING_IS_NUMBER ) )
                    {
                        bool bIsNumber = false;
                        if( xMultiPropSet.is() )
                            rPropSetHelper.getValue(
                                       NUMBERING_IS_NUMBER, xMultiPropSet ) >>= bIsNumber;
                        else
                            rPropSetHelper.getValue(
                                       NUMBERING_IS_NUMBER, xPropSet ) >>= bIsNumber;

                        OUString sListStyleName;
                        if( xMultiPropSet.is() )
                            rPropSetHelper.getValue(
                                       PARA_NUMBERING_STYLENAME, xMultiPropSet ) >>= sListStyleName;
                        else
                            rPropSetHelper.getValue(
                                       PARA_NUMBERING_STYLENAME, xPropSet ) >>= sListStyleName;

                        bool bAssignedtoOutlineStyle = false;
                        {
                            Reference< XChapterNumberingSupplier > xCNSupplier( GetExport().GetModel(), UNO_QUERY );

                            OUString sOutlineName;
                            if (xCNSupplier.is())
                            {
                                Reference< XIndexReplace > xNumRule ( xCNSupplier->getChapterNumberingRules() );
                                DBG_ASSERT( xNumRule.is(), "no chapter numbering rules" );

                                if (xNumRule.is())
                                {
                                    Reference< XPropertySet > xNumRulePropSet( xNumRule, UNO_QUERY );
                                    xNumRulePropSet->getPropertyValue(
                                        OUString("Name") ) >>= sOutlineName;
                                    bAssignedtoOutlineStyle = ( sListStyleName == sOutlineName );
                                }
                            }
                        }

                        if( ! bIsNumber && bAssignedtoOutlineStyle )
                            GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                                      XML_IS_LIST_HEADER,
                                                      XML_TRUE );
                    }

                    {
                        OUString sParaIsNumberingRestart("ParaIsNumberingRestart");
                        bool bIsRestartNumbering = false;

                        Reference< XPropertySetInfo >
                        xPropSetInfo(xMultiPropSet.is() ?
                                     xMultiPropSet->getPropertySetInfo():
                                     xPropSet->getPropertySetInfo());

                        if (xPropSetInfo->
                            hasPropertyByName(sParaIsNumberingRestart))
                        {
                            xPropSet->getPropertyValue(sParaIsNumberingRestart)
                                >>= bIsRestartNumbering;
                        }

                        if (bIsRestartNumbering)
                        {
                            GetExport().AddAttribute(XML_NAMESPACE_TEXT,
                                                     XML_RESTART_NUMBERING,
                                                     XML_TRUE);

                            OUString sNumberingStartValue("NumberingStartValue");

                            if (xPropSetInfo->
                                hasPropertyByName(sNumberingStartValue))
                            {
                                sal_Int32 nStartValue = 0;

                                xPropSet->getPropertyValue(sNumberingStartValue)
                                    >>= nStartValue;

                                OUStringBuffer sTmpStartValue;

                                sTmpStartValue.append(nStartValue);

                                GetExport().
                                    AddAttribute(XML_NAMESPACE_TEXT,
                                                 XML_START_VALUE,
                                                 sTmpStartValue.
                                                 makeStringAndClear());
                            }
                        }
                    }
                }
            }
        }
    }

    Reference < XEnumerationAccess > xEA( rTextContent, UNO_QUERY );
    Reference < XEnumeration > xTextEnum;
    xTextEnum = xEA->createEnumeration();
    const sal_Bool bHasPortions = xTextEnum.is();

    Reference < XEnumeration> xContentEnum;
    Reference < XContentEnumerationAccess > xCEA( rTextContent, UNO_QUERY );
    if( xCEA.is() )
        xContentEnum.set(xCEA->createContentEnumeration( sTextContentService ));
    const sal_Bool bHasContentEnum = xContentEnum.is() &&
                                        xContentEnum->hasMoreElements();

    Reference < XTextSection > xSection;
    if( bHasContentEnum )
    {
        
        
        
        
        if( bAutoStyles )
        {
            if( xPropSet->getPropertySetInfo()->hasPropertyByName( sTextSection ) )
            {
                xSection.set(xPropSet->getPropertyValue( sTextSection ), uno::UNO_QUERY);
            }
        }
        else
        {
            if( rPropSetHelper.hasProperty( TEXT_SECTION ) )
            {
                xSection.set(rPropSetHelper.getValue( TEXT_SECTION ), uno::UNO_QUERY);
            }
        }
    }

    if( bAutoStyles )
    {
        if( bHasContentEnum )
            exportTextContentEnumeration(
                                    xContentEnum, bAutoStyles, xSection,
                                    bIsProgress, sal_True, 0, sal_True );
        if ( bHasPortions )
            exportTextRangeEnumeration( xTextEnum, bAutoStyles, bIsProgress );
    }
    else
    {
        sal_Bool bPrevCharIsSpace = sal_True;
        enum XMLTokenEnum eElem =
            0 < nOutlineLevel ? XML_H : XML_P;
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT, eElem,
                                  sal_True, sal_False );
        if( bHasContentEnum )
            bPrevCharIsSpace = !exportTextContentEnumeration(
                                    xContentEnum, bAutoStyles, xSection,
                                    bIsProgress );
        exportTextRangeEnumeration( xTextEnum, bAutoStyles, bIsProgress,
                                     bPrevCharIsSpace );
    }
}

void XMLTextParagraphExport::exportTextRangeEnumeration(
        const Reference < XEnumeration > & rTextEnum,
        sal_Bool bAutoStyles, sal_Bool bIsProgress,
        sal_Bool bPrvChrIsSpc )
{
    static const OUString sMeta("InContentMetadata");
    static const OUString sFieldMarkName("__FieldMark_");
    static OUString sAnnotation("Annotation");
    static OUString sAnnotationEnd("AnnotationEnd");

    bool bPrevCharIsSpace = bPrvChrIsSpc;

    /* This is  used for exporting to strict OpenDocument 1.2, in which case traditional
     * bookmarks are used instead of fieldmarks. */
    FieldmarkType openFieldMark = NONE;

    while( rTextEnum->hasMoreElements() )
    {
        Reference<XPropertySet> xPropSet(rTextEnum->nextElement(), UNO_QUERY);
        Reference < XTextRange > xTxtRange(xPropSet, uno::UNO_QUERY);
        Reference<XPropertySetInfo> xPropInfo(xPropSet->getPropertySetInfo());

        if (xPropInfo->hasPropertyByName(sTextPortionType))
        {
            OUString sType;
            xPropSet->getPropertyValue(sTextPortionType) >>= sType;

            if( sType.equals(sText))
            {
                exportTextRange( xTxtRange, bAutoStyles,
                                 bPrevCharIsSpace, openFieldMark);
            }
            else if( sType.equals(sTextField))
            {
                exportTextField( xTxtRange, bAutoStyles, bIsProgress );
                bPrevCharIsSpace = false;
            }
            else if ( sType.equals( sAnnotation ) )
            {
                exportTextField( xTxtRange, bAutoStyles, bIsProgress );
                bPrevCharIsSpace = false;
            }
            else if ( sType.equals( sAnnotationEnd ) )
            {
                Reference<XNamed> xBookmark(xPropSet->getPropertyValue(sBookmark), UNO_QUERY);
                const OUString& rName = xBookmark->getName();
                if ( rName.getLength() > 0 )
                {
                    GetExport().AddAttribute(XML_NAMESPACE_OFFICE, XML_NAME, rName);
                }
                SvXMLElementExport aElem( GetExport(), !bAutoStyles, XML_NAMESPACE_OFFICE, XML_ANNOTATION_END, sal_False, sal_False );
            }
            else if( sType.equals( sFrame ) )
            {
                Reference < XEnumeration> xContentEnum;
                Reference < XContentEnumerationAccess > xCEA( xTxtRange,
                                                              UNO_QUERY );
                if( xCEA.is() )
                    xContentEnum.set(xCEA->createContentEnumeration(
                                                    sTextContentService ));
                
                Reference<XTextSection> xSection;
                if( xContentEnum.is() )
                    exportTextContentEnumeration( xContentEnum,
                                                    bAutoStyles,
                                                    xSection, bIsProgress, sal_True,
                                                     &xPropSet  );

                bPrevCharIsSpace = false;
            }
            else if (sType.equals(sFootnote))
            {
                exportTextFootnote(xPropSet,
                                   xTxtRange->getString(),
                                   bAutoStyles, bIsProgress );
                bPrevCharIsSpace = false;
            }
            else if (sType.equals(sBookmark))
            {
                exportTextMark(xPropSet,
                               sBookmark,
                               lcl_XmlBookmarkElements,
                               bAutoStyles);
            }
            else if (sType.equals(sReferenceMark))
            {
                exportTextMark(xPropSet,
                               sReferenceMark,
                               lcl_XmlReferenceElements,
                               bAutoStyles);
            }
            else if (sType.equals(sDocumentIndexMark))
            {
                pIndexMarkExport->ExportIndexMark(xPropSet, bAutoStyles);
            }
            else if (sType.equals(sRedline))
            {
                if (NULL != pRedlineExport)
                    pRedlineExport->ExportChange(xPropSet, bAutoStyles);
            }
            else if (sType.equals(sRuby))
            {
                exportRuby(xPropSet, bAutoStyles);
            }
            else if (sType.equals(sMeta))
            {
                exportMeta(xPropSet, bAutoStyles, bIsProgress);
            }
            else if (sType.equals(sTextFieldStart))
            {
                Reference< ::com::sun::star::text::XFormField > xFormField(xPropSet->getPropertyValue(sBookmark), UNO_QUERY);

                /* As of now, textmarks are a proposed extension to the OpenDocument standard. */
                if ( GetExport().getDefaultVersion() > SvtSaveOptions::ODFVER_012 )
                {
                    Reference<XNamed> xBookmark(xPropSet->getPropertyValue(sBookmark), UNO_QUERY);
                    if (xBookmark.is())
                    {
                        GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_NAME, xBookmark->getName());
                    }

                    if (xFormField.is())
                    {
                        GetExport().AddAttribute(XML_NAMESPACE_FIELD, XML_TYPE, xFormField->getFieldType());
                    }

                    GetExport().StartElement(XML_NAMESPACE_FIELD, XML_FIELDMARK_START, sal_False);
                    if (xFormField.is())
                    {
                        FieldParamExporter(&GetExport(), xFormField->getParameters()).Export();
                    }
                    GetExport().EndElement(XML_NAMESPACE_FIELD, XML_FIELDMARK_START, sal_False);
                }
                /* The OpenDocument standard does not include support for TextMarks for now, so use bookmarks instead. */
                else
                {
                    if (xFormField.is())
                    {
                        OUString sName;
                        Reference< ::com::sun::star::container::XNameAccess > xParameters(xFormField->getParameters(), UNO_QUERY);
                        if (xParameters.is() && xParameters->hasByName("Name"))
                        {
                            const Any aValue = xParameters->getByName("Name");
                            aValue >>= sName;
                        }
                        if (sName.isEmpty())
                        {   
                            
                            sName = sFieldMarkName + OUString::number(
                                    m_pImpl->AddFieldMarkStart(xFormField));
                        }
                        GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_NAME,
                                sName);
                        SvXMLElementExport aElem( GetExport(), !bAutoStyles,
                            XML_NAMESPACE_TEXT, XML_BOOKMARK_START,
                            sal_False, sal_False );
                        const OUString sFieldType = xFormField->getFieldType();
                        if (sFieldType ==  ODF_FORMTEXT)
                        {
                            openFieldMark = TEXT;
                        }
                        else if (sFieldType == ODF_FORMCHECKBOX)
                        {
                            openFieldMark = CHECK;
                        }
                        else
                        {
                            openFieldMark = NONE;
                        }
                    }
                }
            }
            else if (sType.equals(sTextFieldEnd))
            {
                Reference< ::com::sun::star::text::XFormField > xFormField(xPropSet->getPropertyValue(sBookmark), UNO_QUERY);

                if ( GetExport().getDefaultVersion() > SvtSaveOptions::ODFVER_012 )
                {
                    SvXMLElementExport aElem( GetExport(), !bAutoStyles,
                        XML_NAMESPACE_FIELD, XML_FIELDMARK_END,
                        sal_False, sal_False );
                }
                else
                {
                    if (xFormField.is())
                    {
                        OUString sName;
                        Reference< ::com::sun::star::container::XNameAccess > xParameters(xFormField->getParameters(), UNO_QUERY);
                        if (xParameters.is() && xParameters->hasByName("Name"))
                        {
                            const Any aValue = xParameters->getByName("Name");
                            aValue >>= sName;
                        }
                        if (sName.isEmpty())
                        {   
                            
                            sName = sFieldMarkName + OUString::number(
                                m_pImpl->GetFieldMarkIndex(xFormField));
                        }
                        GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_NAME,
                                sName);
                        SvXMLElementExport aElem( GetExport(), !bAutoStyles,
                            XML_NAMESPACE_TEXT, XML_BOOKMARK_END,
                            sal_False, sal_False );
                    }
                }
            }
            else if (sType.equals(sTextFieldStartEnd))
            {
                if ( GetExport().getDefaultVersion() > SvtSaveOptions::ODFVER_012 )
                {
                    Reference<XNamed> xBookmark(xPropSet->getPropertyValue(sBookmark), UNO_QUERY);
                    if (xBookmark.is())
                    {
                        GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_NAME, xBookmark->getName());
                    }
                    Reference< ::com::sun::star::text::XFormField > xFormField(xPropSet->getPropertyValue(sBookmark), UNO_QUERY);
                    if (xFormField.is())
                    {
                        GetExport().AddAttribute(XML_NAMESPACE_FIELD, XML_TYPE, xFormField->getFieldType());
                    }
                    GetExport().StartElement(XML_NAMESPACE_FIELD, XML_FIELDMARK, sal_False);
                    if (xFormField.is())
                    {
                        FieldParamExporter(&GetExport(), xFormField->getParameters()).Export();
                    }
                    GetExport().EndElement(XML_NAMESPACE_FIELD, XML_FIELDMARK, sal_False);
                }
                else
                {
                    Reference<XNamed> xBookmark(xPropSet->getPropertyValue(sBookmark), UNO_QUERY);
                    if (xBookmark.is())
                    {
                        GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_NAME, xBookmark->getName());
                        SvXMLElementExport aElem( GetExport(), !bAutoStyles,
                            XML_NAMESPACE_TEXT, XML_BOOKMARK,
                            sal_False, sal_False );
                    }
                }
            }
            else if (sType.equals(sSoftPageBreak))
            {
                exportSoftPageBreak(xPropSet,   bAutoStyles);
            }
            else {
                OSL_FAIL("unknown text portion type");
            }
        }
        else
        {
            Reference<XServiceInfo> xServiceInfo( xTxtRange, UNO_QUERY );
            if( xServiceInfo->supportsService( sTextFieldService ) )
            {
                exportTextField( xTxtRange, bAutoStyles, bIsProgress );
                bPrevCharIsSpace = false;
            }
            else
            {
                
                exportTextRange( xTxtRange, bAutoStyles, bPrevCharIsSpace, openFieldMark );
            }
        }
    }



}

void XMLTextParagraphExport::exportTable(
        const Reference < XTextContent > &,
        sal_Bool /*bAutoStyles*/, sal_Bool /*bIsProgress*/ )
{
}

void XMLTextParagraphExport::exportTextField(
        const Reference < XTextRange > & rTextRange,
        sal_Bool bAutoStyles, sal_Bool bIsProgress )
{
    Reference < XPropertySet > xPropSet( rTextRange, UNO_QUERY );
    
    if (xPropSet->getPropertySetInfo()->hasPropertyByName( sTextField ))
    {
        Reference < XTextField > xTxtFld(xPropSet->getPropertyValue( sTextField ), uno::UNO_QUERY);
        DBG_ASSERT( xTxtFld.is(), "text field missing" );
        if( xTxtFld.is() )
        {
            exportTextField(xTxtFld, bAutoStyles, bIsProgress, sal_True);
        }
        else
        {
            
            GetExport().Characters(rTextRange->getString());
        }
    }
}

void XMLTextParagraphExport::exportTextField(
        const Reference < XTextField > & xTextField,
        const sal_Bool bAutoStyles, const sal_Bool bIsProgress,
        const sal_Bool bRecursive )
{
    if ( bAutoStyles )
    {
        pFieldExport->ExportFieldAutoStyle( xTextField, bIsProgress,
                bRecursive );
    }
    else
    {
        pFieldExport->ExportField( xTextField, bIsProgress );
    }
}

void XMLTextParagraphExport::exportSoftPageBreak(
    const Reference<XPropertySet> & ,
    sal_Bool )
{
    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                              XML_SOFT_PAGE_BREAK, sal_False,
                              sal_False );
}

void XMLTextParagraphExport::exportTextMark(
    const Reference<XPropertySet> & rPropSet,
    const OUString& rProperty,
    const enum XMLTokenEnum pElements[],
    sal_Bool bAutoStyles)
{
    
    //
    
    
    
    
    

     if (!bAutoStyles)
    {
        
        Reference<XNamed> xName(rPropSet->getPropertyValue(rProperty), UNO_QUERY);
        GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_NAME,
                                 xName->getName());

        
        sal_Int8 nElement;
        if( *(sal_Bool *)rPropSet->getPropertyValue(sIsCollapsed).getValue() )
        {
            nElement = 0;
        }
        else
        {
            nElement = *(sal_Bool *)rPropSet->getPropertyValue(sIsStart).getValue() ? 1 : 2;
        }

        
        if( nElement < 2 ) {
            GetExport().AddAttributeXmlId(xName);
            const uno::Reference<text::XTextContent> xTextContent(
                    xName, uno::UNO_QUERY_THROW);
            GetExport().AddAttributesRDFa(xTextContent);
        }

        
        DBG_ASSERT(pElements != NULL, "illegal element array");
        DBG_ASSERT(nElement >= 0, "illegal element number");
        DBG_ASSERT(nElement <= 2, "illegal element number");
        SvXMLElementExport aElem(GetExport(),
                                 XML_NAMESPACE_TEXT, pElements[nElement],
                                 sal_False, sal_False);
    }
    
}

static sal_Bool lcl_txtpara_isBoundAsChar(
        const Reference < XPropertySet > & rPropSet,
        const Reference < XPropertySetInfo > & rPropSetInfo )
{
    sal_Bool bIsBoundAsChar = sal_False;
    OUString sAnchorType( "AnchorType"  );
    if( rPropSetInfo->hasPropertyByName( sAnchorType ) )
    {
        TextContentAnchorType eAnchor;
        rPropSet->getPropertyValue( sAnchorType ) >>= eAnchor;
        bIsBoundAsChar = TextContentAnchorType_AS_CHARACTER == eAnchor;
    }

    return bIsBoundAsChar;
}

sal_Int32 XMLTextParagraphExport::addTextFrameAttributes(
    const Reference < XPropertySet >& rPropSet,
    sal_Bool bShape,
    OUString *pMinHeightValue,
    OUString *pMinWidthValue)
{
    sal_Int32 nShapeFeatures = SEF_DEFAULT;

    
    
    if( !bShape )
    {
        Reference < XNamed > xNamed( rPropSet, UNO_QUERY );
        if( xNamed.is() )
        {
            OUString sName( xNamed->getName() );
            if( !sName.isEmpty() )
                GetExport().AddAttribute( XML_NAMESPACE_DRAW, XML_NAME,
                                          xNamed->getName() );
        }
    }

    OUStringBuffer sValue;

    
    TextContentAnchorType eAnchor = TextContentAnchorType_AT_PARAGRAPH;
    rPropSet->getPropertyValue( sAnchorType ) >>= eAnchor;
    {
        XMLAnchorTypePropHdl aAnchorTypeHdl;
        OUString sTmp;
        aAnchorTypeHdl.exportXML( sTmp, uno::makeAny(eAnchor),
                                  GetExport().GetMM100UnitConverter() );
        GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_ANCHOR_TYPE, sTmp );
    }

    
    if( TextContentAnchorType_AT_PAGE == eAnchor )
    {
        sal_Int16 nPage = 0;
        rPropSet->getPropertyValue( sAnchorPageNo ) >>= nPage;
        SAL_WARN_IF(nPage <= 0, "xmloff",
                "ERROR: writing invalid anchor-page-number 0");
        ::sax::Converter::convertNumber( sValue, (sal_Int32)nPage );
        GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_ANCHOR_PAGE_NUMBER,
                                  sValue.makeStringAndClear() );
    }
    else
    {
        
        nShapeFeatures |= SEF_EXPORT_NO_WS;
    }

    
    
    if ( !bShape &&
         eAnchor != TextContentAnchorType_AS_CHARACTER )
    {
        
        sal_Int16 nHoriOrient =  HoriOrientation::NONE;
        rPropSet->getPropertyValue( sHoriOrient ) >>= nHoriOrient;
        if( HoriOrientation::NONE == nHoriOrient )
        {
            sal_Int32 nPos = 0;
            rPropSet->getPropertyValue( sHoriOrientPosition ) >>= nPos;
            GetExport().GetMM100UnitConverter().convertMeasureToXML(
                    sValue, nPos );
            GetExport().AddAttribute( XML_NAMESPACE_SVG, XML_X,
                                      sValue.makeStringAndClear() );
        }
    }
    else if( TextContentAnchorType_AS_CHARACTER == eAnchor )
        nShapeFeatures = (nShapeFeatures & ~SEF_EXPORT_X);

    if( !bShape || TextContentAnchorType_AS_CHARACTER == eAnchor  )
    {
        
        sal_Int16 nVertOrient =  VertOrientation::NONE;
        rPropSet->getPropertyValue( sVertOrient ) >>= nVertOrient;
        if( VertOrientation::NONE == nVertOrient )
        {
            sal_Int32 nPos = 0;
            rPropSet->getPropertyValue( sVertOrientPosition ) >>= nPos;
            GetExport().GetMM100UnitConverter().convertMeasureToXML(
                    sValue, nPos );
            GetExport().AddAttribute( XML_NAMESPACE_SVG, XML_Y,
                                      sValue.makeStringAndClear() );
        }
        if( bShape )
            nShapeFeatures = (nShapeFeatures & ~SEF_EXPORT_Y);
    }

    Reference< XPropertySetInfo > xPropSetInfo(rPropSet->getPropertySetInfo());

    
    sal_Int16 nWidthType = SizeType::FIX;
    if( xPropSetInfo->hasPropertyByName( sWidthType ) )
    {
        rPropSet->getPropertyValue( sWidthType ) >>= nWidthType;
    }
    if( xPropSetInfo->hasPropertyByName( sWidth ) )
    {
        sal_Int32 nWidth =  0;
        
        if( SizeType::VARIABLE != nWidthType )
        {
            rPropSet->getPropertyValue( sWidth ) >>= nWidth;
        }
        GetExport().GetMM100UnitConverter().convertMeasureToXML(sValue, nWidth);
        if( SizeType::FIX != nWidthType )
        {
            assert(pMinWidthValue);
            if (pMinWidthValue)
            {
                *pMinWidthValue = sValue.makeStringAndClear();
            }
        }
        else
            GetExport().AddAttribute( XML_NAMESPACE_SVG, XML_WIDTH,
                                      sValue.makeStringAndClear() );
    }
    sal_Bool bSyncWidth = sal_False;
    if( xPropSetInfo->hasPropertyByName( sIsSyncWidthToHeight ) )
    {
        bSyncWidth = *(sal_Bool *)rPropSet->getPropertyValue( sIsSyncWidthToHeight ).getValue();
        if( bSyncWidth )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_REL_WIDTH,
                                      XML_SCALE );
    }
    if( !bSyncWidth && xPropSetInfo->hasPropertyByName( sRelativeWidth ) )
    {
        sal_Int16 nRelWidth =  0;
        rPropSet->getPropertyValue( sRelativeWidth ) >>= nRelWidth;
        DBG_ASSERT( nRelWidth >= 0 && nRelWidth <= 254,
                    "Got illegal relative width from API" );
        if( nRelWidth > 0 )
        {
            ::sax::Converter::convertPercent( sValue, nRelWidth );
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_REL_WIDTH,
                                      sValue.makeStringAndClear() );
        }
    }

    
    sal_Int16 nSizeType = SizeType::FIX;
    if( xPropSetInfo->hasPropertyByName( sSizeType ) )
    {
        rPropSet->getPropertyValue( sSizeType ) >>= nSizeType;
    }
    sal_Bool bSyncHeight = sal_False;
    if( xPropSetInfo->hasPropertyByName( sIsSyncHeightToWidth ) )
    {
        bSyncHeight = *(sal_Bool *)rPropSet->getPropertyValue( sIsSyncHeightToWidth ).getValue();
    }
    sal_Int16 nRelHeight =  0;
    if( !bSyncHeight && xPropSetInfo->hasPropertyByName( sRelativeHeight ) )
    {
        rPropSet->getPropertyValue( sRelativeHeight ) >>= nRelHeight;
    }
    if( xPropSetInfo->hasPropertyByName( sHeight ) )
    {
        sal_Int32 nHeight =  0;
        if( SizeType::VARIABLE != nSizeType )
        {
            rPropSet->getPropertyValue( sHeight ) >>= nHeight;
        }
        GetExport().GetMM100UnitConverter().convertMeasureToXML( sValue,
                                                            nHeight );
        if( SizeType::FIX != nSizeType && 0==nRelHeight && !bSyncHeight &&
             pMinHeightValue )
            *pMinHeightValue = sValue.makeStringAndClear();
        else
            GetExport().AddAttribute( XML_NAMESPACE_SVG, XML_HEIGHT,
                                      sValue.makeStringAndClear() );
    }
    if( bSyncHeight )
    {
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_REL_HEIGHT,
                SizeType::MIN == nSizeType ? XML_SCALE_MIN : XML_SCALE );

    }
    else if( nRelHeight > 0 )
    {
        ::sax::Converter::convertPercent( sValue, nRelHeight );
        if( SizeType::MIN == nSizeType )
        {
            assert(pMinHeightValue);
            if (pMinHeightValue)
            {
                *pMinHeightValue = sValue.makeStringAndClear();
            }
        }
        else
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_REL_HEIGHT,
                                      sValue.makeStringAndClear() );
    }

    OUString sZOrder( "ZOrder"  );
    if( xPropSetInfo->hasPropertyByName( sZOrder ) )
    {
        sal_Int32 nZIndex = 0;
        rPropSet->getPropertyValue( sZOrder ) >>= nZIndex;
        if( -1 != nZIndex )
        {
            ::sax::Converter::convertNumber( sValue, nZIndex );
            GetExport().AddAttribute( XML_NAMESPACE_DRAW, XML_ZINDEX,
                                      sValue.makeStringAndClear() );
        }
    }

    return nShapeFeatures;
}

void XMLTextParagraphExport::exportAnyTextFrame(
        const Reference < XTextContent > & rTxtCntnt,
        FrameType eType,
        sal_Bool bAutoStyles,
        sal_Bool bIsProgress,
        sal_Bool bExportContent,
        const Reference < XPropertySet > *pRangePropSet)
{
    Reference < XPropertySet > xPropSet( rTxtCntnt, UNO_QUERY );

    if( bAutoStyles )
    {
        if( FT_EMBEDDED == eType )
            _collectTextEmbeddedAutoStyles( xPropSet );
        
        else if ( FT_SHAPE != eType )
            Add( XML_STYLE_FAMILY_TEXT_FRAME, xPropSet );

        if( pRangePropSet && lcl_txtpara_isBoundAsChar( xPropSet,
                                            xPropSet->getPropertySetInfo() ) )
            Add( XML_STYLE_FAMILY_TEXT_TEXT, *pRangePropSet );

        switch( eType )
        {
        case FT_TEXT:
            {
                
                if ( bExportContent )
                {
                    Reference < XTextFrame > xTxtFrame( rTxtCntnt, UNO_QUERY );
                    Reference < XText > xTxt(xTxtFrame->getText());
                    exportFrameFrames( sal_True, bIsProgress, &xTxtFrame );
                    exportText( xTxt, bAutoStyles, bIsProgress, sal_True );
                }
            }
            break;
        case FT_SHAPE:
            {
                Reference < XShape > xShape( rTxtCntnt, UNO_QUERY );
                GetExport().GetShapeExport()->collectShapeAutoStyles( xShape );
            }
            break;
        default:
            break;
        }
    }
    else
    {
        Reference< XPropertySetInfo > xPropSetInfo(xPropSet->getPropertySetInfo());
        Reference< XPropertyState > xPropState( xPropSet, UNO_QUERY );
        {
            sal_Bool bAddCharStyles = pRangePropSet &&
                lcl_txtpara_isBoundAsChar( xPropSet, xPropSetInfo );

            sal_Bool bIsUICharStyle;
            sal_Bool bHasAutoStyle = sal_False;
            sal_Bool bDummy;

            OUString sStyle;

            if( bAddCharStyles )
                   sStyle = FindTextStyleAndHyperlink( *pRangePropSet, bDummy, bIsUICharStyle, bHasAutoStyle );
            else
                bIsUICharStyle = sal_False;

            XMLTextCharStyleNamesElementExport aCharStylesExport(
                GetExport(), bIsUICharStyle &&
                             aCharStyleNamesPropInfoCache.hasProperty(
                                            *pRangePropSet ), bHasAutoStyle,
                *pRangePropSet, sCharStyleNames );

            if( !sStyle.isEmpty() )
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_STYLE_NAME,
                                  GetExport().EncodeStyleName( sStyle ) );
            {
                SvXMLElementExport aElem( GetExport(), !sStyle.isEmpty(),
                    XML_NAMESPACE_TEXT, XML_SPAN, sal_False, sal_False );
                {
                    SvXMLElementExport aElement( GetExport(),
                        FT_SHAPE != eType &&
                        addHyperlinkAttributes( xPropSet,
                                                xPropState,xPropSetInfo ),
                        XML_NAMESPACE_DRAW, XML_A, sal_False, sal_False );
                    switch( eType )
                    {
                    case FT_TEXT:
                        _exportTextFrame( xPropSet, xPropSetInfo, bIsProgress );
                        break;
                    case FT_GRAPHIC:
                        _exportTextGraphic( xPropSet, xPropSetInfo );
                        break;
                    case FT_EMBEDDED:
                        _exportTextEmbedded( xPropSet, xPropSetInfo );
                        break;
                    case FT_SHAPE:
                        {
                            Reference < XShape > xShape( rTxtCntnt, UNO_QUERY );
                            sal_Int32 nFeatures =
                                addTextFrameAttributes( xPropSet, sal_True );
                            GetExport().GetShapeExport()
                                ->exportShape( xShape, nFeatures );
                        }
                        break;
                    }
                }
            }
        }
    }
}

void XMLTextParagraphExport::_exportTextFrame(
        const Reference < XPropertySet > & rPropSet,
        const Reference < XPropertySetInfo > & rPropSetInfo,
        sal_Bool bIsProgress )
{
    Reference < XTextFrame > xTxtFrame( rPropSet, UNO_QUERY );
    Reference < XText > xTxt(xTxtFrame->getText());

    OUString sStyle;
    if( rPropSetInfo->hasPropertyByName( sFrameStyleName ) )
    {
        rPropSet->getPropertyValue( sFrameStyleName ) >>= sStyle;
    }

    OUString sAutoStyle( sStyle );
    OUString aMinHeightValue;
    OUString sMinWidthValue;
    sAutoStyle = Find( XML_STYLE_FAMILY_TEXT_FRAME, rPropSet, sStyle );
    if( !sAutoStyle.isEmpty() )
        GetExport().AddAttribute( XML_NAMESPACE_DRAW, XML_STYLE_NAME,
                              GetExport().EncodeStyleName( sAutoStyle ) );
    addTextFrameAttributes(rPropSet, false, &aMinHeightValue, &sMinWidthValue);

    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_DRAW,
                              XML_FRAME, sal_False, sal_True );

    if( !aMinHeightValue.isEmpty() )
        GetExport().AddAttribute( XML_NAMESPACE_FO, XML_MIN_HEIGHT,
                                  aMinHeightValue );

    if (!sMinWidthValue.isEmpty())
    {
        GetExport().AddAttribute( XML_NAMESPACE_FO, XML_MIN_WIDTH,
                                  sMinWidthValue );
    }

    
    if( rPropSetInfo->hasPropertyByName( sChainNextName ) )
    {
        OUString sNext;
        if( (rPropSet->getPropertyValue( sChainNextName ) >>= sNext) && !sNext.isEmpty() )
            GetExport().AddAttribute( XML_NAMESPACE_DRAW,
                                      XML_CHAIN_NEXT_NAME,
                                      sNext );
    }

    {
        SvXMLElementExport aElement( GetExport(), XML_NAMESPACE_DRAW,
                                  XML_TEXT_BOX, sal_True, sal_True );

        
        exportFramesBoundToFrame( xTxtFrame, bIsProgress );

        exportText( xTxt, sal_False, bIsProgress, sal_True );
    }

    
    Reference<XEventsSupplier> xEventsSupp( xTxtFrame, UNO_QUERY );
    GetExport().GetEventExport().Export(xEventsSupp);

    
    GetExport().GetImageMapExport().Export( rPropSet );

    
    exportTitleAndDescription( rPropSet, rPropSetInfo );
}

void XMLTextParagraphExport::exportContour(
    const Reference < XPropertySet > & rPropSet,
    const Reference < XPropertySetInfo > & rPropSetInfo )
{
    if( !rPropSetInfo->hasPropertyByName( sContourPolyPolygon ) )
    {
        return;
    }

    PointSequenceSequence aSourcePolyPolygon;
    rPropSet->getPropertyValue( sContourPolyPolygon ) >>= aSourcePolyPolygon;
    const basegfx::B2DPolyPolygon aPolyPolygon(
        basegfx::tools::UnoPointSequenceSequenceToB2DPolyPolygon(
            aSourcePolyPolygon));
    const sal_uInt32 nPolygonCount(aPolyPolygon.count());

    if(!nPolygonCount)
    {
        return;
    }

    const basegfx::B2DRange aPolyPolygonRange(aPolyPolygon.getB2DRange());
    bool bPixel(false);

    if( rPropSetInfo->hasPropertyByName( sIsPixelContour ) )
    {
        bPixel = *(sal_Bool *)rPropSet->getPropertyValue( sIsPixelContour ).getValue();
    }

    
    OUStringBuffer aStringBuffer( 10 );

    if(bPixel)
    {
        ::sax::Converter::convertMeasurePx(aStringBuffer, basegfx::fround(aPolyPolygonRange.getWidth()));
    }
    else
    {
        GetExport().GetMM100UnitConverter().convertMeasureToXML(aStringBuffer, basegfx::fround(aPolyPolygonRange.getWidth()));
    }

    GetExport().AddAttribute(XML_NAMESPACE_SVG, XML_WIDTH, aStringBuffer.makeStringAndClear());

    
    if(bPixel)
    {
        ::sax::Converter::convertMeasurePx(aStringBuffer, basegfx::fround(aPolyPolygonRange.getHeight()));
    }
    else
    {
        GetExport().GetMM100UnitConverter().convertMeasureToXML(aStringBuffer, basegfx::fround(aPolyPolygonRange.getHeight()));
    }

    GetExport().AddAttribute(XML_NAMESPACE_SVG, XML_HEIGHT, aStringBuffer.makeStringAndClear());

    
    SdXMLImExViewBox aViewBox(0.0, 0.0, aPolyPolygonRange.getWidth(), aPolyPolygonRange.getHeight());
    GetExport().AddAttribute(XML_NAMESPACE_SVG, XML_VIEWBOX, aViewBox.GetExportString());
    enum XMLTokenEnum eElem = XML_TOKEN_INVALID;

    if(1 == nPolygonCount )
    {
        
        const OUString aPointString(
            basegfx::tools::exportToSvgPoints(
                aPolyPolygon.getB2DPolygon(0)));

        
        GetExport().AddAttribute(XML_NAMESPACE_DRAW, XML_POINTS, aPointString);
        eElem = XML_CONTOUR_POLYGON;
    }
    else
    {
        
        const OUString aPolygonString(
            basegfx::tools::exportToSvgD(
                aPolyPolygon,
                true,           
                false,          
                true));         

        
        GetExport().AddAttribute( XML_NAMESPACE_SVG, XML_D, aPolygonString);
        eElem = XML_CONTOUR_PATH;
    }

    if( rPropSetInfo->hasPropertyByName( sIsAutomaticContour ) )
    {
        sal_Bool bTmp = *(sal_Bool *)rPropSet->getPropertyValue(
                                            sIsAutomaticContour ).getValue();
        GetExport().AddAttribute( XML_NAMESPACE_DRAW,
                      XML_RECREATE_ON_EDIT, bTmp ? XML_TRUE : XML_FALSE );
    }

    
    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_DRAW, eElem,
                              sal_True, sal_True );
}

void XMLTextParagraphExport::_exportTextGraphic(
        const Reference < XPropertySet > & rPropSet,
        const Reference < XPropertySetInfo > & rPropSetInfo )
{
    OUString sStyle;
    if( rPropSetInfo->hasPropertyByName( sFrameStyleName ) )
    {
        rPropSet->getPropertyValue( sFrameStyleName ) >>= sStyle;
    }

    OUString sAutoStyle( sStyle );
    sAutoStyle = Find( XML_STYLE_FAMILY_TEXT_FRAME, rPropSet, sStyle );
    if( !sAutoStyle.isEmpty() )
        GetExport().AddAttribute( XML_NAMESPACE_DRAW, XML_STYLE_NAME,
                                  GetExport().EncodeStyleName( sAutoStyle ) );
    addTextFrameAttributes( rPropSet, sal_False );

    
    sal_Int16 nVal = 0;
    rPropSet->getPropertyValue( sGraphicRotation ) >>= nVal;
    if( nVal != 0 )
    {
        OUStringBuffer sRet( GetXMLToken(XML_ROTATE).getLength()+4 );
        sRet.append( GetXMLToken(XML_ROTATE));
        sRet.append( '(' );
        ::sax::Converter::convertNumber( sRet, (sal_Int32)nVal );
        sRet.append( ')' );
        GetExport().AddAttribute( XML_NAMESPACE_SVG, XML_TRANSFORM,
                                  sRet.makeStringAndClear() );
    }

    
    SvXMLElementExport aElem(GetExport(), XML_NAMESPACE_DRAW, XML_FRAME, sal_False, sal_True);

    
    
    OUString sReplacementOrigURL;
    rPropSet->getPropertyValue( sReplacementGraphicURL ) >>= sReplacementOrigURL;

    
    OUString sOrigURL;
    rPropSet->getPropertyValue( sGraphicURL ) >>= sOrigURL;
    OUString sURL(GetExport().AddEmbeddedGraphicObject( sOrigURL ));
    setTextEmbeddedGraphicURL( rPropSet, sURL );

    
    if( !sURL.isEmpty() )
    {
        GetExport().AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, sURL );
        GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
        GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE,
                                                       XML_ONLOAD );
    }

    
    OUString sGrfFilter;
    rPropSet->getPropertyValue( sGraphicFilter ) >>= sGrfFilter;
    if( !sGrfFilter.isEmpty() )
        GetExport().AddAttribute( XML_NAMESPACE_DRAW, XML_FILTER_NAME,
                                  sGrfFilter );

    {
        SvXMLElementExport aElement( GetExport(), XML_NAMESPACE_DRAW,
                                  XML_IMAGE, sal_False, sal_True );

        
        GetExport().AddEmbeddedGraphicObjectAsBase64( sOrigURL );
    }

    
    
    if (!sReplacementOrigURL.isEmpty())
    {
        const OUString sReplacementURL(GetExport().AddEmbeddedGraphicObject( sReplacementOrigURL ));

        
        if(sReplacementURL.getLength())
        {
            GetExport().AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, sReplacementURL);
            GetExport().AddAttribute(XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE);
            GetExport().AddAttribute(XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED);
            GetExport().AddAttribute(XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD);

            
            SvXMLElementExport aElement(GetExport(), XML_NAMESPACE_DRAW, XML_IMAGE, sal_False, sal_True);

            
            GetExport().AddEmbeddedGraphicObjectAsBase64(sReplacementURL);
        }
    }



    
    Reference<XEventsSupplier> xEventsSupp( rPropSet, UNO_QUERY );
    GetExport().GetEventExport().Export(xEventsSupp);

    
    GetExport().GetImageMapExport().Export( rPropSet );

    
    exportTitleAndDescription( rPropSet, rPropSetInfo );

    
    exportContour( rPropSet, rPropSetInfo );
}

void XMLTextParagraphExport::_collectTextEmbeddedAutoStyles(const Reference < XPropertySet > & )
{
    DBG_ASSERT( !this, "no API implementation avialable" );
}

void XMLTextParagraphExport::_exportTextEmbedded(
        const Reference < XPropertySet > &,
        const Reference < XPropertySetInfo > & )
{
    DBG_ASSERT( !this, "no API implementation avialable" );
}

void XMLTextParagraphExport::exportEvents( const Reference < XPropertySet > & rPropSet )
{
    
    Reference<XEventsSupplier> xEventsSupp( rPropSet, UNO_QUERY );
    GetExport().GetEventExport().Export(xEventsSupp);

    
    OUString sImageMap("ImageMap");
    if (rPropSet->getPropertySetInfo()->hasPropertyByName(sImageMap))
        GetExport().GetImageMapExport().Export( rPropSet );
}


void XMLTextParagraphExport::exportTitleAndDescription(
        const Reference < XPropertySet > & rPropSet,
        const Reference < XPropertySetInfo > & rPropSetInfo )
{
    
    if( rPropSetInfo->hasPropertyByName( sTitle ) )
    {
        OUString sObjTitle;
        rPropSet->getPropertyValue( sTitle ) >>= sObjTitle;
        if( !sObjTitle.isEmpty() )
        {
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_SVG,
                                      XML_TITLE, sal_True, sal_False );
            GetExport().Characters( sObjTitle );
        }
    }

    
    if( rPropSetInfo->hasPropertyByName( sDescription ) )
    {
        OUString sObjDesc;
        rPropSet->getPropertyValue( sDescription ) >>= sObjDesc;
        if( !sObjDesc.isEmpty() )
        {
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_SVG,
                                      XML_DESC, sal_True, sal_False );
            GetExport().Characters( sObjDesc );
        }
    }
}

void XMLTextParagraphExport::setTextEmbeddedGraphicURL(
    const Reference < XPropertySet >&,
    OUString& /*rStreamName*/ ) const
{
}

sal_Bool XMLTextParagraphExport::addHyperlinkAttributes(
    const Reference< XPropertySet > & rPropSet,
    const Reference< XPropertyState > & rPropState,
    const Reference< XPropertySetInfo > & rPropSetInfo )
{
    sal_Bool bExport = sal_False;
    OUString sHRef, sName, sTargetFrame, sUStyleName, sVStyleName;
    sal_Bool bServerMap = sal_False;

    if( rPropSetInfo->hasPropertyByName( sHyperLinkURL ) &&
        ( !rPropState.is() || PropertyState_DIRECT_VALUE ==
                    rPropState->getPropertyState( sHyperLinkURL ) ) )
    {
        rPropSet->getPropertyValue( sHyperLinkURL ) >>= sHRef;

        if( !sHRef.isEmpty() )
            bExport = sal_True;
    }

    if ( sHRef.isEmpty() )
    {
        
        OSL_ENSURE( false, "hyperlink without an URL --> no export to ODF" );
        return sal_False;
    }

    if ( rPropSetInfo->hasPropertyByName( sHyperLinkName )
         && ( !rPropState.is()
              || PropertyState_DIRECT_VALUE == rPropState->getPropertyState( sHyperLinkName ) ) )
    {
        rPropSet->getPropertyValue( sHyperLinkName ) >>= sName;
        if( !sName.isEmpty() )
            bExport = sal_True;
    }

    if ( rPropSetInfo->hasPropertyByName( sHyperLinkTarget )
         && ( !rPropState.is()
              || PropertyState_DIRECT_VALUE == rPropState->getPropertyState( sHyperLinkTarget ) ) )
    {
        rPropSet->getPropertyValue( sHyperLinkTarget ) >>= sTargetFrame;
        if( !sTargetFrame.isEmpty() )
            bExport = sal_True;
    }

    if ( rPropSetInfo->hasPropertyByName( sServerMap )
         && ( !rPropState.is()
              || PropertyState_DIRECT_VALUE == rPropState->getPropertyState( sServerMap ) ) )
    {
        bServerMap = *(sal_Bool *) rPropSet->getPropertyValue( sServerMap ).getValue();
        if ( bServerMap )
            bExport = sal_True;
    }

    if ( rPropSetInfo->hasPropertyByName( sUnvisitedCharStyleName )
         && ( !rPropState.is()
              || PropertyState_DIRECT_VALUE == rPropState->getPropertyState( sUnvisitedCharStyleName ) ) )
    {
        rPropSet->getPropertyValue( sUnvisitedCharStyleName ) >>= sUStyleName;
        if( !sUStyleName.isEmpty() )
            bExport = sal_True;
    }

    if ( rPropSetInfo->hasPropertyByName( sVisitedCharStyleName )
         && ( !rPropState.is()
              || PropertyState_DIRECT_VALUE == rPropState->getPropertyState( sVisitedCharStyleName ) ) )
    {
        rPropSet->getPropertyValue( sVisitedCharStyleName ) >>= sVStyleName;
        if( !sVStyleName.isEmpty() )
            bExport = sal_True;
    }

    if ( bExport )
    {
        GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, GetExport().GetRelativeReference( sHRef ) );

        if( !sName.isEmpty() )
            GetExport().AddAttribute( XML_NAMESPACE_OFFICE, XML_NAME, sName );

        if( !sTargetFrame.isEmpty() )
        {
            GetExport().AddAttribute( XML_NAMESPACE_OFFICE,
                                      XML_TARGET_FRAME_NAME, sTargetFrame );
            enum XMLTokenEnum eTok = sTargetFrame == "_blank" ? XML_NEW : XML_REPLACE;
            GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, eTok );
        }

        if( bServerMap  )
            GetExport().AddAttribute( XML_NAMESPACE_OFFICE,
                                      XML_SERVER_MAP, XML_TRUE );

        if( !sUStyleName.isEmpty() )
            GetExport().AddAttribute( XML_NAMESPACE_TEXT,
              XML_STYLE_NAME, GetExport().EncodeStyleName( sUStyleName ) );

        if( !sVStyleName.isEmpty() )
            GetExport().AddAttribute( XML_NAMESPACE_TEXT,
              XML_VISITED_STYLE_NAME, GetExport().EncodeStyleName( sVStyleName ) );
    }

    return bExport;
}

void XMLTextParagraphExport::exportTextRangeSpan(
    const com::sun::star::uno::Reference< com::sun::star::text::XTextRange > & rTextRange,
    Reference< XPropertySet > & xPropSet,
    Reference < XPropertySetInfo > & xPropSetInfo,
    const sal_Bool bIsUICharStyle,
    const sal_Bool bHasAutoStyle,
    const OUString& sStyle,
    bool& rPrevCharIsSpace,
    FieldmarkType& openFieldMark )
{
    XMLTextCharStyleNamesElementExport aCharStylesExport(
            GetExport(),
            bIsUICharStyle && aCharStyleNamesPropInfoCache.hasProperty( xPropSet, xPropSetInfo ),
            bHasAutoStyle,
            xPropSet,
            sCharStyleNames );

    if ( !sStyle.isEmpty() )
    {
        GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_STYLE_NAME, GetExport().EncodeStyleName( sStyle ) );
    }
    {
        SvXMLElementExport aElement( GetExport(), !sStyle.isEmpty(), XML_NAMESPACE_TEXT, XML_SPAN, sal_False, sal_False );
        const OUString aText( rTextRange->getString() );
        SvXMLElementExport aElem2( GetExport(), TEXT == openFieldMark,
            XML_NAMESPACE_TEXT, XML_TEXT_INPUT,
            sal_False, sal_False );
        exportText( aText, rPrevCharIsSpace );
        openFieldMark = NONE;
    }
}

void XMLTextParagraphExport::exportTextRange(
        const Reference< XTextRange > & rTextRange,
        sal_Bool bAutoStyles,
        bool& rPrevCharIsSpace,
        FieldmarkType& openFieldMark )
{
    Reference< XPropertySet > xPropSet( rTextRange, UNO_QUERY );
    if ( bAutoStyles )
    {
        Add( XML_STYLE_FAMILY_TEXT_TEXT, xPropSet );
    }
    else
    {
        sal_Bool bHyperlink = sal_False;
        sal_Bool bIsUICharStyle = sal_False;
        sal_Bool bHasAutoStyle = sal_False;
        const OUString sStyle(
            FindTextStyleAndHyperlink( xPropSet, bHyperlink, bIsUICharStyle, bHasAutoStyle ) );

        Reference < XPropertySetInfo > xPropSetInfo;
        sal_Bool bHyperlinkAttrsAdded = sal_False;
        if ( bHyperlink )
        {
            Reference< XPropertyState > xPropState( xPropSet, UNO_QUERY );
            xPropSetInfo.set( xPropSet->getPropertySetInfo() );
            bHyperlinkAttrsAdded = addHyperlinkAttributes( xPropSet, xPropState, xPropSetInfo );
        }

        if ( bHyperlink && bHyperlinkAttrsAdded )
        {
            SvXMLElementExport aElem( GetExport(), sal_True, XML_NAMESPACE_TEXT, XML_A, sal_False, sal_False );

            
            OUString sHyperLinkEvents(
                "HyperLinkEvents");
            if (xPropSetInfo->hasPropertyByName(sHyperLinkEvents))
            {
                Reference< XNameReplace > xName( xPropSet->getPropertyValue( sHyperLinkEvents ), uno::UNO_QUERY );
                GetExport().GetEventExport().Export( xName, sal_False );
            }

            exportTextRangeSpan( rTextRange, xPropSet, xPropSetInfo, bIsUICharStyle, bHasAutoStyle, sStyle, rPrevCharIsSpace, openFieldMark );
        }
        else
        {
            exportTextRangeSpan( rTextRange, xPropSet, xPropSetInfo, bIsUICharStyle, bHasAutoStyle, sStyle, rPrevCharIsSpace, openFieldMark );
        }
    }
}

void XMLTextParagraphExport::exportText( const OUString& rText,
                                           bool& rPrevCharIsSpace )
{
    sal_Int32 nExpStartPos = 0;
    sal_Int32 nEndPos = rText.getLength();
    sal_Int32 nSpaceChars = 0;
    for( sal_Int32 nPos = 0; nPos < nEndPos; nPos++ )
    {
        sal_Unicode cChar = rText[nPos];
        sal_Bool bExpCharAsText = sal_True;
        sal_Bool bExpCharAsElement = sal_False;
        sal_Bool bCurrCharIsSpace = sal_False;
        switch( cChar )
        {
        case 0x0009:    
        case 0x000A:    
            
            bExpCharAsElement = sal_True;
            bExpCharAsText = sal_False;
            break;
        case 0x000D:
            break;  
        case 0x0020:    
            if( rPrevCharIsSpace )
            {
                
                
                bExpCharAsText = sal_False;
            }
            bCurrCharIsSpace = sal_True;
            break;
        default:
            if( cChar < 0x0020 )
            {
#ifdef DBG_UTIL
                OSL_ENSURE( txtparae_bContainsIllegalCharacters ||
                            cChar >= 0x0020,
                            "illegal character in text content" );
                txtparae_bContainsIllegalCharacters = sal_True;
#endif
                bExpCharAsText = sal_False;
            }
            break;
        }

        
           
        if( nPos > nExpStartPos && !bExpCharAsText )
        {
            DBG_ASSERT( 0==nSpaceChars, "pending spaces" );
            OUString sExp( rText.copy( nExpStartPos, nPos - nExpStartPos ) );
            GetExport().Characters( sExp );
            nExpStartPos = nPos;
        }

        
        
        
        if( nSpaceChars > 0 && !bCurrCharIsSpace )
        {
            DBG_ASSERT( nExpStartPos == nPos, " pending characters" );

            if( nSpaceChars > 1 )
            {
                OUStringBuffer sTmp;
                sTmp.append( (sal_Int32)nSpaceChars );
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_C,
                              sTmp.makeStringAndClear() );
            }

            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                      XML_S, sal_False, sal_False );

            nSpaceChars = 0;
        }

        
        
        if( bExpCharAsElement )
        {
            switch( cChar )
            {
            case 0x0009:    
                {
                    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                              XML_TAB, sal_False,
                                              sal_False );
                }
                break;
            case 0x000A:    
                {
                    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                              XML_LINE_BREAK, sal_False,
                                              sal_False );
                }
                break;
            }
        }

        
        
        
        if( bCurrCharIsSpace && rPrevCharIsSpace )
            nSpaceChars++;
        rPrevCharIsSpace = bCurrCharIsSpace;

        
        
        if( !bExpCharAsText )
        {
            DBG_ASSERT( nExpStartPos == nPos, "wrong export start pos" );
            nExpStartPos = nPos+1;
        }
    }

    if( nExpStartPos < nEndPos )
    {
        DBG_ASSERT( 0==nSpaceChars, " pending spaces " );
        OUString sExp( rText.copy( nExpStartPos, nEndPos - nExpStartPos ) );
        GetExport().Characters( sExp );
    }

    
    if( nSpaceChars > 0 )
    {
        if( nSpaceChars > 1 )
        {
            OUStringBuffer sTmp;
            sTmp.append( (sal_Int32)nSpaceChars );
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_C,
                          sTmp.makeStringAndClear() );
        }

        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT, XML_S,
                                  sal_False, sal_False );
    }
}

void XMLTextParagraphExport::exportTextDeclarations()
{
    pFieldExport->ExportFieldDeclarations();

    
    
    Reference<XPropertySet> xPropertySet( GetExport().GetModel(), UNO_QUERY );
    if (xPropertySet.is())
    {
        OUString sUrl;
        OUString sIndexAutoMarkFileURL(
            "IndexAutoMarkFileURL");
        if (xPropertySet->getPropertySetInfo()->hasPropertyByName(
            sIndexAutoMarkFileURL))
        {
            xPropertySet->getPropertyValue(sIndexAutoMarkFileURL) >>= sUrl;
            if (!sUrl.isEmpty())
            {
                GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_HREF,
                                          GetExport().GetRelativeReference(sUrl) );
                SvXMLElementExport aAutoMarkElement(
                    GetExport(), XML_NAMESPACE_TEXT,
                    XML_ALPHABETICAL_INDEX_AUTO_MARK_FILE,
                    sal_True, sal_True );
            }
        }
    }
}

void XMLTextParagraphExport::exportTextDeclarations(
    const Reference<XText> & rText )
{
    pFieldExport->ExportFieldDeclarations(rText);
}

void XMLTextParagraphExport::exportUsedDeclarations( sal_Bool bOnlyUsed )
{
    pFieldExport->SetExportOnlyUsedFieldDeclarations( bOnlyUsed );
}

void XMLTextParagraphExport::exportTrackedChanges(sal_Bool bAutoStyles)
{
    if (NULL != pRedlineExport)
        pRedlineExport->ExportChangesList( bAutoStyles );
}

void XMLTextParagraphExport::exportTrackedChanges(
    const Reference<XText> & rText,
    sal_Bool bAutoStyle)
{
    if (NULL != pRedlineExport)
        pRedlineExport->ExportChangesList(rText, bAutoStyle);
}

void XMLTextParagraphExport::recordTrackedChangesForXText(
    const Reference<XText> & rText )
{
    if (NULL != pRedlineExport)
        pRedlineExport->SetCurrentXText(rText);
}

void XMLTextParagraphExport::recordTrackedChangesNoXText()
{
    if (NULL != pRedlineExport)
        pRedlineExport->SetCurrentXText();
}

void XMLTextParagraphExport::exportTextAutoStyles()
{
    GetAutoStylePool().exportXML( XML_STYLE_FAMILY_TEXT_PARAGRAPH,
                                   GetExport().GetDocHandler(),
                                   GetExport().GetMM100UnitConverter(),
                                   GetExport().GetNamespaceMap() );

    GetAutoStylePool().exportXML( XML_STYLE_FAMILY_TEXT_TEXT,
                                   GetExport().GetDocHandler(),
                                   GetExport().GetMM100UnitConverter(),
                                   GetExport().GetNamespaceMap() );

    GetAutoStylePool().exportXML( XML_STYLE_FAMILY_TEXT_FRAME,
                                   GetExport().GetDocHandler(),
                                   GetExport().GetMM100UnitConverter(),
                                   GetExport().GetNamespaceMap() );

    GetAutoStylePool().exportXML( XML_STYLE_FAMILY_TEXT_SECTION,
                                  GetExport().GetDocHandler(),
                                  GetExport().GetMM100UnitConverter(),
                                  GetExport().GetNamespaceMap() );

    GetAutoStylePool().exportXML( XML_STYLE_FAMILY_TEXT_RUBY,
                                  GetExport().GetDocHandler(),
                                  GetExport().GetMM100UnitConverter(),
                                  GetExport().GetNamespaceMap() );

    pListAutoPool->exportXML();
}

void XMLTextParagraphExport::exportRuby(
    const Reference<XPropertySet> & rPropSet,
    sal_Bool bAutoStyles )
{
    
    if (*(sal_Bool*)rPropSet->getPropertyValue(sIsCollapsed).getValue())
        return;

    
    sal_Bool bStart = (*(sal_Bool*)rPropSet->getPropertyValue(sIsStart).getValue());

    if (bAutoStyles)
    {
        
        if (bStart)
            Add( XML_STYLE_FAMILY_TEXT_RUBY, rPropSet );
    }
    else
    {
        if (bStart)
        {
            

            
            DBG_ASSERT(! bOpenRuby, "Can't open a ruby inside of ruby!");
            if( bOpenRuby )
                return;

            
            rPropSet->getPropertyValue(sRubyText) >>= sOpenRubyText;
            rPropSet->getPropertyValue(sRubyCharStyleName) >>= sOpenRubyCharStyle;

            
            GetExport().CheckAttrList();
            OUString sEmpty;
            OUString sStyleName(Find( XML_STYLE_FAMILY_TEXT_RUBY, rPropSet,
                                        sEmpty ));
            DBG_ASSERT(!sStyleName.isEmpty(), "I can't find the style!");
            GetExport().AddAttribute(XML_NAMESPACE_TEXT,
                                     XML_STYLE_NAME, sStyleName);

            
            GetExport().StartElement( XML_NAMESPACE_TEXT, XML_RUBY, sal_False);
            GetExport().ClearAttrList();
            GetExport().StartElement( XML_NAMESPACE_TEXT, XML_RUBY_BASE,
                                      sal_False );
            bOpenRuby = sal_True;
        }
        else
        {
            

            
            DBG_ASSERT(bOpenRuby, "Can't close a ruby if none is open!");
            if( !bOpenRuby )
                return;

            
            GetExport().EndElement(XML_NAMESPACE_TEXT, XML_RUBY_BASE,
                                   sal_False);

            
            {
                if (!sOpenRubyCharStyle.isEmpty())
                    GetExport().AddAttribute(
                        XML_NAMESPACE_TEXT, XML_STYLE_NAME,
                        GetExport().EncodeStyleName( sOpenRubyCharStyle) );

                SvXMLElementExport aRubyElement(
                    GetExport(), XML_NAMESPACE_TEXT, XML_RUBY_TEXT,
                    sal_False, sal_False);

                GetExport().Characters(sOpenRubyText);
            }

            
            GetExport().EndElement(XML_NAMESPACE_TEXT, XML_RUBY, sal_False);
            bOpenRuby = sal_False;
        }
    }
}

void XMLTextParagraphExport::exportMeta(
    const Reference<XPropertySet> & i_xPortion,
    sal_Bool i_bAutoStyles, sal_Bool i_isProgress)
{
    static OUString sMeta("InContentMetadata");

    bool doExport(!i_bAutoStyles); 
    
    switch (GetExport().getDefaultVersion()) {
        case SvtSaveOptions::ODFVER_011: 
        case SvtSaveOptions::ODFVER_010: doExport = false; break;
        default: break;
    }

    const Reference< XTextContent > xTextContent(
            i_xPortion->getPropertyValue(sMeta), UNO_QUERY_THROW);
    const Reference< XEnumerationAccess > xEA( xTextContent, UNO_QUERY_THROW );
    const Reference< XEnumeration > xTextEnum( xEA->createEnumeration() );

    if (doExport)
    {
        const Reference<rdf::XMetadatable> xMeta(xTextContent, UNO_QUERY_THROW);

        
        xMeta->ensureMetadataReference();

        
        GetExport().AddAttributeXmlId(xMeta);
        GetExport().AddAttributesRDFa(xTextContent);
    }

    SvXMLElementExport aElem( GetExport(), doExport,
        XML_NAMESPACE_TEXT, XML_META, sal_False, sal_False );

    
    exportTextRangeEnumeration( xTextEnum, i_bAutoStyles, i_isProgress );
}

void XMLTextParagraphExport::PreventExportOfControlsInMuteSections(
    const Reference<XIndexAccess> & rShapes,
    UniReference<xmloff::OFormLayerXMLExport> xFormExport   )
{
    
    if( ( ! rShapes.is() ) || ( ! xFormExport.is() ) )
    {
        
        return;
    }
    DBG_ASSERT( pSectionExport != NULL, "We need the section export." );

    Reference<XEnumeration> xShapesEnum = pBoundFrameSets->GetShapes()->createEnumeration();
    if(!xShapesEnum.is())
        return;
    while( xShapesEnum->hasMoreElements() )
    {
        
        
        
        

        
        Reference<XControlShape> xControlShape(xShapesEnum->nextElement(), UNO_QUERY);
        if( xControlShape.is() )
        {
            
            
            

            Reference<XTextContent> xTextContent( xControlShape, UNO_QUERY );
            if( xTextContent.is() )
            {
                if( pSectionExport->IsMuteSection( xTextContent, sal_False ) )
                {
                    
                    
                    
                    
                    xFormExport->excludeFromExport(
                        xControlShape->getControl() );
                }
                
                
            }
            
        }
        
    }
}

void XMLTextParagraphExport::PushNewTextListsHelper()
{
    mpTextListsHelper = new XMLTextListsHelper();
    maTextListsHelperStack.push_back( mpTextListsHelper );
}

void XMLTextParagraphExport::PopTextListsHelper()
{
    delete mpTextListsHelper;
    mpTextListsHelper = 0;
    maTextListsHelperStack.pop_back();
    if ( !maTextListsHelperStack.empty() )
    {
        mpTextListsHelper = maTextListsHelperStack.back();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
