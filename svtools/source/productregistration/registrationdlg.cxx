/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"
#include "registrationdlg.hxx"

#if 0 /* @@@ */
#include <svtools/svtdata.hxx>
#ifndef _SVTOOLS_HRC
#include <svtools/svtools.hrc>
#endif
#endif /* @@@ */

#ifndef SVTOOLS_REGISTRATIONDLG_HRC
#include "registrationdlg.hrc"
#endif
#include <vcl/msgbox.hxx>
#include <tools/debug.hxx>

//........................................................................
namespace svt
{
//........................................................................

    static void lcl_moveControls( Control** _ppControls, sal_Int32 _nAmount )
    {
        if ( _ppControls )
            while ( *_ppControls )
            {
                Point aPos = (*_ppControls)->GetPosPixel();
                aPos.Y() += _nAmount;
                (*_ppControls)->SetPosPixel( aPos );

                ++_ppControls;
            }
    }

    //====================================================================
    //= RegistrationDialog
    //====================================================================
    //--------------------------------------------------------------------
    RegistrationDialog::RegistrationDialog( Window* _pWindow, const ResId& _rResId, bool _bEvalVersion )
        :ModalDialog( _pWindow, _rResId )
        ,m_eResponse        ( urRegisterLater )
        ,m_aLogo            ( this, ResId( FI_LOGO, *_rResId.GetResMgr() ) )
        ,m_aIntro           ( this, ResId( FT_INTRO, *_rResId.GetResMgr() ) )
        ,m_aNow             ( this, ResId( RB_NOW, *_rResId.GetResMgr() ) )
        ,m_aLater           ( this, ResId( RB_LATER, *_rResId.GetResMgr() ) )
        ,m_aNever           ( this, ResId( RB_NEVER, *_rResId.GetResMgr() ) )
        ,m_aAlreadyDone     ( this, ResId( RB_DONE, *_rResId.GetResMgr() ) )
        ,m_aSeparator       ( this, ResId( FL_SEPARATOR, *_rResId.GetResMgr() ) )
        ,m_aOK              ( this, ResId( BTN_OK, *_rResId.GetResMgr() ) )
        ,m_aHelp            ( this, ResId( BTN_HELP, *_rResId.GetResMgr() ) )
    {
        if ( _bEvalVersion )
        {   // if we're an eval version, we need to hide two of the options
            m_aNever.Hide( );
            m_aAlreadyDone.Hide( );

            // make the explanatory text somewhat smaller
            Size aIntroSize = m_aIntro.GetSizePixel();
            aIntroSize.Height() = LogicToPixel( Size( 0, 18 ), MAP_APPFONT ).Height();
            sal_Int32 nHeightDifference = m_aIntro.GetSizePixel().Height() - aIntroSize.Height();
            m_aIntro.SetSizePixel( aIntroSize );

            // resize the dialog, and move the controls below the ones we just hided
            sal_Int32 nAlreadyDoneLower = m_aAlreadyDone.GetPosPixel().Y() + m_aAlreadyDone.GetSizePixel().Height();
            sal_Int32 nLaterLower = m_aLater.GetPosPixel().Y() + m_aLater.GetSizePixel().Height();
            sal_Int32 nDifference = nAlreadyDoneLower - nLaterLower;

            sal_Int32 nOverallDifference = nDifference + nHeightDifference;

            // move
            Control* pVisibleRadios[] = { &m_aNow, &m_aLater, NULL };
            lcl_moveControls( pVisibleRadios, -nHeightDifference );

            Control* pControlsToMove[] = { &m_aSeparator, &m_aOK, &m_aHelp, NULL };
            lcl_moveControls( pControlsToMove, -nOverallDifference );

            // resize the dialog
            Size aSize = GetSizePixel();
            aSize.Height() -= nOverallDifference;
            SetSizePixel( aSize );
        }
        else
        {
            // the explanatory text needs to be completed
            String sCompleteIntro = m_aIntro.GetText( );
            sCompleteIntro += String( ResId( STR_COMPLETE_INTRO, *_rResId.GetResMgr() ) );
            m_aIntro.SetText( sCompleteIntro );
        }

        FreeResource();

        m_aNow.Check( sal_True );
    }

    //--------------------------------------------------------------------
    short RegistrationDialog::Execute()
    {
        short nResult = ModalDialog::Execute();

        // as a default, assume that the user wants to be reminded
        m_eResponse = urRegisterLater;

        if ( RET_OK == nResult )
        {
            if ( m_aNow.IsChecked() )
                m_eResponse = urRegisterNow;
            else if ( m_aLater.IsChecked() )
                m_eResponse = urRegisterLater;
            else if ( m_aNever.IsChecked() )
                m_eResponse = urRegisterNever;
            else if ( m_aAlreadyDone.IsChecked() )
                m_eResponse = urAlreadyRegistered;
#ifdef DBG_UTIL
            else
            {
                DBG_ERROR( "RegistrationDialog::Execute: invalid dialog state!" );
            }
#endif
        }
        return nResult;
    }
    //--------------------------------------------------------------------
    long RegistrationDialog::PreNotify( NotifyEvent& rNEvt )
    {
        long nHandled;
        if( rNEvt.GetType() == EVENT_KEYINPUT &&
            rNEvt.GetKeyEvent()->GetCharCode() &&
            rNEvt.GetKeyEvent()->GetKeyCode().GetCode() == KEY_ESCAPE)
        {
            EndDialog(RET_CANCEL);
            nHandled = 1;
        }
        else
            nHandled = ModalDialog::PreNotify( rNEvt );
        return nHandled;
    }


//........................................................................
}   // namespace svt
//........................................................................


