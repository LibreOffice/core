/*************************************************************************
 *
 *  $RCSfile: commoncontrol.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 09:04:46 $
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

#ifndef _EXTENSIONS_PROPCTRLR_COMMONCONTROL_HXX_
#define _EXTENSIONS_PROPCTRLR_COMMONCONTROL_HXX_

#ifndef _EXTENSIONS_PROPCTRLR_BRWCONTROL_HXX_
#include "brwcontrol.hxx"
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

//............................................................................
namespace pcr
{
//............................................................................

    //========================================================================
    //= OCommonBehaviourControl
    //========================================================================
    class OCommonBehaviourControl : public IBrowserControl
    {
    protected:
            ::rtl::OUString                 m_sStandardString;
            ::rtl::OUString                 m_aName;
            Window*                         m_pMeAsWindow;
            IBrowserControlListener*        m_pListener;
            void*                           m_pData;
            sal_uInt16                      m_nLine;
            sal_Bool                        m_bDir      : 1;
            sal_Bool                        m_bLocked   : 1;
            sal_Bool                        m_bModified : 1;

    public:
                                            OCommonBehaviourControl(Window* _pMeAsWin);

            virtual void                    setListener(IBrowserControlListener* p) { m_pListener = p; }
            virtual IBrowserControlListener*    getListener()                       { return m_pListener; }

            virtual sal_Bool                HasList()                                   { return sal_False; }
            virtual void                    ClearList()                                 { return; }
            virtual void                    InsertCtrEntry(const ::rtl::OUString& rString, sal_uInt16 nPos = EDITOR_LIST_APPEND)    { return; }

            virtual sal_Bool                IsModified() { return m_bModified; }
            virtual void                    CommitModified();

            virtual void                    SetMyName(const ::rtl::OUString &rString)   { m_aName = rString; }
            virtual ::rtl::OUString         GetMyName()const                            { return m_aName; }

            virtual void                    SetMyData(void* pData)                      { m_pData = pData; }
            virtual void*                   GetMyData()                                 { return m_pData; }

            virtual sal_Bool                GetDirection()                              { return m_bDir; }

            virtual void                    SetLine(sal_uInt16 nLine)                   { m_nLine = nLine; }
            virtual sal_uInt16              GetLine()                                   { return m_nLine; }

            virtual void                    SetLocked(sal_Bool bLocked=sal_True);

            virtual Window*                 GetMe()                                     { return m_pMeAsWindow; }

            virtual void                    SetCtrSize(const Size& rSize)               { m_pMeAsWindow->SetSizePixel(rSize); }
            virtual void                    SetCtrPos(const Point& aPoint)              { m_pMeAsWindow->SetPosPixel(aPoint); }
            virtual Point                   GetCtrPos() const                           { return m_pMeAsWindow->GetPosPixel(); }
            virtual Size                    GetCtrSize() const                          { return m_pMeAsWindow->GetSizePixel(); }
            virtual void                    ShowCtr( sal_Bool bFlag )                   { m_pMeAsWindow->Show( bFlag ); }
            virtual void                    EnableControl( sal_Bool _bDoEnable )        { m_pMeAsWindow->Enable( _bDoEnable ); }
            virtual void                    HideCtr()                                   { m_pMeAsWindow->Hide(); }

            virtual void                    SetCtrHelpId(sal_uInt32 nHelpId)                    { m_pMeAsWindow->SetHelpId(nHelpId); }
            virtual void                    SetTabOrder(Window* pRefWindow, sal_uInt16 nFlags ) { m_pMeAsWindow->SetZOrder(pRefWindow,nFlags); }
            virtual void                    SetCtrParent(Window* pParent)               { m_pMeAsWindow->SetParent(pParent); }
            virtual void                    InvalidateCtr()                             { m_pMeAsWindow->Invalidate(); }
            virtual void                    UpdateCtr()                                 { m_pMeAsWindow->Update(); }

    protected:
            /// may be used to implement the default handling in PreNotify; returns sal_True if handled
            sal_Bool handlePreNotify(NotifyEvent& _rNEvt);

            /// automatically size the window given in the ctor
            void    autoSizeWindow();

            /// may be used by derived classes, they forward the event to the PropCtrListener
            DECL_LINK(ModifiedHdl,Window*);
            DECL_LINK(GetFocusHdl,Window*);
            DECL_LINK(LoseFocusHdl,Window*);

    protected:
            // virtual versions of the handlers, called from within the link
            virtual void modified(Window* _pSource);
            virtual void getFocus(Window* _pSource);
            virtual void commitModified(Window* _pSource);
    };

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_COMMONCONTROL_HXX_

