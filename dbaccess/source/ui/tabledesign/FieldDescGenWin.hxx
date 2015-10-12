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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_TABLEDESIGN_FIELDDESCGENWIN_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_TABLEDESIGN_FIELDDESCGENWIN_HXX

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

        VclPtr<OTableFieldControl>  m_pFieldControl;
    protected:
        virtual void Resize() override;

    public:
        OFieldDescGenWin( vcl::Window* pParent, OTableDesignHelpBar* pHelpBar );
        virtual ~OFieldDescGenWin();
        virtual void dispose() override;

        virtual void GetFocus() override;
        virtual void LoseFocus() override;
        void Init();

        void DisplayData( OFieldDescription* pFieldDescr );
        void SaveData( OFieldDescription* pFieldDescr );
        void SetControlText( sal_uInt16 nControlId, const OUString& rText );
        void SetReadOnly( bool bReadOnly );
        OTableEditorCtrl* GetEditorCtrl();

        // short GetFormatCategory(OFieldDescription* pFieldDescr);
        // gives you one of the CAT_xxx-values (CAT_NUMBER, CAT_DATE ...) belonging to the format specified by the field

        OUString BoolStringPersistent(const OUString& rUIString) const;
        OUString BoolStringUI(const OUString& rPersistentString) const;

        // IClipboardTest
        virtual bool isCutAllowed() override;
        virtual bool isCopyAllowed() override;
        virtual bool isPasteAllowed() override;

        virtual void copy() override;
        virtual void cut() override;
        virtual void paste() override;

        inline OTableFieldControl*  getFieldControl() const { return m_pFieldControl; }
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_TABLEDESIGN_FIELDDESCGENWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
