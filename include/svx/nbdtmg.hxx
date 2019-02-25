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
#ifndef INCLUDED_SVX_NBDTMG_HXX
#define INCLUDED_SVX_NBDTMG_HXX
#include <svx/svxdllapi.h>
#include <memory>
#include <vector>
#include <editeng/numitem.hxx>
#include <vcl/font.hxx>

namespace svx { namespace sidebar {

#define DEFAULT_BULLET_TYPES                            8
#define DEFAULT_NUM_VALUSET_COUNT                       8
#define DEFAULT_NUMBERING_CACHE_FORMAT_VERSION          0x10

enum class NBOType
{
    Bullets = 1, Numbering, Outline
};

class  SVX_DLLPUBLIC NumSettings_Impl
{
    public:
        SvxNumType      nNumberType;
        short           nParentNumbering;
        SvxNumberFormat::LabelFollowedBy eLabelFollowedBy;
        long            nTabValue;
        SvxAdjust       eNumAlign;
        long            nNumAlignAt;
        long            nNumIndentAt;
        OUString   sPrefix;
        OUString   sSuffix;
        OUString   sBulletChar;
        OUString   sBulletFont;
        SvxBrushItem   *pBrushItem;
        Size            aSize;

    public:
        NumSettings_Impl()
            : nNumberType(SVX_NUM_CHARS_UPPER_LETTER)
            , nParentNumbering(0)
            , eLabelFollowedBy(SvxNumberFormat::NOTHING)
            , nTabValue (0)
            , eNumAlign(SvxAdjust::Left)
            , nNumAlignAt(0)
            , nNumIndentAt(0)
            , pBrushItem(nullptr)
            , aSize(0,0)
        {}
};

typedef std::vector< std::shared_ptr<NumSettings_Impl> > NumSettingsArr_Impl;

class  SVX_DLLPUBLIC BulletsSettings
{
public:
    bool            bIsCustomized;
    OUString        sDescription;
    sal_Unicode     cBulletChar;
    vcl::Font       aFont;
    BulletsSettings() : bIsCustomized(false), cBulletChar(0) {}
};


class  SVX_DLLPUBLIC NumberSettings_Impl
{
    public:
        bool              bIsCustomized;
        OUString          sDescription;
        NumSettings_Impl* pNumSetting;
    public:
        NumberSettings_Impl() :
            bIsCustomized(false),
            pNumSetting(nullptr)
            {}
};

typedef std::vector< std::shared_ptr<NumberSettings_Impl> > NumberSettingsArr_Impl;

class  SVX_DLLPUBLIC OutlineSettings_Impl
{
    public:
        bool            bIsCustomized;
        OUString   sDescription;
        NumSettingsArr_Impl *pNumSettingsArr;
    public:
        OutlineSettings_Impl() :
            bIsCustomized(false),
            pNumSettingsArr(nullptr)
            {}
};

class SVX_DLLPUBLIC NBOTypeMgrBase
{
    private:
        const SfxItemSet*   pSet;
        MapUnit         eCoreUnit;
        // store the attributes passed from pSet
        OUString        aBulletCharFmtName;
        OUString        aNumCharFmtName;
        bool    bIsLoading;

        NBOTypeMgrBase(const NBOTypeMgrBase&) = delete;

    public:
        NBOTypeMgrBase()
            : pSet(nullptr)
            , eCoreUnit(MapUnit::MapTwip)
            , bIsLoading(false)
        {}
        virtual ~NBOTypeMgrBase() {}
        virtual void Init()=0;
        virtual sal_uInt16 GetNBOIndexForNumRule(SvxNumRule& aNum,sal_uInt16 mLevel,sal_uInt16 nFromIndex=0) = 0;
        virtual void RelplaceNumRule(SvxNumRule& aNum, sal_uInt16 nIndex, sal_uInt16 mLevel) = 0;
        virtual void ApplyNumRule(SvxNumRule& aNum, sal_uInt16 nIndex, sal_uInt16 mLevel, bool isDefault=false,bool isResetSize=false) = 0;
        virtual OUString GetDescription(sal_uInt16 nIndex, bool isDefault)=0;
        virtual bool IsCustomized(sal_uInt16 nIndex)=0;
        static sal_uInt16 IsSingleLevel(sal_uInt16 nCurLevel);
        // store the attributes passed from pSet
        void SetItems(const SfxItemSet* pArg);
    protected:
        const OUString& GetBulletCharFmtName() { return aBulletCharFmtName;}
        const OUString& GetNumCharFmtName() { return aNumCharFmtName;}
        MapUnit GetMapUnit() { return eCoreUnit;}
    protected:
        void    ImplLoad(const OUString& filename);
        void    ImplStore(const OUString& filename);

};


class SVX_DLLPUBLIC BulletsTypeMgr: public NBOTypeMgrBase
{
    friend class OutlineTypeMgr;
    friend class NumberingTypeMgr;
    private:
        BulletsTypeMgr(const BulletsTypeMgr&) = delete;
    public:
        static sal_Unicode aDynamicBulletTypes[DEFAULT_BULLET_TYPES];
        static sal_Unicode aDynamicRTLBulletTypes[DEFAULT_BULLET_TYPES];
        static BulletsSettings* pActualBullets[DEFAULT_BULLET_TYPES];
    public:
        BulletsTypeMgr();
        virtual void Init() override;
        virtual sal_uInt16 GetNBOIndexForNumRule(SvxNumRule& aNum,sal_uInt16 mLevel,sal_uInt16 nFromIndex=0) override;
        virtual void RelplaceNumRule(SvxNumRule& aNum, sal_uInt16 nIndex, sal_uInt16 mLevel) override;
        virtual void ApplyNumRule(SvxNumRule& aNum, sal_uInt16 nIndex, sal_uInt16 mLevel, bool isDefault=false,bool isResetSize=false) override;
        virtual OUString GetDescription(sal_uInt16 nIndex, bool isDefault) override;
        virtual bool IsCustomized(sal_uInt16 nIndex) override;
        static BulletsTypeMgr& GetInstance();
};


class SVX_DLLPUBLIC NumberingTypeMgr: public NBOTypeMgrBase
{
    private:
        NumberingTypeMgr(const NumberingTypeMgr&) = delete;
    public:
        NumberSettingsArr_Impl      maNumberSettingsArr;
        NumberSettingsArr_Impl      maDefaultNumberSettingsArr;
    public:
        NumberingTypeMgr();
        virtual ~NumberingTypeMgr() override;
        virtual void Init() override;
        virtual sal_uInt16 GetNBOIndexForNumRule(SvxNumRule& aNum,sal_uInt16 mLevel,sal_uInt16 nFromIndex=0) override;
        virtual void RelplaceNumRule(SvxNumRule& aNum, sal_uInt16 nIndex, sal_uInt16 mLevel) override;
        virtual void ApplyNumRule(SvxNumRule& aNum, sal_uInt16 nIndex, sal_uInt16 mLevel, bool isDefault=false,bool isResetSize=false) override;
        virtual OUString GetDescription(sal_uInt16 nIndex, bool isDefault) override;
        virtual bool IsCustomized(sal_uInt16 nIndex) override;
        static NumberingTypeMgr& GetInstance();
};

class SVX_DLLPUBLIC OutlineTypeMgr: public NBOTypeMgrBase
{
    private:
        OutlineTypeMgr(const OutlineTypeMgr&) = delete;
    public:
        OutlineSettings_Impl*       pOutlineSettingsArrs[DEFAULT_NUM_VALUSET_COUNT];
        OutlineSettings_Impl*       pDefaultOutlineSettingsArrs[DEFAULT_NUM_VALUSET_COUNT];
    public:
        OutlineTypeMgr();
        virtual void Init() override;
        virtual sal_uInt16 GetNBOIndexForNumRule(SvxNumRule& aNum,sal_uInt16 mLevel,sal_uInt16 nFromIndex=0) override;
        virtual void RelplaceNumRule(SvxNumRule& aNum, sal_uInt16 nIndex, sal_uInt16 mLevel) override;
        virtual void ApplyNumRule(SvxNumRule& aNum, sal_uInt16 nIndex, sal_uInt16 mLevel, bool isDefault=false,bool isResetSize=false) override;
        virtual OUString GetDescription(sal_uInt16 nIndex, bool isDefault) override;
        virtual bool IsCustomized(sal_uInt16 nIndex) override;
        static OutlineTypeMgr& GetInstance();
};
}}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
