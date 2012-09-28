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
#ifndef _USRPREF_HXX
#define _USRPREF_HXX



#include <unotools/configitem.hxx>
#include <fldupde.hxx>
#include "viewopt.hxx"
#include <tools/fldunit.hxx>

class SwMasterUsrPref;

class SwContentViewConfig : public utl::ConfigItem
{
    SwMasterUsrPref&        rParent;
    sal_Bool                    bWeb;

    com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();
    public:
        SwContentViewConfig(sal_Bool bWeb, SwMasterUsrPref& rParent);
        ~SwContentViewConfig();

    // utl::ConfigItem
    virtual void    Notify( const com::sun::star::uno::Sequence< rtl::OUString > &rPropertyNames );
    virtual void    Commit();

    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
};

class SwLayoutViewConfig : public utl::ConfigItem
{
    SwMasterUsrPref&    rParent;
    sal_Bool                bWeb;

    com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();
    public:
        SwLayoutViewConfig(sal_Bool bWeb, SwMasterUsrPref& rParent);
        ~SwLayoutViewConfig();

    virtual void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    virtual void            Commit();
    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
};

class SwGridConfig : public utl::ConfigItem
{
    SwMasterUsrPref&    rParent;

    com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();
    public:
        SwGridConfig(sal_Bool bWeb, SwMasterUsrPref& rParent);
        ~SwGridConfig();

    virtual void Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
};

class SwCursorConfig : public utl::ConfigItem
{
    SwMasterUsrPref&    rParent;

    com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();
    public:
        SwCursorConfig(SwMasterUsrPref& rParent);
        ~SwCursorConfig();

    virtual void Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
};

class SwWebColorConfig : public utl::ConfigItem
{
    SwMasterUsrPref&        rParent;
    com::sun::star::uno::Sequence<rtl::OUString> aPropNames;

    public:
        SwWebColorConfig(SwMasterUsrPref& rParent);
        ~SwWebColorConfig();

    virtual void Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
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
