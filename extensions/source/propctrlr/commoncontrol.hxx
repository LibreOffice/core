/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: commoncontrol.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:07:15 $
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
            sal_uInt16                      m_nLine;
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

            virtual void                    SetLine(sal_uInt16 nLine)                   { m_nLine = nLine; }
            virtual sal_uInt16              GetLine()                                   { return m_nLine; }

            virtual Window*                 GetMe()                                     { return m_pMeAsWindow; }

            virtual void                    SetCtrSize(const Size& rSize)               { m_pMeAsWindow->SetSizePixel(rSize); }
            virtual void                    SetCtrPos(const Point& aPoint)              { m_pMeAsWindow->SetPosPixel(aPoint); }
            virtual Point                   GetCtrPos() const                           { return m_pMeAsWindow->GetPosPixel(); }
            virtual Size                    GetCtrSize() const                          { return m_pMeAsWindow->GetSizePixel(); }

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

