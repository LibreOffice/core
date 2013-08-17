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
#ifndef DBAUI_TABLEFIELDDESCGENPAGE_HXX
#define DBAUI_TABLEFIELDDESCGENPAGE_HXX

#include <vcl/tabpage.hxx>
#include "IClipBoardTest.hxx"

namespace dbaui
{
    class OTableDesignHelpBar;
    class OFieldDescription;
    class OTableFieldControl;
    class OTableEditorCtrl;
    class OFieldDescGenWin : public TabPage
                            ,public IClipboardTest
    {

        OTableFieldControl  *m_pFieldControl;
    protected:
        virtual void Resize();

    public:
        OFieldDescGenWin( Window* pParent, OTableDesignHelpBar* pHelpBar );
        virtual ~OFieldDescGenWin();

        virtual void GetFocus();
        virtual void LoseFocus();
        virtual void Init();

        void DisplayData( OFieldDescription* pFieldDescr );
        void SaveData( OFieldDescription* pFieldDescr );
        void SetControlText( sal_uInt16 nControlId, const String& rText );
        String GetControlText( sal_uInt16 nControlId );
        void SetReadOnly( sal_Bool bReadOnly );
#if OSL_DEBUG_LEVEL > 0
        OTableEditorCtrl* GetEditorCtrl();
#endif
        //  short GetFormatCategory(OFieldDescription* pFieldDescr);
            // liefert zum am Feld eingestellten Format einen der CAT_xxx-Werte (CAT_NUMBER, CAT_DATE ...)

        String BoolStringPersistent(const String& rUIString) const;
        String BoolStringUI(const String& rPersistentString) const;

        // IClipboardTest
        virtual sal_Bool isCutAllowed();
        virtual sal_Bool isCopyAllowed();
        virtual sal_Bool isPasteAllowed();
        virtual sal_Bool hasChildPathFocus() { return HasChildPathFocus(); }

        virtual void copy();
        virtual void cut();
        virtual void paste();

        inline OTableFieldControl*  getFieldControl() const { return m_pFieldControl; }
    };
}
#endif // DBAUI_TABLEFIELDDESCGENPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
