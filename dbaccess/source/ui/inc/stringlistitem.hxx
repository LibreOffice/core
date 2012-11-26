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



#ifndef _DBAUI_STRINGLISTITEM_HXX_
#define _DBAUI_STRINGLISTITEM_HXX_

#ifndef _SFXPOOLITEM_HXX
#include <svl/poolitem.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

//=========================================================================
//= OStringListItem
//=========================================================================
/** <type>SfxPoolItem</type> which transports a sequence of <type scope="rtl">OUString</type>'s
*/
class OStringListItem : public SfxPoolItem
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString >      m_aList;

public:
    OStringListItem(sal_Int16 nWhich, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rList);
    OStringListItem(const OStringListItem& _rSource);

    virtual int              operator==(const SfxPoolItem& _rItem) const;
    virtual SfxPoolItem*     Clone(SfxItemPool* _pPool = NULL) const;

    ::com::sun::star::uno::Sequence< ::rtl::OUString >  getList() const { return m_aList; }
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_STRINGLISTITEM_HXX_

