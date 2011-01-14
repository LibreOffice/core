/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

