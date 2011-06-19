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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"

#include <unotools/fltrcfg.hxx>
#include <tools/debug.hxx>

#include <rtl/logfile.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

using namespace utl;
using namespace com::sun::star::uno;

using ::rtl::OUString;

#define C2U(cChar) OUString(RTL_CONSTASCII_USTRINGPARAM(cChar))

// -----------------------------------------------------------------------
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

static SvtFilterOptions* pOptions=0;

class SvtAppFilterOptions_Impl : public utl::ConfigItem
{
    sal_Bool                bLoadVBA;
    sal_Bool                bSaveVBA;
public:
    SvtAppFilterOptions_Impl(const OUString& rRoot) :
        utl::ConfigItem(rRoot),
        bLoadVBA(sal_False),
        bSaveVBA(sal_False)  {}
    ~SvtAppFilterOptions_Impl();
    virtual void            Commit();
    virtual void            Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);
    void                    Load();

    sal_Bool                IsLoad() const {return bLoadVBA;}
    void                    SetLoad(sal_Bool bSet)
                            {
                                if(bSet != bLoadVBA)
                                    SetModified();
                                bLoadVBA = bSet;
                            }
    sal_Bool                IsSave() const {return bSaveVBA;}
    void                    SetSave(sal_Bool bSet)
                            {
                                if(bSet != bSaveVBA)
                                    SetModified();
                                bSaveVBA = bSet;
                            }
};

SvtAppFilterOptions_Impl::~SvtAppFilterOptions_Impl()
{
    if(IsModified())
        Commit();
}

void    SvtAppFilterOptions_Impl::Commit()
{
    Sequence<OUString> aNames(2);
    OUString* pNames = aNames.getArray();
    pNames[0] = C2U("Load");
    pNames[1] = C2U("Save");
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    pValues[0].setValue(&bLoadVBA, rType);
    pValues[1].setValue(&bSaveVBA, rType);

    PutProperties(aNames, aValues);
}

void SvtAppFilterOptions_Impl::Notify( const Sequence< rtl::OUString >&  )
{
    // no listeners supported yet
}

void    SvtAppFilterOptions_Impl::Load()
{
    Sequence<OUString> aNames(2);
    OUString* pNames = aNames.getArray();
    pNames[0] = C2U("Load");
    pNames[1] = C2U("Save");

    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();

    if(pValues[0].hasValue())
        bLoadVBA = *(sal_Bool*)pValues[0].getValue();
    if(pValues[1].hasValue())
        bSaveVBA = *(sal_Bool*)pValues[1].getValue();
}

// -----------------------------------------------------------------------
class SvtWriterFilterOptions_Impl : public SvtAppFilterOptions_Impl
{
    sal_Bool                bLoadExecutable;
public:
    SvtWriterFilterOptions_Impl(const OUString& rRoot) :
        SvtAppFilterOptions_Impl(rRoot),
        bLoadExecutable(sal_False)
    {}
    virtual void            Commit();
    void                    Load();

    sal_Bool                IsLoadExecutable() const {return bLoadExecutable;}
    void                    SetLoadExecutable(sal_Bool bSet)
                            {
                                if(bSet != bLoadExecutable)
                                    SetModified();
                                bLoadExecutable = bSet;
                            }
};

void SvtWriterFilterOptions_Impl::Commit()
{
    SvtAppFilterOptions_Impl::Commit();

    Sequence<OUString> aNames(1);
    aNames[0] = C2U("Executable");
    Sequence<Any> aValues(1);
    aValues[0] <<= bLoadExecutable;

    PutProperties(aNames, aValues);
}

void SvtWriterFilterOptions_Impl::Load()
{
    SvtAppFilterOptions_Impl::Load();

    Sequence<OUString> aNames(1);
    aNames[0] = C2U("Executable");

    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    if(pValues[0].hasValue())
        bLoadExecutable = *(sal_Bool*)pValues[0].getValue();
}

// -----------------------------------------------------------------------
class SvtCalcFilterOptions_Impl : public SvtAppFilterOptions_Impl
{
    sal_Bool                bLoadExecutable;
public:
    SvtCalcFilterOptions_Impl(const OUString& rRoot) :
        SvtAppFilterOptions_Impl(rRoot),
        bLoadExecutable(sal_False)
    {}
    virtual void            Commit();
    void                    Load();

    sal_Bool                IsLoadExecutable() const {return bLoadExecutable;}
    void                    SetLoadExecutable(sal_Bool bSet)
                            {
                                if(bSet != bLoadExecutable)
                                    SetModified();
                                bLoadExecutable = bSet;
                            }
};

void SvtCalcFilterOptions_Impl::Commit()
{
    SvtAppFilterOptions_Impl::Commit();

    Sequence<OUString> aNames(1);
    aNames[0] = C2U("Executable");
    Sequence<Any> aValues(1);
    aValues[0] <<= bLoadExecutable;

    PutProperties(aNames, aValues);
}

void SvtCalcFilterOptions_Impl::Load()
{
    SvtAppFilterOptions_Impl::Load();

    Sequence<OUString> aNames(1);
    aNames[0] = C2U("Executable");

    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    if(pValues[0].hasValue())
        bLoadExecutable = *(sal_Bool*)pValues[0].getValue();
}

struct SvtFilterOptions_Impl
{
    sal_uLong nFlags;
    SvtWriterFilterOptions_Impl aWriterCfg;
    SvtCalcFilterOptions_Impl aCalcCfg;
    SvtAppFilterOptions_Impl aImpressCfg;

    SvtFilterOptions_Impl() :
        aWriterCfg(C2U("Office.Writer/Filter/Import/VBA")),
        aCalcCfg(C2U("Office.Calc/Filter/Import/VBA")),
        aImpressCfg(C2U("Office.Impress/Filter/Import/VBA"))
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
            FILTERCFG_USE_ENHANCED_FIELDS;
        Load();
    }

    void SetFlag( sal_uLong nFlag, sal_Bool bSet );
    sal_Bool IsFlag( sal_uLong nFlag ) const;
    void Load()
    {
        aWriterCfg.Load();
        aCalcCfg.Load();
        aImpressCfg.Load();
    }
};

void SvtFilterOptions_Impl::SetFlag( sal_uLong nFlag, sal_Bool bSet )
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

sal_Bool SvtFilterOptions_Impl::IsFlag( sal_uLong nFlag ) const
{
    sal_Bool bRet;
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

// -----------------------------------------------------------------------

SvtFilterOptions::SvtFilterOptions() :
    ConfigItem( C2U("Office.Common/Filter/Microsoft") ),
    pImp(new SvtFilterOptions_Impl)
{
    RTL_LOGFILE_CONTEXT(aLog, "unotools SvtFilterOptions::SvtFilterOptions()");
    EnableNotification(GetPropertyNames());
    Load();
}
// -----------------------------------------------------------------------
SvtFilterOptions::~SvtFilterOptions()
{
    delete pImp;
}

const Sequence<OUString>& SvtFilterOptions::GetPropertyNames()
{
    static Sequence<OUString> aNames;
    if(!aNames.getLength())
    {
        int nCount = 12;
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
            "Import/ImportWWFieldsAsEnhancedFields" // 11
        };
        OUString* pNames = aNames.getArray();
        for(int i = 0; i < nCount; i++)
            pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}
//-----------------------------------------------------------------------
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

        default: OSL_FAIL("illegal value");
    }
    return nFlag;
}

void SvtFilterOptions::Notify( const Sequence<OUString>& )
{
    Load();
}

void SvtFilterOptions::Commit()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
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
                sal_Bool bVal = *(sal_Bool*)pValues[nProp].getValue();
                sal_uLong nFlag = lcl_GetFlag(nProp);
                pImp->SetFlag( nFlag, bVal);
            }
        }
    }
}
// -----------------------------------------------------------------------

void SvtFilterOptions::SetLoadWordBasicCode( sal_Bool bFlag )
{
    pImp->SetFlag( FILTERCFG_WORD_CODE, bFlag );
    SetModified();
}

sal_Bool SvtFilterOptions::IsLoadWordBasicCode() const
{
    return pImp->IsFlag( FILTERCFG_WORD_CODE );
}

void SvtFilterOptions::SetLoadWordBasicExecutable( sal_Bool bFlag )
{
    pImp->SetFlag( FILTERCFG_WORD_WBCTBL, bFlag );
    SetModified();
}

sal_Bool SvtFilterOptions::IsLoadWordBasicExecutable() const
{
    return pImp->IsFlag( FILTERCFG_WORD_WBCTBL );
}

void SvtFilterOptions::SetLoadWordBasicStorage( sal_Bool bFlag )
{
    pImp->SetFlag( FILTERCFG_WORD_STORAGE, bFlag );
    SetModified();
}

sal_Bool SvtFilterOptions::IsLoadWordBasicStorage() const
{
    return pImp->IsFlag( FILTERCFG_WORD_STORAGE );
}

// -----------------------------------------------------------------------

void SvtFilterOptions::SetLoadExcelBasicCode( sal_Bool bFlag )
{
    pImp->SetFlag( FILTERCFG_EXCEL_CODE, bFlag );
    SetModified();
}

sal_Bool SvtFilterOptions::IsLoadExcelBasicCode() const
{
    return pImp->IsFlag( FILTERCFG_EXCEL_CODE );
}

void SvtFilterOptions::SetLoadExcelBasicExecutable( sal_Bool bFlag )
{
    pImp->SetFlag( FILTERCFG_EXCEL_EXECTBL, bFlag );
    SetModified();
}

sal_Bool SvtFilterOptions::IsLoadExcelBasicExecutable() const
{
    return pImp->IsFlag( FILTERCFG_EXCEL_EXECTBL );
}

void SvtFilterOptions::SetLoadExcelBasicStorage( sal_Bool bFlag )
{
    pImp->SetFlag( FILTERCFG_EXCEL_STORAGE, bFlag );
    SetModified();
}

sal_Bool SvtFilterOptions::IsLoadExcelBasicStorage() const
{
    return pImp->IsFlag( FILTERCFG_EXCEL_STORAGE );
}

// -----------------------------------------------------------------------

void SvtFilterOptions::SetLoadPPointBasicCode( sal_Bool bFlag )
{
    pImp->SetFlag( FILTERCFG_PPOINT_CODE, bFlag );
    SetModified();
}

sal_Bool SvtFilterOptions::IsLoadPPointBasicCode() const
{
    return pImp->IsFlag( FILTERCFG_PPOINT_CODE );
}

void SvtFilterOptions::SetLoadPPointBasicStorage( sal_Bool bFlag )
{
    pImp->SetFlag( FILTERCFG_PPOINT_STORAGE, bFlag );
    SetModified();
}

sal_Bool SvtFilterOptions::IsLoadPPointBasicStorage() const
{
    return pImp->IsFlag( FILTERCFG_PPOINT_STORAGE );
}

// -----------------------------------------------------------------------

sal_Bool SvtFilterOptions::IsMathType2Math() const
{
    return pImp->IsFlag( FILTERCFG_MATH_LOAD );
}

void SvtFilterOptions::SetMathType2Math( sal_Bool bFlag )
{
    pImp->SetFlag( FILTERCFG_MATH_LOAD, bFlag );
    SetModified();
}

sal_Bool SvtFilterOptions::IsMath2MathType() const
{
    return pImp->IsFlag( FILTERCFG_MATH_SAVE );
}

void SvtFilterOptions::SetMath2MathType( sal_Bool bFlag )
{
    pImp->SetFlag( FILTERCFG_MATH_SAVE, bFlag );
    SetModified();
}


// -----------------------------------------------------------------------
sal_Bool SvtFilterOptions::IsWinWord2Writer() const
{
    return pImp->IsFlag( FILTERCFG_WRITER_LOAD );
}

void SvtFilterOptions::SetWinWord2Writer( sal_Bool bFlag )
{
    pImp->SetFlag( FILTERCFG_WRITER_LOAD, bFlag );
    SetModified();
}

sal_Bool SvtFilterOptions::IsWriter2WinWord() const
{
    return pImp->IsFlag( FILTERCFG_WRITER_SAVE );
}

void SvtFilterOptions::SetWriter2WinWord( sal_Bool bFlag )
{
    pImp->SetFlag( FILTERCFG_WRITER_SAVE, bFlag );
    SetModified();
}

sal_Bool SvtFilterOptions::IsUseEnhancedFields() const
{
    return pImp->IsFlag( FILTERCFG_USE_ENHANCED_FIELDS );
}

void SvtFilterOptions::SetUseEnhancedFields( sal_Bool bFlag )
{
    pImp->SetFlag( FILTERCFG_USE_ENHANCED_FIELDS, bFlag );
    SetModified();
}

// -----------------------------------------------------------------------
sal_Bool SvtFilterOptions::IsExcel2Calc() const
{
    return pImp->IsFlag( FILTERCFG_CALC_LOAD );
}

void SvtFilterOptions::SetExcel2Calc( sal_Bool bFlag )
{
    pImp->SetFlag( FILTERCFG_CALC_LOAD, bFlag );
    SetModified();
}

sal_Bool SvtFilterOptions::IsCalc2Excel() const
{
    return pImp->IsFlag( FILTERCFG_CALC_SAVE );
}

void SvtFilterOptions::SetCalc2Excel( sal_Bool bFlag )
{
    pImp->SetFlag( FILTERCFG_CALC_SAVE, bFlag );
    SetModified();
}


// -----------------------------------------------------------------------
sal_Bool SvtFilterOptions::IsPowerPoint2Impress() const
{
    return pImp->IsFlag( FILTERCFG_IMPRESS_LOAD );
}

void SvtFilterOptions::SetPowerPoint2Impress( sal_Bool bFlag )
{
    pImp->SetFlag( FILTERCFG_IMPRESS_LOAD, bFlag );
    SetModified();
}

sal_Bool SvtFilterOptions::IsImpress2PowerPoint() const
{
    return pImp->IsFlag( FILTERCFG_IMPRESS_SAVE );
}

void SvtFilterOptions::SetImpress2PowerPoint( sal_Bool bFlag )
{
    pImp->SetFlag( FILTERCFG_IMPRESS_SAVE, bFlag );
    SetModified();
}

SvtFilterOptions* SvtFilterOptions::Get()
{
    if ( !pOptions )
        pOptions = new SvtFilterOptions;
    return pOptions;
}

// -----------------------------------------------------------------------

sal_Bool SvtFilterOptions::IsEnablePPTPreview() const
{
    return pImp->IsFlag( FILTERCFG_ENABLE_PPT_PREVIEW );
}


sal_Bool SvtFilterOptions::IsEnableCalcPreview() const
{
    return pImp->IsFlag( FILTERCFG_ENABLE_EXCEL_PREVIEW );
}


sal_Bool SvtFilterOptions::IsEnableWordPreview() const
{
    return pImp->IsFlag( FILTERCFG_ENABLE_WORD_PREVIEW );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
