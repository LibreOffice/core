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
#ifndef _SVXACCFG_HXX
#define _SVXACCFG_HXX

#include "editeng/editengdllapi.h"
#include <unotools/configitem.hxx>

class SvxAutoCorrect;
class SvxAutoCorrCfg;
class EDITENG_DLLPUBLIC SvxBaseAutoCorrCfg : public utl::ConfigItem
{
    SvxAutoCorrCfg& rParent;
    com::sun::star::uno::Sequence<rtl::OUString>    GetPropertyNames();

public:
    SvxBaseAutoCorrCfg(SvxAutoCorrCfg& rParent);
    ~SvxBaseAutoCorrCfg();

    void                    Load(sal_Bool bInit);
    virtual void            Commit();
    virtual void            Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);
    void                    SetModified() {ConfigItem::SetModified();}
};

class EDITENG_DLLPUBLIC SvxSwAutoCorrCfg : public utl::ConfigItem
{
    SvxAutoCorrCfg& rParent;
    com::sun::star::uno::Sequence<rtl::OUString>    GetPropertyNames();

public:
    SvxSwAutoCorrCfg(SvxAutoCorrCfg& rParent);
    ~SvxSwAutoCorrCfg();

    void                    Load(sal_Bool bInit);
    virtual void            Commit();
    virtual void            Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);
    void                    SetModified() {ConfigItem::SetModified();}
};
/*--------------------------------------------------------------------
    Description:   Configuration for Auto Correction
 --------------------------------------------------------------------*/
class EDITENG_DLLPUBLIC SvxAutoCorrCfg
{
    friend class SvxBaseAutoCorrCfg;
    friend class SvxSwAutoCorrCfg;

    SvxAutoCorrect* pAutoCorrect;

    SvxBaseAutoCorrCfg      aBaseConfig;
    SvxSwAutoCorrCfg        aSwConfig;

    // Flags for Autotext:
    sal_Bool    bFileRel;
    sal_Bool    bNetRel;
    // Help tip for Autotext as you type
    sal_Bool    bAutoTextTip;
    sal_Bool    bAutoTextPreview;
    sal_Bool    bAutoFmtByInput;
    sal_Bool    bSearchInAllCategories;

public:
    void        SetModified()
                {
                    aBaseConfig.SetModified();
                    aSwConfig.SetModified();
                }
    void        Commit()
                {
                    aBaseConfig.Commit();
                    aSwConfig.Commit();
                }

          SvxAutoCorrect* GetAutoCorrect()          { return pAutoCorrect; }
    const SvxAutoCorrect* GetAutoCorrect() const    { return pAutoCorrect; }
    // the pointer is transfered to the possession of the ConfigItems!
    void SetAutoCorrect( SvxAutoCorrect* );

    sal_Bool IsAutoFmtByInput() const       { return bAutoFmtByInput; }
    void SetAutoFmtByInput( sal_Bool bSet ) { bAutoFmtByInput = bSet;aSwConfig.SetModified();}

    sal_Bool IsSaveRelFile() const          { return bFileRel; }
    void SetSaveRelFile( sal_Bool bSet )    { bFileRel = bSet; aSwConfig.SetModified(); }

    sal_Bool IsSaveRelNet() const           { return bNetRel; }
    void SetSaveRelNet( sal_Bool bSet )     { bNetRel = bSet; aSwConfig.SetModified();}

    sal_Bool IsAutoTextPreview() const {return bAutoTextPreview;}
    void     SetAutoTextPreview(sal_Bool bSet) {bAutoTextPreview = bSet; aSwConfig.SetModified();}

    sal_Bool IsAutoTextTip() const          { return bAutoTextTip; }
    void SetAutoTextTip(sal_Bool bSet )     { bAutoTextTip = bSet;aSwConfig.SetModified();}

    sal_Bool IsSearchInAllCategories() const        { return bSearchInAllCategories;}
    void SetSearchInAllCategories(sal_Bool bSet )   { bSearchInAllCategories = bSet; aSwConfig.SetModified(); }

    SvxAutoCorrCfg();
    virtual ~SvxAutoCorrCfg();
    static SvxAutoCorrCfg& Get();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
