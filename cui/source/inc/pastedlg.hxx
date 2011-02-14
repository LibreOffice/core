/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _PASTEDLG_HXX
#define _PASTEDLG_HXX

#ifndef _TABLE_HXX //autogen
#include <tools/table.hxx>
#endif
#include <sot/formats.hxx>
#include <tools/globname.hxx>
#include <svtools/transfer.hxx>

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>

/********************** SvPasteObjectDialog ******************************
*************************************************************************/

struct TransferableObjectDescriptor;
class DataFlavorExVecor;
class TransferableDataHelper;

class SvPasteObjectDialog : public ModalDialog
{
    FixedText aFtSource;
    FixedText aFtObjectSource;
    RadioButton aRbPaste;
    RadioButton aRbPasteLink;
    ListBox aLbInsertList;
    CheckBox aCbDisplayAsIcon;
    PushButton aPbChangeIcon;
    FixedLine aFlChoice;
    OKButton aOKButton1;
    CancelButton aCancelButton1;
    HelpButton aHelpButton1;
    String aSObject;
    Table           aSupplementTable;
    SvGlobalName    aObjClassName;
    String          aObjName;
    sal_uInt16          nAspect;
    sal_Bool            bLink;

    ListBox&        ObjectLB()      { return aLbInsertList; }
    FixedText&      ObjectSource()  { return aFtObjectSource; }
    RadioButton&    PasteLink()     { return aRbPasteLink; }
    CheckBox&       AsIconBox()     { return aCbDisplayAsIcon; }

    const String&   GetObjString()  { return aSObject; }
    void            SelectObject();
    DECL_LINK( SelectHdl, ListBox * );
    DECL_LINK( DoubleClickHdl, ListBox * );
    void            SetDefault();
    sal_uInt16      GetAspect() const { return nAspect; }
    sal_Bool        ShouldLink() const { return bLink; }

public:
                SvPasteObjectDialog( Window* pParent );
                ~SvPasteObjectDialog();

    void        Insert( SotFormatStringId nFormat, const String & rFormatName );
    void        SetObjName( const SvGlobalName & rClass, const String & rObjName );
    sal_uLong       GetFormat( const TransferableDataHelper& aHelper,
                        const DataFlavorExVector* pFormats=0,
                        const TransferableObjectDescriptor* pDesc=0 );
};

#endif // _PASTEDLG_HXX
