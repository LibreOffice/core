/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <xeextlst.hxx>
#include <xeroot.hxx>

#include <oox/export/utils.hxx>
#include <oox/token/namespaces.hxx>

using namespace ::oox;

XclExpExt::XclExpExt( const XclExpRoot& rRoot ):
    XclExpRoot(rRoot)
{
}

XclExtLst::XclExtLst( const XclExpRoot& rRoot ):
    XclExpRoot(rRoot)
{
}

XclExpExtNegativeColor::XclExpExtNegativeColor( const Color& rColor ):
    maColor(rColor)
{
}

void XclExpExtNegativeColor::SaveXml( XclExpXmlStream& rStrm )
{
    rStrm.GetCurrentStream()->singleElementNS( XML_x14, XML_negativeFillColor,
                                                XML_rgb, XclXmlUtils::ToOString( maColor ).getStr(),
                                                FSEND );
}

XclExpExtAxisColor::XclExpExtAxisColor( const Color& rColor ):
    maAxisColor(rColor)
{
}

void XclExpExtAxisColor::SaveXml( XclExpXmlStream& rStrm )
{
    rStrm.GetCurrentStream()->singleElementNS( XML_x14, XML_axisColor,
                                                XML_rgb, XclXmlUtils::ToOString( maAxisColor ).getStr(),
                                                FSEND );
}

XclExpExtIcon::XclExpExtIcon(const XclExpRoot& rRoot, const std::pair<ScIconSetType, sal_Int32>& rCustomEntry):
    XclExpRoot(rRoot),
    nIndex(rCustomEntry.second)
{
    pIconSetName = ScIconSetFormat::getIconSetName(rCustomEntry.first);
}

void XclExpExtIcon::SaveXml(XclExpXmlStream& rStrm)
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();

    if (nIndex == -1)
    {
        nIndex = 0;
        pIconSetName = "NoIcons";
    }

    rWorksheet->singleElementNS(XML_x14, XML_cfIcon,
            XML_iconSet, pIconSetName,
            XML_iconId, OString::number(nIndex).getStr(),
            FSEND);
}

XclExpExtCfvo::XclExpExtCfvo( const XclExpRoot& rRoot, const ScColorScaleEntry& rEntry, const ScAddress& rSrcPos, bool bFirst ):
    XclExpRoot(rRoot),
    meType(rEntry.GetType()),
    mbFirst(bFirst)
{
    if( rEntry.GetType() == COLORSCALE_FORMULA )
    {
        const ScTokenArray* pArr = rEntry.GetFormula();
        OUString aFormula;
        if(pArr)
        {
            aFormula = XclXmlUtils::ToOUString( GetCompileFormulaContext(), rSrcPos, pArr);
        }
        maValue = OUStringToOString(aFormula, RTL_TEXTENCODING_UTF8 );
    }
    else
        maValue = OString::number(rEntry.GetValue());
}

namespace {

const char* getColorScaleType( ScColorScaleEntryType eType, bool bFirst )
{
    switch(eType)
    {
        case COLORSCALE_MIN:
            return "min";
        case COLORSCALE_MAX:
            return "max";
        case COLORSCALE_PERCENT:
            return "percent";
        case COLORSCALE_FORMULA:
            return "formula";
        case COLORSCALE_AUTO:
            if(bFirst)
                return "autoMin";
            else
                return "autoMax";
        case COLORSCALE_PERCENTILE:
            return "percentile";
        default:
            break;
    }
    return "num";
}

}

void XclExpExtCfvo::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElementNS( XML_x14, XML_cfvo,
                                XML_type, getColorScaleType(meType, mbFirst),
                                FSEND );

    if (meType == COLORSCALE_FORMULA ||
            meType == COLORSCALE_PERCENT ||
            meType == COLORSCALE_PERCENTILE ||
            meType == COLORSCALE_VALUE)
    {
        rWorksheet->startElementNS(XML_xm, XML_f, FSEND);
        rWorksheet->writeEscaped(maValue.getStr());
        rWorksheet->endElementNS(XML_xm, XML_f);
    }

    rWorksheet->endElementNS(XML_x14, XML_cfvo);
}

XclExpExtDataBar::XclExpExtDataBar( const XclExpRoot& rRoot, const ScDataBarFormat& rFormat, const ScAddress& rPos ):
    XclExpRoot(rRoot)
{
    const ScDataBarFormatData& rFormatData = *rFormat.GetDataBarData();
    mpLowerLimit.reset(new XclExpExtCfvo(*this, *rFormatData.mpLowerLimit, rPos, true));
    mpUpperLimit.reset(new XclExpExtCfvo(*this, *rFormatData.mpUpperLimit, rPos, false));
    if (rFormatData.mpNegativeColor)
        mpNegativeColor.reset(new XclExpExtNegativeColor(*rFormatData.mpNegativeColor));
    else
        mpNegativeColor.reset( new XclExpExtNegativeColor( rFormatData.maPositiveColor ) );
    mpAxisColor.reset( new XclExpExtAxisColor( rFormatData.maAxisColor ) );

    meAxisPosition = rFormatData.meAxisPosition;
    mbGradient = rFormatData.mbGradient;
    mnMinLength = rFormatData.mnMinLength;
    mnMaxLength = rFormatData.mnMaxLength;
}

namespace {

const char* getAxisPosition(databar::ScAxisPosition eAxisPosition)
{
    switch(eAxisPosition)
    {
        case databar::NONE:
            return "none";
        case databar::AUTOMATIC:
            return "automatic";
        case databar::MIDDLE:
            return "middle";
    }
    return "";
}

}

void XclExpExtDataBar::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElementNS( XML_x14, XML_dataBar,
                                XML_minLength, OString::number(mnMinLength).getStr(),
                                XML_maxLength, OString::number(mnMaxLength).getStr(),
                                XML_axisPosition, getAxisPosition(meAxisPosition),
                                XML_gradient, ToPsz(mbGradient),
                                FSEND );

    mpLowerLimit->SaveXml( rStrm );
    mpUpperLimit->SaveXml( rStrm );
    mpNegativeColor->SaveXml( rStrm );
    mpAxisColor->SaveXml( rStrm );

    rWorksheet->endElementNS( XML_x14, XML_dataBar );
}

XclExpExtIconSet::XclExpExtIconSet(const XclExpRoot& rRoot, const ScIconSetFormat& rFormat, const ScAddress& rPos):
    XclExpRoot(rRoot)
{
    const ScIconSetFormatData& rData = *rFormat.GetIconSetData();
    for (auto const& itr : rData.m_Entries)
    {
        maCfvos.AppendNewRecord(new XclExpExtCfvo(*this, *itr, rPos, false));
    }
    mbCustom = rData.mbCustom;
    mbReverse = rData.mbReverse;
    mbShowValue = rData.mbShowValue;
    mpIconSetName = ScIconSetFormat::getIconSetName(rData.eIconSetType);

    if (mbCustom)
    {
        for (const auto& rItem : rData.maCustomVector)
        {
            maCustom.AppendNewRecord(new XclExpExtIcon(*this, rItem));
        }
    }
}

void XclExpExtIconSet::SaveXml(XclExpXmlStream& rStrm)
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();

    rWorksheet->startElementNS(XML_x14, XML_iconSet,
            XML_iconSet, mpIconSetName,
            XML_custom, mbCustom ? ToPsz10(mbCustom) : nullptr,
            XML_reverse, ToPsz10(mbReverse),
            XML_showValue, ToPsz10(mbShowValue),
            FSEND);

    maCfvos.SaveXml(rStrm);

    if (mbCustom)
    {
        maCustom.SaveXml(rStrm);
    }

    rWorksheet->endElementNS(XML_x14, XML_iconSet);
}

XclExpExtCfRule::XclExpExtCfRule( const XclExpRoot& rRoot, const ScFormatEntry& rFormat, const ScAddress& rPos, const OString& rId, sal_Int32 nPriority ):
    XclExpRoot(rRoot),
    maId(rId),
    pType(nullptr),
    mnPriority(nPriority)
{
    switch (rFormat.GetType())
    {
        case ScFormatEntry::Type::Databar:
        {
            const ScDataBarFormat& rDataBar = static_cast<const ScDataBarFormat&>(rFormat);
            mxEntry.reset( new XclExpExtDataBar( *this, rDataBar, rPos ) );
            pType = "dataBar";
        }
        break;
        case ScFormatEntry::Type::Iconset:
        {
            const ScIconSetFormat& rIconSet = static_cast<const ScIconSetFormat&>(rFormat);
            mxEntry.reset(new XclExpExtIconSet(*this, rIconSet, rPos));
            pType = "iconSet";
        }
        break;
        default:
        break;
    }
}

void XclExpExtCfRule::SaveXml( XclExpXmlStream& rStrm )
{
    if (!mxEntry)
        return;

    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElementNS( XML_x14, XML_cfRule,
                                XML_type, pType,
                                XML_priority, mnPriority == -1 ? nullptr : OString::number(mnPriority).getStr(),
                                XML_id, maId.getStr(),
                                FSEND );

    mxEntry->SaveXml( rStrm );

    rWorksheet->endElementNS( XML_x14, XML_cfRule );

}

XclExpExtConditionalFormatting::XclExpExtConditionalFormatting( const XclExpRoot& rRoot,
        std::vector<XclExpExtCondFormatData>& rData, const ScRangeList& rRange):
    XclExpRoot(rRoot),
    maRange(rRange)
{
    ScAddress aAddr = maRange.front().aStart;
    for (const auto& rItem : rData)
    {
        const ScFormatEntry* pEntry = rItem.pEntry;
        switch (pEntry->GetType())
        {
            case ScFormatEntry::Type::Iconset:
            {
                const ScIconSetFormat& rIconSet = static_cast<const ScIconSetFormat&>(*pEntry);
                bool bNeedsExt = false;
                switch (rIconSet.GetIconSetData()->eIconSetType)
                {
                    case IconSet_3Triangles:
                    case IconSet_3Smilies:
                    case IconSet_3ColorSmilies:
                    case IconSet_5Boxes:
                    case IconSet_3Stars:
                        bNeedsExt = true;
                    break;
                    default:
                    break;
                }

                if (rIconSet.GetIconSetData()->mbCustom)
                    bNeedsExt = true;

                if (bNeedsExt)
                {
                    maCfRules.AppendNewRecord(new XclExpExtCfRule(*this, *pEntry, aAddr, rItem.aGUID, rItem.nPriority));
                }
            }
            break;
            case ScFormatEntry::Type::Databar:
                maCfRules.AppendNewRecord(new XclExpExtCfRule( *this, *pEntry, aAddr, rItem.aGUID, rItem.nPriority));
            break;
            default:
            break;
        }
    }
}

void XclExpExtConditionalFormatting::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElementNS( XML_x14, XML_conditionalFormatting,
                                FSNS( XML_xmlns, XML_xm ), XclXmlUtils::ToOString(rStrm.getNamespaceURL(OOX_NS(xm))).getStr(),
                                FSEND );

    maCfRules.SaveXml( rStrm );
    rWorksheet->startElementNS( XML_xm, XML_sqref, FSEND );
    rWorksheet->write(XclXmlUtils::ToOString(maRange).getStr());

    rWorksheet->endElementNS( XML_xm, XML_sqref );

    rWorksheet->endElementNS( XML_x14, XML_conditionalFormatting );
}

XclExpExtCalcPr::XclExpExtCalcPr( const XclExpRoot& rRoot, formula::FormulaGrammar::AddressConvention eConv ):
    XclExpExt( rRoot )
{
    maURI = OString("{7626C862-2A13-11E5-B345-FEFF819CDC9F}");

    switch (eConv)
    {
        case formula::FormulaGrammar::CONV_OOO:
            maSyntax = OString("CalcA1");
            break;
        case formula::FormulaGrammar::CONV_XL_A1:
            maSyntax = OString("ExcelA1");
            break;
        case formula::FormulaGrammar::CONV_XL_R1C1:
            maSyntax = OString("ExcelR1C1");
            break;
        case formula::FormulaGrammar::CONV_A1_XL_A1:
            maSyntax = OString("CalcA1ExcelA1");
            break;
        case formula::FormulaGrammar::CONV_UNSPECIFIED:
        case formula::FormulaGrammar::CONV_ODF:
        case formula::FormulaGrammar::CONV_XL_OOX:
        case formula::FormulaGrammar::CONV_LOTUS_A1:
        case formula::FormulaGrammar::CONV_LAST:
            maSyntax = OString("Unspecified");
            break;
    }
}

void XclExpExtCalcPr::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_ext,
                                FSNS( XML_xmlns, XML_loext ), XclXmlUtils::ToOString(rStrm.getNamespaceURL(OOX_NS(loext))).getStr(),
                                XML_uri, maURI.getStr(),
                                FSEND );

    rWorksheet->singleElementNS( XML_loext, XML_extCalcPr,
                                 XML_stringRefSyntax, maSyntax.getStr(),
                                 FSEND );

    rWorksheet->endElement( XML_ext );
}

XclExpExtCondFormat::XclExpExtCondFormat( const XclExpRoot& rRoot ):
    XclExpExt( rRoot )
{
    maURI = OString("{78C0D931-6437-407d-A8EE-F0AAD7539E65}");
}

void XclExpExtCondFormat::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_ext,
                                FSNS( XML_xmlns, XML_x14 ), XclXmlUtils::ToOString(rStrm.getNamespaceURL(OOX_NS(xls14Lst))).getStr(),
                                XML_uri, maURI.getStr(),
                                FSEND );

    rWorksheet->startElementNS( XML_x14, XML_conditionalFormattings,
                                FSEND );

    maCF.SaveXml( rStrm );

    rWorksheet->endElementNS( XML_x14, XML_conditionalFormattings );
    rWorksheet->endElement( XML_ext );
}

void XclExpExtCondFormat::AddRecord( const XclExpExtConditionalFormattingRef& aEntry )
{
    maCF.AppendRecord( aEntry );
}

void XclExtLst::SaveXml( XclExpXmlStream& rStrm )
{
    if(maExtEntries.IsEmpty())
        return;

    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_extLst,
                                FSEND );

    maExtEntries.SaveXml(rStrm);

    rWorksheet->endElement( XML_extLst );
}

void XclExtLst::AddRecord( const XclExpExtRef& aEntry )
{
    maExtEntries.AppendRecord( aEntry );
}

XclExpExtRef XclExtLst::GetItem( XclExpExtType eType )
{
    size_t n = maExtEntries.GetSize();
    for( size_t i = 0; i < n; ++i )
    {
        if (maExtEntries.GetRecord( i )->GetType() == eType)
            return maExtEntries.GetRecord( i );
    }

    return XclExpExtRef();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
