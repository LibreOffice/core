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

#include <o3tl/any.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <unotools/fltrcfg.hxx>
#include <tools/debug.hxx>
#include <osl/diagnose.h>

#include <com/sun/star/uno/Sequence.hxx>

using namespace utl;
using namespace com::sun::star::uno;

enum class ConfigFlags {
    NONE                         = 0x0000000,
    WordCode                     = 0x0000001,
    WordStorage                  = 0x0000002,
    ExcelCode                    = 0x0000004,
    ExcelStorage                 = 0x0000008,
    PowerPointCode               = 0x0000010,
    PowerPointStorage            = 0x0000020,
    MathLoad                     = 0x0000100,
    MathSave                     = 0x0000200,
    WriterLoad                   = 0x0000400,
    WriterSave                   = 0x0000800,
    CalcLoad                     = 0x0001000,
    CalcSave                     = 0x0002000,
    ImpressLoad                  = 0x0004000,
    ImpressSave                  = 0x0008000,
    ExcelExecTbl                 = 0x0010000,
    EnablePowerPointPreview      = 0x0020000,
    EnableExcelPreview           = 0x0040000,
    EnableWordPreview            = 0x0080000,
    UseEnhancedFields            = 0x0100000,
    WordWbctbl                   = 0x0200000,
    SmartArtShapeLoad            = 0x0400000,
    CharBackgroundToHighlighting = 0x8000000,
    CreateMSOLockFiles           = 0x2000000,
    VisioLoad                    = 0x4000000,
};
namespace o3tl {
    template<> struct typed_flags<ConfigFlags> : is_typed_flags<ConfigFlags, 0xe7fff3f> {};
}

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
    virtual ~SvtAppFilterOptions_Impl() override;
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
    PutProperties(
        {"Load", "Save"}, {css::uno::Any(bLoadVBA), css::uno::Any(bSaveVBA)});
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
        bLoadVBA = *o3tl::doAccess<bool>(pValues[0]);
    if(pValues[1].hasValue())
        bSaveVBA = *o3tl::doAccess<bool>(pValues[1]);
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
        bLoadExecutable = *o3tl::doAccess<bool>(pValues[0]);
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
        bLoadExecutable = *o3tl::doAccess<bool>(pValues[0]);
}

struct SvtFilterOptions_Impl
{
    ConfigFlags nFlags;
    SvtWriterFilterOptions_Impl aWriterCfg;
    SvtCalcFilterOptions_Impl aCalcCfg;
    SvtAppFilterOptions_Impl aImpressCfg;

    SvtFilterOptions_Impl() :
        aWriterCfg("Office.Writer/Filter/Import/VBA"),
        aCalcCfg("Office.Calc/Filter/Import/VBA"),
        aImpressCfg("Office.Impress/Filter/Import/VBA")
    {
        nFlags = ConfigFlags::WordCode |
            ConfigFlags::WordStorage |
            ConfigFlags::ExcelCode |
            ConfigFlags::ExcelStorage |
            ConfigFlags::PowerPointCode |
            ConfigFlags::PowerPointStorage |
            ConfigFlags::MathLoad |
            ConfigFlags::MathSave |
            ConfigFlags::WriterLoad |
            ConfigFlags::WriterSave |
            ConfigFlags::CalcLoad |
            ConfigFlags::CalcSave |
            ConfigFlags::ImpressLoad |
            ConfigFlags::ImpressSave |
            ConfigFlags::UseEnhancedFields |
            ConfigFlags::SmartArtShapeLoad |
            ConfigFlags::CharBackgroundToHighlighting|
            ConfigFlags::CreateMSOLockFiles;
        Load();
    }

    void SetFlag( ConfigFlags nFlag, bool bSet );
    bool IsFlag( ConfigFlags nFlag ) const;
    void Load()
    {
        aWriterCfg.Load();
        aCalcCfg.Load();
        aImpressCfg.Load();
    }
};

void SvtFilterOptions_Impl::SetFlag( ConfigFlags nFlag, bool bSet )
{
    switch(nFlag)
    {
        case ConfigFlags::WordCode:       aWriterCfg.SetLoad(bSet);break;
        case ConfigFlags::WordStorage:    aWriterCfg.SetSave(bSet);break;
        case ConfigFlags::WordWbctbl: aWriterCfg.SetLoadExecutable(bSet);break;
        case ConfigFlags::ExcelCode:      aCalcCfg.SetLoad(bSet);break;
        case ConfigFlags::ExcelStorage:   aCalcCfg.SetSave(bSet);break;
        case ConfigFlags::ExcelExecTbl:   aCalcCfg.SetLoadExecutable(bSet);break;
        case ConfigFlags::PowerPointCode:     aImpressCfg.SetLoad(bSet);break;
        case ConfigFlags::PowerPointStorage:  aImpressCfg.SetSave(bSet);break;
        default:
            if( bSet )
                nFlags |= nFlag;
            else
                nFlags &= ~nFlag;
    }
}

bool SvtFilterOptions_Impl::IsFlag( ConfigFlags nFlag ) const
{
    bool bRet;
    switch(nFlag)
    {
        case ConfigFlags::WordCode        : bRet = aWriterCfg.IsLoad();break;
        case ConfigFlags::WordStorage     : bRet = aWriterCfg.IsSave();break;
        case ConfigFlags::WordWbctbl      : bRet = aWriterCfg.IsLoadExecutable();break;
        case ConfigFlags::ExcelCode       : bRet = aCalcCfg.IsLoad();break;
        case ConfigFlags::ExcelStorage    : bRet = aCalcCfg.IsSave();break;
        case ConfigFlags::ExcelExecTbl    : bRet = aCalcCfg.IsLoadExecutable();break;
        case ConfigFlags::PowerPointCode      : bRet = aImpressCfg.IsLoad();break;
        case ConfigFlags::PowerPointStorage   : bRet = aImpressCfg.IsSave();break;
        default:
            bRet = bool(nFlags & nFlag );
    }
    return bRet;
}

namespace {

const Sequence<OUString>& GetPropertyNames()
{
    static Sequence<OUString> const aNames
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
            "Export/CharBackgroundToHighlighting",   // 13
            "Import/CreateMSOLockFiles",        // 14
            "Import/VisioToDraw"                // 15
    };
    return aNames;
}

}

SvtFilterOptions::SvtFilterOptions() :
    ConfigItem( "Office.Common/Filter/Microsoft" ),
    pImpl(new SvtFilterOptions_Impl)
{
    EnableNotification(GetPropertyNames());
    Load();
}

SvtFilterOptions::~SvtFilterOptions()
{
}

static ConfigFlags lcl_GetFlag(sal_Int32 nProp)
{
    ConfigFlags nFlag = ConfigFlags::NONE;
    switch(nProp)
    {
        case  0: nFlag = ConfigFlags::MathLoad; break;
        case  1: nFlag = ConfigFlags::WriterLoad; break;
        case  2: nFlag = ConfigFlags::ImpressLoad; break;
        case  3: nFlag = ConfigFlags::CalcLoad; break;
        case  4: nFlag = ConfigFlags::MathSave; break;
        case  5: nFlag = ConfigFlags::WriterSave; break;
        case  6: nFlag = ConfigFlags::ImpressSave; break;
        case  7: nFlag = ConfigFlags::CalcSave; break;
        case  8: nFlag = ConfigFlags::EnablePowerPointPreview; break;
        case  9: nFlag = ConfigFlags::EnableExcelPreview; break;
        case 10: nFlag = ConfigFlags::EnableWordPreview; break;
        case 11: nFlag = ConfigFlags::UseEnhancedFields; break;
        case 12: nFlag = ConfigFlags::SmartArtShapeLoad; break;
        case 13: nFlag = ConfigFlags::CharBackgroundToHighlighting; break;
        case 14: nFlag = ConfigFlags::CreateMSOLockFiles; break;
        case 15:
            nFlag = ConfigFlags::VisioLoad;
            break;

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

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        ConfigFlags nFlag = lcl_GetFlag(nProp);
        pValues[nProp] <<= pImpl->IsFlag(nFlag);

    }
    PutProperties(aNames, aValues);
}

void SvtFilterOptions::Load()
{
    pImpl->Load();
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
                bool bVal = *o3tl::doAccess<bool>(pValues[nProp]);
                ConfigFlags nFlag = lcl_GetFlag(nProp);
                pImpl->SetFlag( nFlag, bVal);
            }
        }
    }
}

void SvtFilterOptions::SetLoadWordBasicCode( bool bFlag )
{
    pImpl->SetFlag( ConfigFlags::WordCode, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsLoadWordBasicCode() const
{
    return pImpl->IsFlag( ConfigFlags::WordCode );
}

void SvtFilterOptions::SetLoadWordBasicExecutable( bool bFlag )
{
    pImpl->SetFlag( ConfigFlags::WordWbctbl, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsLoadWordBasicExecutable() const
{
    return pImpl->IsFlag( ConfigFlags::WordWbctbl );
}

void SvtFilterOptions::SetLoadWordBasicStorage( bool bFlag )
{
    pImpl->SetFlag( ConfigFlags::WordStorage, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsLoadWordBasicStorage() const
{
    return pImpl->IsFlag( ConfigFlags::WordStorage );
}

void SvtFilterOptions::SetLoadExcelBasicCode( bool bFlag )
{
    pImpl->SetFlag( ConfigFlags::ExcelCode, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsLoadExcelBasicCode() const
{
    return pImpl->IsFlag( ConfigFlags::ExcelCode );
}

void SvtFilterOptions::SetLoadExcelBasicExecutable( bool bFlag )
{
    pImpl->SetFlag( ConfigFlags::ExcelExecTbl, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsLoadExcelBasicExecutable() const
{
    return pImpl->IsFlag( ConfigFlags::ExcelExecTbl );
}

void SvtFilterOptions::SetLoadExcelBasicStorage( bool bFlag )
{
    pImpl->SetFlag( ConfigFlags::ExcelStorage, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsLoadExcelBasicStorage() const
{
    return pImpl->IsFlag( ConfigFlags::ExcelStorage );
}

void SvtFilterOptions::SetLoadPPointBasicCode( bool bFlag )
{
    pImpl->SetFlag( ConfigFlags::PowerPointCode, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsLoadPPointBasicCode() const
{
    return pImpl->IsFlag( ConfigFlags::PowerPointCode );
}

void SvtFilterOptions::SetLoadPPointBasicStorage( bool bFlag )
{
    pImpl->SetFlag( ConfigFlags::PowerPointStorage, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsLoadPPointBasicStorage() const
{
    return pImpl->IsFlag( ConfigFlags::PowerPointStorage );
}

bool SvtFilterOptions::IsMathType2Math() const
{
    return pImpl->IsFlag( ConfigFlags::MathLoad );
}

void SvtFilterOptions::SetMathType2Math( bool bFlag )
{
    pImpl->SetFlag( ConfigFlags::MathLoad, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsMath2MathType() const
{
    return pImpl->IsFlag( ConfigFlags::MathSave );
}

void SvtFilterOptions::SetMath2MathType( bool bFlag )
{
    pImpl->SetFlag( ConfigFlags::MathSave, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsWinWord2Writer() const
{
    return pImpl->IsFlag( ConfigFlags::WriterLoad );
}

void SvtFilterOptions::SetWinWord2Writer( bool bFlag )
{
    pImpl->SetFlag( ConfigFlags::WriterLoad, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsWriter2WinWord() const
{
    return pImpl->IsFlag( ConfigFlags::WriterSave );
}

void SvtFilterOptions::SetWriter2WinWord( bool bFlag )
{
    pImpl->SetFlag( ConfigFlags::WriterSave, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsUseEnhancedFields() const
{
    return pImpl->IsFlag( ConfigFlags::UseEnhancedFields );
}

bool SvtFilterOptions::IsExcel2Calc() const
{
    return pImpl->IsFlag( ConfigFlags::CalcLoad );
}

void SvtFilterOptions::SetExcel2Calc( bool bFlag )
{
    pImpl->SetFlag( ConfigFlags::CalcLoad, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsCalc2Excel() const
{
    return pImpl->IsFlag( ConfigFlags::CalcSave );
}

void SvtFilterOptions::SetCalc2Excel( bool bFlag )
{
    pImpl->SetFlag( ConfigFlags::CalcSave, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsPowerPoint2Impress() const
{
    return pImpl->IsFlag( ConfigFlags::ImpressLoad );
}

void SvtFilterOptions::SetPowerPoint2Impress( bool bFlag )
{
    pImpl->SetFlag( ConfigFlags::ImpressLoad, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsImpress2PowerPoint() const
{
    return pImpl->IsFlag( ConfigFlags::ImpressSave );
}

void SvtFilterOptions::SetImpress2PowerPoint( bool bFlag )
{
    pImpl->SetFlag( ConfigFlags::ImpressSave, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsSmartArt2Shape() const
{
    return pImpl->IsFlag( ConfigFlags::SmartArtShapeLoad );
}

void SvtFilterOptions::SetSmartArt2Shape( bool bFlag )
{
    pImpl->SetFlag( ConfigFlags::SmartArtShapeLoad, bFlag );
    SetModified();
}

bool SvtFilterOptions::IsVisio2Draw() const { return pImpl->IsFlag(ConfigFlags::VisioLoad); }

void SvtFilterOptions::SetVisio2Draw(bool bFlag)
{
    pImpl->SetFlag(ConfigFlags::VisioLoad, bFlag);
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
    return pImpl->IsFlag( ConfigFlags::EnablePowerPointPreview );
}

bool SvtFilterOptions::IsEnableCalcPreview() const
{
    return pImpl->IsFlag( ConfigFlags::EnableExcelPreview );
}

bool SvtFilterOptions::IsEnableWordPreview() const
{
    return pImpl->IsFlag( ConfigFlags::EnableWordPreview );
}

bool SvtFilterOptions::IsCharBackground2Highlighting() const
{
    return pImpl->IsFlag( ConfigFlags::CharBackgroundToHighlighting );
}

bool SvtFilterOptions::IsCharBackground2Shading() const
{
    return !pImpl->IsFlag( ConfigFlags::CharBackgroundToHighlighting );
}

void SvtFilterOptions::SetCharBackground2Highlighting()
{
    pImpl->SetFlag( ConfigFlags::CharBackgroundToHighlighting, true );
    SetModified();
}

void SvtFilterOptions::SetCharBackground2Shading()
{
    pImpl->SetFlag( ConfigFlags::CharBackgroundToHighlighting, false );
    SetModified();
}

bool SvtFilterOptions::IsMSOLockFileCreationIsEnabled() const
{
    return pImpl->IsFlag( ConfigFlags::CreateMSOLockFiles );
}

void SvtFilterOptions::EnableMSOLockFileCreation(bool bEnable)
{
    pImpl->SetFlag( ConfigFlags::CreateMSOLockFiles, bEnable );
    SetModified();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
