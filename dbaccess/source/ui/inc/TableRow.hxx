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


#ifndef DBAUI_TABLEROW_HXX
#define DBAUI_TABLEROW_HXX

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef DBAUI_TYPEINFO_HXX
#include "TypeInfo.hxx"
#endif


namespace dbaui
{
//  class OTableRow;
//  friend SvStream& operator<<( SvStream& rStr, OTableRow& _rRow );

    class OFieldDescription;
    class OTypeInfo;
    class OTableRow
    {
    private:
        OFieldDescription*      m_pActFieldDescr;
        long                    m_nPos;
        bool                    m_bReadOnly;
        bool                    m_bOwnsDescriptions;

    protected:
    public:
        OTableRow();
        OTableRow(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xAffectedCol);
        OTableRow( const OTableRow& rRow, long nPosition = -1 );
        ~OTableRow();

        inline OFieldDescription* GetActFieldDescr() const { return m_pActFieldDescr; }
        inline bool isValid() const { return GetActFieldDescr() != NULL; }

        void SetFieldType( const TOTypeInfoSP& _pType, sal_Bool _bForce = sal_False );

        void SetPrimaryKey( sal_Bool bSet );
        sal_Bool IsPrimaryKey() const;

        /** returns the current position in the table.
            @return
                the current position in the table
        */
        inline long GetPos() const { return m_nPos; }
        inline void SetPos(sal_Int32 _nPos) { m_nPos = _nPos; }

        /** set the row readonly
            @param  _bRead
                if <TRUE/> then the row is redonly, otherwise not
        */
        inline void SetReadOnly( bool _bRead=true ){ m_bReadOnly = _bRead; }

        /** returns if the row is readonly
            @return
                <TRUE/> if readonly, otherwise <FALSE/>
        */
        inline bool IsReadOnly() const { return m_bReadOnly; }

        friend SvStream& operator<<( SvStream& rStr,const OTableRow& _rRow );
        friend SvStream& operator>>( SvStream& rStr, OTableRow& _rRow );
    };
}
#endif // DBAUI_TABLEROW_HXX

