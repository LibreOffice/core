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



#ifndef _SV_BTNDLG_HXX
#define _SV_BTNDLG_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/dialog.hxx>

struct ImplBtnDlgItem;
class ImplBtnDlgItemList;
class PushButton;

// ----------------------
// - ButtonDialog-Types -
// ----------------------

#define BUTTONDIALOG_BUTTON_NOTFOUND            ((sal_uInt16)0xFFFF)

#define BUTTONDIALOG_DEFBUTTON                  ((sal_uInt16)0x0001)
#define BUTTONDIALOG_OKBUTTON                   ((sal_uInt16)0x0002)
#define BUTTONDIALOG_CANCELBUTTON               ((sal_uInt16)0x0004)
#define BUTTONDIALOG_HELPBUTTON                 ((sal_uInt16)0x0008)
#define BUTTONDIALOG_FOCUSBUTTON                ((sal_uInt16)0x0010)

// ----------------
// - ButtonDialog -
// ----------------

class VCL_DLLPUBLIC ButtonDialog : public Dialog
{
private:
    ImplBtnDlgItemList* mpItemList;
    Size                maPageSize;
    Size                maCtrlSize;
    long                mnButtonSize;
    sal_uInt16              mnCurButtonId;
    sal_uInt16              mnFocusButtonId;
    sal_Bool                mbFormat;
    Link                maClickHdl;

    SAL_DLLPRIVATE void             ImplInitButtonDialogData();
    SAL_DLLPRIVATE PushButton*      ImplCreatePushButton( sal_uInt16 nBtnFlags );
    SAL_DLLPRIVATE ImplBtnDlgItem*  ImplGetItem( sal_uInt16 nId ) const;
    DECL_DLLPRIVATE_LINK(           ImplClickHdl, PushButton* pBtn );
    SAL_DLLPRIVATE void             ImplPosControls();

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE      ButtonDialog( const ButtonDialog & );
    SAL_DLLPRIVATE      ButtonDialog& operator=( const ButtonDialog& );
protected:
                        ButtonDialog( WindowType nType );
    SAL_DLLPRIVATE long ImplGetButtonSize();

public:
                        ButtonDialog( Window* pParent, WinBits nStyle = WB_STDDIALOG );
                        ButtonDialog( Window* pParent, const ResId& rResId );
                        ~ButtonDialog();

    virtual void        Resize();
    virtual void        StateChanged( StateChangedType nStateChange );

    virtual void        Click();

    void                SetPageSizePixel( const Size& rSize ) { maPageSize = rSize; }
    const Size&         GetPageSizePixel() const { return maPageSize; }

    sal_uInt16              GetCurButtonId() const { return mnCurButtonId; }

    void                AddButton( const XubString& rText, sal_uInt16 nId, sal_uInt16 nBtnFlags, long nSepPixel = 0 );
    void                AddButton( StandardButtonType eType, sal_uInt16 nId, sal_uInt16 nBtnFlags, long nSepPixel = 0 );
    void                AddButton( PushButton* pBtn, sal_uInt16 nId, sal_uInt16 nBtnFlags, long nSepPixel = 0 );
    void                RemoveButton( sal_uInt16 nId );
    void                Clear();
    sal_uInt16              GetButtonCount() const;
    sal_uInt16              GetButtonId( sal_uInt16 nButton ) const;
    PushButton*         GetPushButton( sal_uInt16 nId ) const;
    void                SetButtonText( sal_uInt16 nId, const XubString& rText );
    XubString           GetButtonText( sal_uInt16 nId ) const;
    void                SetButtonHelpText( sal_uInt16 nId, const XubString& rText );
    XubString           GetButtonHelpText( sal_uInt16 nId ) const;
    void                SetButtonHelpId( sal_uInt16 nId, const rtl::OString& rHelpId );
    rtl::OString        GetButtonHelpId( sal_uInt16 nId ) const;

    void                SetFocusButton( sal_uInt16 nId = BUTTONDIALOG_BUTTON_NOTFOUND ) { mnFocusButtonId = nId; }
    sal_uInt16              GetFocusButton() const { return mnFocusButtonId; }

    void                SetClickHdl( const Link& rLink ) { maClickHdl = rLink; }
    const Link&         GetClickHdl() const { return maClickHdl; }
};

#endif  // _SV_BTNDLG_HXX
