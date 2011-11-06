/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    CheckBox aCbDisplayAsIcon;
    PushButton aPbChangeIcon;
    FixedLine aFlChoice;
    ListBox aLbInsertList;
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
