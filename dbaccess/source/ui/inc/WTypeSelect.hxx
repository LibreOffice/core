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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_WTYPESELECT_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_WTYPESELECT_HXX

#include "FieldDescControl.hxx"
#include "TypeInfo.hxx"
#include "WTabPage.hxx"

#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>

class SvStream;
class SvParser;
namespace dbaui
{
    class OTableDesignHelpBar;
    // OWizTypeSelectControl
    class OWizTypeSelectControl : public OFieldDescControl
    {
    protected:
        VclPtr<vcl::Window> m_pParentTabPage;
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
        OWizTypeSelectControl(vcl::Window* pParent, vcl::Window* pParentTabPage, OTableDesignHelpBar* pHelpBar=nullptr);
        virtual ~OWizTypeSelectControl();
        virtual void dispose() override;

        virtual css::uno::Reference< css::sdbc::XDatabaseMetaData> getMetaData() override;
        virtual css::uno::Reference< css::sdbc::XConnection> getConnection() override;
    };

    // Wizard Page: OWizTypeSelectList
    // just defines the css::ucb::Command for the Contextmenu
    class OWizTypeSelectList : public MultiListBox
    {
        bool                    m_bPKey;
        bool                    IsPrimaryKeyAllowed() const;
        void                    setPrimaryKey(  OFieldDescription* _pFieldDescr,
                                                sal_uInt16 _nPos,
                                                bool _bSet = false);
    protected:
        virtual bool            PreNotify( NotifyEvent& rNEvt ) override;
        VclPtr<vcl::Window>     m_pParentTabPage;
    public:
        OWizTypeSelectList( vcl::Window* pParent, WinBits nStyle = WB_BORDER )
            : MultiListBox(pParent,nStyle)
            , m_bPKey(false)
            , m_pParentTabPage(nullptr)
            {}
        virtual ~OWizTypeSelectList();
        virtual void dispose() override;
        void                    SetPKey(bool bPKey) { m_bPKey = bPKey; }
        void                    SetParentTabPage(vcl::Window* pParentTabPage) { m_pParentTabPage = pParentTabPage; }
    };

    // Wizard Page: OWizTypeSelect
    // Serves as base class for different copy properties.
    // Calls FillColumnList, when button AUTO is triggered
    class OWizTypeSelect : public OWizardPage
    {
        friend class OWizTypeSelectControl;
        friend class OWizTypeSelectList;

        DECL_LINK_TYPED( ColumnSelectHdl, ListBox&, void );
        DECL_LINK_TYPED( ButtonClickHdl, Button *, void );
    protected:
        VclPtr<OWizTypeSelectList>      m_pColumnNames;
        VclPtr<FixedText>               m_pColumns;
        VclPtr<OWizTypeSelectControl>   m_pTypeControl;
        VclPtr<FixedText>               m_pAutoType;
        VclPtr<FixedText>               m_pAutoFt;
        VclPtr<NumericField>            m_pAutoEt;
        VclPtr<PushButton>              m_pAutoPb;

        Image                   m_imgPKey;
        SvStream*               m_pParserStream; // stream to read the tokens from or NULL
        OUString                m_sAutoIncrementValue;
        sal_Int32               m_nDisplayRow;
        bool                    m_bAutoIncrementEnabled;
        bool                    m_bDuplicateName;

        void                    fillColumnList(sal_uInt32 nRows);
        virtual SvParser*       createReader(sal_Int32 _nRows) = 0;

        void                    EnableAuto(bool bEnable);
    public:
        virtual void            Reset ( ) override;
        virtual void            ActivatePage( ) override;
        virtual bool            LeavePage() override;
        virtual OUString        GetTitle() const override;

        OWizTypeSelect(vcl::Window* pParent, SvStream* _pStream = nullptr );
        virtual ~OWizTypeSelect();
        virtual void dispose() override;

        inline void setDisplayRow(sal_Int32 _nRow) { m_nDisplayRow = _nRow - 1; }
        inline void setDuplicateName(bool _bDuplicateName) { m_bDuplicateName = _bDuplicateName; }
    };

    typedef VclPtr<OWizTypeSelect> (*TypeSelectionPageFactory)( vcl::Window*, SvStream& );
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_WTYPESELECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
