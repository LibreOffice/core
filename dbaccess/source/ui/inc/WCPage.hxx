/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef DBAUI_WIZARD_CPAGE_HXX
#define DBAUI_WIZARD_CPAGE_HXX

#include "WTabPage.hxx"

#include "QEnumTypes.hxx"
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>

namespace dbaui
{
    class OWizColumnSelect;
    class OWizNormalExtend;
    //========================================================================
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

        BOOL                                    m_bPKeyAllowed;
        BOOL                                    m_bUseHeaderAllowed;


        DECL_LINK( AppendDataClickHdl, Button* );
        DECL_LINK( RadioChangeHdl, Button* );
        DECL_LINK( KeyClickHdl, Button* );

        sal_Bool checkAppendData();
        void SetAppendDataRadio();

    public:
        virtual void            Reset();
        virtual void            ActivatePage();
        virtual BOOL            LeavePage();
        virtual String          GetTitle() const ;

        OCopyTable( Window * pParent );
        virtual ~OCopyTable();

        inline BOOL IsOptionDefData()       const { return m_aRB_DefData.IsChecked(); }
        inline BOOL IsOptionDef()           const { return m_aRB_Def.IsChecked(); }
        inline BOOL IsOptionAppendData()    const { return m_aRB_AppendData.IsChecked(); }
        inline BOOL IsOptionView()          const { return m_aRB_View.IsChecked(); }
        inline BOOL UseHeaderLine()         const { return m_aCB_UseHeaderLine.IsChecked(); }
        String      GetKeyName()            const { return m_edKeyName.GetText(); }

        void setCreateStyleAction();
        inline void disallowViews()
        {
            m_aRB_View.Disable();
        }
        inline void disallowUseHeaderLine()
        {
            m_bUseHeaderAllowed = FALSE;
            m_aCB_UseHeaderLine.Disable();
        }

        void setCreatePrimaryKey( bool _bDoCreate, const ::rtl::OUString& _rSuggestedName );
    };
}
#endif // DBAUI_WIZARD_CPAGE_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
