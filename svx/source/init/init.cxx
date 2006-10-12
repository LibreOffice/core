/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: init.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:51:57 $
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
#include "precompiled_svx.hxx"

#include "svxdlg.hxx"
#include "dialogs.hrc"

#include "vos/mutex.hxx"
#include <vcl/edit.hxx>
#include <vcl/svapp.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>

// ------------------------------------------------------------------------
//Sonderzeichen einfuegen fuer Edits

SVX_DLLPUBLIC String GetSpecialCharsForEdit(Window* pParent, const Font& rFont)
{
    String sRet;
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    if(pFact)
    {
        AbstractSvxCharacterMap* aDlg = pFact->CreateSvxCharacterMap( pParent, ResId(RID_SVXDLG_CHARMAP) );
        aDlg->DisableFontSelection();
        aDlg->SetCharFont(rFont);
        if ( aDlg->Execute() == RET_OK )
            sRet = aDlg->GetChar();
        delete aDlg;
    }
    return sRet;
}
