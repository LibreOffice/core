/*************************************************************************
 *
 *  $RCSfile: pastedlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-10-04 17:48:37 $
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

#ifndef _PASTEDLG_HXX
#define _PASTEDLG_HXX

#ifndef _TABLE_HXX //autogen
#include <tools/table.hxx>
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _GLOBNAME_HXX //autogen
#include <tools/globname.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif

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
    USHORT          nAspect;
    BOOL            bLink;

    ListBox&        ObjectLB()      { return aLbInsertList; }
    FixedText&      ObjectSource()  { return aFtObjectSource; }
    RadioButton&    PasteLink()     { return aRbPasteLink; }
    CheckBox&       AsIconBox()     { return aCbDisplayAsIcon; }

    const String&   GetObjString()  { return aSObject; }
    void            SelectObject();
    DECL_LINK( SelectHdl, ListBox * );
    DECL_LINK( DoubleClickHdl, ListBox * );
    void            SetDefault();
    USHORT      GetAspect() const { return nAspect; }
    BOOL        ShouldLink() const { return bLink; }

public:
                SvPasteObjectDialog( Window* pParent );
                ~SvPasteObjectDialog();

    void        Insert( SotFormatStringId nFormat, const String & rFormatName );
    void        SetObjName( const SvGlobalName & rClass, const String & rObjName );
    ULONG       GetFormat( const TransferableDataHelper& aHelper,
                        const DataFlavorExVector* pFormats=0,
                        const TransferableObjectDescriptor* pDesc=0 );
};

#endif // _PASTEDLG_HXX
