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

#include <sal/config.h>

#include <o3tl/any.hxx>
#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <tools/debug.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/types.h>
#include <sal/log.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
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
#include <xexptran.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlexp.hxx>
#include <txtflde.hxx>
#include <xmloff/txtprmap.hxx>
#include <XMLImageMapExport.hxx>
#include "XMLTextNumRuleInfo.hxx"
#include <xmloff/XMLTextListAutoStylePool.hxx>
#include <xmloff/txtparae.hxx>
#include "XMLSectionExport.hxx"
#include "XMLIndexMarkExport.hxx"
#include <xmloff/XMLEventExport.hxx>
#include "XMLRedlineExport.hxx"
#include <MultiPropertySetHelper.hxx>
#include <xmloff/formlayerexport.hxx>
#include "XMLTextCharStyleNamesElementExport.hxx"
#include <xmloff/odffields.hxx>
#include <xmloff/maptype.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <txtlists.hxx>
#include <com/sun/star/rdf/XMetadatable.hpp>
#include <list>
#include <unordered_map>
#include <memory>
#include <vector>
#include <algorithm>
#include <iterator>
#include <officecfg/Office/Common.hxx>

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
using namespace ::com::sun::star::graphic;
using namespace ::xmloff;
using namespace ::xmloff::token;

namespace
{
    class TextContentSet
    {
        public:
            typedef list<Reference<XTextContent>> contents_t;
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
    {
        size_t operator()(const Reference<XTextFrame>& rFrame) const
            { return sal::static_int_cast<size_t>(reinterpret_cast<sal_uIntPtr>(rFrame.get())); }
    };

    bool lcl_TextContentsUnfiltered(const Reference<XTextContent>&)
        { return true; };

    bool lcl_ShapeFilter(const Reference<XTextContent>& xTxtContent)
    {
        Reference<XShape> xShape(xTxtContent, UNO_QUERY);
        if(!xShape.is())
            return false;
        Reference<XServiceInfo> xServiceInfo(xTxtContent, UNO_QUERY);
        return !xServiceInfo->supportsService("com.sun.star.text.TextFrame") &&
               !xServiceInfo->supportsService("com.sun.star.text.TextGraphicObject") &&
               !xServiceInfo->supportsService("com.sun.star.text.TextEmbeddedObject");
    };

    class BoundFrames
    {
        public:
            typedef bool (*filter_t)(const Reference<XTextContent>&);
            BoundFrames(
                const Reference<XEnumerationAccess>& rEnumAccess,
                const filter_t& rFilter)
                : m_xEnumAccess(rEnumAccess)
            {
                Fill(rFilter);
            };
            BoundFrames()
                {};
            const TextContentSet& GetPageBoundContents() const
                { return m_vPageBounds; };
            const TextContentSet* GetFrameBoundContents(const Reference<XTextFrame>& rParentFrame) const
            {
                framebound_map_t::const_iterator it = m_vFrameBoundsOf.find(rParentFrame);
                if(it == m_vFrameBoundsOf.end())
                    return nullptr;
                return &(it->second);
            };
            Reference<XEnumeration> createEnumeration() const
            {
                if(!m_xEnumAccess.is())
                    return Reference<XEnumeration>();
                return m_xEnumAccess->createEnumeration();
            };

        private:
            typedef std::unordered_map<
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
            FieldParamExporter(SvXMLExport* const pExport, Reference<XNameContainer> const & xFieldParams)
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
            explicit BoundFrameSets(const Reference<XInterface>& rModel);
            const BoundFrames* GetTexts() const
                { return m_pTexts.get(); };
            const BoundFrames* GetGraphics() const
                { return m_pGraphics.get(); };
            const BoundFrames* GetEmbeddeds() const
                { return m_pEmbeddeds.get(); };
            const BoundFrames* GetShapes() const
                { return m_pShapes.get(); };
        private:
            unique_ptr<BoundFrames> m_pTexts;
            unique_ptr<BoundFrames> m_pGraphics;
            unique_ptr<BoundFrames> m_pEmbeddeds;
            unique_ptr<BoundFrames> m_pShapes;
    };
}

#ifdef DBG_UTIL
static bool txtparae_bContainsIllegalCharacters = false;
#endif

// The following map shows which property values are required:

// property                     auto style pass     export

// ParaStyleName                if style exists     always
// ParaConditionalStyleName     if style exists     always
// NumberingRules               if style exists     always
// TextSection                  always              always
// ParaChapterNumberingLevel    never               always
// NumberingIsNumber            never               always

// The conclusion is that for auto styles the first three properties
// should be queried using a multi property set if, and only if, an
// auto style needs to be exported. TextSection should be queried by
// an individual call to getPropertyvalue, because this seems to be
// less expensive than querying the first three properties if they aren't
// required.

// For the export pass all properties can be queried using a multi property
// set.

static const sal_Char* aParagraphPropertyNamesAuto[] =
{
    "NumberingRules",
    "ParaConditionalStyleName",
    "ParaStyleName",
    nullptr
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
    nullptr
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

BoundFrameSets::BoundFrameSets(const Reference<XInterface>& rModel)
    : m_pTexts(new BoundFrames())
    , m_pGraphics(new BoundFrames())
    , m_pEmbeddeds(new BoundFrames())
    , m_pShapes(new BoundFrames())
{
    const Reference<XTextFramesSupplier> xTFS(rModel, UNO_QUERY);
    const Reference<XTextGraphicObjectsSupplier> xGOS(rModel, UNO_QUERY);
    const Reference<XTextEmbeddedObjectsSupplier> xEOS(rModel, UNO_QUERY);
    const Reference<XDrawPageSupplier> xDPS(rModel, UNO_QUERY);
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
    const Type aStringType = ::cppu::UnoType<OUString>::get();
    const Type aBoolType = cppu::UnoType<sal_Bool>::get();
    const Type aSeqType = cppu::UnoType<Sequence<OUString>>::get();
    const Type aIntType = ::cppu::UnoType<sal_Int32>::get();
    Sequence<OUString> vParameters(m_xFieldParams->getElementNames());
    for(const auto & rParameter : vParameters)
    {
        const Any aValue = m_xFieldParams->getByName(rParameter);
        const Type& aValueType = aValue.getValueType();
        if(aValueType == aStringType)
        {
            OUString sValue;
            aValue >>= sValue;
            ExportParameter(rParameter,sValue);

            if ( rParameter == ODF_OLE_PARAM )
            {
                // Save the OLE object
                Reference< embed::XStorage > xTargetStg = m_pExport->GetTargetStorage();
                Reference< embed::XStorage > xDstStg = xTargetStg->openStorageElement(
                        "OLELinks", embed::ElementModes::WRITE );

                if ( !xDstStg->hasByName( sValue ) ) {
                    Reference< XStorageBasedDocument > xStgDoc (
                            m_pExport->GetModel( ), UNO_QUERY );
                    Reference< embed::XStorage > xDocStg = xStgDoc->getDocumentStorage();
                    Reference< embed::XStorage > xOleStg = xDocStg->openStorageElement(
                            "OLELinks", embed::ElementModes::READ );

                    xOleStg->copyElementTo( sValue, xDstStg, sValue );
                    Reference< embed::XTransactedObject > xTransact( xDstStg, UNO_QUERY );
                    if ( xTransact.is( ) )
                        xTransact->commit( );
                }
            }
        }
        else if(aValueType == aBoolType)
        {
            bool bValue = false;
            aValue >>= bValue;
            ExportParameter(rParameter, OUString::boolean(bValue) );
        }
        else if(aValueType == aSeqType)
        {
            Sequence<OUString> vValue;
            aValue >>= vValue;
            for(const OUString & i : vValue)
            {
                ExportParameter(rParameter, i);
            }
        }
        else if(aValueType == aIntType)
        {
            sal_Int32 nValue = 0;
            aValue >>= nValue;
            ExportParameter(rParameter, OUString::number(nValue));
        }
    }
}

void FieldParamExporter::ExportParameter(const OUString& sKey, const OUString& sValue)
{
    m_pExport->AddAttribute(XML_NAMESPACE_FIELD, XML_NAME, sKey);
    m_pExport->AddAttribute(XML_NAMESPACE_FIELD, XML_VALUE, sValue);
    m_pExport->StartElement(XML_NAMESPACE_FIELD, XML_PARAM, false);
    m_pExport->EndElement(XML_NAMESPACE_FIELD, XML_PARAM, false);
}

void XMLTextParagraphExport::Add( sal_uInt16 nFamily,
                                  const Reference < XPropertySet > & rPropSet,
                                  const XMLPropertyState** ppAddStates, bool bDontSeek )
{
    rtl::Reference < SvXMLExportPropertyMapper > xPropMapper;
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
    SAL_WARN_IF( !xPropMapper.is(), "xmloff", "There is the property mapper?" );

    vector< XMLPropertyState > aPropStates =
            xPropMapper->Filter( rPropSet );

    if( ppAddStates )
    {
        while( *ppAddStates )
        {
            aPropStates.push_back( **ppAddStates );
            ppAddStates++;
        }
    }

    if( !aPropStates.empty() )
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
                    bool bAdd = sName.isEmpty();
                    if( !bAdd )
                    {
                        Reference < XPropertySet > xNumPropSet( xNumRule,
                                                                UNO_QUERY );
                        if( xNumPropSet.is() &&
                            xNumPropSet->getPropertySetInfo()
                                       ->hasPropertyByName( "IsAutomatic" ) )
                        {
                            bAdd = *o3tl::doAccess<bool>(xNumPropSet->getPropertyValue( "IsAutomatic" ));
                            // Check on outline style (#i73361#)
                            if ( bAdd &&
                                 xNumPropSet->getPropertySetInfo()
                                           ->hasPropertyByName( "NumberingIsOutline" ) )
                            {
                                bAdd = !(*o3tl::doAccess<bool>(xNumPropSet->getPropertyValue( "NumberingIsOutline" )));
                            }
                        }
                        else
                        {
                            bAdd = true;
                        }
                    }
                    if( bAdd )
                        maListAutoPool.Add( xNumRule );
                }
            }
            break;
        case XML_STYLE_FAMILY_TEXT_TEXT:
            {
                // Get parent and remove hyperlinks (they aren't of interest)
                rtl::Reference< XMLPropertySetMapper > xPM(xPropMapper->getPropertySetMapper());
                for( ::std::vector< XMLPropertyState >::iterator i(aPropStates.begin());
                      nIgnoreProps < 2 && i != aPropStates.end(); )
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
                        i = aPropStates.erase( i );
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
            ; // section styles have no parents
            break;
        }
        if( (aPropStates.size() - nIgnoreProps) > 0 )
        {
            GetAutoStylePool().Add( nFamily, sParent, aPropStates, bDontSeek );
            if( !sCondParent.isEmpty() && sParent != sCondParent )
                GetAutoStylePool().Add( nFamily, sCondParent, aPropStates );
        }
    }
}

static bool lcl_validPropState( const XMLPropertyState& rState )
{
    return rState.mnIndex != -1;
}

void XMLTextParagraphExport::Add( sal_uInt16 nFamily,
                                  MultiPropertySetHelper& rPropSetHelper,
                                  const Reference < XPropertySet > & rPropSet)
{
    rtl::Reference < SvXMLExportPropertyMapper > xPropMapper;
    switch( nFamily )
    {
    case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
        xPropMapper = GetParaPropMapper();
        break;
    }
    SAL_WARN_IF( !xPropMapper.is(), "xmloff", "There is the property mapper?" );

    vector< XMLPropertyState > aPropStates(xPropMapper->Filter( rPropSet ));

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
            bool bAdd = sName.isEmpty();
            if( !bAdd )
            {
                Reference < XPropertySet > xNumPropSet( xNumRule,
                                                        UNO_QUERY );
                if( xNumPropSet.is() &&
                    xNumPropSet->getPropertySetInfo()
                               ->hasPropertyByName( "IsAutomatic" ) )
                {
                    bAdd = *o3tl::doAccess<bool>(xNumPropSet->getPropertyValue( "IsAutomatic" ));
                    // Check on outline style (#i73361#)
                    if ( bAdd &&
                         xNumPropSet->getPropertySetInfo()
                                   ->hasPropertyByName( "NumberingIsOutline" ) )
                    {
                        bAdd = !(*o3tl::doAccess<bool>(xNumPropSet->getPropertyValue( "NumberingIsOutline" )));
                    }
                }
                else
                {
                    bAdd = true;
                }
            }
            if( bAdd )
                maListAutoPool.Add( xNumRule );
        }
    }

    if( !aPropStates.empty() )
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

        if( std::any_of( aPropStates.begin(), aPropStates.end(), lcl_validPropState ) )
        {
            GetAutoStylePool().Add( nFamily, sParent, aPropStates );
            if( !sCondParent.isEmpty() && sParent != sCondParent )
                GetAutoStylePool().Add( nFamily, sCondParent, aPropStates );
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
    rtl::Reference < SvXMLExportPropertyMapper > xPropMapper;
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
    SAL_WARN_IF( !xPropMapper.is(), "xmloff", "There is the property mapper?" );
    if( !xPropMapper.is() )
        return sName;
    vector< XMLPropertyState > aPropStates(xPropMapper->Filter( rPropSet ));
    if( ppAddStates )
    {
        while( *ppAddStates )
        {
            aPropStates.push_back( **ppAddStates );
            ++ppAddStates;
        }
    }
    if( std::any_of( aPropStates.begin(), aPropStates.end(), lcl_validPropState ) )
        sName = GetAutoStylePool().Find( nFamily, sName, aPropStates );

    return sName;
}

OUString XMLTextParagraphExport::FindTextStyleAndHyperlink(
           const Reference < XPropertySet > & rPropSet,
        bool& rbHyperlink,
        bool& rbHasCharStyle,
        bool& rbHasAutoStyle,
        const XMLPropertyState** ppAddStates ) const
{
    rtl::Reference < SvXMLExportPropertyMapper > xPropMapper(GetTextPropMapper());
    vector< XMLPropertyState > aPropStates(xPropMapper->Filter( rPropSet ));

    // Get parent and remove hyperlinks (they aren't of interest)
    OUString sName;
    rbHyperlink = rbHasCharStyle = rbHasAutoStyle = false;
    sal_uInt16 nIgnoreProps = 0;
    rtl::Reference< XMLPropertySetMapper > xPM(xPropMapper->getPropertySetMapper());
    ::std::vector< XMLPropertyState >::iterator aFirstDel = aPropStates.end();
    ::std::vector< XMLPropertyState >::iterator aSecondDel = aPropStates.end();

    for( ::std::vector< XMLPropertyState >::iterator
            i = aPropStates.begin();
         nIgnoreProps < 2 && i != aPropStates.end();
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
            rbHyperlink = true;
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
            aPropStates.push_back( **ppAddStates );
            ppAddStates++;
        }
    }
    if( (aPropStates.size() - nIgnoreProps) > 0 )
    {
        // erase the character style, otherwise the autostyle cannot be found!
        // erase the hyperlink, otherwise the autostyle cannot be found!
        if ( nIgnoreProps )
        {
            // If two elements of a vector have to be deleted,
            // we should delete the second one first.
            if( --nIgnoreProps )
                aPropStates.erase( aSecondDel );
            aPropStates.erase( aFirstDel );
        }
        sName = GetAutoStylePool().Find(
            XML_STYLE_FAMILY_TEXT_TEXT,
            OUString(), // AutoStyles should not have parents!
            aPropStates );
        rbHasAutoStyle = true;
    }

    return sName;
}

// adjustments to support lists independent from list style
void XMLTextParagraphExport::exportListChange(
        const XMLTextNumRuleInfo& rPrevInfo,
        const XMLTextNumRuleInfo& rNextInfo )
{
    // end a list
    if ( rPrevInfo.GetLevel() > 0 )
    {
        sal_Int16 nListLevelsToBeClosed = 0;
        if ( !rNextInfo.BelongsToSameList( rPrevInfo ) ||
             rNextInfo.GetLevel() <= 0 )
        {
            // close complete previous list
            nListLevelsToBeClosed = rPrevInfo.GetLevel();
        }
        else if ( rPrevInfo.GetLevel() > rNextInfo.GetLevel() )
        {
            // close corresponding sub lists
            SAL_WARN_IF( rNextInfo.GetLevel() <= 0, "xmloff",
                        "<rPrevInfo.GetLevel() > 0> not hold. Serious defect." );
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
                    GetExport().EndElement(aElem, true);
                }

                // remove closed list from list stack
                mpTextListsHelper->PopListFromStack();

                --nListLevelsToBeClosed;
            } while ( nListLevelsToBeClosed > 0 );
        }
    }

    const bool bExportODF =
                bool( GetExport().getExportFlags() & SvXMLExportFlags::OASIS );
    const SvtSaveOptions::ODFDefaultVersion eODFDefaultVersion =
                                    GetExport().getDefaultVersion();

    // start a new list
    if ( rNextInfo.GetLevel() > 0 )
    {
        bool bRootListToBeStarted = false;
        sal_Int16 nListLevelsToBeOpened = 0;
        if ( !rPrevInfo.BelongsToSameList( rNextInfo ) ||
             rPrevInfo.GetLevel() <= 0 )
        {
            // new root list
            bRootListToBeStarted = true;
            nListLevelsToBeOpened = rNextInfo.GetLevel();
        }
        else if ( rNextInfo.GetLevel() > rPrevInfo.GetLevel() )
        {
            // open corresponding sub lists
            SAL_WARN_IF( rPrevInfo.GetLevel() <= 0, "xmloff",
                        "<rPrevInfo.GetLevel() > 0> not hold. Serious defect." );
            nListLevelsToBeOpened = rNextInfo.GetLevel() - rPrevInfo.GetLevel();
        }

        if ( nListLevelsToBeOpened > 0 )
        {
            const OUString& sListStyleName( rNextInfo.GetNumRulesName() );
            // Currently only the text documents support <ListId>.
            // Thus, for other document types <sListId> is empty.
            const OUString& sListId( rNextInfo.GetListId() );
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
                        // store that list with list id <sNewListId> is last list,
                        // which has continued list with list id <sListId>
                        mpTextListsHelper->StoreLastContinuingList( sListId,
                                                                    sNewListId );
                        if ( sListStyleName ==
                                mpTextListsHelper->GetListStyleOfLastProcessedList() &&
                             // Inconsistent behavior regarding lists (#i92811#)
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
                    // rhbz#746174: also export list restart for non root list
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
                GetExport().StartElement(aElem, false);

                if(!pListElements)
                    pListElements.reset( new std::vector<OUString> );
                pListElements->push_back(aElem);

                mpTextListsHelper->PushListOnStack( sListId,
                                                    sListStyleName );

                // <text:list-header> or <text:list-item>
                GetExport().CheckAttrList();

                /* Export start value at correct list item (#i97309#) */
                if ( nListLevelsToBeOpened == 1 )
                {
                    if ( rNextInfo.HasStartValue() )
                    {
                        OUStringBuffer aBuffer;
                        aBuffer.append( static_cast<sal_Int32>(rNextInfo.GetStartValue()) );
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
                aElem = GetExport().GetNamespaceMap().GetQNameByKey(
                                            XML_NAMESPACE_TEXT,
                                            GetXMLToken(eLName) );
                GetExport().IgnorableWhitespace();
                GetExport().StartElement(aElem, false);
                pListElements->push_back(aElem);

                // export of <text:number> element for last opened <text:list-item>, if requested
                if ( GetExport().exportTextNumberElement() &&
                     eLName == XML_LIST_ITEM && nListLevelsToBeOpened == 1 && // last iteration --> last opened <text:list-item>
                     !rNextInfo.ListLabelString().isEmpty() )
                {
                    const OUString aTextNumberElem =
                            GetExport().GetNamespaceMap().GetQNameByKey(
                                      XML_NAMESPACE_TEXT,
                                      GetXMLToken(XML_NUMBER) );
                    GetExport().IgnorableWhitespace();
                    GetExport().StartElement( aTextNumberElem, false );
                    GetExport().Characters( rNextInfo.ListLabelString() );
                    GetExport().EndElement( aTextNumberElem, true );
                }
                --nListLevelsToBeOpened;
            } while ( nListLevelsToBeOpened > 0 );
        }
    }

    bool bEndElement = false;

    if ( rNextInfo.GetLevel() > 0 &&
         rNextInfo.IsNumbered() &&
         rPrevInfo.BelongsToSameList( rNextInfo ) &&
         rPrevInfo.GetLevel() >= rNextInfo.GetLevel() )
    {
        assert(pListElements && pListElements->size() >= 2 && "list elements missing");
        bEndElement = pListElements && pListElements->size() >= 2;
    }

    if (!bEndElement)
        return;

    // close previous list-item
    GetExport().EndElement(pListElements->back(), true );
    pListElements->pop_back();

    // Only for sub lists (#i103745#)
    if ( rNextInfo.IsRestart() && !rNextInfo.HasStartValue() &&
         rNextInfo.GetLevel() != 1 )
    {
        // start new sub list respectively list on same list level
        GetExport().EndElement(pListElements->back(), true );
        GetExport().IgnorableWhitespace();
        GetExport().StartElement(pListElements->back(), false);
    }

    // open new list-item
    GetExport().CheckAttrList();
    if( rNextInfo.HasStartValue() )
    {
        OUStringBuffer aBuffer;
        aBuffer.append( static_cast<sal_Int32>(rNextInfo.GetStartValue()) );
        GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_START_VALUE,
                                  aBuffer.makeStringAndClear() );
    }
    // Handle restart without start value on list level 1 (#i103745#)
    else if ( rNextInfo.IsRestart() && /*!rNextInfo.HasStartValue() &&*/
              rNextInfo.GetLevel() == 1 )
    {
        OUStringBuffer aBuffer;
        aBuffer.append( static_cast<sal_Int32>(rNextInfo.GetListLevelStartValue()) );
        GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_START_VALUE,
                                  aBuffer.makeStringAndClear() );
    }
    if ( ( GetExport().getExportFlags() & SvXMLExportFlags::OASIS ) &&
         GetExport().getDefaultVersion() >= SvtSaveOptions::ODFVER_012 )
    {
        const OUString& sListStyleName( rNextInfo.GetNumRulesName() );
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
    GetExport().StartElement(aElem, false );
    pListElements->push_back(aElem);

    // export of <text:number> element for <text:list-item>, if requested
    if ( GetExport().exportTextNumberElement() &&
         !rNextInfo.ListLabelString().isEmpty() )
    {
        const OUString aTextNumberElem =
                GetExport().GetNamespaceMap().GetQNameByKey(
                          XML_NAMESPACE_TEXT,
                          GetXMLToken(XML_NUMBER) );
        GetExport().IgnorableWhitespace();
        GetExport().StartElement( aTextNumberElem, false );
        GetExport().Characters( rNextInfo.ListLabelString() );
        GetExport().EndElement( aTextNumberElem, true );
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
        // rely on SwXFieldmark::CreateXFieldmark returning the same instance
        // because the Reference in m_FieldMarkMap will keep it alive
        assert(it != m_FieldMarkMap.end());
        return it->second;
    }
};

XMLTextParagraphExport::XMLTextParagraphExport(
        SvXMLExport& rExp,
        SvXMLAutoStylePoolP & rASP
        ) :
    XMLStyleExport( rExp, &rASP ),
    m_xImpl(new Impl),
    rAutoStylePool( rASP ),
    pBoundFrameSets(new BoundFrameSets(GetExport().GetModel())),
    pFieldExport( nullptr ),
    pListElements( nullptr ),
    maListAutoPool( GetExport() ),
    pSectionExport( nullptr ),
    pIndexMarkExport( nullptr ),
    pRedlineExport( nullptr ),
    bProgress( false ),
    bBlock( false ),
    bOpenRuby( false ),
    mpTextListsHelper( nullptr ),
    maTextListsHelperStack(),
    mbCollected(false),
    // Implement Title/Description Elements UI (#i73249#)
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
    sDocumentIndexMark("DocumentIndexMark"),
    sEndNotice("EndNotice"),
    sFootnote("Footnote"),
    sFootnoteCounting("FootnoteCounting"),
    sFrame("Frame"),
    sGraphicFilter("GraphicFilter"),
    sGraphicRotation("GraphicRotation"),
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
    sFrameStyleName("FrameStyleName"),
    aCharStyleNamesPropInfoCache( sCharStyleNames )
{
    rtl::Reference < XMLPropertySetMapper > xPropMapper(new XMLTextPropertySetMapper( TextPropMap::PARA, true ));
    xParaPropMapper = new XMLTextExportPropertySetMapper( xPropMapper,
                                                             GetExport() );

    OUString sFamily( GetXMLToken(XML_PARAGRAPH) );
    OUString aPrefix(u'P');
    rAutoStylePool.AddFamily( XML_STYLE_FAMILY_TEXT_PARAGRAPH, sFamily,
                              xParaPropMapper, aPrefix );

    xPropMapper = new XMLTextPropertySetMapper( TextPropMap::TEXT, true );
    xTextPropMapper = new XMLTextExportPropertySetMapper( xPropMapper,
                                                             GetExport() );
    sFamily = GetXMLToken(XML_TEXT);
    aPrefix = "T";
    rAutoStylePool.AddFamily( XML_STYLE_FAMILY_TEXT_TEXT, sFamily,
                              xTextPropMapper, aPrefix );

    xPropMapper = new XMLTextPropertySetMapper( TextPropMap::AUTO_FRAME, true );
    xAutoFramePropMapper = new XMLTextExportPropertySetMapper( xPropMapper,
                                                                  GetExport() );
    sFamily = XML_STYLE_FAMILY_SD_GRAPHICS_NAME;
    aPrefix = "fr";
    rAutoStylePool.AddFamily( XML_STYLE_FAMILY_TEXT_FRAME, sFamily,
                              xAutoFramePropMapper, aPrefix );

    xPropMapper = new XMLTextPropertySetMapper( TextPropMap::SECTION, true );
    xSectionPropMapper = new XMLTextExportPropertySetMapper( xPropMapper,
                                                             GetExport() );
    sFamily = GetXMLToken( XML_SECTION );
    aPrefix = "Sect" ;
    rAutoStylePool.AddFamily( XML_STYLE_FAMILY_TEXT_SECTION, sFamily,
                              xSectionPropMapper, aPrefix );

    xPropMapper = new XMLTextPropertySetMapper( TextPropMap::RUBY, true );
    xRubyPropMapper = new SvXMLExportPropertyMapper( xPropMapper );
    sFamily = GetXMLToken( XML_RUBY );
    aPrefix = "Ru";
    rAutoStylePool.AddFamily( XML_STYLE_FAMILY_TEXT_RUBY, sFamily,
                              xRubyPropMapper, aPrefix );

    xPropMapper = new XMLTextPropertySetMapper( TextPropMap::FRAME, true );
    xFramePropMapper = new XMLTextExportPropertySetMapper( xPropMapper,
                                                              GetExport() );

    pSectionExport.reset( new XMLSectionExport( rExp, *this ) );
    pIndexMarkExport.reset( new XMLIndexMarkExport( rExp ) );

    if( ! IsBlockMode() &&
        Reference<XRedlinesSupplier>( GetExport().GetModel(), UNO_QUERY ).is())
        pRedlineExport.reset( new XMLRedlineExport( rExp ) );

    // The text field helper needs a pre-constructed XMLPropertyState
    // to export the combined characters field. We construct that
    // here, because we need the text property mapper to do it.

    // construct Any value, then find index
    sal_Int32 nIndex = xTextPropMapper->getPropertySetMapper()->FindEntryIndex(
                                "", XML_NAMESPACE_STYLE,
                                GetXMLToken(XML_TEXT_COMBINE));
    pFieldExport.reset( new XMLTextFieldExport( rExp, new XMLPropertyState( nIndex, uno::makeAny(true) ) ) );
    PushNewTextListsHelper();
}

XMLTextParagraphExport::~XMLTextParagraphExport()
{
    pRedlineExport.reset();
    pIndexMarkExport.reset();
    pSectionExport.reset();
    pFieldExport.reset();
    pListElements.reset();
#ifdef DBG_UTIL
    txtparae_bContainsIllegalCharacters = false;
#endif
    PopTextListsHelper();
    SAL_WARN_IF( !maTextListsHelperStack.empty(), "xmloff",
                "misusage of text lists helper stack - it is not empty. Serious defect" );
}

SvXMLExportPropertyMapper *XMLTextParagraphExport::CreateShapeExtPropMapper(
        SvXMLExport& rExport )
{
    rtl::Reference < XMLPropertySetMapper > xPropMapper =
        new XMLTextPropertySetMapper( TextPropMap::SHAPE, true );
    return new XMLTextExportPropertySetMapper( xPropMapper, rExport );
}

SvXMLExportPropertyMapper *XMLTextParagraphExport::CreateCharExtPropMapper(
        SvXMLExport& rExport)
{
    XMLPropertySetMapper *pPropMapper =
        new XMLTextPropertySetMapper( TextPropMap::TEXT, true );
    return new XMLTextExportPropertySetMapper( pPropMapper, rExport );
}

SvXMLExportPropertyMapper *XMLTextParagraphExport::CreateParaExtPropMapper(
        SvXMLExport& rExport)
{
    XMLPropertySetMapper *pPropMapper =
        new XMLTextPropertySetMapper( TextPropMap::SHAPE_PARA, true );
    return new XMLTextExportPropertySetMapper( pPropMapper, rExport );
}

SvXMLExportPropertyMapper *XMLTextParagraphExport::CreateParaDefaultExtPropMapper(
        SvXMLExport& rExport)
{
    XMLPropertySetMapper *pPropMapper =
        new XMLTextPropertySetMapper( TextPropMap::TEXT_ADDITIONAL_DEFAULTS, true );
    return new XMLTextExportPropertySetMapper( pPropMapper, rExport );
}

void XMLTextParagraphExport::exportPageFrames( bool bIsProgress )
{
    const TextContentSet& rTexts = pBoundFrameSets->GetTexts()->GetPageBoundContents();
    const TextContentSet& rGraphics = pBoundFrameSets->GetGraphics()->GetPageBoundContents();
    const TextContentSet& rEmbeddeds = pBoundFrameSets->GetEmbeddeds()->GetPageBoundContents();
    const TextContentSet& rShapes = pBoundFrameSets->GetShapes()->GetPageBoundContents();
    for(TextContentSet::const_iterator_t it = rTexts.getBegin();
        it != rTexts.getEnd();
        ++it)
        exportTextFrame(*it, false/*bAutoStyles*/, bIsProgress, true);
    for(TextContentSet::const_iterator_t it = rGraphics.getBegin();
        it != rGraphics.getEnd();
        ++it)
        exportTextGraphic(*it, false/*bAutoStyles*/);
    for(TextContentSet::const_iterator_t it = rEmbeddeds.getBegin();
        it != rEmbeddeds.getEnd();
        ++it)
        exportTextEmbedded(*it, false/*bAutoStyles*/);
    for(TextContentSet::const_iterator_t it = rShapes.getBegin();
        it != rShapes.getEnd();
        ++it)
        exportShape(*it, false/*bAutoStyles*/);
}

void XMLTextParagraphExport::exportFrameFrames(
        bool bAutoStyles,
        bool bIsProgress,
        const Reference < XTextFrame > *pParentTxtFrame )
{
    const TextContentSet* const pTexts = pBoundFrameSets->GetTexts()->GetFrameBoundContents(*pParentTxtFrame);
    if(pTexts)
        for(TextContentSet::const_iterator_t it = pTexts->getBegin();
            it != pTexts->getEnd();
            ++it)
            exportTextFrame(*it, bAutoStyles, bIsProgress, true);
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

// bookmarks, reference marks (and TOC marks) are the same except for the
// element names. We use the same method for export and it an array with
// the proper element names
static const enum XMLTokenEnum lcl_XmlReferenceElements[] = {
    XML_REFERENCE_MARK, XML_REFERENCE_MARK_START, XML_REFERENCE_MARK_END };
static const enum XMLTokenEnum lcl_XmlBookmarkElements[] = {
    XML_BOOKMARK, XML_BOOKMARK_START, XML_BOOKMARK_END };

// This function replaces the text portion iteration during auto style
// collection.
void XMLTextParagraphExport::collectTextAutoStylesOptimized( bool bIsProgress )
{
    GetExport().GetShapeExport(); // make sure the graphics styles family is added

    if (mbCollected)
        return;

    const bool bAutoStyles = true;
    const bool bExportContent = false;

    // Export AutoStyles:
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
            Reference< XAutoStyleFamily > xAutoStyles = *o3tl::doAccess<Reference<XAutoStyleFamily>>(aAny);
            Reference < XEnumeration > xAutoStylesEnum( xAutoStyles->createEnumeration() );

            while ( xAutoStylesEnum->hasMoreElements() )
            {
                aAny = xAutoStylesEnum->nextElement();
                Reference< XAutoStyle > xAutoStyle = *o3tl::doAccess<Reference<XAutoStyle>>(aAny);
                Reference < XPropertySet > xPSet( xAutoStyle, uno::UNO_QUERY );
                Add( nFamily, xPSet, nullptr, true );
            }
        }
    }

    // Export Field AutoStyles:
    Reference< XTextFieldsSupplier > xTextFieldsSupp( GetExport().GetModel(), UNO_QUERY );
    if ( xTextFieldsSupp.is() )
    {
        Reference< XEnumerationAccess > xTextFields = xTextFieldsSupp->getTextFields();
        Reference < XEnumeration > xTextFieldsEnum( xTextFields->createEnumeration() );

        while ( xTextFieldsEnum->hasMoreElements() )
        {
            Any aAny = xTextFieldsEnum->nextElement();
            Reference< XTextField > xTextField = *o3tl::doAccess<Reference<XTextField>>(aAny);
            exportTextField( xTextField, bAutoStyles, bIsProgress,
                !xAutoStylesSupp.is(), nullptr );
            try
            {
                Reference < XPropertySet > xSet( xTextField, UNO_QUERY );
                Reference < XText > xText;
                Any a = xSet->getPropertyValue("TextRange");
                a >>= xText;
                if ( xText.is() )
                {
                    exportText( xText, true, bIsProgress, bExportContent );
                    GetExport().GetTextParagraphExport()
                        ->collectTextAutoStyles( xText );
                }
            }
            catch (Exception&)
            {
            }
        }
    }

    // Export text frames:
    Reference<XEnumeration> xTextFramesEnum = pBoundFrameSets->GetTexts()->createEnumeration();
    if(xTextFramesEnum.is())
        while(xTextFramesEnum->hasMoreElements())
        {
            Reference<XTextContent> xTxtCntnt(xTextFramesEnum->nextElement(), UNO_QUERY);
            if(xTxtCntnt.is())
                exportTextFrame(xTxtCntnt, bAutoStyles, bIsProgress, bExportContent);
        }

    // Export graphic objects:
    Reference<XEnumeration> xGraphicsEnum = pBoundFrameSets->GetGraphics()->createEnumeration();
    if(xGraphicsEnum.is())
        while(xGraphicsEnum->hasMoreElements())
        {
            Reference<XTextContent> xTxtCntnt(xGraphicsEnum->nextElement(), UNO_QUERY);
            if(xTxtCntnt.is())
                exportTextGraphic(xTxtCntnt, true);
        }

    // Export embedded objects:
    Reference<XEnumeration> xEmbeddedsEnum = pBoundFrameSets->GetEmbeddeds()->createEnumeration();
    if(xEmbeddedsEnum.is())
        while(xEmbeddedsEnum->hasMoreElements())
        {
            Reference<XTextContent> xTxtCntnt(xEmbeddedsEnum->nextElement(), UNO_QUERY);
            if(xTxtCntnt.is())
                exportTextEmbedded(xTxtCntnt, true);
        }

    // Export shapes:
    Reference<XEnumeration> xShapesEnum = pBoundFrameSets->GetShapes()->createEnumeration();
    if(xShapesEnum.is())
        while(xShapesEnum->hasMoreElements())
        {
            Reference<XTextContent> xTxtCntnt(xShapesEnum->nextElement(), UNO_QUERY);
            if(xTxtCntnt.is())
            {
                Reference<XServiceInfo> xServiceInfo(xTxtCntnt, UNO_QUERY);
                if( xServiceInfo->supportsService(sShapeService))
                    exportShape(xTxtCntnt, true);
            }
        }

    sal_Int32 nCount;
    // AutoStyles for sections
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
                Reference< XTextSection > xSection = *o3tl::doAccess<Reference<XTextSection>>(aAny);
                Reference < XPropertySet > xPSet( xSection, uno::UNO_QUERY );
                Add( XML_STYLE_FAMILY_TEXT_SECTION, xPSet );
            }
        }
    }

    // AutoStyles for tables (Note: suppress autostyle collection for paragraphs in exportTable)
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
                Reference< XTextTable > xTable = *o3tl::doAccess<Reference<XTextTable>>(aAny);
                exportTable( xTable, true, true );
            }
        }
    }

    Reference< XNumberingRulesSupplier > xNumberingRulesSupp( GetExport().GetModel(), UNO_QUERY );
    if ( xNumberingRulesSupp.is() )
    {
        Reference< XIndexAccess > xNumberingRules = xNumberingRulesSupp->getNumberingRules();
        nCount = xNumberingRules->getCount();
        // Custom outline assignment lost after re-importing sxw (#i73361#)
        for( sal_Int32 i = 0; i < nCount; ++i )
        {
            Reference< XIndexReplace > xNumRule( xNumberingRules->getByIndex( i ), UNO_QUERY );
            if( xNumRule.is() && xNumRule->getCount() )
            {
                Reference < XNamed > xNamed( xNumRule, UNO_QUERY );
                OUString sName;
                if( xNamed.is() )
                    sName = xNamed->getName();
                bool bAdd = sName.isEmpty();
                if( !bAdd )
                {
                    Reference < XPropertySet > xNumPropSet( xNumRule,
                                                            UNO_QUERY );
                    if( xNumPropSet.is() &&
                        xNumPropSet->getPropertySetInfo()
                                   ->hasPropertyByName( "IsAutomatic" ) )
                    {
                        bAdd = *o3tl::doAccess<bool>(xNumPropSet->getPropertyValue( "IsAutomatic" ));
                        // Check on outline style (#i73361#)
                        if ( bAdd &&
                             xNumPropSet->getPropertySetInfo()
                                       ->hasPropertyByName( "NumberingIsOutline" ) )
                        {
                            bAdd = !(*o3tl::doAccess<bool>(xNumPropSet->getPropertyValue( "NumberingIsOutline" )));
                        }
                    }
                    else
                    {
                        bAdd = true;
                    }
                }
                if( bAdd )
                    maListAutoPool.Add( xNumRule );
            }
        }
    }
    mbCollected = true;
}

void XMLTextParagraphExport::exportText(
        const Reference < XText > & rText,
        bool bAutoStyles,
        bool bIsProgress,
        bool bExportParagraph,
        TextPNS eExtensionNS)
{
    if( bAutoStyles )
        GetExport().GetShapeExport(); // make sure the graphics styles family
                                      // is added
    Reference < XEnumerationAccess > xEA( rText, UNO_QUERY );
    if( ! xEA.is() )
        return;

    Reference < XEnumeration > xParaEnum(xEA->createEnumeration());
    Reference < XPropertySet > xPropertySet( rText, UNO_QUERY );
    Reference < XTextSection > xBaseSection;

    // #97718# footnotes don't supply paragraph enumerations in some cases
    // This is always a bug, but at least we don't want to crash.
    SAL_WARN_IF( !xParaEnum.is(), "xmloff", "We need a paragraph enumeration" );
    if( ! xParaEnum.is() )
        return;

    if (xPropertySet.is())
    {
        Reference < XPropertySetInfo > xInfo ( xPropertySet->getPropertySetInfo() );

        if( xInfo.is() )
        {
            if (xInfo->hasPropertyByName( sTextSection ))
            {
                xPropertySet->getPropertyValue(sTextSection) >>= xBaseSection ;
            }
        }
    }

    // #96530# Export redlines at start & end of XText before & after
    // exporting the text content enumeration
    if( !bAutoStyles && (pRedlineExport != nullptr) )
        pRedlineExport->ExportStartOrEndRedline( xPropertySet, true );
    exportTextContentEnumeration( xParaEnum, bAutoStyles, xBaseSection,
                                  bIsProgress, bExportParagraph, nullptr, eExtensionNS );
    if( !bAutoStyles && (pRedlineExport != nullptr) )
        pRedlineExport->ExportStartOrEndRedline( xPropertySet, false );
}

void XMLTextParagraphExport::exportText(
        const Reference < XText > & rText,
        const Reference < XTextSection > & rBaseSection,
        bool bAutoStyles,
        bool bIsProgress,
        bool bExportParagraph)
{
    if( bAutoStyles )
        GetExport().GetShapeExport(); // make sure the graphics styles family
                                      // is added
    Reference < XEnumerationAccess > xEA( rText, UNO_QUERY );
    Reference < XEnumeration > xParaEnum(xEA->createEnumeration());

    // #98165# don't continue without a paragraph enumeration
    if( ! xParaEnum.is() )
        return;

    // #96530# Export redlines at start & end of XText before & after
    // exporting the text content enumeration
    Reference<XPropertySet> xPropertySet;
    if( !bAutoStyles && (pRedlineExport != nullptr) )
    {
        xPropertySet.set(rText, uno::UNO_QUERY );
        pRedlineExport->ExportStartOrEndRedline( xPropertySet, true );
    }
    exportTextContentEnumeration( xParaEnum, bAutoStyles, rBaseSection,
                                  bIsProgress, bExportParagraph );
    if( !bAutoStyles && (pRedlineExport != nullptr) )
        pRedlineExport->ExportStartOrEndRedline( xPropertySet, false );
}

void XMLTextParagraphExport::exportTextContentEnumeration(
        const Reference < XEnumeration > & rContEnum,
        bool bAutoStyles,
        const Reference < XTextSection > & rBaseSection,
        bool bIsProgress,
        bool bExportParagraph,
        const Reference < XPropertySet > *pRangePropSet,
        TextPNS eExtensionNS )
{
    SAL_WARN_IF( !rContEnum.is(), "xmloff", "No enumeration to export!" );
    bool bHasMoreElements = rContEnum->hasMoreElements();
    if( !bHasMoreElements )
        return;

    XMLTextNumRuleInfo aPrevNumInfo;
    XMLTextNumRuleInfo aNextNumInfo;

    bool bHasContent = false;
    Reference<XTextSection> xCurrentTextSection(rBaseSection);

    MultiPropertySetHelper aPropSetHelper(
                               bAutoStyles ? aParagraphPropertyNamesAuto :
                                          aParagraphPropertyNames );

    bool bHoldElement = false;
    Reference < XTextContent > xTxtCntnt;
    while( bHoldElement || bHasMoreElements )
    {
        if (bHoldElement)
        {
            bHoldElement = false;
        }
        else
        {
            xTxtCntnt.set(rContEnum->nextElement(), uno::UNO_QUERY);

            aPropSetHelper.resetValues();

        }

        Reference<XServiceInfo> xServiceInfo( xTxtCntnt, UNO_QUERY );
        if( xServiceInfo->supportsService( sParagraphService ) )
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

            // if we found a mute section: skip all section content
            if (pSectionExport->IsMuteSection(xCurrentTextSection))
            {
                // Make sure headings are exported anyway.
                if( !bAutoStyles )
                    pSectionExport->ExportMasterDocHeadingDummies();

                while (rContEnum->hasMoreElements() &&
                       XMLSectionExport::IsInSection( xCurrentTextSection,
                                                    xTxtCntnt, true ))
                {
                    xTxtCntnt.set(rContEnum->nextElement(), uno::UNO_QUERY);
                    aPropSetHelper.resetValues();
                    aNextNumInfo.Reset();
                }
                // the first non-mute element still needs to be processed
                bHoldElement =
                    ! XMLSectionExport::IsInSection( xCurrentTextSection,
                                                   xTxtCntnt, false );
            }
            else
                exportParagraph( xTxtCntnt, bAutoStyles, bIsProgress,
                                 bExportParagraph, aPropSetHelper, eExtensionNS );
            bHasContent = true;
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
                // export start + end redlines (for wholly redlined tables)
                if ((! bAutoStyles) && (nullptr != pRedlineExport))
                    pRedlineExport->ExportStartOrEndRedline(xTxtCntnt, true);

                exportTable( xTxtCntnt, bAutoStyles, bIsProgress  );

                if ((! bAutoStyles) && (nullptr != pRedlineExport))
                    pRedlineExport->ExportStartOrEndRedline(xTxtCntnt, false);
            }
            else if( !bAutoStyles )
            {
                // Make sure headings are exported anyway.
                pSectionExport->ExportMasterDocHeadingDummies();
            }

            bHasContent = true;
        }
        else if( xServiceInfo->supportsService( sTextFrameService ) )
        {
            exportTextFrame( xTxtCntnt, bAutoStyles, bIsProgress, true, pRangePropSet );
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
            SAL_WARN_IF( xTxtCntnt.is(), "xmloff", "unknown text content" );
        }

        if( !bAutoStyles )
        {
            aPrevNumInfo = aNextNumInfo;
        }

        bHasMoreElements = rContEnum->hasMoreElements();
    }

    if( bHasContent && !bAutoStyles )
    {
        aNextNumInfo.Reset();

        // close open lists and sections; no new styles
        exportListAndSectionChange( xCurrentTextSection, rBaseSection,
                                    aPrevNumInfo, aNextNumInfo,
                                    bAutoStyles );
    }
}

void XMLTextParagraphExport::exportParagraph(
        const Reference < XTextContent > & rTextContent,
        bool bAutoStyles, bool bIsProgress, bool bExportParagraph,
        MultiPropertySetHelper& rPropSetHelper, TextPNS eExtensionNS)
{
    sal_Int16 nOutlineLevel = -1;

    if( bIsProgress )
    {
        ProgressBarHelper *pProgress = GetExport().GetProgressBarHelper();
        pProgress->SetValue( pProgress->GetValue()+1 );
    }

    // get property set or multi property set and initialize helper
    Reference<XMultiPropertySet> xMultiPropSet( rTextContent, UNO_QUERY );
    Reference<XPropertySet> xPropSet( rTextContent, UNO_QUERY );

    // check for supported properties
    if( !rPropSetHelper.checkedProperties() )
        rPropSetHelper.hasProperties( xPropSet->getPropertySetInfo() );

//  if( xMultiPropSet.is() )
//      rPropSetHelper.getValues( xMultiPropSet );
//  else
//      rPropSetHelper.getValues( xPropSet );

    if( bExportParagraph )
    {
        if( bAutoStyles )
        {
            Add( XML_STYLE_FAMILY_TEXT_PARAGRAPH, rPropSetHelper, xPropSet );
        }
        else
        {
            // xml:id for RDF metadata
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
                    // FIXME: this is just temporary until EditEngine
                    // paragraphs implement XMetadatable.
                    // then that must be used and not the mapper, because
                    // when both can be used we get two xml:id!
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
                                SAL_WARN_IF( !xNumRule.is(), "xmloff", "no chapter numbering rules" );

                                if (xNumRule.is())
                                {
                                    Reference< XPropertySet > xNumRulePropSet( xNumRule, UNO_QUERY );
                                    xNumRulePropSet->getPropertyValue(
                                        "Name" ) >>= sOutlineName;
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
                        bool bIsRestartNumbering = false;

                        Reference< XPropertySetInfo >
                        xPropSetInfo(xMultiPropSet.is() ?
                                     xMultiPropSet->getPropertySetInfo():
                                     xPropSet->getPropertySetInfo());

                        if (xPropSetInfo->
                            hasPropertyByName("ParaIsNumberingRestart"))
                        {
                            xPropSet->getPropertyValue("ParaIsNumberingRestart")
                                >>= bIsRestartNumbering;
                        }

                        if (bIsRestartNumbering)
                        {
                            GetExport().AddAttribute(XML_NAMESPACE_TEXT,
                                                     XML_RESTART_NUMBERING,
                                                     XML_TRUE);

                            if (xPropSetInfo->
                                hasPropertyByName("NumberingStartValue"))
                            {
                                sal_Int32 nStartValue = 0;

                                xPropSet->getPropertyValue("NumberingStartValue")
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
    const bool bHasPortions = xTextEnum.is();

    Reference < XEnumeration> xContentEnum;
    Reference < XContentEnumerationAccess > xCEA( rTextContent, UNO_QUERY );
    if( xCEA.is() )
        xContentEnum.set(xCEA->createContentEnumeration( sTextContentService ));
    const bool bHasContentEnum = xContentEnum.is() &&
                                        xContentEnum->hasMoreElements();

    Reference < XTextSection > xSection;
    if( bHasContentEnum )
    {
        // For the auto styles, the multi property set helper is only used
        // if hard attributes are existing. Therefore, it seems to be a better
        // strategy to have the TextSection property separate, because otherwise
        // we always retrieve the style names even if they are not required.
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

    bool bPrevCharIsSpace(true); // true because whitespace at start is ignored

    if( bAutoStyles )
    {
        if( bHasContentEnum )
            exportTextContentEnumeration(
                                    xContentEnum, bAutoStyles, xSection,
                                    bIsProgress );
        if ( bHasPortions )
        {
            exportTextRangeEnumeration(xTextEnum, bAutoStyles, bIsProgress, bPrevCharIsSpace);
        }
    }
    else
    {
        enum XMLTokenEnum eElem =
            0 < nOutlineLevel ? XML_H : XML_P;
        SvXMLElementExport aElem( GetExport(), eExtensionNS == TextPNS::EXTENSION ? XML_NAMESPACE_LO_EXT : XML_NAMESPACE_TEXT, eElem,
                                  true, false );
        if( bHasContentEnum )
        {
            exportTextContentEnumeration(
                                    xContentEnum, bAutoStyles, xSection,
                                    bIsProgress );
        }
        exportTextRangeEnumeration(xTextEnum, bAutoStyles, bIsProgress, bPrevCharIsSpace);
    }
}

void XMLTextParagraphExport::exportTextRangeEnumeration(
        const Reference < XEnumeration > & rTextEnum,
        bool bAutoStyles, bool bIsProgress,
        bool & rPrevCharIsSpace)
{
    static const char sFieldMarkName[] = "__FieldMark_";

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

            if( sType == sText)
            {
                exportTextRange( xTxtRange, bAutoStyles,
                                 rPrevCharIsSpace, openFieldMark);
            }
            else if( sType == sTextField)
            {
                exportTextField(xTxtRange, bAutoStyles, bIsProgress, &rPrevCharIsSpace);
            }
            else if ( sType == "Annotation" )
            {
                exportTextField(xTxtRange, bAutoStyles, bIsProgress, &rPrevCharIsSpace);
            }
            else if ( sType == "AnnotationEnd" )
            {
                if (!bAutoStyles)
                {
                    Reference<XNamed> xBookmark(xPropSet->getPropertyValue(sBookmark), UNO_QUERY);
                    const OUString& rName = xBookmark->getName();
                    if (!rName.isEmpty())
                    {
                        GetExport().AddAttribute(XML_NAMESPACE_OFFICE, XML_NAME, rName);
                    }
                    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_OFFICE, XML_ANNOTATION_END, false, false );
                }
            }
            else if( sType == sFrame )
            {
                Reference < XEnumeration> xContentEnum;
                Reference < XContentEnumerationAccess > xCEA( xTxtRange,
                                                              UNO_QUERY );
                if( xCEA.is() )
                    xContentEnum.set(xCEA->createContentEnumeration(
                                                    sTextContentService ));
                // frames are never in sections
                Reference<XTextSection> xSection;
                if( xContentEnum.is() )
                    exportTextContentEnumeration( xContentEnum,
                                                    bAutoStyles,
                                                    xSection, bIsProgress, true,
                                                     &xPropSet  );

            }
            else if (sType == sFootnote)
            {
                exportTextFootnote(xPropSet,
                                   xTxtRange->getString(),
                                   bAutoStyles, bIsProgress );
            }
            else if (sType == sBookmark)
            {
                exportTextMark(xPropSet,
                               sBookmark,
                               lcl_XmlBookmarkElements,
                               bAutoStyles);
            }
            else if (sType == sReferenceMark)
            {
                exportTextMark(xPropSet,
                               sReferenceMark,
                               lcl_XmlReferenceElements,
                               bAutoStyles);
            }
            else if (sType == sDocumentIndexMark)
            {
                pIndexMarkExport->ExportIndexMark(xPropSet, bAutoStyles);
            }
            else if (sType == sRedline)
            {
                if (nullptr != pRedlineExport)
                    pRedlineExport->ExportChange(xPropSet, bAutoStyles);
            }
            else if (sType == sRuby)
            {
                exportRuby(xPropSet, bAutoStyles);
            }
            else if (sType == "InContentMetadata")
            {
                exportMeta(xPropSet, bAutoStyles, bIsProgress, rPrevCharIsSpace);
            }
            else if (sType == sTextFieldStart)
            {
                Reference< css::text::XFormField > xFormField(xPropSet->getPropertyValue(sBookmark), UNO_QUERY);

                /* As of now, textmarks are a proposed extension to the OpenDocument standard. */
                if (!bAutoStyles)
                {
                    if (GetExport().getDefaultVersion() > SvtSaveOptions::ODFVER_012)
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

                        GetExport().StartElement(XML_NAMESPACE_FIELD, XML_FIELDMARK_START, false);
                        if (xFormField.is())
                        {
                            FieldParamExporter(&GetExport(), xFormField->getParameters()).Export();
                        }
                        GetExport().EndElement(XML_NAMESPACE_FIELD, XML_FIELDMARK_START, false);
                    }
                    /* The OpenDocument standard does not include support for TextMarks for now, so use bookmarks instead. */
                    else
                    {
                        if (xFormField.is())
                        {
                            OUString sName;
                            Reference< css::container::XNameAccess > xParameters(xFormField->getParameters(), UNO_QUERY);
                            if (xParameters.is() && xParameters->hasByName("Name"))
                            {
                                const Any aValue = xParameters->getByName("Name");
                                aValue >>= sName;
                            }
                            if (sName.isEmpty())
                            {   // name attribute is mandatory, so have to pull a
                                // rabbit out of the hat here
                                sName = sFieldMarkName + OUString::number(
                                        m_xImpl->AddFieldMarkStart(xFormField));
                            }
                            GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_NAME,
                                    sName);
                            SvXMLElementExport aElem( GetExport(), !bAutoStyles,
                                XML_NAMESPACE_TEXT, XML_BOOKMARK_START,
                                false, false );
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
            }
            else if (sType == sTextFieldEnd)
            {
                if (!bAutoStyles)
                {
                    Reference< css::text::XFormField > xFormField(xPropSet->getPropertyValue(sBookmark), UNO_QUERY);

                    if ( GetExport().getDefaultVersion() > SvtSaveOptions::ODFVER_012 )
                    {
                        SvXMLElementExport aElem( GetExport(), !bAutoStyles,
                            XML_NAMESPACE_FIELD, XML_FIELDMARK_END,
                            false, false );
                    }
                    else
                    {
                        if (xFormField.is())
                        {
                            OUString sName;
                            Reference< css::container::XNameAccess > xParameters(xFormField->getParameters(), UNO_QUERY);
                            if (xParameters.is() && xParameters->hasByName("Name"))
                            {
                                const Any aValue = xParameters->getByName("Name");
                                aValue >>= sName;
                            }
                            if (sName.isEmpty())
                            {   // name attribute is mandatory, so have to pull a
                                // rabbit out of the hat here
                                sName = sFieldMarkName + OUString::number(
                                    m_xImpl->GetFieldMarkIndex(xFormField));
                            }
                            GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_NAME,
                                    sName);
                            SvXMLElementExport aElem( GetExport(), !bAutoStyles,
                                XML_NAMESPACE_TEXT, XML_BOOKMARK_END,
                                false, false );
                        }
                    }
                }
            }
            else if (sType == sTextFieldStartEnd)
            {
                if (!bAutoStyles)
                {
                    if (GetExport().getDefaultVersion() > SvtSaveOptions::ODFVER_012)
                    {
                        Reference<XNamed> xBookmark(xPropSet->getPropertyValue(sBookmark), UNO_QUERY);
                        if (xBookmark.is())
                        {
                            GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_NAME, xBookmark->getName());
                        }
                        Reference< css::text::XFormField > xFormField(xPropSet->getPropertyValue(sBookmark), UNO_QUERY);
                        if (xFormField.is())
                        {
                            GetExport().AddAttribute(XML_NAMESPACE_FIELD, XML_TYPE, xFormField->getFieldType());
                        }
                        GetExport().StartElement(XML_NAMESPACE_FIELD, XML_FIELDMARK, false);
                        if (xFormField.is())
                        {
                            FieldParamExporter(&GetExport(), xFormField->getParameters()).Export();
                        }
                        GetExport().EndElement(XML_NAMESPACE_FIELD, XML_FIELDMARK, false);
                    }
                    else
                    {
                        Reference<XNamed> xBookmark(xPropSet->getPropertyValue(sBookmark), UNO_QUERY);
                        if (xBookmark.is())
                        {
                            GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_NAME, xBookmark->getName());
                            SvXMLElementExport aElem( GetExport(), !bAutoStyles,
                                XML_NAMESPACE_TEXT, XML_BOOKMARK,
                                false, false );
                        }
                    }
                }
            }
            else if (sType == sSoftPageBreak)
            {
                exportSoftPageBreak();
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
                exportTextField(xTxtRange, bAutoStyles, bIsProgress, &rPrevCharIsSpace);
            }
            else
            {
                // no TextPortionType property -> non-Writer app -> text
                exportTextRange(xTxtRange, bAutoStyles, rPrevCharIsSpace, openFieldMark);
            }
        }
    }

// now that there are nested enumerations for meta(-field), this may be valid!
//  SAL_WARN_IF( bOpenRuby, "xmloff", "Red Alert: Ruby still open!" );
}

void XMLTextParagraphExport::exportTable(
        const Reference < XTextContent > &,
        bool /*bAutoStyles*/, bool /*bIsProgress*/ )
{
}

void XMLTextParagraphExport::exportTextField(
        const Reference < XTextRange > & rTextRange,
        bool bAutoStyles, bool bIsProgress, bool *const pPrevCharIsSpace)
{
    Reference < XPropertySet > xPropSet( rTextRange, UNO_QUERY );
    // non-Writer apps need not support Property TextField, so test first
    if (xPropSet->getPropertySetInfo()->hasPropertyByName( sTextField ))
    {
        Reference < XTextField > xTxtFld(xPropSet->getPropertyValue( sTextField ), uno::UNO_QUERY);
        SAL_WARN_IF( !xTxtFld.is(), "xmloff", "text field missing" );
        if( xTxtFld.is() )
        {
            exportTextField(xTxtFld, bAutoStyles, bIsProgress, true, pPrevCharIsSpace);
        }
        else
        {
            // write only characters
            GetExport().Characters(rTextRange->getString());
        }
    }
}

void XMLTextParagraphExport::exportTextField(
        const Reference < XTextField > & xTextField,
        const bool bAutoStyles, const bool bIsProgress,
        const bool bRecursive, bool *const pPrevCharIsSpace)
{
    if ( bAutoStyles )
    {
        pFieldExport->ExportFieldAutoStyle( xTextField, bIsProgress,
                bRecursive );
    }
    else
    {
        assert(pPrevCharIsSpace);
        pFieldExport->ExportField(xTextField, bIsProgress, *pPrevCharIsSpace);
    }
}

void XMLTextParagraphExport::exportSoftPageBreak()
{
    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                              XML_SOFT_PAGE_BREAK, false,
                              false );
}

void XMLTextParagraphExport::exportTextMark(
    const Reference<XPropertySet> & rPropSet,
    const OUString& rProperty,
    const enum XMLTokenEnum pElements[],
    bool bAutoStyles)
{
    // mib said: "Hau wech!"

    // (Originally, I'd export a span element in case the (book|reference)mark
    //  was formatted. This actually makes a difference in case some pervert
    //  sets a point reference mark in the document and, say, formats it bold.
    //  This basically meaningless formatting will now been thrown away
    //  (aka cleaned up), since mib said: ...                   dvo

     if (!bAutoStyles)
    {
        // name element
        Reference<XNamed> xName(rPropSet->getPropertyValue(rProperty), UNO_QUERY);
        GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_NAME,
                                 xName->getName());

        // start, end, or point-reference?
        sal_Int8 nElement;
        if( *o3tl::doAccess<bool>(rPropSet->getPropertyValue(sIsCollapsed)) )
        {
            nElement = 0;
        }
        else
        {
            nElement = *o3tl::doAccess<bool>(rPropSet->getPropertyValue(sIsStart)) ? 1 : 2;
        }

        // bookmark, bookmark-start: xml:id and RDFa for RDF metadata
        if( nElement < 2 ) {
            GetExport().AddAttributeXmlId(xName);
            const uno::Reference<text::XTextContent> xTextContent(
                    xName, uno::UNO_QUERY_THROW);
            GetExport().AddAttributesRDFa(xTextContent);
        }

        // bookmark-start: add attributes hidden and condition
        if (nElement == 1)
        {
            Reference<XPropertySet> bkmkProps(rPropSet->getPropertyValue(rProperty), UNO_QUERY);
            Reference<XPropertySetInfo> bkmkPropInfo(bkmkProps->getPropertySetInfo(), UNO_QUERY);
            OUString sHidden("BookmarkHidden");
            if (bkmkPropInfo->hasPropertyByName(sHidden))
            {
                bool bHidden = false;
                bkmkProps->getPropertyValue(sHidden) >>= bHidden;
                if (bHidden)
                {
                    GetExport().AddAttribute(XML_NAMESPACE_LO_EXT, "hidden", "true");
                    OUString sCondition("BookmarkCondition");
                    if (bkmkPropInfo->hasPropertyByName(sCondition))
                    {
                        OUString sBookmarkCondition;
                        bkmkProps->getPropertyValue(sCondition) >>= sBookmarkCondition;
                        GetExport().AddAttribute(XML_NAMESPACE_LO_EXT, "condition", sBookmarkCondition);
                    }
                }
            }
        }

        // export element
        assert(pElements != nullptr);
        assert(0 <= nElement && nElement <= 2);
        SvXMLElementExport aElem(GetExport(),
                                 XML_NAMESPACE_TEXT, pElements[nElement],
                                 false, false);
    }
    // else: no styles. (see above)
}

static bool lcl_txtpara_isBoundAsChar(
        const Reference < XPropertySet > & rPropSet,
        const Reference < XPropertySetInfo > & rPropSetInfo )
{
    bool bIsBoundAsChar = false;
    OUString sAnchorType( "AnchorType"  );
    if( rPropSetInfo->hasPropertyByName( sAnchorType ) )
    {
        TextContentAnchorType eAnchor;
        rPropSet->getPropertyValue( sAnchorType ) >>= eAnchor;
        bIsBoundAsChar = TextContentAnchorType_AS_CHARACTER == eAnchor;
    }

    return bIsBoundAsChar;
}

XMLShapeExportFlags XMLTextParagraphExport::addTextFrameAttributes(
    const Reference < XPropertySet >& rPropSet,
    bool bShape,
    basegfx::B2DPoint* pCenter,
    OUString* pMinHeightValue,
    OUString* pMinWidthValue)
{
    XMLShapeExportFlags nShapeFeatures = SEF_DEFAULT;

    // draw:name (#97662#: not for shapes, since those names will be
    // treated in the shape export)
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

    // text:anchor-type
    TextContentAnchorType eAnchor = TextContentAnchorType_AT_PARAGRAPH;
    rPropSet->getPropertyValue( sAnchorType ) >>= eAnchor;
    {
        XMLAnchorTypePropHdl aAnchorTypeHdl;
        OUString sTmp;
        aAnchorTypeHdl.exportXML( sTmp, uno::makeAny(eAnchor),
                                  GetExport().GetMM100UnitConverter() );
        GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_ANCHOR_TYPE, sTmp );
    }

    // text:anchor-page-number
    if( TextContentAnchorType_AT_PAGE == eAnchor )
    {
        sal_Int16 nPage = 0;
        rPropSet->getPropertyValue( sAnchorPageNo ) >>= nPage;
        SAL_WARN_IF(nPage <= 0, "xmloff",
                "ERROR: writing invalid anchor-page-number 0");
        GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_ANCHOR_PAGE_NUMBER,
                                  OUString::number( nPage ) );
    }
    else
    {
        nShapeFeatures |= XMLShapeExportFlags::NO_WS;
    }

    // OD 2004-06-01 #i27691# - correction: no export of svg:x, if object
    // is anchored as-character.
    if ( !bShape &&
         eAnchor != TextContentAnchorType_AS_CHARACTER )
    {
        // svg:x
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
            if(nullptr != pCenter)
            {
                // add left edge to Center
                pCenter->setX(pCenter->getX() + nPos);
            }
        }
    }
    else if( TextContentAnchorType_AS_CHARACTER == eAnchor )
        nShapeFeatures = (nShapeFeatures & ~XMLShapeExportFlags::X);

    if( !bShape || TextContentAnchorType_AS_CHARACTER == eAnchor  )
    {
        // svg:y
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
            if(nullptr != pCenter)
            {
                // add top edge to Center
                pCenter->setY(pCenter->getY() + nPos);
            }
        }
        if( bShape )
            nShapeFeatures = (nShapeFeatures & ~XMLShapeExportFlags::Y);
    }

    Reference< XPropertySetInfo > xPropSetInfo(rPropSet->getPropertySetInfo());

    // svg:width
    sal_Int16 nWidthType = SizeType::FIX;
    if( xPropSetInfo->hasPropertyByName( sWidthType ) )
    {
        rPropSet->getPropertyValue( sWidthType ) >>= nWidthType;
    }
    if( xPropSetInfo->hasPropertyByName( sWidth ) )
    {
        sal_Int32 nWidth =  0;
        // VAR size will be written as zero min-size
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
        {
            GetExport().AddAttribute( XML_NAMESPACE_SVG, XML_WIDTH,
                                      sValue.makeStringAndClear() );
            if(nullptr != pCenter)
            {
                // add half width to Center
                pCenter->setX(pCenter->getX() + (0.5 * nWidth));
            }
        }
    }
    bool bSyncWidth = false;
    if( xPropSetInfo->hasPropertyByName( sIsSyncWidthToHeight ) )
    {
        bSyncWidth = *o3tl::doAccess<bool>(rPropSet->getPropertyValue( sIsSyncWidthToHeight ));
        if( bSyncWidth )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_REL_WIDTH,
                                      XML_SCALE );
    }
    if( !bSyncWidth && xPropSetInfo->hasPropertyByName( sRelativeWidth ) )
    {
        sal_Int16 nRelWidth =  0;
        rPropSet->getPropertyValue( sRelativeWidth ) >>= nRelWidth;
        SAL_WARN_IF( nRelWidth < 0 || nRelWidth > 254, "xmloff",
                    "Got illegal relative width from API" );
        if( nRelWidth > 0 )
        {
            ::sax::Converter::convertPercent( sValue, nRelWidth );
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_REL_WIDTH,
                                      sValue.makeStringAndClear() );
        }
    }

    // svg:height, fo:min-height or style:rel-height
    sal_Int16 nSizeType = SizeType::FIX;
    if( xPropSetInfo->hasPropertyByName( sSizeType ) )
    {
        rPropSet->getPropertyValue( sSizeType ) >>= nSizeType;
    }
    bool bSyncHeight = false;
    if( xPropSetInfo->hasPropertyByName( sIsSyncHeightToWidth ) )
    {
        bSyncHeight = *o3tl::doAccess<bool>(rPropSet->getPropertyValue( sIsSyncHeightToWidth ));
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
        {
            *pMinHeightValue = sValue.makeStringAndClear();
        }
        else
        {
            GetExport().AddAttribute( XML_NAMESPACE_SVG, XML_HEIGHT,
                                      sValue.makeStringAndClear() );
            if(nullptr != pCenter)
            {
                // add half height to Center
                pCenter->setY(pCenter->getY() + (0.5 * nHeight));
            }
        }
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
            GetExport().AddAttribute( XML_NAMESPACE_DRAW, XML_ZINDEX,
                                      OUString::number( nZIndex ) );
        }
    }

    return nShapeFeatures;
}

void XMLTextParagraphExport::exportAnyTextFrame(
        const Reference < XTextContent > & rTxtCntnt,
        FrameType eType,
        bool bAutoStyles,
        bool bIsProgress,
        bool bExportContent,
        const Reference < XPropertySet > *pRangePropSet)
{
    Reference < XPropertySet > xPropSet( rTxtCntnt, UNO_QUERY );

    if( bAutoStyles )
    {
        if( FrameType::Embedded == eType )
            _collectTextEmbeddedAutoStyles( xPropSet );
        // No text frame style for shapes (#i28745#)
        else if ( FrameType::Shape != eType )
            Add( XML_STYLE_FAMILY_TEXT_FRAME, xPropSet );

        if( pRangePropSet && lcl_txtpara_isBoundAsChar( xPropSet,
                                            xPropSet->getPropertySetInfo() ) )
            Add( XML_STYLE_FAMILY_TEXT_TEXT, *pRangePropSet );

        switch( eType )
        {
        case FrameType::Text:
            {
                // frame bound frames
                if ( bExportContent )
                {
                    Reference < XTextFrame > xTxtFrame( rTxtCntnt, UNO_QUERY );
                    Reference < XText > xTxt(xTxtFrame->getText());
                    exportFrameFrames( true, bIsProgress, &xTxtFrame );
                    exportText( xTxt, bAutoStyles, bIsProgress, true );
                }
            }
            break;
        case FrameType::Shape:
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
            bool bAddCharStyles = pRangePropSet &&
                lcl_txtpara_isBoundAsChar( xPropSet, xPropSetInfo );

            bool bIsUICharStyle;
            bool bHasAutoStyle = false;

            OUString sStyle;

            if( bAddCharStyles )
            {
                bool bDummy;
                sStyle = FindTextStyleAndHyperlink( *pRangePropSet, bDummy, bIsUICharStyle, bHasAutoStyle );
            }
            else
                bIsUICharStyle = false;

            bool bDoSomething = bIsUICharStyle
                && aCharStyleNamesPropInfoCache.hasProperty( *pRangePropSet );
            XMLTextCharStyleNamesElementExport aCharStylesExport(
                GetExport(), bDoSomething, bHasAutoStyle,
                bDoSomething ? *pRangePropSet : Reference<XPropertySet>(),
                sCharStyleNames );

            if( !sStyle.isEmpty() )
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_STYLE_NAME,
                                  GetExport().EncodeStyleName( sStyle ) );
            {
                SvXMLElementExport aElem( GetExport(), !sStyle.isEmpty(),
                    XML_NAMESPACE_TEXT, XML_SPAN, false, false );
                {
                    SvXMLElementExport aElement( GetExport(),
                        FrameType::Shape != eType &&
                        addHyperlinkAttributes( xPropSet,
                                                xPropState,xPropSetInfo ),
                        XML_NAMESPACE_DRAW, XML_A, false, false );
                    switch( eType )
                    {
                    case FrameType::Text:
                        _exportTextFrame( xPropSet, xPropSetInfo, bIsProgress );
                        break;
                    case FrameType::Graphic:
                        _exportTextGraphic( xPropSet, xPropSetInfo );
                        break;
                    case FrameType::Embedded:
                        _exportTextEmbedded( xPropSet, xPropSetInfo );
                        break;
                    case FrameType::Shape:
                        {
                            Reference < XShape > xShape( rTxtCntnt, UNO_QUERY );
                            XMLShapeExportFlags nFeatures =
                                addTextFrameAttributes( xPropSet, true );
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
        bool bIsProgress )
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
    addTextFrameAttributes(rPropSet, false, nullptr, &aMinHeightValue, &sMinWidthValue);

    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_DRAW,
                              XML_FRAME, false, true );

    if( !aMinHeightValue.isEmpty() )
        GetExport().AddAttribute( XML_NAMESPACE_FO, XML_MIN_HEIGHT,
                                  aMinHeightValue );

    if (!sMinWidthValue.isEmpty())
    {
        GetExport().AddAttribute( XML_NAMESPACE_FO, XML_MIN_WIDTH,
                                  sMinWidthValue );
    }

    // draw:chain-next-name
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
                                  XML_TEXT_BOX, true, true );

        // frames bound to frame
        exportFrameFrames( false, bIsProgress, &xTxtFrame );

        exportText( xTxt, false, bIsProgress, true );
    }

    // script:events
    Reference<XEventsSupplier> xEventsSupp( xTxtFrame, UNO_QUERY );
    GetExport().GetEventExport().Export(xEventsSupp);

    // image map
    GetExport().GetImageMapExport().Export( rPropSet );

    // svg:title and svg:desc (#i73249#)
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
        basegfx::utils::UnoPointSequenceSequenceToB2DPolyPolygon(
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
        bPixel = *o3tl::doAccess<bool>(rPropSet->getPropertyValue( sIsPixelContour ));
    }

    // svg: width
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

    // svg: height
    if(bPixel)
    {
        ::sax::Converter::convertMeasurePx(aStringBuffer, basegfx::fround(aPolyPolygonRange.getHeight()));
    }
    else
    {
        GetExport().GetMM100UnitConverter().convertMeasureToXML(aStringBuffer, basegfx::fround(aPolyPolygonRange.getHeight()));
    }

    GetExport().AddAttribute(XML_NAMESPACE_SVG, XML_HEIGHT, aStringBuffer.makeStringAndClear());

    // svg:viewbox
    SdXMLImExViewBox aViewBox(0.0, 0.0, aPolyPolygonRange.getWidth(), aPolyPolygonRange.getHeight());
    GetExport().AddAttribute(XML_NAMESPACE_SVG, XML_VIEWBOX, aViewBox.GetExportString());
    enum XMLTokenEnum eElem = XML_TOKEN_INVALID;

    if(1 == nPolygonCount )
    {
        // simple polygon shape, can be written as svg:points sequence
        const OUString aPointString(
            basegfx::utils::exportToSvgPoints(
                aPolyPolygon.getB2DPolygon(0)));

        // write point array
        GetExport().AddAttribute(XML_NAMESPACE_DRAW, XML_POINTS, aPointString);
        eElem = XML_CONTOUR_POLYGON;
    }
    else
    {
        // polypolygon, needs to be written as a svg:path sequence
        const OUString aPolygonString(
            basegfx::utils::exportToSvgD(
                aPolyPolygon,
                true,           // bUseRelativeCoordinates
                false,          // bDetectQuadraticBeziers: not used in old, but maybe activated now
                true));         // bHandleRelativeNextPointCompatible

        // write point array
        GetExport().AddAttribute( XML_NAMESPACE_SVG, XML_D, aPolygonString);
        eElem = XML_CONTOUR_PATH;
    }

    if( rPropSetInfo->hasPropertyByName( sIsAutomaticContour ) )
    {
        bool bTmp = *o3tl::doAccess<bool>(rPropSet->getPropertyValue(
                                            sIsAutomaticContour ));
        GetExport().AddAttribute( XML_NAMESPACE_DRAW,
                      XML_RECREATE_ON_EDIT, bTmp ? XML_TRUE : XML_FALSE );
    }

    // write object now
    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_DRAW, eElem,
                              true, true );
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

    // check if we need to use svg:transform
    sal_Int16 nRotation(0);
    rPropSet->getPropertyValue( sGraphicRotation ) >>= nRotation;
    const bool bUseRotation(0 != nRotation);
    basegfx::B2DPoint aCenter(0.0, 0.0);

    // add TextFrame attributes like svg:x/y/width/height, also get back
    // object's center point if rotation is used and has to be exported
    addTextFrameAttributes(rPropSet, false, bUseRotation ? &aCenter : nullptr);

    // svg:transform
    if(bUseRotation)
    {
        // RotateFlyFrameFix: im/export full 'draw:transform' using existing tooling.
        // Currently only rotation is used, but combinations with 'draw:transform'
        // may be necessary in the future, so that svg:x/svg:y/svg:width/svg:height
        // may be extended/replaced with 'draw:transform' (see draw objects)
        SdXMLImExTransform2D aSdXMLImExTransform2D;

        // Convert from 10th degree integer to deg.
        // CAUTION: internal rotation is classically mathematically 'wrong' defined by ignoring that
        // we have a right-handed coordinate system, so need to correct this by mirroring
        // the rotation to get the correct transformation. See also case XML_TOK_TEXT_FRAME_TRANSFORM
        // in XMLTextFrameContext_Impl::XMLTextFrameContext_Impl and #i78696#
        // CAUTION-II: due to tdf#115782 it is better for current ODF to indeed write it with the wrong
        // orientation as in all other cases - ARGH! We will need to correct this in future ODF ASAP!
        const double fRotate(static_cast< double >(nRotation) * (F_PI/1800.0));

        // transform to rotation center which is the object's center
        aSdXMLImExTransform2D.AddTranslate(-aCenter);

        // add rotation itself
        // tdf#115529 but correct value modulo 2PI to have it positive and in the range of [0.0 .. 2PI[
        aSdXMLImExTransform2D.AddRotate(basegfx::normalizeToRange(fRotate, F_2PI));

        // back-transform after rotation
        aSdXMLImExTransform2D.AddTranslate(aCenter);

        // Note: using GetTwipUnitConverter instead of GetMM100UnitConverter may be needed,
        // but is not generally available (as it should be, a 'current' UnitConverter should
        // be available at GetExport() - and maybe was once). May have to be addressed as soon
        // as translate transformations are used here.
        GetExport().AddAttribute(
            XML_NAMESPACE_DRAW,
            XML_TRANSFORM,
            aSdXMLImExTransform2D.GetExportString(GetExport().GetMM100UnitConverter()));
    }

    // original content
    SvXMLElementExport aElem(GetExport(), XML_NAMESPACE_DRAW, XML_FRAME, false, true);

    {
        // xlink:href
        uno::Reference<graphic::XGraphic> xGraphic;
        rPropSet->getPropertyValue("Graphic") >>= xGraphic;

        OUString sInternalURL;
        OUString sOutMimeType;

        if (xGraphic.is())
        {
            sInternalURL = GetExport().AddEmbeddedXGraphic(xGraphic, sOutMimeType);
        }

        // If there still is no url, then graphic is empty
        if (!sInternalURL.isEmpty())
        {
            GetExport().AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, sInternalURL);
            GetExport().AddAttribute(XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE);
            GetExport().AddAttribute(XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED);
            GetExport().AddAttribute(XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD);
        }

        // draw:filter-name
        OUString sGrfFilter;
        rPropSet->getPropertyValue( sGraphicFilter ) >>= sGrfFilter;
        if( !sGrfFilter.isEmpty() )
            GetExport().AddAttribute( XML_NAMESPACE_DRAW, XML_FILTER_NAME,
                                      sGrfFilter );

        if (GetExport().getDefaultVersion() > SvtSaveOptions::ODFVER_012)
        {
            if (sOutMimeType.isEmpty())
            {
                GetExport().GetGraphicMimeTypeFromStream(xGraphic, sOutMimeType);
            }
            if (!sOutMimeType.isEmpty())
            {
                GetExport().AddAttribute(XML_NAMESPACE_LO_EXT, "mime-type", sOutMimeType);
            }
        }


        // optional office:binary-data
        if (xGraphic.is())
        {
            SvXMLElementExport aElement(GetExport(), XML_NAMESPACE_DRAW, XML_IMAGE, false, true );
            GetExport().AddEmbeddedXGraphicAsBase64(xGraphic);
        }
    }

    const bool bAddReplacementImages = officecfg::Office::Common::Save::Graphic::AddReplacementImages::get();
    if (bAddReplacementImages)
    {
        // replacement graphic for backwards compatibility, but
        // only for SVG and metafiles currently
        uno::Reference<graphic::XGraphic> xReplacementGraphic;
        rPropSet->getPropertyValue("ReplacementGraphic") >>= xReplacementGraphic;

        OUString sInternalURL;
        OUString sOutMimeType;

        //Resolves: fdo#62461 put preferred image first above, followed by
        //fallback here
        if (xReplacementGraphic.is())
        {
            sInternalURL = GetExport().AddEmbeddedXGraphic(xReplacementGraphic, sOutMimeType);
        }

        // If there is no url, then graphic is empty
        if (!sInternalURL.isEmpty())
        {
            GetExport().AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, sInternalURL);
            GetExport().AddAttribute(XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE);
            GetExport().AddAttribute(XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED);
            GetExport().AddAttribute(XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD);
        }

        if (GetExport().getDefaultVersion() > SvtSaveOptions::ODFVER_012)
        {
            if (sOutMimeType.isEmpty())
            {
                GetExport().GetGraphicMimeTypeFromStream(xReplacementGraphic, sOutMimeType);
            }
            if (!sOutMimeType.isEmpty())
            {
                GetExport().AddAttribute(XML_NAMESPACE_LO_EXT, "mime-type", sOutMimeType);
            }
        }


        // optional office:binary-data
        if (xReplacementGraphic.is())
        {
            SvXMLElementExport aElement(GetExport(), XML_NAMESPACE_DRAW, XML_IMAGE, true, true);
            GetExport().AddEmbeddedXGraphicAsBase64(xReplacementGraphic);
        }
    }

    // script:events
    Reference<XEventsSupplier> xEventsSupp( rPropSet, UNO_QUERY );
    GetExport().GetEventExport().Export(xEventsSupp);

    // image map
    GetExport().GetImageMapExport().Export( rPropSet );

    // svg:title and svg:desc (#i73249#)
    exportTitleAndDescription( rPropSet, rPropSetInfo );

    // draw:contour
    exportContour( rPropSet, rPropSetInfo );
}

void XMLTextParagraphExport::_collectTextEmbeddedAutoStyles(const Reference < XPropertySet > & )
{
    SAL_WARN( "xmloff", "no API implementation available" );
}

void XMLTextParagraphExport::_exportTextEmbedded(
        const Reference < XPropertySet > &,
        const Reference < XPropertySetInfo > & )
{
    SAL_WARN( "xmloff", "no API implementation available" );
}

void XMLTextParagraphExport::exportEvents( const Reference < XPropertySet > & rPropSet )
{
    // script:events
    Reference<XEventsSupplier> xEventsSupp( rPropSet, UNO_QUERY );
    GetExport().GetEventExport().Export(xEventsSupp);

    // image map
    if (rPropSet->getPropertySetInfo()->hasPropertyByName("ImageMap"))
        GetExport().GetImageMapExport().Export( rPropSet );
}

// Implement Title/Description Elements UI (#i73249#)
void XMLTextParagraphExport::exportTitleAndDescription(
        const Reference < XPropertySet > & rPropSet,
        const Reference < XPropertySetInfo > & rPropSetInfo )
{
    // svg:title
    if( rPropSetInfo->hasPropertyByName( sTitle ) )
    {
        OUString sObjTitle;
        rPropSet->getPropertyValue( sTitle ) >>= sObjTitle;
        if( !sObjTitle.isEmpty() )
        {
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_SVG,
                                      XML_TITLE, true, false );
            GetExport().Characters( sObjTitle );
        }
    }

    // svg:description
    if( rPropSetInfo->hasPropertyByName( sDescription ) )
    {
        OUString sObjDesc;
        rPropSet->getPropertyValue( sDescription ) >>= sObjDesc;
        if( !sObjDesc.isEmpty() )
        {
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_SVG,
                                      XML_DESC, true, false );
            GetExport().Characters( sObjDesc );
        }
    }
}

bool XMLTextParagraphExport::addHyperlinkAttributes(
    const Reference< XPropertySet > & rPropSet,
    const Reference< XPropertyState > & rPropState,
    const Reference< XPropertySetInfo > & rPropSetInfo )
{
    bool bExport = false;
    OUString sHRef, sName, sTargetFrame, sUStyleName, sVStyleName;
    bool bServerMap = false;

    if( rPropSetInfo->hasPropertyByName( sHyperLinkURL ) &&
        ( !rPropState.is() || PropertyState_DIRECT_VALUE ==
                    rPropState->getPropertyState( sHyperLinkURL ) ) )
    {
        rPropSet->getPropertyValue( sHyperLinkURL ) >>= sHRef;

        if( !sHRef.isEmpty() )
            bExport = true;
    }

    if ( sHRef.isEmpty() )
    {
        // hyperlink without an URL does not make sense
        OSL_ENSURE( false, "hyperlink without an URL --> no export to ODF" );
        return false;
    }

    if ( rPropSetInfo->hasPropertyByName( sHyperLinkName )
         && ( !rPropState.is()
              || PropertyState_DIRECT_VALUE == rPropState->getPropertyState( sHyperLinkName ) ) )
    {
        rPropSet->getPropertyValue( sHyperLinkName ) >>= sName;
        if( !sName.isEmpty() )
            bExport = true;
    }

    if ( rPropSetInfo->hasPropertyByName( sHyperLinkTarget )
         && ( !rPropState.is()
              || PropertyState_DIRECT_VALUE == rPropState->getPropertyState( sHyperLinkTarget ) ) )
    {
        rPropSet->getPropertyValue( sHyperLinkTarget ) >>= sTargetFrame;
        if( !sTargetFrame.isEmpty() )
            bExport = true;
    }

    if ( rPropSetInfo->hasPropertyByName( sServerMap )
         && ( !rPropState.is()
              || PropertyState_DIRECT_VALUE == rPropState->getPropertyState( sServerMap ) ) )
    {
        bServerMap = *o3tl::doAccess<bool>(rPropSet->getPropertyValue( sServerMap ));
        if ( bServerMap )
            bExport = true;
    }

    if ( rPropSetInfo->hasPropertyByName( sUnvisitedCharStyleName )
         && ( !rPropState.is()
              || PropertyState_DIRECT_VALUE == rPropState->getPropertyState( sUnvisitedCharStyleName ) ) )
    {
        rPropSet->getPropertyValue( sUnvisitedCharStyleName ) >>= sUStyleName;
        if( !sUStyleName.isEmpty() )
            bExport = true;
    }

    if ( rPropSetInfo->hasPropertyByName( sVisitedCharStyleName )
         && ( !rPropState.is()
              || PropertyState_DIRECT_VALUE == rPropState->getPropertyState( sVisitedCharStyleName ) ) )
    {
        rPropSet->getPropertyValue( sVisitedCharStyleName ) >>= sVStyleName;
        if( !sVStyleName.isEmpty() )
            bExport = true;
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
    const css::uno::Reference< css::text::XTextRange > & rTextRange,
    Reference< XPropertySet > const & xPropSet,
    Reference < XPropertySetInfo > & xPropSetInfo,
    const bool bIsUICharStyle,
    const bool bHasAutoStyle,
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
        SvXMLElementExport aElement( GetExport(), !sStyle.isEmpty(), XML_NAMESPACE_TEXT, XML_SPAN, false, false );
        const OUString aText( rTextRange->getString() );
        SvXMLElementExport aElem2( GetExport(), TEXT == openFieldMark,
            XML_NAMESPACE_TEXT, XML_TEXT_INPUT,
            false, false );
        exportCharacterData(aText, rPrevCharIsSpace);
        openFieldMark = NONE;
    }
}

void XMLTextParagraphExport::exportTextRange(
        const Reference< XTextRange > & rTextRange,
        bool bAutoStyles,
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
        bool bHyperlink = false;
        bool bIsUICharStyle = false;
        bool bHasAutoStyle = false;
        const OUString sStyle(
            FindTextStyleAndHyperlink( xPropSet, bHyperlink, bIsUICharStyle, bHasAutoStyle ) );

        Reference < XPropertySetInfo > xPropSetInfo;
        bool bHyperlinkAttrsAdded = false;
        if ( bHyperlink )
        {
            Reference< XPropertyState > xPropState( xPropSet, UNO_QUERY );
            xPropSetInfo.set( xPropSet->getPropertySetInfo() );
            bHyperlinkAttrsAdded = addHyperlinkAttributes( xPropSet, xPropState, xPropSetInfo );
        }

        if ( bHyperlink && bHyperlinkAttrsAdded )
        {
            SvXMLElementExport aElem( GetExport(), true, XML_NAMESPACE_TEXT, XML_A, false, false );

            // export events (if supported)
            OUString sHyperLinkEvents(
                "HyperLinkEvents");
            if (xPropSetInfo->hasPropertyByName(sHyperLinkEvents))
            {
                Reference< XNameReplace > xName( xPropSet->getPropertyValue( sHyperLinkEvents ), uno::UNO_QUERY );
                GetExport().GetEventExport().Export( xName, false );
            }

            exportTextRangeSpan( rTextRange, xPropSet, xPropSetInfo, bIsUICharStyle, bHasAutoStyle, sStyle, rPrevCharIsSpace, openFieldMark );
        }
        else
        {
            exportTextRangeSpan( rTextRange, xPropSet, xPropSetInfo, bIsUICharStyle, bHasAutoStyle, sStyle, rPrevCharIsSpace, openFieldMark );
        }
    }
}

void XMLTextParagraphExport::exportCharacterData(const OUString& rText,
                                           bool& rPrevCharIsSpace )
{
    sal_Int32 nExpStartPos = 0;
    sal_Int32 nEndPos = rText.getLength();
    sal_Int32 nSpaceChars = 0;
    for( sal_Int32 nPos = 0; nPos < nEndPos; nPos++ )
    {
        sal_Unicode cChar = rText[nPos];
        bool bExpCharAsText = true;
        bool bExpCharAsElement = false;
        bool bCurrCharIsSpace = false;
        switch( cChar )
        {
        case 0x0009:    // Tab
        case 0x000A:    // LF
            // These characters are exported as text.
            bExpCharAsElement = true;
            bExpCharAsText = false;
            break;
        case 0x000D:
            break;  // legal character
        case 0x0020:    // Blank
            if( rPrevCharIsSpace )
            {
                // If the previous character is a space character,
                // too, export a special space element.
                bExpCharAsText = false;
            }
            bCurrCharIsSpace = true;
            break;
        default:
            if( cChar < 0x0020 )
            {
#ifdef DBG_UTIL
                OSL_ENSURE( txtparae_bContainsIllegalCharacters ||
                            cChar >= 0x0020,
                            "illegal character in text content" );
                txtparae_bContainsIllegalCharacters = true;
#endif
                bExpCharAsText = false;
            }
            break;
        }

        // If the current character is not exported as text
           // the text that has not been exported by now has to be exported now.
        if( nPos > nExpStartPos && !bExpCharAsText )
        {
            SAL_WARN_IF( 0 != nSpaceChars, "xmloff", "pending spaces" );
            OUString sExp( rText.copy( nExpStartPos, nPos - nExpStartPos ) );
            GetExport().Characters( sExp );
            nExpStartPos = nPos;
        }

        // If there are spaces left that have not been exported and the
        // current character is not a space , the pending spaces have to be
        // exported now.
        if( nSpaceChars > 0 && !bCurrCharIsSpace )
        {
            SAL_WARN_IF( nExpStartPos != nPos, "xmloff", " pending characters" );

            if( nSpaceChars > 1 )
            {
                OUStringBuffer sTmp;
                sTmp.append( nSpaceChars );
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_C,
                              sTmp.makeStringAndClear() );
            }

            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                      XML_S, false, false );

            nSpaceChars = 0;
        }

        // If the current character has to be exported as a special
        // element, the element will be exported now.
        if( bExpCharAsElement )
        {
            switch( cChar )
            {
            case 0x0009:    // Tab
                {
                    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                              XML_TAB, false,
                                              false );
                }
                break;
            case 0x000A:    // LF
                {
                    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                              XML_LINE_BREAK, false,
                                              false );
                }
                break;
            }
        }

        // If the current character is a space, and the previous one
        // is a space, too, the number of pending spaces is incremented
        // only.
        if( bCurrCharIsSpace && rPrevCharIsSpace )
            nSpaceChars++;
        rPrevCharIsSpace = bCurrCharIsSpace;

        // If the current character is not exported as text, the start
        // position for text is the position behind the current position.
        if( !bExpCharAsText )
        {
            SAL_WARN_IF( nExpStartPos != nPos, "xmloff", "wrong export start pos" );
            nExpStartPos = nPos+1;
        }
    }

    if( nExpStartPos < nEndPos )
    {
        SAL_WARN_IF( 0 != nSpaceChars, "xmloff", " pending spaces " );
        OUString sExp( rText.copy( nExpStartPos, nEndPos - nExpStartPos ) );
        GetExport().Characters( sExp );
    }

    // If there are some spaces left, they have to be exported now.
    if( nSpaceChars > 0 )
    {
        if( nSpaceChars > 1 )
        {
            OUStringBuffer sTmp;
            sTmp.append( nSpaceChars );
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_C,
                          sTmp.makeStringAndClear() );
        }

        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT, XML_S,
                                  false, false );
    }
}

void XMLTextParagraphExport::exportTextDeclarations()
{
    pFieldExport->ExportFieldDeclarations();

    // get XPropertySet from the document and ask for AutoMarkFileURL.
    // If it exists, export the auto-mark-file element.
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
                    true, true );
            }
        }
    }
}

void XMLTextParagraphExport::exportTextDeclarations(
    const Reference<XText> & rText )
{
    pFieldExport->ExportFieldDeclarations(rText);
}

void XMLTextParagraphExport::exportUsedDeclarations()
{
    pFieldExport->SetExportOnlyUsedFieldDeclarations( false/*bOnlyUsed*/ );
}

void XMLTextParagraphExport::exportTrackedChanges(bool bAutoStyles)
{
    if (nullptr != pRedlineExport)
        pRedlineExport->ExportChangesList( bAutoStyles );
}

void XMLTextParagraphExport::exportTrackedChanges(
    const Reference<XText> & rText,
    bool bAutoStyle)
{
    if (nullptr != pRedlineExport)
        pRedlineExport->ExportChangesList(rText, bAutoStyle);
}

void XMLTextParagraphExport::recordTrackedChangesForXText(
    const Reference<XText> & rText )
{
    if (nullptr != pRedlineExport)
        pRedlineExport->SetCurrentXText(rText);
}

void XMLTextParagraphExport::recordTrackedChangesNoXText()
{
    if (nullptr != pRedlineExport)
        pRedlineExport->SetCurrentXText();
}

void XMLTextParagraphExport::exportTextAutoStyles()
{
    GetAutoStylePool().exportXML( XML_STYLE_FAMILY_TEXT_PARAGRAPH );

    GetAutoStylePool().exportXML( XML_STYLE_FAMILY_TEXT_TEXT );

    GetAutoStylePool().exportXML( XML_STYLE_FAMILY_TEXT_FRAME );

    GetAutoStylePool().exportXML( XML_STYLE_FAMILY_TEXT_SECTION );

    GetAutoStylePool().exportXML( XML_STYLE_FAMILY_TEXT_RUBY );

    maListAutoPool.exportXML();
}

void XMLTextParagraphExport::exportRuby(
    const Reference<XPropertySet> & rPropSet,
    bool bAutoStyles )
{
    // early out: a collapsed ruby makes no sense
    if (*o3tl::doAccess<bool>(rPropSet->getPropertyValue(sIsCollapsed)))
        return;

    // start value ?
    bool bStart = *o3tl::doAccess<bool>(rPropSet->getPropertyValue(sIsStart));

    if (bAutoStyles)
    {
        // ruby auto styles
        if (bStart)
            Add( XML_STYLE_FAMILY_TEXT_RUBY, rPropSet );
    }
    else
    {
        if (bStart)
        {
            // ruby start

            // we can only start a ruby if none is open
            assert(!bOpenRuby && "Can't open a ruby inside of ruby!");
            if( bOpenRuby )
                return;

            // save ruby text + ruby char style
            rPropSet->getPropertyValue(sRubyText) >>= sOpenRubyText;
            rPropSet->getPropertyValue(sRubyCharStyleName) >>= sOpenRubyCharStyle;

            // ruby style
            GetExport().CheckAttrList();
            OUString sStyleName(Find(XML_STYLE_FAMILY_TEXT_RUBY, rPropSet, ""));
            SAL_WARN_IF(sStyleName.isEmpty(), "xmloff", "Can't find ruby style!");
            GetExport().AddAttribute(XML_NAMESPACE_TEXT,
                                     XML_STYLE_NAME, sStyleName);

            // export <text:ruby> and <text:ruby-base> start elements
            GetExport().StartElement( XML_NAMESPACE_TEXT, XML_RUBY, false);
            GetExport().ClearAttrList();
            GetExport().StartElement( XML_NAMESPACE_TEXT, XML_RUBY_BASE,
                                      false );
            bOpenRuby = true;
        }
        else
        {
            // ruby end

            // check for an open ruby
            assert(bOpenRuby && "Can't close a ruby if none is open!");
            if( !bOpenRuby )
                return;

            // close <text:ruby-base>
            GetExport().EndElement(XML_NAMESPACE_TEXT, XML_RUBY_BASE,
                                   false);

            // write the ruby text (with char style)
            {
                if (!sOpenRubyCharStyle.isEmpty())
                    GetExport().AddAttribute(
                        XML_NAMESPACE_TEXT, XML_STYLE_NAME,
                        GetExport().EncodeStyleName( sOpenRubyCharStyle) );

                SvXMLElementExport aRubyElement(
                    GetExport(), XML_NAMESPACE_TEXT, XML_RUBY_TEXT,
                    false, false);

                GetExport().Characters(sOpenRubyText);
            }

            // and finally, close the ruby
            GetExport().EndElement(XML_NAMESPACE_TEXT, XML_RUBY, false);
            bOpenRuby = false;
        }
    }
}

void XMLTextParagraphExport::exportMeta(
    const Reference<XPropertySet> & i_xPortion,
    bool i_bAutoStyles, bool i_isProgress, bool & rPrevCharIsSpace)
{
    bool doExport(!i_bAutoStyles); // do not export element if autostyles
    // check version >= 1.2
    switch (GetExport().getDefaultVersion()) {
        case SvtSaveOptions::ODFVER_011: // fall through
        case SvtSaveOptions::ODFVER_010: doExport = false; break;
        default: break;
    }

    const Reference< XTextContent > xTextContent(
            i_xPortion->getPropertyValue("InContentMetadata"), UNO_QUERY_THROW);
    const Reference< XEnumerationAccess > xEA( xTextContent, UNO_QUERY_THROW );
    const Reference< XEnumeration > xTextEnum( xEA->createEnumeration() );

    if (doExport)
    {
        const Reference<rdf::XMetadatable> xMeta(xTextContent, UNO_QUERY_THROW);

        // text:meta with neither xml:id nor RDFa is invalid
        xMeta->ensureMetadataReference();

        // xml:id and RDFa for RDF metadata
        GetExport().AddAttributeXmlId(xMeta);
        GetExport().AddAttributesRDFa(xTextContent);
    }

    SvXMLElementExport aElem( GetExport(), doExport,
        XML_NAMESPACE_TEXT, XML_META, false, false );

    // recurse to export content
    exportTextRangeEnumeration(xTextEnum, i_bAutoStyles, i_isProgress, rPrevCharIsSpace);
}

void XMLTextParagraphExport::PreventExportOfControlsInMuteSections(
    const Reference<XIndexAccess> & rShapes,
    const rtl::Reference<xmloff::OFormLayerXMLExport>& xFormExport   )
{
    // check parameters ad pre-conditions
    if( ( ! rShapes.is() ) || ( ! xFormExport.is() ) )
    {
        // if we don't have shapes or a form export, there's nothing to do
        return;
    }
    SAL_WARN_IF( pSectionExport == nullptr, "xmloff", "We need the section export." );

    Reference<XEnumeration> xShapesEnum = pBoundFrameSets->GetShapes()->createEnumeration();
    if(!xShapesEnum.is())
        return;
    while( xShapesEnum->hasMoreElements() )
    {
        // now we need to check
        // 1) if this is a control shape, and
        // 2) if it's in a mute section
        // if both answers are 'yes', notify the form layer export

        // we join accessing the shape and testing for control
        Reference<XControlShape> xControlShape(xShapesEnum->nextElement(), UNO_QUERY);
        if( xControlShape.is() )
        {
            //            Reference<XPropertySet> xPropSet( xControlShape, UNO_QUERY );
            //            Reference<XTextContent> xTextContent;
            //            xPropSet->getPropertyValue("TextRange") >>= xTextContent;

            Reference<XTextContent> xTextContent( xControlShape, UNO_QUERY );
            if( xTextContent.is() )
            {
                if( pSectionExport->IsMuteSection( xTextContent, false ) )
                {
                    // Ah, we've found a shape that
                    // 1) is a control shape
                    // 2) is anchored in a mute section
                    // so: don't export it!
                    xFormExport->excludeFromExport(
                        xControlShape->getControl() );
                }
                // else: not in mute section -> should be exported -> nothing
                // to do
            }
            // else: no anchor -> ignore
        }
        // else: no control shape -> nothing to do
    }
}

void XMLTextParagraphExport::PushNewTextListsHelper()
{
    maTextListsHelperStack.emplace_back( new XMLTextListsHelper() );
    mpTextListsHelper = maTextListsHelperStack.back().get();
}

void XMLTextParagraphExport::PopTextListsHelper()
{
    mpTextListsHelper = nullptr;
    maTextListsHelperStack.pop_back();
    if ( !maTextListsHelperStack.empty() )
    {
        mpTextListsHelper = maTextListsHelperStack.back().get();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
