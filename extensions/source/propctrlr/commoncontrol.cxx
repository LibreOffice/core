/*************************************************************************
 *
 *  $RCSfile: commoncontrol.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 16:03:49 $
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
#include "commoncontrol.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _EXTENSIONS_PROPCTRLR_BRWCONTROLLISTENER_HXX_
#include "brwcontrollistener.hxx"
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif

//............................................................................
namespace pcr
{
//............................................................................
    //==================================================================
    //= OCommonBehaviourControl
    //==================================================================
    //------------------------------------------------------------------
    OCommonBehaviourControl::OCommonBehaviourControl(Window* _pMeAsWin)
        :m_pListener(NULL)
        ,m_pData(NULL)
        ,m_bDir(sal_True)
        ,m_bLocked(sal_False)
        ,m_bModified(sal_False)
        ,m_nLine(0)
        ,m_sStandardString(getStandardString())
        ,m_pMeAsWindow(_pMeAsWin)
    {
        DBG_ASSERT(m_pMeAsWindow != NULL, "OCommonBehaviourControl::OCommonBehaviourControl: invalid window!");
    }

    //------------------------------------------------------------------
    void OCommonBehaviourControl::SetLocked(sal_Bool _bFlag)
    {
        m_bLocked = _bFlag;
        Font aFont = m_pMeAsWindow->GetFont();
        if (m_bLocked)
        {
            aFont.SetColor(Color(COL_GRAY));
        }
        else
        {
            aFont = m_pMeAsWindow->GetParent()->GetFont();
        }
        m_pMeAsWindow->SetFont(aFont);
    }

    //------------------------------------------------------------------
    void OCommonBehaviourControl::autoSizeWindow()
    {
        ComboBox aComboBox(m_pMeAsWindow, WB_DROPDOWN);
        aComboBox.SetPosSizePixel(Point(0,0), Size(100,100));
        m_pMeAsWindow->SetSizePixel(aComboBox.GetSizePixel());
    }

    //------------------------------------------------------------------
    sal_Bool OCommonBehaviourControl::handlePreNotify(NotifyEvent& rNEvt)
    {
        if (EVENT_KEYINPUT == rNEvt.GetType())
        {
            const KeyCode& aKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
            sal_uInt16 nKey = aKeyCode.GetCode();

            if (nKey == KEY_RETURN && !aKeyCode.IsShift())
            {
                LoseFocusHdl(m_pMeAsWindow);
                m_bDir = sal_True;
                if (m_pListener != NULL)
                    m_pListener->TravelLine(this);
                return sal_True;
            }
        }
        return sal_False;
    }

    //------------------------------------------------------------------
    void OCommonBehaviourControl::CommitModified()
    {
        if (IsModified() && getListener())
            getListener()->Commit(this);
        m_bModified = sal_False;
    }

    //------------------------------------------------------------------
    void OCommonBehaviourControl::modified(Window* _pSource)
    {
        m_bModified = sal_True;
        if (m_pListener != NULL)
            m_pListener->Modified(this);
    }

    //------------------------------------------------------------------
    void OCommonBehaviourControl::getFocus(Window* _pSource)
    {
        if (m_pListener != NULL)
            m_pListener->GetFocus(this);
    }

    //------------------------------------------------------------------
    void OCommonBehaviourControl::commitModified(Window* _pSource)
    {
        if (m_pListener != NULL && m_bModified)
            m_pListener->Commit(this);
        m_bModified = sal_False;
    }

    //------------------------------------------------------------------
    IMPL_LINK( OCommonBehaviourControl, ModifiedHdl, Window*, _pWin )
    {
        modified(_pWin);
        return 0;
    }

    //------------------------------------------------------------------
    IMPL_LINK( OCommonBehaviourControl, GetFocusHdl, Window*, _pWin )
    {
        getFocus(_pWin);
        return 0;
    }

    //------------------------------------------------------------------
    IMPL_LINK( OCommonBehaviourControl, LoseFocusHdl, Window*, _pWin )
    {
        commitModified(_pWin);
        return 0;
    }

//............................................................................
} // namespace pcr
//............................................................................

