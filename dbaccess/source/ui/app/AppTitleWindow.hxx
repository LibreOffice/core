/*************************************************************************
 *
 *  $RCSfile: AppTitleWindow.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:30:23 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBAUI_TITLE_WINDOW_HXX
#define DBAUI_TITLE_WINDOW_HXX

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

namespace dbaui
{
    class OTitleWindow : public Window
    {
        FixedText   m_aSpace1;
        FixedText   m_aSpace2;
        FixedText   m_aTitle;
        Window*     m_pChild;
        BOOL        m_bShift;
    public:
        OTitleWindow(Window* _pParent,USHORT _nTitleId,WinBits _nBits,BOOL _bShift = TRUE);
        virtual ~OTitleWindow();

        // window overloads
        virtual void Resize();
        virtual void GetFocus();

        /** sets the child window which should be displayed below the title. It will be destroyed at the end.
            @param  _pChild
                The child window.
        */
        void setChildWindow(Window* _pChild);

        /** gets the child window.

            @return
                The child winodw.
        */
        inline Window* getChildWindow() const { return m_pChild; }

        /** sets the title text out of the resource
            @param  _nTitleId
                The resource id of the title text.
        */
        void setTitle(USHORT _nTitleId);


        /** Gets the min Width in Pixel which is needed to display the whole

            @return
                the min width
        */
        long GetWidthPixel() const;
    };
// .............................................................
} // namespace dbaui
// .............................................................
#endif // DBAUI_TITLE_WINDOW_HXX

