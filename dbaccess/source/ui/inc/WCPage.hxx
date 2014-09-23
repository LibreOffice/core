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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_WCPAGE_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_WCPAGE_HXX

#include "WTabPage.hxx"

#include "QEnumTypes.hxx"
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>

namespace dbaui
{
    class OWizColumnSelect;
    class OWizNormalExtend;
    class OCopyTable : public OWizardPage
    {
    protected:
        Edit*              m_pEdTableName;
        RadioButton*       m_pRB_DefData;
        RadioButton*       m_pRB_Def;
        RadioButton*       m_pRB_View;
        RadioButton*       m_pRB_AppendData;
        CheckBox*          m_pCB_UseHeaderLine;
        CheckBox*          m_pCB_PrimaryColumn;
        FixedText*         m_pFT_KeyName;
        Edit*              m_pEdKeyName;
        sal_Int16          m_nOldOperation;

        OWizColumnSelect*  m_pPage2;
        OWizNormalExtend*  m_pPage3;

        bool               m_bPKeyAllowed;
        bool               m_bUseHeaderAllowed;

        DECL_LINK( AppendDataClickHdl, Button* );
        DECL_LINK( RadioChangeHdl, Button* );
        DECL_LINK( KeyClickHdl, Button* );

        bool checkAppendData();
        void SetAppendDataRadio();

    public:
        virtual void            Reset() SAL_OVERRIDE;
        virtual void            ActivatePage() SAL_OVERRIDE;
        virtual bool            LeavePage() SAL_OVERRIDE;
        virtual OUString        GetTitle() const SAL_OVERRIDE ;

        OCopyTable( vcl::Window * pParent );
        virtual ~OCopyTable();

        bool IsOptionDefData() const { return m_pRB_DefData->IsChecked(); }
        bool IsOptionDef() const { return m_pRB_Def->IsChecked(); }
        bool IsOptionAppendData() const { return m_pRB_AppendData->IsChecked(); }
        bool IsOptionView() const { return m_pRB_View->IsChecked(); }
        bool UseHeaderLine() const { return m_pCB_UseHeaderLine->IsChecked(); }
        OUString GetKeyName() const { return m_pEdKeyName->GetText(); }

        void setCreateStyleAction();
        inline void disallowViews()
        {
            m_pRB_View->Disable();
        }
        inline void disallowUseHeaderLine()
        {
            m_bUseHeaderAllowed = false;
            m_pCB_UseHeaderLine->Disable();
        }

        void setCreatePrimaryKey( bool _bDoCreate, const OUString& _rSuggestedName );
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_WCPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
