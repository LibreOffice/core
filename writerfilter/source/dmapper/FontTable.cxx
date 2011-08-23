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

#include <FontTable.hxx>
#ifndef INCLUDED_RESOURCESIDS
#include <doctok/resourceids.hxx>
#include <ooxml/resourceids.hxx>
#endif
#include <vector>
#include <stdio.h>

namespace writerfilter {
namespace dmapper
{

struct FontTable_Impl
{
    std::vector< FontEntry > aFontEntries;
    FontEntry::Pointer_t pCurrentEntry;
    FontTable_Impl() {}
};
/*-- 19.06.2006 12:04:32---------------------------------------------------

  -----------------------------------------------------------------------*/
FontTable::FontTable() :
    m_pImpl( new FontTable_Impl )
{
}
/*-- 19.06.2006 12:04:33---------------------------------------------------

  -----------------------------------------------------------------------*/
FontTable::~FontTable()
{
    delete m_pImpl;
}
/*-- 19.06.2006 12:04:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void FontTable::attribute(Id Name, Value & val)
{
    OSL_ENSURE( m_pImpl->pCurrentEntry, "current entry has to be set here");
    if(!m_pImpl->pCurrentEntry)
        return ;
    int nIntValue = val.getInt();
    ::rtl::OUString sValue = val.getString();
    //printf ( "FontTable::attribute(0x%.4x, 0x%.4x) [%s]\n", (unsigned int)Name, (unsigned int)nIntValue, ::rtl::OUStringToOString(sValue, RTL_TEXTENCODING_DONTKNOW).getStr());
    /* WRITERFILTERSTATUS: table: FontTable_attributedata */
    switch(Name)
    {
//        case NS_rtf::LN_ISTD: break;
//        case NS_rtf::LN_ISTARTAT: break;
//        case NS_rtf::LN_NFC: break;
//        case NS_rtf::LN_JC: break;
//        case NS_rtf::LN_FLEGAL: break;
//        case NS_rtf::LN_FNORESTART: break;
//        case NS_rtf::LN_FPREV: break;
//        case NS_rtf::LN_FPREVSPACE: break;
//        case NS_rtf::LN_FWORD6: break;
//        case NS_rtf::LN_UNUSED5_7: break;
//        case NS_rtf::LN_RGBXCHNUMS: break;
//        case NS_rtf::LN_IXCHFOLLOW: break;
//        case NS_rtf::LN_DXASPACE: break;
//        case NS_rtf::LN_DXAINDENT: break;
//        case NS_rtf::LN_CBGRPPRLCHPX: break;
//        case NS_rtf::LN_CBGRPPRLPAPX: break;
//        case NS_rtf::LN_LSID: break;
//        case NS_rtf::LN_TPLC: break;
//        case NS_rtf::LN_RGISTD: break;
//        case NS_rtf::LN_FSIMPLELIST: break;
//        case NS_rtf::LN_FRESTARTHDN: break;
//        case NS_rtf::LN_UNSIGNED26_2: break;
//        case NS_rtf::LN_ILVL: break;
//        case NS_rtf::LN_FSTARTAT: break;
//        case NS_rtf::LN_FFORMATTING: break;
//        case NS_rtf::LN_UNSIGNED4_6: break;
//        case NS_rtf::LN_UNUSED4: break;
//        case NS_rtf::LN_UNUSED8: break;
//        case NS_rtf::LN_CLFOLVL: break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_rtf::LN_CBFFNM1:
            m_pImpl->pCurrentEntry->sFontName1 = sValue;
        break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_rtf::LN_PRQ:
            m_pImpl->pCurrentEntry->nPitchRequest = static_cast<sal_Int16>( nIntValue );
        break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_rtf::LN_FTRUETYPE:
            m_pImpl->pCurrentEntry->bTrueType = nIntValue == 1 ? true : false;
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_UNUSED1_3: //unused
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FF: //unused
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_UNUSED1_7: //unused
        break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_rtf::LN_WWEIGHT:
            m_pImpl->pCurrentEntry->nBaseWeight = nIntValue;
        break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_rtf::LN_CHS:
            m_pImpl->pCurrentEntry->nTextEncoding = nIntValue;
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_IXCHSZALT:
        break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_rtf::LN_PANOSE:
            m_pImpl->pCurrentEntry->sPanose += sValue;
        break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_rtf::LN_FS:
            m_pImpl->pCurrentEntry->sFontSignature += sValue;
        break;
//        case NS_rtf::LN_STI: break;
//        case NS_rtf::LN_FSCRATCH: break;
//        case NS_rtf::LN_FINVALHEIGHT: break;
//        case NS_rtf::LN_FHASUPE: break;
//        case NS_rtf::LN_FMASSCOPY: break;
//        case NS_rtf::LN_SGC: break;
//        case NS_rtf::LN_ISTDBASE: break;
//        case NS_rtf::LN_CUPX: break;
//        case NS_rtf::LN_ISTDNEXT: break;
//        case NS_rtf::LN_BCHUPE: break;
//        case NS_rtf::LN_FAUTOREDEF: break;
//        case NS_rtf::LN_FHIDDEN: break;
//        case NS_rtf::LN_UNUSED8_3: break;
//        case NS_rtf::LN_CSTD: break;
//        case NS_rtf::LN_CBSTDBASEINFILE: break;
//        case NS_rtf::LN_FSTDSTYLENAMESWRITTEN: break;
//        case NS_rtf::LN_UNUSED4_2: break;
//        case NS_rtf::LN_STIMAXWHENSAVED: break;
//        case NS_rtf::LN_ISTDMAXFIXEDWHENSAVED: break;
//        case NS_rtf::LN_NVERBUILTINNAMESWHENSAVED: break;
//        case NS_rtf::LN_RGFTCSTANDARDCHPSTSH: break;
//        case NS_rtf::LN_WIDENT: break;
//        case NS_rtf::LN_NFIB: break;
//        case NS_rtf::LN_NPRODUCT: break;
//        case NS_rtf::LN_LID: break;
//        case NS_rtf::LN_PNNEXT: break;
//        case NS_rtf::LN_FDOT: break;
//        case NS_rtf::LN_FGLSY: break;
//        case NS_rtf::LN_FCOMPLEX: break;
//        case NS_rtf::LN_FHASPIC: break;
//        case NS_rtf::LN_CQUICKSAVES: break;
//        case NS_rtf::LN_FENCRYPTED: break;
//        case NS_rtf::LN_FWHICHTBLSTM: break;
//        case NS_rtf::LN_FREADONLYRECOMMENDED: break;
//        case NS_rtf::LN_FWRITERESERVATION: break;
//        case NS_rtf::LN_FEXTCHAR: break;
//        case NS_rtf::LN_FLOADOVERRIDE: break;
//        case NS_rtf::LN_FFAREAST: break;
//        case NS_rtf::LN_FCRYPTO: break;
//        case NS_rtf::LN_NFIBBACK: break;
//        case NS_rtf::LN_LKEY: break;
//        case NS_rtf::LN_ENVR: break;
//        case NS_rtf::LN_FMAC: break;
//        case NS_rtf::LN_FEMPTYSPECIAL: break;
//        case NS_rtf::LN_FLOADOVERRIDEPAGE: break;
//        case NS_rtf::LN_FFUTURESAVEDUNDO: break;
//        case NS_rtf::LN_FWORD97SAVED: break;
//        case NS_rtf::LN_FSPARE0: break;
//        case NS_rtf::LN_CHSTABLES: break;
//        case NS_rtf::LN_FCMIN: break;
//        case NS_rtf::LN_FCMAC: break;
//        case NS_rtf::LN_CSW: break;
//        case NS_rtf::LN_WMAGICCREATED: break;
//        case NS_rtf::LN_WMAGICREVISED: break;
//        case NS_rtf::LN_WMAGICCREATEDPRIVATE: break;
//        case NS_rtf::LN_WMAGICREVISEDPRIVATE: break;
//        case NS_rtf::LN_PNFBPCHPFIRST_W6: break;
//        case NS_rtf::LN_PNCHPFIRST_W6: break;
//        case NS_rtf::LN_CPNBTECHP_W6: break;
//        case NS_rtf::LN_PNFBPPAPFIRST_W6: break;
//        case NS_rtf::LN_PNPAPFIRST_W6: break;
//        case NS_rtf::LN_CPNBTEPAP_W6: break;
//        case NS_rtf::LN_PNFBPLVCFIRST_W6: break;
//        case NS_rtf::LN_PNLVCFIRST_W6: break;
//        case NS_rtf::LN_CPNBTELVC_W6: break;
//        case NS_rtf::LN_LIDFE: break;
//        case NS_rtf::LN_CLW: break;
//        case NS_rtf::LN_CBMAC: break;
//        case NS_rtf::LN_LPRODUCTCREATED: break;
//        case NS_rtf::LN_LPRODUCTREVISED: break;
//        case NS_rtf::LN_CCPTEXT: break;
//        case NS_rtf::LN_CCPFTN: break;
//        case NS_rtf::LN_CCPHDD: break;
//        case NS_rtf::LN_CCPMCR: break;
//        case NS_rtf::LN_CCPATN: break;
//        case NS_rtf::LN_CCPEDN: break;
//        case NS_rtf::LN_CCPTXBX: break;
//        case NS_rtf::LN_CCPHDRTXBX: break;
//        case NS_rtf::LN_PNFBPCHPFIRST: break;
//        case NS_rtf::LN_PNCHPFIRST: break;
//        case NS_rtf::LN_CPNBTECHP: break;
//        case NS_rtf::LN_PNFBPPAPFIRST: break;
//        case NS_rtf::LN_PNPAPFIRST: break;
//        case NS_rtf::LN_CPNBTEPAP: break;
//        case NS_rtf::LN_PNFBPLVCFIRST: break;
//        case NS_rtf::LN_PNLVCFIRST: break;
//        case NS_rtf::LN_CPNBTELVC: break;
//        case NS_rtf::LN_FCISLANDFIRST: break;
//        case NS_rtf::LN_FCISLANDLIM: break;
//        case NS_rtf::LN_CFCLCB: break;
//        case NS_rtf::LN_FCSTSHFORIG: break;
//        case NS_rtf::LN_LCBSTSHFORIG: break;
//        case NS_rtf::LN_FCSTSHF: break;
//        case NS_rtf::LN_LCBSTSHF: break;
//        case NS_rtf::LN_FCPLCFFNDREF: break;
//        case NS_rtf::LN_LCBPLCFFNDREF: break;
//        case NS_rtf::LN_FCPLCFFNDTXT: break;
//        case NS_rtf::LN_LCBPLCFFNDTXT: break;
//        case NS_rtf::LN_FCPLCFANDREF: break;
//        case NS_rtf::LN_LCBPLCFANDREF: break;
//        case NS_rtf::LN_FCPLCFANDTXT: break;
//        case NS_rtf::LN_LCBPLCFANDTXT: break;
//        case NS_rtf::LN_FCPLCFSED: break;
//        case NS_rtf::LN_LCBPLCFSED: break;
//        case NS_rtf::LN_FCPLCFPAD: break;
//        case NS_rtf::LN_LCBPLCFPAD: break;
//        case NS_rtf::LN_FCPLCFPHE: break;
//        case NS_rtf::LN_LCBPLCFPHE: break;
//        case NS_rtf::LN_FCSTTBFGLSY: break;
//        case NS_rtf::LN_LCBSTTBFGLSY: break;
//        case NS_rtf::LN_FCPLCFGLSY: break;
//        case NS_rtf::LN_LCBPLCFGLSY: break;
//        case NS_rtf::LN_FCPLCFHDD: break;
//        case NS_rtf::LN_LCBPLCFHDD: break;
//        case NS_rtf::LN_FCPLCFBTECHPX: break;
//        case NS_rtf::LN_LCBPLCFBTECHPX: break;
//        case NS_rtf::LN_FCPLCFBTEPAPX: break;
//        case NS_rtf::LN_LCBPLCFBTEPAPX: break;
//        case NS_rtf::LN_FCPLCFSEA: break;
//        case NS_rtf::LN_LCBPLCFSEA: break;
//        case NS_rtf::LN_FCSTTBFFFN: break;
//        case NS_rtf::LN_LCBSTTBFFFN: break;
//        case NS_rtf::LN_FCPLCFFLDMOM: break;
//        case NS_rtf::LN_LCBPLCFFLDMOM: break;
//        case NS_rtf::LN_FCPLCFFLDHDR: break;
//        case NS_rtf::LN_LCBPLCFFLDHDR: break;
//        case NS_rtf::LN_FCPLCFFLDFTN: break;
//        case NS_rtf::LN_LCBPLCFFLDFTN: break;
//        case NS_rtf::LN_FCPLCFFLDATN: break;
//        case NS_rtf::LN_LCBPLCFFLDATN: break;
//        case NS_rtf::LN_FCPLCFFLDMCR: break;
//        case NS_rtf::LN_LCBPLCFFLDMCR: break;
//        case NS_rtf::LN_FCSTTBFBKMK: break;
//        case NS_rtf::LN_LCBSTTBFBKMK: break;
//        case NS_rtf::LN_FCPLCFBKF: break;
//        case NS_rtf::LN_LCBPLCFBKF: break;
//        case NS_rtf::LN_FCPLCFBKL: break;
//        case NS_rtf::LN_LCBPLCFBKL: break;
//        case NS_rtf::LN_FCCMDS: break;
//        case NS_rtf::LN_LCBCMDS: break;
//        case NS_rtf::LN_FCPLCMCR: break;
//        case NS_rtf::LN_LCBPLCMCR: break;
//        case NS_rtf::LN_FCSTTBFMCR: break;
//        case NS_rtf::LN_LCBSTTBFMCR: break;
//        case NS_rtf::LN_FCPRDRVR: break;
//        case NS_rtf::LN_LCBPRDRVR: break;
//        case NS_rtf::LN_FCPRENVPORT: break;
//        case NS_rtf::LN_LCBPRENVPORT: break;
//        case NS_rtf::LN_FCPRENVLAND: break;
//        case NS_rtf::LN_LCBPRENVLAND: break;
//        case NS_rtf::LN_FCWSS: break;
//        case NS_rtf::LN_LCBWSS: break;
//        case NS_rtf::LN_FCDOP: break;
//        case NS_rtf::LN_LCBDOP: break;
//        case NS_rtf::LN_FCSTTBFASSOC: break;
//        case NS_rtf::LN_LCBSTTBFASSOC: break;
//        case NS_rtf::LN_FCCLX: break;
//        case NS_rtf::LN_LCBCLX: break;
//        case NS_rtf::LN_FCPLCFPGDFTN: break;
//        case NS_rtf::LN_LCBPLCFPGDFTN: break;
//        case NS_rtf::LN_FCAUTOSAVESOURCE: break;
//        case NS_rtf::LN_LCBAUTOSAVESOURCE: break;
//        case NS_rtf::LN_FCGRPXSTATNOWNERS: break;
//        case NS_rtf::LN_LCBGRPXSTATNOWNERS: break;
//        case NS_rtf::LN_FCSTTBFATNBKMK: break;
//        case NS_rtf::LN_LCBSTTBFATNBKMK: break;
//        case NS_rtf::LN_FCPLCDOAMOM: break;
//        case NS_rtf::LN_LCBPLCDOAMOM: break;
//        case NS_rtf::LN_FCPLCDOAHDR: break;
//        case NS_rtf::LN_LCBPLCDOAHDR: break;
//        case NS_rtf::LN_FCPLCSPAMOM: break;
//        case NS_rtf::LN_LCBPLCSPAMOM: break;
//        case NS_rtf::LN_FCPLCSPAHDR: break;
//        case NS_rtf::LN_LCBPLCSPAHDR: break;
//        case NS_rtf::LN_FCPLCFATNBKF: break;
//        case NS_rtf::LN_LCBPLCFATNBKF: break;
//        case NS_rtf::LN_FCPLCFATNBKL: break;
//        case NS_rtf::LN_LCBPLCFATNBKL: break;
//        case NS_rtf::LN_FCPMS: break;
//        case NS_rtf::LN_LCBPMS: break;
//        case NS_rtf::LN_FCFORMFLDSTTBF: break;
//        case NS_rtf::LN_LCBFORMFLDSTTBF: break;
//        case NS_rtf::LN_FCPLCFENDREF: break;
//        case NS_rtf::LN_LCBPLCFENDREF: break;
//        case NS_rtf::LN_FCPLCFENDTXT: break;
//        case NS_rtf::LN_LCBPLCFENDTXT: break;
//        case NS_rtf::LN_FCPLCFFLDEDN: break;
//        case NS_rtf::LN_LCBPLCFFLDEDN: break;
//        case NS_rtf::LN_FCPLCFPGDEDN: break;
//        case NS_rtf::LN_LCBPLCFPGDEDN: break;
//        case NS_rtf::LN_FCDGGINFO: break;
//        case NS_rtf::LN_LCBDGGINFO: break;
//        case NS_rtf::LN_FCSTTBFRMARK: break;
//        case NS_rtf::LN_LCBSTTBFRMARK: break;
//        case NS_rtf::LN_FCSTTBFCAPTION: break;
//        case NS_rtf::LN_LCBSTTBFCAPTION: break;
//        case NS_rtf::LN_FCSTTBFAUTOCAPTION: break;
//        case NS_rtf::LN_LCBSTTBFAUTOCAPTION: break;
//        case NS_rtf::LN_FCPLCFWKB: break;
//        case NS_rtf::LN_LCBPLCFWKB: break;
//        case NS_rtf::LN_FCPLCFSPL: break;
//        case NS_rtf::LN_LCBPLCFSPL: break;
//        case NS_rtf::LN_FCPLCFTXBXTXT: break;
//        case NS_rtf::LN_LCBPLCFTXBXTXT: break;
//        case NS_rtf::LN_FCPLCFFLDTXBX: break;
//        case NS_rtf::LN_LCBPLCFFLDTXBX: break;
//        case NS_rtf::LN_FCPLCFHDRTXBXTXT: break;
//        case NS_rtf::LN_LCBPLCFHDRTXBXTXT: break;
//        case NS_rtf::LN_FCPLCFFLDHDRTXBX: break;
//        case NS_rtf::LN_LCBPLCFFLDHDRTXBX: break;
//        case NS_rtf::LN_FCSTWUSER: break;
//        case NS_rtf::LN_LCBSTWUSER: break;
//        case NS_rtf::LN_FCSTTBTTMBD: break;
//        case NS_rtf::LN_LCBSTTBTTMBD: break;
//        case NS_rtf::LN_FCUNUSED: break;
//        case NS_rtf::LN_LCBUNUSED: break;
//        case NS_rtf::LN_FCPGDMOTHER: break;
//        case NS_rtf::LN_LCBPGDMOTHER: break;
//        case NS_rtf::LN_FCBKDMOTHER: break;
//        case NS_rtf::LN_LCBBKDMOTHER: break;
//        case NS_rtf::LN_FCPGDFTN: break;
//        case NS_rtf::LN_LCBPGDFTN: break;
//        case NS_rtf::LN_FCBKDFTN: break;
//        case NS_rtf::LN_LCBBKDFTN: break;
//        case NS_rtf::LN_FCPGDEDN: break;
//        case NS_rtf::LN_LCBPGDEDN: break;
//        case NS_rtf::LN_FCBKDEDN: break;
//        case NS_rtf::LN_LCBBKDEDN: break;
//        case NS_rtf::LN_FCSTTBFINTLFLD: break;
//        case NS_rtf::LN_LCBSTTBFINTLFLD: break;
//        case NS_rtf::LN_FCROUTESLIP: break;
//        case NS_rtf::LN_LCBROUTESLIP: break;
//        case NS_rtf::LN_FCSTTBSAVEDBY: break;
//        case NS_rtf::LN_LCBSTTBSAVEDBY: break;
//        case NS_rtf::LN_FCSTTBFNM: break;
//        case NS_rtf::LN_LCBSTTBFNM: break;
//        case NS_rtf::LN_FCPLCFLST: break;
//        case NS_rtf::LN_LCBPLCFLST: break;
//        case NS_rtf::LN_FCPLFLFO: break;
//        case NS_rtf::LN_LCBPLFLFO: break;
//        case NS_rtf::LN_FCPLCFTXBXBKD: break;
//        case NS_rtf::LN_LCBPLCFTXBXBKD: break;
//        case NS_rtf::LN_FCPLCFTXBXHDRBKD: break;
//        case NS_rtf::LN_LCBPLCFTXBXHDRBKD: break;
//        case NS_rtf::LN_FCDOCUNDO: break;
//        case NS_rtf::LN_LCBDOCUNDO: break;
//        case NS_rtf::LN_FCRGBUSE: break;
//        case NS_rtf::LN_LCBRGBUSE: break;
//        case NS_rtf::LN_FCUSP: break;
//        case NS_rtf::LN_LCBUSP: break;
//        case NS_rtf::LN_FCUSKF: break;
//        case NS_rtf::LN_LCBUSKF: break;
//        case NS_rtf::LN_FCPLCUPCRGBUSE: break;
//        case NS_rtf::LN_LCBPLCUPCRGBUSE: break;
//        case NS_rtf::LN_FCPLCUPCUSP: break;
//        case NS_rtf::LN_LCBPLCUPCUSP: break;
//        case NS_rtf::LN_FCSTTBGLSYSTYLE: break;
//        case NS_rtf::LN_LCBSTTBGLSYSTYLE: break;
//        case NS_rtf::LN_FCPLGOSL: break;
//        case NS_rtf::LN_LCBPLGOSL: break;
//        case NS_rtf::LN_FCPLCOCX: break;
//        case NS_rtf::LN_LCBPLCOCX: break;
//        case NS_rtf::LN_FCPLCFBTELVC: break;
//        case NS_rtf::LN_LCBPLCFBTELVC: break;
//        case NS_rtf::LN_DWLOWDATETIME: break;
//        case NS_rtf::LN_DWHIGHDATETIME: break;
//        case NS_rtf::LN_FCPLCFLVC: break;
//        case NS_rtf::LN_LCBPLCFLVC: break;
//        case NS_rtf::LN_FCPLCASUMY: break;
//        case NS_rtf::LN_LCBPLCASUMY: break;
//        case NS_rtf::LN_FCPLCFGRAM: break;
//        case NS_rtf::LN_LCBPLCFGRAM: break;
//        case NS_rtf::LN_FCSTTBLISTNAMES: break;
//        case NS_rtf::LN_LCBSTTBLISTNAMES: break;
//        case NS_rtf::LN_FCSTTBFUSSR: break;
//        case NS_rtf::LN_LCBSTTBFUSSR: break;
//        case NS_rtf::LN_FN: break;
//        case NS_rtf::LN_FCSEPX: break;
//        case NS_rtf::LN_FNMPR: break;
//        case NS_rtf::LN_FCMPR: break;
//        case NS_rtf::LN_ICOFORE: break;
//        case NS_rtf::LN_ICOBACK: break;
//        case NS_rtf::LN_IPAT: break;
//        case NS_rtf::LN_SHDFORECOLOR: break;
//        case NS_rtf::LN_SHDBACKCOLOR: break;
//        case NS_rtf::LN_SHDPATTERN: break;
//        case NS_rtf::LN_DPTLINEWIDTH: break;
//        case NS_rtf::LN_BRCTYPE: break;
//        case NS_rtf::LN_ICO: break;
//        case NS_rtf::LN_DPTSPACE: break;
//        case NS_rtf::LN_FSHADOW: break;
//        case NS_rtf::LN_FFRAME: break;
//        case NS_rtf::LN_UNUSED2_15: break;
//        case NS_rtf::LN_FFIRSTMERGED: break;
//        case NS_rtf::LN_FMERGED: break;
//        case NS_rtf::LN_FVERTICAL: break;
//        case NS_rtf::LN_FBACKWARD: break;
//        case NS_rtf::LN_FROTATEFONT: break;
//        case NS_rtf::LN_FVERTMERGE: break;
//        case NS_rtf::LN_FVERTRESTART: break;
//        case NS_rtf::LN_VERTALIGN: break;
//        case NS_rtf::LN_FUNUSED: break;
//        case NS_rtf::LN_WUNUSED: break;
//        case NS_rtf::LN_BRCTOP: break;
//        case NS_rtf::LN_BRCLEFT: break;
//        case NS_rtf::LN_BRCBOTTOM: break;
//        case NS_rtf::LN_BRCRIGHT: break;
//        case NS_rtf::LN_IBKL: break;
//        case NS_rtf::LN_ITCFIRST: break;
//        case NS_rtf::LN_FPUB: break;
//        case NS_rtf::LN_ITCLIM: break;
//        case NS_rtf::LN_FCOL: break;
//        case NS_rtf::LN_LINECOLOR: break;
//        case NS_rtf::LN_LINEWIDTH: break;
//        case NS_rtf::LN_LINETYPE: break;
//        case NS_rtf::LN_MM: break;
//        case NS_rtf::LN_XEXT: break;
//        case NS_rtf::LN_YEXT: break;
//        case NS_rtf::LN_HMF: break;
//        case NS_rtf::LN_LCB: break;
//        case NS_rtf::LN_CBHEADER: break;
//        case NS_rtf::LN_MFP: break;
//        case NS_rtf::LN_BM_RCWINMF: break;
//        case NS_rtf::LN_DXAGOAL: break;
//        case NS_rtf::LN_DYAGOAL: break;
//        case NS_rtf::LN_MX: break;
//        case NS_rtf::LN_MY: break;
//        case NS_rtf::LN_DXACROPLEFT: break;
//        case NS_rtf::LN_DYACROPTOP: break;
//        case NS_rtf::LN_DXACROPRIGHT: break;
//        case NS_rtf::LN_DYACROPBOTTOM: break;
//        case NS_rtf::LN_BRCL: break;
//        case NS_rtf::LN_FFRAMEEMPTY: break;
//        case NS_rtf::LN_FBITMAP: break;
//        case NS_rtf::LN_FDRAWHATCH: break;
//        case NS_rtf::LN_FERROR: break;
//        case NS_rtf::LN_BPP: break;
//        case NS_rtf::LN_DXAORIGIN: break;
//        case NS_rtf::LN_DYAORIGIN: break;
//        case NS_rtf::LN_CPROPS: break;
//        case NS_rtf::LN_LINEPROPSTOP: break;
//        case NS_rtf::LN_LINEPROPSLEFT: break;
//        case NS_rtf::LN_LINEPROPSBOTTOM: break;
//        case NS_rtf::LN_LINEPROPSRIGHT: break;
//        case NS_rtf::LN_LINEPROPSHORIZONTAL: break;
//        case NS_rtf::LN_LINEPROPSVERTICAL: break;
//        case NS_rtf::LN_headerr: break;
//        case NS_rtf::LN_footerr: break;
//        case NS_rtf::LN_endnote: break;
//        case NS_rtf::LN_BOOKMARKNAME: break;

//        case NS_rtf::LN_LISTLEVEL: break;
//        case NS_rtf::LN_LFO: break;
        case NS_rtf::LN_F:
        break;
        case NS_rtf::LN_ALTFONTNAME:
            /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
            m_pImpl->pCurrentEntry->sAlternativeFont = sValue;
        break;
        case NS_rtf::LN_XSZFFN:
            /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Font_name:
            /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
            m_pImpl->pCurrentEntry->sFontName = sValue;
        break;
//        case NS_rtf::LN_XSTZNAME: break;
//        case NS_rtf::LN_XSTZNAME1: break;
//        case NS_rtf::LN_UPXSTART: break;
//        case NS_rtf::LN_UPX: break;
//        case NS_rtf::LN_sed: break;
//        case NS_rtf::LN_picf: break;
//        case NS_rtf::LN_rgbrc: break;
//        case NS_rtf::LN_shd: break;
//        case NS_rtf::LN_cellShd: break;
//        case NS_rtf::LN_cellTopColor: break;
//        case NS_rtf::LN_cellLeftColor: break;
//        case NS_rtf::LN_cellBottomColor: break;
//        case NS_rtf::LN_cellRightColor: break;

//        case NS_rtf::LN_LISTTABLE: break;
//        case NS_rtf::LN_LFOTABLE: break;
//        case NS_rtf::LN_FONTTABLE: break;
//        case NS_rtf::LN_STYLESHEET: break;
        default:
        {
            //----> debug
            int nVal = val.getInt();
            ++nVal;
            //<---- debug
        }
    }
}
/*-- 19.06.2006 12:04:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void FontTable::sprm(Sprm& rSprm)
{
    OSL_ENSURE( m_pImpl->pCurrentEntry, "current entry has to be set here");
    if(!m_pImpl->pCurrentEntry)
        return ;
    sal_uInt32 nSprmId = rSprm.getId();
    (void)nSprmId;

    Value::Pointer_t pValue = rSprm.getValue();
    sal_Int32 nIntValue = pValue->getInt();
    (void)nIntValue;
    rtl::OUString sStringValue = pValue->getString();

    //printf ( "FontTable::sprm(0x%.4x, 0x%.4x) [%s]\n", (unsigned int)nSprmId, (unsigned int)nIntValue, ::rtl::OUStringToOString(sStringValue, RTL_TEXTENCODING_DONTKNOW).getStr());

/*    switch(nSprmId)
    {
    default:
        {
            OSL_ENSURE( false, "FontTable::sprm()"); //
            //Value::Pointer_t pValue_ = rSprm.getValue();
        }
    }*/
}

/*-- 19.06.2006 12:04:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void FontTable::entry(int /*pos*/, writerfilter::Reference<Properties>::Pointer_t ref)
{
    //create a new font entry
    OSL_ENSURE( !m_pImpl->pCurrentEntry, "current entry has to be NULL here");
    m_pImpl->pCurrentEntry.reset(new FontEntry);
    ref->resolve(*this);
    //append it to the table
    m_pImpl->aFontEntries.push_back( *m_pImpl->pCurrentEntry );
    m_pImpl->pCurrentEntry.reset();
}
/*-- 19.06.2006 12:04:34---------------------------------------------------

  -----------------------------------------------------------------------*/
void FontTable::startSectionGroup()
{
}
/*-- 19.06.2006 12:04:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void FontTable::endSectionGroup()
{
}
/*-- 19.06.2006 12:04:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void FontTable::startParagraphGroup()
{
}
/*-- 19.06.2006 12:04:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void FontTable::endParagraphGroup()
{
}
/*-- 19.06.2006 12:04:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void FontTable::startCharacterGroup()
{
}
/*-- 19.06.2006 12:04:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void FontTable::endCharacterGroup()
{
}
/*-- 19.06.2006 12:04:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void FontTable::text(const sal_uInt8*, size_t )
{
}
/*-- 19.06.2006 12:04:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void FontTable::utext(const sal_uInt8* , size_t)
{
}
/*-- 19.06.2006 12:04:37---------------------------------------------------

  -----------------------------------------------------------------------*/
void FontTable::props(writerfilter::Reference<Properties>::Pointer_t)
{
}
/*-- 19.06.2006 12:04:37---------------------------------------------------

  -----------------------------------------------------------------------*/
void FontTable::table(Id, writerfilter::Reference<Table>::Pointer_t)
{
}
/*-- 19.06.2006 12:04:38---------------------------------------------------

  -----------------------------------------------------------------------*/
void FontTable::substream(Id, ::writerfilter::Reference<Stream>::Pointer_t)
{
}
/*-- 19.06.2006 12:04:39---------------------------------------------------

  -----------------------------------------------------------------------*/
void FontTable::info(const string& )
{
}

void FontTable::startShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > )
{
}

void FontTable::endShape( )
{
}

/*-- 21.06.2006 11:21:38---------------------------------------------------

  -----------------------------------------------------------------------*/
const FontEntry::Pointer_t FontTable::getFontEntry(sal_uInt32 nIndex)
{
    FontEntry::Pointer_t pRet;
    if(m_pImpl->aFontEntries.size() > nIndex)
    {        
        pRet.reset(&m_pImpl->aFontEntries[nIndex]);
    }
    return pRet;
}
/*-- 21.06.2006 11:21:38---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_uInt32 FontTable::size()
{
    return m_pImpl->aFontEntries.size();
}

}//namespace dmapper
}//namespace writerfilter
