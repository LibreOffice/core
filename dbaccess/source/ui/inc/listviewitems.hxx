/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: listviewitems.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:56:51 $
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

        virtual USHORT IsA();

        virtual void Paint(const Point& rPos, SvLBox& rDev, sal_uInt16 nFlags, SvLBoxEntry* pEntry);
        virtual void InitViewData( SvLBox* pView,SvLBoxEntry* pEntry, SvViewDataItem* _pViewData);

        sal_Bool    isEmphasized() const { return m_bEmphasized; }
        void        emphasize(sal_Bool _bEmphasize) { m_bEmphasized = _bEmphasize; }
    };

//........................................................................
}   // namespace dbaui
//........................................................................

#endif // _DBAUI_LISTVIEWITEMS_HXX_

