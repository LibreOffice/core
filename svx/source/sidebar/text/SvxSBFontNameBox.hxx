/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef SVX_SIDEBAR_SB_FONT_NAME_BOX_HXX
#define SVX_SIDEBAR_SB_FONT_NAME_BOX_HXX

#include <sfx2/bindings.hxx>
#include <svtools/ctrlbox.hxx>
#include <svtools/ctrltool.hxx>

namespace svx
{
    class ToolboxButtonColorUpdater;
}

namespace svx { namespace sidebar {

class SvxSBFontNameBox : public FontNameBox//, public SfxListener
{
private:
    const FontList* pFontList;
    Font            aCurFont;
    OUString        aCurText;
    sal_uInt16      nFtCount;
    void            EnableControls_Impl();
    SfxBindings*    pBindings;//
protected:
    virtual void    Select();

public:
    SvxSBFontNameBox(Window* pParent);
    void            FillList();
    sal_uInt16 GetListCount() const { return nFtCount; }
    void            Clear() { FontNameBox::Clear(); nFtCount = 0; }
    void            Fill( const FontList* pList )
                        { FontNameBox::Fill( pList );
                          nFtCount = pList->GetFontNameCount(); }
    void            SetBindings(SfxBindings* pBinding);//
    virtual long    PreNotify( NotifyEvent& rNEvt );
    virtual long    Notify(  NotifyEvent& rNEvt );//
};

} } // end of namespace svx::sidebar

#endif
