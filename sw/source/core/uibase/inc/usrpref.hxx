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
#ifndef INCLUDED_SW_SOURCE_UI_INC_USRPREF_HXX
#define INCLUDED_SW_SOURCE_UI_INC_USRPREF_HXX

#include <unotools/configitem.hxx>
#include <fldupde.hxx>
#include "viewopt.hxx"
#include <tools/fldunit.hxx>

class SwMasterUsrPref;

class SwContentViewConfig : public utl::ConfigItem
{
    SwMasterUsrPref&        rParent;
    sal_Bool                    bWeb;

    com::sun::star::uno::Sequence<OUString> GetPropertyNames();
    public:
        SwContentViewConfig(sal_Bool bWeb, SwMasterUsrPref& rParent);
        virtual ~SwContentViewConfig();

    // utl::ConfigItem
    virtual void    Notify( const com::sun::star::uno::Sequence< OUString > &rPropertyNames ) SAL_OVERRIDE;
    virtual void    Commit() SAL_OVERRIDE;

    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
};

class SwLayoutViewConfig : public utl::ConfigItem
{
    SwMasterUsrPref&    rParent;
    sal_Bool                bWeb;

    com::sun::star::uno::Sequence<OUString> GetPropertyNames();
    public:
        SwLayoutViewConfig(sal_Bool bWeb, SwMasterUsrPref& rParent);
        virtual ~SwLayoutViewConfig();

    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) SAL_OVERRIDE;
    virtual void            Commit() SAL_OVERRIDE;
    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
};

class SwGridConfig : public utl::ConfigItem
{
    SwMasterUsrPref&    rParent;

    com::sun::star::uno::Sequence<OUString> GetPropertyNames();
    public:
        SwGridConfig(sal_Bool bWeb, SwMasterUsrPref& rParent);
        virtual ~SwGridConfig();

    virtual void Commit() SAL_OVERRIDE;
    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) SAL_OVERRIDE;
    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
};

class SwCursorConfig : public utl::ConfigItem
{
    SwMasterUsrPref&    rParent;

    com::sun::star::uno::Sequence<OUString> GetPropertyNames();
    public:
        SwCursorConfig(SwMasterUsrPref& rParent);
        virtual ~SwCursorConfig();

    virtual void Commit() SAL_OVERRIDE;
    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) SAL_OVERRIDE;
    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
};

class SwWebColorConfig : public utl::ConfigItem
{
    SwMasterUsrPref&        rParent;
    com::sun::star::uno::Sequence<OUString> aPropNames;

    public:
        SwWebColorConfig(SwMasterUsrPref& rParent);
        virtual ~SwWebColorConfig();

    virtual void Commit() SAL_OVERRIDE;
    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) SAL_OVERRIDE;
    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
};

class SwMasterUsrPref : public SwViewOption
{
    friend class SwContentViewConfig;
    friend class SwLayoutViewConfig;
    friend class SwGridConfig;
    friend class SwCursorConfig;
    friend class SwWebColorConfig;

    SwFldUpdateFlags eFldUpdateFlags;    //udpate of fields and charts
    sal_Int32   nLinkUpdateMode;
    FieldUnit   eUserMetric;
    FieldUnit   eHScrollMetric;
    sal_Bool    bIsHScrollMetricSet;
    FieldUnit   eVScrollMetric;
    sal_Bool    bIsVScrollMetricSet;

    sal_Int32   nDefTab;            //default tab stop distance

    sal_Bool    bIsSquaredPageMode; //default page mode for text grid
    sal_Bool    bIsAlignMathObjectsToBaseline;

    SwContentViewConfig aContentConfig;
    SwLayoutViewConfig  aLayoutConfig;
    SwGridConfig        aGridConfig;
    SwCursorConfig      aCursorConfig;
    SwWebColorConfig*   pWebColorConfig;

    sal_Bool bApplyCharUnit; // apply_char_unit
public:
    SwMasterUsrPref(sal_Bool bWeb);
    ~SwMasterUsrPref();

    void SetUsrPref(const SwViewOption &rCopy);

    void Commit()
        {
            aContentConfig.Commit();
            aLayoutConfig.Commit();
            aGridConfig.Commit();
            aCursorConfig.Commit();
            if(pWebColorConfig)
                pWebColorConfig->Commit();
        }
    void SetModified()
        {
            aContentConfig.SetModified();
            aLayoutConfig.SetModified();
            aGridConfig.SetModified();
            aCursorConfig.SetModified();
            if(pWebColorConfig)
                pWebColorConfig->SetModified();
        }

    void SetUpdateLinkMode(sal_Int32 nSet, sal_Bool bNoModify = sal_False)
        {
            nLinkUpdateMode = nSet;
            if(!bNoModify)
                aContentConfig.SetModified();
        }
    sal_Int32 GetUpdateLinkMode() const {return nLinkUpdateMode; }

    void SetUpdateFields(sal_Bool bSet, sal_Bool bNoModify = sal_False)
        {
            if(bSet && eFldUpdateFlags == AUTOUPD_OFF)
            {
                eFldUpdateFlags = AUTOUPD_FIELD_ONLY;
                if(!bNoModify)
                    aContentConfig.SetModified();
             }
            else if(!bSet)
            {
                eFldUpdateFlags = AUTOUPD_OFF;
                if(!bNoModify)
                    aContentConfig.SetModified();
            }
        };
    sal_Bool IsUpdateFields()const {return eFldUpdateFlags != AUTOUPD_OFF; }

    SwFldUpdateFlags   GetFldUpdateFlags()const {return eFldUpdateFlags;}
    void        SetFldUpdateFlags(SwFldUpdateFlags eSet, sal_Bool bNoModify = sal_False)
        {
            eFldUpdateFlags = eSet;
            if(!bNoModify)
                aContentConfig.SetModified();
        }

    void SetUpdateCharts(sal_Bool bSet, sal_Bool bNoModify = sal_False)
        {
            if(bSet)
            {
                eFldUpdateFlags = AUTOUPD_FIELD_AND_CHARTS;
                if(!bNoModify)
                    aContentConfig.SetModified();
             }
             else if(eFldUpdateFlags == AUTOUPD_FIELD_AND_CHARTS)
             {
                eFldUpdateFlags = AUTOUPD_FIELD_ONLY;
                if(!bNoModify)
                    aContentConfig.SetModified();
             }
        };
    sal_Bool IsUpdateCharts()const {return eFldUpdateFlags == AUTOUPD_FIELD_AND_CHARTS; }

    FieldUnit   GetMetric() const { return eUserMetric;}
    void        SetMetric(FieldUnit eSet, sal_Bool bNoModify = sal_False)
                {
                    eUserMetric = eSet;
                    if(!bNoModify)
                        aLayoutConfig.SetModified();
                }

    sal_Bool    IsHScrollMetric()const {return bIsHScrollMetricSet;}
    FieldUnit   GetHScrollMetric() const { return bIsHScrollMetricSet ? eHScrollMetric : eUserMetric;}
    void        SetHScrollMetric(FieldUnit eSet, sal_Bool bNoModify = sal_False)
                {
                    eHScrollMetric = eSet; bIsHScrollMetricSet = sal_True;
                    if(!bNoModify)
                        aLayoutConfig.SetModified();
                }

    sal_Bool    IsVScrollMetric()const {return bIsVScrollMetricSet;}
    FieldUnit   GetVScrollMetric() const { return bIsVScrollMetricSet ? eVScrollMetric : eUserMetric;}
    void        SetVScrollMetric(FieldUnit eSet, sal_Bool bNoModify = sal_False)
                {
                    eVScrollMetric = eSet; bIsVScrollMetricSet = sal_True;
                    if(!bNoModify)
                        aLayoutConfig.SetModified();
                }

    sal_Bool    IsApplyCharUnit() const
    {
        return bApplyCharUnit;
    }
    void   SetApplyCharUnit(sal_Bool bSet, sal_Bool bNoModify = sal_False)
    {
        bApplyCharUnit = bSet;
        if(!bNoModify)
            aLayoutConfig.SetModified();
    }

    sal_Int32   GetDefTab() const { return nDefTab;}
    void        SetDefTab( sal_Int32  nSet, sal_Bool bNoModify = sal_False )
                {
                    nDefTab = nSet;
                    if(!bNoModify)
                        aLayoutConfig.SetModified();
                }

    //default page mode for text grid
    sal_Bool    IsSquaredPageMode() const {return bIsSquaredPageMode;}
    void        SetDefaultPageMode( sal_Bool bVal, sal_Bool bNoModify = sal_False )
                {
                    bIsSquaredPageMode = bVal;
                    if(!bNoModify)
                        aLayoutConfig.SetModified();
                }

    sal_Bool    IsAlignMathObjectsToBaseline() const { return bIsAlignMathObjectsToBaseline; }
    void        SetAlignMathObjectsToBaseline( sal_Bool bVal, sal_Bool bNoModify = sal_False )
                {
                    bIsAlignMathObjectsToBaseline = bVal;
                    if(!bNoModify)
                        aLayoutConfig.SetModified();
                }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
