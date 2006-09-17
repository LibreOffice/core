/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: urlcontrol.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 14:42:27 $
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

#ifndef SVTOOL_URLCONTROL_HXX
#include "urlcontrol.hxx"
#endif
#ifndef SVTOOLS_FILENOTATION_HXX_
#include "filenotation.hxx"
#endif

//.........................................................................
namespace svt
{
//.........................................................................

    //=====================================================================
    //= OFileURLControl
    //=====================================================================
    //---------------------------------------------------------------------
    OFileURLControl::OFileURLControl(Window* _pParent)
        :SvtURLBox(_pParent, INET_PROT_FILE)
    {
        DisableHistory();
    }

    //---------------------------------------------------------------------
    OFileURLControl::OFileURLControl(Window* _pParent, const ResId& _rId)
        :SvtURLBox(_pParent, _rId, INET_PROT_FILE)
    {
        DisableHistory();
    }

    //---------------------------------------------------------------------
    long OFileURLControl::PreNotify( NotifyEvent& _rNEvt )
    {
        if (GetSubEdit() == _rNEvt.GetWindow())
            if (EVENT_KEYINPUT == _rNEvt.GetType())
                if (KEY_RETURN == _rNEvt.GetKeyEvent()->GetKeyCode().GetCode())
                    if (IsInDropDown())
                        m_sPreservedText = GetURL();

        return SvtURLBox::PreNotify(_rNEvt);
    }

    //---------------------------------------------------------------------
    long OFileURLControl::Notify( NotifyEvent& _rNEvt )
    {
        if (GetSubEdit() == _rNEvt.GetWindow())
            if (EVENT_KEYINPUT == _rNEvt.GetType())
                if (KEY_RETURN == _rNEvt.GetKeyEvent()->GetKeyCode().GetCode())
                    if (IsInDropDown())
                    {
                        long nReturn = SvtURLBox::Notify(_rNEvt);

                        // build a system dependent (thus more user readable) file name
                        OFileNotation aTransformer(m_sPreservedText, OFileNotation::N_URL);
                        SetText(aTransformer.get(OFileNotation::N_SYSTEM));
                        Modify();

                        // Update the pick list
                        UpdatePickList();

                        return nReturn;
                    }

        return SvtURLBox::Notify(_rNEvt);
    }

//.........................................................................
}   // namespace svt
//.........................................................................

