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

#ifndef INCLUDED_SC_SOURCE_UI_INC_SOLVEROPTIONS_HXX
#define INCLUDED_SC_SOURCE_UI_INC_SOLVEROPTIONS_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <svx/checklbx.hxx>
#include <com/sun/star/uno/Sequence.hxx>

namespace com { namespace sun { namespace star {
    namespace beans { struct PropertyValue; }
} } }

class ScSolverOptionsDialog : public ModalDialog
{
    VclPtr<ListBox> m_pLbEngine;
    VclPtr<SvxCheckListBox> m_pLbSettings;
    VclPtr<PushButton> m_pBtnEdit;

    SvLBoxButtonData* mpCheckButtonData;
    com::sun::star::uno::Sequence<OUString> maImplNames;
    com::sun::star::uno::Sequence<OUString> maDescriptions;
    OUString        maEngine;
    com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> maProperties;

    DECL_LINK_TYPED( EngineSelectHdl, ListBox&, void );
    DECL_LINK_TYPED( SettingsSelHdl, SvTreeListBox*, void );
    DECL_LINK_TYPED( SettingsDoubleClickHdl, SvTreeListBox*, bool );
    DECL_LINK_TYPED( ButtonHdl, Button*, void );

    void    ReadFromComponent();
    void    FillListBox();
    void    EditOption();

public:
    ScSolverOptionsDialog( vcl::Window* pParent,
                           const com::sun::star::uno::Sequence<OUString>& rImplNames,
                           const com::sun::star::uno::Sequence<OUString>& rDescriptions,
                           const OUString& rEngine,
                           const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rProperties );
    virtual ~ScSolverOptionsDialog();
    virtual void dispose() override;

    // already updated in selection handler
    const OUString& GetEngine() const { return maEngine; }
    const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& GetProperties();
};

class ScSolverIntegerDialog : public ModalDialog
{
    VclPtr<VclFrame>     m_pFrame;
    VclPtr<NumericField> m_pNfValue;

public:
    ScSolverIntegerDialog( vcl::Window * pParent );
    virtual ~ScSolverIntegerDialog();
    virtual void dispose() override;

    void        SetOptionName( const OUString& rName );
    void        SetValue( sal_Int32 nValue );
    sal_Int32   GetValue() const;
};

class ScSolverValueDialog : public ModalDialog
{
    VclPtr<VclFrame>   m_pFrame;
    VclPtr<Edit>       m_pEdValue;

public:
    ScSolverValueDialog( vcl::Window * pParent );
    virtual ~ScSolverValueDialog();
    virtual void dispose() override;

    void        SetOptionName( const OUString& rName );
    void        SetValue( double fValue );
    double      GetValue() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
