/*************************************************************************
 *
 *  $RCSfile: AppSwapWindow.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:30:04 $
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
 *  Contributor(s): Ocke Janssen
 *
 *
 ************************************************************************/
#ifndef DBAUI_APPSWAPWINDOW_HXX
#define DBAUI_APPSWAPWINDOW_HXX

#ifndef DBACCESS_TABLEDESIGN_ICLIPBOARDTEST_HXX
#include "IClipBoardTest.hxx"
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef DBAUI_APPICONCONTROL_HXX
#include "AppIconControl.hxx"
#endif
#ifndef DBAUI_APPELEMENTTYPE_HXX
#include "AppElementType.hxx"
#endif

namespace dbaui
{
    class OAppBorderWindow;
    //==================================================================
    class OApplicationSwapWindow :  public Control,
                                    public IClipboardTest
    {
        OApplicationIconControl             m_aIconControl;
        ElementType                         m_eLastType;
        OAppBorderWindow*                   m_pBorderWin;

        void ImplInitSettings( BOOL bFont, BOOL bForeground, BOOL bBackground );

        DECL_LINK( OnContainerSelectHdl,    SvtIconChoiceCtrl* );
        DECL_LINK( ChangeToLastSelected, void* );
    protected:
        virtual void DataChanged(const DataChangedEvent& rDCEvt);
    public:
        OApplicationSwapWindow(Window* _pParent,OAppBorderWindow* _pBorderWindow);
        virtual ~OApplicationSwapWindow();
        // window overloads
        virtual void Resize();


        inline sal_Bool isCutAllowed()      { return sal_False; }
        inline sal_Bool isCopyAllowed()     { return sal_False; }
        inline sal_Bool isPasteAllowed()    { return sal_False; }
        virtual sal_Bool hasChildPathFocus() { return HasChildPathFocus(); }
        inline void copy()  { }
        inline void cut()   { }
        inline void paste() { }

        inline ULONG                    GetEntryCount() const { return m_aIconControl.GetEntryCount(); }
        inline SvxIconChoiceCtrlEntry*  GetEntry( ULONG nPos ) const { return m_aIconControl.GetEntry(nPos); }
        inline Rectangle                GetBoundingBox( SvxIconChoiceCtrlEntry* pEntry ) const { return m_aIconControl.GetBoundingBox(pEntry); }

        /// return the element of currently select entry
        ElementType getElementType() const;


        /** clears the selection in the icon choice control and calls the handler
        */
        void clearSelection();

        /** changes the container which should be displayed. The select handler will also be called.
            @param  _eType
                Which container to show.
        */
        void changeContainer(ElementType _eType);
    };
// .............................................................
} // namespace dbaui
// .............................................................
#endif // DBAUI_APPSWAPWINDOW_HXX

