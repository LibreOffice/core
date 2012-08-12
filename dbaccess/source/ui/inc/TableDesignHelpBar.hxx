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
#ifndef DBAUI_TABLEDESIGNHELPBAR_HXX
#define DBAUI_TABLEDESIGNHELPBAR_HXX

#include <tools/string.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include "IClipBoardTest.hxx"

class MultiLineEdit;

namespace dbaui
{
    // Deriving from TabPage is a trick to notice changes
    // of the system colors
    class OTableDesignHelpBar : public TabPage
                                ,public IClipboardTest
    {
    private:
        String          m_sHelpText;
        MultiLineEdit*  m_pTextWin;
        sal_uInt16          m_nDummy;

    protected:
        virtual void Resize();

    public:
        OTableDesignHelpBar( Window* pParent );
        virtual ~OTableDesignHelpBar();

        void SetHelpText( const String& rText );
        String GetHelpText() const { return m_sHelpText; }

        virtual long PreNotify( NotifyEvent& rNEvt );

        // IClipboardTest
        virtual sal_Bool isCutAllowed();
        virtual sal_Bool isCopyAllowed();
        virtual sal_Bool isPasteAllowed();
        virtual sal_Bool hasChildPathFocus() { return HasChildPathFocus(); }

        virtual void copy();
        virtual void cut();
        virtual void paste();
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
