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



#ifndef SD_FU_FORMATPAINTBRUSH_HXX
#define SD_FU_FORMATPAINTBRUSH_HXX

#include "futext.hxx"

// header for class SfxItemSet
#include <svl/itemset.hxx>
#include <boost/scoped_ptr.hpp>

namespace sd {

class DrawViewShell;

class FuFormatPaintBrush : public FuText
{
public:
    static FunctionReference Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq );

    virtual bool MouseMove(const MouseEvent& rMEvt);
    virtual bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual bool MouseButtonDown(const MouseEvent& rMEvt);
    virtual bool KeyInput(const KeyEvent& rKEvt);

    virtual void Activate();
    virtual void Deactivate();

    static void GetMenuState( DrawViewShell& rDrawViewShell, SfxItemSet &rSet );
    static bool CanCopyThisType( sal_uInt32 nObjectInventor, sal_uInt16 nObjectIdentifier );

private:
    FuFormatPaintBrush ( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq);

    void DoExecute( SfxRequest& rReq );

    bool HasContentForThisType(const SdrObject& rSdrObject) const;
    void Paste( bool, bool );

    void implcancel();

    ::boost::shared_ptr<SfxItemSet> mpItemSet;
    bool   mbPermanent;
    bool   mbOldIsQuickTextEditMode;
};

} // end of namespace sd

#endif
