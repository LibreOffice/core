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

#include "xmlstyli.hxx"
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlimppr.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmlnumfi.hxx>
#include <xmloff/XMLGraphicsDefaultStyle.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntries.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <comphelper/extract.hxx>
#include <xmloff/xmlprcon.hxx>
#include <xmloff/xmluconv.hxx>
#include "XMLTableHeaderFooterContext.hxx"
#include "XMLConverter.hxx"
#include "XMLTableShapeImportHelper.hxx"
#include "sheetdata.hxx"
#include "xmlannoi.hxx"
#include "textuno.hxx"
#include "cellsuno.hxx"

#include "docuno.hxx"
#include "unonames.hxx"
#include "document.hxx"

#define XML_LINE_LEFT 0
#define XML_LINE_RIGHT 1
#define XML_LINE_TOP 2
#define XML_LINE_BOTTOM 3

#define XML_LINE_TLBR 0
#define XML_LINE_BLTR 1

using namespace ::com::sun::star;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace xmloff::token;
using namespace ::formula;

using rtl::OUString;
using com::sun::star::uno::Reference;
using com::sun::star::uno::UNO_QUERY;

ScXMLCellImportPropertyMapper::ScXMLCellImportPropertyMapper(
        const UniReference< XMLPropertySetMapper >& rMapper,
        SvXMLImport& rImportP) :
    SvXMLImportPropertyMapper( rMapper, rImportP )
{
}

ScXMLCellImportPropertyMapper::~ScXMLCellImportPropertyMapper()
{
}

void ScXMLCellImportPropertyMapper::finished(::std::vector< XMLPropertyState >& rProperties, sal_Int32 nStartIndex, sal_Int32 nEndIndex ) const
{
    static const sal_Int16 aPaddingCTF[4] = { CTF_SC_LEFTPADDING, CTF_SC_RIGHTPADDING,
                                            CTF_SC_TOPPADDING, CTF_SC_BOTTOMPADDING };
    static const sal_Int16 aBorderCTF[4] = { CTF_SC_LEFTBORDER, CTF_SC_RIGHTBORDER,
                                            CTF_SC_TOPBORDER, CTF_SC_BOTTOMBORDER };

    SvXMLImportPropertyMapper::finished(rProperties, nStartIndex, nEndIndex);
    XMLPropertyState* pAllPaddingProperty(NULL);
    XMLPropertyState* pPadding[4] = { NULL, NULL, NULL, NULL };
    XMLPropertyState* pNewPadding[4] = { NULL, NULL, NULL, NULL };
    XMLPropertyState* pAllBorderProperty = NULL;
    XMLPropertyState* pBorders[4] = { NULL, NULL, NULL, NULL };
    XMLPropertyState* pNewBorders[4] = { NULL, NULL, NULL, NULL };
    XMLPropertyState* pAllBorderWidthProperty = NULL;
    XMLPropertyState* pBorderWidths[4] = { NULL, NULL, NULL, NULL };
    XMLPropertyState* pDiagBorders[2] = { 0 };
    XMLPropertyState* pOldDiagBorderWidths[2] = { 0 };      // old attribute names without "s"
    XMLPropertyState* pDiagBorderWidths[2] = { 0 };

    ::std::vector< XMLPropertyState >::iterator endproperty(rProperties.end());
    for (::std::vector< XMLPropertyState >::iterator aIter =  rProperties.begin();
        aIter != endproperty; ++aIter)
    {
        XMLPropertyState*property = &(*aIter);
        if (property->mnIndex != -1)
        {
            sal_Int16 nContextID = getPropertySetMapper()->GetEntryContextId(property->mnIndex);
            switch (nContextID)
            {
                case CTF_SC_ALLPADDING                  : pAllPaddingProperty = &*property; break;
                case CTF_SC_LEFTPADDING                 : pPadding[XML_LINE_LEFT] = &*property; break;
                case CTF_SC_RIGHTPADDING                : pPadding[XML_LINE_RIGHT] = &*property; break;
                case CTF_SC_TOPPADDING                  : pPadding[XML_LINE_TOP] = &*property; break;
                case CTF_SC_BOTTOMPADDING               : pPadding[XML_LINE_BOTTOM] = &*property; break;
                case CTF_SC_ALLBORDER                   : pAllBorderProperty = &*property; break;
                case CTF_SC_LEFTBORDER                  : pBorders[XML_LINE_LEFT] = &*property; break;
                case CTF_SC_RIGHTBORDER                 : pBorders[XML_LINE_RIGHT] = &*property; break;
                case CTF_SC_TOPBORDER                   : pBorders[XML_LINE_TOP] = &*property; break;
                case CTF_SC_BOTTOMBORDER                : pBorders[XML_LINE_BOTTOM] = &*property; break;
                case CTF_SC_ALLBORDERWIDTH              : pAllBorderWidthProperty = &*property; break;
                case CTF_SC_LEFTBORDERWIDTH             : pBorderWidths[XML_LINE_LEFT] = &*property; break;
                case CTF_SC_RIGHTBORDERWIDTH            : pBorderWidths[XML_LINE_RIGHT] = &*property; break;
                case CTF_SC_TOPBORDERWIDTH              : pBorderWidths[XML_LINE_TOP] = &*property; break;
                case CTF_SC_BOTTOMBORDERWIDTH           : pBorderWidths[XML_LINE_BOTTOM] = &*property; break;
                case CTF_SC_DIAGONALTLBR                : pDiagBorders[XML_LINE_TLBR] = &*property; break;
                case CTF_SC_DIAGONALBLTR                : pDiagBorders[XML_LINE_BLTR] = &*property; break;
                case CTF_SC_DIAGONALTLBRWIDTH           : pOldDiagBorderWidths[XML_LINE_TLBR] = &*property; break;
                case CTF_SC_DIAGONALTLBRWIDTHS          : pDiagBorderWidths[XML_LINE_TLBR] = &*property; break;
                case CTF_SC_DIAGONALBLTRWIDTH           : pOldDiagBorderWidths[XML_LINE_BLTR] = &*property; break;
                case CTF_SC_DIAGONALBLTRWIDTHS          : pDiagBorderWidths[XML_LINE_BLTR] = &*property; break;
            }
        }
    }
    sal_uInt16 i;

    // #i27594#; copy Value, but don't insert
    if (pAllBorderWidthProperty)
        pAllBorderWidthProperty->mnIndex = -1;
    if (pAllBorderProperty)
        pAllBorderProperty->mnIndex = -1;
    if (pAllPaddingProperty)
        pAllPaddingProperty->mnIndex = -1;

    for (i = 0; i < 4; ++i)
    {
        if (pAllPaddingProperty && !pPadding[i])
            pNewPadding[i] = new XMLPropertyState(maPropMapper->FindEntryIndex(aPaddingCTF[i]), pAllPaddingProperty->maValue);
        if (pAllBorderProperty && !pBorders[i])
        {
            pNewBorders[i] = new XMLPropertyState(maPropMapper->FindEntryIndex(aBorderCTF[i]), pAllBorderProperty->maValue);
            pBorders[i] = pNewBorders[i];
        }
        if( !pBorderWidths[i] )
            pBorderWidths[i] = pAllBorderWidthProperty;
        else
            pBorderWidths[i]->mnIndex = -1;
        if( pBorders[i] )
        {
            table::BorderLine2 aBorderLine;
            pBorders[i]->maValue >>= aBorderLine;
            if( pBorderWidths[i] )
            {
                // Merge style:border-line-width values to fo:border values. Do
                // not override fo:border line width or line style with an
                // empty value!
                table::BorderLine2 aBorderLineWidth;
                pBorderWidths[i]->maValue >>= aBorderLineWidth;
                aBorderLine.OuterLineWidth = aBorderLineWidth.OuterLineWidth;
                aBorderLine.InnerLineWidth = aBorderLineWidth.InnerLineWidth;
                aBorderLine.LineDistance = aBorderLineWidth.LineDistance;
                pBorders[i]->maValue <<= aBorderLine;
            }
        }
    }
    for( i = 0; i < 2; ++i )
    {
        if( pDiagBorders[i] && ( pDiagBorderWidths[i] || pOldDiagBorderWidths[i] ) )
        {
            table::BorderLine2 aBorderLine;
            pDiagBorders[i]->maValue >>= aBorderLine;
            table::BorderLine2 aBorderLineWidth;
            if (pDiagBorderWidths[i])
                pDiagBorderWidths[i]->maValue >>= aBorderLineWidth;     // prefer new attribute
            else
                pOldDiagBorderWidths[i]->maValue >>= aBorderLineWidth;
            aBorderLine.OuterLineWidth = aBorderLineWidth.OuterLineWidth;
            aBorderLine.InnerLineWidth = aBorderLineWidth.InnerLineWidth;
            aBorderLine.LineDistance = aBorderLineWidth.LineDistance;
            pDiagBorders[i]->maValue <<= aBorderLine;
            if (pDiagBorderWidths[i])
                pDiagBorderWidths[i]->mnIndex = -1;
            if (pOldDiagBorderWidths[i])
                pOldDiagBorderWidths[i]->mnIndex = -1;      // reset mnIndex for old and new attribute if both are present
        }
    }

    for (i = 0; i < 4; ++i)
    {
        if (pNewPadding[i])
        {
            rProperties.push_back(*pNewPadding[i]);
            delete pNewPadding[i];
        }
        if (pNewBorders[i])
        {
            rProperties.push_back(*pNewBorders[i]);
            delete pNewBorders[i];
        }
    }
}

ScXMLRowImportPropertyMapper::ScXMLRowImportPropertyMapper(
        const UniReference< XMLPropertySetMapper >& rMapper,
        SvXMLImport& rImportP) :
    SvXMLImportPropertyMapper( rMapper, rImportP )
{
}

ScXMLRowImportPropertyMapper::~ScXMLRowImportPropertyMapper()
{
}

void ScXMLRowImportPropertyMapper::finished(::std::vector< XMLPropertyState >& rProperties, sal_Int32 nStartIndex, sal_Int32 nEndIndex ) const
{
    SvXMLImportPropertyMapper::finished(rProperties, nStartIndex, nEndIndex);
    XMLPropertyState* pHeight(NULL);
    XMLPropertyState* pOptimalHeight(NULL);
    XMLPropertyState* pPageBreak(NULL);
    ::std::vector< XMLPropertyState >::iterator endproperty(rProperties.end());
    for (::std::vector< XMLPropertyState >::iterator aIter = rProperties.begin();
        aIter != endproperty; ++aIter)
    {
        XMLPropertyState* property = &(*aIter);
        if (property->mnIndex != -1)
        {
            sal_Int16 nContextID = getPropertySetMapper()->GetEntryContextId(property->mnIndex);
            switch (nContextID)
            {
                case CTF_SC_ROWHEIGHT                   : pHeight = property; break;
                case CTF_SC_ROWOPTIMALHEIGHT            : pOptimalHeight = property; break;
                case CTF_SC_ROWBREAKBEFORE              : pPageBreak = property; break;
            }
        }
    }
    if (pPageBreak)
    {
        if(!(::cppu::any2bool(pPageBreak->maValue)))
            pPageBreak->mnIndex = -1;
    }
    if (pOptimalHeight)
    {
        if (::cppu::any2bool(pOptimalHeight->maValue))
        {
            if (pHeight)
            {
                // set the stored height, but keep "optimal" flag:
                // pass the height value as OptimalHeight property (only allowed while loading!)
                pOptimalHeight->maValue = pHeight->maValue;
                pHeight->mnIndex = -1;
            }
            else
                pOptimalHeight->mnIndex = -1;
        }
    }
    else if (pHeight)
    {
        rProperties.push_back(XMLPropertyState(maPropMapper->FindEntryIndex(CTF_SC_ROWOPTIMALHEIGHT), ::cppu::bool2any( false )));
    }
    // don't access pointers to rProperties elements after push_back!
}

class ScXMLMapContext : public SvXMLImportContext
{
    rtl::OUString sApplyStyle;
    rtl::OUString sCondition;
    rtl::OUString sBaseCell;
public:

    ScXMLMapContext(
            SvXMLImport& rImport, sal_uInt16 nPrfx,
            const rtl::OUString& rLName,
            const uno::Reference< xml::sax::XAttributeList > & xAttrList );
    virtual ~ScXMLMapContext();

    const rtl::OUString& GetApplyStyle() const { return sApplyStyle; }
    const rtl::OUString& GetCondition() const { return sCondition; }
    const rtl::OUString& GetBaseCell() const { return sBaseCell; }
};

ScXMLMapContext::ScXMLMapContext(SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName, const uno::Reference< xml::sax::XAttributeList > & xAttrList )
    : SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& rAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName ));
        const OUString& rValue(xAttrList->getValueByIndex( i ));

        // TODO: use a map here
        if( XML_NAMESPACE_STYLE == nPrefix )
        {
            if( IsXMLToken(aLocalName, XML_CONDITION ) )
                sCondition = rValue;
            else if( IsXMLToken(aLocalName, XML_APPLY_STYLE_NAME ) )
                sApplyStyle = GetImport().GetStyleDisplayName( XML_STYLE_FAMILY_TABLE_CELL, rValue);
            else if ( IsXMLToken(aLocalName, XML_BASE_CELL_ADDRESS ) )
                sBaseCell = rValue;
        }
    }
}

ScXMLMapContext::~ScXMLMapContext()
{
}

namespace {

template< typename Type >
inline void lclAppendProperty( uno::Sequence< beans::PropertyValue >& rProps, const OUString& rPropName, const Type& rValue )
{
    sal_Int32 nLength = rProps.getLength();
    rProps.realloc( nLength + 1 );
    rProps[ nLength ].Name = rPropName;
    rProps[ nLength ].Value <<= rValue;
}

} // namespace

void XMLTableStyleContext::SetOperator( uno::Sequence< beans::PropertyValue >& rProps, sheet::ConditionOperator eOp ) const
{
    lclAppendProperty( rProps, OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_OPERATOR ) ), eOp );
}

void XMLTableStyleContext::SetBaseCellAddress( uno::Sequence< beans::PropertyValue >& rProps, const OUString& rBaseCell ) const
{
    /*  Source position must be set as string, because it may refer
        to a sheet that hasn't been loaded yet. */
    lclAppendProperty( rProps, OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_SOURCESTR ) ), rBaseCell );
}

void XMLTableStyleContext::SetStyle( uno::Sequence<beans::PropertyValue>& rProps, const OUString& rApplyStyle ) const
{
    lclAppendProperty( rProps, OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_STYLENAME ) ), rApplyStyle );
}

void XMLTableStyleContext::SetFormula( uno::Sequence< beans::PropertyValue >& rProps,
        sal_Int32 nFormulaIdx, const OUString& rFormula, const OUString& rFormulaNmsp,
        FormulaGrammar::Grammar eGrammar, bool bHasNmsp ) const
{
    OUString aFormula, aFormulaNmsp;
    FormulaGrammar::Grammar eNewGrammar = FormulaGrammar::GRAM_UNSPECIFIED;
    if( bHasNmsp )
    {
        // the entire attribute contains a namespace: internal namespace not allowed
        aFormula = rFormula;
        aFormulaNmsp = rFormulaNmsp;
        eNewGrammar = eGrammar;
    }
    else
    {
        // the attribute does not contain a namespace: try to find a namespace of an external grammar
        GetScImport().ExtractFormulaNamespaceGrammar( aFormula, aFormulaNmsp, eNewGrammar, rFormula, true );
        if( eNewGrammar != FormulaGrammar::GRAM_EXTERNAL )
            eNewGrammar = eGrammar;
    }

    // add formula, formula namespace, and grammar with appropriate property names
    sal_Int32 nGrammar = static_cast< sal_Int32 >( eNewGrammar );
    switch( nFormulaIdx )
    {
        case 1:
            lclAppendProperty( rProps, OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_FORMULA1 ) ), aFormula );
            lclAppendProperty( rProps, OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_FORMULANMSP1 ) ), aFormulaNmsp );
            lclAppendProperty( rProps, OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_GRAMMAR1 ) ), nGrammar );
        break;
        case 2:
            lclAppendProperty( rProps, OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_FORMULA2 ) ), aFormula );
            lclAppendProperty( rProps, OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_FORMULANMSP2 ) ), aFormulaNmsp );
            lclAppendProperty( rProps, OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_GRAMMAR2 ) ), nGrammar );
        break;
        default:
            OSL_FAIL( "XMLTableStyleContext::SetFormula - invalid formula index" );
    }
}

void XMLTableStyleContext::GetConditionalFormat(uno::Any& aAny,
        const rtl::OUString& sTempCondition,
        const rtl::OUString& sApplyStyle, const rtl::OUString& sBaseCell) const
{
    if (!sTempCondition.isEmpty() && !sApplyStyle.isEmpty())
    {
        uno::Reference<sheet::XSheetConditionalEntries> xConditionalEntries(aAny, uno::UNO_QUERY);
        if (xConditionalEntries.is())
        {
            uno::Sequence<beans::PropertyValue> aProps;
            if (!sBaseCell.isEmpty())
                SetBaseCellAddress(aProps, sBaseCell);
            SetStyle(aProps, sApplyStyle);

            // extract leading namespace from condition string
            OUString aCondition, aConditionNmsp;
            FormulaGrammar::Grammar eGrammar = FormulaGrammar::GRAM_UNSPECIFIED;
            GetScImport().ExtractFormulaNamespaceGrammar( aCondition, aConditionNmsp, eGrammar, sTempCondition );
            bool bHasNmsp = aCondition.getLength() < sTempCondition.getLength();

            // parse a condition from the attribute string
            ScXMLConditionParseResult aParseResult;
            ScXMLConditionHelper::parseCondition( aParseResult, aCondition, 0 );

            /*  Check the result. A valid value in aParseResult.meToken implies
                that the other members of aParseResult are filled with valid
                data for that token. */
            switch( aParseResult.meToken )
            {
                case XML_COND_CELLCONTENT:      // condition is 'cell-content()<operator><expression>'
                case XML_COND_ISTRUEFORMULA:    // condition is 'is-true-formula(<expression>)'
                case XML_COND_ISBETWEEN:        // condition is 'cell-content-is-between(<expression1>,<expression2>)'
                case XML_COND_ISNOTBETWEEN:     // condition is 'cell-content-is-not-between(<expression1>,<expression2>)'
                    SetOperator( aProps, aParseResult.meOperator );
                    SetFormula( aProps, 1, aParseResult.maOperand1, aConditionNmsp, eGrammar, bHasNmsp );
                    SetFormula( aProps, 2, aParseResult.maOperand2, aConditionNmsp, eGrammar, bHasNmsp );
                break;

                default:;   // unacceptable or unknown condition
            }

            xConditionalEntries->addNew( aProps );
            aAny <<= xConditionalEntries;
        }
    }
}

void XMLTableStyleContext::SetAttribute( sal_uInt16 nPrefixKey,
                                        const OUString& rLocalName,
                                        const OUString& rValue )
{
    // TODO: use a map here
    if( IsXMLToken(rLocalName, XML_DATA_STYLE_NAME ) )
        sDataStyleName = rValue;
    else if ( IsXMLToken(rLocalName, XML_MASTER_PAGE_NAME ) )
        sPageStyle = rValue;
    else
        XMLPropStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
}

struct ScXMLMapContent
{
    rtl::OUString sCondition;
    rtl::OUString sApplyStyle;
    rtl::OUString sBaseCell;
};

TYPEINIT1( XMLTableStyleContext, XMLPropStyleContext );

XMLTableStyleContext::XMLTableStyleContext( ScXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const uno::Reference< XAttributeList > & xAttrList,
        SvXMLStylesContext& rStyles, sal_uInt16 nFamily, bool bDefaultStyle ) :
    XMLPropStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles, nFamily, bDefaultStyle ),
    sDataStyleName(),
    sNumberFormat(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberFormat"))),
    pStyles(&rStyles),
    nNumberFormat(-1),
    nLastSheet(-1),
    bConditionalFormatCreated(false),
    bParentSet(false)
{
}

XMLTableStyleContext::~XMLTableStyleContext()
{
}

SvXMLImportContext *XMLTableStyleContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext(NULL);

    if( (XML_NAMESPACE_STYLE == nPrefix) &&
        IsXMLToken(rLocalName, XML_MAP ) )
    {
        pContext = new ScXMLMapContext(GetImport(), nPrefix, rLocalName, xAttrList);

        ScXMLMapContent aMap;
        aMap.sCondition = ((ScXMLMapContext*)pContext)->GetCondition();
        aMap.sApplyStyle = ((ScXMLMapContext*)pContext)->GetApplyStyle();
        aMap.sBaseCell = ((ScXMLMapContext*)pContext)->GetBaseCell();
        aMaps.push_back(aMap);
    }
    if (!pContext)
        pContext = XMLPropStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                           xAttrList );
    return pContext;
}

void XMLTableStyleContext::FillPropertySet(
    const uno::Reference< XPropertySet > & rPropSet )
{
    if (!IsDefaultStyle())
    {
        if (GetFamily() == XML_STYLE_FAMILY_TABLE_CELL)
        {
            if (!bParentSet)
            {
                AddProperty(CTF_SC_CELLSTYLE, uno::makeAny(GetImport().GetStyleDisplayName( XML_STYLE_FAMILY_TABLE_CELL, GetParentName() )));
                bParentSet = true;
            }
            sal_Int32 nNumFmt = GetNumberFormat();
            if (nNumFmt >= 0)
                AddProperty(CTF_SC_NUMBERFORMAT, uno::makeAny(nNumFmt));
            if (!GetScImport().HasNewCondFormatData() && !bConditionalFormatCreated && (!aMaps.empty()))
            {
                aConditionalFormat = rPropSet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_CONDXML)));
                std::vector<ScXMLMapContent>::iterator aItr(aMaps.begin());
                std::vector<ScXMLMapContent>::iterator aEndItr(aMaps.end());
                while(aItr != aEndItr)
                {
                    //rPropSet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_CONDITIONALFORMAT)),
                    GetConditionalFormat(aConditionalFormat, aItr->sCondition, aItr->sApplyStyle, aItr->sBaseCell);

                    ++aItr;
                }
                AddProperty(CTF_SC_IMPORT_MAP, aConditionalFormat);
                bConditionalFormatCreated = true;
            }
        }
        else if (GetFamily() == XML_STYLE_FAMILY_TABLE_TABLE)
        {
            if (!sPageStyle.isEmpty())
                AddProperty(CTF_SC_MASTERPAGENAME, uno::makeAny(GetImport().GetStyleDisplayName( XML_STYLE_FAMILY_MASTER_PAGE, sPageStyle )));
        }
    }
    XMLPropStyleContext::FillPropertySet(rPropSet);
}

void XMLTableStyleContext::SetDefaults()
{
    if ((GetFamily() == XML_STYLE_FAMILY_TABLE_CELL) && GetImport().GetModel().is())
    {
        uno::Reference <lang::XMultiServiceFactory> xMultiServiceFactory(GetImport().GetModel(), uno::UNO_QUERY);
        if (xMultiServiceFactory.is())
        {
            uno::Reference <beans::XPropertySet> xProperties(xMultiServiceFactory->createInstance(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.Defaults"))), uno::UNO_QUERY);
            if (xProperties.is())
                FillPropertySet(xProperties);
        }
    }
}

void XMLTableStyleContext::AddProperty(const sal_Int16 nContextID, const uno::Any& rValue)
{
    XMLPropertyState* property = FindProperty(nContextID);
    if (property)
        property->mnIndex = -1; // #i46996# remove old property, so it isn't double
    sal_Int32 nIndex(static_cast<XMLTableStylesContext *>(pStyles)->GetIndex(nContextID));
    OSL_ENSURE(nIndex != -1, "Property not found in Map");
    XMLPropertyState aPropState(nIndex, rValue);
    GetProperties().push_back(aPropState); // has to be insertes in a sort order later
}

XMLPropertyState* XMLTableStyleContext::FindProperty(const sal_Int16 nContextID)
{
    XMLPropertyState* pRet = NULL;
    UniReference < XMLPropertySetMapper > xPrMap;
    UniReference < SvXMLImportPropertyMapper > xImpPrMap =
        pStyles->GetImportPropertyMapper( GetFamily() );
    OSL_ENSURE( xImpPrMap.is(), "There is the import prop mapper" );
    if( xImpPrMap.is() )
        xPrMap = xImpPrMap->getPropertySetMapper();
    if( xPrMap.is() )
    {
        ::std::vector< XMLPropertyState >::iterator endproperty(GetProperties().end());
        ::std::vector< XMLPropertyState >::iterator aIter(GetProperties().begin());
        while(!pRet && aIter != endproperty)
        {
            XMLPropertyState* property = &(*aIter);
            if (property->mnIndex != -1 && xPrMap->GetEntryContextId(property->mnIndex) == nContextID)
            {
                pRet = property;
            }
            else
                ++aIter;
        }
    }
    return pRet;
}

sal_Int32 XMLTableStyleContext::GetNumberFormat()
{
    if (nNumberFormat < 0 && !sDataStyleName.isEmpty())
    {
        const SvXMLNumFormatContext* pStyle = static_cast<const SvXMLNumFormatContext*>(
            pStyles->FindStyleChildContext(XML_STYLE_FAMILY_DATA_STYLE, sDataStyleName, sal_True));

        if (!pStyle)
        {
            XMLTableStylesContext* pMyStyles = static_cast<XMLTableStylesContext*>(GetScImport().GetStyles());
            if (pMyStyles)
                pStyle = static_cast<const SvXMLNumFormatContext*>(
                    pMyStyles->FindStyleChildContext(XML_STYLE_FAMILY_DATA_STYLE, sDataStyleName, sal_True));
            else
            {
                OSL_FAIL("not possible to get style");
            }
        }
        if (pStyle)
            nNumberFormat = const_cast<SvXMLNumFormatContext*>(pStyle)->GetKey();
    }
    return nNumberFormat;
}

// ----------------------------------------------------------------------------

SvXMLStyleContext *XMLTableStylesContext::CreateStyleStyleChildContext(
        sal_uInt16 nFamily, sal_uInt16 nPrefix, const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLStyleContext *pStyle;
    // use own wrapper for text and paragraph, to record style usage
    if (nFamily == XML_STYLE_FAMILY_TEXT_PARAGRAPH || nFamily == XML_STYLE_FAMILY_TEXT_TEXT)
        pStyle = new ScCellTextStyleContext( GetImport(), nPrefix, rLocalName,
                                            xAttrList, *this, nFamily );
    else
        pStyle = SvXMLStylesContext::CreateStyleStyleChildContext(
                    nFamily, nPrefix, rLocalName, xAttrList );

    if (!pStyle)
    {
        switch( nFamily )
        {
        case XML_STYLE_FAMILY_TABLE_CELL:
        case XML_STYLE_FAMILY_TABLE_COLUMN:
        case XML_STYLE_FAMILY_TABLE_ROW:
        case XML_STYLE_FAMILY_TABLE_TABLE:
            pStyle = new XMLTableStyleContext( GetScImport(), nPrefix, rLocalName,
                                               xAttrList, *this, nFamily );
            break;
        }
    }

    return pStyle;
}

SvXMLStyleContext *XMLTableStylesContext::CreateDefaultStyleStyleChildContext(
        sal_uInt16 nFamily, sal_uInt16 nPrefix, const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLStyleContext *pStyle(SvXMLStylesContext::CreateDefaultStyleStyleChildContext( nFamily, nPrefix,
                                                            rLocalName,
                                                            xAttrList ));
    if (!pStyle)
    {
        switch( nFamily )
        {
            case XML_STYLE_FAMILY_TABLE_CELL:
                pStyle = new XMLTableStyleContext( GetScImport(), nPrefix, rLocalName,
                                            xAttrList, *this, nFamily, sal_True);
            break;
            case XML_STYLE_FAMILY_SD_GRAPHICS_ID:
                pStyle = new XMLGraphicsDefaultStyle( GetScImport(), nPrefix, rLocalName,
                                            xAttrList, *this);
            break;
        }
    }

    return pStyle;
}

XMLTableStylesContext::XMLTableStylesContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx ,
        const OUString& rLName ,
        const uno::Reference< XAttributeList > & xAttrList,
        const bool bTempAutoStyles ) :
    SvXMLStylesContext( rImport, nPrfx, rLName, xAttrList ),
    sCellStyleServiceName( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.style.CellStyle" ) )),
    sColumnStyleServiceName( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_NAME ))),
    sRowStyleServiceName( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( XML_STYLE_FAMILY_TABLE_ROW_STYLES_NAME ))),
    sTableStyleServiceName( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( XML_STYLE_FAMILY_TABLE_TABLE_STYLES_NAME ))),
    nNumberFormatIndex(-1),
    nConditionalFormatIndex(-1),
    nCellStyleIndex(-1),
    nMasterPageNameIndex(-1),
    bAutoStyles(bTempAutoStyles)
{
}

XMLTableStylesContext::~XMLTableStylesContext()
{
}

void XMLTableStylesContext::EndElement()
{
    SvXMLStylesContext::EndElement();
    if (bAutoStyles)
        GetImport().GetTextImport()->SetAutoStyles( this );
    else
        ((ScXMLImport&)GetImport()).InsertStyles();
}

UniReference < SvXMLImportPropertyMapper >
    XMLTableStylesContext::GetImportPropertyMapper(
                    sal_uInt16 nFamily ) const
{
    UniReference < SvXMLImportPropertyMapper > xMapper(SvXMLStylesContext::GetImportPropertyMapper(nFamily));

    if (!xMapper.is())
    {
        switch( nFamily )
        {
            case XML_STYLE_FAMILY_TABLE_CELL:
            {
                if( !xCellImpPropMapper.is() )
                {
                    ((XMLTableStylesContext *)this)->xCellImpPropMapper =
                        new ScXMLCellImportPropertyMapper( GetScImport().GetCellStylesPropertySetMapper(), const_cast<SvXMLImport&>(GetImport()) );
                    xCellImpPropMapper->ChainImportMapper(XMLTextImportHelper::CreateParaExtPropMapper(const_cast<SvXMLImport&>(GetImport()), const_cast<XMLFontStylesContext*>(GetScImport().GetFontDecls())));
                }
                xMapper = xCellImpPropMapper;
            }
            break;
            case XML_STYLE_FAMILY_TABLE_COLUMN:
            {
                if( !xColumnImpPropMapper.is() )
                    ((XMLTableStylesContext *)this)->xColumnImpPropMapper =
                        new SvXMLImportPropertyMapper( GetScImport().GetColumnStylesPropertySetMapper(), const_cast<SvXMLImport&>(GetImport()) );
                xMapper = xColumnImpPropMapper;
            }
             break;
            case XML_STYLE_FAMILY_TABLE_ROW:
            {
                if( !xRowImpPropMapper.is() )
                    ((XMLTableStylesContext *)this)->xRowImpPropMapper =
                        new ScXMLRowImportPropertyMapper( GetScImport().GetRowStylesPropertySetMapper(), const_cast<SvXMLImport&>(GetImport()) );
                xMapper = xRowImpPropMapper;
            }
             break;
            case XML_STYLE_FAMILY_TABLE_TABLE:
            {
                if( !xTableImpPropMapper.is() )
                    ((XMLTableStylesContext *)this)->xTableImpPropMapper =
                        new SvXMLImportPropertyMapper( GetScImport().GetTableStylesPropertySetMapper(), const_cast<SvXMLImport&>(GetImport()) );
                xMapper = xTableImpPropMapper;
            }
             break;
        }
    }

    return xMapper;
}

uno::Reference < XNameContainer >
        XMLTableStylesContext::GetStylesContainer( sal_uInt16 nFamily ) const
{
    uno::Reference < XNameContainer > xStyles(SvXMLStylesContext::GetStylesContainer(nFamily));
    if (!xStyles.is())
    {
        OUString sName;
        switch( nFamily )
        {
            case XML_STYLE_FAMILY_TABLE_TABLE:
            {
                if( xTableStyles.is() )
                    xStyles.set(xTableStyles);
                else
                    sName =
                        OUString( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "TableStyles" ) ));
            }
            break;
            case XML_STYLE_FAMILY_TABLE_CELL:
            {
                if( xCellStyles.is() )
                    xStyles.set(xCellStyles);
                else
                    sName =
                        OUString( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "CellStyles" ) ));
            }
            break;
            case XML_STYLE_FAMILY_TABLE_COLUMN:
            {
                if( xColumnStyles.is() )
                    xStyles.set(xColumnStyles);
                else
                    sName =
                        OUString( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ColumnStyles" ) ));
            }
            break;
            case XML_STYLE_FAMILY_TABLE_ROW:
            {
                if( xRowStyles.is() )
                    xStyles.set(xRowStyles);
                else
                    sName =
                        OUString( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "RowStyles" ) ));
            }
            break;
        }
        if( !xStyles.is() && !sName.isEmpty() && GetScImport().GetModel().is() )
        {
            uno::Reference< XStyleFamiliesSupplier > xFamiliesSupp(
                                            GetScImport().GetModel(), UNO_QUERY );
            if (xFamiliesSupp.is())
            {
                uno::Reference< XNameAccess > xFamilies(xFamiliesSupp->getStyleFamilies());

                try
                {
                    xStyles.set(xFamilies->getByName( sName ), uno::UNO_QUERY);
                }
                catch ( uno::Exception& )
                {
                    // #i97680# Named table/column/row styles aren't supported, getByName will throw an exception.
                    // For better interoperability, these styles should then be handled as automatic styles.
                    // For now, NULL is returned (and the style is ignored).
                }
                switch( nFamily )
                {
                case XML_STYLE_FAMILY_TABLE_TABLE:
                    ((XMLTableStylesContext *)this)->xTableStyles.set(xStyles);
                    break;
                case XML_STYLE_FAMILY_TABLE_CELL:
                    ((XMLTableStylesContext *)this)->xCellStyles.set(xStyles);
                    break;
                case XML_STYLE_FAMILY_TABLE_COLUMN:
                    ((XMLTableStylesContext *)this)->xColumnStyles.set(xStyles);
                    break;
                case XML_STYLE_FAMILY_TABLE_ROW:
                    ((XMLTableStylesContext *)this)->xRowStyles.set(xStyles);
                    break;
                }
            }
        }
    }

    return xStyles;
}

OUString XMLTableStylesContext::GetServiceName( sal_uInt16 nFamily ) const
{
    rtl::OUString sServiceName(SvXMLStylesContext::GetServiceName(nFamily));
    if (sServiceName.isEmpty())
    {
        switch( nFamily )
        {
        case XML_STYLE_FAMILY_TABLE_COLUMN:
            sServiceName = sColumnStyleServiceName;
            break;
        case XML_STYLE_FAMILY_TABLE_ROW:
            sServiceName = sRowStyleServiceName;
            break;
        case XML_STYLE_FAMILY_TABLE_CELL:
            sServiceName = sCellStyleServiceName;
            break;
        case XML_STYLE_FAMILY_TABLE_TABLE:
            sServiceName = sTableStyleServiceName;
            break;
        }
    }
    return sServiceName;
}

sal_Int32 XMLTableStylesContext::GetIndex(const sal_Int16 nContextID)
{
    if (nContextID == CTF_SC_CELLSTYLE)
    {
        if (nCellStyleIndex == -1)
            nCellStyleIndex =
                GetImportPropertyMapper(XML_STYLE_FAMILY_TABLE_CELL)->getPropertySetMapper()->FindEntryIndex(nContextID);
        return nCellStyleIndex;
    }
    else if (nContextID == CTF_SC_NUMBERFORMAT)
    {
        if (nNumberFormatIndex == -1)
            nNumberFormatIndex =
                GetImportPropertyMapper(XML_STYLE_FAMILY_TABLE_CELL)->getPropertySetMapper()->FindEntryIndex(nContextID);
        return nNumberFormatIndex;
    }
    else if (nContextID == CTF_SC_IMPORT_MAP)
    {
        if (nConditionalFormatIndex == -1)
            nConditionalFormatIndex =
                GetImportPropertyMapper(XML_STYLE_FAMILY_TABLE_CELL)->getPropertySetMapper()->FindEntryIndex(nContextID);
        return nConditionalFormatIndex;
    }
    else if (nContextID == CTF_SC_MASTERPAGENAME)
    {
        if (nMasterPageNameIndex == -1)
                nMasterPageNameIndex =
                GetImportPropertyMapper(XML_STYLE_FAMILY_TABLE_TABLE)->getPropertySetMapper()->FindEntryIndex(nContextID);
        return nMasterPageNameIndex;
    }
    else
        return -1;
}

// ---------------------------------------------------------------------------
TYPEINIT1( ScXMLMasterStylesContext, SvXMLStylesContext );

sal_Bool ScXMLMasterStylesContext::InsertStyleFamily( sal_uInt16 ) const
{
    return sal_True;
}

ScXMLMasterStylesContext::ScXMLMasterStylesContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const uno::Reference< XAttributeList > & xAttrList ) :
    SvXMLStylesContext( rImport, nPrfx, rLName, xAttrList )
{
}

ScXMLMasterStylesContext::~ScXMLMasterStylesContext()
{
}

SvXMLStyleContext *ScXMLMasterStylesContext::CreateStyleChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< XAttributeList > & xAttrList )
{
    SvXMLStyleContext *pContext(0);

    if( (XML_NAMESPACE_STYLE == nPrefix) &&
        IsXMLToken(rLocalName, XML_MASTER_PAGE) &&
         InsertStyleFamily( XML_STYLE_FAMILY_MASTER_PAGE ) )
        pContext = new ScMasterPageContext(
                        GetImport(), nPrefix, rLocalName, xAttrList,
                        !GetImport().GetTextImport()->IsInsertMode() );

    // any other style will be ignored here!

    return pContext;
}

SvXMLStyleContext *ScXMLMasterStylesContext::CreateStyleStyleChildContext(
        sal_uInt16 /* nFamily */,
        sal_uInt16 /* nPrefix */,
        const OUString& /* rLocalName */,
        const uno::Reference< XAttributeList > & /* xAttrList */ )
{
    return 0;
}

void ScXMLMasterStylesContext::EndElement()
{
    FinishStyles(sal_True);
}

TYPEINIT1( ScMasterPageContext, XMLTextMasterPageContext );

ScMasterPageContext::ScMasterPageContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const uno::Reference< XAttributeList > & xAttrList,
        bool bOverwrite ) :
    XMLTextMasterPageContext( rImport, nPrfx, rLName, xAttrList, bOverwrite ),
    bContainsRightHeader(false),
    bContainsRightFooter(false)
{
}

ScMasterPageContext::~ScMasterPageContext()
{
}

SvXMLImportContext *ScMasterPageContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< XAttributeList > & xAttrList )
{
    return XMLTextMasterPageContext::CreateChildContext( nPrefix, rLocalName, xAttrList );
}

SvXMLImportContext *ScMasterPageContext::CreateHeaderFooterContext(
            sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            const sal_Bool bFooter,
            const sal_Bool bLeft,
            const sal_Bool /*bFirst*/ )
{
    if (!bLeft)
    {
        if (bFooter)
            bContainsRightFooter = sal_True;
        else
            bContainsRightHeader = sal_True;
    }
    if (!xPropSet.is())
        xPropSet.set(GetStyle(), UNO_QUERY );
    return new XMLTableHeaderFooterContext( GetImport(),
                                                nPrefix, rLocalName,
                                                xAttrList,
                                                xPropSet,
                                                bFooter, bLeft );
}

void ScMasterPageContext::ClearContent(const rtl::OUString& rContent)
{
    if (!xPropSet.is())
        xPropSet.set(GetStyle(), UNO_QUERY );

    if (xPropSet.is())
    {
        uno::Reference < sheet::XHeaderFooterContent > xHeaderFooterContent(xPropSet->getPropertyValue( rContent ), uno::UNO_QUERY);
        if (xHeaderFooterContent.is())
        {
            xHeaderFooterContent->getLeftText()->setString(sEmpty);
            xHeaderFooterContent->getCenterText()->setString(sEmpty);
            xHeaderFooterContent->getRightText()->setString(sEmpty);
            xPropSet->setPropertyValue( rContent, uno::makeAny(xHeaderFooterContent) );
        }
    }
}

void ScMasterPageContext::Finish( sal_Bool bOverwrite )
{
    XMLTextMasterPageContext::Finish(bOverwrite);
    if (!bContainsRightFooter)
        ClearContent(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_PAGE_RIGHTFTRCON)));
    if (!bContainsRightHeader)
        ClearContent(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_PAGE_RIGHTHDRCON)));
}

// ---------------------------------------------------------------------------

ScCellTextStyleContext::ScCellTextStyleContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const rtl::OUString& rLName, const uno::Reference<xml::sax::XAttributeList> & xAttrList,
            SvXMLStylesContext& rStyles, sal_uInt16 nFamily, sal_Bool bDefaultStyle ) :
    XMLTextStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles, nFamily, bDefaultStyle ),
    nLastSheet(-1)
{
}

ScCellTextStyleContext::~ScCellTextStyleContext()
{
}

void ScCellTextStyleContext::FillPropertySet( const uno::Reference<beans::XPropertySet>& xPropSet )
{
    XMLTextStyleContext::FillPropertySet( xPropSet );

    ScXMLImport& rXMLImport = GetScImport();

    ScCellTextCursor* pCellImp = ScCellTextCursor::getImplementation( xPropSet );
    if (pCellImp)
    {
        ScAddress aPos = pCellImp->GetCellObj().GetPosition();
        if ( aPos.Tab() != nLastSheet )
        {
            ESelection aSel = pCellImp->GetSelection();

            ScSheetSaveData* pSheetData = ScModelObj::getImplementation(GetImport().GetModel())->GetSheetSaveData();
            pSheetData->AddTextStyle( GetName(), aPos, aSel );

            nLastSheet = aPos.Tab();
        }
    }
    else if ( rXMLImport.GetTables().GetCurrentSheet() != nLastSheet )
    {
        ScDrawTextCursor* pDrawImp = ScDrawTextCursor::getImplementation( xPropSet );
        if (pDrawImp)
        {
            XMLTableShapeImportHelper* pTableShapeImport = (XMLTableShapeImportHelper*)GetScImport().GetShapeImport().get();
            ScXMLAnnotationContext* pAnnotationContext = pTableShapeImport->GetAnnotationContext();
            if (pAnnotationContext)
            {
                pAnnotationContext->AddContentStyle( GetFamily(), GetName(), pDrawImp->GetSelection() );
                nLastSheet = rXMLImport.GetTables().GetCurrentSheet();
            }
        }

        // if it's a different shape, BlockSheet is called from XMLTableShapeImportHelper::finishShape
        // formatted text in page headers/footers can be ignored
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
