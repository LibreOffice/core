/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DomainMapper.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: os $ $Date: 2006-11-20 12:19:03 $
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
sal_Int32 lcl_ConvertColor(sal_Int32 nIntValue)
{

    sal_uInt8
        r(static_cast<sal_uInt8>(nIntValue&0xFF)),
        g(static_cast<sal_uInt8>(((nIntValue)>>8)&0xFF)),
        b(static_cast<sal_uInt8>((nIntValue>>16)&0xFF)),
        t(static_cast<sal_uInt8>((nIntValue>>24)&0xFF));
    sal_Int32 nRet = (t<<24) + (r<<16) + (g<<8) + b;
    return nRet;
}

/*-- 09.06.2006 09:52:11---------------------------------------------------

  -----------------------------------------------------------------------*/
DomainMapper::DomainMapper( const uno::Reference< uno::XComponentContext >& xContext,
                            uno::Reference< lang::XComponent > xModel) :
    m_pImpl( new DomainMapper_Impl( *this, xContext, xModel ))
{
}
/*-- 09.06.2006 09:52:12---------------------------------------------------

-----------------------------------------------------------------------*/
DomainMapper::~DomainMapper()
{
}
/*-- 09.06.2006 09:52:12---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::attribute(doctok::Id Name, doctok::Value & val)
{
    sal_Int32 nIntValue = val.getInt();
    if( Name >= NS_rtf::LN_WIDENT && Name <= NS_rtf::LN_LCBSTTBFUSSR )
        m_pImpl->GetFIB().SetData( Name, nIntValue );
    else
    {

        /* WRITERFILTERSTATUS: table: attributedata */
        switch( Name )
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
            const StyleSheetEntry* pEntry = m_pImpl->GetStyleSheetTable()->FindStyleSheetByISTD(nIntValue);
            if(pEntry)
            {
                bool bParaStyle = pEntry->nPropertyCalls > 1;
                if(bParaStyle)
                    m_pImpl->SetCurrentParaStyleId(nIntValue);
                m_pImpl->GetTopContext()->Insert(
                                                 bParaStyle ?
                                                 PROP_PARA_STYLE_NAME  : PROP_CHAR_STYLE_NAME,
                                                 uno::makeAny( pEntry->sStyleName ) );
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
                m_pImpl->GetFIB().SetData( Name, nIntValue );
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
            break;
        case NS_rtf::LN_BRCLEFT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_BRCBOTTOM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_BRCRIGHT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
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

            {
                //section properties
                doctok::Reference<Properties>::Pointer_t pProperties = val.getProperties();
                if( pProperties.get())
                {
                    pProperties->resolve(*this);
                }
            }
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
            m_pImpl->ModifyCurrentTabStop(Name, nIntValue);
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
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_cellLeftColor:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_cellBottomColor:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_cellRightColor:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
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
        default:
            {
                //int nVal = val.getInt();
            }
        }
    }
}
/*-- 09.06.2006 09:52:12---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::sprm(doctok::Sprm & sprm_)
{
    m_pImpl->getTableManager().sprm(sprm_);

    DomainMapper::sprm( sprm_, m_pImpl->GetTopContext() );
}
/*-- 20.06.2006 09:58:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper::sprm( doctok::Sprm& sprm_, PropertyMapPtr rContext, SprmType eSprmType )
{
    OSL_ENSURE(rContext.get(), "PropertyMap has to be valid!");
    if(!rContext.get())
        return ;
    sal_uInt32 nId = sprm_.getId();

    //TODO: In rtl-paragraphs the meaning of left/right are to be exchanged
    bool bExchangeLeftRight = false;
    // if( nId == 0x2461 && AlreadyInRTLPara() )
    //      bExchangeLeftRight = true;
    doctok::Value::Pointer_t pValue = sprm_.getValue();
    sal_Int32 nIntValue = pValue->getInt();
    /* WRITERFILTERSTATUS: table: sprmdata */
    switch(nId)
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
        {
            sal_Int16 nAdjust = 0;
            sal_Int16 nLastLineAdjust = 0;
            switch(nIntValue)
            {
            case 0: nAdjust = static_cast< sal_Int16 > (
                                                        bExchangeLeftRight ? style::ParagraphAdjust_RIGHT : style::ParagraphAdjust_LEFT);
                break;
            case 1: nAdjust = style::ParagraphAdjust_CENTER;   break;
            case 2: nAdjust = static_cast< sal_Int16 > (
                                                        bExchangeLeftRight ? style::ParagraphAdjust_LEFT : style::ParagraphAdjust_RIGHT);
                break;
            case 4:
                nLastLineAdjust = style::ParagraphAdjust_BLOCK;
                //no break;
            case 3: nAdjust = style::ParagraphAdjust_BLOCK;    break;
            }
            rContext->Insert( PROP_PARA_ADJUST, uno::makeAny( nAdjust ) );
            rContext->Insert( PROP_PARA_LAST_LINE_ADJUST, uno::makeAny( nLastLineAdjust ) );
        }
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
        rContext->Insert(PROP_PARA_KEEP_TOGETHER, uno::makeAny( nIntValue ? true : false) );
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
        rContext->Insert( PROP_NUMBERING_LEVEL, uno::makeAny( (sal_Int16)nIntValue ));
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
                rContext->Insert( PROP_NUMBERING_RULES,
                                  uno::makeAny(pListTable->GetNumberingRules(nListId)));
                //TODO: Merge overwrittern numbering levels from LFO table
            }
        }
        break;
    case 0x240C:   // sprmPFNoLineNumb
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 1 */
        /* WRITERFILTERSTATUS: comment:  */
        rContext->Insert(PROP_PARA_LINE_NUMBER_COUNT, uno::makeAny( nIntValue ? false : true) );
        break;
    case 0xC60D:   // sprmPChgTabsPapx
        /* WRITERFILTERSTATUS: done: 90, planned: 8, spent: 8 */
        /* WRITERFILTERSTATUS: comment: bar tab stops a unavailable */
        {
            doctok::Reference<Properties>::Pointer_t pProperties = sprm_.getProps();
            // Initialize tab stop vector from style sheet
            uno::Any aValue = m_pImpl->GetPropertyFromStyleSheet(PROP_PARA_TAB_STOPS);
            uno::Sequence< style::TabStop > aStyleTabStops;
            if(aValue >>= aStyleTabStops)
            {
                m_pImpl->InitTabStopFromStyle( aStyleTabStops );
            }

            //create a new tab stop property - this is done with the contained properties
            if( pProperties.get())
                pProperties->resolve(*this);
            //add this property
            rContext->Insert(PROP_PARA_TAB_STOPS, uno::makeAny( m_pImpl->GetCurrentTabStopAndClear()));
        }
        break;
    case 0x845d:    //right margin Asian - undocumented
    case 0x845e:    //left margin Asian - undocumented
    case 16:      // sprmPDxaRight - right margin
    case 0x840E:   // sprmPDxaRight - right margin
    case 17:
    case 0x840F:   // sprmPDxaLeft
        /* WRITERFILTERSTATUS: done: 50, planned: 5, spent: 1 */
        if( 0x840e == nId || 0x17 == nId|| (bExchangeLeftRight && nId == 0x845d) || ( !bExchangeLeftRight && nId == 0x845e))
            rContext->Insert(
                             eSprmType == SPRM_DEFAULT ? PROP_PARA_LEFT_MARGIN : PROP_LEFT_MARGIN,

                             uno::makeAny( ConversionHelper::convertToMM100( nIntValue ) ));
        else if(eSprmType == SPRM_DEFAULT)
            rContext->Insert(
                             PROP_PARA_RIGHT_MARGIN,
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
            rContext->Insert(PROP_PARA_LINE_SPACING, uno::makeAny( aSpacing ));
        }
        break;
    case 21 : // legacy version
    case 0xA413:   // sprmPDyaBefore
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 1 */
        rContext->Insert(PROP_PARA_TOP_MARGIN, uno::makeAny( ConversionHelper::convertToMM100( nIntValue ) ));
        break;
    case 22 :
    case 0xA414:   // sprmPDyaAfter
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 1 */
        rContext->Insert(PROP_PARA_BOTTOM_MARGIN, uno::makeAny( ConversionHelper::convertToMM100( nIntValue ) ));
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
            switch( nId )
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
            rContext->Insert(eBorderId, uno::makeAny( aBorderLine ));
            rContext->Insert(eBorderDistId, uno::makeAny( nLineDistance ));
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
        rContext->Insert(PROP_PARA_IS_HYPHENATION, uno::makeAny( nIntValue ? false : true ));
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
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
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
        rContext->Insert(PROP_PARA_IS_HANGING_PUNCTUATION, uno::makeAny( nIntValue ? false : true ));
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
    case 0x6A09:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCSymbol
    case 0x080A:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFOle2
    case 0x480B:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCIdCharType
    case 0x2A0C:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
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
        break;  // sprmCKcd
    case 0x0858:// sprmCFEmboss
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case 060:// sprmCFBold
    case 0x085C:// sprmCFBoldBi    (offset 0x27 to normal bold)
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case 0x085D:// sprmCFItalicBi  (offset 0x27 to normal italic)
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case 0x835: //sprmCFBold
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
            switch( nId )
            {
            case 060:// sprmCFBold
            case 0x085C: // sprmCFBoldBi
            case 0x835: /*sprmCFBold*/
                ePropertyId = nId != 0x085C ? PROP_CHAR_WEIGHT : PROP_CHAR_WEIGHT_COMPLEX;
                break;
            case 0x085D: // sprmCFItalicBi
            case 0x836: /*sprmCFItalic*/
                ePropertyId = nId == 0x836 ? PROP_CHAR_POSTURE : PROP_CHAR_POSTURE_COMPLEX;
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
                    sal_Int16 nStyleValue;
                    double fDoubleValue;
                    uno::Any aStyleVal = m_pImpl->GetPropertyFromStyleSheet(ePropertyId);
                    if( !aStyleVal.hasValue() )
                    {
                        nIntValue = 0x83a == nId ?
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
                        nIntValue = 0x83a == nId ?
                            nStyleValue ? 0 : 4 :
                            nStyleValue ? 0 : 1;
                    }
                    else
                    {
                        OSL_ASSERT("what type was it");
                    }
                }
                sal_uInt16 nPropertyNameId = 0;
                switch( nId )
                {
                case 060:/*sprmCFBold*/
                case 0x085C: // sprmCFBoldBi
                case 0x835: /*sprmCFBold*/
                    rContext->Insert(ePropertyId,
                                     uno::makeAny( nIntValue ? awt::FontWeight::BOLD : awt::FontWeight::NORMAL ) );
                    break;
                case 0x085D: // sprmCFItalicBi
                case 0x836: /*sprmCFItalic*/
                    rContext->Insert(ePropertyId,
                                     uno::makeAny( nIntValue ? awt::FontSlant_ITALIC : awt::FontSlant_NONE ) );
                    break;
                case 0x837: /*sprmCFStrike*/
                    rContext->Insert(ePropertyId,
                                     uno::makeAny( nIntValue ? awt::FontStrikeout::SINGLE : awt::FontStrikeout::NONE ) );
                    break;
                case 0x2A53 : /*sprmCFDStrike double strike through*/
                    rContext->Insert(ePropertyId,
                                     uno::makeAny( awt::FontStrikeout::DOUBLE ) );
                    break;
                case 0x838: /*sprmCFOutline*/
                    nPropertyNameId = static_cast<sal_uInt16>( ePropertyId );
                    break;
                case 0x83a: /*sprmCFSmallCaps*/
                    rContext->Insert(ePropertyId,
                                     uno::makeAny( nIntValue ? style::CaseMap::SMALLCAPS : style::CaseMap::NONE));
                    break;
                case 0x83b: /*sprmCFCaps*/
                    rContext->Insert(ePropertyId,
                                     uno::makeAny( nIntValue ? style::CaseMap::UPPERCASE : style::CaseMap::NONE));
                    break;
                case 0x83c: /*sprmCFVanish*/
                    break;
                case 0x0858: /*sprmCFEmboss*/
                    rContext->Insert(ePropertyId,
                                     uno::makeAny( nIntValue ? awt::FontRelief::EMBOSSED : awt::FontRelief::NONE ));
                    break;

                }
                if(nPropertyNameId)
                    rContext->Insert((PropertyIds)nPropertyNameId, uno::makeAny( nIntValue ? true : false ) );
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
            sal_Int16 eUnderline = awt::FontUnderline::NONE;
            switch(nIntValue)
            {
            case 0: eUnderline = awt::FontUnderline::NONE; break;
            case 2: rContext->Insert(PROP_CHAR_WORD_MODE, uno::makeAny( true ) ); // TODO: how to get rid of it?
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
            rContext->Insert(PROP_CHAR_UNDERLINE, uno::makeAny( eUnderline ) );
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
        break;  // sprmCIco
    case 0x4A61:    // sprmCHpsBi
    case 0x4A43:    // sprmCHps
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        {
            //multiples of half points (12pt == 24)
            double fVal = double(nIntValue) / 2.;
            rContext->Insert(
                             0x4A61 == nId ? PROP_CHAR_HEIGHT_COMPLEX : PROP_CHAR_HEIGHT, uno::makeAny( fVal ) );
        }
        break;
    case 0x2A44:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCHpsInc
    case 0x4845:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
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
            rContext->Insert(PROP_CHAR_ESCAPEMENT,         uno::makeAny( nEscapement ) );
            rContext->Insert(PROP_CHAR_ESCAPEMENT_HEIGHT,  uno::makeAny( nProp ) );
        }
        break;
    case 0xCA49:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCHpsNew50
    case 0xCA4A:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCHpsInc1
    case 0x71 : //"sprmCDxaSpace"
    case 0x96 : //"sprmCDxaSpace"
    case 0x8840:  // sprmCDxaSpace
        /* WRITERFILTERSTATUS: done: 50, planned: 2, spent: 0 */
        //Kerning half point values
        //TODO: there are two kerning values -
        // in ww8par6.cxx 0x484b is used as boolean AutoKerning
        rContext->Insert(PROP_CHAR_CHAR_KERNING, uno::makeAny( sal_Int16(ConversionHelper::convertToMM100(sal_Int16(nIntValue))) ) );
        break;
    case 0x484B:  // sprmCHpsKern    auto kerning is bound to a minimum font size in Word - but not in Writer :-(
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        rContext->Insert(PROP_CHAR_AUTO_KERNING, uno::makeAny( true ) );
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
                switch(nId)
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
                rContext->Insert(eFontName, uno::makeAny( pFontEntry->sFontName  ));
                //                rContext->Insert(eFontStyle, uno::makeAny( pFontEntry->  ));
                //                rContext->Insert(eFontFamily, uno::makeAny( pFontEntry->  ));
                rContext->Insert(eFontCharSet, uno::makeAny( (sal_Int16)pFontEntry->nTextEncoding  ));
                rContext->Insert(eFontPitch, uno::makeAny( pFontEntry->nPitchRequest  ));
            }
        }
        break;
    case 0x4852:  // sprmCCharScale
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        rContext->Insert(PROP_CHAR_SCALE_WIDTH,
                         uno::makeAny( sal_Int16(nIntValue) ));

        break;
    case 0x0854: // sprmCFImprint   1 or 0
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        // FontRelief: NONE, EMBOSSED, ENGRAVED
        rContext->Insert(PROP_CHAR_RELIEF,
                         uno::makeAny( nIntValue ? awt::FontRelief::ENGRAVED : awt::FontRelief::NONE ));
        break;
    case 0x0856:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFObj
    case 0xCA57:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCPropRMark
    case 0x2859:
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
    case 0x486D:   // sprmCRgLid0    language Western
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
    case 0x486E:   // sprmCRgLid1    language Asian
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        {
            lang::Locale aLocale;
            MsLangId::convertLanguageToLocale( (LanguageType)nIntValue, aLocale );
            rContext->Insert(0x486D == nId ? PROP_CHAR_LOCALE :
                             0x486E == nId ? PROP_CHAR_LOCALE_ASIAN : PROP_CHAR_LOCALE_COMPLEX,
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
        break;  // sprmPicBrcBottom
    case 0x6C05:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPicBrcRight
    case 0x3000:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmScnsPgn
    case 0x3001:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSiHeadingPgn
    case 0xD202:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSOlstAnm
    case 0xF203:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSDxaColWidth
    case 0xF204:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSDxaColSpacing
    case 0x3005:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSFEvenlySpaced
    case 0x3006:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSFProtected
    case 0x5007:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSDmBinFirst
    case 0x5008:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSDmBinOther
    case 0x3009: // sprmSBkc
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        //break type
        /*
          0 - No break
          1 - New Colunn
          2 - New page
          3 - Even page
          4 - odd page
          ->get the related page style name and apply it at the current paragraph
        */
        OSL_ASSERT("TODO: not handled yet");
        break;
    case 0x300A: // sprmSFTitlePage
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        OSL_ASSERT("TODO: not handled yet"); //section has title page
        break;
    case 0x500B:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSCcolumns
    case 0x900C:           // sprmSDxaColumns
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        OSL_ASSERT("TODO: not handled yet"); //column distance? 0x2c4
        break;
    case 0x300D:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSFAutoPgn
    case 0x300E:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSNfcPgn
    case 0xB00F:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSDyaPgn
    case 0xB010:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSDxaPgn
    case 0x3011:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSFPgnRestart
    case 0x3012:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSFEndnote
    case 0x3013:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSLnc
    case 0x3014:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSGprfIhdt
    case 0x5015:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSNLnnMod
    case 0x9016:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSDxaLnn
    case 0xB017: // sprmSDyaHdrTop
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
    case 0xB018: // sprmSDyaHdrBottom
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        OSL_ASSERT("TODO: not handled yet"); //header top/bottom distance 0x2c5 == 0,5in
        break;
    case 0x3019:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        break;  // sprmSLBetween
    case 0x301A:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        break;  // sprmSVjc
    case 0x501B:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        break;  // sprmSLnnMin
    case 0x501C:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        break;  // sprmSPgnStart
    case 0x301D:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        break;  // sprmSBOrientation
    case 0x301E:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        break;  // sprmSBCustomize
    case 0xB020: // sprmSYaPage
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case 0xB01F:   // sprmSXaPage
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        OSL_ASSERT("TODO: not handled yet"); //paper width 0x2fd0 =0 8,5 in, height 3de0 == 11 in

        break;
    case 0xB021:  // sprmSDxaLeft
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case 0xB022: // sprmSDxaRight
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case 0x9023: // sprmSDyaTop
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case 0x9024: // sprmSDyaBottom
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        OSL_ASSERT("TODO: not handled yet"); //page margins 0x705/0x705 == 1,25 in and 0x5a0 == 1 in
        break;
    case 0xB025:   // sprmSDzaGutter
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        OSL_ASSERT("TODO: not handled yet"); // gutter is added to one of the margins of a section depending on RTL, can be placed on top either
        break;
    case 0x5026:   // sprmSDmPaperReq
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        OSL_ASSERT("TODO: not handled yet"); //paper code
        break;
    case 0xD227:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSPropRMark
    case 0x3228:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSFBiDi
    case 0x3229:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSFFacingCol
    case 0x322A:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSFRTLGutter
    case 0x702B:   // sprmSBrcTop
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
    case 0x702C:   // sprmSBrcLeft
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
    case 0x702D:  // sprmSBrcBottom
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
    case 0x702E:  // sprmSBrcRight
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        {
            table::BorderLine aBorderLine;
            sal_Int32 nLineDistance = ConversionHelper::MakeBorderLine( nIntValue, aBorderLine );
            PropertyMapPtr pContext = m_pImpl->GetTopContextOfType(CONTEXT_SECTION);
            SectionPropertyMap* pSectionContext = dynamic_cast< SectionPropertyMap* >( pContext.get() );
            if(pSectionContext)
            {
                static const BorderPosition aPositions[4] =
                    {
                        BORDER_TOP,
                        BORDER_LEFT,
                        BORDER_BOTTOM,
                        BORDER_RIGHT
                    };
                pSectionContext->SetBorder( aPositions[nId - 0x702B], nLineDistance, aBorderLine );
            }
        }
        break;

    case 0x522F:  // sprmSPgbProp
        {
            PropertyMapPtr pContext = m_pImpl->GetTopContextOfType(CONTEXT_SECTION);
            SectionPropertyMap* pSectionContext = dynamic_cast< SectionPropertyMap* >( pContext.get() );
            if(pSectionContext)
            {
                pSectionContext->ApplyBorderToPageStyles( m_pImpl->GetPageStyles(), nIntValue );
            }
        }
        break;
    case 0x7030:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSDxtCharSpace
    case 0x9031:   // sprmSDyaLinePitch
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        OSL_ASSERT("TODO: not handled yet"); //line pitch of grid
        break;
    case 0x703a: //undocumented, grid related?
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        OSL_ASSERT("TODO: not handled yet"); //nIntValue like 0x008a2373 ?
        break;
    case 0x5032:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSClm
    case 0x5033:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSTextFlow
    case 0x5400:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTJc
    case 0x9601:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTDxaLeft
    case 0x9602:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTDxaGapHalf
    case 0x3403:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTFCantSplit
    case 0x3404:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTTableHeader
    case 0xD605:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTTableBorders
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
            sal_Int32 nColor = lcl_ConvertColor(nIntValue);
            rContext->Insert(PROP_CHAR_COLOR, uno::makeAny( nColor ) );
        }
        break;
    case 0x4873:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break; //sprmCRgLid - undocumented
    case 0x4874:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break; //seems to be a language id for Asian text - undocumented
    case 0x6877: //underlining color
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        {
            sal_Int32 nColor = lcl_ConvertColor(nIntValue);
            rContext->Insert(PROP_CHAR_UNDERLINE_HAS_COLOR, uno::makeAny( true ) );
            rContext->Insert(PROP_CHAR_UNDERLINE_COLOR, uno::makeAny( nColor ) );
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
    case 0xd634:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break; //undocumented
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
    default:
        {
            //doctok::Value::Pointer_t pValue_ = sprm_.getValue();
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
/*-- 09.06.2006 09:52:14---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::startCharacterGroup()
{
    m_pImpl->PushProperties(CONTEXT_CHARACTER);
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
    try
    {
        m_pImpl->getTableManager().text(data_, len);

        bool bContinue = true;
        //TODO: Determine the right text encoding (FIB?)
        ::rtl::OUString sText( (const sal_Char*) data_, len, RTL_TEXTENCODING_MS_1252 );
        if(len == 1)
        {
            bContinue = false;
            switch(*data_)
            {
            case 0x07:
            case 0x0d:
                m_pImpl->finishParagraph(m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH)); break;
            case 0x13: m_pImpl->SetFieldMode( true );break;
            case 0x14: /* delimiter not necessarily available */
            case 0x15: /* end of field */
                m_pImpl->SetFieldMode( false );
                break;
            default: bContinue = true;
            }
        }
        if(bContinue)
        {
            if( m_pImpl->IsFieldMode())
                m_pImpl->CreateField( sText );
            else if( m_pImpl->IsFieldAvailable())
                /*depending on the success of the field insert operation this result will be
                  set at the field or directly inserted into the text*/
                m_pImpl->SetFieldResult( sText );
            else
            {
                PropertyMapPtr pContext = m_pImpl->GetTopContext();
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
    try
    {
        m_pImpl->getTableManager().utext(data_, len);

        if(len == 1 && ((*data_) == 0x0d || (*data_) == 0x07))
            m_pImpl->finishParagraph(m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH));
        else
        {
            PropertyMapPtr pContext = m_pImpl->GetTopContext();
            OUString sText;
            OUStringBuffer aBuffer = OUStringBuffer(len);
            aBuffer.append( (const sal_Unicode *) data_, len);
            sText = aBuffer.makeStringAndClear();
            //--> debug
            //sal_uInt32 nSize = pContext->size();
            //<--
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

        {
            // create a font table object that listens to the attributes
            // each entry call inserts a new font entry
            ref->resolve( *m_pImpl->GetFontTable() );
        }
        break;
    case NS_rtf::LN_STYLESHEET:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        //same as above to import style sheets
        m_pImpl->SetStyleSheetImport( true );
        ref->resolve( *m_pImpl->GetStyleSheetTable() );
        m_pImpl->GetStyleSheetTable()->ApplyStyleSheets(m_pImpl->GetTextDocument());
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
void DomainMapper::substream(doctok::Id name, ::doctok::Reference<Stream>::Pointer_t ref)
{
    m_pImpl->getTableManager().startLevel();

    //->debug
    //string sName = (*doctok::QNameToString::Instance())(name);
    //--<debug
    //import of page header/footer

    /* WRITERFILTERSTATUS: table: attributedata */
    switch( name )
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
    }
    ref->resolve(*this);
    switch( name )
    {
    case NS_rtf::LN_headerl:
    case NS_rtf::LN_headerr:
    case NS_rtf::LN_headerf:
    case NS_rtf::LN_footerl:
    case NS_rtf::LN_footerr:
    case NS_rtf::LN_footerf:
        m_pImpl->PopPageHeaderFooter();
        break;
    }

    m_pImpl->getTableManager().endLevel();
}
/*-- 09.06.2006 09:52:16---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::info(const string & /*info_*/)
{
}
}

