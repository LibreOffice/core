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
#ifndef _NBDTMG_HXX
#define _NBDTMG_HXX
#include "svx/svxdllapi.h"
#include <boost/shared_ptr.hpp>
#include <vector>
#include <editeng/numitem.hxx>
#include <vcl/font.hxx>

namespace svx { namespace sidebar {

#define DEFAULT_BULLET_TYPES                            8
#define DEFAULT_NONE                                    10
#define DEFAULT_NUM_TYPE_MEMBER                         5
#define DEFAULT_NUM_VALUSET_COUNT                       8
#define DEFAULT_NUMBERING_CACHE_FORMAT_VERSION          0x10

typedef sal_uInt16 NBOType;
namespace eNBOType
{
    const NBOType BULLETS = 0x01;
    const NBOType GRAPHICBULLETS = 0x02;
    const NBOType NUMBERING = 0x03;
    const NBOType OUTLINE = 0x04;
    const NBOType MIXBULLETS = 0x05;
}

typedef sal_uInt16 NBType;
namespace eNBType
{
    const NBOType BULLETS = 0x01;
    const NBOType GRAPHICBULLETS = 0x02;
}

class  SVX_DLLPUBLIC NumSettings_Impl
{
    public:
        short       nNumberType;
        short       nParentNumbering;
        SvxNumberFormat::LabelFollowedBy eLabelFollowedBy;
        long        nTabValue;
        SvxAdjust   eNumAlign;
        long            nNumAlignAt;
        long            nNumIndentAt;
        rtl::OUString   sPrefix;
        rtl::OUString   sSuffix;
        rtl::OUString   sBulletChar;
        rtl::OUString   sBulletFont;
        SvxBrushItem   *pBrushItem;
        Size            aSize;

    public:
        NumSettings_Impl()
            : nNumberType(0)
            , nParentNumbering(0)
            , pBrushItem(0)
            , aSize(0,0)
        {}
        ~NumSettings_Impl(){}
};

typedef NumSettings_Impl* NumSettings_ImplPtr;
typedef std::vector< boost::shared_ptr<NumSettings_Impl> > NumSettingsArr_Impl;

class  SVX_DLLPUBLIC BulletsSettings
{
    public:
        sal_Bool        bIsCustomized;
        rtl::OUString   sDescription;
        NBType      eType;
    public:
        BulletsSettings(NBType eTy) :
            bIsCustomized(sal_False),
            eType(eTy)
            {}
        virtual ~BulletsSettings(){}
};

class  SVX_DLLPUBLIC BulletsSettings_Impl:public BulletsSettings
{
    public:
        sal_Unicode cBulletChar;
        //rtl::OUString sBulletFont;
        Font            aFont;

    public:
        BulletsSettings_Impl(NBType eTy) :
            BulletsSettings(eTy)
            {}
        virtual ~BulletsSettings_Impl(){}
};

class  SVX_DLLPUBLIC GrfBulDataRelation: public BulletsSettings
{
    public:
        OUString    sGrfName;
        sal_uInt16  nTabIndex;
        sal_uInt16  nGallaryIndex;
        const Graphic*  pGrfObj;
        Size aSize;
    GrfBulDataRelation(NBType eTy):
        BulletsSettings(eTy),
        nTabIndex((sal_uInt16)0xFFFF),
        nGallaryIndex((sal_uInt16)0xFFFF),
        pGrfObj(0),
        aSize(0,0)
    {}
    virtual ~GrfBulDataRelation(){}
};

class  SVX_DLLPUBLIC MixBulletsSettings_Impl
{
    public:
        NBType          eType;
        sal_uInt16          nIndex; //index in the tab page display
        sal_uInt16          nIndexDefault;
        BulletsSettings*    pBullets;
    public:
        MixBulletsSettings_Impl(NBType eTy) :
            eType(eTy),
            nIndex((sal_uInt16)0xFFFF),
            nIndexDefault((sal_uInt16)0xFFFF),
            pBullets(0)
            {}
        ~MixBulletsSettings_Impl(){}
};

class  SVX_DLLPUBLIC NumberSettings_Impl
{
    public:
        sal_Bool        bIsCustomized;
        rtl::OUString   sDescription;
        sal_uInt16      nIndex; //index in the tab page display
        sal_uInt16      nIndexDefault;
        NumSettings_Impl    *pNumSetting;
    public:
        NumberSettings_Impl() :
            bIsCustomized(sal_False),
            nIndex((sal_uInt16)0xFFFF),
            nIndexDefault((sal_uInt16)0xFFFF),
            pNumSetting(NULL)
            {}
        ~NumberSettings_Impl(){}
};

typedef NumberSettings_Impl* NumberSettings_ImplPtr;
typedef std::vector< boost::shared_ptr<NumberSettings_Impl> > NumberSettingsArr_Impl;

class  SVX_DLLPUBLIC OutlineSettings_Impl
{
    public:
        sal_Bool        bIsCustomized;
        rtl::OUString   sDescription;
        NumSettingsArr_Impl *pNumSettingsArr;
    public:
        OutlineSettings_Impl() :
            bIsCustomized(sal_False),
            pNumSettingsArr(NULL)
            {}
        ~OutlineSettings_Impl(){
        }
};

class SVX_DLLPUBLIC NBOTypeMgrBase
{
    public:
        NBOType         eType;
    private:
        const SfxItemSet*   pSet;
        SfxMapUnit      eCoreUnit;
        //Sym3_2508    store the attributes passed from pSet
        OUString        aNumCharFmtName;
        void            StoreBulCharFmtName_impl();
        void            StoreMapUnit_impl();

    public:
        NBOTypeMgrBase(const NBOType aType)
            : eType(aType)
            , pSet(0)
            , eCoreUnit(SFX_MAPUNIT_TWIP)
            , aNumCharFmtName(String())
            , bIsLoading(false)
        {}
        NBOTypeMgrBase(const NBOType aType,const SfxItemSet* pArg)
            : eType(aType)
            , pSet(pArg)
            , eCoreUnit(SFX_MAPUNIT_TWIP)
            , aNumCharFmtName(String())
            , bIsLoading(false)
        {}
        NBOTypeMgrBase(const NBOTypeMgrBase& aTypeMgr)
        {
            eType = aTypeMgr.eType;
            pSet = aTypeMgr.pSet;
            eCoreUnit = aTypeMgr.eCoreUnit;
            aNumCharFmtName = aTypeMgr.aNumCharFmtName;
            bIsLoading = false;
        }
        virtual ~NBOTypeMgrBase() {}
        virtual void Init()=0;
        virtual sal_uInt16 GetNBOIndexForNumRule(SvxNumRule& aNum,sal_uInt16 mLevel,sal_uInt16 nFromIndex=0)=0;
        virtual sal_Bool RelplaceNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF)=0;
        virtual sal_Bool ApplyNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF, sal_Bool isDefault=false,sal_Bool isResetSize=false)=0;
        virtual OUString GetDescription(sal_uInt16 nIndex,sal_Bool isDefault=false)=0;
        virtual sal_Bool IsCustomized(sal_uInt16 nIndex)=0;
        sal_uInt16 IsSingleLevel(sal_uInt16 nCurLevel);
        const SfxItemSet* GetItems() { return pSet;}
        //Sym3_2508    store the attributes passed from pSet
        void SetItems(const SfxItemSet* pArg) { pSet = pArg;StoreBulCharFmtName_impl();StoreMapUnit_impl();}
    protected:
        OUString GetBulCharFmtName();
        SfxMapUnit GetMapUnit();
    protected:
        bool    bIsLoading;
        void    ImplLoad(String filename);
        void    ImplStore(String filename);

};


class SVX_DLLPUBLIC BulletsTypeMgr: public NBOTypeMgrBase
{
    friend class OutlineTypeMgr;
    friend class NumberingTypeMgr;
    public:
        static sal_Unicode aDynamicBulletTypes[DEFAULT_BULLET_TYPES];
        static sal_Unicode aDynamicRTLBulletTypes[DEFAULT_BULLET_TYPES];
        static BulletsSettings_Impl* pActualBullets[DEFAULT_BULLET_TYPES];
    public:
        BulletsTypeMgr();
        BulletsTypeMgr(const BulletsTypeMgr& aTypeMgr);
        virtual ~BulletsTypeMgr() {}
        virtual void Init();
        virtual sal_uInt16 GetNBOIndexForNumRule(SvxNumRule& aNum,sal_uInt16 mLevel,sal_uInt16 nFromIndex=0);
        virtual sal_Bool RelplaceNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF);
        virtual sal_Bool ApplyNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF,sal_Bool isDefault=false,sal_Bool isResetSize=false);
        virtual OUString GetDescription(sal_uInt16 nIndex,sal_Bool isDefault=false);
        virtual sal_Bool IsCustomized(sal_uInt16 nIndex);
        sal_Unicode GetBulChar(sal_uInt16 nIndex);
        Font GetBulCharFont(sal_uInt16 nIndex);
        static BulletsTypeMgr& GetInstance();
};

class SVX_DLLPUBLIC GraphyicBulletsTypeMgr: public NBOTypeMgrBase
{
    friend class OutlineTypeMgr;
    friend class NumberingTypeMgr;
    public:
        typedef std::vector<GrfBulDataRelation*> ListType;
        ListType aGrfDataLst;
    public:
        GraphyicBulletsTypeMgr();
        GraphyicBulletsTypeMgr(const GraphyicBulletsTypeMgr& aTypeMgr);
        virtual ~GraphyicBulletsTypeMgr();
        virtual void Init();
        virtual sal_uInt16 GetNBOIndexForNumRule(SvxNumRule& aNum,sal_uInt16 mLevel,sal_uInt16 nFromIndex=0);
        virtual sal_Bool RelplaceNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF);
        virtual sal_Bool ApplyNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF,sal_Bool isDefault=false,sal_Bool isResetSize=false);
        virtual OUString GetDescription(sal_uInt16 nIndex,sal_Bool isDefault=false);
        virtual sal_Bool IsCustomized(sal_uInt16 nIndex);
        OUString GetGrfName(sal_uInt16 nIndex);
        static GraphyicBulletsTypeMgr& GetInstance();
};

class SVX_DLLPUBLIC MixBulletsTypeMgr: public NBOTypeMgrBase
{
    friend class OutlineTypeMgr;
    friend class NumberingTypeMgr;
    public:
        static MixBulletsSettings_Impl* pActualBullets[DEFAULT_BULLET_TYPES];
        static MixBulletsSettings_Impl* pDefaultActualBullets[DEFAULT_BULLET_TYPES];
    public:
        MixBulletsTypeMgr();
        MixBulletsTypeMgr(const MixBulletsTypeMgr& aTypeMgr);
        virtual ~MixBulletsTypeMgr() {}
        virtual void Init();
        virtual sal_uInt16 GetNBOIndexForNumRule(SvxNumRule& aNum,sal_uInt16 mLevel,sal_uInt16 nFromIndex=0);
        virtual sal_Bool RelplaceNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF);
        virtual sal_Bool ApplyNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF,sal_Bool isDefault=false,sal_Bool isResetSize=false);
        virtual OUString GetDescription(sal_uInt16 nIndex,sal_Bool isDefault=false);
        virtual sal_Bool IsCustomized(sal_uInt16 nIndex);
        static MixBulletsTypeMgr& GetInstance();
};

class SVX_DLLPUBLIC NumberingTypeMgr: public NBOTypeMgrBase
{
    public:
        NumberSettingsArr_Impl*     pNumberSettingsArr;
        NumberSettingsArr_Impl*     pDefaultNumberSettingsArr;
    public:
        NumberingTypeMgr();
        NumberingTypeMgr(const NumberingTypeMgr& aTypeMgr);
        virtual ~NumberingTypeMgr();
        virtual void Init();
        virtual sal_uInt16 GetNBOIndexForNumRule(SvxNumRule& aNum,sal_uInt16 mLevel,sal_uInt16 nFromIndex=0);
        virtual sal_Bool RelplaceNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF);
        virtual sal_Bool ApplyNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF,sal_Bool isDefault=false,sal_Bool isResetSize=false);
        virtual OUString GetDescription(sal_uInt16 nIndex,sal_Bool isDefault=false);
        virtual sal_Bool IsCustomized(sal_uInt16 nIndex);
        static NumberingTypeMgr& GetInstance();
};

class SVX_DLLPUBLIC OutlineTypeMgr: public NBOTypeMgrBase
{
    public:
        OutlineSettings_Impl*       pOutlineSettingsArrs[DEFAULT_NUM_VALUSET_COUNT];
        OutlineSettings_Impl*       pDefaultOutlineSettingsArrs[DEFAULT_NUM_VALUSET_COUNT];
    public:
        OutlineTypeMgr();
        OutlineTypeMgr(const OutlineTypeMgr& aTypeMgr);
        virtual ~OutlineTypeMgr() {}
        virtual void Init();
        virtual sal_uInt16 GetNBOIndexForNumRule(SvxNumRule& aNum,sal_uInt16 mLevel,sal_uInt16 nFromIndex=0);
        virtual sal_Bool RelplaceNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF);
        virtual sal_Bool ApplyNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF,sal_Bool isDefault=false,sal_Bool isResetSize=false);
        virtual OUString GetDescription(sal_uInt16 nIndex,sal_Bool isDefault=false);
        virtual sal_Bool IsCustomized(sal_uInt16 nIndex);
        static OutlineTypeMgr& GetInstance();
};
}}
#endif

