/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ListTable.cxx,v $
 * $Revision: 1.8 $
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
#include <ListTable.hxx>
#include <dmapper/DomainMapper.hxx>
#include <PropertyIds.hxx>
#include <doctok/resourceids.hxx>
#include <doctok/sprmids.hxx>
#include <ooxml/resourceids.hxx>
#include <ConversionHelper.hxx>
#ifndef INCLUDED_WW8_RESOURCE_MODEL_HXX
#include <resourcemodel/WW8ResourceModel.hxx>
#endif
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <vector>

#define NUMBERING_MAX_LEVELS    10
namespace writerfilter {
namespace dmapper
{
using namespace com::sun::star;
/*-- 12.11.2007 11:38:57---------------------------------------------------

  -----------------------------------------------------------------------*/
class WRITERFILTER_DLLPRIVATE Numbering_numHdl : public Properties
{
    ListTable&      m_rListTable;
    sal_Int32       m_nAbstractNumId;
    ::rtl::OUString m_sNumId;
public:
    Numbering_numHdl( ListTable& rListTable ) :
        m_rListTable( rListTable ),
        m_nAbstractNumId( -1 )
        {}
    virtual ~Numbering_numHdl();

    // Properties
    virtual void attribute(Id Name, Value & val);
    virtual void sprm(Sprm & sprm);

    sal_Int32       GetAbstractNumId() const { return m_nAbstractNumId;}
    sal_Int32       GetNumId() const { return m_sNumId.toInt32(); }

};
typedef boost::shared_ptr< Numbering_numHdl >          Numbering_numHdlPtr;
/*-- 12.11.2007 11:42:04---------------------------------------------------

  -----------------------------------------------------------------------*/
Numbering_numHdl::~Numbering_numHdl()
{
}
/*-- 12.11.2007 11:42:22---------------------------------------------------

  -----------------------------------------------------------------------*/
void Numbering_numHdl::attribute(Id nName, Value & rVal)
{
    switch( nName )
    {
        case NS_ooxml::LN_CT_Num_numId:
            /* WRITERFILTERSTATUS: */
            m_sNumId = rVal.getString();
        break;
        case NS_ooxml::LN_CT_NumLvl_ilvl :
            m_rListTable.setOverwriteLevel(m_nAbstractNumId, rVal.getInt());
        break;
        default:;
    }
}
/*-- 12.11.2007 11:42:22---------------------------------------------------

  -----------------------------------------------------------------------*/
void Numbering_numHdl::sprm(Sprm & rSprm)
{
    sal_uInt32 nSprmId = rSprm.getId();
    switch( nSprmId )
    {
        case NS_ooxml::LN_CT_Num_abstractNumId:
            /* WRITERFILTERSTATUS: */
        {
            m_nAbstractNumId = rSprm.getValue()->getInt();
        }
        break;
        case NS_ooxml::LN_CT_Num_lvlOverride:
            /* WRITERFILTERSTATUS: */
        {
            //contains a list override
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if(pProperties.get())
                pProperties->resolve(*this);
            m_rListTable.resetOverwrite();
        }
        case NS_ooxml::LN_CT_NumLvl_lvl:
            /* WRITERFILTERSTATUS: */
            m_rListTable.sprm( rSprm );
        break;
        default:;
    }
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
    ::rtl::OUString                                 sBulletChar;
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
        {}
    ~ListPropertyMap(){}

    uno::Sequence< beans::PropertyValue >  GetPropertyValuesList( PropertyValueVector_t& rCharStyleProperties  );
};
/*-- 26.06.2006 13:44:57---------------------------------------------------

  -----------------------------------------------------------------------*/
#define MAKE_PROPVAL(NameId, Value) \
    beans::PropertyValue(aPropNameSupplier.GetName(NameId), 0, uno::makeAny(Value), beans::PropertyState_DIRECT_VALUE )

uno::Sequence< beans::PropertyValue >  ListPropertyMap::GetPropertyValuesList( PropertyValueVector_t& rCharStyleProperties )
{
    const sal_Int16 aWWToUnoAdjust[] =
    {
        text::HoriOrientation::LEFT,
        text::HoriOrientation::CENTER,
        text::HoriOrientation::RIGHT,
    };

    PropertyNameSupplier& aPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
    PropertyValueVector_t aNumberingProperties;

    if( nIStartAt >= 0)
        aNumberingProperties.push_back( MAKE_PROPVAL(PROP_START_WITH, (sal_Int16)nIStartAt) );

    sal_Int16 nNumberFormat = ConversionHelper::ConvertNumberingType(nNFC);
    if( nNFC >= 0)
        aNumberingProperties.push_back( MAKE_PROPVAL(PROP_NUMBERING_TYPE, nNumberFormat ));

    if( nJC >= 0 && nJC <= sal::static_int_cast<sal_Int32>(sizeof(aWWToUnoAdjust) / sizeof(sal_Int16)) )
        aNumberingProperties.push_back( MAKE_PROPVAL(PROP_ADJUST, aWWToUnoAdjust[nJC]));

    // todo: this is not the bullet char
    if( nNumberFormat == style::NumberingType::CHAR_SPECIAL && sBulletChar.getLength() )
        aNumberingProperties.push_back( MAKE_PROPVAL(PROP_BULLET_CHAR, sBulletChar.copy(0,1)));

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
//    if(pProperties)
//    {

        _PropertyMap::const_iterator aMapIter = /*pProperties->*/begin();
        _PropertyMap::const_iterator aEndIter = /*pProperties->*/end();
        for( ; aMapIter != aEndIter; ++aMapIter )
        {
            switch( aMapIter->first.eId )
            {
                case PROP_FIRST_LINE_OFFSET:
                case PROP_LEFT_MARGIN:
                    aNumberingProperties.push_back(
                        beans::PropertyValue( aPropNameSupplier.GetName( aMapIter->first.eId ), 0, aMapIter->second, beans::PropertyState_DIRECT_VALUE ));
                break;
                case PROP_CHAR_FONT_NAME:
                    aNumberingProperties.push_back(
                        beans::PropertyValue( aPropNameSupplier.GetName( PROP_BULLET_FONT_NAME ), 0, aMapIter->second, beans::PropertyState_DIRECT_VALUE ));
                break;
                default:
                {
                    rCharStyleProperties.push_back(beans::PropertyValue( aPropNameSupplier.GetName( aMapIter->first.eId ), 0, aMapIter->second, beans::PropertyState_DIRECT_VALUE ));
                }

            }
        }
//    }
    uno::Sequence< beans::PropertyValue > aRet(aNumberingProperties.size());
    beans::PropertyValue* pValues = aRet.getArray();
    PropertyValueVector_t::const_iterator aIt = aNumberingProperties.begin();
    PropertyValueVector_t::const_iterator aEndIt = aNumberingProperties.end();
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
    sal_Int32                                       nAbstractNumId;

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
    ,nAbstractNumId(-1)
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
        const uno::Reference< lang::XMultiServiceFactory > xFactory) :
    m_pImpl( new ListTable_Impl(rDMapper, xFactory) ),
    m_nOverwriteListId( -1 ),
    m_nOverwriteLevel( -1 )
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
void ListTable::attribute(Id nName, Value & rVal)
{
    OSL_ENSURE( m_pImpl->m_pCurrentEntry.get(), "current entry has to be set here");
    if(!m_pImpl->m_pCurrentEntry.get())
        return ;
    int nIntValue = rVal.getInt();
    /* WRITERFILTERSTATUS: table: ListTable_attributedata */
    switch(nName)
    {
        case NS_rtf::LN_RGBXCHNUMS:
        /* WRITERFILTERSTATUS: done: 50, planned: 0, spent: 0 */
            if(m_pImpl->m_pCurrentEntry->pCurrentProperties.get())
                m_pImpl->m_pCurrentEntry->pCurrentProperties->sRGBXchNums += rVal.getString();
        break;
        case NS_ooxml::LN_CT_LevelText_val:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        {
            //this strings contains the definition of the level
            //the level number is marked as %n
            //these numbers can be mixed randomly toghether with seperators pre- and suffixes
            //the Writer supports only a number of upper levels to show, separators is always a dot
            //and each level can have a prefix and a suffix
            if(m_pImpl->m_pCurrentEntry->pCurrentProperties.get())
            {
                m_pImpl->m_pCurrentEntry->pCurrentProperties->sBulletChar = rVal.getString();
            }
        }
        break;
//        case NS_rtf::LN_ISTD: break;
        case NS_rtf::LN_ISTARTAT:
        case NS_rtf::LN_NFC:
        case NS_rtf::LN_JC:
        case NS_rtf::LN_FLEGAL:
        case NS_rtf::LN_FNORESTART:
        case NS_rtf::LN_FPREV:
        case NS_rtf::LN_FPREVSPACE:
        case NS_rtf::LN_FWORD6:
        case NS_rtf::LN_IXCHFOLLOW:
            /* WRITERFILTERSTATUS: done: 75, planned: 0, spent: 0 */
            ApplyLevelValues( nName, nIntValue);
        break;
        case NS_rtf::LN_UNUSED5_7:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            //unused
        break;
        case NS_rtf::LN_LSID:
        /* WRITERFILTERSTATUS: done: 75, planned: 0, spent: 0 */
            m_pImpl->m_pCurrentEntry->nListId = nIntValue;
        break;
        case NS_rtf::LN_TPLC:
        /* WRITERFILTERSTATUS: done: 75, planned: 0, spent: 0 */
            m_pImpl->m_pCurrentEntry->nTPLC = nIntValue;
        break;
        case NS_rtf::LN_RGISTD:
        /* WRITERFILTERSTATUS: done: 75, planned: 0, spent: 0 */
            m_pImpl->m_pCurrentEntry->sRGISTD += rVal.getString();
        break;
        case NS_rtf::LN_FSIMPLELIST:
        /* WRITERFILTERSTATUS: done: 75, planned: 0, spent: 0 */
            m_pImpl->m_pCurrentEntry->nSimpleList = nIntValue;
        break;
        case NS_rtf::LN_FRESTARTHDN:
        /* WRITERFILTERSTATUS: done: 75, planned: 0, spent: 0 */
            m_pImpl->m_pCurrentEntry->nRestart = nIntValue;
        break;
        case NS_rtf::LN_UNSIGNED26_2:
        /* WRITERFILTERSTATUS: done: 75, planned: 0, spent: 0 */
            m_pImpl->m_pCurrentEntry->nUnsigned = nIntValue;
        break;

        case NS_rtf::LN_LISTLEVEL:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        {
            //add a new level to the level vector and make it the current one
            m_pImpl->AddLevel();

            writerfilter::Reference<Properties>::Pointer_t pProperties;
            if((pProperties = rVal.getProperties()).get())
                pProperties->resolve(*this);
        }
        break;
        case NS_ooxml::LN_CT_AbstractNum_abstractNumId:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        {
            sal_Int32 nVal = rVal.getString().toInt32();
            m_pImpl->m_pCurrentEntry->nAbstractNumId = nVal;
            m_pImpl->m_pCurrentEntry->nListId = nVal;
        }
        break;
        case NS_ooxml::LN_CT_Ind_left:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            m_pImpl->m_pCurrentEntry->pCurrentProperties->Insert(
                PROP_LEFT_MARGIN, true, uno::makeAny( ConversionHelper::convertTwipToMM100(nIntValue ) ));
            break;
        case NS_ooxml::LN_CT_Ind_hanging:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            m_pImpl->m_pCurrentEntry->pCurrentProperties->Insert(
                PROP_FIRST_LINE_OFFSET, true, uno::makeAny( - ConversionHelper::convertTwipToMM100(nIntValue ) ));
        break;
        case NS_ooxml::LN_CT_Lvl_ilvl: //overrides previous level - unsupported
        case NS_ooxml::LN_CT_Lvl_tplc: //template code - unsupported
        case NS_ooxml::LN_CT_Lvl_tentative: //marks level as unused in the document - unsupported
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Ind_firstLine: //todo: first line indent in numbering not yet supported
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        break;
        default:
        {
#if OSL_DEBUG_LEVEL > 0
            ::rtl::OString sMessage( "ListTable::attribute() - Id: ");
            sMessage += ::rtl::OString::valueOf( sal_Int32( nName ), 10 );
            sMessage += ::rtl::OString(" / 0x");
            sMessage += ::rtl::OString::valueOf( sal_Int32( nName ), 16 );
            sMessage += ::rtl::OString(" value: ");
            sMessage += ::rtl::OString::valueOf( sal_Int32( nIntValue ), 10 );
            sMessage += ::rtl::OString(" / 0x");
            sMessage += ::rtl::OString::valueOf( sal_Int32( nIntValue ), 16 );
            OSL_ENSURE( false, sMessage.getStr()); //
#endif
        }
    }
}
/*-- 23.06.2006 12:04:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void ListTable::sprm(Sprm & rSprm)
{
    //fill the attributes of the style sheet
    sal_uInt32 nSprmId = rSprm.getId();
    if( m_pImpl->m_pCurrentEntry.get() ||
        nSprmId == NS_ooxml::LN_CT_Numbering_abstractNum ||
        nSprmId == NS_ooxml::LN_CT_Numbering_num )
    {
        sal_Int32 nIntValue = rSprm.getValue()->getInt();
        /* WRITERFILTERSTATUS: table: ListTable_sprm */
        switch( nSprmId )
        {
            case NS_ooxml::LN_CT_Numbering_abstractNum:
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if(pProperties.get())
                {
                    //create a new list entry
                    OSL_ENSURE( !m_pImpl->m_pCurrentEntry.get(), "current entry has to be NULL here");
                    m_pImpl->m_pCurrentEntry.reset( new ListEntry );
                    pProperties->resolve( *this );
                    //append it to the table
                    m_pImpl->m_aListEntries.push_back( m_pImpl->m_pCurrentEntry );
                    m_pImpl->m_pCurrentEntry = ListEntryPtr();
                }
            }
            break;
            case NS_ooxml::LN_CT_Numbering_num:
            /* WRITERFILTERSTATUS: done: 50, planned: 0, spent: 0 */
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if(pProperties.get())
                {
                    Numbering_numHdlPtr pNumHdl( new Numbering_numHdl( *this ) );
                    pProperties->resolve(*pNumHdl);
                    //todo: is the order of numberings guaranteed?
                    //sal_Int32       pNumhdl->GetNumId();
                    m_pImpl->m_rDMapper.AddListIDToLFOTable( pNumHdl->GetAbstractNumId() );
                }
            }
            break;
            case NS_ooxml::LN_CT_AbstractNum_multiLevelType:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;
            case NS_rtf::LN_TPLC:
            /* WRITERFILTERSTATUS: done: 50, planned: 0, spent: 0 */
                m_pImpl->m_pCurrentEntry->nTPLC = nIntValue;
            break;
            case NS_ooxml::LN_CT_AbstractNum_lvl:
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            {
                m_pImpl->AddLevel();
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if(pProperties.get())
                    pProperties->resolve(*this);
                }
            break;
// not a useful number in ooxml
//            case NS_rtf::LN_LSID:
//                m_pImpl->m_pCurrentEntry->nListId = nIntValue;
//            break;
            case NS_rtf::LN_RGBXCHNUMS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
                break;
            case NS_rtf::LN_ISTARTAT:
            case NS_rtf::LN_NFC:
            case NS_rtf::LN_JC:
            case NS_rtf::LN_FLEGAL:
            case NS_rtf::LN_FNORESTART:
            case NS_rtf::LN_FPREV:
            case NS_rtf::LN_FPREVSPACE:
            case NS_rtf::LN_FWORD6:
            case NS_rtf::LN_IXCHFOLLOW:
            /* WRITERFILTERSTATUS: done: 75, planned: 0, spent: 0 */
                ApplyLevelValues( nSprmId, nIntValue );
            break;
            case NS_ooxml::LN_CT_Lvl_lvlText:
            case NS_ooxml::LN_CT_Lvl_rPr : //contains LN_EG_RPrBase_rFonts
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if(pProperties.get())
                    pProperties->resolve(*this);
            }
            break;
            case NS_ooxml::LN_CT_NumLvl_lvl:
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            {
                // overwrite level
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if(pProperties.get())
                    pProperties->resolve(*this);
            }
            break;
            case NS_ooxml::LN_CT_Lvl_lvlJc:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
                //todo: ????
            break;
            case NS_ooxml::LN_CT_Lvl_pPr:
            case NS_ooxml::LN_CT_PPrBase_ind:
            /* WRITERFILTERSTATUS: done: 50, planned: 0, spent: 0 */
            {
                //todo: how to handle paragraph properties within numbering levels (except LeftIndent and FirstLineIndent)?
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if(pProperties.get())
                    pProperties->resolve(*this);
            }
            break;
            case NS_ooxml::LN_CT_PPrBase_tabs:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
                //no paragraph attributes in ListTable char style sheets
            break;
            case NS_ooxml::LN_CT_Lvl_suff:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
                //todo: currently unsupported suffix
                //can be: "none", "space", "tab"
            break;
            case NS_ooxml::LN_EG_RPrBase_rFonts: //contains font properties
            case NS_ooxml::LN_EG_RPrBase_color:
            case NS_ooxml::LN_EG_RPrBase_u:
            case NS_sprm::LN_CHps:    // sprmCHps
            case NS_ooxml::LN_EG_RPrBase_lang:
            case NS_ooxml::LN_EG_RPrBase_eastAsianLayout:
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
                //no break!
            default:
                if(m_pImpl->m_pCurrentEntry->pCurrentProperties.get())
                {
                    m_pImpl->m_rDMapper.PushListProperties( m_pImpl->m_pCurrentEntry->pCurrentProperties );
                    m_pImpl->m_rDMapper.sprm( rSprm );
                    m_pImpl->m_rDMapper.PopListProperties();
                }
        }
    }
}
/*-- 12.11.2007 09:36:09---------------------------------------------------

  -----------------------------------------------------------------------*/
void    ListTable::ApplyLevelValues( sal_Int32 nId, sal_Int32 nIntValue)
{
    if(m_pImpl->m_pCurrentEntry->pCurrentProperties.get())
    switch(nId)
    {
        case NS_rtf::LN_ISTARTAT:
            /* WRITERFILTERSTATUS: */
                m_pImpl->m_pCurrentEntry->pCurrentProperties->nIStartAt = nIntValue;
        break;
        case NS_rtf::LN_NFC:
            /* WRITERFILTERSTATUS: */
            m_pImpl->m_pCurrentEntry->pCurrentProperties->nNFC = nIntValue;
        break;
        case NS_rtf::LN_JC:
            /* WRITERFILTERSTATUS: */
            m_pImpl->m_pCurrentEntry->pCurrentProperties->nJC = nIntValue;
        break;
        case NS_rtf::LN_FLEGAL:
            /* WRITERFILTERSTATUS: */
            m_pImpl->m_pCurrentEntry->pCurrentProperties->nFLegal = nIntValue;
        break;
        case NS_rtf::LN_FNORESTART:
            /* WRITERFILTERSTATUS: */
            m_pImpl->m_pCurrentEntry->pCurrentProperties->nFNoRestart = nIntValue;
        break;
        case NS_rtf::LN_FPREV:
            /* WRITERFILTERSTATUS: */
            m_pImpl->m_pCurrentEntry->pCurrentProperties->nFPrev = nIntValue;
        break;
        case NS_rtf::LN_FPREVSPACE:
            /* WRITERFILTERSTATUS: */
            m_pImpl->m_pCurrentEntry->pCurrentProperties->nFPrevSpace = nIntValue;
        break;
        case NS_rtf::LN_FWORD6:
            /* WRITERFILTERSTATUS: */
            m_pImpl->m_pCurrentEntry->pCurrentProperties->nFWord6 = nIntValue;
        break;
        case NS_rtf::LN_IXCHFOLLOW:
            /* WRITERFILTERSTATUS: */
            m_pImpl->m_pCurrentEntry->pCurrentProperties->nXChFollow = nIntValue;
        break;
        default:
            OSL_ENSURE( false, "this line should never be reached");
    }
}
/*-- 23.06.2006 12:04:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void ListTable::entry(int, writerfilter::Reference<Properties>::Pointer_t ref)
{

    if( m_pImpl->m_rDMapper.IsOOXMLImport() )
    {
        ref->resolve(*this);
    }
    else
    {
        //create a new list entry
        OSL_ENSURE( !m_pImpl->m_pCurrentEntry.get(), "current entry has to be NULL here");
        m_pImpl->m_pCurrentEntry.reset( new ListEntry );
        ref->resolve(*this);
        //append it to the table
        m_pImpl->m_aListEntries.push_back( m_pImpl->m_pCurrentEntry );
        m_pImpl->m_pCurrentEntry = ListEntryPtr();
    }
}
/*-- 26.06.2006 10:27:55---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_uInt32 ListTable::size() const
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
                    PropertyNameSupplier& aPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
                    while(aIter != aEnd)
                    {
                        PropertyValueVector_t aCharStyleProperties;
                        uno::Sequence< beans::PropertyValue> aValues = (*aIter)->GetPropertyValuesList(aCharStyleProperties);
                        if( aCharStyleProperties.size() )
                        {
                            //create (or find) a character style containing the character attributes of the symbol
                            //and apply it to the numbering level
                            ::rtl::OUString sStyle = m_pImpl->m_rDMapper.getOrCreateCharStyle( aCharStyleProperties );
                            aValues.realloc( aValues.getLength() + 1);
                            aValues[aValues.getLength() - 1].Name = aPropNameSupplier.GetName( PROP_CHAR_STYLE_NAME );
                            aValues[aValues.getLength() - 1].Value <<= sStyle;
                        }
                        //now parse the text to find %n from %1 to %nLevel+1
                        //everything before the first % and the last %x is prefix and suffix
                        ::rtl::OUString sLevelText( (*aIter)->sBulletChar );
                        sal_Int32 nCurrentIndex = 0;
                        sal_Int32 nFound = sLevelText.indexOf( '%', nCurrentIndex );
                        if( nFound > 0 )
                        {
                            ::rtl::OUString sPrefix = sLevelText.copy( 0, nFound );
                            aValues.realloc( aValues.getLength() + 1 );
                            aValues[ aValues.getLength() - 1 ] = MAKE_PROPVAL(PROP_PREFIX, sPrefix);
                            sLevelText = sLevelText.copy( nFound );
                        }
                        sal_Int32 nMinLevel = nLevel;
                        //now the text should either be empty or start with %
                        nFound = 0;
                        while( nFound >= 0 )
                        {
                            if( sLevelText.getLength() > 1 )
                            {
                                sal_Unicode cLevel = sLevelText.getStr()[1];
                                if( cLevel >= '1' && cLevel <= '9' )
                                {
                                    if( cLevel - '1' < nMinLevel )
                                        nMinLevel = cLevel - '1';
                                    //remove first char - next char is removed later
                                    sLevelText = sLevelText.copy( 1 );
                                }
                            }
                            //remove old '%' or number
                            sLevelText = sLevelText.copy( 1 );
                            nCurrentIndex = 0;
                            nFound = sLevelText.indexOf( '%', nCurrentIndex );
                            //remove the text before the next %
                            if(nFound > 0)
                                sLevelText = sLevelText.copy( nFound -1 );
                        }
                        if( nMinLevel < nLevel )
                        {
                            aValues.realloc( aValues.getLength() + 1);
                            aValues[ aValues.getLength() - 1 ] =
                                MAKE_PROPVAL(PROP_PARENT_NUMBERING, sal_Int16( nLevel - nMinLevel ));
                        }
                        if( sLevelText.getLength() )
                        {
                            aValues.realloc( aValues.getLength() + 1);
                            aValues[ aValues.getLength() - 1 ] = MAKE_PROPVAL(PROP_SUFFIX, sLevelText);
                        }

                        (*aIt)->m_xNumRules->replaceByIndex(nLevel, uno::makeAny(aValues));

                        ++aIter;
                        ++nLevel;
                    }

                }
                catch( const uno::Exception& rEx)
                {
                    (void)rEx;
                    OSL_ENSURE( false, "ListTable::GetNumberingRules");
                }
            }
            xRet = (*aIt)->m_xNumRules;
            break;
        }
    }
    return xRet;
}
/*-- 19.11.2007 13:25:32---------------------------------------------------

  -----------------------------------------------------------------------*/
void ListTable::setOverwriteLevel(sal_Int32 nAbstractNumId, sal_Int32 nLevel)
{
    m_nOverwriteListId = nAbstractNumId;
    m_nOverwriteLevel = nLevel;
    OSL_ENSURE(!m_pImpl->m_pCurrentEntry.get(), "where to put the overwrite level");
    std::vector< ListEntryPtr >::const_iterator aIt = m_pImpl->m_aListEntries.begin();
    std::vector< ListEntryPtr >::const_iterator aEndIt = m_pImpl->m_aListEntries.end();
    for(; aIt != aEndIt; ++aIt)
    {
        if( (*aIt)->nListId == nAbstractNumId )
        {
            m_pImpl->m_pCurrentEntry = *aIt;
            break;
        }
    }
    OSL_ENSURE( m_pImpl->m_pCurrentEntry.get(), "list not found");
}
/*-- 19.11.2007 13:25:32---------------------------------------------------

  -----------------------------------------------------------------------*/
void ListTable::resetOverwrite()
{
    m_nOverwriteListId =  -1;
    m_nOverwriteLevel = -1;
    m_pImpl->m_pCurrentEntry.reset();
}

}//namespace dmapper
}//namespace writerfilter

