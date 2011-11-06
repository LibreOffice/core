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



#ifndef FORMULA_FUNCUTL_HXX
#define FORMULA_FUNCUTL_HXX

// #include <vcl/scrbar.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include "formula/formuladllapi.h"


namespace formula
{
    class IControlReferenceHandler;

class FORMULA_DLLPUBLIC RefEdit : public Edit
{
private:
    Timer               aTimer;
    IControlReferenceHandler*      pAnyRefDlg;         // parent dialog
    sal_Bool                bSilentFocus;       // for SilentGrabFocus()

    DECL_LINK( UpdateHdl, Timer* );

protected:
    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        GetFocus();
    virtual void        LoseFocus();

public:
                        RefEdit( Window* _pParent,IControlReferenceHandler* pParent, const ResId& rResId );
                        RefEdit( Window* pParent, const ResId& rResId );
    virtual             ~RefEdit();

    void                SetRefString( const XubString& rStr );
    using Edit::SetText;
    virtual void        SetText( const XubString& rStr );
    virtual void        Modify();

    void                StartUpdateData();

    void                SilentGrabFocus();  // does not update any references

    void                SetRefDialog( IControlReferenceHandler* pDlg );
    inline IControlReferenceHandler* GetRefDialog() { return pAnyRefDlg; }
};


//============================================================================

class FORMULA_DLLPUBLIC RefButton : public ImageButton
{
private:
    Image               aImgRefStart;   /// Start reference input
    Image               aImgRefStartHC; /// Start reference input (high contrast)
    Image               aImgRefDone;    /// Stop reference input
    Image               aImgRefDoneHC;  /// Stop reference input (high contrast)
    IControlReferenceHandler*      pAnyRefDlg;     // parent dialog
    RefEdit*            pRefEdit;       // zugeordnetes Edit-Control

protected:
    virtual void        Click();
    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        GetFocus();
    virtual void        LoseFocus();

public:
                        RefButton( Window* _pParent, const ResId& rResId);
                        RefButton( Window* _pParent, const ResId& rResId, RefEdit* pEdit ,IControlReferenceHandler* pDlg);

    void                SetReferences( IControlReferenceHandler* pDlg, RefEdit* pEdit );

    void                SetStartImage();
    void                SetEndImage();
    inline void         DoRef() { Click(); }
};

} // formula
#endif // FORMULA_FUNCUTL_HXX

