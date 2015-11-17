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

#include <config_features.h>

#include <unotools/fltrcfg.hxx>
#include <tools/debug.hxx>
#include <tools/solar.h>
#include <osl/diagnose.h>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

using namespace utl;
using namespace com::sun::star::uno;

#define FILTERCFG_WORD_CODE             0x0001
#define FILTERCFG_WORD_STORAGE          0x0002
#define FILTERCFG_EXCEL_CODE            0x0004
#define FILTERCFG_EXCEL_STORAGE         0x0008
#define FILTERCFG_PPOINT_CODE           0x0010
#define FILTERCFG_PPOINT_STORAGE        0x0020
#define FILTERCFG_MATH_LOAD             0x0100
#define FILTERCFG_MATH_SAVE             0x0200
#define FILTERCFG_WRITER_LOAD           0x0400
#define FILTERCFG_WRITER_SAVE           0x0800
#define FILTERCFG_CALC_LOAD             0x1000
#define FILTERCFG_CALC_SAVE             0x2000
#define FILTERCFG_IMPRESS_LOAD          0x4000
#define FILTERCFG_IMPRESS_SAVE          0x8000
#define FILTERCFG_EXCEL_EXECTBL         0x10000
#define FILTERCFG_ENABLE_PPT_PREVIEW    0x20000
#define FILTERCFG_ENABLE_EXCEL_PREVIEW  0x40000
#define FILTERCFG_ENABLE_WORD_PREVIEW   0x80000
#define FILTERCFG_USE_ENHANCED_FIELDS   0x100000
#define FILTERCFG_WORD_WBCTBL           0x200000
#define FILTERCFG_SMARTART_SHAPE_LOAD   0x400000
#define FILTERCFG_CHAR_BACKGROUND_TO_HIGHLIGHTING   0x8000000

class SvtAppFilterOptions_Impl : public utl::ConfigItem
{
private:
    bool                bLoadVBA;
    bool                bSaveVBA;

protected:
    virtual void            ImplCommit() override;

public:
    explicit SvtAppFilterOptions_Impl(const OUString& rRoot) :
        utl::ConfigItem(rRoot),
        bLoadVBA(false),
        bSaveVBA(false)  {}
    virtual ~SvtAppFilterOptions_Impl();
    virtual void            Notify( const css::uno::Sequence<OUString>& aPropertyNames) override;
    void                    Load();

    bool                IsLoad() const {return bLoadVBA;}
    void                    SetLoad(bool bSet)
                            {
                                if(bSet != bLoadVBA)
                                    SetModified();
                                bLoadVBA = bSet;
                            }
    bool                IsSave() const {return bSaveVBA;}
    void                    SetSave(bool bSet)
                            {
                                if(bSet != bSaveVBA)
                                    SetModified();
                                bSaveVBA = bSet;
                            }
};

SvtAppFilterOptions_Impl::~SvtAppFilterOptions_Impl()
{
    assert(!IsModified()); // should have been committed
}

void    SvtAppFilterOptions_Impl::ImplCommit()
{
    Sequence<OUString> aNames(2);
    OUString* pNames = aNames.getArray();
    pNames[0] = "Load";
    pNames[1] = "Save";
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = cppu::UnoType<bool>::get();
    pValues[0].setValue(&bLoadVBA, rType);
    pValues[1].setValue(&bSaveVBA, rType);

    PutProperties(aNames, aValues);
}

void SvtAppFilterOptions_Impl::Notify( const Sequence< OUString >&  )
{
    // no listeners supported yet
}

void    SvtAppFilterOptions_Impl::Load()
{
    Sequence<OUString> aNames(2);
    OUString* pNames = aNames.getArray();
    pNames[0] = "Load";
    pNames[1] = "Save";

    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();

    if(pValues[0].hasValue())
        bLoadVBA = *static_cast<sal_Bool const *>(pValues[0].getValue());
    if(pValues[1].hasValue())
        bSaveVBA = *static_cast<sal_Bool const *>(pValues[1].getValue());
}

class SvtWriterFilterOptions_Impl : public SvtAppFilterOptions_Impl
{
private:
    bool                bLoadExecutable;

    virtual void            ImplCommit() override;

public:
    explicit SvtWriterFilterOptions_Impl(const OUString& rRoot) :
        SvtAppFilterOptions_Impl(rRoot),
        bLoadExecutable(false)
    {}
    void                    Load();

    bool                IsLoadExecutable() const {return bLoadExecutable;}
    void                    SetLoadExecutable(bool bSet)
                            {
                                if(bSet != bLoadExecutable)
                                    SetModified();
                                bLoadExecutable = bSet;
                            }
};

void SvtWriterFilterOptions_Impl::ImplCommit()
{
    SvtAppFilterOptions_Impl::ImplCommit();

    Sequence<OUString> aNames { "Executable" };
    Sequence<Any> aValues(1);
    aValues[0] <<= bLoadExecutable;

    PutProperties(aNames, aValues);
}

void SvtWriterFilterOptions_Impl::Load()
{
    SvtAppFilterOptions_Impl::Load();

    Sequence<OUString> aNames { "Executable" };

    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    if(pValues[0].hasValue())
        bLoadExecutable = *static_cast<sal_Bool const *>(pValues[0].getValue());
}

class SvtCalcFilterOptions_Impl : public SvtAppFilterOptions_Impl
{
private:
    bool                bLoadExecutable;

    virtual void            ImplCommit() override;

public:
    explicit SvtCalcFilterOptions_Impl(const OUString& rRoot) :
        SvtAppFilterOptions_Impl(rRoot),
        bLoadExecutable(false)
    {}
    void                    Load();

    bool                IsLoadExecutable() const {return bLoadExecutable;}
    void                    SetLoadExecutable(bool bSet)
                            {
                                if(bSet != bLoadExecutable)
                                    SetModified();
                                bLoadExecutable = bSet;
                            }
};

void SvtCalcFilterOptions_Impl::ImplCommit()
{
    SvtAppFilterOptions_Impl::ImplCommit();

    Sequence<OUString> aNames { "Executable" };
    Sequence<Any> aValues(1);
    aValues[0] <<= bLoadExecutable;

    PutProperties(aNames, aValues);
}

void SvtCalcFilterOptions_Impl::Load()
{
    SvtAppFilterOptions_Impl::Load();

    Sequence<OUString> aNames { "Executable" };

    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    if(pValues[0].hasValue())
        bLoadExecutable = *static_cast<sal_Bool const *>(pValues[0].getValue());
}

struct SvtFilterOptions_Impl
{
    sal_uLong nFlags;
    SvtWriterFilterOptions_Impl aWriterCfg;
    SvtCalcFilterOptions_Impl aCalcCfg;
    SvtAppFilterOptions_Impl aImpressCfg;

    SvtFilterOptions_Impl() :
        aWriterCfg("Office.Writer/Filter/Import/VBA"),
        aCalcCfg("Office.Calc/Filter/Import/VBA"),
        aImpressCfg("Office.Impress/Filter/Import/VBA")
    {
        nFlags = FILTERCFG_WORD_CODE |
            FILTERCFG_WORD_STORAGE |
            FILTERCFG_EXCEL_CODE |
            FILTERCFG_EXCEL_STORAGE |
            FILTERCFG_PPOINT_CODE |
            FILTERCFG_PPOINT_STORAGE |
            FILTERCFG_MATH_LOAD |
            FILTERCFG_MATH_SAVE |
            FILTERCFG_WRITER_LOAD |
            FILTERCFG_WRITER_SAVE |
            FILTERCFG_CALC_LOAD |
            FILTERCFG_CALC_SAVE |
            FILTERCFG_IMPRESS_LOAD |
            FILTERCFG_IMPRESS_SAVE |
            FILTERCFG_USE_ENHANCED_FIELDS |
            FILTERCFG_SMARTART_SHAPE_LOAD |
            FILTERCFG_CHAR_BACKGROUND_TO_HIGHLIGHTING;
        Load();
    }

    void SetFlag( sal_uLong nFlag, bool bSet );
    bool IsFlag( sal_uLong nFlag ) const;
    void Load()
    {
        aWriterCfg.Load();
        aCalcCfg.Load();
        aImpressCfg.Load();
    }
};

void SvtFilterOptions_Impl::SetFlag( sal_uLong nFlag, bool bSet )
{
    switch(nFlag)
    {
        case FILTERCFG_WORD_CODE:       aWriterCfg.SetLoad(bSet);break;
        case FILTERCFG_WORD_STORAGE:    aWriterCfg.SetSave(bSet);break;
        case FILTERCFG_WORD_WBCTBL: aWriterCfg.SetLoadExecutable(bSet);break;
        case FILTERCFG_EXCEL_CODE:      aCalcCfg.SetLoad(bSet);break;
        case FILTERCFG_EXCEL_STORAGE:   aCalcCfg.SetSave(bSet);break;
        case FILTERCFG_EXCEL_EXECTBL:   aCalcCfg.SetLoadExecutable(bSet);break;
        case FILTERCFG_PPOINT_CODE:     aImpressCfg.SetLoad(bSet);break;
        case FILTERCFG_PPOINT_STORAGE:  aImpressCfg.SetSave(bSet);break;
        default:
            if( bSet )
                nFlags |= nFlag;
            else
                nFlags &= ~nFlag;
    }
}

bool SvtFilterOptions_Impl::IsFlag( sal_uLong nFlag ) const
{
    bool bRet;
    switch(nFlag)
    {
        case FILTERCFG_WORD_CODE        : bRet = aWriterCfg.IsLoad();break;
        case FILTERCFG_WORD_STORAGE     : bRet = aWriterCfg.IsSave();break;
        case FILTERCFG_WORD_WBCTBL      : bRet = aWriterCfg.IsLoadExecutable();break;
        case FILTERCFG_EXCEL_CODE       : bRet = aCalcCfg.IsLoad();break;
        case FILTERCFG_EXCEL_STORAGE    : bRet = aCalcCfg.IsSave();break;
        case FILTERCFG_EXCEL_EXECTBL    : bRet = aCalcCfg.IsLoadExecutable();break;
        case FILTERCFG_PPOINT_CODE      : bRet = aImpressCfg.IsLoad();break;
        case FILTERCFG_PPOINT_STORAGE   : bRet = aImpressCfg.IsSave();break;
        default:
            bRet = 0 != (nFlags & nFlag );
    }
    return bRet;
}

SvtFilterOptions::SvtFilterOptions() :
    ConfigItem( "Office.Common/Filter/Microsoft" ),
    pImp(new SvtFilterOptions_Impl)
{
    EnableNotification(GetPropertyNames());
    Load();
}

SvtFilterOptions::~SvtFilterOptions()
{
    delete pImp;
}

const Sequence<OUString>& SvtFilterOptions::GetPropertyNames()
{
    static Sequence<OUString> aNames;
    if(!aNames.getLength())
    {
        int nCount = 14;
        aNames.realloc(nCount);
        static const char* aPropNames[] =
        {
            "Import/MathTypeToMath",            //  0
            "Import/WinWordToWriter",           //  1
            "Import/PowerPointToImpress",       //  2
            "Import/ExcelToCalc",               //  3
            "Export/MathToMathType",            //  4
            "Export/WriterToWinWord",           //  5
            "Export/ImpressToPowerPoint",       //  6
            "Export/CalcToExcel",               //  7
            "Export/EnablePowerPointPreview",   //  8
            "Export/EnableExcelPreview",        //  9
            "Export/EnableWordPreview",         // 10
            "Import/ImportWWFieldsAsEnhancedFields", // 11
            "Import/SmartArtToShapes",          // 12
            "Export/CharBackgroundToHighlighting"    // 13
        };
        OUString* pNames = aNames.getArray();
        for(int i = 0; i < nCount; i++)
            pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}

static sal_uLong lcl_GetFlag(sal_Int32 nProp)
{
    sal_uLong nFlag = 0;
    switch(nProp)
    {
        case  0: nFlag = FILTERCFG_MATH_LOAD; break;
        case  1: nFlag = FILTERCFG_WRITER_LOAD; break;
        case  2: nFlag = FILTERCFG_IMPRESS_LOAD; break;
        case  3: nFlag = FILTERCFG_CALC_LOAD; break;
        case  4: nFlag = FILTERCFG_MATH_SAVE; break;
        case  5: nFlag = FILTERCFG_WRITER_SAVE; break;
        case  6: nFlag = FILTERCFG_IMPRESS_SAVE; break;
        case  7: nFlag = FILTERCFG_CALC_SAVE; break;
        case  8: nFlag = FILTERCFG_ENABLE_PPT_PREVIEW; break;
        case  9: nFlag = FILTERCFG_ENABLE_EXCEL_PREVIEW; break;
        case 10: nFlag = FILTERCFG_ENABLE_WORD_PREVIEW; break;
        case 11: nFlag = FILTERCFG_USE_ENHANCED_FIELDS; break;
        case 12: nFlag = FILTERCFG_SMARTART_SHAPE_LOAD; break;
        case 13: nFlag = FILTERCFG_CHAR_BACKGROUND_TO_HIGHLIGHTING; break;

        default: OSL_FAIL("illegal value");
    }
    return nFlag;
}

void SvtFilterOptions::Notify( const Sequence<OUString>& )
{
    Load();
}

void SvtFilterOptions::ImplCommit()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = cppu::UnoType<bool>::get();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        sal_uLong nFlag = lcl_GetFlag(nProp);
        sal_Bool bVal = pImp->IsFlag( nFlag);
        pValues[nProp].setValue(&bVal, rType);

    }
    PutProperties(aNames, aValues);
}

void SvtFilterOptions::Load()
{
    pImp->Load();
    const Sequence<OUString>& rNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(rNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == rNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == rNames.getLength())
    {
        for(int nProp = 0; nProp < rNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                bool bVal = *static_cast<sal_Bool const *>(pValues[nProp].getValue());
                sal_uLong nFlag = lcl_GetFlag(nProp);
                pImp->SetFlag( nFlag, bVal);
            }
        }
    }
}

void SvtFilterOptions::SetLoadWordBasicCode( bool bFlag )
{
    pImp->SetFlag( FILTERCFG_WORD_CODE, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsLoadWordBasicCode() const
{
    return pImp->IsFlag( FILTERCFG_WORD_CODE );
}

void SvtFilterOptions::SetLoadWordBasicExecutable( bool bFlag )
{
    pImp->SetFlag( FILTERCFG_WORD_WBCTBL, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsLoadWordBasicExecutable() const
{
    return pImp->IsFlag( FILTERCFG_WORD_WBCTBL );
}

void SvtFilterOptions::SetLoadWordBasicStorage( bool bFlag )
{
    pImp->SetFlag( FILTERCFG_WORD_STORAGE, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsLoadWordBasicStorage() const
{
    return pImp->IsFlag( FILTERCFG_WORD_STORAGE );
}

void SvtFilterOptions::SetLoadExcelBasicCode( bool bFlag )
{
    pImp->SetFlag( FILTERCFG_EXCEL_CODE, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsLoadExcelBasicCode() const
{
    return pImp->IsFlag( FILTERCFG_EXCEL_CODE );
}

void SvtFilterOptions::SetLoadExcelBasicExecutable( bool bFlag )
{
    pImp->SetFlag( FILTERCFG_EXCEL_EXECTBL, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsLoadExcelBasicExecutable() const
{
    return pImp->IsFlag( FILTERCFG_EXCEL_EXECTBL );
}

void SvtFilterOptions::SetLoadExcelBasicStorage( bool bFlag )
{
    pImp->SetFlag( FILTERCFG_EXCEL_STORAGE, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsLoadExcelBasicStorage() const
{
    return pImp->IsFlag( FILTERCFG_EXCEL_STORAGE );
}

void SvtFilterOptions::SetLoadPPointBasicCode( bool bFlag )
{
    pImp->SetFlag( FILTERCFG_PPOINT_CODE, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsLoadPPointBasicCode() const
{
    return pImp->IsFlag( FILTERCFG_PPOINT_CODE );
}

void SvtFilterOptions::SetLoadPPointBasicStorage( bool bFlag )
{
    pImp->SetFlag( FILTERCFG_PPOINT_STORAGE, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsLoadPPointBasicStorage() const
{
    return pImp->IsFlag( FILTERCFG_PPOINT_STORAGE );
}

bool SvtFilterOptions::IsMathType2Math() const
{
    return pImp->IsFlag( FILTERCFG_MATH_LOAD );
}

void SvtFilterOptions::SetMathType2Math( bool bFlag )
{
    pImp->SetFlag( FILTERCFG_MATH_LOAD, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsMath2MathType() const
{
    return pImp->IsFlag( FILTERCFG_MATH_SAVE );
}

void SvtFilterOptions::SetMath2MathType( bool bFlag )
{
    pImp->SetFlag( FILTERCFG_MATH_SAVE, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsWinWord2Writer() const
{
    return pImp->IsFlag( FILTERCFG_WRITER_LOAD );
}

void SvtFilterOptions::SetWinWord2Writer( bool bFlag )
{
    pImp->SetFlag( FILTERCFG_WRITER_LOAD, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsWriter2WinWord() const
{
    return pImp->IsFlag( FILTERCFG_WRITER_SAVE );
}

void SvtFilterOptions::SetWriter2WinWord( bool bFlag )
{
    pImp->SetFlag( FILTERCFG_WRITER_SAVE, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsUseEnhancedFields() const
{
    return pImp->IsFlag( FILTERCFG_USE_ENHANCED_FIELDS );
}

bool SvtFilterOptions::IsExcel2Calc() const
{
    return pImp->IsFlag( FILTERCFG_CALC_LOAD );
}

void SvtFilterOptions::SetExcel2Calc( bool bFlag )
{
    pImp->SetFlag( FILTERCFG_CALC_LOAD, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsCalc2Excel() const
{
    return pImp->IsFlag( FILTERCFG_CALC_SAVE );
}

void SvtFilterOptions::SetCalc2Excel( bool bFlag )
{
    pImp->SetFlag( FILTERCFG_CALC_SAVE, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsPowerPoint2Impress() const
{
    return pImp->IsFlag( FILTERCFG_IMPRESS_LOAD );
}

void SvtFilterOptions::SetPowerPoint2Impress( bool bFlag )
{
    pImp->SetFlag( FILTERCFG_IMPRESS_LOAD, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsImpress2PowerPoint() const
{
    return pImp->IsFlag( FILTERCFG_IMPRESS_SAVE );
}

void SvtFilterOptions::SetImpress2PowerPoint( bool bFlag )
{
    pImp->SetFlag( FILTERCFG_IMPRESS_SAVE, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsSmartArt2Shape() const
{
    return pImp->IsFlag( FILTERCFG_SMARTART_SHAPE_LOAD );
}

void SvtFilterOptions::SetSmartArt2Shape( bool bFlag )
{
    pImp->SetFlag( FILTERCFG_SMARTART_SHAPE_LOAD, bFlag );
    SetModified();
}

namespace
{
    class theFilterOptions
         : public rtl::Static<SvtFilterOptions, theFilterOptions>
    {
    };
}

SvtFilterOptions& SvtFilterOptions::Get()
{
    return theFilterOptions::get();
}

bool SvtFilterOptions::IsEnablePPTPreview() const
{
    return pImp->IsFlag( FILTERCFG_ENABLE_PPT_PREVIEW );
}

bool SvtFilterOptions::IsEnableCalcPreview() const
{
    return pImp->IsFlag( FILTERCFG_ENABLE_EXCEL_PREVIEW );
}

bool SvtFilterOptions::IsEnableWordPreview() const
{
    return pImp->IsFlag( FILTERCFG_ENABLE_WORD_PREVIEW );
}


bool SvtFilterOptions::IsCharBackground2Highlighting() const
{
    return pImp->IsFlag( FILTERCFG_CHAR_BACKGROUND_TO_HIGHLIGHTING );
}

bool SvtFilterOptions::IsCharBackground2Shading() const
{
    return !pImp->IsFlag( FILTERCFG_CHAR_BACKGROUND_TO_HIGHLIGHTING );
}

void SvtFilterOptions::SetCharBackground2Highlighting()
{
    pImp->SetFlag( FILTERCFG_CHAR_BACKGROUND_TO_HIGHLIGHTING, true );
    SetModified();
}

void SvtFilterOptions::SetCharBackground2Shading()
{
    pImp->SetFlag( FILTERCFG_CHAR_BACKGROUND_TO_HIGHLIGHTING, false );
    SetModified();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
