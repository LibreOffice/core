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
#ifndef _SVT_FLTRCFG_HXX
#define _SVT_FLTRCFG_HXX

// -----------------------------------------------------------------------

#include "unotools/unotoolsdllapi.h"
#include <unotools/configitem.hxx>

struct SvtFilterOptions_Impl;
class UNOTOOLS_DLLPUBLIC SvtFilterOptions : public utl::ConfigItem
{
    SvtFilterOptions_Impl* pImp;

    const com::sun::star::uno::Sequence<OUString>& GetPropertyNames();
public:
                    SvtFilterOptions();
    virtual        ~SvtFilterOptions();

    virtual void            Notify( const com::sun::star::uno::Sequence<OUString>& aPropertyNames);
    virtual void            Commit();
    void                    Load();

    void SetLoadWordBasicCode( sal_Bool bFlag );
    sal_Bool IsLoadWordBasicCode() const;
    void SetLoadWordBasicExecutable( sal_Bool bFlag );
    sal_Bool IsLoadWordBasicExecutable() const;
    void SetLoadWordBasicStorage( sal_Bool bFlag );
    sal_Bool IsLoadWordBasicStorage() const;

    void SetLoadExcelBasicCode( sal_Bool bFlag );
    sal_Bool IsLoadExcelBasicCode() const;
    void SetLoadExcelBasicExecutable( sal_Bool bFlag );
    sal_Bool IsLoadExcelBasicExecutable() const;
    void SetLoadExcelBasicStorage( sal_Bool bFlag );
    sal_Bool IsLoadExcelBasicStorage() const;

    void SetLoadPPointBasicCode( sal_Bool bFlag );
    sal_Bool IsLoadPPointBasicCode() const;
    void SetLoadPPointBasicStorage( sal_Bool bFlag );
    sal_Bool IsLoadPPointBasicStorage() const;

    sal_Bool IsMathType2Math() const;
    void SetMathType2Math( sal_Bool bFlag );
    sal_Bool IsMath2MathType() const;
    void SetMath2MathType( sal_Bool bFlag );

    sal_Bool IsWinWord2Writer() const;
    void SetWinWord2Writer( sal_Bool bFlag );
    sal_Bool IsWriter2WinWord() const;
    void SetWriter2WinWord( sal_Bool bFlag );

    sal_Bool IsUseEnhancedFields() const;

    sal_Bool IsExcel2Calc() const;
    void SetExcel2Calc( sal_Bool bFlag );
    sal_Bool IsCalc2Excel() const;
    void SetCalc2Excel( sal_Bool bFlag );

    sal_Bool IsPowerPoint2Impress() const;
    void SetPowerPoint2Impress( sal_Bool bFlag );
    sal_Bool IsImpress2PowerPoint() const;
    void SetImpress2PowerPoint( sal_Bool bFlag );

    sal_Bool IsEnablePPTPreview() const;
    sal_Bool IsEnableCalcPreview() const;
    sal_Bool IsEnableWordPreview() const;

    static SvtFilterOptions& Get();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
