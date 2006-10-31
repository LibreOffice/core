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
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
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
DomainMapper::DomainMapper(uno::Reference< lang::XComponent > xModel) :
    m_pImpl( new DomainMapper_Impl( *this, xModel ))
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
        switch( Name )
        {
            case NS_rtf::LN_ISTD: //index of applied style
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
    //        case NS_rtf::LN_ISTARTAT: break;
    //        case NS_rtf::LN_NFC: break;
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
            case NS_rtf::LN_CHS:
            {
                m_pImpl->GetFIB().SetLNCHS( nIntValue );
            }
            break;
            break;
    //        case NS_rtf::LN_IXCHSZALT: break;
    //        case NS_rtf::LN_PANOSE: break;
    //        case NS_rtf::LN_FS: break;
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
    /*        case NS_rtf::LN_WIDENT:             case NS_rtf::LN_NFIB:            case NS_rtf::LN_NPRODUCT:            case NS_rtf::LN_LID:            case NS_rtf::LN_PNNEXT:            case NS_rtf::LN_FDOT:            case NS_rtf::LN_FGLSY:            case NS_rtf::LN_FCOMPLEX:            case NS_rtf::LN_FHASPIC:
            case NS_rtf::LN_CQUICKSAVES:            case NS_rtf::LN_FENCRYPTED:            case NS_rtf::LN_FWHICHTBLSTM:            case NS_rtf::LN_FREADONLYRECOMMENDED:            case NS_rtf::LN_FWRITERESERVATION:            case NS_rtf::LN_FEXTCHAR:            case NS_rtf::LN_FLOADOVERRIDE:            case NS_rtf::LN_FFAREAST:
            case NS_rtf::LN_FCRYPTO:            case NS_rtf::LN_NFIBBACK:            case NS_rtf::LN_LKEY:            case NS_rtf::LN_ENVR:            case NS_rtf::LN_FMAC:            case NS_rtf::LN_FEMPTYSPECIAL:            case NS_rtf::LN_FLOADOVERRIDEPAGE:            case NS_rtf::LN_FFUTURESAVEDUNDO:
            case NS_rtf::LN_FWORD97SAVED:             case NS_rtf::LN_FSPARE0:            case NS_rtf::LN_CHSTABLES:            case NS_rtf::LN_FCMIN:            case NS_rtf::LN_FCMAC:            case NS_rtf::LN_CSW:            case NS_rtf::LN_WMAGICCREATED:            case NS_rtf::LN_WMAGICREVISED:            case NS_rtf::LN_WMAGICCREATEDPRIVATE:
            case NS_rtf::LN_WMAGICREVISEDPRIVATE:            case NS_rtf::LN_PNFBPCHPFIRST_W6:            case NS_rtf::LN_PNCHPFIRST_W6:            case NS_rtf::LN_CPNBTECHP_W6:            case NS_rtf::LN_PNFBPPAPFIRST_W6:            case NS_rtf::LN_PNPAPFIRST_W6:            case NS_rtf::LN_CPNBTEPAP_W6:            case NS_rtf::LN_PNFBPLVCFIRST_W6:
            case NS_rtf::LN_PNLVCFIRST_W6:            case NS_rtf::LN_CPNBTELVC_W6:            case NS_rtf::LN_LIDFE:            case NS_rtf::LN_CLW:            case NS_rtf::LN_CBMAC:            case NS_rtf::LN_LPRODUCTCREATED:            case NS_rtf::LN_LPRODUCTREVISED:            case NS_rtf::LN_CCPTEXT:
            case NS_rtf::LN_CCPFTN:            case NS_rtf::LN_CCPHDD:            case NS_rtf::LN_CCPMCR:            case NS_rtf::LN_CCPATN:            case NS_rtf::LN_CCPEDN:            case NS_rtf::LN_CCPTXBX:            case NS_rtf::LN_CCPHDRTXBX:            case NS_rtf::LN_PNFBPCHPFIRST:            case NS_rtf::LN_PNCHPFIRST:            case NS_rtf::LN_CPNBTECHP:
            case NS_rtf::LN_PNFBPPAPFIRST:            case NS_rtf::LN_PNPAPFIRST:            case NS_rtf::LN_CPNBTEPAP:            case NS_rtf::LN_PNFBPLVCFIRST:            case NS_rtf::LN_PNLVCFIRST:            case NS_rtf::LN_CPNBTELVC:            case NS_rtf::LN_FCISLANDFIRST:            case NS_rtf::LN_FCISLANDLIM:            case NS_rtf::LN_CFCLCB:            case NS_rtf::LN_FCSTSHFORIG:
            case NS_rtf::LN_LCBSTSHFORIG:            case NS_rtf::LN_FCSTSHF:            case NS_rtf::LN_LCBSTSHF:            case NS_rtf::LN_FCPLCFFNDREF:           case NS_rtf::LN_LCBPLCFFNDREF:            case NS_rtf::LN_FCPLCFFNDTXT:            case NS_rtf::LN_LCBPLCFFNDTXT:            case NS_rtf::LN_FCPLCFANDREF:            case NS_rtf::LN_LCBPLCFANDREF:            case NS_rtf::LN_FCPLCFANDTXT:            case NS_rtf::LN_LCBPLCFANDTXT:            case NS_rtf::LN_FCPLCFSED:            case NS_rtf::LN_LCBPLCFSED:
            case NS_rtf::LN_FCPLCFPAD:            case NS_rtf::LN_LCBPLCFPAD:            case NS_rtf::LN_FCPLCFPHE:            case NS_rtf::LN_LCBPLCFPHE:            case NS_rtf::LN_FCSTTBFGLSY:            case NS_rtf::LN_LCBSTTBFGLSY:            case NS_rtf::LN_FCPLCFGLSY:            case NS_rtf::LN_LCBPLCFGLSY:            case NS_rtf::LN_FCPLCFHDD:            case NS_rtf::LN_LCBPLCFHDD:            case NS_rtf::LN_FCPLCFBTECHPX:            case NS_rtf::LN_LCBPLCFBTECHPX:
            case NS_rtf::LN_FCPLCFBTEPAPX:            case NS_rtf::LN_LCBPLCFBTEPAPX:            case NS_rtf::LN_FCPLCFSEA:            case NS_rtf::LN_LCBPLCFSEA:            case NS_rtf::LN_FCSTTBFFFN:            case NS_rtf::LN_LCBSTTBFFFN:            case NS_rtf::LN_FCPLCFFLDMOM:            case NS_rtf::LN_LCBPLCFFLDMOM:            case NS_rtf::LN_FCPLCFFLDHDR:            case NS_rtf::LN_LCBPLCFFLDHDR:
            case NS_rtf::LN_FCPLCFFLDFTN:            case NS_rtf::LN_LCBPLCFFLDFTN:            case NS_rtf::LN_FCPLCFFLDATN:            case NS_rtf::LN_LCBPLCFFLDATN:            case NS_rtf::LN_FCPLCFFLDMCR:            case NS_rtf::LN_LCBPLCFFLDMCR:            case NS_rtf::LN_FCSTTBFBKMK:            case NS_rtf::LN_LCBSTTBFBKMK:            case NS_rtf::LN_FCPLCFBKF:            case NS_rtf::LN_LCBPLCFBKF:            case NS_rtf::LN_FCPLCFBKL:            case NS_rtf::LN_LCBPLCFBKL:
            case NS_rtf::LN_FCCMDS:            case NS_rtf::LN_LCBCMDS:            case NS_rtf::LN_FCPLCMCR:            case NS_rtf::LN_LCBPLCMCR:            case NS_rtf::LN_FCSTTBFMCR:            case NS_rtf::LN_LCBSTTBFMCR:            case NS_rtf::LN_FCPRDRVR:            case NS_rtf::LN_LCBPRDRVR:            case NS_rtf::LN_FCPRENVPORT:           case NS_rtf::LN_LCBPRENVPORT:            case NS_rtf::LN_FCPRENVLAND:            case NS_rtf::LN_LCBPRENVLAND:
            case NS_rtf::LN_FCWSS:            case NS_rtf::LN_LCBWSS:            case NS_rtf::LN_FCDOP:            case NS_rtf::LN_LCBDOP:            case NS_rtf::LN_FCSTTBFASSOC:            case NS_rtf::LN_LCBSTTBFASSOC:            case NS_rtf::LN_FCCLX:            case NS_rtf::LN_LCBCLX:            case NS_rtf::LN_FCPLCFPGDFTN:            case NS_rtf::LN_LCBPLCFPGDFTN:           case NS_rtf::LN_FCAUTOSAVESOURCE:
            case NS_rtf::LN_LCBAUTOSAVESOURCE:            case NS_rtf::LN_FCGRPXSTATNOWNERS:            case NS_rtf::LN_LCBGRPXSTATNOWNERS:            case NS_rtf::LN_FCSTTBFATNBKMK:            case NS_rtf::LN_LCBSTTBFATNBKMK:            case NS_rtf::LN_FCPLCDOAMOM:            case NS_rtf::LN_LCBPLCDOAMOM:            case NS_rtf::LN_FCPLCDOAHDR:            case NS_rtf::LN_LCBPLCDOAHDR:            case NS_rtf::LN_FCPLCSPAMOM:            case NS_rtf::LN_LCBPLCSPAMOM:            case NS_rtf::LN_FCPLCSPAHDR:
            case NS_rtf::LN_LCBPLCSPAHDR:            case NS_rtf::LN_FCPLCFATNBKF:            case NS_rtf::LN_LCBPLCFATNBKF:            case NS_rtf::LN_FCPLCFATNBKL:            case NS_rtf::LN_LCBPLCFATNBKL:            case NS_rtf::LN_FCPMS:            case NS_rtf::LN_LCBPMS:            case NS_rtf::LN_FCFORMFLDSTTBF:            case NS_rtf::LN_LCBFORMFLDSTTBF:            case NS_rtf::LN_FCPLCFENDREF:
            case NS_rtf::LN_LCBPLCFENDREF:            case NS_rtf::LN_FCPLCFENDTXT:            case NS_rtf::LN_LCBPLCFENDTXT:            case NS_rtf::LN_FCPLCFFLDEDN:            case NS_rtf::LN_LCBPLCFFLDEDN:            case NS_rtf::LN_FCPLCFPGDEDN:            case NS_rtf::LN_LCBPLCFPGDEDN:            case NS_rtf::LN_FCDGGINFO:            case NS_rtf::LN_LCBDGGINFO:            case NS_rtf::LN_FCSTTBFRMARK:
            case NS_rtf::LN_LCBSTTBFRMARK:            case NS_rtf::LN_FCSTTBFCAPTION:            case NS_rtf::LN_LCBSTTBFCAPTION:            case NS_rtf::LN_FCSTTBFAUTOCAPTION:            case NS_rtf::LN_LCBSTTBFAUTOCAPTION:            case NS_rtf::LN_FCPLCFWKB:            case NS_rtf::LN_LCBPLCFWKB:            case NS_rtf::LN_FCPLCFSPL:            case NS_rtf::LN_LCBPLCFSPL:            case NS_rtf::LN_FCPLCFTXBXTXT:            case NS_rtf::LN_LCBPLCFTXBXTXT:
            case NS_rtf::LN_FCPLCFFLDTXBX:            case NS_rtf::LN_LCBPLCFFLDTXBX:            case NS_rtf::LN_FCPLCFHDRTXBXTXT:            case NS_rtf::LN_LCBPLCFHDRTXBXTXT:            case NS_rtf::LN_FCPLCFFLDHDRTXBX:            case NS_rtf::LN_LCBPLCFFLDHDRTXBX:            case NS_rtf::LN_FCSTWUSER:            case NS_rtf::LN_LCBSTWUSER:            case NS_rtf::LN_FCSTTBTTMBD:            case NS_rtf::LN_LCBSTTBTTMBD:
            case NS_rtf::LN_FCUNUSED:            case NS_rtf::LN_LCBUNUSED:            case NS_rtf::LN_FCPGDMOTHER:            case NS_rtf::LN_LCBPGDMOTHER:            case NS_rtf::LN_FCBKDMOTHER:            case NS_rtf::LN_LCBBKDMOTHER:            case NS_rtf::LN_FCPGDFTN:            case NS_rtf::LN_LCBPGDFTN:            case NS_rtf::LN_FCBKDFTN:            case NS_rtf::LN_LCBBKDFTN:            case NS_rtf::LN_FCPGDEDN:            case NS_rtf::LN_LCBPGDEDN:
            case NS_rtf::LN_FCBKDEDN:            case NS_rtf::LN_LCBBKDEDN:            case NS_rtf::LN_FCSTTBFINTLFLD:            case NS_rtf::LN_LCBSTTBFINTLFLD:            case NS_rtf::LN_FCROUTESLIP:            case NS_rtf::LN_LCBROUTESLIP:            case NS_rtf::LN_FCSTTBSAVEDBY:            case NS_rtf::LN_LCBSTTBSAVEDBY:            case NS_rtf::LN_FCSTTBFNM:            case NS_rtf::LN_LCBSTTBFNM:            case NS_rtf::LN_FCPLCFLST:
            case NS_rtf::LN_LCBPLCFLST:            case NS_rtf::LN_FCPLFLFO:            case NS_rtf::LN_LCBPLFLFO:            case NS_rtf::LN_FCPLCFTXBXBKD:            case NS_rtf::LN_LCBPLCFTXBXBKD:            case NS_rtf::LN_FCPLCFTXBXHDRBKD:            case NS_rtf::LN_LCBPLCFTXBXHDRBKD:            case NS_rtf::LN_FCDOCUNDO:            case NS_rtf::LN_LCBDOCUNDO:            case NS_rtf::LN_FCRGBUSE:            case NS_rtf::LN_LCBRGBUSE:
            case NS_rtf::LN_FCUSP:            case NS_rtf::LN_LCBUSP:            case NS_rtf::LN_FCUSKF:            case NS_rtf::LN_LCBUSKF:            case NS_rtf::LN_FCPLCUPCRGBUSE:            case NS_rtf::LN_LCBPLCUPCRGBUSE:            case NS_rtf::LN_FCPLCUPCUSP:            case NS_rtf::LN_LCBPLCUPCUSP:           case NS_rtf::LN_FCSTTBGLSYSTYLE:
            case NS_rtf::LN_LCBSTTBGLSYSTYLE:            case NS_rtf::LN_FCPLGOSL:            case NS_rtf::LN_LCBPLGOSL:            case NS_rtf::LN_FCPLCOCX:           case NS_rtf::LN_LCBPLCOCX:            case NS_rtf::LN_FCPLCFBTELVC:            case NS_rtf::LN_LCBPLCFBTELVC:            case NS_rtf::LN_DWLOWDATETIME:            case NS_rtf::LN_DWHIGHDATETIME:            case NS_rtf::LN_FCPLCFLVC:            case NS_rtf::LN_LCBPLCFLVC:           case NS_rtf::LN_FCPLCASUMY:
            case NS_rtf::LN_LCBPLCASUMY:            case NS_rtf::LN_FCPLCFGRAM:            case NS_rtf::LN_LCBPLCFGRAM:            case NS_rtf::LN_FCSTTBLISTNAMES:            case NS_rtf::LN_LCBSTTBLISTNAMES:            case NS_rtf::LN_FCSTTBFUSSR:
            case NS_rtf::LN_LCBSTTBFUSSR:             {
                m_pImpl->GetFIB().SetData( Name, nIntValue );
            }
            break;*/
            case NS_rtf::LN_FN:
            case NS_rtf::LN_FCSEPX:
            case NS_rtf::LN_FNMPR:
            case NS_rtf::LN_FCMPR:
                //section descriptor, unused or internally used
            break;
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
    //        case NS_rtf::LN_XSTZNAME: break;
    //        case NS_rtf::LN_XSTZNAME1: break;
    //        case NS_rtf::LN_UPXSTART: break;
    //        case NS_rtf::LN_UPX: break;
            case NS_rtf::LN_sed:
            {
                //section properties
                doctok::Reference<Properties>::Pointer_t pProperties = val.getProperties();
                if( pProperties.get())
                {
                    pProperties->resolve(*this);
                }
            }
            break;
            case NS_rtf::LN_tbdAdd: //
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
            case NS_rtf::LN_dxaDel: //deleted tab
            case NS_rtf::LN_dxaAdd: //set tab
            case NS_rtf::LN_TLC: //tab leading characters - for decimal tabs
            case NS_rtf::LN_JC: //tab justification
                    m_pImpl->ModifyCurrentTabStop(Name, nIntValue);
            break;
            case NS_rtf::LN_UNUSED0_6: // really unused
            break;
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

                             uno::makeAny( lcl_convertToMM100(nIntValue ) ));
        else if(eSprmType == SPRM_DEFAULT)
            rContext->Insert(
                             PROP_PARA_RIGHT_MARGIN,
                             uno::makeAny( lcl_convertToMM100(nIntValue ) ));
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
                         uno::makeAny( lcl_convertToMM100(nIntValue ) ));
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
                    aSpacing.Height = sal_Int16(lcl_convertToMM100(-nDistance));
                }
                else if(nDistance >0)
                {
                    aSpacing.Mode = style::LineSpacingMode::MINIMUM;
                    aSpacing.Height = sal_Int16(lcl_convertToMM100(nDistance));
                }
            }
            rContext->Insert(PROP_PARA_LINE_SPACING, uno::makeAny( aSpacing ));
        }
        break;
    case 21 : // legacy version
    case 0xA413:   // sprmPDyaBefore
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 1 */
        rContext->Insert(PROP_PARA_TOP_MARGIN, uno::makeAny( lcl_convertToMM100( nIntValue ) ));
        break;
    case 22 :
    case 0xA414:   // sprmPDyaAfter
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 1 */
        rContext->Insert(PROP_PARA_BOTTOM_MARGIN, uno::makeAny( lcl_convertToMM100( nIntValue ) ));
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
        {
            //is being resolved on the tokenizer side
            /*
                        doctok::Reference<Properties>::Pointer_t pProperties = sprm_.getProps();
                        if( pProperties.get())
                        {
                        pProperties->resolve(*this);
                        }
            */        }
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
        rContext->Insert(PROP_CHAR_CHAR_KERNING, uno::makeAny( sal_Int16(lcl_convertToMM100(sal_Int16(nIntValue))) ) );
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
                static const SectionPropertyMap::BorderPosition aPositions[4] =
                    {
                        SectionPropertyMap::BORDER_TOP,
                        SectionPropertyMap::BORDER_LEFT,
                        SectionPropertyMap::BORDER_BOTTOM,
                        SectionPropertyMap::BORDER_RIGHT
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
    ref->resolve(*this);
}
/*-- 09.06.2006 09:52:15---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper::table(doctok::Id name, doctok::Reference<Table>::Pointer_t ref)
{
    switch(name)
    {
        case NS_rtf::LN_FONTTABLE:
        {
            // create a font table object that listens to the attributes
            // each entry call inserts a new font entry
            ref->resolve( *m_pImpl->GetFontTable() );
        }
        break;
        case NS_rtf::LN_STYLESHEET:
            //same as above to import style sheets
            m_pImpl->SetStyleSheetImport( true );
            ref->resolve( *m_pImpl->GetStyleSheetTable() );
            m_pImpl->GetStyleSheetTable()->ApplyStyleSheets(m_pImpl->GetTextDocument());
            m_pImpl->SetStyleSheetImport( false );
        break;
        case NS_rtf::LN_LISTTABLE:
            //the same for list tables
            ref->resolve( *m_pImpl->GetListTable() );
        break;
        case NS_rtf::LN_LFOTABLE:
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
    switch( name )
    {
        case NS_rtf::LN_headerl:
            m_pImpl->PushPageHeader(SectionPropertyMap::PAGE_LEFT);
        break;
        case NS_rtf::LN_headerr:
            m_pImpl->PushPageHeader(SectionPropertyMap::PAGE_RIGHT);
        break;
        case NS_rtf::LN_headerf:
            m_pImpl->PushPageHeader(SectionPropertyMap::PAGE_FIRST);
        break;
        case NS_rtf::LN_footerl:
            m_pImpl->PushPageFooter(SectionPropertyMap::PAGE_LEFT);
        break;
        case NS_rtf::LN_footerr:
            m_pImpl->PushPageFooter(SectionPropertyMap::PAGE_RIGHT);
        break;
        case NS_rtf::LN_footerf:
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

