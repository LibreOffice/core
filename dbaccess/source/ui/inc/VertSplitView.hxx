/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VertSplitView.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:41:29 $
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
#ifndef DBAUI_VERTSPLITVIEW_HXX
#define DBAUI_VERTSPLITVIEW_HXX

#ifndef _SV_SPLIT_HXX
#include <vcl/split.hxx>
#endif

namespace dbaui
{
    //==================================================================
    class OSplitterView : public Window
    {
        Splitter*                           m_pSplitter;
        Window*                             m_pLeft;
        Window*                             m_pRight;
        sal_Bool                            m_bVertical;

        void ImplInitSettings( BOOL bFont, BOOL bForeground, BOOL bBackground );
        DECL_LINK( SplitHdl, Splitter* );
    protected:
        virtual void DataChanged(const DataChangedEvent& rDCEvt);
    public:
        OSplitterView(Window* _pParent,sal_Bool _bVertical = sal_True);
        virtual ~OSplitterView();
        // window overloads
        virtual void GetFocus();

        void setSplitter(Splitter* _pSplitter);
        void set(Window* _pRight,Window* _pLeft = NULL);
        virtual void Resize();
    };
}
#endif // DBAUI_VERTSPLITVIEW_HXX

