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



#ifndef _SV_DIALOG_HXX
#define _SV_DIALOG_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/syswin.hxx>

// parameter to pass to the dialogue constructor if really no parent is wanted
// whereas NULL chooses the deafult dialogue parent
#define DIALOG_NO_PARENT ((Window*)0xffffffff)

// ----------
// - Dialog -
// ----------

struct DialogImpl;

class VCL_DLLPUBLIC Dialog : public SystemWindow
{
private:
    Window*         mpDialogParent;
    Dialog*         mpPrevExecuteDlg;
    DialogImpl*     mpDialogImpl;
    long            mnMousePositioned;
    sal_Bool            mbInExecute;
    sal_Bool            mbOldSaveBack;
    sal_Bool            mbInClose;
    sal_Bool            mbModalMode;

    SAL_DLLPRIVATE void    ImplInitDialogData();
    SAL_DLLPRIVATE void    ImplInitSettings();

//#if 0 // _SOLAR__PRIVATE
    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE         Dialog (const Dialog &);
    SAL_DLLPRIVATE         Dialog & operator= (const Dialog &);

    DECL_DLLPRIVATE_LINK( ImplAsyncCloseHdl, void* );
protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void    ImplDialogRes( const ResId& rResId );
    SAL_DLLPRIVATE void    ImplCenterDialog();

public:
    SAL_DLLPRIVATE sal_Bool    IsInClose() const { return mbInClose; }
//#endif

protected:
                    Dialog( WindowType nType );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );

public:
    explicit        Dialog( Window* pParent, WinBits nStyle = WB_STDDIALOG );
    explicit        Dialog( Window* pParent, const ResId& );
    virtual         ~Dialog();

    virtual long    Notify( NotifyEvent& rNEvt );
    virtual void    StateChanged( StateChangedType nStateChange );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    virtual sal_Bool    Close();

    virtual short   Execute();
    sal_Bool            IsInExecute() const { return mbInExecute; }

    ////////////////////////////////////////
    // Dialog::Execute replacement API
public:
    // Link impl: DECL_LINK( MyEndDialogHdl, Dialog* ); <= param is dialog just ended
    virtual void    StartExecuteModal( const Link& rEndDialogHdl );
    sal_Bool            IsStartedModal() const;
    long            GetResult() const;
private:
    sal_Bool            ImplStartExecuteModal();
    void            ImplEndExecuteModal();
public:

    // Dialog::Execute replacement API
    ////////////////////////////////////////

    void            EndDialog( long nResult = 0 );
    static void     EndAllDialogs( Window* pParent=NULL );

    void            GetDrawWindowBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                         sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const;

    void            SetModalInputMode( sal_Bool bModal );
    void            SetModalInputMode( sal_Bool bModal, sal_Bool bSubModalDialogs );
    sal_Bool            IsModalInputMode() const { return mbModalMode; }

    void            GrabFocusToFirstControl();
};

// ------------------
// - ModelessDialog -
// ------------------

class VCL_DLLPUBLIC ModelessDialog : public Dialog
{
//#if 0 // _SOLAR__PRIVATE
    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE         ModelessDialog (const ModelessDialog &);
    SAL_DLLPRIVATE         ModelessDialog & operator= (const ModelessDialog &);
//#endif

public:
    explicit        ModelessDialog( Window* pParent, WinBits nStyle = WB_STDMODELESS );
    explicit        ModelessDialog( Window* pParent, const ResId& );
};

// ---------------
// - ModalDialog -
// ---------------

class VCL_DLLPUBLIC ModalDialog : public Dialog
{
public:
    explicit        ModalDialog( Window* pParent, WinBits nStyle = WB_STDMODAL );
    explicit        ModalDialog( Window* pParent, const ResId& );

private:
    using Window::Show;
    void            Show( sal_Bool bVisible = sal_True );
    using Window::Hide;
    void            Hide();

//#if 0 // _SOLAR__PRIVATE
    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE         ModalDialog (const ModalDialog &);
    SAL_DLLPRIVATE         ModalDialog & operator= (const ModalDialog &);
//#endif
};

#endif  // _SV_DIALOG_HXX

