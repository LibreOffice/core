/*************************************************************************
 *
 *  $RCSfile: bibview.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-17 16:15:16 $
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

#ifndef BIB_HRC
#include "bib.hrc"
#endif
#ifndef ADDRCONT_HXX
#include "bibcont.hxx"
#endif
#ifndef ADRBEAM_HXX
#include "bibbeam.hxx"
#endif
#ifndef BIBMOD_HXX
#include "bibmod.hxx"
#endif
#ifndef _BIB_GENERAL_HXX
#include "general.hxx"
#endif
#ifndef _BIB_VIEW_HXX
#include "bibview.hxx"
#endif
#ifndef _BIB_DATMAN_HXX
#include "datman.hxx"
#endif
#ifndef ADRRESID_HXX
#include "bibresid.hxx"
#endif
#ifndef BIBMOD_HXX
#include "bibmod.hxx"
#endif
#include "sections.hrc"
#ifndef _BIBCONFIG_HXX
#include "bibconfig.hxx"
#endif


#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETUPDATE_HPP_
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

#define C2U( cChar ) ::rtl::OUString::createFromAscii( cChar )

//.........................................................................
namespace bib
{
//.........................................................................

    // -----------------------------------------------------------------------
    BibView::BibView( Window* _pParent, BibDataManager* _pManager, WinBits _nStyle )
        :BibWindow( _pParent, _nStyle )
        ,m_pDatMan( _pManager )
        ,m_xDatMan( _pManager )
        ,m_pGeneralPage( NULL )
    {
        if ( m_xDatMan.is() )
            connectForm( m_xDatMan );
    }

    // -----------------------------------------------------------------------
    BibView::~BibView()
    {
        BibGeneralPage* pGeneralPage = m_pGeneralPage;
        m_pGeneralPage = NULL;

        pGeneralPage->CommitActiveControl();
        Reference< XForm > xForm = m_pDatMan->getForm();
        Reference< XPropertySet > xProps( xForm, UNO_QUERY );
        Reference< sdbc::XResultSetUpdate > xResUpd( xProps, UNO_QUERY );
        DBG_ASSERT( xResUpd.is(), "BibView::~BibView: invalid form!" );

        if ( xResUpd.is() )
        {
            Any aModified = xProps->getPropertyValue( C2U( "IsModified" ) );
            sal_Bool bFlag;
            if ( ( aModified >>= bFlag ) && bFlag )
            {

                Any aNew = xProps->getPropertyValue( C2U( "IsNew" ) );
                aNew >>= bFlag;
                if ( bFlag )
                    xResUpd->insertRow();
                else
                    xResUpd->updateRow();
            }
        }

        if ( isFormConnected() )
            disconnectForm();

        pGeneralPage->RemoveListeners();
        m_xGeneralPage = NULL;
    }

    /* -----------------16.11.99 13:13-------------------

     --------------------------------------------------*/
    void BibView::UpdatePages()
    {
        // TODO:
        // this is _strange_: Why not updating the existent general page?
        // I consider the current behaviour a HACK.
        // frank.schoenheit@sun.com
        if ( m_pGeneralPage )
        {
            m_pGeneralPage->Hide();
            m_pGeneralPage->RemoveListeners();
            m_xGeneralPage = 0;
        }

        m_xGeneralPage = m_pGeneralPage = new BibGeneralPage( this, m_pDatMan );

        Resize();

        if( HasFocus() )
            // "delayed" GetFocus() because GetFocus() is initially called before GeneralPage is created
            m_pGeneralPage->GrabFocus();

        String sErrorString( m_pGeneralPage->GetErrorString() );
        if ( sErrorString.Len() )
        {
            sal_Bool bExecute = sal_True;
            if(!m_pDatMan->HasActiveConnection())
            {
                //no connection is available -> the data base has to be assigned
                m_pDatMan->DispatchDBChangeDialog();
                bExecute = sal_False;
            }
            else if(BibModul::GetConfig()->IsShowColumnAssignmentWarning())
            {
                sErrorString += '\n';
                sErrorString += String( BibResId( RID_MAP_QUESTION ) );
                QueryBox aQuery( this, WB_YES_NO, sErrorString );
                aQuery.SetDefaultCheckBoxText();
                short nResult = aQuery.Execute();
                BibModul::GetConfig()->SetShowColumnAssignmentWarning(
                    !aQuery.GetCheckBoxState());
                if( RET_YES != nResult )
                {
                    bExecute = sal_False;
                }
            }
            if(bExecute)
            {
                Application::PostUserEvent( STATIC_LINK( this, BibView, CallMappingHdl ) );
            }
        }
    }
    //---------------------------------------------------------------------
    //--- 19.10.01 16:55:49 -----------------------------------------------

    void BibView::_loaded( const EventObject& _rEvent )
    {
        UpdatePages();
        FormControlContainer::_loaded( _rEvent );
    }

    void BibView::_reloaded( const EventObject& _rEvent )
    {
        UpdatePages();
        FormControlContainer::_loaded( _rEvent );
    }

    /* -----------------------------02.02.00 16:49--------------------------------

     ---------------------------------------------------------------------------*/
    IMPL_STATIC_LINK( BibView, CallMappingHdl, BibView*, EMPTYARG )
    {
        pThis->m_pDatMan->CreateMappingDialog( pThis );
        return 0;
    }
    /* -----------------------------13.04.00 16:12--------------------------------

     ---------------------------------------------------------------------------*/
    void BibView::Resize()
    {
        if ( m_pGeneralPage )
        {
            ::Size aSz( GetOutputSizePixel() );
            m_pGeneralPage->SetSizePixel( aSz );
        }
        Window::Resize();
    }

    //---------------------------------------------------------------------
    //--- 18.10.01 18:52:45 -----------------------------------------------

    Reference< awt::XControlContainer > BibView::getControlContainer()
    {
        Reference< awt::XControlContainer > xReturn;
        if ( m_pGeneralPage )
            xReturn = m_pGeneralPage->GetControlContainer();
        return xReturn;
    }

    void BibView::GetFocus()
    {
        if( m_pGeneralPage )
            m_pGeneralPage->GrabFocus();
    }

    BOOL BibView::HandleShortCutKey( const KeyEvent& rKeyEvent )
    {
        return m_pGeneralPage? m_pGeneralPage->HandleShortCutKey( rKeyEvent ) : FALSE;
    }

//.........................................................................
}   // namespace bib
//.........................................................................
