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



#ifndef SD_FU_TEXT_HXX
#define SD_FU_TEXT_HXX

#ifndef _EDITDATA_HXX
#include <editeng/editdata.hxx>
#endif
#include "fuconstr.hxx"
#include <svx/svdotext.hxx>

struct StyleRequestData;
class SdrTextObj;
class OutlinerParaObject;
class FontList;
class OutlinerView;

namespace sd {

/*************************************************************************
|*
|* Basisklasse fuer Textfunktionen
|*
\************************************************************************/

class FuText
    : public FuConstruct
{
public:
    TYPEINFO();

    static FunctionReference Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq );
    virtual void DoExecute( SfxRequest& rReq );

    virtual sal_Bool KeyInput(const KeyEvent& rKEvt);
    virtual sal_Bool MouseMove(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt);
    virtual sal_Bool Command(const CommandEvent& rCEvt);
    virtual sal_Bool RequestHelp(const HelpEvent& rHEvt);
    virtual void ReceiveRequest(SfxRequest& rReq);
    virtual void DoubleClick(const MouseEvent& rMEvt);

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren

    void    SetInEditMode(const MouseEvent& rMEvt, sal_Bool bQuickDrag);
    sal_Bool    DeleteDefaultText();
    SdrTextObj* GetTextObj() { return static_cast< SdrTextObj* >( mxTextObj.get() ); }

    DECL_LINK(SpellError, void* );

    // #97016#
    virtual SdrObject* CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle);

    /** is called when the currenct function should be aborted. <p>
        This is used when a function gets a KEY_ESCAPE but can also
        be called directly.

        @returns true if a active function was aborted
    */
    virtual bool cancel();

    static void ChangeFontSize( bool, OutlinerView*, const FontList*, ::sd::View* );

protected:
    FuText (ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);

    virtual void disposing();

    SdrObjectWeakRef    mxTextObj;
    Link                aOldLink;
    sal_Bool                bFirstObjCreated;

    SfxRequest&         rRequest;

private:
    // #97016#
    void ImpSetAttributesForNewTextObject(SdrTextObj* pTxtObj);
    void ImpSetAttributesFitToSize(SdrTextObj* pTxtObj);
    void ImpSetAttributesFitToSizeVertical(SdrTextObj* pTxtObj);
    void ImpSetAttributesFitCommon(SdrTextObj* pTxtObj);
};

} // end of namespace sd

#endif
