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



#ifndef DBACCESS_OPTIONALBOOLITEM_HXX
#define DBACCESS_OPTIONALBOOLITEM_HXX

#include <svl/poolitem.hxx>

#include <boost/optional.hpp>

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= OptionalBoolItem
    //====================================================================
    class OptionalBoolItem : public SfxPoolItem
    {
        ::boost::optional< bool >   m_aValue;

    public:
        OptionalBoolItem( sal_Int16 nWhich );
        OptionalBoolItem( const OptionalBoolItem& _rSource );

        virtual int              operator==( const SfxPoolItem& _rItem ) const;
        virtual SfxPoolItem*     Clone( SfxItemPool* _pPool = NULL ) const;

        bool    HasValue() const                { return !!m_aValue; }
        void    ClearValue()                    { m_aValue.reset(); }
        bool    GetValue() const                { return *m_aValue; }
        void    SetValue( const bool _bValue )  { m_aValue.reset( _bValue ); }

        const ::boost::optional< bool >&
            GetFullValue() const { return m_aValue; }
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_OPTIONALBOOLITEM_HXX
