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

#include <vcl/tabpage.hxx>
#include "FieldDescGenWin.hxx"
#include "IClipBoardTest.hxx"

class FixedText;
namespace dbaui
{
    class OFieldDescGenWin;
    class OTableDesignHelpBar;
    class OFieldDescription;
    // derivative of TabPage is a trick of TH,
    // to notice a change in system colours
    class OTableFieldDescWin : public TabPage
                                ,public IClipboardTest
    {
        enum ChildFocusState
        {
            DESCRIPTION,
            HELP,
            NONE
        };
    private:
        VclPtr<OTableDesignHelpBar>    m_pHelpBar;
        VclPtr<OFieldDescGenWin>       m_pGenPage;
        VclPtr<FixedText>              m_pHeader;
        ChildFocusState         m_eChildFocus;

        IClipboardTest* getActiveChild() const;

    protected:
        virtual void Resize() override;
        virtual void Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;

    public:
        explicit OTableFieldDescWin( vcl::Window* pParent);
        virtual ~OTableFieldDescWin();
        virtual void dispose() override;

        void Init();

        void DisplayData( OFieldDescription* pFieldDescr );
        void SaveData( OFieldDescription* pFieldDescr );
        void SetReadOnly( bool bReadOnly );

        // Window overrides
        virtual bool PreNotify( NotifyEvent& rNEvt ) override;
        virtual void GetFocus() override;
        virtual void LoseFocus() override;

        void SetControlText( sal_uInt16 nControlId, const OUString& rText )
                { m_pGenPage->SetControlText(nControlId,rText); }

        // short GetFormatCategory(OFieldDescription* pFieldDescr) { return m_pGenPage ? m_pGenPage->GetFormatCategory(pFieldDescr) : -1; }
        // delivers a CAT_xxx (CAT_NUMBER, CAT_DATE ...) value to a Format set in the field

        OUString  BoolStringPersistent(const OUString& rUIString) const { return m_pGenPage->BoolStringPersistent(rUIString); }
        OUString  BoolStringUI(const OUString& rPersistentString) const { return m_pGenPage->BoolStringUI(rPersistentString); }

        // IClipboardTest
        virtual bool isCutAllowed() override;
        virtual bool isCopyAllowed() override;
        virtual bool isPasteAllowed() override;

        virtual void copy() override;
        virtual void cut() override;
        virtual void paste() override;

        inline OFieldDescGenWin* getGenPage() const { return m_pGenPage; }
        inline OTableDesignHelpBar* getHelpBar() const { return m_pHelpBar; }

    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_TABLEDESIGN_TABLEFIELDDESCWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
