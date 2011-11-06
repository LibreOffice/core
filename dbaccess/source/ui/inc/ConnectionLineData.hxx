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


#ifndef DBAUI_CONNECTIONLINEDATA_HXX
#define DBAUI_CONNECTIONLINEDATA_HXX

#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif
#ifndef _VOS_REFERNCE_HXX_
#include <vos/refernce.hxx>
#endif
#include <vector>

#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

#ifndef DBAUI_REFFUNCTOR_HXX
#include "RefFunctor.hxx"
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

namespace dbaui
{

    //==================================================================
    // ConnData     ---------->*    ConnLineData
    //    ^1                            ^1
    //    |                             |
    //  Conn        ---------->*    ConnLine
    //==================================================================


    //==================================================================
    /**
        the class OConnectionLineData contains the data of a connection
        e.g. the source and the destanation field
    **/
    class OConnectionLineData : public ::vos::OReference
    {
        ::rtl::OUString m_aSourceFieldName;
        ::rtl::OUString m_aDestFieldName;

        friend bool operator==(const OConnectionLineData& lhs, const OConnectionLineData& rhs);
        friend bool operator!=(const OConnectionLineData& lhs, const OConnectionLineData& rhs) { return !(lhs == rhs); }
    protected:
        virtual ~OConnectionLineData();
    public:
        OConnectionLineData();
        OConnectionLineData( const ::rtl::OUString& rSourceFieldName, const ::rtl::OUString& rDestFieldName );
        OConnectionLineData( const OConnectionLineData& rConnLineData );

        // eine Kopie der eigenen Instanz liefern (das ist mir irgendwie angenehmer als ein virtueller Zuweisungsoperator)
        void CopyFrom(const OConnectionLineData& rSource);

        // Memberzugriff (schreiben)
        void SetFieldName(EConnectionSide nWhich, const ::rtl::OUString& strFieldName)
        {
            if (nWhich==JTCS_FROM)
                m_aSourceFieldName = strFieldName;
            else
                m_aDestFieldName = strFieldName;
        }
        void SetSourceFieldName( const ::rtl::OUString& rSourceFieldName){ SetFieldName(JTCS_FROM, rSourceFieldName); }
        void SetDestFieldName( const ::rtl::OUString& rDestFieldName ){ SetFieldName(JTCS_TO, rDestFieldName); }

        inline bool clearSourceFieldName() { SetSourceFieldName(::rtl::OUString()); return true;}
        inline bool clearDestFieldName() { SetDestFieldName(::rtl::OUString());     return true;}

        // Memberzugriff (lesen)
        ::rtl::OUString GetFieldName(EConnectionSide nWhich) const { return (nWhich == JTCS_FROM) ? m_aSourceFieldName : m_aDestFieldName; }
        ::rtl::OUString GetSourceFieldName() const { return GetFieldName(JTCS_FROM); }
        ::rtl::OUString GetDestFieldName() const { return GetFieldName(JTCS_TO); }

        bool Reset();
        OConnectionLineData& operator=( const OConnectionLineData& rConnLineData );
    };

    //-------------------------------------------------------------------------
    //------------------------------------------------------------------
    typedef ::vos::ORef< OConnectionLineData >      OConnectionLineDataRef;
    typedef ::std::vector< OConnectionLineDataRef > OConnectionLineDataVec;
}
#endif // DBAUI_CONNECTIONLINEDATA_HXX

