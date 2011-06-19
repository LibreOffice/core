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
#ifndef _SVT_FLTRCFG_HXX
#define _SVT_FLTRCFG_HXX

// -----------------------------------------------------------------------

#include "unotools/unotoolsdllapi.h"
#include <unotools/configitem.hxx>

struct SvtFilterOptions_Impl;
class UNOTOOLS_DLLPUBLIC SvtFilterOptions : public utl::ConfigItem
{
    SvtFilterOptions_Impl* pImp;

    const com::sun::star::uno::Sequence<rtl::OUString>& GetPropertyNames();
public:
                    SvtFilterOptions();
    virtual        ~SvtFilterOptions();

    virtual void            Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);
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
    void SetUseEnhancedFields( sal_Bool  bFlag );

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

    static SvtFilterOptions* Get();
};

#endif




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
