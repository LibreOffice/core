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

#ifndef SC_SOLVEROPTIONS_HXX
#define SC_SOLVEROPTIONS_HXX

#include <vcl/dialog.hxx>

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <svx/checklbx.hxx>
#include <com/sun/star/uno/Sequence.hxx>

namespace com { namespace sun { namespace star {
    namespace beans { struct PropertyValue; }
} } }



class ScSolverOptionsDialog : public ModalDialog
{
    FixedText       maFtEngine;
    ListBox         maLbEngine;
    FixedText       maFtSettings;
    SvxCheckListBox maLbSettings;
    PushButton      maBtnEdit;
    FixedLine       maFlButtons;
    HelpButton      maBtnHelp;
    OKButton        maBtnOk;
    CancelButton    maBtnCancel;

    SvLBoxButtonData* mpCheckButtonData;
    com::sun::star::uno::Sequence<rtl::OUString> maImplNames;
    com::sun::star::uno::Sequence<rtl::OUString> maDescriptions;
    String          maEngine;
    com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> maProperties;

    DECL_LINK( EngineSelectHdl, ListBox* );
    DECL_LINK( SettingsSelHdl, SvxCheckListBox* );
    DECL_LINK( SettingsDoubleClickHdl, SvTreeListBox* );
    DECL_LINK( ButtonHdl, PushButton* );

    void    ReadFromComponent();
    void    FillListBox();
    void    EditOption();

public:
    ScSolverOptionsDialog( Window* pParent,
                           const com::sun::star::uno::Sequence<rtl::OUString>& rImplNames,
                           const com::sun::star::uno::Sequence<rtl::OUString>& rDescriptions,
                           const String& rEngine,
                           const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rProperties );
    ~ScSolverOptionsDialog();

    const String& GetEngine() const;
    const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& GetProperties();
};


class ScSolverIntegerDialog : public ModalDialog
{
    FixedText       maFtName;
    NumericField    maNfValue;
    FixedLine       maFlButtons;
    OKButton        maBtnOk;
    CancelButton    maBtnCancel;

public:
    ScSolverIntegerDialog( Window * pParent );
    ~ScSolverIntegerDialog();

    void        SetOptionName( const String& rName );
    void        SetValue( sal_Int32 nValue );
    sal_Int32   GetValue() const;
};

class ScSolverValueDialog : public ModalDialog
{
    FixedText       maFtName;
    Edit            maEdValue;
    FixedLine       maFlButtons;
    OKButton        maBtnOk;
    CancelButton    maBtnCancel;

public:
    ScSolverValueDialog( Window * pParent );
    ~ScSolverValueDialog();

    void        SetOptionName( const String& rName );
    void        SetValue( double fValue );
    double      GetValue() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
