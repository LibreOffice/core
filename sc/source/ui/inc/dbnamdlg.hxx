/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbnamdlg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:20:20 $
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

#ifndef SC_DBNAMDLG_HXX
#define SC_DBNAMDLG_HXX

#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _MOREBTN_HXX //autogen
#include <vcl/morebtn.hxx>
#endif

#ifndef SC_ANYREFDG_HXX
#include "anyrefdg.hxx"
#endif

#ifndef SC_DBCOLECT_HXX
#include "dbcolect.hxx"
#endif

#ifndef _SC_EXPFTEXT_HXX
#include "expftext.hxx"
#endif

class ScViewData;
class ScDocument;


//============================================================================

class ScDbNameDlg : public ScAnyRefDlg
{
public:
                    ScDbNameDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                 ScViewData*    ptrViewData );
                    ~ScDbNameDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );

    virtual BOOL    IsRefInputMode() const;
    virtual void    SetActive();
    virtual BOOL    Close();

private:
    FixedLine           aFlName;
    ComboBox            aEdName;

    FixedLine           aFlAssign;
    ScRefEdit           aEdAssign;
    ScRefButton         aRbAssign;

    FixedLine           aFlOptions;
    CheckBox            aBtnHeader;
    CheckBox            aBtnDoSize;
    CheckBox            aBtnKeepFmt;
    CheckBox            aBtnStripData;
    ScExpandedFixedText aFTSource;     //@18.09.97 erweiterter FixedText
    FixedText           aFTOperations;

    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    PushButton      aBtnAdd;
    PushButton      aBtnRemove;
    MoreButton      aBtnMore;

    BOOL            bSaved;


    const String    aStrAdd;    // "Hinzufuegen"
    const String    aStrModify; // "Aendern"
    const String    aStrNoName; // "unbenannt"
    const String    aStrInvalid;

    String          aStrSource;
    String          aStrOperations;

    ScViewData*     pViewData;
    ScDocument*     pDoc;
    BOOL            bRefInputMode;

    ScDBCollection  aLocalDbCol;
    ScRange         theCurArea;
    List            aRemoveList;

#ifdef _DBNAMDLG_CXX
private:
    void            Init();
    void            UpdateNames();
    void            UpdateDBData( const String& rStrName );
    void            SetInfoStrings( const ScDBData* pDBData );

    DECL_LINK( CancelBtnHdl, void * );
    DECL_LINK( OkBtnHdl, void * );
    DECL_LINK( AddBtnHdl, void * );
    DECL_LINK( RemoveBtnHdl, void * );
    DECL_LINK( NameModifyHdl, void * );
    DECL_LINK( AssModifyHdl, void * );
#endif
};



#endif // SC_DBNAMDLG_HXX

