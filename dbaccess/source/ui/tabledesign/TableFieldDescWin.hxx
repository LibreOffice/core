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
    // Ableitung von TabPage ist ein Trick von TH,
    // um Aenderungen der Systemfarben zu bemerken
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
        OTableDesignHelpBar*    m_pHelpBar;
        OFieldDescGenWin*       m_pGenPage;
        FixedText*              m_pHeader;
        ChildFocusState         m_eChildFocus;

        IClipboardTest* getActiveChild() const;

    protected:
        virtual void Resize() SAL_OVERRIDE;
        virtual void Paint( const Rectangle& rRect ) SAL_OVERRIDE;

    public:
        OTableFieldDescWin( Window* pParent);
        virtual ~OTableFieldDescWin();

        virtual void Init();

        void DisplayData( OFieldDescription* pFieldDescr );
        void SaveData( OFieldDescription* pFieldDescr );
        void SetReadOnly( sal_Bool bReadOnly );

        // window overloads
        virtual bool PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
        virtual void GetFocus() SAL_OVERRIDE;
        virtual void LoseFocus() SAL_OVERRIDE;

        void SetControlText( sal_uInt16 nControlId, const OUString& rText )
                { m_pGenPage->SetControlText(nControlId,rText); }
        OUString GetControlText( sal_uInt16 nControlId )
                { return m_pGenPage->GetControlText(nControlId); }

        //  short GetFormatCategory(OFieldDescription* pFieldDescr) { return m_pGenPage ? m_pGenPage->GetFormatCategory(pFieldDescr) : -1; }
            // liefert zum am Feld eingestellten Format einen der CAT_xxx-Werte (CAT_NUMBER, CAT_DATE ...)

        OUString  BoolStringPersistent(const OUString& rUIString) const { return m_pGenPage->BoolStringPersistent(rUIString); }
        OUString  BoolStringUI(const OUString& rPersistentString) const { return m_pGenPage->BoolStringUI(rPersistentString); }

        // IClipboardTest
        virtual sal_Bool isCutAllowed() SAL_OVERRIDE;
        virtual sal_Bool isCopyAllowed() SAL_OVERRIDE;
        virtual sal_Bool isPasteAllowed() SAL_OVERRIDE;
        virtual sal_Bool hasChildPathFocus() SAL_OVERRIDE { return HasChildPathFocus(); }

        virtual void copy() SAL_OVERRIDE;
        virtual void cut() SAL_OVERRIDE;
        virtual void paste() SAL_OVERRIDE;

        inline OFieldDescGenWin* getGenPage() const { return m_pGenPage; }
        inline OTableDesignHelpBar* getHelpBar() const { return m_pHelpBar; }

    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_TABLEDESIGN_TABLEFIELDDESCWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
