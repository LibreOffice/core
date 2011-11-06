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



#ifndef _DBAUI_LISTVIEWITEMS_HXX_
#define _DBAUI_LISTVIEWITEMS_HXX_

#ifndef _SVLBOXITM_HXX
#include <svtools/svlbitm.hxx>
#endif

//........................................................................
namespace dbaui
{
//........................................................................

    #define SV_ITEM_ID_BOLDLBSTRING SV_ITEM_ID_LBOXSTRING

    //====================================================================
    //= OBoldListboxString
    //====================================================================
    class OBoldListboxString : public SvLBoxString
    {
        sal_Bool    m_bEmphasized;

    public:
        OBoldListboxString(SvLBoxEntry* _pEntry, sal_uInt16 _nFlags, const XubString& _rStr)
            :SvLBoxString(_pEntry, _nFlags, _rStr)
            ,m_bEmphasized(sal_False)
        {
        }

        virtual sal_uInt16 IsA();

        virtual void Paint(const Point& rPos, SvLBox& rDev, sal_uInt16 nFlags, SvLBoxEntry* pEntry);
        virtual void InitViewData( SvLBox* pView,SvLBoxEntry* pEntry, SvViewDataItem* _pViewData);

        sal_Bool    isEmphasized() const { return m_bEmphasized; }
        void        emphasize(sal_Bool _bEmphasize) { m_bEmphasized = _bEmphasize; }
    };

//........................................................................
}   // namespace dbaui
//........................................................................

#endif // _DBAUI_LISTVIEWITEMS_HXX_

