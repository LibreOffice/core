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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_USRPREF_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_USRPREF_HXX

#include <unotools/configitem.hxx>
#include <fldupde.hxx>
#include "viewopt.hxx"
#include <tools/fldunit.hxx>

class SwMasterUsrPref;

class SwContentViewConfig : public utl::ConfigItem
{
private:
    SwMasterUsrPref&        rParent;
    bool                    bWeb;

    com::sun::star::uno::Sequence<OUString> GetPropertyNames();

    virtual void    ImplCommit() override;

public:
    SwContentViewConfig(bool bWeb, SwMasterUsrPref& rParent);
    virtual ~SwContentViewConfig();

    // utl::ConfigItem
    virtual void    Notify( const com::sun::star::uno::Sequence< OUString > &rPropertyNames ) override;

    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
};

class SwLayoutViewConfig : public utl::ConfigItem
{
private:
    SwMasterUsrPref&    rParent;
    bool                bWeb;

    com::sun::star::uno::Sequence<OUString> GetPropertyNames();

    virtual void        ImplCommit() override;

public:
    SwLayoutViewConfig(bool bWeb, SwMasterUsrPref& rParent);
    virtual ~SwLayoutViewConfig();

    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) override;
    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
};

class SwGridConfig : public utl::ConfigItem
{
private:
    SwMasterUsrPref&    rParent;

    static com::sun::star::uno::Sequence<OUString> GetPropertyNames();

    virtual void ImplCommit() override;

public:
    SwGridConfig(bool bWeb, SwMasterUsrPref& rParent);
    virtual ~SwGridConfig();

    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) override;
    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
};

class SwCursorConfig : public utl::ConfigItem
{
private:
    SwMasterUsrPref&    rParent;

    static com::sun::star::uno::Sequence<OUString> GetPropertyNames();

    virtual void ImplCommit() override;

public:
    SwCursorConfig(SwMasterUsrPref& rParent);
    virtual ~SwCursorConfig();

    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) override;
    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
};

class SwWebColorConfig : public utl::ConfigItem
{
private:
    SwMasterUsrPref&        rParent;
    com::sun::star::uno::Sequence<OUString> aPropNames;

    virtual void ImplCommit() override;

public:
    SwWebColorConfig(SwMasterUsrPref& rParent);
    virtual ~SwWebColorConfig();

    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) override;
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

    SwFieldUpdateFlags eFieldUpdateFlags;    //update of fields and charts
    sal_Int32   nLinkUpdateMode;
    FieldUnit   eUserMetric;
    FieldUnit   eHScrollMetric;
    bool    bIsHScrollMetricSet;
    FieldUnit   eVScrollMetric;
    bool    bIsVScrollMetricSet;

    sal_Int32   nDefTab;            //default tab stop distance

    bool    bIsSquaredPageMode; //default page mode for text grid
    bool    bIsAlignMathObjectsToBaseline;

    SwContentViewConfig aContentConfig;
    SwLayoutViewConfig  aLayoutConfig;
    SwGridConfig        aGridConfig;
    SwCursorConfig      aCursorConfig;
    SwWebColorConfig*   pWebColorConfig;

    bool bApplyCharUnit; // apply_char_unit
public:
    SwMasterUsrPref(bool bWeb);
    ~SwMasterUsrPref();

    void SetUsrPref(const SwViewOption &rCopy);

    void SetModified()
        {
            aContentConfig.SetModified();
            aLayoutConfig.SetModified();
            aGridConfig.SetModified();
            aCursorConfig.SetModified();
            if(pWebColorConfig)
                pWebColorConfig->SetModified();
        }

    void SetUpdateLinkMode(sal_Int32 nSet, bool bNoModify = false)
        {
            nLinkUpdateMode = nSet;
            if(!bNoModify)
                aContentConfig.SetModified();
        }
    sal_Int32 GetUpdateLinkMode() const {return nLinkUpdateMode; }

    void SetUpdateFields(bool bSet, bool bNoModify = false)
        {
            if(bSet && eFieldUpdateFlags == AUTOUPD_OFF)
            {
                eFieldUpdateFlags = AUTOUPD_FIELD_ONLY;
                if(!bNoModify)
                    aContentConfig.SetModified();
             }
            else if(!bSet)
            {
                eFieldUpdateFlags = AUTOUPD_OFF;
                if(!bNoModify)
                    aContentConfig.SetModified();
            }
        };
    bool IsUpdateFields()const {return eFieldUpdateFlags != AUTOUPD_OFF; }

    SwFieldUpdateFlags   GetFieldUpdateFlags()const {return eFieldUpdateFlags;}
    void        SetFieldUpdateFlags(SwFieldUpdateFlags eSet, bool bNoModify = false)
        {
            eFieldUpdateFlags = eSet;
            if(!bNoModify)
                aContentConfig.SetModified();
        }

    void SetUpdateCharts(bool bSet, bool bNoModify = false)
        {
            if(bSet)
            {
                eFieldUpdateFlags = AUTOUPD_FIELD_AND_CHARTS;
                if(!bNoModify)
                    aContentConfig.SetModified();
             }
             else if(eFieldUpdateFlags == AUTOUPD_FIELD_AND_CHARTS)
             {
                eFieldUpdateFlags = AUTOUPD_FIELD_ONLY;
                if(!bNoModify)
                    aContentConfig.SetModified();
             }
        };
    bool IsUpdateCharts()const {return eFieldUpdateFlags == AUTOUPD_FIELD_AND_CHARTS; }

    FieldUnit   GetMetric() const { return eUserMetric;}
    void        SetMetric(FieldUnit eSet, bool bNoModify = false)
                {
                    eUserMetric = eSet;
                    if(!bNoModify)
                        aLayoutConfig.SetModified();
                }

    bool    IsHScrollMetric()const {return bIsHScrollMetricSet;}
    FieldUnit   GetHScrollMetric() const { return bIsHScrollMetricSet ? eHScrollMetric : eUserMetric;}
    void        SetHScrollMetric(FieldUnit eSet, bool bNoModify = false)
                {
                    eHScrollMetric = eSet; bIsHScrollMetricSet = true;
                    if(!bNoModify)
                        aLayoutConfig.SetModified();
                }

    bool    IsVScrollMetric()const {return bIsVScrollMetricSet;}
    FieldUnit   GetVScrollMetric() const { return bIsVScrollMetricSet ? eVScrollMetric : eUserMetric;}
    void        SetVScrollMetric(FieldUnit eSet, bool bNoModify = false)
                {
                    eVScrollMetric = eSet; bIsVScrollMetricSet = true;
                    if(!bNoModify)
                        aLayoutConfig.SetModified();
                }

    bool    IsApplyCharUnit() const
    {
        return bApplyCharUnit;
    }
    void   SetApplyCharUnit(bool bSet, bool bNoModify = false)
    {
        bApplyCharUnit = bSet;
        if(!bNoModify)
            aLayoutConfig.SetModified();
    }

    sal_Int32   GetDefTab() const { return nDefTab;}
    void        SetDefTab( sal_Int32  nSet, bool bNoModify = false )
                {
                    nDefTab = nSet;
                    if(!bNoModify)
                        aLayoutConfig.SetModified();
                }

    //default page mode for text grid
    bool    IsSquaredPageMode() const {return bIsSquaredPageMode;}
    void        SetDefaultPageMode( bool bVal, bool bNoModify = false )
                {
                    bIsSquaredPageMode = bVal;
                    if(!bNoModify)
                        aLayoutConfig.SetModified();
                }

    bool    IsAlignMathObjectsToBaseline() const { return bIsAlignMathObjectsToBaseline; }
    void        SetAlignMathObjectsToBaseline( bool bVal, bool bNoModify = false )
                {
                    bIsAlignMathObjectsToBaseline = bVal;
                    if(!bNoModify)
                        aLayoutConfig.SetModified();
                }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
