/*************************************************************************
 *
 *  $RCSfile: filtdlg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:58 $
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

#ifndef SC_FILTDLG_HXX
#define SC_FILTDLG_HXX

#ifndef VCL
#endif

#ifndef _MOREBTN_HXX //autogen
#include <vcl/morebtn.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx" // -> ScQueryParam
#endif

#ifndef SC_ANYREFDG_HXX
#include "anyrefdg.hxx"
#endif


//----------------------------------------------------------------------------

class ScFilterOptionsMgr;
class ScRangeData;
class ScViewData;
class ScDocument;
class ScQueryItem;
class TypedStrCollection;

//==================================================================
// Gemeinsame Resource-Objekte:

#define _COMMON_FILTER_RSCOBJS \
    CheckBox        aBtnCase; \
    CheckBox        aBtnRegExp; \
    CheckBox        aBtnHeader; \
    CheckBox        aBtnUnique; \
    CheckBox        aBtnCopyResult; \
    ListBox         aLbCopyArea; \
    ScRefEdit       aEdCopyArea; \
    ScRefButton     aRbCopyArea; \
    CheckBox        aBtnDestPers; \
    FixedText       aFtDbAreaLabel; \
    FixedInfo       aFtDbArea; \
    GroupBox        aGbOptions; \
    OKButton        aBtnOk; \
    CancelButton    aBtnCancel; \
    HelpButton      aBtnHelp; \
    MoreButton      aBtnMore; \
    const String    aStrUndefined; \
    const String    aStrNoName; \
    const String    aStrNone;


#define _INIT_COMMON_FILTER_RSCOBJS \
    aBtnCase        ( this, ScResId( BTN_CASE ) ), \
    aBtnRegExp      ( this, ScResId( BTN_REGEXP ) ), \
    aBtnHeader      ( this, ScResId( BTN_HEADER ) ), \
    aBtnUnique      ( this, ScResId( BTN_UNIQUE ) ), \
    aBtnCopyResult  ( this, ScResId( BTN_COPY_RESULT ) ), \
    aLbCopyArea     ( this, ScResId( LB_COPY_AREA ) ), \
    aEdCopyArea     ( this, ScResId( ED_COPY_AREA ) ), \
    aRbCopyArea     ( this, ScResId( RB_COPY_AREA ), &aEdCopyArea ), \
    aBtnDestPers    ( this, ScResId( BTN_DEST_PERS ) ), \
    aFtDbAreaLabel  ( this, ScResId( FT_DBAREA_LABEL ) ), \
    aFtDbArea       ( this, ScResId( FT_DBAREA ) ), \
    aGbOptions      ( this, ScResId( GB_OPTIONS ) ), \
    aBtnOk          ( this, ScResId( BTN_OK ) ), \
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ), \
    aBtnHelp        ( this, ScResId( BTN_HELP ) ), \
    aBtnMore        ( this, ScResId( BTN_MORE ) ), \
    aStrNoName      ( ScGlobal::GetRscString(STR_DB_NONAME) ), \
    aStrNone        ( ScResId( SCSTR_NONE ) ), \
    aStrUndefined   ( ScResId( SCSTR_UNDEFINED ) ),


//============================================================================
class ScFilterDlg : public ScAnyRefDlg
{
public:
                    ScFilterDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                 const SfxItemSet&  rArgSet );
                    ~ScFilterDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );

    virtual BOOL    IsRefInputMode() const;
    virtual void    SetActive();

    virtual BOOL    Close();

private:
    ListBox         aLbField1;
    ListBox         aLbCond1;
    ComboBox        aEdVal1;
    //----------------------------
    ListBox         aLbConnect1;
    ListBox         aLbField2;
    ListBox         aLbCond2;
    ComboBox        aEdVal2;
    //----------------------------
    ListBox         aLbConnect2;
    ListBox         aLbField3;
    ListBox         aLbCond3;
    ComboBox        aEdVal3;
    //----------------------------
    FixedText       aFtConnect;
    FixedText       aFtField;
    FixedText       aFtCond;
    FixedText       aFtVal;
    GroupBox        aGbCriteria;

    _COMMON_FILTER_RSCOBJS

    const String    aStrEmpty;
    const String    aStrNotEmpty;
    const String    aStrRow;
    const String    aStrColumn;

    ScFilterOptionsMgr* pOptionsMgr;

    const USHORT        nWhichQuery;
    const ScQueryParam  theQueryData;
    ScQueryItem*        pOutItem;
    ScViewData*         pViewData;
    ScDocument*         pDoc;
    USHORT              nSrcTab;

    ComboBox*           aValueEdArr[3];
    ListBox*            aFieldLbArr[3];
    ListBox*            aCondLbArr[3];
    USHORT              nFieldCount;
    BOOL                bRefInputMode;

    TypedStrCollection* pEntryLists[MAXCOL+1];
    USHORT              nHeaderPos[MAXCOL+1];

    // Hack: RefInput-Kontrolle
    Timer*  pTimer;

#ifdef _FILTDLG_CXX
private:
    void            Init            ( const SfxItemSet& rArgSet );
    void            FillFieldLists  ();
    void            FillAreaList    ();
    void            UpdateValueList ( USHORT nList );
    void            UpdateHdrInValueList( USHORT nList );
    void            ClearValueList  ( USHORT nList );
    USHORT          GetFieldSelPos  ( USHORT nField );
    ScQueryItem*    GetOutputItem   ();

    // Handler:
    DECL_LINK( LbSelectHdl,  ListBox* );
    DECL_LINK( ValModifyHdl, ComboBox* );
    DECL_LINK( CheckBoxHdl,  CheckBox* );
    DECL_LINK( EndDlgHdl,    Button* );
    DECL_LINK( MoreClickHdl, MoreButton* );

    // Hack: RefInput-Kontrolle
    DECL_LINK( TimeOutHdl,   Timer* );
#endif
};


//============================================================================
class ScSpecialFilterDlg : public ScAnyRefDlg
{
public:
                    ScSpecialFilterDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                        const SfxItemSet&   rArgSet );
                    ~ScSpecialFilterDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );

    virtual BOOL    IsRefInputMode() const;
    virtual void    SetActive();

    virtual BOOL    Close();

private:
    ListBox     aLbFilterArea;
    FixedText   aFtFilterArea;
    ScRefEdit   aEdFilterArea;
    ScRefButton aRbFilterArea;

    _COMMON_FILTER_RSCOBJS

    ScFilterOptionsMgr* pOptionsMgr;

    const USHORT        nWhichQuery;
    const ScQueryParam  theQueryData;
    ScQueryItem*        pOutItem;
    ScViewData*         pViewData;
    ScDocument*         pDoc;

    ScRefEdit*          pRefInputEdit;
    BOOL                bRefInputMode;

    // Hack: RefInput-Kontrolle
    Timer*  pTimer;

#ifdef _SFILTDLG_CXX
private:
    void            Init( const SfxItemSet& rArgSet );
    ScQueryItem*    GetOutputItem( const ScQueryParam& rParam,
                                    const ScRange& rSource );

    // Handler
    DECL_LINK( FilterAreaSelHdl, ListBox* );
    DECL_LINK( FilterAreaModHdl, ScRefEdit* );
    DECL_LINK( EndDlgHdl,        Button* );

    // Hack: RefInput-Kontrolle
    DECL_LINK( TimeOutHdl,       Timer* );
#endif
};



#endif // SC_FILTDLG_HXX

