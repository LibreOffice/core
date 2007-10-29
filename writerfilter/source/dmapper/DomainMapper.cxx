/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DomainMapper.cxx,v $
 *
 *  $Revision: 1.63 $
 *
 *  last change: $Author: obo $ $Date: 2007-10-29 13:52:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef INCLUDED_DOMAINMAPPER_HXX
#include <dmapper/DomainMapper.hxx>
#endif
#ifndef INCLUDED_DMAPPER_DOMAINMAPPER_IMPL_HXX
#include <DomainMapper_Impl.hxx>
#endif
#ifndef INCLUDED_DMAPPER_CONVERSIONHELPER_HXX
#include <ConversionHelper.hxx>
#endif
#ifndef INCLUDED_I18NPOOL_MSLANGID_HXX
#include <i18npool/mslangid.hxx>
#endif
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextPortionAppend.hpp>
#include <com/sun/star/text/XParagraphAppend.hpp>
#ifndef _COM_SUN_STAR_TEXT_FONTEMPHASIS_HPP_
#include <com/sun/star/text/FontEmphasis.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTRELIEF_HPP_
#include <com/sun/star/awt/FontRelief.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTWEIGHT_HPP_
#include <com/sun/star/awt/FontWeight.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTUNDERLINE_HPP_
#include <com/sun/star/awt/FontUnderline.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSTRIKEOUT_HPP_
#include <com/sun/star/awt/FontStrikeout.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSLANT_HPP_
#include <com/sun/star/awt/FontSlant.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_PARAGRAPHADJUST_HPP_
#include <com/sun/star/style/ParagraphAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_CASEMAP_HPP_
#include <com/sun/star/style/CaseMap.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_LINESPACING_HPP_
#include <com/sun/star/style/LineSpacing.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_LINESPACINGMODE_HPP_
#include <com/sun/star/style/LineSpacingMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_BORDERLINE_HPP_
#include <com/sun/star/table/BorderLine.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTGRIDMODE_HPP_
#include <com/sun/star/text/TextGridMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRITINGMODE_HPP_
#include <com/sun/star/text/WritingMode.hpp>
#endif
#include <com/sun/star/text/XFootnote.hpp>
#ifndef _COM_SUN_STAR_STYLE_NUMBERINGTYPE_HPP_
#include <com/sun/star/style/NumberingType.hpp>
#endif
#include <comphelper/types.hxx>

#include <rtl/ustrbuf.hxx>
#include <boost/shared_ptr.hpp>
#include <com/sun/star/uno/Any.hxx>
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#include <vector>
#include <iostream>

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::writerfilter;
namespace dmapper{

/* ---- Fridrich's mess begins here ---- */
struct _PageSz
{
    sal_Int32 code;
    sal_Int32 h;
    bool      orient;
    sal_Int32 w;
} CT_PageSz;

struct _PageMar
{
    sal_Int32 top;
    sal_Int32 right;
    sal_Int32 bottom;
    sal_Int32 left;
    sal_Int32 header;
    sal_Int32 footer;
    sal_Int32 gutter;
} CT_PageMar;

struct _Column
{
    sal_Int32 w;
    sal_Int32 space;
} CT_Column;

struct _Columns
{
    bool equalWidth;
    sal_Int32 space;
    sal_Int32 num;
    bool sep;
    std::vector<_Column> cols;
} CT_Columns;

#if 0
struct _HdrFtrRef
{
   sal_Int32 type;
   ::rtl::OUString rId;
} CT_HdrFtrRef;

std::map<SectionPropertyMap::PageType, ::rtl::OUString> xHeaderStreamMap;
std::map<SectionPropertyMap::PageType, ::rtl::OUString> xFooterStreamMap;
#endif

/* ---- Fridrich's mess (hopefully) ends here ---- */

/*-- 09.06.2006 09:52:11---------------------------------------------------

  -----------------------------------------------------------------------*/
DomainMapper::DomainMapper( const uno::Reference< uno::XComponentContext >& xContext,
                            uno::Reference< lang::XComponent > xModel) :
    m_pImpl( new DomainMapper_Impl( *this, xContext, xModel )),
    mnBackgroundColor(0), mbIsHighlightSet(false)
{
}
/*-- 09.06.2006 09:52:12---------------------------------------------------

-----------------------------------------------------------------------*/
DomainMapper::~DomainMapper()
{
}
/*-- 09.06.2006 09:52:12---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::attribute(doctok::Id nName, doctok::Value & val)
{

    sal_Int32 nIntValue = val.getInt();
    rtl::OUString sStringValue = val.getString();
    //printf ( "DomainMapper::attribute(0x%.4x, 0x%.4x) [%s]\n", (unsigned int)nName, (unsigned int)nIntValue, ::rtl::OUStringToOString(sStringValue, RTL_TEXTENCODING_DONTKNOW).getStr());
    if( nName >= NS_rtf::LN_WIDENT && nName <= NS_rtf::LN_LCBSTTBFUSSR )
        m_pImpl->GetFIB().SetData( nName, nIntValue );
    else //if( !m_pImpl->getTableManager().attribute( nName, val) )
    {


        /* WRITERFILTERSTATUS: table: attributedata */
        switch( nName )
        {
            /* attributes to be ignored */
        case NS_rtf::LN_UNUSED4:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_UNUSED8:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_UNUSED1_3:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_UNUSED1_7:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_UNUSED8_3:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FWRITERESERVATION:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FLOADOVERRIDE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FFAREAST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCRYPTO:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_NFIBBACK:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LKEY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_ENVR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FMAC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FWORD97SAVED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCMAC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_PNFBPCHPFIRST_W6:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_PNCHPFIRST_W6:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_CPNBTECHP_W6:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_PNFBPPAPFIRST_W6:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_PNPAPFIRST_W6:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_CPNBTEPAP_W6:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_PNFBPLVCFIRST_W6:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_PNLVCFIRST_W6:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_CPNBTELVC_W6:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_CBMAC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LPRODUCTCREATED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LPRODUCTREVISED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_CCPMCR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_PNFBPCHPFIRST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_PNFBPPAPFIRST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_PNFBPLVCFIRST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCISLANDFIRST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCISLANDLIM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCSTSHFORIG:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBSTSHFORIG:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCFPAD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCFPAD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCSTTBFGLSY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBSTTBFGLSY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCFGLSY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCFGLSY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCFSEA:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCFSEA:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCFFLDMCR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCFFLDMCR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCCMDS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBCMDS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCMCR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCMCR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCSTTBFMCR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBSTTBFMCR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPRDRVR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPRDRVR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPRENVPORT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPRENVPORT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPRENVLAND:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPRENVLAND:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCWSS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBWSS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCFPGDFTN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCFPGDFTN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCAUTOSAVESOURCE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBAUTOSAVESOURCE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCDOAMOM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCDOAMOM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCDOAHDR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCDOAHDR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPMS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPMS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCFPGDEDN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCFPGDEDN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCFWKB:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCFWKB:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCFSPL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCFSPL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCSTWUSER:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBSTWUSER:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCUNUSED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBUNUSED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCSTTBFINTLFLD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBSTTBFINTLFLD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCROUTESLIP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBROUTESLIP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCSTTBSAVEDBY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBSTTBSAVEDBY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCSTTBFNM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBSTTBFNM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCDOCUNDO:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBDOCUNDO:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCRGBUSE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBRGBUSE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCUSP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBUSP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCUSKF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBUSKF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCUPCRGBUSE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCUPCRGBUSE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCUPCUSP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCUPCUSP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLGOSL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLGOSL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCOCX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCOCX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_DWLOWDATETIME:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_DWHIGHDATETIME:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCASUMY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCASUMY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCFGRAM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCFGRAM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCSTTBFUSSR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;

        case NS_rtf::LN_ISTD: //index of applied style
            /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
            {
            //search for the style with the given id and apply it
            //as CharStyleName or ParaStyleName
            const StyleSheetEntry* pEntry = m_pImpl->GetStyleSheetTable()->FindStyleSheetByISTD(
                                              ::rtl::OUString::valueOf(static_cast<sal_Int32>(nIntValue), 16));
            if(pEntry)
            {
                bool bParaStyle = (pEntry->nStyleTypeCode == STYLE_TYPE_PARA);
                if(bParaStyle)
                    m_pImpl->SetCurrentParaStyleId(::rtl::OUString::valueOf(static_cast<sal_Int32>(nIntValue), 16));
                m_pImpl->GetTopContext()->Insert(
                                                 bParaStyle ?
                                                 PROP_PARA_STYLE_NAME  : PROP_CHAR_STYLE_NAME,
                                                 true,
                                                 uno::makeAny(
                                                 m_pImpl->GetStyleSheetTable()->ConvertStyleName( pEntry->sStyleName ) ) );
            }
        }
        break;
        case NS_rtf::LN_ISTARTAT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_NFC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FLEGAL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FNORESTART:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FPREV:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FPREVSPACE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FWORD6:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_UNUSED5_7:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_RGBXCHNUMS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_IXCHFOLLOW:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DXASPACE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DXAINDENT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_CBGRPPRLCHPX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_CBGRPPRLPAPX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LSID:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_TPLC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_RGISTD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FSIMPLELIST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FRESTARTHDN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_UNSIGNED26_2:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_ILVL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FSTARTAT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FFORMATTING:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_UNSIGNED4_6:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_CLFOLVL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_CBFFNM1:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_PRQ:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FTRUETYPE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_WWEIGHT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_CHS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

            {
                m_pImpl->GetFIB().SetLNCHS( nIntValue );
            }
            break;
        case NS_rtf::LN_IXCHSZALT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_PANOSE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_STI:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FSCRATCH:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FINVALHEIGHT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FHASUPE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FMASSCOPY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_SGC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_ISTDBASE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_CUPX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_ISTDNEXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_BCHUPE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FAUTOREDEF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FHIDDEN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_CSTD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_CBSTDBASEINFILE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FSTDSTYLENAMESWRITTEN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_UNUSED4_2:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_STIMAXWHENSAVED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_ISTDMAXFIXEDWHENSAVED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_NVERBUILTINNAMESWHENSAVED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_RGFTCSTANDARDCHPSTSH:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_WIDENT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_NFIB:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_NPRODUCT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_rtf::LN_LID:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_rtf::LN_PNNEXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_rtf::LN_FDOT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_rtf::LN_FGLSY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_rtf::LN_FCOMPLEX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_rtf::LN_FHASPIC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CQUICKSAVES:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FENCRYPTED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FWHICHTBLSTM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FREADONLYRECOMMENDED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FEXTCHAR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FEMPTYSPECIAL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FLOADOVERRIDEPAGE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FFUTURESAVEDUNDO:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FSPARE0:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CHSTABLES:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCMIN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CSW:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_WMAGICCREATED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_WMAGICREVISED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_WMAGICCREATEDPRIVATE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */


        case NS_rtf::LN_WMAGICREVISEDPRIVATE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LIDFE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CLW:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CCPTEXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */


        case NS_rtf::LN_CCPFTN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CCPHDD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CCPATN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CCPEDN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CCPTXBX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CCPHDRTXBX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_PNCHPFIRST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CPNBTECHP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_PNPAPFIRST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CPNBTEPAP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_PNLVCFIRST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CPNBTELVC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CFCLCB:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSTSHF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBSTSHF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFFNDREF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFFNDREF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFFNDTXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFFNDTXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFANDREF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFANDREF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFANDTXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFANDTXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFSED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFSED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFPHE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFPHE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFHDD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFHDD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFBTECHPX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFBTECHPX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */


        case NS_rtf::LN_FCPLCFBTEPAPX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFBTEPAPX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSTTBFFFN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBSTTBFFFN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFFLDMOM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFFLDMOM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFFLDHDR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFFLDHDR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */


        case NS_rtf::LN_FCPLCFFLDFTN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFFLDFTN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFFLDATN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFFLDATN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSTTBFBKMK:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBSTTBFBKMK:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFBKF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFBKF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFBKL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFBKL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCDOP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBDOP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSTTBFASSOC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBSTTBFASSOC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCCLX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBCLX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCGRPXSTATNOWNERS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBGRPXSTATNOWNERS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSTTBFATNBKMK:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBSTTBFATNBKMK:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCSPAMOM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCSPAMOM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCSPAHDR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */


        case NS_rtf::LN_LCBPLCSPAHDR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFATNBKF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFATNBKF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFATNBKL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFATNBKL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCFORMFLDSTTBF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBFORMFLDSTTBF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFENDREF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */


        case NS_rtf::LN_LCBPLCFENDREF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFENDTXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFENDTXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFFLDEDN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFFLDEDN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCDGGINFO:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBDGGINFO:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSTTBFRMARK:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */


        case NS_rtf::LN_LCBSTTBFRMARK:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSTTBFCAPTION:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBSTTBFCAPTION:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSTTBFAUTOCAPTION:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBSTTBFAUTOCAPTION:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFTXBXTXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFFLDTXBX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFFLDTXBX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFHDRTXBXTXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFHDRTXBXTXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFFLDHDRTXBX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFFLDHDRTXBX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSTTBTTMBD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBSTTBTTMBD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPGDMOTHER:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPGDMOTHER:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCBKDMOTHER:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBBKDMOTHER:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPGDFTN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPGDFTN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCBKDFTN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBBKDFTN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPGDEDN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPGDEDN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */


        case NS_rtf::LN_FCBKDEDN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBBKDEDN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFLST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */


        case NS_rtf::LN_LCBPLCFLST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLFLFO:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLFLFO:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFTXBXBKD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFTXBXBKD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFTXBXHDRBKD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFTXBXHDRBKD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSTTBGLSYSTYLE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */


        case NS_rtf::LN_LCBSTTBGLSYSTYLE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFBTELVC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFBTELVC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFLVC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFLVC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSTTBLISTNAMES:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBSTTBLISTNAMES:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBSTTBFUSSR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            {
                m_pImpl->GetFIB().SetData( nName, nIntValue );
            }
            break;
        case NS_rtf::LN_FN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSEPX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FNMPR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCMPR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

            //section descriptor, unused or internally used
            break;
        case NS_rtf::LN_ICOFORE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_ICOBACK:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_IPAT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_SHDFORECOLOR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_SHDBACKCOLOR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_SHDPATTERN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DPTLINEWIDTH:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_BRCTYPE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_ICO:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DPTSPACE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FSHADOW:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FFRAME:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_UNUSED2_15:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FFIRSTMERGED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FMERGED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FVERTICAL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FBACKWARD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FROTATEFONT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FVERTMERGE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FVERTRESTART:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_VERTALIGN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FUNUSED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_BRCTOP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_rtf::LN_BRCLEFT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_rtf::LN_BRCBOTTOM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_rtf::LN_BRCRIGHT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        {
            table::BorderLine aBorderLine;
            sal_Int32 nLineDistance = ConversionHelper::MakeBorderLine( nIntValue, aBorderLine );
            (void)nLineDistance;
            PropertyIds eBorderId = PROP_LEFT_BORDER;
            PropertyIds eBorderDistId = PROP_LEFT_BORDER_DISTANCE  ;
            switch( nName )
            {
                case NS_rtf::LN_BRCTOP:
                    eBorderId = PROP_TOP_BORDER            ;
                    eBorderDistId = PROP_TOP_BORDER_DISTANCE;
                break;
                case NS_rtf::LN_BRCLEFT:
//                    eBorderId = PROP_LEFT_BORDER;
//                    eBorderDistId = PROP_LEFT_BORDER_DISTANCE  ;
                break;
                case NS_rtf::LN_BRCBOTTOM:
                    eBorderId = PROP_BOTTOM_BORDER         ;
                    eBorderDistId = PROP_BOTTOM_BORDER_DISTANCE;
                break;
                case NS_rtf::LN_BRCRIGHT:
                    eBorderId = PROP_RIGHT_BORDER          ;
                    eBorderDistId = PROP_RIGHT_BORDER_DISTANCE ;
                break;
                default:;
            }
            //todo: where to put the border properties
            //rContext->Insert(eBorderId, uno::makeAny( aBorderLine ));
            //rContext->Insert(eBorderDistId, uno::makeAny( nLineDistance ));
        }
        break;
        case NS_rtf::LN_IBKL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_ITCFIRST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FPUB:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_ITCLIM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FCOL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LINECOLOR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LINEWIDTH:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LINETYPE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_MM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_XEXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_YEXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_HMF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LCB:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_CBHEADER:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_MFP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_BM_RCWINMF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DXAGOAL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DYAGOAL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_MX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_MY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DXACROPLEFT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DYACROPTOP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DXACROPRIGHT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DYACROPBOTTOM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_BRCL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FFRAMEEMPTY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FBITMAP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FDRAWHATCH:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FERROR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_BPP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DXAORIGIN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DYAORIGIN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_CPROPS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LINEPROPSTOP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LINEPROPSLEFT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LINEPROPSBOTTOM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LINEPROPSRIGHT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LINEPROPSHORIZONTAL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LINEPROPSVERTICAL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_headerr:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_footerr:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_endnote:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_BOOKMARKNAME:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            // sStringValue contains the bookmark name
            // if it is already known then it's time to insert the bookmark
            m_pImpl->AddBookmark( sStringValue );
        break;

        case NS_rtf::LN_LISTLEVEL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LFO:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_F:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_ALTFONTNAME:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_XSZFFN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_XSTZNAME:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_XSTZNAME1:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_UPXSTART:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_UPX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_sed:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            //section properties
            resolveAttributeProperties(val);
            break;
        case NS_rtf::LN_tbdAdd:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            //
            {
                doctok::Reference<Properties>::Pointer_t pProperties = val.getProperties();
                if( pProperties.get())
                {
                    pProperties->resolve(*this);
                    //increment to the next tab stop
                    m_pImpl->NextTabStop();
                }
            }
            break;
        case NS_rtf::LN_dxaDel:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            //deleted tab
        case NS_rtf::LN_dxaAdd:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            //set tab
        case NS_rtf::LN_TLC:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            //tab leading characters - for decimal tabs
        case NS_rtf::LN_JC:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            //tab justification
            m_pImpl->ModifyCurrentTabStop(nName, nIntValue);
            break;
        case NS_rtf::LN_UNUSED0_6:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            // really unused
            break;
        case NS_rtf::LN_rgbrc:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_shd:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_cellShd:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_cellTopColor:
        case NS_rtf::LN_cellLeftColor:
        case NS_rtf::LN_cellBottomColor:
        case NS_rtf::LN_cellRightColor:
            OSL_ASSERT("handled by DomainMapperTableManager");
        break;

        case NS_rtf::LN_LISTTABLE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LFOTABLE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FONTTABLE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_STYLESHEET:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;

        case NS_rtf::LN_fcEastAsianLayout:
        /*  it seems that the value is following:
                ???? XX YYYY ZZ
            where
                XX seems to be the run id
                ZZ is the length of the function that is normally 6
                Lower byte of YYYY determines whether it is
                    vertical text flow (0x01), or
                    two lines in one layout (0x02).
                For 0x01, if the higher byte of YYYY is zero, the text is not scaled to fit the line height,
                    in oposite case, it is to be scaled.
                For 0x02, the higher byte of YYYY is determinig the prefix and suffix of the run:
                    no brackets (0x00) ,
                    () round brackets (0x01),
                    [] square backets (0x02),
                    <> angle brackets (0x03) and
                    {} curly brackets (0x04).
                ???? is different and we do not know its signification
          */

            if ((nIntValue & 0x000000FF) == 6)
            {
                switch ((nIntValue & 0x0000FF00) >> 8)
                {
                case 1: // vertical text
                    m_pImpl->GetTopContext()->Insert(PROP_CHAR_ROTATION, true, uno::makeAny ( sal_Int16(900) ));
                    m_pImpl->GetTopContext()->Insert(PROP_CHAR_ROTATION_IS_FIT_TO_LINE, true, uno::makeAny (((nIntValue & 0x00FF0000) >> 16) != 0));
                    break;
                case 2: // two lines in one
                    m_pImpl->GetTopContext()->Insert(PROP_CHAR_COMBINE_IS_ON, true, uno::makeAny ( true ));
                    m_pImpl->GetTopContext()->Insert(PROP_CHAR_COMBINE_PREFIX, true, uno::makeAny ( getBracketStringFromEnum((nIntValue & 0x00FF0000) >> 16)));
                    m_pImpl->GetTopContext()->Insert(PROP_CHAR_COMBINE_SUFFIX, true, uno::makeAny ( getBracketStringFromEnum((nIntValue & 0x00FF0000) >> 16, false)));
                    break;
                default:
                    break;
                }
            }
            break;
        case NS_rtf::LN_FRD : //footnote reference descriptor, if nIntValue > 0 then automatic, custom otherwise
            //ignored
        break;
        case NS_rtf::LN_FONT: //font of footnote symbol
            m_pImpl->GetTopContext()->SetFootnoteFontId( nIntValue );
        break;
        case NS_ooxml::LN_CT_Sym_char:
        case NS_rtf::LN_CHAR: //footnote symbol character
            m_pImpl->GetTopContext()->SetFootnoteSymbol( sal_Unicode(nIntValue));
        break;
        case NS_ooxml::LN_CT_Sym_font:
            m_pImpl->GetTopContext()->SetFootnoteFontName( sStringValue );
        break;
        case NS_ooxml::LN_CT_Underline_val:
            handleUnderlineType(nIntValue, m_pImpl->GetTopContext());
            break;
        case NS_ooxml::LN_CT_Color_val:
            m_pImpl->GetTopContext()->Insert(PROP_CHAR_COLOR, true, uno::makeAny( nIntValue ) );
            break;
        case NS_ooxml::LN_CT_Underline_color:
            m_pImpl->GetTopContext()->Insert(PROP_CHAR_UNDERLINE_HAS_COLOR, true, uno::makeAny( true ) );
            m_pImpl->GetTopContext()->Insert(PROP_CHAR_UNDERLINE_COLOR, true, uno::makeAny( nIntValue ) );
            break;

        case NS_ooxml::LN_CT_TabStop_val:
            if (nIntValue == NS_ooxml::LN_Value_ST_TabJc_clear)
                m_pImpl->m_aCurrentTabStop.bDeleted = true;
            else
            {
                m_pImpl->m_aCurrentTabStop.bDeleted = false;
                m_pImpl->m_aCurrentTabStop.Alignment = getTabAlignFromValue(nIntValue);
            }
            break;
        case NS_ooxml::LN_CT_TabStop_leader:
            m_pImpl->m_aCurrentTabStop.FillChar = getFillCharFromValue(nIntValue);
            break;
        case NS_ooxml::LN_CT_TabStop_pos:
            m_pImpl->m_aCurrentTabStop.Position = ConversionHelper::convertToMM100(nIntValue);
            break;

        case NS_ooxml::LN_CT_Fonts_ascii:
            m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME, true, uno::makeAny( sStringValue ));
            break;
        case NS_ooxml::LN_CT_Fonts_eastAsia:
            m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME_ASIAN, true, uno::makeAny( sStringValue ));
            break;
        case NS_ooxml::LN_CT_Fonts_cs:
            m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME_COMPLEX, true, uno::makeAny( sStringValue ));
            break;
        case NS_ooxml::LN_CT_Spacing_before:
            m_pImpl->GetTopContext()->Insert(PROP_PARA_TOP_MARGIN, true, uno::makeAny( ConversionHelper::convertToMM100( nIntValue ) ));
            break;
        case NS_ooxml::LN_CT_Spacing_beforeLines:
            break;
        case NS_ooxml::LN_CT_Spacing_after:
            m_pImpl->GetTopContext()->Insert(PROP_PARA_BOTTOM_MARGIN, true, uno::makeAny( ConversionHelper::convertToMM100( nIntValue ) ));
            break;
        case NS_ooxml::LN_CT_Spacing_afterLines:
            break;
        case NS_ooxml::LN_CT_Spacing_line:
            {
                style::LineSpacing aSpacing;
                aSpacing.Mode = style::LineSpacingMode::PROP;
                aSpacing.Height = sal_Int16(sal_Int32(nIntValue) * 100 /240);
                m_pImpl->GetTopContext()->Insert(PROP_PARA_LINE_SPACING, true, uno::makeAny( aSpacing ));
            }
            break;

        case NS_ooxml::LN_CT_Ind_left:
            m_pImpl->GetTopContext()->Insert(
                PROP_PARA_LEFT_MARGIN, true, uno::makeAny( ConversionHelper::convertToMM100(nIntValue ) ));
            break;
        case NS_ooxml::LN_CT_Ind_right:
            m_pImpl->GetTopContext()->Insert(
                PROP_PARA_RIGHT_MARGIN, true, uno::makeAny( ConversionHelper::convertToMM100(nIntValue ) ));
            break;
        case NS_ooxml::LN_CT_Ind_hanging:
            m_pImpl->GetTopContext()->Insert(
                PROP_PARA_FIRST_LINE_INDENT, true, uno::makeAny( - ConversionHelper::convertToMM100(nIntValue ) ));
            break;
        case NS_ooxml::LN_CT_Ind_firstLine:
            m_pImpl->GetTopContext()->Insert(
                PROP_PARA_FIRST_LINE_INDENT, true, uno::makeAny( ConversionHelper::convertToMM100(nIntValue ) ));
            break;

        case NS_ooxml::LN_CT_EastAsianLayout_id:
            break;
        case NS_ooxml::LN_CT_EastAsianLayout_combine:
            m_pImpl->GetTopContext()->Insert(PROP_CHAR_COMBINE_IS_ON, true, uno::makeAny ( nIntValue ? true : false ));
            break;
        case NS_ooxml::LN_CT_EastAsianLayout_combineBrackets:
            {
                rtl::OUString sCombinePrefix = getBracketStringFromEnum(nIntValue);
                rtl::OUString sCombineSuffix = getBracketStringFromEnum(nIntValue, false);
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_COMBINE_PREFIX, true, uno::makeAny ( sCombinePrefix ));
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_COMBINE_SUFFIX, true, uno::makeAny ( sCombineSuffix ));
            }
            break;
        case NS_ooxml::LN_CT_EastAsianLayout_vert:
            {
                sal_Int16 nRotationAngle = (nIntValue ? 900 : 0);
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_ROTATION, true, uno::makeAny ( nRotationAngle ));
            }
            break;
        case NS_ooxml::LN_CT_EastAsianLayout_vertCompress:
            m_pImpl->GetTopContext()->Insert(PROP_CHAR_ROTATION_IS_FIT_TO_LINE, true, uno::makeAny ( nIntValue ? true : false));
            break;

        case NS_ooxml::LN_CT_PageSz_code:
            CT_PageSz.code = nIntValue;
            break;
        case NS_ooxml::LN_CT_PageSz_h:
            {
                sal_Int32 nHeight = ConversionHelper::SnapPageDimension(nIntValue);
                CT_PageSz.h = ConversionHelper::convertToMM100(nHeight);
            }
            break;
        case NS_ooxml::LN_CT_PageSz_orient:
            CT_PageSz.orient = (nIntValue != 0);
            break;
        case NS_ooxml::LN_CT_PageSz_w:
            {
                sal_Int32 nWidth = ConversionHelper::SnapPageDimension(nIntValue);
                CT_PageSz.w = ConversionHelper::convertToMM100(nWidth);
            }
            break;

        case NS_ooxml::LN_CT_PageMar_top:
            CT_PageMar.top = ConversionHelper::convertToMM100(nIntValue);
            break;
        case NS_ooxml::LN_CT_PageMar_right:
            CT_PageMar.right = ConversionHelper::convertToMM100(nIntValue);
            break;
        case NS_ooxml::LN_CT_PageMar_bottom:
            CT_PageMar.bottom = ConversionHelper::convertToMM100(nIntValue);
            break;
        case NS_ooxml::LN_CT_PageMar_left:
            CT_PageMar.left = ConversionHelper::convertToMM100(nIntValue);
            break;
        case NS_ooxml::LN_CT_PageMar_header:
            CT_PageMar.header = ConversionHelper::convertToMM100(nIntValue);
            break;
        case NS_ooxml::LN_CT_PageMar_footer:
            CT_PageMar.footer = ConversionHelper::convertToMM100(nIntValue);
            break;
        case NS_ooxml::LN_CT_PageMar_gutter:
            CT_PageMar.gutter = ConversionHelper::convertToMM100(nIntValue);
            break;

        case NS_ooxml::LN_CT_Columns_equalWidth:
            CT_Columns.equalWidth = (nIntValue != 0);
            break;
        case NS_ooxml::LN_CT_Columns_space:
            CT_Columns.space = ConversionHelper::convertToMM100( nIntValue );
            break;
        case NS_ooxml::LN_CT_Columns_num:
            CT_Columns.num = nIntValue;
            break;
        case NS_ooxml::LN_CT_Columns_sep:
            CT_Columns.sep = (nIntValue != 0);
            break;

        case NS_ooxml::LN_CT_Column_w:
            CT_Column.w = ConversionHelper::convertToMM100( nIntValue );
            break;
        case NS_ooxml::LN_CT_Column_space:
            CT_Column.space = ConversionHelper::convertToMM100( nIntValue );
            break;

        case NS_ooxml::LN_CT_PPrBase_pStyle:
            m_pImpl->GetTopContext()->Insert( PROP_PARA_STYLE_NAME, true, uno::makeAny( m_pImpl->GetStyleSheetTable()->ConvertStyleName( sStringValue )));
            break;
        case NS_ooxml::LN_EG_RPrBase_rStyle:
            m_pImpl->GetTopContext()->Insert( PROP_CHAR_STYLE_NAME, true, uno::makeAny( m_pImpl->GetStyleSheetTable()->ConvertStyleName( sStringValue )));
            break;

        default:
            {
                OSL_ASSERT("unknown attribute");
            }
        }
    }
}
/*-- 09.06.2006 09:52:12---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::sprm(doctok::Sprm & rSprm)
{
    if( !m_pImpl->getTableManager().sprm(rSprm))
        DomainMapper::sprm( rSprm, m_pImpl->GetTopContext() );
}
/*-- 20.06.2006 09:58:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper::sprm( doctok::Sprm& rSprm, PropertyMapPtr rContext, SprmType eSprmType )
{
    OSL_ENSURE(rContext.get(), "PropertyMap has to be valid!");
    if(!rContext.get())
        return ;

    sal_uInt32 nSprmId = rSprm.getId();
    //needed for page properties
    SectionPropertyMap* pSectionContext = 0;
    //the section context is not availabe before the first call of startSectionGroup()
    if( !m_pImpl->IsStyleSheetImport() )
    {
        PropertyMapPtr pContext = m_pImpl->GetTopContextOfType(CONTEXT_SECTION);
        OSL_ENSURE(pContext.get(), "Section context is not in the stack!");
        pSectionContext = dynamic_cast< SectionPropertyMap* >( pContext.get() );
    }

    //TODO: In rtl-paragraphs the meaning of left/right are to be exchanged
    bool bExchangeLeftRight = false;
    // if( nSprmId == 0x2461 && AlreadyInRTLPara() )
    //      bExchangeLeftRight = true;
    doctok::Value::Pointer_t pValue = rSprm.getValue();
    sal_Int32 nIntValue = pValue->getInt();
    rtl::OUString sStringValue = pValue->getString();
    //printf ( "DomainMapper::sprm(0x%.4x, 0x%.4x) [%s]\n", (unsigned int)nSprmId, (unsigned int)nIntValue, ::rtl::OUStringToOString(sStringValue, RTL_TEXTENCODING_DONTKNOW).getStr());
    /* WRITERFILTERSTATUS: table: sprmdata */

    switch(nSprmId)
    {
    case 2:  // sprmPIstd
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
    case 0x4600:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPIstd - style code
    case 3: // "sprmPIstdPermute
    case 0xC601:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPIstdPermute
    case 0x2602:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPIncLvl
    case 0x2461: // sprmPJc Asian (undocumented)
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
    case 0x2403: // sprmPJc
        handleParaJustification(nIntValue, rContext, bExchangeLeftRight);
        break;
    case 0x2404:
        /* WRITERFILTERSTATUS: done: 0, planned: 3, spent: 0 */
        /* WRITERFILTERSTATUS: comment: */
        break;  // sprmPFSideBySide

    case 0x2405:   // sprmPFKeep
        /* WRITERFILTERSTATUS: done: 0, planned: 3, spent: 0 */
        /* WRITERFILTERSTATUS: comment: */
        break;
    case 0x2406:   // sprmPFKeepFollow
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 1 */
        /* WRITERFILTERSTATUS: comment:  */
        rContext->Insert(PROP_PARA_KEEP_TOGETHER, true, uno::makeAny( nIntValue ? true : false) );
        break;
    case 0x2407:
        /* WRITERFILTERSTATUS: done: 0, planned: 3, spent: 0 */
        /* WRITERFILTERSTATUS: comment:  */
        break;  // sprmPFPageBreakBefore
    case 0x2408:
        break;  // sprmPBrcl
    case 0x2409:
        break;  // sprmPBrcp
    case 0x260A: // sprmPIlvl
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 1 */
        /* WRITERFILTERSTATUS: comment:  */
        rContext->Insert( PROP_NUMBERING_LEVEL, true, uno::makeAny( (sal_Int16)nIntValue ));
        break;
    case 0x460B: // sprmPIlfo
        /* WRITERFILTERSTATUS: done: 50, planned: 0, spent: 1 */
        /* WRITERFILTERSTATUS: comment:  */
        {
            //convert the ListTable entry to a NumberingRules propery and apply it
            sal_Int32 nListId = m_pImpl->GetLFOTable()->GetListID( nIntValue );
            if(nListId >= 0)
            {
                ListTablePtr pListTable = m_pImpl->GetListTable();
                rContext->Insert( PROP_NUMBERING_RULES, true,
                                  uno::makeAny(pListTable->GetNumberingRules(nListId)));
                //TODO: Merge overwrittern numbering levels from LFO table
            }
        }
        break;
    case 0x240C:   // sprmPFNoLineNumb
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 1 */
        /* WRITERFILTERSTATUS: comment:  */
        rContext->Insert(PROP_PARA_LINE_NUMBER_COUNT, true, uno::makeAny( nIntValue ? false : true) );
        break;
    case 0xC60D:   // sprmPChgTabsPapx
        /* WRITERFILTERSTATUS: done: 90, planned: 8, spent: 8 */
        /* WRITERFILTERSTATUS: comment: bar tab stops a unavailable */
        {
            // Initialize tab stop vector from style sheet
            uno::Any aValue = m_pImpl->GetPropertyFromStyleSheet(PROP_PARA_TAB_STOPS);
            uno::Sequence< style::TabStop > aStyleTabStops;
            if(aValue >>= aStyleTabStops)
            {
                m_pImpl->InitTabStopFromStyle( aStyleTabStops );
            }

            //create a new tab stop property - this is done with the contained properties
            resolveSprmProps(rSprm);
            //add this property
            rContext->Insert(PROP_PARA_TAB_STOPS, true, uno::makeAny( m_pImpl->GetCurrentTabStopAndClear()));
        }
        break;
    case 0x845d:    //right margin Asian - undocumented
    case 0x845e:    //left margin Asian - undocumented
    case 16:      // sprmPDxaRight - right margin
    case 0x840E:   // sprmPDxaRight - right margin
    case 17:
    case 0x840F:   // sprmPDxaLeft
        /* WRITERFILTERSTATUS: done: 50, planned: 5, spent: 1 */
        if( 0x840F == nSprmId || 0x17 == nSprmId|| (bExchangeLeftRight && nSprmId == 0x845d) || ( !bExchangeLeftRight && nSprmId == 0x845e))
            rContext->Insert(
                             eSprmType == SPRM_DEFAULT ? PROP_PARA_LEFT_MARGIN : PROP_LEFT_MARGIN,
                             true,
                             uno::makeAny( ConversionHelper::convertToMM100( nIntValue ) ));
        else if(eSprmType == SPRM_DEFAULT)
            rContext->Insert(
                             PROP_PARA_RIGHT_MARGIN, true,
                             uno::makeAny( ConversionHelper::convertToMM100(nIntValue ) ));
        //TODO: what happens to the right margins in numberings?
        break;
    case 18: // sprmPNest
    case 0x4610: // sprmPNest
        //not handled in the old WW8 filter
        break;
    case 0x8460:    //first line indent Asian - undocumented
    case 19:
    case 0x8411:   // sprmPDxaLeft1
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 1 */
        rContext->Insert(
                         eSprmType == SPRM_DEFAULT ? PROP_PARA_FIRST_LINE_INDENT : PROP_FIRST_LINE_OFFSET,
                         true,
                         uno::makeAny( ConversionHelper::convertToMM100(nIntValue ) ));
        break;
    case 20 : // sprmPDyaLine
    case 0x6412:   // sprmPDyaLine
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 1 */
        {
            style::LineSpacing aSpacing;
            sal_Int16 nDistance = sal_Int16(nIntValue & 0xffff);
            if(nIntValue & 0xffff0000)
            {
                // single line in Writer is 100, in Word it is 240
                aSpacing.Mode = style::LineSpacingMode::PROP;
                aSpacing.Height = sal_Int16(sal_Int32(nDistance) * 100 /240);
            }
            else
            {
                if(nDistance < 0)
                {
                    aSpacing.Mode = style::LineSpacingMode::FIX;
                    aSpacing.Height = sal_Int16(ConversionHelper::convertToMM100(-nDistance));
                }
                else if(nDistance >0)
                {
                    aSpacing.Mode = style::LineSpacingMode::MINIMUM;
                    aSpacing.Height = sal_Int16(ConversionHelper::convertToMM100(nDistance));
                }
            }
            rContext->Insert(PROP_PARA_LINE_SPACING, true, uno::makeAny( aSpacing ));
        }
        break;
    case 21 : // legacy version
    case 0xA413:   // sprmPDyaBefore
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 1 */
        rContext->Insert(PROP_PARA_TOP_MARGIN, true, uno::makeAny( ConversionHelper::convertToMM100( nIntValue ) ));
        break;
    case 22 :
    case 0xA414:   // sprmPDyaAfter
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 1 */
        rContext->Insert(PROP_PARA_BOTTOM_MARGIN, true, uno::makeAny( ConversionHelper::convertToMM100( nIntValue ) ));
        break;

    case  23: //sprmPChgTabs
    case 0xC615: // sprmPChgTabs
        /* WRITERFILTERSTATUS: done: 0, planned: 3, spent: 0 */
        OSL_ASSERT("unhandled");
        //tabs of list level?
        break;
    case 24: // "sprmPFInTable"
    case 0x2416:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFInTable
    case 0x6649: //sprmPTableDepth
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        //not handled via sprm but via text( 0x07 )
    break;
    case 25: // "sprmPTtp" pap.fTtp
    case 0x2417:   // sprmPFTtp  was: Read_TabRowEnd
        break;
    case 26:  // "sprmPDxaAbs
    case 0x8418:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPDxaAbs
    case 27: //sprmPDyaAbs
    case 0x8419:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPDyaAbs
    case 0x841A:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPDxaWidth
    case 0x261B:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPPc
    case 0x461C:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPBrcTop10
    case 0x461D:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPBrcLeft10
    case 0x461E:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPBrcBottom10
    case 0x461F:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPBrcRight10
    case 0x4620:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPBrcBetween10
    case 0x4621:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPBrcBar10
    case 0x4622:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPDxaFromText10
    case 0x2423:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPWr

    case 0x6424:   // sprmPBrcTop
        /* WRITERFILTERSTATUS: done: 50, planned: 8, spent: 4 */
        /* WRITERFILTERSTATUS: comment: page borders are no handled yet, conversion incomplete */
    case 0x6425:   // sprmPBrcLeft
        /* WRITERFILTERSTATUS: done: 50, planned: 8, spent: 4 */
        /* WRITERFILTERSTATUS: comment: page borders are no handled yet, conversion incomplete */
    case 0x6426:   // sprmPBrcBottom
        /* WRITERFILTERSTATUS: done: 50, planned: 8, spent: 4 */
        /* WRITERFILTERSTATUS: comment: page borders are no handled yet, conversion incomplete */
    case 0x6427:   // sprmPBrcRight
        /* WRITERFILTERSTATUS: done: 50, planned: 8, spent: 4 */
        /* WRITERFILTERSTATUS: comment: page borders are no handled yet, conversion incomplete */
    case 0x6428:   // sprmPBrcBetween
        /* WRITERFILTERSTATUS: done: 0, planned: 8, spent: 0 */
        /* WRITERFILTERSTATUS: comment:  */
        {
            table::BorderLine aBorderLine;
            sal_Int32 nLineDistance = ConversionHelper::MakeBorderLine( nIntValue, aBorderLine );
            PropertyIds eBorderId = PROP_LEFT_BORDER;
            PropertyIds eBorderDistId = PROP_LEFT_BORDER_DISTANCE  ;
            switch( nSprmId )
            {
            case 0x6428:   // sprmPBrcBetween
                OSL_ASSERT("TODO: inner border is not handled");
                break;
            case 0x6425:   // sprmPBrcLeft
                eBorderId = PROP_LEFT_BORDER;
                eBorderDistId = PROP_LEFT_BORDER_DISTANCE  ;
                break;
            case 0x6427:   // sprmPBrcRight
                eBorderId = PROP_RIGHT_BORDER          ;
                eBorderDistId = PROP_RIGHT_BORDER_DISTANCE ;
                break;
            case 0x6424:   // sprmPBrcTop
                eBorderId = PROP_TOP_BORDER            ;
                eBorderDistId = PROP_TOP_BORDER_DISTANCE;
                break;
            case 0x6426:   // sprmPBrcBottom
            default:
                eBorderId = PROP_BOTTOM_BORDER         ;
                eBorderDistId = PROP_BOTTOM_BORDER_DISTANCE;
            }
            rContext->Insert(eBorderId, true, uno::makeAny( aBorderLine ));
            rContext->Insert(eBorderDistId, true, uno::makeAny( nLineDistance ));
        }
        break;
    case 0xc64e:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        /* WRITERFILTERSTATUS: comment: probably _real_ border colors, unhandled */
    case 0xc64f:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        /* WRITERFILTERSTATUS: comment: probably _real_ border colors, unhandled */
    case 0xc650:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        /* WRITERFILTERSTATUS: comment: probably _real_ border colors, unhandled */
    case 0xc651:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        /* WRITERFILTERSTATUS: comment: probably _real_ border colors, unhandled */
        OSL_ASSERT("TODO: border color definition");
        break;
    case 0x6629:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPBrcBar
    case 0x242A:   // sprmPFNoAutoHyph
        rContext->Insert(PROP_PARA_IS_HYPHENATION, true, uno::makeAny( nIntValue ? false : true ));
        break;
    case 0x442B:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPWHeightAbs
    case 0x442C:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPDcs

    case 0x442D: // sprmPShd
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        OSL_ASSERT("not handled");
        //contains fore color, back color and shadow percentage, results in a brush
        break;

    case 0x842E:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPDyaFromText
    case 0x842F:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPDxaFromText
    case 0x2430:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFLocked
    case 0x2431:
    {
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        uno::Any aVal( uno::makeAny( sal_Int32(nIntValue ? 2 : 0 )));
        rContext->Insert( PROP_PARA_WIDOWS, true, aVal );
        rContext->Insert( PROP_PARA_ORPHANS, true, aVal );
    }
    break;  // sprmPFWidowControl
    case 0xC632:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPRuler
    case 0x2433:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFKinsoku
    case 0x2434:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFWordWrap
    case 0x2435: ;  // sprmPFOverflowPunct - hanging punctuation
        rContext->Insert(PROP_PARA_IS_HANGING_PUNCTUATION, true, uno::makeAny( nIntValue ? false : true ));
        break;
    case 0x2436:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFTopLinePunct
    case 0x2437:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFAutoSpaceDE
    case 0x2438:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFAutoSpaceDN
    case 0x4439:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPWAlignFont
    case 0x443A:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFrameTextFlow
    case 0x243B:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPISnapBaseLine
    case 0xC63E:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPAnld
    case 0xC63F:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPPropRMark
    case 0x2640:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPOutLvl
    case 0x2441:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFBiDi
    case 0x2443:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFNumRMIns
    case 0x2444:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPCrLf
    case 0xC645:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPNumRM
    case 0x6645:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPHugePapx
    case 0x2447:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFUsePgsuSettings
    case 0x2448:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFAdjustRight
    case 0x0800:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFRMarkDel
    case 0x0801:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFRMark
    case 0x0802:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFFldVanish
    case 0x0855:   // sprmCFSpec
        break;
    case 0x6A03:   // sprmCPicLocation
            //is being resolved on the tokenizer side
        break;
    case 0x4804:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCIbstRMark
    case 0x6805:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCDttmRMark
    case 0x0806:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFData
    case 0x4807:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCIdslRMark
    case 0xEA08:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCChs
    case 0x6A09: // sprmCSymbol
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        resolveSprmProps(rSprm); //resolves LN_FONT and LN_CHAR
    break;
    case 0x080A:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFOle2
    case 0x480B:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCIdCharType
    case 0x2A0C:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        {
            sal_Int32 nColor = 0;
            if(true ==( mbIsHighlightSet = getColorFromIndex(nIntValue, nColor)))
                rContext->Insert(PROP_CHAR_BACK_COLOR, true, uno::makeAny( nColor ));
            else if (mnBackgroundColor)
                rContext->Insert(PROP_CHAR_BACK_COLOR, true, uno::makeAny( mnBackgroundColor ));
        }
        break;  // sprmCHighlight
    case 0x680E:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCObjLocation
    case 0x2A10:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFFtcAsciSymb
    case 0x4A30:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCIstd
    case 0xCA31:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCIstdPermute
    case 0x2A32:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCDefault
    case 0x2A33:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCPlain
    case 0x2A34:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        rContext->Insert(PROP_CHAR_EMPHASIS, true, uno::makeAny ( getEmphasisValue (nIntValue)));
        break;  // sprmCKcd
    case 0x0858:// sprmCFEmboss
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case 60:// sprmCFBold
    case 0x085C:// sprmCFBoldBi    (offset 0x27 to normal bold)
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case 0x085D:// sprmCFItalicBi  (offset 0x27 to normal italic)
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case 0x835: //sprmCFBold
    case 61: /*sprmCFItalic*/
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case 0x836: //sprmCFItalic
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case 0x837: //sprmCFStrike
        /* WRITERFILTERSTATUS: done: 100, planned: , spent:  0.5*/
    case 0x838: //sprmCFOutline
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case 0x839: //sprmCFShadow
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case 0x83a: //sprmCFSmallCaps
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case 0x83b: //sprmCFCaps
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case 0x83c: //sprmCFVanish
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case 0x2A53:   // sprmCFDStrike
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
        {
            PropertyIds ePropertyId = PROP_CHAR_WEIGHT; //initialized to prevent warning!
            switch( nSprmId )
            {
            case 060:// sprmCFBold
            case 0x085C: // sprmCFBoldBi
            case 0x835: /*sprmCFBold*/
                ePropertyId = nSprmId != 0x085C ? PROP_CHAR_WEIGHT : PROP_CHAR_WEIGHT_COMPLEX;
                break;
            case 61: /*sprmCFItalic*/
            case 0x085D: // sprmCFItalicBi
            case 0x836: /*sprmCFItalic*/
                ePropertyId = nSprmId == 0x836 ? PROP_CHAR_POSTURE : PROP_CHAR_POSTURE_COMPLEX;
                break;
            case 0x837: /*sprmCFStrike*/
            case 0x2A53 : /*sprmCFDStrike double strike through*/
                ePropertyId = PROP_CHAR_STRIKEOUT;
                break;
            case 0x838: /*sprmCFOutline*/
                ePropertyId = PROP_CHAR_CONTOURED;
                break;
            case 0x839: /*sprmCFShadow*/
                ePropertyId = PROP_CHAR_SHADOWED;
                break;
            case 0x83a: /*sprmCFSmallCaps*/
            case 0x83b: /*sprmCFCaps*/
                ePropertyId = PROP_CHAR_CASEMAP;
                break;
            case 0x83c: /*sprmCFVanish*/
                ePropertyId = PROP_CHAR_HIDDEN;
                break;
            case 0x0858: /*sprmCFEmboss*/
                ePropertyId = PROP_CHAR_RELIEF;
                break;
            }
            //expected: 0,1,128,129
            if(nIntValue != 128) //inherited from paragraph - ignore
            {
                if( nIntValue == 129) //inverted style sheet value
                {
                    //get value from style sheet and invert it
                    sal_Int16 nStyleValue = 0;
                    double fDoubleValue;
                    uno::Any aStyleVal = m_pImpl->GetPropertyFromStyleSheet(ePropertyId);
                    if( !aStyleVal.hasValue() )
                    {
                        nIntValue = 0x83a == nSprmId ?
                            4 : 1;
                    }
                    else if(aStyleVal.getValueTypeClass() == uno::TypeClass_FLOAT )
                    {
                        //only in case of awt::FontWeight
                        aStyleVal >>= fDoubleValue;
                        nIntValue = fDoubleValue  > 100. ?  0 : 1;
                    }
                    else if((aStyleVal >>= nStyleValue) ||
                            (nStyleValue = (sal_Int16)comphelper::getEnumAsINT32(aStyleVal)) >= 0 )
                    {
                        nIntValue = 0x83a == nSprmId ?
                            nStyleValue ? 0 : 4 :
                            nStyleValue ? 0 : 1;
                    }
                    else
                    {
                        OSL_ASSERT("what type was it");
                    }
                }

                switch( nSprmId )
                {
                    case 060:/*sprmCFBold*/
                    case 0x835: /*sprmCFBold*/
                    case 0x085C: // sprmCFBoldBi
                    {
                        uno::Any aBold( uno::makeAny( nIntValue ? awt::FontWeight::BOLD : awt::FontWeight::NORMAL ) );
                        rContext->Insert(ePropertyId, true, aBold );
                        if( nSprmId != 0x085c ) // sprmCFBoldBi
                            rContext->Insert(PROP_CHAR_WEIGHT_ASIAN, true, aBold );
                    }
                    break;
                    case 61: /*sprmCFItalic*/
                    case 0x836: /*sprmCFItalic*/
                    case 0x085D: // sprmCFItalicBi
                    {
                        uno::Any aPosture( uno::makeAny( nIntValue ? awt::FontSlant_ITALIC : awt::FontSlant_NONE ) );
                        rContext->Insert( ePropertyId, true, aPosture );
                        if( nSprmId != 0x085D ) // sprmCFItalicBi
                            rContext->Insert(PROP_CHAR_POSTURE_ASIAN, true, aPosture );
                    }
                    break;
                    case 0x837: /*sprmCFStrike*/
                        rContext->Insert(ePropertyId, true,
                                         uno::makeAny( nIntValue ? awt::FontStrikeout::SINGLE : awt::FontStrikeout::NONE ) );
                    break;
                    case 0x2A53 : /*sprmCFDStrike double strike through*/
                        rContext->Insert(ePropertyId, true,
                                         uno::makeAny( awt::FontStrikeout::DOUBLE ) );
                    break;
                    case 0x838: /*sprmCFOutline*/
                    case 0x839: /*sprmCFShadow*/
                    case 0x83c: /*sprmCFVanish*/
                        rContext->Insert(ePropertyId, true, uno::makeAny( nIntValue ? true : false ));
                    break;
                    case 0x83a: /*sprmCFSmallCaps*/
                        rContext->Insert(ePropertyId, true,
                                         uno::makeAny( nIntValue ? style::CaseMap::SMALLCAPS : style::CaseMap::NONE));
                    break;
                    case 0x83b: /*sprmCFCaps*/
                        rContext->Insert(ePropertyId, true,
                                         uno::makeAny( nIntValue ? style::CaseMap::UPPERCASE : style::CaseMap::NONE));
                    break;
                    case 0x0858: /*sprmCFEmboss*/
                        rContext->Insert(ePropertyId, true,
                                         uno::makeAny( nIntValue ? awt::FontRelief::EMBOSSED : awt::FontRelief::NONE ));
                    break;

                }
            }
        }
        break;
    case 0x4A3D:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFtcDefault
    case 0x2A3E: // sprmCKul
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        {
            // Parameter:  0 = none,    1 = single,  2 = by Word,
            // 3 = double,  4 = dotted,  5 = hidden
            // 6 = thick,   7 = dash,    8 = dot(not used)
            // 9 = dotdash 10 = dotdotdash 11 = wave
            handleUnderlineType(nIntValue, rContext);
        }
        break;
    case 0xEA3F:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCSizePos
    case 0x4A41:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCLid
    case 0x2A42:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        {
            sal_Int32 nColor = 0;
            if (getColorFromIndex(nIntValue, nColor))
                rContext->Insert(PROP_CHAR_COLOR, true, uno::makeAny( nColor ) );
        }
        break;  // sprmCIco
    case 0x4A61:    // sprmCHpsBi
    case 0x4A43:    // sprmCHps
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        {
            //multiples of half points (12pt == 24)
            double fVal = double(nIntValue) / 2.;
            uno::Any aVal = uno::makeAny( fVal );
            if( 0x4A61 == nSprmId )
                rContext->Insert( PROP_CHAR_HEIGHT_COMPLEX, true, aVal );
            else
            {
                //Asian get the same value as Western
                rContext->Insert( PROP_CHAR_HEIGHT, true, aVal );
                rContext->Insert( PROP_CHAR_HEIGHT_ASIAN, true, aVal );
            }
        }
        break;
    case 0x2A44:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCHpsInc
    case 0x4845:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        {
        // FIXME: ww8 filter in ww8par6.cxx has a Read_SubSuperProp function
        // that counts the escapement from this value and font size. So it will be
        // on our TODO list
            sal_Int16 nEscapement = 0;
            sal_Int8 nProp  = 100;
            if (nIntValue < 0)
                nEscapement = -58;
            else if (nIntValue > 0)
                nEscapement = 58;
            else /* (nIntValue == 0) */
                nProp = 0;
            rContext->Insert(PROP_CHAR_ESCAPEMENT,         true, uno::makeAny( nEscapement ) );
            rContext->Insert(PROP_CHAR_ESCAPEMENT_HEIGHT,  true, uno::makeAny( nProp ) );
        }
        break;  // sprmCHpsPos
    case 0x2A46:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCHpsPosAdj
    case 0xCA47:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCMajority
    case 0x2A48:   // sprmCIss
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        {
            //sub/super script 1: super, 2: sub, 0: normal
            sal_Int16 nEscapement = 0;
            sal_Int8 nProp  = 58;
            switch(nIntValue)
            {
            case 1: //super
                nEscapement = 101;
                break;
            case 2: //sub
                nEscapement = -101;
                break;
            case 0: nProp = 0;break; //none
            }
            rContext->Insert(PROP_CHAR_ESCAPEMENT,         true, uno::makeAny( nEscapement ) );
            rContext->Insert(PROP_CHAR_ESCAPEMENT_HEIGHT,  true, uno::makeAny( nProp ) );
        }
        break;
    case 0xCA49:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCHpsNew50
    case 0xCA4A:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCHpsInc1
    case 71 : //"sprmCDxaSpace"
    case 96 : //"sprmCDxaSpace"
    case 0x8840:  // sprmCDxaSpace
        /* WRITERFILTERSTATUS: done: 50, planned: 2, spent: 0 */
        //Kerning half point values
        //TODO: there are two kerning values -
        // in ww8par6.cxx 0x484b is used as boolean AutoKerning
        rContext->Insert(PROP_CHAR_CHAR_KERNING, true, uno::makeAny( sal_Int16(ConversionHelper::convertToMM100(sal_Int16(nIntValue))) ) );
        break;
    case 0x484B:  // sprmCHpsKern    auto kerning is bound to a minimum font size in Word - but not in Writer :-(
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        rContext->Insert(PROP_CHAR_AUTO_KERNING, true, uno::makeAny( true ) );
        break;
    case 0xCA4C:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCMajority50
    case 0x4A4D:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCHpsMul
    case 0x484E:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCYsri
    case 0x4A4F:  // sprmCRgFtc0     //ascii font index
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
    case 0x4A50:  // sprmCRgFtc1     //Asian font index
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
    case 0x4A51:  // sprmCRgFtc2     //CTL font index
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
    case 0x4A5E: // sprmCFtcBi      //font index of a CTL font
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        {
            FontTablePtr pFontTable = m_pImpl->GetFontTable();
            if(nIntValue >= 0 && pFontTable->size() > sal_uInt32(nIntValue))
            {
                PropertyIds eFontName    = PROP_CHAR_FONT_NAME;
                PropertyIds eFontStyle   = PROP_CHAR_FONT_STYLE;
                PropertyIds eFontFamily  = PROP_CHAR_FONT_FAMILY;
                PropertyIds eFontCharSet = PROP_CHAR_FONT_CHAR_SET;
                PropertyIds eFontPitch   = PROP_CHAR_FONT_PITCH;
                switch(nSprmId)
                {
                case 0x4A4F:
                    //already initialized
                    break;
                case 0x4A50:
                    eFontName =     PROP_CHAR_FONT_NAME_ASIAN;
                    eFontStyle =    PROP_CHAR_FONT_STYLE_ASIAN;
                    eFontFamily =   PROP_CHAR_FONT_FAMILY_ASIAN;
                    eFontCharSet =  PROP_CHAR_FONT_CHAR_SET_ASIAN;
                    eFontPitch =    PROP_CHAR_FONT_PITCH_ASIAN;
                    break;
                case 0x4A51:
                case 0x4A5E:
                    eFontName =     PROP_CHAR_FONT_NAME_COMPLEX;
                    eFontStyle =    PROP_CHAR_FONT_STYLE_COMPLEX;
                    eFontFamily =   PROP_CHAR_FONT_FAMILY_COMPLEX;
                    eFontCharSet =  PROP_CHAR_FONT_CHAR_SET_COMPLEX;
                    eFontPitch =    PROP_CHAR_FONT_PITCH_COMPLEX;
                    break;
                }
                const FontEntry* pFontEntry = pFontTable->getFontEntry(sal_uInt32(nIntValue));
                rContext->Insert(eFontName, true, uno::makeAny( pFontEntry->sFontName  ));
                //                rContext->Insert(eFontStyle, uno::makeAny( pFontEntry->  ));
                //                rContext->Insert(eFontFamily, uno::makeAny( pFontEntry->  ));
                rContext->Insert(eFontCharSet, true, uno::makeAny( (sal_Int16)pFontEntry->nTextEncoding  ));
                rContext->Insert(eFontPitch, true, uno::makeAny( pFontEntry->nPitchRequest  ));
            }
        }
        break;
    case 0x4852:  // sprmCCharScale
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        rContext->Insert(PROP_CHAR_SCALE_WIDTH, true,
                         uno::makeAny( sal_Int16(nIntValue) ));
        break;
    case 0x0854: // sprmCFImprint   1 or 0
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        // FontRelief: NONE, EMBOSSED, ENGRAVED
        rContext->Insert(PROP_CHAR_RELIEF, true,
                         uno::makeAny( nIntValue ? awt::FontRelief::ENGRAVED : awt::FontRelief::NONE ));
        break;
    case 0x0856:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFObj
    case 0xCA57:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCPropRMark
    case 0x2859:
        // The file-format has many character animations. We have only
        // one, so we use it always. Suboptimal solution though.
        if (nIntValue)
            rContext->Insert(PROP_CHAR_FLASH, true, uno::makeAny( true ));
        else
            rContext->Insert(PROP_CHAR_FLASH, true, uno::makeAny( false ));
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCSfxText
    case 0x085A:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFBiDi
    case 0x085B:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFDiacColor
    case 0x4A60:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCIcoBi
    case 0xCA62:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCDispFldRMark
    case 0x4863:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCIbstRMarkDel
    case 0x6864:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCDttmRMarkDel
    case 0x6865:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCBrc
    case 0x4866:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCShd
    case 0x4867:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCIdslRMarkDel
    case 0x0868:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFUsePgsuSettings
    case 0x486B:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCCpg
    case 0x485F:  // sprmCLidBi      language complex
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
    case 0x4873: //sprmCRgLid
        /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 1 */
        //undocumented but interpreted as western language
    case 0x486D:   // sprmCRgLid0    language Western
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
    case 0x486E:   // sprmCRgLid1    language Asian
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        {
            lang::Locale aLocale;
            MsLangId::convertLanguageToLocale( (LanguageType)nIntValue, aLocale );
            rContext->Insert(0x486D == nSprmId ? PROP_CHAR_LOCALE :
                             0x486E == nSprmId ? PROP_CHAR_LOCALE_ASIAN : PROP_CHAR_LOCALE_COMPLEX,
                             true,
                             uno::makeAny( aLocale ) );
        }
        break;

    case 0x286F:   // sprmCIdctHint
        //list table - text offset???
        break;
    case 0x2E00:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPicBrcl
    case 0xCE01:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPicScale
    case 0x6C02:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPicBrcTop
    case 0x6C03:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPicBrcLeft
    case 0x6C04:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPicBrcBoConversionHelper::convertToMM100ttom
    case 0x6C05:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPicBrcRight
    case 0x3000:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmScnsPgn
    case 0x3001:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetEvenlySpaced( nIntValue > 0 );

        break;  // sprmSiHeadingPgn
    case 0xD202:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSOlstAnm
    case 136:
    case 0xF203: // sprmSDxaColWidth
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        // contains the twip width of the column as 3-byte-code
        // the lowet byte contains the index
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->AppendColumnWidth( ConversionHelper::convertToMM100( (nIntValue & 0xffff00) >> 8 ));
        break;
    case 0xF204: // sprmSDxaColSpacing
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        // the lowet byte contains the index
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->AppendColumnSpacing( ConversionHelper::convertToMM100( (nIntValue & 0xffff00) >> 8 ));
        break;
    case 138:
    case 0x3005:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetEvenlySpaced( nIntValue > 0 );
        break;  // sprmSFEvenlySpaced
    case 0x3006: // sprmSFProtected
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        //todo: missing feature - unlocked sections in protected documents
        break;
    case 0x5007: // sprmSDmBinFirst
        /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetFirstPaperBin(nIntValue);
        break;
    case 0x5008: // sprmSDmBinOther
        /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetPaperBin( nIntValue );
        break;
    case 0x3009: // sprmSBkc
        /* WRITERFILTERSTATUS: done: 0.5, planned: 2, spent: 0 */
        /* break type
          0 - No break
          1 - New Colunn
          2 - New page
          3 - Even page
          4 - odd page
        */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetBreakType( nIntValue );
        break;
    case 143:
    case 0x300A: // sprmSFTitlePage
    {
        /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetTitlePage( nIntValue > 0 ? true : false );//section has title page
    }
    break;
    case 144:
    case 0x500B: // sprmSCcolumns
        /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
        //no of columns - 1
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetColumnCount( (sal_Int16) nIntValue );
    break;
    case 145:
    case 0x900C:           // sprmSDxaColumns
        /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
        //column distance - default 708 twip
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetColumnDistance( ConversionHelper::convertToMM100( nIntValue ) );
        break;
    case 0x300D:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSFAutoPgn
    case 147:
    case 0x300E: // sprmSNfcPgn
        /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
        //page numbering 0 - Arab, 1 - ROMAN, 2 - roman, 3 - ABC, 4 abc
        sal_Int16 nNumbering;
        switch( nIntValue )
        {
            case 1:  nNumbering = style::NumberingType::ROMAN_UPPER;
            case 2:  nNumbering = style::NumberingType::ROMAN_LOWER;
            case 3:  nNumbering = style::NumberingType::CHARS_UPPER_LETTER;
            case 4:  nNumbering = style::NumberingType::CHARS_LOWER_LETTER;
            case 0:
            default:
                    nNumbering = style::NumberingType::ARABIC;
        }
        rContext->Insert( PROP_NUMBERING_TYPE, false, uno::makeAny( nNumbering ) );
    break;
    case 0xB00F:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSDyaPgn
    case 0xB010:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSDxaPgn
    case 150:
    case 0x3011: // sprmSFPgnRestart
    {
        /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetPageNoRestart( nIntValue > 0 );
    }
    break;
    case 0x3012:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSFEndnote
    case 154:
    case 0x5015:// sprmSNLnnMod
        /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if( pSectionContext )
            pSectionContext->SetLnnMod( nIntValue );
    break;
    case 155:
    case 0x9016: // sprmSDxaLnn
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if( pSectionContext )
            pSectionContext->SetdxaLnn( nIntValue );
    break;
    case 152:
    case 0x3013:// sprmSLnc
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if( pSectionContext )
            pSectionContext->SetLnc( nIntValue );
    break;
    case 160:
    case 0x501B: // sprmSLnnMin
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if( pSectionContext )
            pSectionContext->SetLnnMin( nIntValue );
    break;

    case 0x3014:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        //flags about header/footer sharing and footnotes?
        /* ww8scan.hxx:
         * WW8_HEADER_EVEN = 0x01, WW8_HEADER_ODD = 0x02, WW8_FOOTER_EVEN = 0x04,
         * WW8_FOOTER_ODD = 0x08, WW8_HEADER_FIRST = 0x10, WW8_FOOTER_FIRST = 0x20
         */

//        if(pSectionContext)
    break;  // sprmSGprfIhdt
    case 0xB017: // sprmSDyaHdrTop
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        // default 720 twip
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetHeaderTop( ConversionHelper::convertToMM100( nIntValue ));
    break;
    case 0xB018: // sprmSDyaHdrBottom
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        // default 720 twip
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetHeaderBottom( ConversionHelper::convertToMM100( nIntValue ) );
    break;
    case 158:
    case 0x3019: // sprmSLBetween
        /* WRITERFILTERSTATUS: done: 1, planned: 0.5, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetSeparatorLine( nIntValue > 0 );
    break;
    case 0x301A:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        break;  // sprmSVjc
    case 161:
    case 0x501C: // sprmSPgnStart
        /* WRITERFILTERSTATUS: done: 1, planned: 0.5, spent: 0 */
        //page number
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetPageNumber( nIntValue );
    break;
    case 162:
    case 0x301D:
        /* WRITERFILTERSTATUS: done: 1, planned: 0.5, spent: 0 */
        //todo: the old filter assumed that a value of 2 points to double-pages layout
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetLandscape( nIntValue > 0 );
        rContext->Insert( PROP_IS_LANDSCAPE , false, uno::makeAny( nIntValue > 0 ));
    break;  // sprmSBOrientation
    case 0x301E:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        break;  // sprmSBCustomize
    case 165:
    case 0xB020: // sprmSYaPage
    {
        /* WRITERFILTERSTATUS: done: 1, planned: 0.5, spent: 0 */
        //page height, rounded to default values, default: 0x3dc0 twip
        sal_Int32 nHeight = ConversionHelper::SnapPageDimension( nIntValue );
        rContext->Insert( PROP_HEIGHT, false, uno::makeAny( ConversionHelper::convertToMM100( nHeight ) ) );
    }
    break;
    case 0xB01F:   // sprmSXaPage
    {
        /* WRITERFILTERSTATUS: done: 1, planned: 0.5, spent: 0 */
        //page width, rounded to default values, default 0x2fd0 twip
        sal_Int32 nWidth = ConversionHelper::SnapPageDimension( nIntValue );
        rContext->Insert( PROP_WIDTH, false, uno::makeAny( ConversionHelper::convertToMM100( nWidth ) ) );
    }
    break;
    case 166:
    case 0xB021:  // sprmSDxaLeft
    {
        /* WRITERFILTERSTATUS: done: 1, planned: 0.5, spepSectionContext->SetSeparatorLine( nIntValue > 0 )nt: 0 */
        //left page margin default 0x708 twip
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        sal_Int32 nConverted = ConversionHelper::convertToMM100( nIntValue );
        if(pSectionContext)
            pSectionContext->SetLeftMargin( nConverted );
        rContext->Insert( PROP_LEFT_MARGIN, false, uno::makeAny( nConverted ));
    }
    break;
    case 167:
    case 0xB022: // sprmSDxaRight
    {
        /* WRITERFILTERSTATUS: done: 1, planned: 0.5, spent: 0 */
        //right page margin default 0x708 twip
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        sal_Int32 nConverted = ConversionHelper::convertToMM100( nIntValue );
        if(pSectionContext)
            pSectionContext->SetRightMargin( nConverted );
        rContext->Insert( PROP_RIGHT_MARGIN, false, uno::makeAny( nConverted ));
    }
    break;
    case 168:
    case 0x9023: // sprmSDyaTop
    {
        /* WRITERFILTERSTATUS: done: 1, planned: 0.5, spent: 0 */
        //top page margin default 1440 twip
        //todo: check cast of SVBT16
        sal_Int32 nConverted = ConversionHelper::convertToMM100( static_cast< sal_Int16 >( nIntValue ) );
        rContext->Insert( PROP_TOP_MARGIN, false, uno::makeAny( nConverted ) );
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetTopMargin( nConverted );
    }
    break;
    case 169:
    case 0x9024: // sprmSDyaBottom
    {
        /* WRITERFILTERSTATUS: done: 1, planned: 0.5, spent: 0 */
        //bottom page margin default 1440 twip
        //todo: check cast of SVBT16
        sal_Int32 nConverted = ConversionHelper::convertToMM100( static_cast< sal_Int16 >( nIntValue ) );
        rContext->Insert( PROP_BOTTOM_MARGIN, false, uno::makeAny( nConverted) );
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetBottomMargin( nConverted );
    }
    break;
    case 170:
    case 0xB025:   // sprmSDzaGutter
    {
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        // gutter is added to one of the margins of a section depending on RTL, can be placed on top either
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
        {
            pSectionContext->SetDzaGutter( ConversionHelper::convertToMM100( nIntValue  ) );
        }
    }
    break;
    case 0x5026:   // sprmSDmPaperReq
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        //paper code - no handled in old filter
        break;
    case 0xD227:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSPropRMark
    case 0x3228:// sprmSFBiDi
    {
        /* WRITERFILTERSTATUS: done: 0.5, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetSFBiDi( nIntValue > 0 );
    }
    break;
    case 0x3229:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSFFacingCol
    case 0x322A: // sprmSFRTLGutter
    {
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetGutterRTL( nIntValue > 0 );
    }
    break;
    case 0x702B:   // sprmSBrcTop
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
    case 0x702C:   // sprmSBrcLeft
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
    case 0x702D:  // sprmSBrcBottom
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
    case 0x702E:  // sprmSBrcRight
        /* WRITERFILTERSTATUS: Sectiondone: 100, planned: 0.5, spent: 0 */
        {
            table::BorderLine aBorderLine;
            sal_Int32 nLineDistance = ConversionHelper::MakeBorderLine( nIntValue, aBorderLine );
            OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
            if(pSectionContext)
            {
                static const BorderPosition aPositions[4] =
                    {
                        BORDER_TOP,
                        BORDER_LEFT,
                        BORDER_BOTTOM,
                        BORDER_RIGHT
                    };
                pSectionContext->SetBorder( aPositions[nSprmId - 0x702B], nLineDistance, aBorderLine );
            }
        }
        break;

    case 0x522F:  // sprmSPgbProp
        {
            OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
            if(pSectionContext)
            {
                pSectionContext->ApplyBorderToPageStyles( m_pImpl->GetPageStyles(), m_pImpl->GetTextFactory(), nIntValue );
            }
        }
        break;
    case 0x7030:
    {
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
        {
            pSectionContext->SetDxtCharSpace( nIntValue );
        }
    }
    break;  // sprmSDxtCharSpace
    case 0x9031:   // sprmSDyaLinePitch
    {
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        //see SwWW8ImplReader::SetDocumentGrid
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
        {
            pSectionContext->SetGridLinePitch( ConversionHelper::convertToMM100( nIntValue ) );
        }
    }
    break;
    case 0x703a: //undocumented, grid related?
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        OSL_ASSERT("TODO: not handled yet"); //nIntValue like 0x008a2373 ?
        break;
    case 0x5032:
    {
        /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
        sal_Int16 nGridType = text::TextGridMode::NONE;
        switch( nIntValue )
        {
            case 0:
                nGridType = text::TextGridMode::NONE;
            break;
            case 3:
                //Text snaps to char grid, this doesn't make a lot of sense to
                //me. This is closer than LINES_CHARS
                nGridType = text::TextGridMode::LINES;
            break;
            case 1:
                nGridType = text::TextGridMode::LINES_AND_CHARS;
            break;
            case 2:
                nGridType = text::TextGridMode::LINES;
            break;
            default:;
        }
        rContext->Insert( PROP_GRID_MODE, false, uno::makeAny( nGridType ) );

    //Seems to force this behaviour in word ?
    if(nGridType != text::TextGridMode::NONE)
        m_pImpl->SetDocumentSettingsProperty(
            PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_ADD_EXTERNAL_LEADING ),
            uno::makeAny( true ) );
    }
    break;  // sprmSClm
    case 0x5033:
    {
        /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
        /* 0 HoriLR 1 Vert TR 2 Vert TR 3 Vert TT 4 HoriLT
            only 0 and 1 can be imported correctly
          */
        sal_Int16 nDirection = text::WritingMode_LR_TB;
        switch( nIntValue )
        {
            case 0:
            case 4:
                nDirection = text::WritingMode_LR_TB;
            break;
            case 1:
            case 2:
            case 3:
                nDirection = text::WritingMode_TB_RL;
            break;
            default:;
        }
        rContext->Insert(PROP_WRITING_MODE, false, uno::makeAny( nDirection ) );
    }
    break;  // sprmSTextFlow
    case 0x5400: // sprmTJc
    case 0x9601:
    case 0x9602:
    case 0x3403:
    case 0x3404:
    case 0xD605: // sprmTTableBorders
    {
        OSL_ASSERT("table propeties should be handled by the table manager");
    }
    break;
    case 0xD606:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTDefTable10
    case 0x9407:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTDyaRowHeight
    case 0xD608:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTDefTable
    case 0xD609:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTDefTableShd
    case 0x740A:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTTlp
    case 0x560B:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTFBiDi
    case 0x740C:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTHTMLProps
    case 0xD620:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTSetBrc
    case 0x7621:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTInsert
    case 0x5622:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTDelete
    case 0x7623:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTDxaCol
    case 0x5624:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTMerge
    case 0x5625:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTSplit
    case 0xD626:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTSetBrc10
    case 164: // sprmTSetShd
    case 0x7627:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTSetShd
    case 0x7628:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTSetShdOdd
    case 0x7629:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTTextFlow
    case 0xD62A:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTDiagLine
    case 0xD62B:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTVertMerge
    case 0xD62C:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTVertAlign
        // the following are not part of the official documentation
    case 0x6870: //TxtForeColor
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        {
            //contains a color as 0xTTRRGGBB while SO uses 0xTTRRGGBB
            sal_Int32 nColor = ConversionHelper::ConvertColor(nIntValue);
            rContext->Insert(PROP_CHAR_COLOR, true, uno::makeAny( nColor ) );
        }
        break;
    case 0x4874:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break; //seems to be a language id for Asian text - undocumented
    case 0x6877: //underlining color
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        {
            sal_Int32 nColor = ConversionHelper::ConvertColor(nIntValue);
            rContext->Insert(PROP_CHAR_UNDERLINE_HAS_COLOR, true, uno::makeAny( true ) );
            rContext->Insert(PROP_CHAR_UNDERLINE_COLOR, true, uno::makeAny( nColor ) );
        }
        break;
    case 0x6815:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break; //undocumented
    case 0x6816:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break; //undocumented
    case 0x6467:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break; //undocumented
    case 0xF617:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break; //undocumented
    case 0xd634: // sprmTNewSpacing - table spacing ( see WW8TabBandDesc::ProcessSpacing() )
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;
    case 0xf661:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break; //undocumented
    case 0x4888:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case 0x6887:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        //properties of list levels - undocumented
        break;
    case 0xd234:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case 0xd235:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case 0xd236:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case 0xd237:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        break;//undocumented section properties
    case 0xca78:
        resolveSprmProps(rSprm);
        break;
    case NS_ooxml::LN_CT_Tabs_tab:
        resolveSprmProps(rSprm);
        m_pImpl->IncorporateTabStop(m_pImpl->m_aCurrentTabStop);
        break;

    case NS_ooxml::LN_CT_PPr_sectPr:
    case NS_ooxml::LN_EG_RPrBase_color:
    case NS_ooxml::LN_EG_RPrBase_rFonts:
    case NS_ooxml::LN_EG_RPrBase_bdr:
    case NS_ooxml::LN_EG_RPrBase_eastAsianLayout:
    case NS_ooxml::LN_EG_RPrBase_u:
    case NS_ooxml::LN_EG_RPrBase_lang:
    case NS_ooxml::LN_CT_PPrBase_spacing:
    case NS_ooxml::LN_CT_PPrBase_ind:
    case NS_ooxml::LN_CT_PPrBase_tabs:
    case NS_ooxml::LN_EG_SectPrContents_footnotePr:
    case NS_ooxml::LN_EG_SectPrContents_endnotePr:
    case NS_ooxml::LN_CT_RPrDefault_rPr:
    case NS_ooxml::LN_CT_PPrDefault_pPr:
    case NS_ooxml::LN_CT_DocDefaults_pPrDefault:
    case NS_ooxml::LN_CT_DocDefaults_rPrDefault:
    case NS_ooxml::LN_CT_Style_pPr:
    case NS_ooxml::LN_CT_Style_rPr:
//    case NS_ooxml::LN_CT_PPr_rPr:
        resolveSprmProps(rSprm);
        break;

    case NS_ooxml::LN_EG_SectPrContents_pgSz:
        CT_PageSz.code = 0;
        CT_PageSz.h = ConversionHelper::convertToMM100( ConversionHelper::SnapPageDimension( sal_Int32(15840) ));
        CT_PageSz.w = ConversionHelper::convertToMM100( ConversionHelper::SnapPageDimension( sal_Int32(12240) ));
        CT_PageSz.orient = false;
        resolveSprmProps(rSprm);
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
        {
            pSectionContext->Insert( PROP_HEIGHT, false, uno::makeAny( CT_PageSz.h ) );
            pSectionContext->Insert( PROP_IS_LANDSCAPE, false, uno::makeAny( CT_PageSz.orient ));
            pSectionContext->Insert( PROP_WIDTH, false, uno::makeAny( CT_PageSz.w ) );
            pSectionContext->SetLandscape( CT_PageSz.orient );
        }
        break;

    case NS_ooxml::LN_EG_SectPrContents_pgMar:
        CT_PageMar.header = CT_PageMar.footer = CT_PageMar.top = CT_PageMar.bottom =
            ConversionHelper::convertToMM100( sal_Int32(1440));
        CT_PageMar.right = CT_PageMar.left = ConversionHelper::convertToMM100( sal_Int32(1800));
        CT_PageMar.gutter = 0;
        resolveSprmProps(rSprm);
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
        {
            pSectionContext->SetTopMargin( CT_PageMar.top );
            pSectionContext->SetRightMargin( CT_PageMar.right );
            pSectionContext->SetBottomMargin( CT_PageMar.bottom );
            pSectionContext->SetLeftMargin( CT_PageMar.left );
            pSectionContext->SetHeaderTop( CT_PageMar.header );
            pSectionContext->SetHeaderBottom( CT_PageMar.footer );
        }
        break;

    case NS_ooxml::LN_EG_SectPrContents_cols:
        CT_Columns.equalWidth = CT_Columns.sep = false;
        CT_Columns.space = CT_Columns.num = 0;
        CT_Columns.cols.erase(CT_Columns.cols.begin(), CT_Columns.cols.end());
        resolveSprmProps(rSprm);
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
        {
            if (CT_Columns.equalWidth)
            {
                pSectionContext->SetEvenlySpaced( true );
                pSectionContext->SetColumnCount( (sal_Int16) (CT_Columns.num - 1) );
                pSectionContext->SetColumnDistance( CT_Columns.space );
                pSectionContext->SetSeparatorLine( CT_Columns.sep );
            }
            else if (!CT_Columns.cols.empty())
            {
                pSectionContext->SetEvenlySpaced( false );
                pSectionContext->SetColumnDistance( CT_Columns.space );
                pSectionContext->SetColumnCount( (sal_Int16)(CT_Columns.cols.size() -1));
                std::vector<_Column>::const_iterator tmpIter = CT_Columns.cols.begin();
                for (; tmpIter != CT_Columns.cols.end(); tmpIter++)
                {
                    pSectionContext->AppendColumnWidth( tmpIter->w );
                    if ((tmpIter != CT_Columns.cols.end() - 1) || (tmpIter->space > 0))
                        pSectionContext->AppendColumnSpacing( tmpIter->space );
                }
                pSectionContext->SetSeparatorLine( CT_Columns.sep );
            }
        }

        break;

    case NS_ooxml::LN_CT_Columns_col:
        CT_Column.w = CT_Column.space = 0;
        resolveSprmProps(rSprm);
        CT_Columns.cols.push_back(CT_Column);
        break;

    default:
        {
            OSL_ASSERT("DomainMapper::sprm()"); //
            //doctok::Value::Pointer_t pValue_ = rSprm.getValue();
        }
    }
}
/*-- 09.06.2006 09:52:13---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::entry(int /*pos*/,
                         doctok::Reference<Properties>::Pointer_t ref)
{
    ref->resolve(*this);
}
/*-- 09.06.2006 09:52:13---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::data(const sal_uInt8* /*buf*/, size_t /*len*/,
                        doctok::Reference<Properties>::Pointer_t /*ref*/)
{
}
/*-- 09.06.2006 09:52:13---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::startSectionGroup()
{
    m_pImpl->PushProperties(CONTEXT_SECTION);
}
/*-- 09.06.2006 09:52:13---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::endSectionGroup()
{
    PropertyMapPtr pContext = m_pImpl->GetTopContextOfType(CONTEXT_SECTION);
    SectionPropertyMap* pSectionContext = dynamic_cast< SectionPropertyMap* >( pContext.get() );
    OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
    if(pSectionContext)
        pSectionContext->CloseSectionGroup( *m_pImpl );
    m_pImpl->PopProperties(CONTEXT_SECTION);
}
/*-- 09.06.2006 09:52:13---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::startParagraphGroup()
{
    m_pImpl->getTableManager().startParagraphGroup();
    m_pImpl->PushProperties(CONTEXT_PARAGRAPH);
}
/*-- 09.06.2006 09:52:14---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::endParagraphGroup()
{
    m_pImpl->PopProperties(CONTEXT_PARAGRAPH);
    m_pImpl->getTableManager().endParagraphGroup();
}

/*-- 13.06.2007 16:15:55---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper::PushStyleSheetProperties( PropertyMapPtr pStyleProperties )
{
    m_pImpl->PushStyleProperties( pStyleProperties );
}
/*-- 13.06.2007 16:15:55---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper::PopStyleSheetProperties()
{
    m_pImpl->PopProperties( CONTEXT_STYLESHEET );
}

/*-- 09.06.2006 09:52:14---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::startCharacterGroup()
{
    m_pImpl->PushProperties(CONTEXT_CHARACTER);
    DomainMapperTableManager& rTableManager = m_pImpl->getTableManager();
    if( rTableManager.getTableStyleName().getLength() )
    {
        PropertyMapPtr pTopContext = m_pImpl->GetTopContext();
        const StyleSheetEntry* pStyleSheetEntry = m_pImpl->GetStyleSheetTable()->FindStyleSheetByISTD(
                                                        rTableManager.getTableStyleName());
        OSL_ENSURE( pStyleSheetEntry, "table style not found" );
        PropertyMap::const_iterator aPropIter = pStyleSheetEntry->pProperties->begin();
        while(aPropIter != pStyleSheetEntry->pProperties->end())
        {
            //copy all text properties form the table style to the current run attributes
            if( aPropIter->first.bIsTextProperty )
                pTopContext->insert(*aPropIter);
            ++aPropIter;
        }
    }
}
/*-- 09.06.2006 09:52:14---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::endCharacterGroup()
{
    m_pImpl->PopProperties(CONTEXT_CHARACTER);
}
/*-- 09.06.2006 09:52:14---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::text(const sal_uInt8 * data_, size_t len)
{
    //TODO: Determine the right text encoding (FIB?)
    ::rtl::OUString sText( (const sal_Char*) data_, len, RTL_TEXTENCODING_MS_1252 );
    try
    {
        bool bContinue = true;
        if(len == 1)
        {
            bContinue = false;
            switch(*data_)
            {
                case 0x02: break; //footnote character
                case 0x0c: break; //page break
                case 0x07:
                    m_pImpl->getTableManager().text(data_, len);
                case 0x0d:
                    m_pImpl->finishParagraph(m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH)); break;
                case 0x13: m_pImpl->PushFieldContext();break;
                case 0x14:
                    // delimiter not necessarily available
                    // appears only if field contains further content
                    m_pImpl->CloseFieldCommand();
                break;
                case 0x15: /* end of field */
                    m_pImpl->PopFieldContext();
                break;
            default: bContinue = true;
            }
        }
        if(bContinue)
        {
            PropertyMapPtr pContext = m_pImpl->GetTopContext();
            if( pContext->GetFootnote().is() )
            {
                if( !pContext->GetFootnoteSymbol() )
                    pContext->GetFootnote()->setLabel( sText );
                //otherwise ignore sText
            }
            else if( m_pImpl->IsOpenFieldCommand() )
                m_pImpl->AppendFieldCommand(sText);
            else if( m_pImpl->IsOpenField() && m_pImpl->IsFieldResultAsString())
                /*depending on the success of the field insert operation this result will be
                  set at the field or directly inserted into the text*/
                m_pImpl->SetFieldResult( sText );
            else
            {
                //--> debug
                //sal_uInt32 nSize = pContext->size();
                //<--

                m_pImpl->appendTextPortion( sText, pContext );
            }
        }
    }
    catch( const uno::RuntimeException& )
    {
        std::clog << __FILE__ << "(l" << __LINE__ << ")" << std::endl;
    }
}
/*-- 09.06.2006 09:52:15---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::utext(const sal_uInt8 * data_, size_t len)
{
    OUString sText;
    OUStringBuffer aBuffer = OUStringBuffer(len);
    aBuffer.append( (const sal_Unicode *) data_, len);
    sText = aBuffer.makeStringAndClear();
    try
    {
        m_pImpl->getTableManager().utext(data_, len);

        if(len == 1 && ((*data_) == 0x0d || (*data_) == 0x07))
            m_pImpl->finishParagraph(m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH));
        else
        {

            PropertyMapPtr pContext = m_pImpl->GetTopContext();

            //-->debug
            uno::Reference<text::XFootnote> xTest = pContext->GetFootnote();
            //<--debug
            if( xTest.is() )
//            if( pContext->GetFootnote().is() )
            {
                if( !pContext->GetFootnoteSymbol() )
                    pContext->GetFootnote()->setLabel( sText );
                //otherwise ignore sText
            }
            else
                m_pImpl->appendTextPortion( sText, pContext );
        }
    }
    catch( const uno::RuntimeException& )
    {
    }
}
/*-- 09.06.2006 09:52:15---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::props(doctok::Reference<Properties>::Pointer_t ref)
{
    string sType = ref->getType();
    if( sType == "PICF" )
    {
        m_pImpl->ImportGraphic(ref, false);
    }
    else if( sType == "FSPA" )
    {
        m_pImpl->ImportGraphic(ref, true);
    }
    else
        ref->resolve(*this);
}
/*-- 09.06.2006 09:52:15---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::table(doctok::Id name, doctok::Reference<Table>::Pointer_t ref)
{
    /* WRITERFILTERSTATUS: table: attributedata */
    switch(name)
    {
    case NS_rtf::LN_FONTTABLE:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        // create a font table object that listens to the attributes
        // each entry call inserts a new font entry
        ref->resolve( *m_pImpl->GetFontTable() );
        break;
    case NS_rtf::LN_STYLESHEET:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        //same as above to import style sheets
        m_pImpl->SetStyleSheetImport( true );
        ref->resolve( *m_pImpl->GetStyleSheetTable() );
        m_pImpl->GetStyleSheetTable()->ApplyStyleSheets(m_pImpl->GetTextDocument(), m_pImpl->GetFontTable());
        m_pImpl->SetStyleSheetImport( false );
        break;
    case NS_rtf::LN_LISTTABLE:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        //the same for list tables
        ref->resolve( *m_pImpl->GetListTable() );
        break;
    case NS_rtf::LN_LFOTABLE:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        ref->resolve( *m_pImpl->GetLFOTable() );
        break;
    default:
        OSL_ASSERT("which table is to be filled here?");
    }
}
/*-- 09.06.2006 09:52:16---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::substream(doctok::Id rName, ::doctok::Reference<Stream>::Pointer_t ref)
{
    m_pImpl->getTableManager().startLevel();

    //->debug
    //string sName = (*doctok::QNameToString::Instance())(rName);
    //--<debug
    //import of page header/footer

    /* WRITERFILTERSTATUS: table: attributedata */
    switch( rName )
    {
    case NS_rtf::LN_headerl:
        /* WRITERFILTERSTATUS: done: 50, planned: 2, spent: 0 */

        m_pImpl->PushPageHeader(SectionPropertyMap::PAGE_LEFT);
        break;
    case NS_rtf::LN_headerr:
        /* WRITERFILTERSTATUS: done: 50, planned: 2, spent: 0 */

        m_pImpl->PushPageHeader(SectionPropertyMap::PAGE_RIGHT);
        break;
    case NS_rtf::LN_headerf:
        /* WRITERFILTERSTATUS: done: 50, planned: 2, spent: 0 */

        m_pImpl->PushPageHeader(SectionPropertyMap::PAGE_FIRST);
        break;
    case NS_rtf::LN_footerl:
        /* WRITERFILTERSTATUS: done: 50, planned: 2, spent: 0 */

        m_pImpl->PushPageFooter(SectionPropertyMap::PAGE_LEFT);
        break;
    case NS_rtf::LN_footerr:
        /* WRITERFILTERSTATUS: done: 50, planned: 2, spent: 0 */

        m_pImpl->PushPageFooter(SectionPropertyMap::PAGE_RIGHT);
        break;
    case NS_rtf::LN_footerf:
        /* WRITERFILTERSTATUS: done: 50, planned: 2, spent: 0 */

        m_pImpl->PushPageFooter(SectionPropertyMap::PAGE_FIRST);
        break;
    case NS_rtf::LN_footnote:
    case NS_rtf::LN_endnote:
        m_pImpl->PushFootOrEndnote( NS_rtf::LN_footnote == rName );
    break;
    }
    ref->resolve(*this);
    switch( rName )
    {
    case NS_rtf::LN_headerl:
    case NS_rtf::LN_headerr:
    case NS_rtf::LN_headerf:
    case NS_rtf::LN_footerl:
    case NS_rtf::LN_footerr:
    case NS_rtf::LN_footerf:
        m_pImpl->PopPageHeaderFooter();
    break;
    case NS_rtf::LN_footnote:
    case NS_rtf::LN_endnote:
        m_pImpl->PopFootOrEndnote();
    break;
    }

    m_pImpl->getTableManager().endLevel();
}
/*-- 09.06.2006 09:52:16---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::info(const string & /*info_*/)
{
}

void DomainMapper::handleUnderlineType(const sal_Int32 nIntValue, const ::boost::shared_ptr<PropertyMap> pContext)
{
    sal_Int16 eUnderline = awt::FontUnderline::NONE;

    switch(nIntValue)
    {
    case 0: eUnderline = awt::FontUnderline::NONE; break;
    case 2: pContext->Insert(PROP_CHAR_WORD_MODE, true, uno::makeAny( true ) ); // TODO: how to get rid of it?
    case 1: eUnderline = awt::FontUnderline::SINGLE;       break;
    case 3: eUnderline = awt::FontUnderline::DOUBLE;       break;
    case 4: eUnderline = awt::FontUnderline::DOTTED;       break;
    case 7: eUnderline = awt::FontUnderline::DASH;         break;
    case 9: eUnderline = awt::FontUnderline::DASHDOT;      break;
    case 10:eUnderline = awt::FontUnderline::DASHDOTDOT;   break;
    case 6: eUnderline = awt::FontUnderline::BOLD;         break;
    case 11:eUnderline = awt::FontUnderline::WAVE;         break;
    case 20:eUnderline = awt::FontUnderline::BOLDDOTTED;   break;
    case 23:eUnderline = awt::FontUnderline::BOLDDASH;     break;
    case 39:eUnderline = awt::FontUnderline::LONGDASH;     break;
    case 55:eUnderline = awt::FontUnderline::BOLDLONGDASH; break;
    case 25:eUnderline = awt::FontUnderline::BOLDDASHDOT;  break;
    case 26:eUnderline = awt::FontUnderline::BOLDDASHDOTDOT;break;
    case 27:eUnderline = awt::FontUnderline::BOLDWAVE;     break;
    case 43:eUnderline = awt::FontUnderline::DOUBLEWAVE;   break;
    default: ;
    }
    pContext->Insert(PROP_CHAR_UNDERLINE, true, uno::makeAny( eUnderline ) );
}

void DomainMapper::handleParaJustification(const sal_Int32 nIntValue, const ::boost::shared_ptr<PropertyMap> pContext, const bool bExchangeLeftRight)
{
    sal_Int16 nAdjust = 0;
    sal_Int16 nLastLineAdjust = 0;
    switch(nIntValue)
    {
    case 1:
        nAdjust = style::ParagraphAdjust_CENTER;
        break;
    case 2:
        nAdjust = static_cast< sal_Int16 > (bExchangeLeftRight ? style::ParagraphAdjust_LEFT : style::ParagraphAdjust_RIGHT);
        break;
    case 4:
        nLastLineAdjust = style::ParagraphAdjust_BLOCK;
        //no break;
    case 3:
        nAdjust = style::ParagraphAdjust_BLOCK;
        break;
    case 0:
    default:
        nAdjust = static_cast< sal_Int16 > (bExchangeLeftRight ? style::ParagraphAdjust_RIGHT : style::ParagraphAdjust_LEFT);
        break;
    }
    pContext->Insert( PROP_PARA_ADJUST, true, uno::makeAny( nAdjust ) );
    pContext->Insert( PROP_PARA_LAST_LINE_ADJUST, true, uno::makeAny( nLastLineAdjust ) );
}

bool DomainMapper::getColorFromIndex(const sal_Int32 nIndex, sal_Int32 &nColor)
{
    nColor = 0;
    if ((nIndex < 1) || (nIndex > 16))
        return false;

    switch (nIndex)
    {
    case 1: nColor=0x000000; break; //black
    case 2: nColor=0x0000ff; break; //blue
    case 3: nColor=0x00ffff; break; //cyan
    case 4: nColor=0x00ff00; break; //green
    case 5: nColor=0xff00ff; break; //magenta
    case 6: nColor=0xff0000; break; //red
    case 7: nColor=0xffff00; break; //yellow
    case 8: nColor=0xffffff; break; //white
    case 9: nColor=0x000080;  break;//dark blue
    case 10: nColor=0x008080; break; //dark cyan
    case 11: nColor=0x008000; break; //dark green
    case 12: nColor=0x800080; break; //dark magenta
    case 13: nColor=0x800000; break; //dark red
    case 14: nColor=0x808000; break; //dark yellow
    case 15: nColor=0x808080; break; //dark gray
    case 16: nColor=0xC0C0C0; break; //light gray
    default:
        return false;
    }
    return true;
}

sal_Int16 DomainMapper::getEmphasisValue(const sal_Int32 nIntValue)
{
    switch (nIntValue)
    {
    case 1:
        return com::sun::star::text::FontEmphasis::DOT_ABOVE;
    case 2:
        return com::sun::star::text::FontEmphasis::ACCENT_ABOVE;
    case 3:
        return com::sun::star::text::FontEmphasis::CIRCLE_ABOVE;
    case 4:
        return com::sun::star::text::FontEmphasis::DOT_BELOW;
    default:
        return com::sun::star::text::FontEmphasis::NONE;
    }
}

rtl::OUString DomainMapper::getBracketStringFromEnum(const sal_Int32 nIntValue, const bool bIsPrefix)
{
    switch(nIntValue)
    {
    case 1:
        if (bIsPrefix)
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "(" ));
        return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( ")" ));

    case 2:
        if (bIsPrefix)
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "[" ));
        return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "]" ));

    case 3:
        if (bIsPrefix)
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "<" ));
        return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( ">" ));

    case 4:
        if (bIsPrefix)
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "{" ));
        return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "}" ));

    case 0:
    default:
        return rtl::OUString();
    }
}

void DomainMapper::resolveSprmProps(doctok::Sprm & rSprm)
{
    doctok::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
    if( pProperties.get())
        pProperties->resolve(*this);
}

void DomainMapper::resolveAttributeProperties(doctok::Value & val)
{
    doctok::Reference<Properties>::Pointer_t pProperties = val.getProperties();
    if( pProperties.get())
        pProperties->resolve(*this);
}


com::sun::star::style::TabAlign DomainMapper::getTabAlignFromValue(const sal_Int32 nIntValue)
{
    switch (nIntValue)
    {
    case 0:
    case 4: // bar not supported
        return com::sun::star::style::TabAlign_LEFT;
    case 1:
        return com::sun::star::style::TabAlign_CENTER;
    case 2:
        return com::sun::star::style::TabAlign_RIGHT;
    case 3:
        return com::sun::star::style::TabAlign_DECIMAL;
    default:
        return com::sun::star::style::TabAlign_DEFAULT;
    }
    return com::sun::star::style::TabAlign_LEFT;
}

sal_Unicode DomainMapper::getFillCharFromValue(const sal_Int32 nIntValue)
{
    switch (nIntValue)
    {
    case 1: // dot
        return sal_Unicode(0x002e);
    case 2: // hyphen
        return sal_Unicode(0x002d);
    case 3: // underscore
    case 4: // heavy FIXME ???
        return sal_Unicode(0x005f);
    case NS_ooxml::LN_Value_ST_TabTlc_middleDot: // middleDot
        return sal_Unicode(0x00b7);
    case 0: // none
    default:
        return sal_Unicode(0x0020); // blank space
    }
}

} //namespace dmapper
