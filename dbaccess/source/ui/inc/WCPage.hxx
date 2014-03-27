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
        FixedText                               m_ftTableName;
        Edit                                    m_edTableName;
        FixedLine                               m_aFL_Options;
        RadioButton                             m_aRB_DefData;
        RadioButton                             m_aRB_Def;
        RadioButton                             m_aRB_View;
        RadioButton                             m_aRB_AppendData;
        CheckBox                                m_aCB_UseHeaderLine;
        CheckBox                                m_aCB_PrimaryColumn;
        FixedText                               m_aFT_KeyName;
        Edit                                    m_edKeyName;
        sal_Int16                               m_nOldOperation;

        OWizColumnSelect*                       m_pPage2;
        OWizNormalExtend*                       m_pPage3;

        sal_Bool                                    m_bPKeyAllowed;
        sal_Bool                                    m_bUseHeaderAllowed;

        DECL_LINK( AppendDataClickHdl, Button* );
        DECL_LINK( RadioChangeHdl, Button* );
        DECL_LINK( KeyClickHdl, Button* );

        sal_Bool checkAppendData();
        void SetAppendDataRadio();

    public:
        virtual void            Reset() SAL_OVERRIDE;
        virtual void            ActivatePage() SAL_OVERRIDE;
        virtual sal_Bool        LeavePage() SAL_OVERRIDE;
        virtual OUString        GetTitle() const SAL_OVERRIDE ;

        OCopyTable( Window * pParent );
        virtual ~OCopyTable();

        inline sal_Bool IsOptionDefData()       const { return m_aRB_DefData.IsChecked(); }
        inline sal_Bool IsOptionDef()           const { return m_aRB_Def.IsChecked(); }
        inline sal_Bool IsOptionAppendData()    const { return m_aRB_AppendData.IsChecked(); }
        inline sal_Bool IsOptionView()          const { return m_aRB_View.IsChecked(); }
        inline sal_Bool UseHeaderLine()         const { return m_aCB_UseHeaderLine.IsChecked(); }
        OUString        GetKeyName()            const { return m_edKeyName.GetText(); }

        void setCreateStyleAction();
        inline void disallowViews()
        {
            m_aRB_View.Disable();
        }
        inline void disallowUseHeaderLine()
        {
            m_bUseHeaderAllowed = sal_False;
            m_aCB_UseHeaderLine.Disable();
        }

        void setCreatePrimaryKey( bool _bDoCreate, const OUString& _rSuggestedName );
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_WCPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
