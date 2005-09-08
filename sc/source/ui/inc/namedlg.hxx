/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: namedlg.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:38:57 $
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

#ifndef SC_NAMEDLG_HXX
#define SC_NAMEDLG_HXX

#ifndef _MOREBTN_HXX //autogen
#include <vcl/morebtn.hxx>
#endif
#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

#ifndef SC_RANGENAM_HXX
#include "rangenam.hxx"
#endif

#ifndef SC_ANYREFDG_HXX
#include "anyrefdg.hxx"
#endif

class ScViewData;
class ScDocument;


//==================================================================

class ScNameDlg : public ScAnyRefDlg
{
private:
    FixedLine       aFlName;
    ComboBox        aEdName;

    FixedLine       aFlAssign;
    ScRefEdit       aEdAssign;
    ScRefButton     aRbAssign;

    FixedLine       aFlType;
    CheckBox        aBtnPrintArea;
    CheckBox        aBtnColHeader;
    CheckBox        aBtnCriteria;
    CheckBox        aBtnRowHeader;

    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    PushButton      aBtnAdd;
    PushButton      aBtnRemove;
    MoreButton      aBtnMore;
    BOOL            bSaved;

    const String    aStrAdd;    // "Hinzufuegen"
    const String    aStrModify; // "Aendern"
    const String    errMsgInvalidSym;

    ScViewData*     pViewData;
    ScDocument*     pDoc;
    ScRangeName     aLocalRangeName;
    const ScAddress theCursorPos;
    Selection       theCurSel;

#ifdef _NAMEDLG_CXX
private:
    void Init();
    void UpdateChecks();
    void UpdateNames();
    void CalcCurTableAssign( String& aAssign, USHORT nPos );

    // Handler:
    DECL_LINK( OkBtnHdl, void * );
    DECL_LINK( CancelBtnHdl, void * );
    DECL_LINK( AddBtnHdl, void * );
    DECL_LINK( RemoveBtnHdl, void * );
    DECL_LINK( EdModifyHdl, Edit * );
    DECL_LINK( NameSelectHdl, void * );
    DECL_LINK( AssignGetFocusHdl, void * );
#endif

protected:

    virtual void    RefInputDone( BOOL bForced = FALSE );


public:
                    ScNameDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                               ScViewData*      ptrViewData,
                               const ScAddress& aCursorPos );
                    ~ScNameDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual BOOL    IsRefInputMode() const;

    virtual void    SetActive();
    virtual BOOL    Close();

};



#endif // SC_NAMEDLG_HXX

