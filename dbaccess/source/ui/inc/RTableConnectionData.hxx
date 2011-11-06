/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef DBAUI_RTABLECONNECTIONDATA_HXX
#define DBAUI_RTABLECONNECTIONDATA_HXX

#ifndef DBAUI_TABLECONNECTIONDATA_HXX
#include "TableConnectionData.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif

namespace dbaui
{
    const sal_uInt16 CARDINAL_UNDEFINED = 0x0000;
    const sal_uInt16 CARDINAL_ONE_MANY  = 0x0001;
    const sal_uInt16 CARDINAL_MANY_ONE  = 0x0002;
    const sal_uInt16 CARDINAL_ONE_ONE   = 0x0004;

    class OConnectionLineData;
    //==================================================================
    class ORelationTableConnectionData :    public OTableConnectionData
    {
        friend bool operator==(const ORelationTableConnectionData& lhs, const ORelationTableConnectionData& rhs);
        friend bool operator!=(const ORelationTableConnectionData& lhs, const ORelationTableConnectionData& rhs) { return !(lhs == rhs); }

        ::osl::Mutex    m_aMutex;

        // @see com.sun.star.sdbc.KeyRule
        sal_Int32 m_nUpdateRules;
        sal_Int32 m_nDeleteRules;
        sal_Int32 m_nCardinality;

        sal_Bool checkPrimaryKey(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& i_xTable,EConnectionSide _eEConnectionSide) const;
        sal_Bool IsSourcePrimKey()  const { return checkPrimaryKey(getReferencingTable()->getTable(),JTCS_FROM);    }
        sal_Bool IsDestPrimKey()    const { return checkPrimaryKey(getReferencedTable()->getTable(),JTCS_TO);       }

    protected:
        virtual OConnectionLineDataRef CreateLineDataObj();
        virtual OConnectionLineDataRef CreateLineDataObj( const OConnectionLineData& rConnLineData );

        ORelationTableConnectionData& operator=( const ORelationTableConnectionData& rConnData );
    public:
        ORelationTableConnectionData();
        ORelationTableConnectionData( const ORelationTableConnectionData& rConnData );
        ORelationTableConnectionData( const TTableWindowData::value_type& _pReferencingTable,
                                      const TTableWindowData::value_type& _pReferencedTable,
                                      const ::rtl::OUString& rConnName = ::rtl::OUString() );
        virtual ~ORelationTableConnectionData();

        virtual void CopyFrom(const OTableConnectionData& rSource);
        virtual OTableConnectionData* NewInstance() const { return new ORelationTableConnectionData(); }

        /** Update create a new relation

            @return true if successful
        */
        virtual sal_Bool Update();


        void        SetCardinality();
        inline void SetUpdateRules( sal_Int32 nAttr ){ m_nUpdateRules = nAttr; }
        inline void SetDeleteRules( sal_Int32 nAttr ){ m_nDeleteRules = nAttr; }

        inline sal_Int32    GetUpdateRules() const { return m_nUpdateRules; }
        inline sal_Int32    GetDeleteRules() const { return m_nDeleteRules; }
        inline sal_Int32    GetCardinality() const { return m_nCardinality; }

        sal_Bool        IsConnectionPossible();
        void        ChangeOrientation();
        sal_Bool        DropRelation();
    };
}

#endif // DBAUI_RTABLECONNECTIONDATA_HXX



