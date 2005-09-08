/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pastedlg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:53:21 $
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
