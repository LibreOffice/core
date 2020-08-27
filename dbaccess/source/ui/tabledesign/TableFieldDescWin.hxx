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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_TABLEDESIGN_TABLEFIELDDESCWIN_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_TABLEDESIGN_TABLEFIELDDESCWIN_HXX

#include <IClipBoardTest.hxx>
#include <ChildWindow.hxx>
#include "TableFieldControl.hxx"

namespace dbaui
{
    class OTableDesignHelpBar;
    class OTableDesignView;
    class OFieldDescription;

    class OTableFieldDescWin final : public OChildWindow
                                   , public IClipboardTest
    {
        enum ChildFocusState
        {
            DESCRIPTION,
            HELP,
            NONE
        };
    private:
        std::unique_ptr<OTableDesignHelpBar> m_xHelpBar;
        std::unique_ptr<weld::Container> m_xBox;
        std::unique_ptr<OTableFieldControl> m_xFieldControl;
        std::unique_ptr<weld::Label>   m_xHeader;
        Link<weld::Widget&, void> m_aFocusInHdl;

        ChildFocusState m_eChildFocus;

        IClipboardTest* getActiveChild() const;

        DECL_LINK(HelpFocusIn, weld::Widget&, void);
        DECL_LINK(FieldFocusIn, weld::Widget&, void);

    public:
        explicit OTableFieldDescWin(weld::Container* pParent, OTableDesignView* pView);
        virtual ~OTableFieldDescWin() override;

        void Init();

        void DisplayData( OFieldDescription* pFieldDescr );
        void SaveData( OFieldDescription* pFieldDescr );
        void SetReadOnly( bool bReadOnly );

        void SetControlText( sal_uInt16 nControlId, const OUString& rText )
                { m_xFieldControl->SetControlText(nControlId,rText); }

        OUString  BoolStringPersistent(const OUString& rUIString) const { return m_xFieldControl->BoolStringPersistent(rUIString); }
        OUString  BoolStringUI(const OUString& rPersistentString) const { return m_xFieldControl->BoolStringUI(rPersistentString); }

        virtual bool HasChildPathFocus() const override;
        virtual void GrabFocus() override;

        // IClipboardTest
        virtual bool isCutAllowed() override;
        virtual bool isCopyAllowed() override;
        virtual bool isPasteAllowed() override;

        virtual void copy() override;
        virtual void cut() override;
        virtual void paste() override;

        void connect_focus_in(const Link<weld::Widget&, void>& rLink)
        {
            m_aFocusInHdl = rLink;
        }

        OTableFieldControl* getGenPage() const { return m_xFieldControl.get(); }
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_TABLEDESIGN_TABLEFIELDDESCWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
