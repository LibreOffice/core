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
#pragma once

#include "FieldDescControl.hxx"
#include "TypeInfo.hxx"
#include "WTabPage.hxx"

class SvStream;

namespace dbaui
{
    class OWizTypeSelect;
    class OTableDesignHelpBar;
    // OWizTypeSelectControl
    class OWizTypeSelectControl final : public OFieldDescControl
    {
        OWizTypeSelect* m_pParentTabPage;
        virtual void        ActivateAggregate( EControlType eType ) override;
        virtual void        DeactivateAggregate( EControlType eType ) override;

        virtual void        CellModified(long nRow, sal_uInt16 nColId ) override;

        virtual css::lang::Locale  GetLocale() const override;
        virtual css::uno::Reference< css::util::XNumberFormatter > GetFormatter() const override;
        virtual TOTypeInfoSP        getTypeInfo(sal_Int32 _nPos) override;
        virtual const OTypeInfoMap* getTypeInfo() const override;
        virtual bool                isAutoIncrementValueEnabled() const override;
        virtual OUString            getAutoIncrementValue() const override;

    public:
        OWizTypeSelectControl(weld::Container* pPage, OWizTypeSelect* pParentTabPage);
        virtual ~OWizTypeSelectControl() override;

        virtual css::uno::Reference< css::sdbc::XDatabaseMetaData> getMetaData() override;
        virtual css::uno::Reference< css::sdbc::XConnection> getConnection() override;
    };

    // Wizard Page: OWizTypeSelectList
    // just defines the css::ucb::Command for the Contextmenu
    class OWizTypeSelectList
    {
        std::unique_ptr<weld::TreeView> m_xControl;
        bool                    m_bPKey;
        bool                    IsPrimaryKeyAllowed() const;
        void                    setPrimaryKey(  OFieldDescription* _pFieldDescr,
                                                sal_uInt16 _nPos,
                                                bool _bSet = false);

        DECL_LINK(CommandHdl, const CommandEvent&, bool);

        Link<weld::TreeView&, void> m_aChangeHdl;

    public:
        OWizTypeSelectList(std::unique_ptr<weld::TreeView> xControl);
        void SetPKey(bool bPKey) { m_bPKey = bPKey; }
        weld::TreeView* GetWidget() { return m_xControl.get(); }
        OUString get_selected_id() const { return m_xControl->get_selected_id(); }
        void clear() { m_xControl->clear(); }
        void append(const OUString& rId, const OUString& rStr)
        {
            m_xControl->append(rId, rStr);
        }
        void set_image(int nRow, const OUString& rImage)
        {
            m_xControl->set_image(nRow, rImage);
        }
        void set_selection_mode(SelectionMode eMode) { m_xControl->set_selection_mode(eMode); }
        int count_selected_rows() const { return m_xControl->count_selected_rows(); }
        void select(int pos) { m_xControl->select(pos); }
        void connect_changed(const Link<weld::TreeView&, void>& rLink)
        {
            m_aChangeHdl = rLink;
            m_xControl->connect_changed(rLink);
        }
    };

    // Wizard Page: OWizTypeSelect
    // Serves as base class for different copy properties.
    // Calls FillColumnList, when button AUTO is triggered
    class OWizTypeSelect : public OWizardPage
    {
        friend class OWizTypeSelectControl;
        friend class OWizTypeSelectList;

        DECL_LINK( ColumnSelectHdl, weld::TreeView&, void );
        DECL_LINK( ButtonClickHdl, weld::Button&, void );
    protected:
        std::unique_ptr<OWizTypeSelectList> m_xColumnNames;
        std::unique_ptr<weld::Label> m_xColumns;
        std::unique_ptr<weld::Container> m_xControlContainer;
        std::unique_ptr<OWizTypeSelectControl> m_xTypeControl;
        std::unique_ptr<weld::Label> m_xAutoType;
        std::unique_ptr<weld::Label> m_xAutoFt;
        std::unique_ptr<weld::SpinButton> m_xAutoEt;
        std::unique_ptr<weld::Button> m_xAutoPb;

        SvStream*               m_pParserStream; // stream to read the tokens from or NULL
        OUString                m_sAutoIncrementValue;
        sal_Int32               m_nDisplayRow;
        bool                    m_bAutoIncrementEnabled;
        bool                    m_bDuplicateName;

        virtual void            createReaderAndCallParser(sal_Int32 _nRows) = 0;

        void                    EnableAuto(bool bEnable);
    public:
        virtual void            Reset ( ) override;
        virtual void            Activate( ) override;
        virtual bool            LeavePage() override;
        virtual OUString        GetTitle() const override;

        OWizTypeSelect(weld::Container* pParent, OCopyTableWizard* pWizard, SvStream* pStream = nullptr);
        virtual ~OWizTypeSelect() override;

        void setDisplayRow(sal_Int32 _nRow) { m_nDisplayRow = _nRow - 1; }
        void setDuplicateName(bool _bDuplicateName) { m_bDuplicateName = _bDuplicateName; }
    };

    typedef std::unique_ptr<OWizTypeSelect> (*TypeSelectionPageFactory)(weld::Container*, OCopyTableWizard*, SvStream&);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
