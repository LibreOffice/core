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

#include <memory>
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

    css::uno::Sequence<OUString> GetPropertyNames();

    virtual void    ImplCommit() override;

public:
    SwContentViewConfig(bool bWeb, SwMasterUsrPref& rParent);
    virtual ~SwContentViewConfig() override;

    // utl::ConfigItem
    virtual void    Notify( const css::uno::Sequence< OUString > &rPropertyNames ) override;

    void                    Load();
    using ConfigItem::SetModified;
};

class SwLayoutViewConfig : public utl::ConfigItem
{
private:
    SwMasterUsrPref&    rParent;
    bool                bWeb;

    css::uno::Sequence<OUString> GetPropertyNames();

    virtual void        ImplCommit() override;

public:
    SwLayoutViewConfig(bool bWeb, SwMasterUsrPref& rParent);
    virtual ~SwLayoutViewConfig() override;

    virtual void Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    void                    Load();
    using ConfigItem::SetModified;
};

class SwGridConfig : public utl::ConfigItem
{
private:
    SwMasterUsrPref&    rParent;

    static css::uno::Sequence<OUString> GetPropertyNames();

    virtual void ImplCommit() override;

public:
    SwGridConfig(bool bWeb, SwMasterUsrPref& rParent);
    virtual ~SwGridConfig() override;

    virtual void Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    void                    Load();
    using ConfigItem::SetModified;
};

class SwCursorConfig : public utl::ConfigItem
{
private:
    SwMasterUsrPref&    rParent;

    static css::uno::Sequence<OUString> GetPropertyNames();

    virtual void ImplCommit() override;

public:
    SwCursorConfig(SwMasterUsrPref& rParent);
    virtual ~SwCursorConfig() override;

    virtual void Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    void                    Load();
    using ConfigItem::SetModified;
};

class SwWebColorConfig : public utl::ConfigItem
{
private:
    SwMasterUsrPref&        rParent;
    css::uno::Sequence<OUString> aPropNames;

    virtual void ImplCommit() override;

public:
    SwWebColorConfig(SwMasterUsrPref& rParent);
    virtual ~SwWebColorConfig() override;

    virtual void Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    void                    Load();
    using ConfigItem::SetModified;
};

class SwMasterUsrPref : public SwViewOption
{
    friend class SwContentViewConfig;
    friend class SwLayoutViewConfig;
    friend class SwGridConfig;
    friend class SwCursorConfig;
    friend class SwWebColorConfig;

    SwFieldUpdateFlags m_eFieldUpdateFlags;    //update of fields and charts
    sal_Int32   m_nLinkUpdateMode;
    FieldUnit   m_eUserMetric;
    FieldUnit   m_eHScrollMetric;
    bool    m_bIsHScrollMetricSet;
    FieldUnit   m_eVScrollMetric;
    bool    m_bIsVScrollMetricSet;

    sal_Int32   m_nDefTab;            //default tab stop distance

    bool    m_bIsSquaredPageMode; //default page mode for text grid
    bool    m_bIsAlignMathObjectsToBaseline;

    SwContentViewConfig m_aContentConfig;
    SwLayoutViewConfig  m_aLayoutConfig;
    SwGridConfig        m_aGridConfig;
    SwCursorConfig      m_aCursorConfig;
    std::unique_ptr<SwWebColorConfig>   m_pWebColorConfig;

    bool m_bApplyCharUnit; // apply_char_unit
public:
    SwMasterUsrPref(bool bWeb);
    ~SwMasterUsrPref();

    void SetUsrPref(const SwViewOption &rCopy);

    void SetModified()
        {
            m_aContentConfig.SetModified();
            m_aLayoutConfig.SetModified();
            m_aGridConfig.SetModified();
            m_aCursorConfig.SetModified();
            if(m_pWebColorConfig)
                m_pWebColorConfig->SetModified();
        }

    void SetUpdateLinkMode(sal_Int32 nSet, bool bNoModify = false)
        {
            m_nLinkUpdateMode = nSet;
            if(!bNoModify)
                m_aContentConfig.SetModified();
        }
    sal_Int32 GetUpdateLinkMode() const {return m_nLinkUpdateMode; }

    void SetUpdateFields(bool bSet)
        {
            if(bSet && m_eFieldUpdateFlags == AUTOUPD_OFF)
            {
                m_eFieldUpdateFlags = AUTOUPD_FIELD_ONLY;
             }
            else if(!bSet)
            {
                m_eFieldUpdateFlags = AUTOUPD_OFF;
            }
        };
    bool IsUpdateFields()const {return m_eFieldUpdateFlags != AUTOUPD_OFF; }

    SwFieldUpdateFlags   GetFieldUpdateFlags()const {return m_eFieldUpdateFlags;}
    void        SetFieldUpdateFlags(SwFieldUpdateFlags eSet)
        {
            m_eFieldUpdateFlags = eSet;
            m_aContentConfig.SetModified();
        }

    void SetUpdateCharts(bool bSet)
        {
            if(bSet)
            {
                m_eFieldUpdateFlags = AUTOUPD_FIELD_AND_CHARTS;
             }
             else if(m_eFieldUpdateFlags == AUTOUPD_FIELD_AND_CHARTS)
             {
                m_eFieldUpdateFlags = AUTOUPD_FIELD_ONLY;
             }
        };
    bool IsUpdateCharts()const {return m_eFieldUpdateFlags == AUTOUPD_FIELD_AND_CHARTS; }

    FieldUnit   GetMetric() const { return m_eUserMetric;}
    void        SetMetric(FieldUnit eSet, bool bNoModify = false)
                {
                    m_eUserMetric = eSet;
                    if(!bNoModify)
                        m_aLayoutConfig.SetModified();
                }

    bool        IsHScrollMetric()const {return m_bIsHScrollMetricSet;}
    FieldUnit   GetHScrollMetric() const { return m_bIsHScrollMetricSet ? m_eHScrollMetric : m_eUserMetric;}
    void        SetHScrollMetric(FieldUnit eSet)
                {
                    m_eHScrollMetric = eSet; m_bIsHScrollMetricSet = true;
                    m_aLayoutConfig.SetModified();
                }

    bool        IsVScrollMetric()const {return m_bIsVScrollMetricSet;}
    FieldUnit   GetVScrollMetric() const { return m_bIsVScrollMetricSet ? m_eVScrollMetric : m_eUserMetric;}
    void        SetVScrollMetric(FieldUnit eSet)
                {
                    m_eVScrollMetric = eSet; m_bIsVScrollMetricSet = true;
                    m_aLayoutConfig.SetModified();
                }

    bool    IsApplyCharUnit() const
    {
        return m_bApplyCharUnit;
    }
    void   SetApplyCharUnit(bool bSet)
    {
        m_bApplyCharUnit = bSet;
        m_aLayoutConfig.SetModified();
    }

    sal_Int32   GetDefTab() const { return m_nDefTab;}
    void        SetDefTab( sal_Int32  nSet, bool bNoModify = false )
                {
                    m_nDefTab = nSet;
                    if(!bNoModify)
                        m_aLayoutConfig.SetModified();
                }

    //default page mode for text grid
    bool        IsSquaredPageMode() const {return m_bIsSquaredPageMode;}
    void        SetDefaultPageMode( bool bVal, bool bNoModify = false )
                {
                    m_bIsSquaredPageMode = bVal;
                    if(!bNoModify)
                        m_aLayoutConfig.SetModified();
                }

    bool        IsAlignMathObjectsToBaseline() const { return m_bIsAlignMathObjectsToBaseline; }
    void        SetAlignMathObjectsToBaseline( bool bVal )
                {
                    m_bIsAlignMathObjectsToBaseline = bVal;
                    m_aLayoutConfig.SetModified();
                }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
