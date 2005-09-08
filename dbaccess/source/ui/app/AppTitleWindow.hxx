/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AppTitleWindow.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:22:42 $
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
        void ImplInitSettings( BOOL bFont, BOOL bForeground, BOOL bBackground );
    protected:
        virtual void DataChanged(const DataChangedEvent& rDCEvt);
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

