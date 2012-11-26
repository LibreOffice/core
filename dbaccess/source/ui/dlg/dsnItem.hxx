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



#ifndef _DBAUI_DSNITEM_HXX_
#define _DBAUI_DSNITEM_HXX_

#include <svl/poolitem.hxx>

//.........................................................................
namespace dbaccess
{
//.........................................................................
    class ODsnTypeCollection;
//.........................................................................
}
//.........................................................................
//.........................................................................
namespace dbaui
{
//.........................................................................
    //=========================================================================
    //= DbuTypeCollectionItem
    //=========================================================================
    /** allows an ODsnTypeCollection to be transported in an SfxItemSet
    */
    class DbuTypeCollectionItem : public SfxPoolItem
    {
        ::dbaccess::ODsnTypeCollection* m_pCollection;

    public:
        DbuTypeCollectionItem(sal_Int16 nWhich = 0, ::dbaccess::ODsnTypeCollection* _pCollection = NULL);
        DbuTypeCollectionItem(const DbuTypeCollectionItem& _rSource);

        virtual int              operator==(const SfxPoolItem& _rItem) const;
        virtual SfxPoolItem*     Clone(SfxItemPool* _pPool = NULL) const;

        ::dbaccess::ODsnTypeCollection* getCollection() const { return m_pCollection; }
    };
//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_DSNITEM_HXX_

