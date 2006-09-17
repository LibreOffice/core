/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fileurlbox.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 14:38:11 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#ifndef SVTOOLS_FILEURLBOX_HXX
#include "fileurlbox.hxx"
#endif
#ifndef _OSL_FILE_H_
#include <osl/file.h>
#endif
#ifndef SVTOOLS_FILENOTATION_HXX
#include "filenotation.hxx"
#endif

//.........................................................................
namespace svt
{
//.........................................................................

    //=====================================================================
    //= FileURLBox
    //=====================================================================
    //---------------------------------------------------------------------
    FileURLBox::FileURLBox(Window* _pParent)
        :SvtURLBox(_pParent, INET_PROT_FILE)
    {
        DisableHistory();
    }

    //---------------------------------------------------------------------
    FileURLBox::FileURLBox( Window* _pParent, WinBits _nStyle )
        :SvtURLBox( _pParent, _nStyle, INET_PROT_FILE )
    {
        DisableHistory();
    }

    //---------------------------------------------------------------------
    FileURLBox::FileURLBox(Window* _pParent, const ResId& _rId)
        :SvtURLBox(_pParent, _rId, INET_PROT_FILE)
    {
        DisableHistory();
    }

    //---------------------------------------------------------------------
    void FileURLBox::DisplayURL( const String& _rURL )
    {
        String sOldText = GetText();

        OFileNotation aTransformer( _rURL, OFileNotation::N_URL );
        String sNewText = aTransformer.get( OFileNotation::N_SYSTEM );
        SetText( sNewText );

        if ( sOldText != sNewText )
            Modify();

        UpdatePickList();
    }

    //---------------------------------------------------------------------
    long FileURLBox::PreNotify( NotifyEvent& _rNEvt )
    {
        switch ( _rNEvt.GetType() )
        {
        case EVENT_KEYINPUT:
            if  (   ( GetSubEdit() == _rNEvt.GetWindow()                         )
                &&  ( KEY_RETURN == _rNEvt.GetKeyEvent()->GetKeyCode().GetCode() )
                &&  ( IsInDropDown()                                             )
                )
                m_sPreservedText = GetURL();
            break;

        case EVENT_LOSEFOCUS:
            if ( IsWindowOrChild( _rNEvt.GetWindow() ) )
                DisplayURL( GetText() );
            break;
        }

        return SvtURLBox::PreNotify(_rNEvt);
    }

    //---------------------------------------------------------------------
    long FileURLBox::Notify( NotifyEvent& _rNEvt )
    {
        switch ( _rNEvt.GetType() )
        {
        case EVENT_KEYINPUT:
            if  (   ( GetSubEdit() == _rNEvt.GetWindow()                         )
                &&  ( KEY_RETURN == _rNEvt.GetKeyEvent()->GetKeyCode().GetCode() )
                &&  ( IsInDropDown()                                             )
                )
            {
                long nReturn = SvtURLBox::Notify(_rNEvt);
                DisplayURL( m_sPreservedText );
                return nReturn;
            }
            break;
        }

        return SvtURLBox::Notify(_rNEvt);
    }

//.........................................................................
}   // namespace svt
//.........................................................................

