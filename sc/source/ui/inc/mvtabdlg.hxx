/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mvtabdlg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:38:06 $
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

#ifndef SC_MVTABDLG_HXX
#define SC_MVTABDLG_HXX


#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

//------------------------------------------------------------------------

class ScMoveTableDlg : public ModalDialog
{
public:
                    ScMoveTableDlg( Window* pParent );
                    ~ScMoveTableDlg();

    USHORT  GetSelectedDocument     () const;
    SCTAB   GetSelectedTable        () const;
    BOOL    GetCopyTable            () const;
    void    SetCopyTable            (BOOL bFlag=TRUE);
    void    EnableCopyTable         (BOOL bFlag=TRUE);

private:
    FixedText       aFtDoc;
    ListBox         aLbDoc;
    FixedText       aFtTable;
    ListBox         aLbTable;
    CheckBox        aBtnCopy;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    USHORT          nDocument;
    SCTAB           nTable;
    BOOL            bCopyTable;
    //--------------------------------------
    void    Init            ();
    void    InitDocListBox  ();
    DECL_LINK( OkHdl, void * );
    DECL_LINK( SelHdl, ListBox * );
};


#endif // SC_MVTABDLG_HXX


