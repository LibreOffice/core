/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RTableConnectionData.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 15:37:01 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#ifndef DBAUI_RTABLECONNECTIONDATA_HXX
#include "RTableConnectionData.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_KEYRULE_HPP_
#include <com/sun/star/sdbc/KeyRule.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XKEYSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADESCRIPTORFACTORY_HPP_
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XAPPEND_HPP_
#include <com/sun/star/sdbcx/XAppend.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDROP_HPP_
#include <com/sun/star/sdbcx/XDrop.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
using namespace dbaui;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

DBG_NAME(ORelationTableConnectionData)
//========================================================================
// class ORelationTableConnectionData
//========================================================================
//------------------------------------------------------------------------
ORelationTableConnectionData::ORelationTableConnectionData()
    :OTableConnectionData()
    ,m_nUpdateRules(KeyRule::NO_ACTION)
    ,m_nDeleteRules(KeyRule::NO_ACTION)
    ,m_nCardinality(CARDINAL_UNDEFINED)
{
    DBG_CTOR(ORelationTableConnectionData,NULL);
}
//------------------------------------------------------------------------
ORelationTableConnectionData::ORelationTableConnectionData( const TTableWindowData::value_type& _pReferencingTable,
                                                            const TTableWindowData::value_type& _pReferencedTable,
                                                            const ::rtl::OUString& rConnName )
    :OTableConnectionData( _pReferencingTable, _pReferencedTable )
    ,m_nUpdateRules(KeyRule::NO_ACTION)
    ,m_nDeleteRules(KeyRule::NO_ACTION)
    ,m_nCardinality(CARDINAL_UNDEFINED)
{
    DBG_CTOR(ORelationTableConnectionData,NULL);
    m_aConnName = rConnName;

    if ( m_aConnName.Len() )
        SetCardinality();
}

//------------------------------------------------------------------------
ORelationTableConnectionData::ORelationTableConnectionData( const ORelationTableConnectionData& rConnData )
    :OTableConnectionData( rConnData )
{
    DBG_CTOR(ORelationTableConnectionData,NULL);
    *this = rConnData;
}

//------------------------------------------------------------------------
ORelationTableConnectionData::~ORelationTableConnectionData()
{
    DBG_DTOR(ORelationTableConnectionData,NULL);
}

//------------------------------------------------------------------------
BOOL ORelationTableConnectionData::DropRelation()
{
    DBG_CHKTHIS(ORelationTableConnectionData,NULL);
    ::osl::MutexGuard aGuard( m_aMutex );
    ////////////////////////////////////////////////////////////
    // Relation loeschen
    Reference<XKeysSupplier> xSup(getReferencingTable()->getTable(),UNO_QUERY);
    Reference< XIndexAccess> xKeys;
    if(xSup.is() )
        xKeys = xSup->getKeys();
    else
        return FALSE;

    if( m_aConnName.Len() && xKeys.is() )
    {
        const sal_Int32 nCount = xKeys->getCount();
        for(sal_Int32 i = 0;i < nCount;++i)
        {
            Reference< XPropertySet> xKey(xKeys->getByIndex(i),UNO_QUERY);
            OSL_ENSURE(xKey.is(),"Key is not valid!");
            if(xKey.is())
            {
                ::rtl::OUString sName;
                xKey->getPropertyValue(PROPERTY_NAME) >>= sName;
                if(String(sName) == m_aConnName)
                {
                    Reference< XDrop> xDrop(xKeys,UNO_QUERY);
                    OSL_ENSURE(xDrop.is(),"can't drop key because we haven't a drop interface!");
                    if(xDrop.is())
                        xDrop->dropByIndex(i);
                    break;
                }
            }
        }
    }
    return TRUE;
}

//------------------------------------------------------------------------
void ORelationTableConnectionData::ChangeOrientation()
{
    DBG_CHKTHIS(ORelationTableConnectionData,NULL);
    //////////////////////////////////////////////////////////////////////
    // Source- und DestFieldName der Linien austauschen
    ::rtl::OUString sTempString;
    OConnectionLineDataVec::iterator aIter = m_vConnLineData.begin();
    for(;aIter != m_vConnLineData.end();++aIter)
    {
        sTempString = (*aIter)->GetSourceFieldName();
        (*aIter)->SetSourceFieldName( (*aIter)->GetDestFieldName() );
        (*aIter)->SetDestFieldName( sTempString );
    }

    //////////////////////////////////////////////////////////////////////
    // Member anpassen
    TTableWindowData::value_type pTemp = m_pReferencingTable;
    m_pReferencingTable = m_pReferencedTable;
    m_pReferencedTable = pTemp;
}

//------------------------------------------------------------------------
void ORelationTableConnectionData::SetCardinality()
{
    DBG_CHKTHIS(ORelationTableConnectionData,NULL);
    ::osl::MutexGuard aGuard( m_aMutex );
    m_nCardinality = CARDINAL_UNDEFINED;

    if( IsSourcePrimKey() )
    {
        if( IsDestPrimKey() )
            m_nCardinality = CARDINAL_ONE_ONE;
        else
            m_nCardinality = CARDINAL_ONE_MANY;
    }

    if( IsDestPrimKey() )
    {
        if( !IsSourcePrimKey() )
            m_nCardinality = CARDINAL_MANY_ONE;
    }

}
// -----------------------------------------------------------------------------
BOOL ORelationTableConnectionData::checkPrimaryKey(const Reference< XPropertySet>& _xTable,EConnectionSide _eEConnectionSide) const
{
    // check if Table has the primary key column dependig on _eEConnectionSide
    USHORT  nPrimKeysCount      = 0,
            nValidLinesCount    = 0;
    ::std::vector<Reference<XNameAccess> > vKeyColumns  = ::dbaui::getKeyColumns(_xTable,KeyType::PRIMARY);
    if ( vKeyColumns.size() == 1 )
    {
//      OSL_ENSURE(vKeyColumns.size()==1,"There can be only one primary key in a table!");
        Sequence< ::rtl::OUString> aKeyColumns;
        Reference<XNameAccess> xKeyColumns = *vKeyColumns.begin();
        if ( xKeyColumns.is() )
        {
            aKeyColumns = xKeyColumns->getElementNames();
            const ::rtl::OUString* pKeyBegin    = aKeyColumns.getConstArray();
            const ::rtl::OUString* pKeyEnd      = pKeyBegin + aKeyColumns.getLength();

            for(;pKeyBegin != pKeyEnd;++pKeyBegin)
            {
                OConnectionLineDataVec::const_iterator aIter = m_vConnLineData.begin();
                for(;aIter != m_vConnLineData.end();++aIter)
                {
                    ++nValidLinesCount;
                    if ( (*aIter)->GetFieldName(_eEConnectionSide) == *pKeyBegin )
                    {
                        ++nPrimKeysCount;
                        break;
                    }
                }
            }
        }
        if ( nPrimKeysCount != aKeyColumns.getLength() )
            return FALSE;
    }
    if ( !nPrimKeysCount || nPrimKeysCount != nValidLinesCount )
        return FALSE;

    return TRUE;
}
//------------------------------------------------------------------------
BOOL ORelationTableConnectionData::IsConnectionPossible()
{
    DBG_CHKTHIS(ORelationTableConnectionData,NULL);
    ::osl::MutexGuard aGuard( m_aMutex );

    //////////////////////////////////////////////////////////////////////
    // Wenn die SourceFelder ein PrimKey sind, ist nur die Orientierung falsch
    if ( IsSourcePrimKey() && !IsDestPrimKey() )
        ChangeOrientation();

    return TRUE;
}

//------------------------------------------------------------------------
OConnectionLineDataRef ORelationTableConnectionData::CreateLineDataObj()
{
    return new OConnectionLineData();
}

//------------------------------------------------------------------------
OConnectionLineDataRef ORelationTableConnectionData::CreateLineDataObj( const OConnectionLineData& rConnLineData )
{
    return new OConnectionLineData( rConnLineData );
}

//------------------------------------------------------------------------
void ORelationTableConnectionData::CopyFrom(const OTableConnectionData& rSource)
{
    // wie in der Basisklasse zurueckziehen auf das (nicht-virtuelle) operator=
    *this = *static_cast<const ORelationTableConnectionData*>(&rSource);
}

//------------------------------------------------------------------------
ORelationTableConnectionData& ORelationTableConnectionData::operator=( const ORelationTableConnectionData& rConnData )
{
    if (&rConnData == this)
        return *this;

    OTableConnectionData::operator=( rConnData );
    m_nUpdateRules = rConnData.GetUpdateRules();
    m_nDeleteRules = rConnData.GetDeleteRules();
    m_nCardinality = rConnData.GetCardinality();

    return *this;
}
namespace dbaui
{
//-------------------------------------------------------------------------
bool operator==(const ORelationTableConnectionData& lhs, const ORelationTableConnectionData& rhs)
{
    bool bEqual = (lhs.m_nUpdateRules == rhs.m_nUpdateRules)
        && (lhs.m_nDeleteRules == rhs.m_nDeleteRules)
        && (lhs.m_nCardinality == rhs.m_nCardinality)
        && (lhs.getReferencingTable() == rhs.getReferencingTable())
        && (lhs.getReferencedTable() == rhs.getReferencedTable())
        && (lhs.m_aConnName == rhs.m_aConnName)
        && (lhs.m_vConnLineData.size() == rhs.m_vConnLineData.size());

    if ( bEqual )
    {
        std::vector< OConnectionLineDataRef >::const_iterator aIter = lhs.m_vConnLineData.begin();
        std::vector< OConnectionLineDataRef >::const_iterator aEnd = lhs.m_vConnLineData.end();
        for (sal_Int32 i = 0; aIter != aEnd; ++aIter,++i)
        {
            if ( *(rhs.m_vConnLineData[i]) != **aIter )
                break;
        }
        bEqual = aIter == aEnd;
    }
    return bEqual;
}
}
//------------------------------------------------------------------------
BOOL ORelationTableConnectionData::Update()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    ////////////////////////////////////////////////////////////
    // Alte Relation loeschen
    {
        Reference<XKeysSupplier> xSup(getReferencingTable()->getTable(),UNO_QUERY);
        Reference< XIndexAccess> xKeys;
        if ( xSup.is() )
            xKeys = xSup->getKeys();
        else
            return FALSE;

        DropRelation();
        if( !IsConnectionPossible() )
            return FALSE;
    }

    // reassign the keys because the orientaion could be changed
    Reference<XKeysSupplier> xSup(getReferencingTable()->getTable(),UNO_QUERY);
    Reference< XIndexAccess> xKeys;
    if ( xSup.is() )
        xKeys = xSup->getKeys();

    if ( !xKeys.is() )
        return FALSE;
    ////////////////////////////////////////////////////////////
    // Neue Relation erzeugen
    Reference<XDataDescriptorFactory> xKeyFactory(xKeys,UNO_QUERY);
    OSL_ENSURE(xKeyFactory.is(),"No XDataDescriptorFactory Interface!");
    Reference<XAppend> xAppend(xKeyFactory,UNO_QUERY);
    OSL_ENSURE(xAppend.is(),"No XAppend Interface!");

    Reference<XPropertySet> xKey = xKeyFactory->createDataDescriptor();
    OSL_ENSURE(xKey.is(),"Key is null!");
    if ( xKey.is() )
    {
        // build a foreign key name
        ::rtl::OUString sSourceName;
        Reference<XPropertySet> xProp(xSup,UNO_QUERY_THROW);
        xProp->getPropertyValue(PROPERTY_NAME) >>= sSourceName;
        ::rtl::OUString sKeyName = sSourceName;
        sKeyName += getReferencedTable()->GetTableName();

        xKey->setPropertyValue(PROPERTY_NAME,makeAny(sKeyName));
        xKey->setPropertyValue(PROPERTY_TYPE,makeAny(KeyType::FOREIGN));
        xKey->setPropertyValue(PROPERTY_REFERENCEDTABLE,makeAny(::rtl::OUString(getReferencedTable()->GetTableName())));
        xKey->setPropertyValue(PROPERTY_UPDATERULE, makeAny(GetUpdateRules()));
        xKey->setPropertyValue(PROPERTY_DELETERULE, makeAny(GetDeleteRules()));
    }

    Reference<XColumnsSupplier> xColSup(xKey,UNO_QUERY);
    if ( xColSup.is() )
    {
        Reference<XNameAccess> xColumns = xColSup->getColumns();
        Reference<XDataDescriptorFactory> xColumnFactory(xColumns,UNO_QUERY);
        Reference<XAppend> xColumnAppend(xColumns,UNO_QUERY);
        if ( xColumnFactory.is() )
        {
            OConnectionLineDataVec::iterator aIter = m_vConnLineData.begin();
            for(;aIter != m_vConnLineData.end();++aIter)
            {
                if((*aIter)->GetSourceFieldName().getLength() && (*aIter)->GetDestFieldName().getLength())
                {
                    Reference<XPropertySet> xColumn;
                    xColumn = xColumnFactory->createDataDescriptor();
                    if ( xColumn.is() )
                    {
                        xColumn->setPropertyValue(PROPERTY_NAME,makeAny((*aIter)->GetSourceFieldName()));
                        xColumn->setPropertyValue(PROPERTY_RELATEDCOLUMN,makeAny((*aIter)->GetDestFieldName()));
                        xColumnAppend->appendByDescriptor(xColumn);
                    }
                }
            }

            if ( xColumns->hasElements() )
                xAppend->appendByDescriptor(xKey);
        }
        // to get the key we have to reget it because after append it is no longer valid
    }

    // get the name of foreign key // search for columns
    m_aConnName = ::rtl::OUString();
    xKey = NULL;
    for(sal_Int32 i=0;i<xKeys->getCount();++i)
    {
        xKeys->getByIndex(i) >>= xKey;
        OSL_ENSURE(xKey.is(),"Key is not valid!");
        if ( xKey.is() )
        {
            sal_Int32 nType = 0;
            xKey->getPropertyValue(PROPERTY_TYPE) >>= nType;
            ::rtl::OUString sReferencedTable;
            xKey->getPropertyValue(PROPERTY_REFERENCEDTABLE) >>= sReferencedTable;
            if ( sReferencedTable == ::rtl::OUString(getReferencedTable()->GetTableName()) )
            {
                xColSup.set(xKey,UNO_QUERY_THROW);
                try
                {
                    Reference<XNameAccess> xColumns = xColSup->getColumns();
                    Sequence< ::rtl::OUString> aNames = xColumns->getElementNames();
                    const ::rtl::OUString* pIter = aNames.getConstArray();
                    const ::rtl::OUString* pEnd = pIter + aNames.getLength();

                    Reference<XPropertySet> xColumn;
                    ::rtl::OUString sName,sRelatedColumn;
                    for ( ; pIter != pEnd ; ++pIter )
                    {
                        xColumn.set(xColumns->getByName(*pIter),UNO_QUERY_THROW);
                        xColumn->getPropertyValue(PROPERTY_NAME)            >>= sName;
                        xColumn->getPropertyValue(PROPERTY_RELATEDCOLUMN)   >>= sRelatedColumn;

                        OConnectionLineDataVec::iterator aIter = m_vConnLineData.begin();
                        for(;aIter != m_vConnLineData.end();++aIter)
                        {
                            if(    (*aIter)->GetSourceFieldName() == sName
                                && (*aIter)->GetDestFieldName() == sRelatedColumn )
                            {
                                break;
                            }
                        }
                        if ( aIter == m_vConnLineData.end() )
                            break;
                    }
                    if ( pIter == pEnd )
                    {
                        xKey->getPropertyValue(PROPERTY_NAME) >>= sName;
                        m_aConnName = sName;
                        //here we already know our column structure so we don't have to recreate the table connection data
                        xColSup.clear();
                        break;
                    }
                }
                catch(Exception&)
                {
                }
            }
        }
        xKey = NULL;
    }

//  OSL_ENSURE(xKey.is(),"No key found have insertion!");

    if ( xColSup.is() )
    {
        // The fields the relation marks may not be the same as our LineDatas mark after the relation has been updated
        OConnectionLineDataVec().swap(m_vConnLineData);

        Reference<XNameAccess> xColumns = xColSup->getColumns();
        Sequence< ::rtl::OUString> aNames = xColumns->getElementNames();
        const ::rtl::OUString* pIter = aNames.getConstArray();
        const ::rtl::OUString* pEnd = pIter + aNames.getLength();

        m_vConnLineData.reserve( aNames.getLength() );
        Reference<XPropertySet> xColumn;
        ::rtl::OUString sName,sRelatedColumn;

        for(;pIter != pEnd;++pIter)
        {
            xColumns->getByName(*pIter) >>= xColumn;
            if ( xColumn.is() )
            {
                OConnectionLineDataRef pNewData = CreateLineDataObj();

                xColumn->getPropertyValue(PROPERTY_NAME)            >>= sName;
                xColumn->getPropertyValue(PROPERTY_RELATEDCOLUMN)   >>= sRelatedColumn;

                pNewData->SetSourceFieldName(sName);
                pNewData->SetDestFieldName(sRelatedColumn);
                m_vConnLineData.push_back(pNewData);
            }
        }
    }
    // NOTE : the caller is resposible for updating any other objects referencing the old LineDatas (for instance a ConnLine)

    ////////////////////////////////////////////////////////////
    // Kardinalitaet bestimmen
    SetCardinality();

    return TRUE;
}
// -----------------------------------------------------------------------------

