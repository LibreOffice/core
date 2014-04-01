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
#ifndef INCLUDED_EDITENG_ACORRCFG_HXX
#define INCLUDED_EDITENG_ACORRCFG_HXX

#include <editeng/editengdllapi.h>
#include <unotools/configitem.hxx>

class SvxAutoCorrect;
class SvxAutoCorrCfg;
class EDITENG_DLLPUBLIC SvxBaseAutoCorrCfg : public utl::ConfigItem
{
    SvxAutoCorrCfg& rParent;
    com::sun::star::uno::Sequence<OUString>    GetPropertyNames();

public:
    SvxBaseAutoCorrCfg(SvxAutoCorrCfg& rParent);
    virtual ~SvxBaseAutoCorrCfg();

    void                    Load(bool bInit);
    virtual void            Commit() SAL_OVERRIDE;
    virtual void            Notify( const com::sun::star::uno::Sequence<OUString>& aPropertyNames) SAL_OVERRIDE;
    void                    SetModified() {ConfigItem::SetModified();}
};

class EDITENG_DLLPUBLIC SvxSwAutoCorrCfg : public utl::ConfigItem
{
    SvxAutoCorrCfg& rParent;
    com::sun::star::uno::Sequence<OUString>    GetPropertyNames();

public:
    SvxSwAutoCorrCfg(SvxAutoCorrCfg& rParent);
    virtual ~SvxSwAutoCorrCfg();

    void                    Load(bool bInit);
    virtual void            Commit() SAL_OVERRIDE;
    virtual void            Notify( const com::sun::star::uno::Sequence<OUString>& aPropertyNames) SAL_OVERRIDE;
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
    bool    bFileRel;
    bool    bNetRel;
    // Help tip for Autotext as you type
    bool    bAutoTextTip;
    bool    bAutoTextPreview;
    bool    bAutoFmtByInput;
    bool    bSearchInAllCategories;

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
    // the pointer is transferred to the possession of the ConfigItems!
    void SetAutoCorrect( SvxAutoCorrect* );

    bool IsAutoFmtByInput() const       { return bAutoFmtByInput; }
    void SetAutoFmtByInput( bool bSet ) { bAutoFmtByInput = bSet;aSwConfig.SetModified();}

    bool IsSaveRelFile() const          { return bFileRel; }
    void SetSaveRelFile( bool bSet )    { bFileRel = bSet; aSwConfig.SetModified(); }

    bool IsSaveRelNet() const           { return bNetRel; }
    void SetSaveRelNet( bool bSet )     { bNetRel = bSet; aSwConfig.SetModified();}

    bool IsAutoTextPreview() const {return bAutoTextPreview;}
    void SetAutoTextPreview(bool bSet) {bAutoTextPreview = bSet; aSwConfig.SetModified();}

    bool IsAutoTextTip() const          { return bAutoTextTip; }
    void SetAutoTextTip(bool bSet )     { bAutoTextTip = bSet;aSwConfig.SetModified();}

    bool IsSearchInAllCategories() const        { return bSearchInAllCategories;}
    void SetSearchInAllCategories(bool bSet )   { bSearchInAllCategories = bSet; aSwConfig.SetModified(); }

    SvxAutoCorrCfg();
    virtual ~SvxAutoCorrCfg();
    static SvxAutoCorrCfg& Get();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
