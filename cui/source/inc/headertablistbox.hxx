/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_CUI_SOURCE_INC_HEADERTABLISTBOX_HXX
#define INCLUDED_CUI_SOURCE_INC_HEADERTABLISTBOX_HXX

#include <svtools/headbar.hxx>
#include <svtools/svtabbx.hxx>


class MacroEventListBox : public Control
{
private:
    HeaderBar                   maHeaderBar;
    SvHeaderTabListBox          maListBox;
protected:
    DECL_LINK( HeaderEndDrag_Impl, HeaderBar* );
    virtual bool                Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
public:
    MacroEventListBox( Window* pParent, WinBits nStyle );

    virtual void Resize() SAL_OVERRIDE;
    virtual Size GetOptimalSize() const SAL_OVERRIDE;

    SvHeaderTabListBox& GetListBox()
    {
        return maListBox;
    }

    HeaderBar& GetHeaderBar()
    {
        return maHeaderBar;
    }

    void                        ConnectElements( void );/**< should be called after all manipulations on elements are done
                                                             calcs real sizes depending on sizes of this */
    void                        Show( sal_Bool bVisible = sal_True, sal_uInt16 nFlags = 0 );    ///< same meaning as Windows::Show()
    void                        Enable( bool bEnable = true, bool bChild = true );  ///< same meaning as Windows::Enable()
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
