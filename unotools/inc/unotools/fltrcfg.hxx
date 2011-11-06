/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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




