/*************************************************************************
 *
 *  $RCSfile: pfiltdlg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:00 $
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

#ifndef SC_PFILTDLG_HXX
#define SC_PFILTDLG_HXX

#ifndef _SV_HXX
#endif

#ifndef _MOREBTN_HXX //autogen
#include <vcl/morebtn.hxx>
#endif
#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif

#include "global.hxx" // -> ScQueryParam

//------------------------------------------------------------------

class ScViewData;
class ScDocument;
class ScQueryItem;
class TypedStrCollection;

//==================================================================

class ScPivotFilterDlg : public ModalDialog
{
public:
                    ScPivotFilterDlg( Window* pParent,
                                      const SfxItemSet& rArgSet, USHORT nSourceTab );
                    ~ScPivotFilterDlg();

    const ScQueryItem&  GetOutputItem();

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

    CheckBox        aBtnCase;
    CheckBox        aBtnRegExp;
    CheckBox        aBtnUnique;
    FixedText       aFtDbAreaLabel;
    FixedInfo       aFtDbArea;
    GroupBox        aGbOptions;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    MoreButton      aBtnMore;
    const String    aStrUndefined;
    const String    aStrNoName;
    const String    aStrNone;
    const String    aStrEmpty;
    const String    aStrNotEmpty;
    const String    aStrRow;
    const String    aStrColumn;

    const USHORT        nWhichQuery;
    const ScQueryParam  theQueryData;
    ScQueryItem*        pOutItem;
    ScViewData*         pViewData;
    ScDocument*         pDoc;
    USHORT              nSrcTab;

    USHORT              nFieldCount;
    ComboBox*           aValueEdArr[3];
    ListBox*            aFieldLbArr[3];
    ListBox*            aCondLbArr[3];

    TypedStrCollection* pEntryLists[MAXCOL+1];

#ifdef _PFILTDLG_CXX
private:
    void    Init            ( const SfxItemSet& rArgSet );
    void    FillFieldLists  ();
    void    UpdateValueList ( USHORT nList );
    void    ClearValueList  ( USHORT nList );
    USHORT  GetFieldSelPos  ( USHORT nField );

    // Handler:
    DECL_LINK( LbSelectHdl, ListBox* );
    DECL_LINK( ValModifyHdl, ComboBox* );
    DECL_LINK( CheckBoxHdl,  CheckBox* );
#endif
};


#endif // SC_PFILTDLG_HXX

