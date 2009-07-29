/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: LFOTable.cxx,v $
 * $Revision: 1.5 $
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
#include <LFOTable.hxx>
#include <doctok/resourceids.hxx>
#include <vector>

namespace writerfilter {
namespace dmapper
{
using namespace std;
using namespace writerfilter;

struct LFOLevel
{
    sal_Int32           nIStartAt;
    sal_Int32           nFStartAt;
    sal_Int32           nFFormatting;
    ::rtl::OUString     sILevel;

    LFOLevel() :
        nIStartAt(-1)
        ,nFStartAt(-1)
        ,nFFormatting(-1)
        {}
};
typedef ::boost::shared_ptr< LFOLevel > LFOLevelPtr;
struct LFOEntry
{
    sal_Int32               nListId;
    sal_Int32               nCLFOLevel;
    vector< LFOLevelPtr >      aLFOLevels; //usually empty

    LFOEntry() :
        nListId(-1)
        ,nCLFOLevel(-1)
        {}
};
typedef ::boost::shared_ptr<LFOEntry> LFOEntryPtr;

struct LFOTable_Impl
{
    ::std::vector< LFOEntryPtr >    m_aLFOEntries; //properties of each level
    LFOEntryPtr                     m_pCurrentEntry;

};
/*-- 27.06.2006 15:13:03---------------------------------------------------

  -----------------------------------------------------------------------*/
LFOTable::LFOTable() :
    m_pImpl( new LFOTable_Impl )
{
}
/*-- 27.06.2006 15:13:03---------------------------------------------------

  -----------------------------------------------------------------------*/
LFOTable::~LFOTable()
{
}
/*-- 27.06.2006 15:13:04---------------------------------------------------

  -----------------------------------------------------------------------*/
void LFOTable::attribute(Id Name, Value & val)
{
    OSL_ENSURE(m_pImpl->m_pCurrentEntry, "no current entry to write to");
    if(!m_pImpl->m_pCurrentEntry)
        return;

    int nIntValue = val.getInt();
    /* WRITERFILTERSTATUS: table: LFOTable_attributedata */
    switch( Name )
    {
//        case NS_rtf::LN_ISTD: break;//index of applied style
        case NS_rtf::LN_ISTARTAT:
        case NS_rtf::LN_ILVL:
        case NS_rtf::LN_FSTARTAT:
        case NS_rtf::LN_FFORMATTING:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            if(m_pImpl->m_pCurrentEntry->aLFOLevels.size())
            {
                vector< LFOLevelPtr >::reverse_iterator aEndIter = m_pImpl->m_pCurrentEntry->aLFOLevels.rbegin();
                switch( Name )
                {
                    case NS_rtf::LN_ISTARTAT:
                        /* WRITERFILTERSTATUS:*/
                        (*aEndIter)->nIStartAt = nIntValue;
                    break;
                    case NS_rtf::LN_ILVL:
                        /* WRITERFILTERSTATUS:*/
                        (*aEndIter)->sILevel = val.getString();
                    break;
                    case NS_rtf::LN_FSTARTAT:
                        /* WRITERFILTERSTATUS:*/
                        (*aEndIter)->nFStartAt = nIntValue;
                    break;
                    case NS_rtf::LN_FFORMATTING:
                        /* WRITERFILTERSTATUS:*/
                        (*aEndIter)->nFFormatting = nIntValue;
                    break;
                    default:;
                }
            }
        break;
        case NS_rtf::LN_LSID:
        /* WRITERFILTERSTATUS: done: 50, planned: 0, spent: 0 */
            m_pImpl->m_pCurrentEntry->nListId = nIntValue;
        break;
        case NS_rtf::LN_UNUSED4:
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_rtf::LN_UNUSED8:
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
            // as the names state they are unused
        break;
        case NS_rtf::LN_CLFOLVL:
        /* WRITERFILTERSTATUS: done: 50, planned: 0, spent: 0 */
            m_pImpl->m_pCurrentEntry->nCLFOLevel = nIntValue;
        break;
        case NS_rtf::LN_LFO:
            /* WRITERFILTERSTATUS: done: 50, planned: 0, spent: 0 */
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties;
            if(m_pImpl->m_pCurrentEntry && (pProperties = val.getProperties()).get())
            {
                LFOLevelPtr pLevel( new LFOLevel );
                m_pImpl->m_pCurrentEntry->aLFOLevels.push_back(pLevel);
            }
        }
        break;
        default:
        {
            OSL_ENSURE( false, "LFOTable::attribute: default statement");
            //---->debug
            int nVal = val.getInt();
            ++nVal;
            //<----debug
        }
    }
}
/*-- 27.06.2006 15:13:04---------------------------------------------------

  -----------------------------------------------------------------------*/
void LFOTable::sprm(Sprm& )
{
    OSL_ENSURE( false, "Which sprm should be handled here?");
}
/*-- 27.06.2006 15:13:04---------------------------------------------------

  -----------------------------------------------------------------------*/
void LFOTable::entry(int, writerfilter::Reference<Properties>::Pointer_t ref)
{
    //create a new LFO entry
    OSL_ENSURE( !m_pImpl->m_pCurrentEntry.get(), "current entry has to be NULL here");
    m_pImpl->m_pCurrentEntry.reset( new LFOEntry );
    ref->resolve(*this);
    //append it to the table
    m_pImpl->m_aLFOEntries.push_back( m_pImpl->m_pCurrentEntry );
    m_pImpl->m_pCurrentEntry = LFOEntryPtr();
}
/*-- 27.06.2006 15:13:05---------------------------------------------------
    1 based access to the LFO table
  -----------------------------------------------------------------------*/
sal_Int32 LFOTable::GetListID(sal_uInt32 nLFO)
{
    sal_Int32 nRet = -1;
    if( nLFO > 0 && nLFO <= m_pImpl->m_aLFOEntries.size())
        nRet = m_pImpl->m_aLFOEntries[nLFO - 1]->nListId;
    return nRet;
}
/*-- 12.11.2007 10:31:23---------------------------------------------------

  -----------------------------------------------------------------------*/
void LFOTable::AddListID( sal_Int32 nAbstractNumId )
{
    LFOEntryPtr pNew( new LFOEntry );
    pNew->nListId = nAbstractNumId;
    m_pImpl->m_aLFOEntries.push_back( pNew );
}

}//namespace dmapper
}//namespace writerfilter
