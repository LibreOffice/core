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
#ifndef INCLUDED_UNOTOOLS_FLTRCFG_HXX
#define INCLUDED_UNOTOOLS_FLTRCFG_HXX

#include <unotools/unotoolsdllapi.h>
#include <unotools/configitem.hxx>

struct SvtFilterOptions_Impl;
class UNOTOOLS_DLLPUBLIC SvtFilterOptions : public utl::ConfigItem
{
private:
    SvtFilterOptions_Impl* pImp;

    const css::uno::Sequence<OUString>& GetPropertyNames();

    virtual void            ImplCommit() override;

public:
                    SvtFilterOptions();
    virtual        ~SvtFilterOptions();

    virtual void            Notify( const css::uno::Sequence<OUString>& aPropertyNames) override;
    void                    Load();

    void SetLoadWordBasicCode( bool bFlag );
    bool IsLoadWordBasicCode() const;
    void SetLoadWordBasicExecutable( bool bFlag );
    bool IsLoadWordBasicExecutable() const;
    void SetLoadWordBasicStorage( bool bFlag );
    bool IsLoadWordBasicStorage() const;

    void SetLoadExcelBasicCode( bool bFlag );
    bool IsLoadExcelBasicCode() const;
    void SetLoadExcelBasicExecutable( bool bFlag );
    bool IsLoadExcelBasicExecutable() const;
    void SetLoadExcelBasicStorage( bool bFlag );
    bool IsLoadExcelBasicStorage() const;

    void SetLoadPPointBasicCode( bool bFlag );
    bool IsLoadPPointBasicCode() const;
    void SetLoadPPointBasicStorage( bool bFlag );
    bool IsLoadPPointBasicStorage() const;

    bool IsMathType2Math() const;
    void SetMathType2Math( bool bFlag );
    bool IsMath2MathType() const;
    void SetMath2MathType( bool bFlag );

    bool IsWinWord2Writer() const;
    void SetWinWord2Writer( bool bFlag );
    bool IsWriter2WinWord() const;
    void SetWriter2WinWord( bool bFlag );

    bool IsUseEnhancedFields() const;

    bool IsExcel2Calc() const;
    void SetExcel2Calc( bool bFlag );
    bool IsCalc2Excel() const;
    void SetCalc2Excel( bool bFlag );

    bool IsPowerPoint2Impress() const;
    void SetPowerPoint2Impress( bool bFlag );
    bool IsImpress2PowerPoint() const;
    void SetImpress2PowerPoint( bool bFlag );

    bool IsSmartArt2Shape() const;
    void SetSmartArt2Shape( bool bFlag );

    bool IsEnablePPTPreview() const;
    bool IsEnableCalcPreview() const;
    bool IsEnableWordPreview() const;

    bool IsCharBackground2Highlighting() const;
    bool IsCharBackground2Shading() const;
    void SetCharBackground2Highlighting();
    void SetCharBackground2Shading();

    static SvtFilterOptions& Get();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
