/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StyleSheetTable.cxx,v $
 *
 *  $Revision: 1.31 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:53:37 $
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
#include <StyleSheetTable.hxx>
#include <dmapper/DomainMapper.hxx>
#include <ConversionHelper.hxx>
#include <BorderHandler.hxx>
#include <doctok/resourceids.hxx>
#include <ooxml/resourceids.hxx>
#include <vector>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <map>
#include <stdio.h>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;
namespace writerfilter {
namespace dmapper
{

typedef ::std::map< ::rtl::OUString, ::rtl::OUString> StringPairMap_t;

/*-- 21.06.2006 07:34:44---------------------------------------------------

  -----------------------------------------------------------------------*/
StyleSheetEntry::StyleSheetEntry() :
        sStyleIdentifierI()
        ,sStyleIdentifierD()
        ,bIsDefaultStyle(false)
        ,bInvalidHeight(false)
        ,bHasUPE(false)
        ,nStyleTypeCode(STYLE_TYPE_UNKNOWN)
        ,sBaseStyleIdentifier()
        ,sNextStyleIdentifier()
        ,pProperties(new StyleSheetPropertyMap)
        {
        }

/*-- 06.02.2008 11:30:46---------------------------------------------------

  -----------------------------------------------------------------------*/
struct ListCharStylePropertyMap_t
{
    ::rtl::OUString         sCharStyleName;
    PropertyValueVector_t   aPropertyValues;

    ListCharStylePropertyMap_t(const ::rtl::OUString& rCharStyleName, const PropertyValueVector_t& rPropertyValues):
        sCharStyleName( rCharStyleName ),
        aPropertyValues( rPropertyValues )
        {}
};
typedef std::vector< ListCharStylePropertyMap_t > ListCharStylePropertyVector_t;
/*-- 19.06.2006 12:04:32---------------------------------------------------

  -----------------------------------------------------------------------*/
struct StyleSheetTable_Impl
{
    DomainMapper&                           m_rDMapper;
    uno::Reference< text::XTextDocument>    m_xTextDocument;
    uno::Reference< beans::XPropertySet>    m_xTextDefaults;
    std::vector< StyleSheetEntry >          m_aStyleSheetEntries;
    StyleSheetEntry                         *m_pCurrentEntry;
    PropertyMapPtr                          m_pDefaultParaProps, m_pDefaultCharProps;
    PropertyMapPtr                          m_pCurrentProps;
    StringPairMap_t                         m_aStyleNameMap;
    ListCharStylePropertyVector_t           m_aListCharStylePropertyVector;


    StyleSheetTable_Impl(DomainMapper& rDMapper, uno::Reference< text::XTextDocument> xTextDocument);

    ::rtl::OUString HasListCharStyle( const PropertyValueVector_t& rCharProperties );
};
/*-- 15.11.2007 08:30:02---------------------------------------------------

  -----------------------------------------------------------------------*/
StyleSheetTable_Impl::StyleSheetTable_Impl(DomainMapper& rDMapper, uno::Reference< text::XTextDocument> xTextDocument ) :
            m_rDMapper( rDMapper ),
            m_xTextDocument( xTextDocument ),
            m_pCurrentEntry(0),
            m_pDefaultParaProps(new PropertyMap),
            m_pDefaultCharProps(new PropertyMap)
{
    //set font height default to 10pt
    uno::Any aVal = uno::makeAny( double(10.) );
    m_pDefaultCharProps->Insert( PROP_CHAR_HEIGHT, true, aVal );
    m_pDefaultCharProps->Insert( PROP_CHAR_HEIGHT_ASIAN, true, aVal );
    m_pDefaultCharProps->Insert( PROP_CHAR_HEIGHT_COMPLEX, true, aVal );
}
/*-- 06.02.2008 11:45:21---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString StyleSheetTable_Impl::HasListCharStyle( const PropertyValueVector_t& rPropValues )
{
    ::rtl::OUString sRet;
    ListCharStylePropertyVector_t::const_iterator aListVectorIter = m_aListCharStylePropertyVector.begin();
    while( aListVectorIter != m_aListCharStylePropertyVector.end() )
    {
        //if size is identical
        if( aListVectorIter->aPropertyValues.size() == rPropValues.size() )
        {
            bool bBreak = false;
            //then search for all contained properties
            PropertyValueVector_t::const_iterator aList1Iter = rPropValues.begin();
            while( aList1Iter != rPropValues.end() && !bBreak)
            {
                //find the property
                bool bElementFound = false;
                PropertyValueVector_t::const_iterator aList2Iter = aListVectorIter->aPropertyValues.begin();
                while( aList2Iter != aListVectorIter->aPropertyValues.end() && !bBreak )
                {
                    if( aList2Iter->Name == aList1Iter->Name )
                    {
                        bElementFound = true;
                        if( aList2Iter->Value != aList1Iter->Value )
                            bBreak = true;
                        break;
                    }
                    ++aList2Iter;
                }
                //set break flag if property hasn't been found
                if(!bElementFound )
                {
                    bBreak = true;
                    break;
                }
                ++aList1Iter;
            }
            if( !bBreak )
                return aListVectorIter->sCharStyleName;
        }
        ++aListVectorIter;
    }
    return sRet;
}
/*-- 19.06.2006 12:04:32---------------------------------------------------

  -----------------------------------------------------------------------*/
StyleSheetTable::StyleSheetTable(DomainMapper& rDMapper, uno::Reference< text::XTextDocument> xTextDocument) :
    m_pImpl( new StyleSheetTable_Impl(rDMapper, xTextDocument) )
{
}
/*-- 19.06.2006 12:04:33---------------------------------------------------

  -----------------------------------------------------------------------*/
StyleSheetTable::~StyleSheetTable()
{
    delete m_pImpl;
}
/*-- 19.06.2006 12:04:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void StyleSheetTable::attribute(Id Name, Value & val)
{
    OSL_ENSURE( m_pImpl->m_pCurrentEntry, "current entry has to be set here");
    if(!m_pImpl->m_pCurrentEntry)
        return ;
    int nIntValue = val.getInt();
    (void)nIntValue;
    ::rtl::OUString sValue = val.getString();
//    printf ( "StyleSheetTable::attribute(0x%.4x, 0x%.4x) [%s]\n", (unsigned int)Name, (unsigned int)nIntValue, ::rtl::OUStringToOString(sValue, RTL_TEXTENCODING_DONTKNOW).getStr());
    /* WRITERFILTERSTATUS: table: StyleSheetTable_attributedata */
    switch(Name)
    {
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_ISTD:
            m_pImpl->m_pCurrentEntry->sStyleIdentifierD = ::rtl::OUString::valueOf(static_cast<sal_Int32>(nIntValue), 16);
        break;
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
//        case NS_rtf::LN_CBFFNM1: break;
//        case NS_rtf::LN_PRQ: break;
//        case NS_rtf::LN_FTRUETYPE: break;
//        case NS_rtf::LN_UNUSED1_3: break;
//        case NS_rtf::LN_FF: break;
//        case NS_rtf::LN_UNUSED1_7: break;
//        case NS_rtf::LN_WWEIGHT: break;
//        case NS_rtf::LN_CHS: break;
//        case NS_rtf::LN_IXCHSZALT: break;
//        case NS_rtf::LN_PANOSE: break;
//        case NS_rtf::LN_FS: break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_STI:
        {
            ::rtl::OUString tempStyleIdentifier = GetStyleIdFromIndex(static_cast<sal_uInt32>(nIntValue));
            if (tempStyleIdentifier.getLength())
                m_pImpl->m_pCurrentEntry->sStyleIdentifierI = tempStyleIdentifier;
            if (nIntValue == 0 || nIntValue == 65)
                m_pImpl->m_pCurrentEntry->bIsDefaultStyle = true;
        }
        break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_SGC:
            m_pImpl->m_pCurrentEntry->nStyleTypeCode = (StyleType)nIntValue;
        break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_ISTDBASE:
            if (static_cast<sal_uInt32>(nIntValue) != 0xfff)
                m_pImpl->m_pCurrentEntry->sBaseStyleIdentifier = ::rtl::OUString::valueOf(static_cast<sal_Int32>(nIntValue), 16);
        break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_ISTDNEXT:
            if (static_cast<sal_uInt32>(nIntValue) != 0xfff)
                m_pImpl->m_pCurrentEntry->sNextStyleIdentifier = ::rtl::OUString::valueOf(static_cast<sal_Int32>(nIntValue), 16);
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FSCRATCH:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FINVALHEIGHT:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FHASUPE:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FMASSCOPY:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_CUPX:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_BCHUPE:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FAUTOREDEF:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FHIDDEN:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_UNUSED8_3:
            //noone seems to care about it
        break;
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
//        case NS_rtf::LN_F: break;
//        case NS_rtf::LN_ALTFONTNAME: break;
//        case NS_rtf::LN_XSZFFN: break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_XSTZNAME:
            m_pImpl->m_pCurrentEntry->sStyleName1 = sValue;
            if (m_pImpl->m_pCurrentEntry->sStyleIdentifierI.getLength())
                m_pImpl->m_pCurrentEntry->sStyleIdentifierI = sValue;
        break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_XSTZNAME1:
            m_pImpl->m_pCurrentEntry->sStyleName = sValue;
            if (m_pImpl->m_pCurrentEntry->sStyleIdentifierI.getLength())
                m_pImpl->m_pCurrentEntry->sStyleIdentifierI = sValue;
        break;
//        case NS_rtf::LN_UPXSTART: break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_UPX:
            resolveAttributeProperties(val);
        break;
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
//        case NS_rtf::LN_StyleSheetTable: break;
//        case NS_rtf::LN_STYLESHEET: break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_type:
/*          defaults should be set at the service "com.sun.star.text.Defaults"
             if (nIntValue == 1)
                *(m_pImpl->m_pCurrentEntry->pProperties) = *(m_pImpl->m_pDefaultParaProps);
            else if (nIntValue == 2)
                *(m_pImpl->m_pCurrentEntry->pProperties) = *(m_pImpl->m_pDefaultCharProps);*/
            m_pImpl->m_pCurrentEntry->nStyleTypeCode = (StyleType)nIntValue;
        break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_default:
            m_pImpl->m_pCurrentEntry->bIsDefaultStyle = (nIntValue != 0);
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_customStyle:
        break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_styleId:
            m_pImpl->m_pCurrentEntry->sStyleIdentifierI = sValue;
            m_pImpl->m_pCurrentEntry->sStyleIdentifierD = sValue;
        break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_TblWidth_w:
            dynamic_cast< StyleSheetPropertyMap* >( m_pImpl->m_pCurrentEntry->pProperties.get() )->SetCT_TblWidth_w( nIntValue );
        break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_TblWidth_type:
            dynamic_cast< StyleSheetPropertyMap* >( m_pImpl->m_pCurrentEntry->pProperties.get() )->SetCT_TblWidth_type( nIntValue );
        break;
        default:
        {
            //----> debug
            int nVal = val.getInt();
            ++nVal;
            //<---- debug
        }
        break;
    }
}
/*-- 19.06.2006 12:04:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void StyleSheetTable::sprm(Sprm & rSprm)
{
    sal_uInt32 nSprmId = rSprm.getId();
    Value::Pointer_t pValue = rSprm.getValue();
    sal_Int32 nIntValue = pValue.get() ? pValue->getInt() : 0;
    (void)nIntValue;
    rtl::OUString sStringValue = pValue.get() ? pValue->getString() : rtl::OUString();
    //printf ( "StyleSheetTable::sprm(0x%.4x, 0x%.4x) [%s]\n", (unsigned int)nSprmId, (unsigned int)nIntValue, ::rtl::OUStringToOString(sStringValue, RTL_TEXTENCODING_DONTKNOW).getStr());

    /* WRITERFILTERSTATUS: table: StyleSheetTable_sprm */
    switch(nSprmId)
    {
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_name:
            //this is only a UI name!
            m_pImpl->m_pCurrentEntry->sStyleName = sStringValue;
            m_pImpl->m_pCurrentEntry->sStyleName1 = sStringValue;
            break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_basedOn:
            m_pImpl->m_pCurrentEntry->sBaseStyleIdentifier = sStringValue;
            break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_next:
            m_pImpl->m_pCurrentEntry->sNextStyleIdentifier = sStringValue;
            break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_aliases:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_link:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_autoRedefine:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_hidden:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_uiPriority:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_semiHidden:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_unhideWhenUsed:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_qFormat:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_locked:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_personal:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_personalCompose:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_personalReply:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_rsid:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_trPr:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_tcPr:
        break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_tblPr: //contains table properties
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_tblStylePr: //contains  to table properties
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_DocDefaults_pPrDefault:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_DocDefaults_rPrDefault:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_TblPrBase_tblInd: //table properties - at least width value and type
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_ooxml::LN_EG_RPrBase_rFonts: //table fonts
            resolveSprmProps(rSprm);
            break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_PPrDefault_pPr:
            m_pImpl->m_rDMapper.PushStyleSheetProperties( m_pImpl->m_pDefaultParaProps );
            m_pImpl->m_rDMapper.sprm( rSprm );
            m_pImpl->m_rDMapper.PopStyleSheetProperties();
            applyDefaults( true );
        break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_RPrDefault_rPr:
            m_pImpl->m_rDMapper.PushStyleSheetProperties( m_pImpl->m_pDefaultCharProps );
            m_pImpl->m_rDMapper.sprm( rSprm );
            m_pImpl->m_rDMapper.PopStyleSheetProperties();
            applyDefaults( false );
        break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_TblPrBase_jc:     //table alignment - row properties!
             m_pImpl->m_pCurrentEntry->pProperties->Insert( PROP_HORI_ORIENT, false,
                uno::makeAny( ConversionHelper::convertTableJustification( nIntValue )));
        break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_TrPrBase_jc:     //table alignment - row properties!
                dynamic_cast< StyleSheetPropertyMap* >( m_pImpl->m_pCurrentEntry->pProperties.get() )->SetCT_TrPrBase_jc(nIntValue);
        break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_TblPrBase_tblBorders: //table borders, might be defined in table style
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get())
            {
                BorderHandlerPtr pBorderHandler( new BorderHandler(m_pImpl->m_rDMapper.IsOOXMLImport()) );
                pProperties->resolve(*pBorderHandler);
                m_pImpl->m_pCurrentEntry->pProperties->insert( pBorderHandler->getProperties(), true );
            }
        }
        break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_pPr:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Style_rPr:
        default:
            if (!m_pImpl->m_pCurrentEntry)
                break;
            m_pImpl->m_rDMapper.sprm( rSprm, m_pImpl->m_pCurrentEntry->pProperties );
    }
}
/*-- 19.06.2006 12:04:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void StyleSheetTable::entry(int /*pos*/, writerfilter::Reference<Properties>::Pointer_t ref)
{
    //create a new style entry
    // printf("StyleSheetTable::entry(...)\n");
    OSL_ENSURE( !m_pImpl->m_pCurrentEntry, "current entry has to be NULL here");
    m_pImpl->m_pCurrentEntry = new StyleSheetEntry;
    m_pImpl->m_rDMapper.PushStyleSheetProperties( m_pImpl->m_pCurrentEntry->pProperties );
    ref->resolve(*this);
    //append it to the table
    m_pImpl->m_rDMapper.PopStyleSheetProperties();
    if( !m_pImpl->m_rDMapper.IsOOXMLImport() || m_pImpl->m_pCurrentEntry->sStyleName.getLength() >0)
    {
        m_pImpl->m_pCurrentEntry->sConvertedStyleName = ConvertStyleName( m_pImpl->m_pCurrentEntry->sStyleName );
        m_pImpl->m_aStyleSheetEntries.push_back( *m_pImpl->m_pCurrentEntry );
    }
    else
    {
        //TODO: this entry contains the default settings - they have to be added to the settings
    }
    m_pImpl->m_pCurrentEntry = 0;
}
/*-- 21.06.2006 15:34:49---------------------------------------------------
    sorting helper
  -----------------------------------------------------------------------*/
typedef std::vector< beans::PropertyValue > _PropValVector;
class PropValVector : public _PropValVector
{
public:
    PropValVector(){}

    void    Insert( beans::PropertyValue aVal );
    uno::Sequence< uno::Any > getValues();
    uno::Sequence< ::rtl::OUString > getNames();
};
void    PropValVector::Insert( beans::PropertyValue aVal )
{
    _PropValVector::iterator aIt = begin();
    while(aIt != end())
    {
        if(aIt->Name > aVal.Name)
        {
            insert( aIt, aVal );
            return;
        }
        ++aIt;
    }
    push_back( aVal );
}
uno::Sequence< uno::Any > PropValVector::getValues()
{
    uno::Sequence< uno::Any > aRet( size() );
    uno::Any* pValues = aRet.getArray();
    sal_Int32 nVal = 0;
    _PropValVector::iterator aIt = begin();
    while(aIt != end())
    {
        pValues[nVal++] = aIt->Value;
        ++aIt;
    }
    return aRet;
}
uno::Sequence< ::rtl::OUString > PropValVector::getNames()
{
    uno::Sequence< ::rtl::OUString > aRet( size() );
    ::rtl::OUString* pNames = aRet.getArray();
    sal_Int32 nVal = 0;
    _PropValVector::iterator aIt = begin();
    while(aIt != end())
    {
        pNames[nVal++] = aIt->Name;
        ++aIt;
    }
    return aRet;
}
/*-- 21.06.2006 13:35:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void StyleSheetTable::ApplyStyleSheets( FontTablePtr rFontTable )
{
    try
    {
        uno::Reference< style::XStyleFamiliesSupplier > xStylesSupplier( m_pImpl->m_xTextDocument, uno::UNO_QUERY_THROW );
        uno::Reference< lang::XMultiServiceFactory > xDocFactory( m_pImpl->m_xTextDocument, uno::UNO_QUERY_THROW );
        uno::Reference< container::XNameAccess > xStyleFamilies = xStylesSupplier->getStyleFamilies();
        uno::Reference<container::XNameContainer> xCharStyles;
        uno::Reference<container::XNameContainer> xParaStyles;

        PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
        xStyleFamilies->getByName(rPropNameSupplier.GetName( PROP_CHARACTER_STYLES )) >>= xCharStyles;
        xStyleFamilies->getByName(rPropNameSupplier.GetName( PROP_PARAGRAPH_STYLES )) >>= xParaStyles;
        if(xCharStyles.is() && xParaStyles.is())
        {
            std::vector< StyleSheetEntry >::iterator aIt = m_pImpl->m_aStyleSheetEntries.begin();
            while( aIt != m_pImpl->m_aStyleSheetEntries.end() )
            {
                bool bParaStyle = aIt->nStyleTypeCode == STYLE_TYPE_PARA;
                bool bInsert = false;
                uno::Reference< container::XNameContainer > xStyles = bParaStyle ? xParaStyles : xCharStyles;
                uno::Reference< style::XStyle > xStyle;
                ::rtl::OUString sConvertedStyleName = ConvertStyleName( aIt->sStyleName );
                if(xStyles->hasByName( sConvertedStyleName ))
                    xStyles->getByName( sConvertedStyleName ) >>= xStyle;
                else
                {
                    bInsert = true;
                    xStyle = uno::Reference< style::XStyle >(xDocFactory->createInstance(
                                bParaStyle ?
                                    rPropNameSupplier.GetName( PROP_SERVICE_PARA_STYLE ) :
                                    rPropNameSupplier.GetName( PROP_SERVICE_CHAR_STYLE )),
                                    uno::UNO_QUERY_THROW);
                }
                if( aIt->sBaseStyleIdentifier.getLength() )
                {
                    try
                    {
                        //TODO: Handle cases where a paragraph <> character style relation is needed
                        xStyle->setParentStyle(ConvertStyleName( aIt->sBaseStyleIdentifier ));
                    }
                    catch( const uno::RuntimeException& )
                    {
                        OSL_ENSURE( false, "Styles parent could not be set");
                    }
                }
                else if( bParaStyle )
                {
                    //now it's time to set the default parameters - for paragraph styles
                    //Fonts: Western first entry in font table
                    //CJK: second entry
                    //CTL: third entry, if it exists

                    sal_uInt32 nFontCount = rFontTable->size();
                    if( !m_pImpl->m_rDMapper.IsOOXMLImport() && nFontCount > 2 )
                    {
                        uno::Any aTwoHundredFortyTwip = uno::makeAny(12.);
//                      font size to 240 twip (12 pts) for all if not set
                        aIt->pProperties->Insert(PROP_CHAR_HEIGHT, true, aTwoHundredFortyTwip, false);
//                      western font not already set -> apply first font
                        const FontEntry* pWesternFontEntry = rFontTable->getFontEntry( 0 );
                        rtl::OUString sWesternFontName = pWesternFontEntry->sFontName;
                        aIt->pProperties->Insert(PROP_CHAR_FONT_NAME, true, uno::makeAny( sWesternFontName ), false);

//                      CJK  ... apply second font
                        const FontEntry* pCJKFontEntry  = rFontTable->getFontEntry( 2 );
                        aIt->pProperties->Insert(PROP_CHAR_FONT_NAME_ASIAN, true, uno::makeAny( pCJKFontEntry->sFontName ), false);
                        aIt->pProperties->Insert(PROP_CHAR_HEIGHT_ASIAN, true, aTwoHundredFortyTwip, false);
//                      CTL  ... apply third font, if available
                        if( nFontCount > 3 )
                        {
                            const FontEntry* pCTLFontEntry  = rFontTable->getFontEntry( 3 );
                            aIt->pProperties->Insert(PROP_CHAR_FONT_NAME_COMPLEX, true, uno::makeAny( pCTLFontEntry->sFontName ), false);
                            aIt->pProperties->Insert(PROP_CHAR_HEIGHT_COMPLEX, true, aTwoHundredFortyTwip, false);
                        }
                    }
//                  Widow/Orphan -> set both to two if not already set
                    uno::Any aTwo = uno::makeAny(sal_Int8(2));
                    aIt->pProperties->Insert(PROP_PARA_WIDOWS, true, aTwo, false);
                    aIt->pProperties->Insert(PROP_PARA_ORPHANS, true, aTwo, false);
//                  Left-to-right direction if not already set
                    aIt->pProperties->Insert(PROP_WRITING_MODE, true, uno::makeAny( sal_Int16(text::WritingMode_LR_TB) ), false);
//                  font color COL_AUTO if not already set
                    aIt->pProperties->Insert(PROP_CHAR_COLOR, true, uno::makeAny( sal_Int32(0xffffffff) ), false);
                }

                uno::Sequence< beans::PropertyValue > aPropValues = aIt->pProperties->GetPropertyValues();
                bool bAddFollowStyle = false;
                if(bParaStyle && !aIt->sNextStyleIdentifier.getLength() )
                {
                        bAddFollowStyle = true;
                }
                //remove Left/RightMargin values from TOX heading styles
                if( bParaStyle )
                {
                    uno::Reference< beans::XPropertyState >xState( xStyle, uno::UNO_QUERY_THROW );
                    if( sConvertedStyleName.equalsAscii( "Contents Heading" ) ||
                        sConvertedStyleName.equalsAscii( "User Index Heading" ) ||
                        sConvertedStyleName.equalsAscii( "Index Heading" ))
                    {
                        //left margin is set to NULL by default
                        uno::Reference< beans::XPropertyState >xState1( xStyle, uno::UNO_QUERY_THROW );
                        xState1->setPropertyToDefault(rPropNameSupplier.GetName( PROP_PARA_LEFT_MARGIN ));
                    }
                    else if( sConvertedStyleName.equalsAscii( "Text body" ) )
                        xState->setPropertyToDefault(rPropNameSupplier.GetName( PROP_PARA_BOTTOM_MARGIN ));
                    else if( sConvertedStyleName.equalsAscii( "Heading 1" ) ||
                            sConvertedStyleName.equalsAscii( "Heading 2" ) ||
                            sConvertedStyleName.equalsAscii( "Heading 3" ) ||
                            sConvertedStyleName.equalsAscii( "Heading 4" ) ||
                            sConvertedStyleName.equalsAscii( "Heading 5" ) ||
                            sConvertedStyleName.equalsAscii( "Heading 6" ) ||
                            sConvertedStyleName.equalsAscii( "Heading 7" ) ||
                            sConvertedStyleName.equalsAscii( "Heading 8" ) ||
                            sConvertedStyleName.equalsAscii( "Heading 9" ) )
                    {
                        xState->setPropertyToDefault(rPropNameSupplier.GetName( PROP_CHAR_WEIGHT ));
                        xState->setPropertyToDefault(rPropNameSupplier.GetName( PROP_CHAR_WEIGHT_ASIAN ));
                        xState->setPropertyToDefault(rPropNameSupplier.GetName( PROP_CHAR_WEIGHT_COMPLEX ));
                        xState->setPropertyToDefault(rPropNameSupplier.GetName( PROP_CHAR_POSTURE ));
                        xState->setPropertyToDefault(rPropNameSupplier.GetName( PROP_CHAR_POSTURE_ASIAN ));
                        xState->setPropertyToDefault(rPropNameSupplier.GetName( PROP_CHAR_POSTURE_COMPLEX ));
                    }
                }

                if(bAddFollowStyle || aPropValues.getLength())
                {
                    const beans::PropertyValue* pPropValues = aPropValues.getConstArray();
                    PropValVector aSortedPropVals;
                    for( sal_Int32 nProp = 0; nProp < aPropValues.getLength(); ++nProp)
                    {
                        aSortedPropVals.Insert( pPropValues[nProp] );
                    }
                    if(bAddFollowStyle)
                    {
                        //find the name of the Next style
                        std::vector< StyleSheetEntry >::iterator aNextStyleIt = m_pImpl->m_aStyleSheetEntries.begin();
                        for( ; aNextStyleIt !=  m_pImpl->m_aStyleSheetEntries.end(); ++aNextStyleIt )
                        {
                            if( aNextStyleIt->sStyleName.getLength() &&
                                    aNextStyleIt->sStyleName == aIt->sNextStyleIdentifier)
                            {
                                beans::PropertyValue aNew;
                                aNew.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FollowStyle"));
                                aNew.Value = uno::makeAny(ConvertStyleName( aNextStyleIt->sStyleIdentifierD ));
                                aSortedPropVals.Insert( aNew );
                                break;
                            }
                        }
                    }

                    try
                    {
                        uno::Reference< beans::XMultiPropertySet > xMultiPropertySet( xStyle, uno::UNO_QUERY_THROW);
                        xMultiPropertySet->setPropertyValues( aSortedPropVals.getNames(), aSortedPropVals.getValues() );
                    }
                    catch( const beans::UnknownPropertyException& rUnknown)
                    {
                        (void) rUnknown;
                        OSL_ENSURE( false, "Some style properties could not be set");
                    }
                }
                if(bInsert)
                    xStyles->insertByName( sConvertedStyleName, uno::makeAny( xStyle) );
                ++aIt;
            }
        }
    }
    catch( uno::Exception& rEx)
    {
        (void)rEx;
        OSL_ENSURE( false, "Styles could not be imported completely");
    }
}
/*-- 22.06.2006 15:56:56---------------------------------------------------

  -----------------------------------------------------------------------*/
const StyleSheetEntry* StyleSheetTable::FindStyleSheetByISTD(const ::rtl::OUString& sIndex)
{
    const StyleSheetEntry* pRet = 0;
    for( sal_uInt32 nPos = 0; nPos < m_pImpl->m_aStyleSheetEntries.size(); ++nPos )
    {
        if( m_pImpl->m_aStyleSheetEntries[nPos].sStyleIdentifierD == sIndex)
        {
            pRet = &m_pImpl->m_aStyleSheetEntries[nPos];
            break;
        }
    }
    return pRet;
}
/*-- 28.12.2007 14:45:45---------------------------------------------------

  -----------------------------------------------------------------------*/
const StyleSheetEntry* StyleSheetTable::FindStyleSheetByStyleName(const ::rtl::OUString& sIndex)
{
    const StyleSheetEntry* pRet = 0;
    for( sal_uInt32 nPos = 0; nPos < m_pImpl->m_aStyleSheetEntries.size(); ++nPos )
    {
        if( m_pImpl->m_aStyleSheetEntries[nPos].sStyleName == sIndex)
        {
            pRet = &m_pImpl->m_aStyleSheetEntries[nPos];
            break;
        }
    }
    return pRet;
}
/*-- 28.12.2007 14:45:45---------------------------------------------------

  -----------------------------------------------------------------------*/
const StyleSheetEntry* StyleSheetTable::FindStyleSheetByConvertedStyleName(const ::rtl::OUString& sIndex)
{
    const StyleSheetEntry* pRet = 0;
    for( sal_uInt32 nPos = 0; nPos < m_pImpl->m_aStyleSheetEntries.size(); ++nPos )
    {
        if( m_pImpl->m_aStyleSheetEntries[nPos].sConvertedStyleName == sIndex)
        {
            pRet = &m_pImpl->m_aStyleSheetEntries[nPos];
            break;
        }
    }
    return pRet;
}

/*-- 17.07.2006 11:47:00---------------------------------------------------

  -----------------------------------------------------------------------*/
const StyleSheetEntry* StyleSheetTable::FindParentStyleSheet(::rtl::OUString sBaseStyle)
{
    if( !sBaseStyle.getLength() )
        return 0;
    if( m_pImpl->m_pCurrentEntry)
        sBaseStyle = m_pImpl->m_pCurrentEntry->sBaseStyleIdentifier;

    const StyleSheetEntry* pRet = 0;
    for( sal_uInt32 nPos = 0; nPos < m_pImpl->m_aStyleSheetEntries.size(); ++nPos )
    {
        if( m_pImpl->m_aStyleSheetEntries[nPos].sStyleIdentifierD == sBaseStyle )
        {
            pRet = &m_pImpl->m_aStyleSheetEntries[nPos];
            break;
        }
    }
    return pRet;
}
/*-- 21.12.2006 15:58:23---------------------------------------------------

  -----------------------------------------------------------------------*/
static const sal_Char *aStyleNamePairs[] =
{
    "Normal",                     "Standard",
    "heading 1",                  "Heading 1",
    "heading 2",                  "Heading 2",
    "heading 3",                  "Heading 3",
    "heading 4",                  "Heading 4",
    "heading 5",                  "Heading 5",
    "heading 6",                  "Heading 6",
    "heading 7",                  "Heading 7",
    "heading 8",                  "Heading 8",
    "heading 9",                  "Heading 9",
    "Heading1",                   "Heading 1",
    "Heading2",                   "Heading 2",
    "Heading3",                   "Heading 3",
    "Heading4",                   "Heading 4",
    "Heading5",                   "Heading 5",
    "Heading6",                   "Heading 6",
    "Heading7",                   "Heading 7",
    "Heading8",                   "Heading 8",
    "Heading9",                   "Heading 9",
    "Heading 1",                  "Heading 1",
    "Heading 2",                  "Heading 2",
    "Heading 3",                  "Heading 3",
    "Heading 4",                  "Heading 4",
    "Heading 5",                  "Heading 5",
    "Heading 6",                  "Heading 6",
    "Heading 7",                  "Heading 7",
    "Heading 8",                  "Heading 8",
    "Heading 9",                  "Heading 9",
    "Index 1",                   "Index 1",
    "Index 2",                   "Index 2",
    "Index 3",                   "Index 3",
    "Index 4",                   0,
    "Index 5",                   0,
    "Index 6",                   0,
    "Index 7",                   0,
    "Index 8",                   0,
    "Index 9",                   0,
    "TOC 1",                     "Contents 1",
    "TOC 2",                     "Contents 2",
    "TOC 3",                     "Contents 3",
    "TOC 4",                     "Contents 4",
    "TOC 5",                     "Contents 5",
    "TOC 6",                     "Contents 6",
    "TOC 7",                     "Contents 7",
    "TOC 8",                     "Contents 8",
    "TOC 9",                     "Contents 9",
    "TOC Heading",               "Contents Heading",
    "TOCHeading",                "Contents Heading",
    "toc 1",                     "Contents 1",
    "toc 2",                     "Contents 2",
    "toc 3",                     "Contents 3",
    "toc 4",                     "Contents 4",
    "toc 5",                     "Contents 5",
    "toc 6",                     "Contents 6",
    "toc 7",                     "Contents 7",
    "toc 8",                     "Contents 8",
    "toc 9",                     "Contents 9",
    "TOC1",                     "Contents 1",
    "TOC2",                     "Contents 2",
    "TOC3",                     "Contents 3",
    "TOC4",                     "Contents 4",
    "TOC5",                     "Contents 5",
    "TOC6",                     "Contents 6",
    "TOC7",                     "Contents 7",
    "TOC8",                     "Contents 8",
    "TOC9",                     "Contents 9",
    "Normal Indent",             0,
    "Footnote Text",             "Footnote",
    "Annotation Text",           0,
    "Header",                    "Header",
    "header",                    "Header",
    "Footer",                    "Footer",
    "footer",                    "Footer",
    "Index Heading",             "Index Heading",
    "Caption",                   0,
    "Table of Figures",          0,
    "Envelope Address",          "Addressee",
    "Envelope Return",           "Sender",
    "Footnote Reference",        "Footnote anchor",
    "Annotation Reference",      0,
    "Line Number",               "Line numbering",
    "Page Number",               "Page Number",
    "Endnote Reference",         "Endnote anchor",
    "Endnote Text",              "Endnote Symbol",
    "Table of Authorities",      0,
    "Macro Text",                0,
    "TOA Heading",               0,
    "List",                      "List",
    "List 2",                    0,
    "List 3",                    0,
    "List 4",                    0,
    "List 5",                    0,
    "List Bullet",               0,
    "List Bullet 2",             0,
    "List Bullet 3",             0,
    "List Bullet 4",             0,
    "List Bullet 5",             0,
    "List Number",               0,
    "List Number 2",             0,
    "List Number 3",             0,
    "List Number 4",             0,
    "List Number 5",             0,
    "Title",                     "Title",
    "Closing",                   0,
    "Signature",                 "Signature",
    "Default Paragraph Font",    0,
    "DefaultParagraphFont",      "Default Paragraph Font",
    "Body Text",                 "Text body",
    "BodyText",                  "Text body",
    "BodyTextIndentItalic",     "Text body indent italic",
    "Body Text Indent",          "Text body indent",
    "BodyTextIndent",           "Text body indent",
    "BodyTextIndent2",          "Text body indent2",
    "List Continue",             0,
    "List Continue 2",           0,
    "List Continue 3",           0,
    "List Continue 4",           0,
    "List Continue 5",           0,
    "Message Header",            0,
    "Subtitle",                  "Subtitle",
    "Salutation",                0,
    "Date",                      0,
    "Body Text First Indent",    "Body Text Indent",
    "Body Text First Indent 2",  0,
    "Note Heading",              0,
    "Body Text 2",               0,
    "Body Text 3",               0,
    "Body Text Indent 2",        0,
    "Body Text Indent 3",        0,
    "Block Text",                0,
    "Hyperlink",                 "Internet link",
    "Followed Hyperlink",        "Visited Internet Link",
    "Strong",                    "Strong Emphasis",
    "Emphasis",                  "Emphasis",
    "Document Map",              0,
    "Plain Text",                0,
    "NoList",                   "No List",
    "AbstractHeading",          "Abstract Heading",
    "AbstractBody",             "Abstract Body",
    "PageNumber",               "page number"
    "TableNormal",              "Normal Table",
    "DocumentMap",              "Document Map"
};


::rtl::OUString StyleSheetTable::ConvertStyleName( const ::rtl::OUString& rWWName, bool bExtendedSearch)
{
    ::rtl::OUString sRet( rWWName );
    if( bExtendedSearch )
    {
        //search for the rWWName in the IdentifierD of the existing styles and convert the sStyleName member
        std::vector< StyleSheetEntry >::iterator aIt = m_pImpl->m_aStyleSheetEntries.begin();
        //TODO: performance issue - put styles list into a map sorted by it's sStyleIdentifierD members
        while( aIt != m_pImpl->m_aStyleSheetEntries.end() )
        {
            if( rWWName == aIt->sStyleIdentifierD )
                sRet = aIt->sStyleName;
            ++aIt;
        }
    }
    if(!m_pImpl->m_aStyleNameMap.size())
    {
        for( sal_uInt32 nPair = 0; nPair < sizeof(aStyleNamePairs) / sizeof( sal_Char*) / 2; ++nPair)
        {
                m_pImpl->m_aStyleNameMap.insert( StringPairMap_t::value_type(
                    ::rtl::OUString::createFromAscii(aStyleNamePairs[2 * nPair]),
                    ::rtl::OUString::createFromAscii(aStyleNamePairs[2 * nPair + 1]) ));
        }
    }
    StringPairMap_t::iterator aIt = m_pImpl->m_aStyleNameMap.find( sRet );
    if(aIt != m_pImpl->m_aStyleNameMap.end() && aIt->second.getLength())
        sRet = aIt->second;
    return sRet;
}

::rtl::OUString StyleSheetTable::GetStyleIdFromIndex(const sal_uInt32 sti)
{
    ::rtl::OUString sRet;
    if (sti >= (sizeof(aStyleNamePairs) / sizeof( sal_Char*) / 2))
        sRet = ::rtl::OUString();
    else
        sRet = ::rtl::OUString::createFromAscii(aStyleNamePairs[2 * sti]);
    return sRet;
}

void StyleSheetTable::resolveSprmProps(Sprm & rSprm)
{
    writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
    if( pProperties.get())
        pProperties->resolve(*this);
}

void StyleSheetTable::resolveAttributeProperties(Value & val)
{
    writerfilter::Reference<Properties>::Pointer_t pProperties = val.getProperties();
    if( pProperties.get())
        pProperties->resolve(*this);
}
/*-- 18.07.2007 15:59:34---------------------------------------------------

  -----------------------------------------------------------------------*/
void StyleSheetTable::applyDefaults(bool bParaProperties)
{
    try{
        if(!m_pImpl->m_xTextDefaults.is())
        {
            m_pImpl->m_xTextDefaults = uno::Reference< beans::XPropertySet>(
                m_pImpl->m_rDMapper.GetTextFactory()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.Defaults"))),
                uno::UNO_QUERY_THROW );
        }
        PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
        if( bParaProperties && m_pImpl->m_pDefaultParaProps.get() && m_pImpl->m_pDefaultParaProps->size())
        {
            PropertyMap::iterator aMapIter = m_pImpl->m_pDefaultParaProps->begin();
            for( ; aMapIter != m_pImpl->m_pDefaultParaProps->end(); ++aMapIter )
            {
                try
                {
                    m_pImpl->m_xTextDefaults->setPropertyValue(rPropNameSupplier.GetName( aMapIter->first.eId ), aMapIter->second);
                }
                catch( const uno::Exception& )
                {
                    OSL_ENSURE( false, "setPropertyValue exception");
                }
            }
        }
        if( !bParaProperties && m_pImpl->m_pDefaultCharProps.get() && m_pImpl->m_pDefaultCharProps->size())
        {
            PropertyMap::iterator aMapIter = m_pImpl->m_pDefaultCharProps->begin();
            for( ; aMapIter != m_pImpl->m_pDefaultCharProps->end(); ++aMapIter )
            {
                try
                {
                    m_pImpl->m_xTextDefaults->setPropertyValue(rPropNameSupplier.GetName( aMapIter->first.eId ), aMapIter->second);
                }
                catch( const uno::Exception& )
                {
                    OSL_ENSURE( false, "setPropertyValue exception");
                }
            }
        }
    }
    catch( const uno::Exception& )
    {
    }
}
/*-- 05.02.2008 10:27:36---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString StyleSheetTable::getOrCreateCharStyle( PropertyValueVector_t& rCharProperties )
{
    //find out if any of the styles already has the required properties then return it's name
    ::rtl::OUString sListLabel = m_pImpl->HasListCharStyle(rCharProperties);
    if( sListLabel.getLength() )
        return sListLabel;
    const char cListLabel[] = "ListLabel ";
    uno::Reference< style::XStyleFamiliesSupplier > xStylesSupplier( m_pImpl->m_xTextDocument, uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xStyleFamilies = xStylesSupplier->getStyleFamilies();
    uno::Reference<container::XNameContainer> xCharStyles;
    xStyleFamilies->getByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharacterStyles"))) >>= xCharStyles;
    //search for all character styles with the name sListLabel + <index>
    sal_Int32 nStyleFound = 0;
    uno::Sequence< ::rtl::OUString > aStyleNames = xCharStyles->getElementNames();
    const ::rtl::OUString* pStyleNames = aStyleNames.getConstArray();
    for( sal_Int32 nStyle = 0; nStyle < aStyleNames.getLength(); ++nStyle )
    {
        if( pStyleNames[nStyle].matchAsciiL( cListLabel, sizeof( cListLabel ) - 1  ))
        {
            ::rtl::OUString sSuffix = pStyleNames[nStyle].copy( sizeof( cListLabel ) - 1 );
            sal_Int32 nSuffix = sSuffix.toInt32();
            if( nSuffix > 0 )
            {
                if( nSuffix > nStyleFound )
                    nStyleFound = nSuffix;
            }
        }
    }
    sListLabel = ::rtl::OUString::createFromAscii( cListLabel );
    sListLabel += ::rtl::OUString::valueOf( ++nStyleFound );
    //create a new one otherwise
    uno::Reference< lang::XMultiServiceFactory > xDocFactory( m_pImpl->m_xTextDocument, uno::UNO_QUERY_THROW );
    PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
    try
    {
        uno::Reference< style::XStyle > xStyle( xDocFactory->createInstance(
            rPropNameSupplier.GetName( PROP_SERVICE_CHAR_STYLE )), uno::UNO_QUERY_THROW);
        //uno::Reference< container::XNamed >xNamed( xStyle, uno::UNO_QUERY_THROW );
        //xNamed->setName( sListLabel );
        uno::Reference< beans::XPropertySet > xStyleProps(xStyle, uno::UNO_QUERY_THROW );
        PropertyValueVector_t::const_iterator aCharPropIter = rCharProperties.begin();
        while( aCharPropIter != rCharProperties.end())
        {
            try
            {
                xStyleProps->setPropertyValue( aCharPropIter->Name, aCharPropIter->Value );
            }
            catch( const uno::Exception& rEx )
            {
                (void)rEx;
                OSL_ENSURE( false, "Exception in StyleSheetTable::getOrCreateCharStyle - Style::setPropertyValue");
            }
            ++aCharPropIter;
        }
        xCharStyles->insertByName( sListLabel, uno::makeAny( xStyle) );
        m_pImpl->m_aListCharStylePropertyVector.push_back( ListCharStylePropertyMap_t( sListLabel, rCharProperties ));
    }
    catch( const uno::Exception& rEx )
    {
        (void)rEx;
        OSL_ENSURE( false, "Exception in StyleSheetTable::getOrCreateCharStyle");
    }

    return sListLabel;
}

}//namespace dmapper
}//namespace writerfilter
