/*************************************************************************
 *
 *  $RCSfile: fileurlbox.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-03-19 12:26:56 $
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

