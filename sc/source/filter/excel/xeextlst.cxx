/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xeextlst.hxx"
#include "xeroot.hxx"
#include "xehelper.hxx"
#include "xestyle.hxx"
#include "xename.hxx"
#include "xecontent.hxx"
#include "tokenarray.hxx"

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
            aFormula = XclXmlUtils::ToOUString( GetRoot().GetDoc(), rSrcPos,
                    pArr->Clone());
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
    rWorksheet->singleElementNS( XML_x14, XML_cfvo,
                                XML_type, getColorScaleType(meType, mbFirst),
                                XML_value, maValue.getStr(),
                                FSEND );
}

XclExpExtDataBar::XclExpExtDataBar( const XclExpRoot& rRoot, const ScDataBarFormat& rFormat, const ScAddress& rPos ):
    XclExpRoot(rRoot)
{
    const ScDataBarFormatData& rFormatData = *rFormat.GetDataBarData();
    mpLowerLimit.reset( new XclExpExtCfvo( *this, *rFormatData.mpLowerLimit.get(), rPos, true ) );
    mpUpperLimit.reset( new XclExpExtCfvo( *this, *rFormatData.mpUpperLimit.get(), rPos, false ) );
    if(rFormatData.mpNegativeColor.get())
        mpNegativeColor.reset( new XclExpExtNegativeColor( *rFormatData.mpNegativeColor.get() ) );
    else
        mpNegativeColor.reset( new XclExpExtNegativeColor( rFormatData.maPositiveColor ) );
    mpAxisColor.reset( new XclExpExtAxisColor( rFormatData.maAxisColor ) );

    meAxisPosition = rFormatData.meAxisPosition;
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
                                XML_minLength, OString::number(0).getStr(),
                                XML_maxLength, OString::number(100).getStr(),
                                XML_axisPosition, getAxisPosition(meAxisPosition),
                                FSEND );

    mpLowerLimit->SaveXml( rStrm );
    mpUpperLimit->SaveXml( rStrm );
    mpNegativeColor->SaveXml( rStrm );
    mpAxisColor->SaveXml( rStrm );

    rWorksheet->endElementNS( XML_x14, XML_dataBar );
}

XclExpExtCfRule::XclExpExtCfRule( const XclExpRoot& rRoot, const ScDataBarFormat& rFormat, const ScAddress& rPos, const OString& rId ):
    XclExpRoot(rRoot),
    maId(rId)
{
    maDataBar.reset( new XclExpExtDataBar( *this, rFormat, rPos ) );
}

void XclExpExtCfRule::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElementNS( XML_x14, XML_cfRule,
                                XML_type, "dataBar",
                                XML_id, maId.getStr(),
                                FSEND );

    maDataBar->SaveXml( rStrm );

    rWorksheet->endElementNS( XML_x14, XML_cfRule );

}

XclExpExtConditionalFormatting::XclExpExtConditionalFormatting( const XclExpRoot& rRoot, const ScDataBarFormat& rFormat, const ScAddress& rPos, const OString& rId ):
    XclExpRoot(rRoot)
{
    maCfRule.reset( new XclExpExtCfRule( *this, rFormat, rPos, rId ) );
    maRange = rFormat.GetRange();
}

void XclExpExtConditionalFormatting::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElementNS( XML_x14, XML_conditionalFormatting,
                                FSNS( XML_xmlns, XML_xm ), "http://schemas.microsoft.com/office/excel/2006/main",
                                FSEND );

    maCfRule->SaveXml( rStrm );
    rWorksheet->startElementNS( XML_xm, XML_sqref, FSEND );
    rWorksheet->write(XclXmlUtils::ToOString(maRange).getStr());

    rWorksheet->endElementNS( XML_xm, XML_sqref );

    rWorksheet->endElementNS( XML_x14, XML_conditionalFormatting );
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
                                FSNS( XML_xmlns, XML_x14 ), "http://schemas.microsoft.com/office/spreadsheetml/2009/9/main",
                                XML_uri, maURI.getStr(),
                                FSEND );

    rWorksheet->startElementNS( XML_x14, XML_conditionalFormattings,
                                FSEND );

    maCF.SaveXml( rStrm );

    rWorksheet->endElementNS( XML_x14, XML_conditionalFormattings );
    rWorksheet->endElement( XML_ext );
}

void XclExpExtCondFormat::AddRecord( XclExpExtConditionalFormattingRef aEntry )
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

void XclExtLst::AddRecord( XclExpExtRef aEntry )
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
