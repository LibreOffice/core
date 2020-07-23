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

#if !defined(VCL_DLLIMPLEMENTATION) && !defined(TOOLKIT_DLLIMPLEMENTATION) && !defined(VCL_INTERNALS)
#error "don't use this in new code"
#endif

#include <vcl/formatter.hxx>
#include <vcl/toolkit/spinfld.hxx>

class VCL_DLLPUBLIC FormattedField : public SpinField
{
public:
    FormattedField(vcl::Window* pParent, WinBits nStyle);

    virtual void dispose() override;

    // Spin-Handling
    virtual void Up() override;
    virtual void Down() override;
    // Default Implementation: +/- default spin size to the double value
    virtual void First() override;
    virtual void Last() override;
    // Default Implementation: Current double is set to the first or last value

    virtual bool set_property(const OString &rKey, const OUString &rValue) override;
public:
    virtual void SetText( const OUString& rStr ) override;
    virtual void SetText( const OUString& rStr, const Selection& rNewSelection ) override;

    void SetValueFromString(const OUString& rStr); // currently used by online

    virtual void DumpAsPropertyTree(tools::JsonWriter&) override;

    virtual FactoryFunction GetUITestFactory() const override;

    Formatter& GetFormatter();
    void SetFormatter(Formatter* pFormatter);

protected:
    std::unique_ptr<Formatter> m_xOwnFormatter;
    Formatter* m_pFormatter;

    virtual bool EventNotify(NotifyEvent& rNEvt) override;
    virtual void Modify() override;

    bool PreNotify(NotifyEvent& rNEvt) override;
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) DoubleNumericField final : public FormattedField
{
public:
    DoubleNumericField(vcl::Window* pParent, WinBits nStyle);

    virtual ~DoubleNumericField() override;

    validation::NumberValidator& GetNumberValidator() { return *m_pNumberValidator; }
    void ResetConformanceTester();

private:

    std::unique_ptr<validation::NumberValidator> m_pNumberValidator;
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) DoubleCurrencyField final : public FormattedField
{
public:
    DoubleCurrencyField(vcl::Window* pParent, WinBits nStyle);

    const OUString& getCurrencySymbol() const { return m_sCurrencySymbol; }
    void        setCurrencySymbol(const OUString& rSymbol);

    bool        getPrependCurrSym() const { return m_bPrependCurrSym; }
    void        setPrependCurrSym(bool _bPrepend);

    void UpdateCurrencyFormat();
private:
    OUString   m_sCurrencySymbol;
    bool       m_bPrependCurrSym;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
