/*************************************************************************
 *
 *  $RCSfile: listviewitems.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 17:52:41 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

