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

#include <vcl/weld.hxx>
#include <com/sun/star/uno/Sequence.hxx>

namespace com { namespace sun { namespace star {
    namespace beans { struct PropertyValue; }
} } }

class ScSolverOptionsString
{
    bool        mbIsDouble;
    double      mfDoubleValue;
    sal_Int32   mnIntValue;
    OUString    msStr;

public:
    explicit ScSolverOptionsString(const OUString& rStr)
        : mbIsDouble(false)
        , mfDoubleValue(0.0)
        , mnIntValue(0)
        , msStr(rStr)
    {
    }

    bool      IsDouble() const        { return mbIsDouble; }
    double    GetDoubleValue() const  { return mfDoubleValue; }
    sal_Int32 GetIntValue() const     { return mnIntValue; }
    const OUString& GetText() const   { return msStr; }

    void      SetDoubleValue( double fNew ) { mbIsDouble = true; mfDoubleValue = fNew; }
    void      SetIntValue( sal_Int32 nNew ) { mbIsDouble = false; mnIntValue = nNew; }
};

class ScSolverOptionsDialog : public weld::GenericDialogController
{
    css::uno::Sequence<OUString> maImplNames;
    css::uno::Sequence<OUString> maDescriptions;
    OUString maEngine;
    css::uno::Sequence<css::beans::PropertyValue> maProperties;

    std::vector<std::unique_ptr<ScSolverOptionsString>> m_aOptions;

    std::unique_ptr<weld::ComboBox> m_xLbEngine;
    std::unique_ptr<weld::TreeView> m_xLbSettings;
    std::unique_ptr<weld::Button> m_xBtnEdit;

    DECL_LINK( EngineSelectHdl, weld::ComboBox&, void );
    DECL_LINK( SettingsSelHdl, weld::TreeView&, void );
    DECL_LINK( SettingsDoubleClickHdl, weld::TreeView&, void );
    DECL_LINK( ButtonHdl, weld::Button&, void );

    void    ReadFromComponent();
    void    FillListBox();
    void    EditOption();

public:
    ScSolverOptionsDialog( weld::Window* pParent,
                           const css::uno::Sequence<OUString>& rImplNames,
                           const css::uno::Sequence<OUString>& rDescriptions,
                           const OUString& rEngine,
                           const css::uno::Sequence<css::beans::PropertyValue>& rProperties );
    virtual ~ScSolverOptionsDialog() override;

    // already updated in selection handler
    const OUString& GetEngine() const { return maEngine; }
    const css::uno::Sequence<css::beans::PropertyValue>& GetProperties();
};

class ScSolverIntegerDialog : public weld::GenericDialogController
{
    std::unique_ptr<weld::Frame> m_xFrame;
    std::unique_ptr<weld::SpinButton> m_xNfValue;

public:
    ScSolverIntegerDialog(weld::Window* pParent);
    virtual ~ScSolverIntegerDialog() override;

    void        SetOptionName( const OUString& rName );
    void        SetValue( sal_Int32 nValue );
    sal_Int32   GetValue() const;
};

class ScSolverValueDialog : public weld::GenericDialogController
{
    std::unique_ptr<weld::Frame> m_xFrame;
    std::unique_ptr<weld::Entry> m_xEdValue;

public:
    ScSolverValueDialog(weld::Window* pParent);
    virtual ~ScSolverValueDialog() override;

    void        SetOptionName( const OUString& rName );
    void        SetValue( double fValue );
    double      GetValue() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
