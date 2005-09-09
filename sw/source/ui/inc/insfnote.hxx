/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: insfnote.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:21:39 $
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
#ifndef _INSFNOTE_HXX
#define _INSFNOTE_HXX

#ifndef _SVX_STDDLG_HXX //autogen
#include <svx/stddlg.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

class SwWrtShell;

class SwInsFootNoteDlg: public SvxStandardDialog
{
    SwWrtShell     &rSh;

    //Alles fuer das/die Zeichen
    String          aFontName;
    CharSet         eCharSet;
    char            cExtChar;
    BOOL            bExtCharAvailable;
    BOOL            bEdit;
    RadioButton     aNumberAutoBtn;
    RadioButton     aNumberCharBtn;
    Edit            aNumberCharEdit;
    PushButton      aNumberExtChar;
    FixedLine       aNumberFL;

    //Alles fuer die Auswahl Fussnote/Endnote
    RadioButton     aFtnBtn;
    RadioButton     aEndNoteBtn;
    FixedLine        aTypeFL;

    OKButton        aOkBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;
    ImageButton     aPrevBT;
    ImageButton     aNextBT;

    DECL_LINK( NumberCharHdl, Button * );
    DECL_LINK( NumberEditHdl, void * );
    DECL_LINK( NumberAutoBtnHdl, Button *);
    DECL_LINK( NumberExtCharHdl, Button *);
    DECL_LINK( NextPrevHdl, Button * );

    virtual void    Apply();

    void            Init();

public:
    SwInsFootNoteDlg(Window * pParent, SwWrtShell &rSh, BOOL bEd = FALSE);
    ~SwInsFootNoteDlg();

    CharSet         GetCharSet() { return eCharSet; }
    BOOL            IsExtCharAvailable() { return bExtCharAvailable; }
    String          GetFontName() { return aFontName; }
    BOOL            IsEndNote() { return aEndNoteBtn.IsChecked(); }
    String          GetStr()
                    {
                        if ( aNumberCharBtn.IsChecked() )
                            return aNumberCharEdit.GetText();
                        else
                            return String();
                    }
};

#endif
