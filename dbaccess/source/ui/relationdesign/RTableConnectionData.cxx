/*************************************************************************
 *
 *  $RCSfile: RTableConnectionData.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-28 10:09:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
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

DBG_NAME(ORelationTableConnectionData);
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
ORelationTableConnectionData::ORelationTableConnectionData( const Reference< XNameAccess>& _xTables)
    :OTableConnectionData()
    ,m_nUpdateRules(KeyRule::NO_ACTION)
    ,m_nDeleteRules(KeyRule::NO_ACTION)
    ,m_nCardinality(CARDINAL_UNDEFINED)
    ,m_xTables(_xTables)
{
    DBG_CTOR(ORelationTableConnectionData,NULL);
}

//------------------------------------------------------------------------
ORelationTableConnectionData::ORelationTableConnectionData( const Reference< XNameAccess>& _xTables,
                                                            const ::rtl::OUString& rSourceWinName,
                                                            const ::rtl::OUString& rDestWinName,
                                                            const ::rtl::OUString& rConnName )
    :OTableConnectionData( rSourceWinName, rDestWinName, rConnName )

    ,m_nUpdateRules(KeyRule::NO_ACTION)
    ,m_nDeleteRules(KeyRule::NO_ACTION)
    ,m_nCardinality(CARDINAL_UNDEFINED)
    ,m_xTables(_xTables)
{
    DBG_CTOR(ORelationTableConnectionData,NULL);
    SetCardinality();
    if(m_xTables->hasByName(rSourceWinName))
        m_xTables->getByName(rSourceWinName) >>= m_xSource;
    if(m_xTables->hasByName(rDestWinName))
        m_xTables->getByName(rDestWinName) >>= m_xDest;
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
    ////////////////////////////////////////////////////////////
    // Relation loeschen
    Reference<XKeysSupplier> xSup(m_xSource,UNO_QUERY);
    Reference< XIndexAccess> xKeys;
    if(xSup.is() )
        xKeys = xSup->getKeys();
    else
        return FALSE;

    if( m_aConnName.Len() && xKeys.is() )
    {
        for(sal_Int32 i=0;i<xKeys->getCount();++i)
        {
            Reference< XPropertySet> xKey;
            xKeys->getByIndex(i) >>= xKey;
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
    ::std::vector<OConnectionLineData*>::iterator aIter = m_vConnLineData.begin();
    for(;aIter != m_vConnLineData.end();++aIter)
    {
        sTempString = (*aIter)->GetSourceFieldName();
        (*aIter)->SetSourceFieldName( (*aIter)->GetDestFieldName() );
        (*aIter)->SetDestFieldName( sTempString );
    }

    //////////////////////////////////////////////////////////////////////
    // Member anpassen
    sTempString         = m_aSourceWinName;
    m_aSourceWinName    = m_aDestWinName;
    m_aDestWinName      = sTempString;

    Reference<XPropertySet> xTemp;
    xTemp       = m_xSource;
    m_xSource   = m_xDest;
    m_xDest     = xTemp;
}

//------------------------------------------------------------------------
void ORelationTableConnectionData::SetSourceWinName( const String& rSourceWinName )
{
    OTableConnectionData::SetSourceWinName(rSourceWinName);
    if(m_xTables->hasByName(rSourceWinName))
        m_xTables->getByName(rSourceWinName) >>= m_xSource;
}

//------------------------------------------------------------------------
void ORelationTableConnectionData::SetDestWinName( const String& rDestWinName )
{
    OTableConnectionData::SetDestWinName(rDestWinName);
    if(m_xTables->hasByName(rDestWinName))
        m_xTables->getByName(rDestWinName) >>= m_xDest;
}

//------------------------------------------------------------------------
void ORelationTableConnectionData::SetCardinality()
{
    DBG_CHKTHIS(ORelationTableConnectionData,NULL);
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
    if(vKeyColumns.size())
    {
        OSL_ENSURE(vKeyColumns.size()==1,"There can be only one primary key in a table!");
        Reference<XNameAccess> xKeyColumns = *vKeyColumns.begin();
        Sequence< ::rtl::OUString> aKeyColumns = xKeyColumns->getElementNames();
        const ::rtl::OUString* pKeyBegin    = aKeyColumns.getConstArray();
        const ::rtl::OUString* pKeyEnd      = pKeyBegin + aKeyColumns.getLength();

        for(;pKeyBegin != pKeyEnd;++pKeyBegin)
        {
            ::std::vector<OConnectionLineData*>::const_iterator aIter = m_vConnLineData.begin();
            for(;aIter != m_vConnLineData.end();++aIter)
            {
                if( (*aIter)->IsValid() )
                    nValidLinesCount++;
                if((*aIter)->GetFieldName(_eEConnectionSide) == *pKeyBegin)
                {
                    nPrimKeysCount++;
                    break;
                }
            }
        }
        if(nPrimKeysCount != aKeyColumns.getLength())
            return FALSE;
    }
    if( !nPrimKeysCount || nPrimKeysCount != nValidLinesCount )
        return FALSE;

    return TRUE;
}
//------------------------------------------------------------------------
BOOL ORelationTableConnectionData::IsConnectionPossible()
{
    DBG_CHKTHIS(ORelationTableConnectionData,NULL);
    if( !m_xSource.is() || !m_xDest.is() )
        return FALSE;

    if( !m_aSourceWinName.Len() || !m_aDestWinName.Len() )
        return FALSE;

    //////////////////////////////////////////////////////////////////////
    // Wenn die SourceFelder ein PrimKey sind, ist nur die Orientierung falsch
    if( IsSourcePrimKey() )
        ChangeOrientation();

    return TRUE;
}

//------------------------------------------------------------------------
OConnectionLineData* ORelationTableConnectionData::CreateLineDataObj()
{
    return new OConnectionLineData();
}

//------------------------------------------------------------------------
OConnectionLineData* ORelationTableConnectionData::CreateLineDataObj( const OConnectionLineData& rConnLineData )
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

    //  m_sDatabaseName = rConnData.getTablesName();

    m_xTables   = rConnData.getTables();
    m_xSource   = rConnData.getSource();
    m_xDest     = rConnData.getDest();

    return *this;
}

//------------------------------------------------------------------------
BOOL ORelationTableConnectionData::Update()
{
    ////////////////////////////////////////////////////////////
    // Alte Relation loeschen
    Reference<XKeysSupplier> xSup(m_xSource,UNO_QUERY);
    Reference< XIndexAccess> xKeys;
    if(xSup.is() )
        xKeys = xSup->getKeys();
    else
        return FALSE;

    if( m_aConnName.Len() && xKeys.is() )
    {
        for(sal_Int32 i=0;i<xKeys->getCount();++i)
        {
            Reference< XPropertySet> xKey;
            xKeys->getByIndex(i) >>= xKey;
            OSL_ENSURE(xKey.is(),"Key is not valid!");
            if(xKey.is())
            {
                ::rtl::OUString sName;;
                xKey->getPropertyValue(PROPERTY_NAME) >>= sName;
                if(sName == ::rtl::OUString(m_aConnName))
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

    if( !IsConnectionPossible() )
        return FALSE;

    ////////////////////////////////////////////////////////////
    // Neue Relation erzeugen
    Reference<XDataDescriptorFactory> xKeyFactory(xKeys,UNO_QUERY);
    OSL_ENSURE(xKeyFactory.is(),"No XDataDescriptorFactory Interface!");
    Reference<XAppend> xAppend(xKeyFactory,UNO_QUERY);
    OSL_ENSURE(xAppend.is(),"No XAppend Interface!");

    Reference<XPropertySet> xKey = xKeyFactory->createDataDescriptor();
    OSL_ENSURE(xKey.is(),"Key is null!");
    xKey->setPropertyValue(PROPERTY_TYPE,makeAny(KeyType::FOREIGN));
    xKey->setPropertyValue(PROPERTY_REFERENCEDTABLE,makeAny(::rtl::OUString(m_aDestWinName)));
    xKey->setPropertyValue(PROPERTY_UPDATERULE, makeAny(GetUpdateRules()));
    xKey->setPropertyValue(PROPERTY_DELETERULE, makeAny(GetDeleteRules()));

    Reference<XColumnsSupplier> xColSup(xKey,UNO_QUERY);
    if(xColSup.is())
    {
        Reference<XNameAccess> xColumns = xColSup->getColumns();
        Reference<XDataDescriptorFactory> xColumnFactory(xColumns,UNO_QUERY);
        Reference<XAppend> xColumnAppend(xColumns,UNO_QUERY);

        ::std::vector<OConnectionLineData*>::iterator aIter = m_vConnLineData.begin();
        for(;aIter != m_vConnLineData.end();++aIter)
        {
            if((*aIter)->GetSourceFieldName().getLength() && (*aIter)->GetDestFieldName().getLength())
            {
                Reference<XPropertySet> xColumn;
                xColumn = xColumnFactory->createDataDescriptor();
                xColumn->setPropertyValue(PROPERTY_NAME,makeAny((*aIter)->GetSourceFieldName()));
                xColumn->setPropertyValue(PROPERTY_RELATEDCOLUMN,makeAny((*aIter)->GetDestFieldName()));
                xColumnAppend->appendByDescriptor(xColumn);
            }
        }

        if(xColumns->getElementNames().getLength())
            xAppend->appendByDescriptor(xKey);
        // to get the key we have to reget it because after append it is no longer valid
    }

    // get the name of foreign key // search for columns
    m_aConnName = ::rtl::OUString();
    xKey = NULL;
    for(sal_Int32 i=0;i<xKeys->getCount();++i)
    {
        xKeys->getByIndex(i) >>= xKey;
        OSL_ENSURE(xKey.is(),"Key is not valid!");
        if(xKey.is())
        {
            sal_Int32 nType = 0;
            xKey->getPropertyValue(PROPERTY_TYPE) >>= nType;
            if(nType == KeyType::FOREIGN)
            {
                ::rtl::OUString sName;
                xKey->getPropertyValue(PROPERTY_REFERENCEDTABLE) >>= sName;
                if(sName == ::rtl::OUString(m_aDestWinName))
                {
                    xKey->getPropertyValue(PROPERTY_NAME) >>= sName;
                    m_aConnName = sName;
                    break;
                }
            }
        }
        xKey = NULL;
    }

    OSL_ENSURE(xKey.is(),"No key found have insertion!");

    xColSup = Reference<XColumnsSupplier>(xKey,UNO_QUERY);
    if(xColSup.is())
    {
        // The fields the relation marks may not be the same as our LineDatas mark after the relation has been updated
        ::std::vector<OConnectionLineData*>::iterator aIter = m_vConnLineData.begin();
        for(;aIter != m_vConnLineData.end();++aIter)
            delete *aIter;
        m_vConnLineData.clear();

        Reference<XNameAccess> xColumns = xColSup->getColumns();
        Sequence< ::rtl::OUString> aNames = xColumns->getElementNames();
        const ::rtl::OUString* pBegin = aNames.getConstArray();
        const ::rtl::OUString* pEnd = pBegin + aNames.getLength();
        for(;pBegin != pEnd;++pBegin)
        {
            Reference<XPropertySet> xColumn;
            xColumns->getByName(*pBegin) >>= xColumn;
            if (xColumn.is())
            {
                OConnectionLineData* pNewData = CreateLineDataObj();

                ::rtl::OUString sName,sRelatedColumn;

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

