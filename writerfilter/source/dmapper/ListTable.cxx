#ifndef INCLUDED_LISTTABLE_HXX
#include <ListTable.hxx>
#endif
#ifndef INCLUDED_DOMAINMAPPER_HXX
#include <dmapper/DomainMapper.hxx>
#endif
#ifndef INCLUDED_DMAPPER_PROPERTYIDS_HXX
#include <PropertyIds.hxx>
#endif
#ifndef INCLUDED_RESOURCESIDS
#include <doctok/resourceids.hxx>
#endif
#ifndef INCLUDED_WW8_RESOURCE_MODEL_HXX
#include <doctok/WW8ResourceModel.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_NUMBERINGTYPE_HPP_
#include <com/sun/star/style/NumberingType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATION_HPP_
#include <com/sun/star/text/HoriOrientation.hpp>
#endif
#include <vector>

#define NUMBERING_MAX_LEVELS    10
namespace dmapper
{
using namespace writerfilter;
using namespace com::sun::star;

/*-- 26.06.2006 13:14:29---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int16 lcl_ConvertNumberingType(sal_Int32 nNFC)
{
    sal_Int16 nRet;
    switch(nNFC)
    {
        case 0: nRet = style::NumberingType::ARABIC;                break;
        case 1: nRet = style::NumberingType::ROMAN_UPPER;           break;
        case 2: nRet = style::NumberingType::ROMAN_LOWER;           break;
        case 3: nRet = style::NumberingType::CHARS_UPPER_LETTER_N;  break;
        case 4: nRet = style::NumberingType::CHARS_LOWER_LETTER_N;  break;
        case 5: nRet = style::NumberingType::ARABIC;                break;//ORDINAL
        case 23:
        case 25:
            nRet = style::NumberingType::CHAR_SPECIAL;
        break;
        case 255: nRet = style::NumberingType::NUMBER_NONE; break;
        default: nRet = style::NumberingType::ARABIC;
    }
    return nRet;
}
/*-- 26.06.2006 13:14:29---------------------------------------------------

  -----------------------------------------------------------------------*/
class ListPropertyMap : public PropertyMap
{
    friend class ListTable;

    sal_Int32                                       nIStartAt;       //LN_ISTARTAT
    sal_Int32                                       nNFC;            //LN_NFC
    sal_Int32                                       nJC;             //LN_JC
    sal_Int32                                       nFLegal;         //LN_FLEGAL
    sal_Int32                                       nFNoRestart;     //LN_FNORESTART
    sal_Int32                                       nFPrev;          //LN_FPREV
    sal_Int32                                       nFPrevSpace;     //LN_FPREVSPACE
    sal_Int32                                       nFWord6;         //LN_FWORD6
    ::rtl::OUString                                 sRGBXchNums;     //LN_RGBXCHNUMS
    sal_Int32                                       nXChFollow;      //LN_IXCHFOLLOW
    PropertyMapPtr                                  pProperties;    //LN_LISTLEVEL
public:
    ListPropertyMap() :
        nIStartAt(-1)
        ,nNFC(-1)
        ,nJC(-1)
        ,nFLegal(-1)
        ,nFNoRestart(-1)
        ,nFPrev(-1)
        ,nFPrevSpace(-1)
        ,nFWord6(-1)
        ,nXChFollow(-1)
        ,pProperties(new PropertyMap)
        {}
    ~ListPropertyMap(){}

    PropertyMapPtr  GetProperties() { return pProperties; }
    uno::Sequence< beans::PropertyValue >  GetPropertyValues();
};
/*-- 26.06.2006 13:44:57---------------------------------------------------

  -----------------------------------------------------------------------*/
#define MAKE_PROPVAL(NameId, Value) \
    beans::PropertyValue(aPropNameSupplier.GetName(NameId), 0, uno::makeAny(Value), beans::PropertyState_DIRECT_VALUE )

uno::Sequence< beans::PropertyValue >  ListPropertyMap::GetPropertyValues()
{
    const sal_Int16 aWWToUnoAdjust[] =
    {
        text::HoriOrientation::LEFT,
        text::HoriOrientation::CENTER,
        text::HoriOrientation::RIGHT,
    };

    PropertyNameSupplier& aPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
    vector< beans::PropertyValue > aNumberingProperties;

    if( nIStartAt >= 0)
        aNumberingProperties.push_back( MAKE_PROPVAL(PROP_START_WITH, (sal_Int16)nIStartAt) );

    if( nNFC >= 0)
        aNumberingProperties.push_back( MAKE_PROPVAL(PROP_NUMBERING_TYPE, lcl_ConvertNumberingType(nNFC)));

    if( nJC >= 0 && nJC <= sal::static_int_cast<sal_Int32>(sizeof(aWWToUnoAdjust) / sizeof(sal_Int16)) )
        aNumberingProperties.push_back( MAKE_PROPVAL(PROP_ADJUST, aWWToUnoAdjust[nJC]));

    //TODO: handling of nFLegal?
    //TODO: nFNoRestart lower levels do not restart when higher levels are incremented, like:
    //1.
    //1.1
    //2.2
    //2.3
    //3.4
    //

    if( nFWord6 > 0) //Word 6 compatibility
    {
        if( nFPrev == 1)
            aNumberingProperties.push_back( MAKE_PROPVAL( PROP_PARENT_NUMBERING, (sal_Int16) NUMBERING_MAX_LEVELS ));
        //TODO: prefixing space     nFPrevSpace;     - has not been used in WW8 filter
    }

//    TODO: sRGBXchNums;     array of inherited numbers

//    TODO: nXChFollow; following character 0 - tab, 1 - space, 2 - nothing
    if(pProperties)
    {

        _PropertyMap::const_iterator aMapIter = pProperties->begin();
        _PropertyMap::const_iterator aEndIter = pProperties->end();
        for( ; aMapIter != aEndIter; ++aMapIter )
        {
            aNumberingProperties.push_back(
                    beans::PropertyValue(aMapIter->first, 0, aMapIter->second, beans::PropertyState_DIRECT_VALUE ));
        }
    }
    uno::Sequence< beans::PropertyValue > aRet(aNumberingProperties.size());
    beans::PropertyValue* pValues = aRet.getArray();
    vector< beans::PropertyValue >::const_iterator aIt = aNumberingProperties.begin();
    vector< beans::PropertyValue >::const_iterator aEndIt = aNumberingProperties.end();
    for(sal_uInt32 nIndex = 0; aIt != aEndIt; ++aIt,++nIndex)
    {
        pValues[nIndex] = *aIt;
    }
    return aRet;
}
typedef boost::shared_ptr<ListPropertyMap> ListPropertyMapPtr;

struct ListEntry
{
    sal_Int32                                       nListId;        //LN_LSID
    sal_Int32                                       nTPLC;          //LN_TPLC
    ::rtl::OUString                                 sRGISTD;        //LN_RGISTD
    sal_Int32                                       nSimpleList;    //LN_FSIMPLELIST
    sal_Int32                                       nRestart;       //LN_FRESTARTHDN
    sal_Int32                                       nUnsigned;      //LN_UNSIGNED26_2

    ::std::vector< ListPropertyMapPtr >             aLevelProperties; //properties of each level

    ListPropertyMapPtr                              pCurrentProperties;
    uno::Reference< container::XIndexReplace >      m_xNumRules;

    ListEntry();

};
typedef boost::shared_ptr<ListEntry> ListEntryPtr;
/*-- 23.06.2006 13:58:51---------------------------------------------------

  -----------------------------------------------------------------------*/
ListEntry::ListEntry() :
    nListId(-1)
    ,nTPLC(-1)
    ,nSimpleList(-1)
    ,nRestart(-1)
    ,nUnsigned(-1)
{
}
/*-- 23.06.2006 13:58:51---------------------------------------------------

  -----------------------------------------------------------------------*/
struct ListTable_Impl
{
    DomainMapper&                                   m_rDMapper;
    uno::Reference< lang::XMultiServiceFactory >    m_xFactory;

    std::vector< ListEntryPtr >                     m_aListEntries;
    ListEntryPtr                                    m_pCurrentEntry;


    ListTable_Impl(DomainMapper& rDMapper, uno::Reference< lang::XMultiServiceFactory > xFactory) :
            m_rDMapper( rDMapper )
            ,m_xFactory( xFactory )
            {}

    void    AddLevel();
};
/*-- 26.06.2006 14:23:19---------------------------------------------------

  -----------------------------------------------------------------------*/
void ListTable_Impl::AddLevel()
{
    ListPropertyMapPtr pLevel( new ListPropertyMap );
    m_pCurrentEntry->pCurrentProperties = pLevel;
    m_pCurrentEntry->aLevelProperties.push_back(pLevel);
}
/*-- 23.06.2006 12:04:32---------------------------------------------------

  -----------------------------------------------------------------------*/
ListTable::ListTable(
        DomainMapper& rDMapper,
        uno::Reference< lang::XMultiServiceFactory > xFactory) :
    m_pImpl( new ListTable_Impl(rDMapper, xFactory) )
{
}
/*-- 23.06.2006 12:04:33---------------------------------------------------

  -----------------------------------------------------------------------*/
ListTable::~ListTable()
{
    delete m_pImpl;
}
/*-- 23.06.2006 12:04:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void ListTable::attribute(doctok::Id Name, doctok::Value & val)
{
    OSL_ENSURE( m_pImpl->m_pCurrentEntry.get(), "current entry has to be set here");
    if(!m_pImpl->m_pCurrentEntry.get())
        return ;
    int nIntValue = val.getInt();
    switch(Name)
    {
//        case NS_rtf::LN_ISTD: break;
        case NS_rtf::LN_ISTARTAT:
        case NS_rtf::LN_NFC:
        case NS_rtf::LN_JC:
        case NS_rtf::LN_FLEGAL:
        case NS_rtf::LN_FNORESTART:
        case NS_rtf::LN_FPREV:
        case NS_rtf::LN_FPREVSPACE:
        case NS_rtf::LN_FWORD6:
        case NS_rtf::LN_RGBXCHNUMS:
        case NS_rtf::LN_IXCHFOLLOW:
            if(m_pImpl->m_pCurrentEntry->pCurrentProperties.get())
            switch(Name)
            {
                case NS_rtf::LN_ISTARTAT:
                        m_pImpl->m_pCurrentEntry->pCurrentProperties->nIStartAt = nIntValue;
                break;
                case NS_rtf::LN_NFC:
                    m_pImpl->m_pCurrentEntry->pCurrentProperties->nNFC = nIntValue;
                break;
                case NS_rtf::LN_JC:
                    m_pImpl->m_pCurrentEntry->pCurrentProperties->nJC = nIntValue;
                break;
                case NS_rtf::LN_FLEGAL:
                    m_pImpl->m_pCurrentEntry->pCurrentProperties->nFLegal = nIntValue;
                break;
                case NS_rtf::LN_FNORESTART:
                    m_pImpl->m_pCurrentEntry->pCurrentProperties->nFNoRestart = nIntValue;
                break;
                case NS_rtf::LN_FPREV:
                    m_pImpl->m_pCurrentEntry->pCurrentProperties->nFPrev = nIntValue;
                break;
                case NS_rtf::LN_FPREVSPACE:
                    m_pImpl->m_pCurrentEntry->pCurrentProperties->nFPrevSpace = nIntValue;
                break;
                case NS_rtf::LN_FWORD6:
                    m_pImpl->m_pCurrentEntry->pCurrentProperties->nFWord6 = nIntValue;
                break;
                case NS_rtf::LN_RGBXCHNUMS:
                    m_pImpl->m_pCurrentEntry->pCurrentProperties->sRGBXchNums += val.getString();
                break;
                case NS_rtf::LN_IXCHFOLLOW:
                    m_pImpl->m_pCurrentEntry->pCurrentProperties->nXChFollow = nIntValue;
                break;
                default:
                    OSL_ASSERT("this line should never be reached");
            }
        break;
        case NS_rtf::LN_UNUSED5_7:
            //unused
        break;
//        case NS_rtf::LN_DXASPACE: break;
//        case NS_rtf::LN_DXAINDENT: break;
//        case NS_rtf::LN_CBGRPPRLCHPX: break;
//        case NS_rtf::LN_CBGRPPRLPAPX: break;
        case NS_rtf::LN_LSID:
            m_pImpl->m_pCurrentEntry->nListId = nIntValue;
        break;
        case NS_rtf::LN_TPLC:
            m_pImpl->m_pCurrentEntry->nTPLC = nIntValue;
        break;
        case NS_rtf::LN_RGISTD:
            m_pImpl->m_pCurrentEntry->sRGISTD += val.getString();
        break;
        case NS_rtf::LN_FSIMPLELIST:
            m_pImpl->m_pCurrentEntry->nSimpleList = nIntValue;
        break;
        case NS_rtf::LN_FRESTARTHDN:
            m_pImpl->m_pCurrentEntry->nRestart = nIntValue;
        break;
        case NS_rtf::LN_UNSIGNED26_2:
            m_pImpl->m_pCurrentEntry->nUnsigned = nIntValue;
        break;
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

        case NS_rtf::LN_LISTLEVEL:
        {
            //add a new level to the level vector and make it the current one
            m_pImpl->AddLevel();

            doctok::Reference<Properties>::Pointer_t pProperties;
            if((pProperties = val.getProperties()).get())
                pProperties->resolve(*this);
        }
        break;
//        case NS_rtf::LN_LFO: break;
//        case NS_rtf::LN_F: break;
//        case NS_rtf::LN_ALTFONTNAME: break;
//        case NS_rtf::LN_XSZFFN: break;
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
            //---->debug
            int nVal = val.getInt();
            ++nVal;
            //<----debug
        }
    }
}
/*-- 23.06.2006 12:04:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void ListTable::sprm(doctok::Sprm & sprm_)
{
    //fill the attributes of the style sheet
    if(m_pImpl->m_pCurrentEntry.get())
    {
        m_pImpl->m_rDMapper.sprm( sprm_,
                m_pImpl->m_pCurrentEntry->pCurrentProperties->GetProperties(), SPRM_LIST);
    }
}
/*-- 23.06.2006 12:04:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void ListTable::entry(int, doctok::Reference<Properties>::Pointer_t ref)
{
    //create a new list entry
    OSL_ENSURE( !m_pImpl->m_pCurrentEntry.get(), "current entry has to be NULL here");
    m_pImpl->m_pCurrentEntry.reset( new ListEntry );
    ref->resolve(*this);
    //append it to the table
    m_pImpl->m_aListEntries.push_back( m_pImpl->m_pCurrentEntry );
    m_pImpl->m_pCurrentEntry = ListEntryPtr();
}
/*-- 26.06.2006 10:27:55---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_uInt32 ListTable::size()
{
    return m_pImpl->m_aListEntries.size();
}
/*-- 26.06.2006 10:33:56---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< container::XIndexReplace > ListTable::GetNumberingRules(sal_Int32 nListId)
{
    uno::Reference< container::XIndexReplace > xRet;
    std::vector< ListEntryPtr >::const_iterator aIt = m_pImpl->m_aListEntries.begin();
    std::vector< ListEntryPtr >::const_iterator aEndIt = m_pImpl->m_aListEntries.end();
    for(; aIt != aEndIt; ++aIt)
    {
        if((*aIt)->nListId == nListId)
        {
            if( !(*aIt)->m_xNumRules.is() && m_pImpl->m_xFactory.is())
            {
                try
                {
                    (*aIt)->m_xNumRules = uno::Reference< container::XIndexReplace >(
                            m_pImpl->m_xFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.NumberingRules"))),
                            uno::UNO_QUERY_THROW);

                    //now fill the numbering levels appropriately
                    ::std::vector< ListPropertyMapPtr >::const_iterator aIter = (*aIt)->aLevelProperties.begin();
                    ::std::vector< ListPropertyMapPtr >::const_iterator aEnd = (*aIt)->aLevelProperties.end();
                    sal_Int32 nLevel = 0;
                    while(aIter != aEnd)
                    {
                        uno::Sequence< beans::PropertyValue> aValues = (*aIter)->GetPropertyValues();
                        (*aIt)->m_xNumRules->replaceByIndex(nLevel, uno::makeAny(aValues));

                        ++aIter;
                        ++nLevel;
                    }

                }
                catch( const uno::Exception& )
                {
                }
            }
            xRet = (*aIt)->m_xNumRules;
            break;
        }
    }
    return xRet;
}

}//namespace dmapper

