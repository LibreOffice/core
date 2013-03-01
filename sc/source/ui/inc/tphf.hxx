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

#ifndef SC_TPHF_HXX
#define SC_TPHF_HXX

#include <svx/hdft.hxx>

class ScStyleDlg;

//========================================================================

class ScHFPage : public SvxHFPage
{
public:
    virtual         ~ScHFPage();

    virtual void    Reset( const SfxItemSet& rSet );
    virtual sal_Bool    FillItemSet( SfxItemSet& rOutSet );

    void            SetPageStyle( const String& rName )    { aStrPageStyle = rName; }
    void            SetStyleDlg ( const ScStyleDlg* pDlg ) { pStyleDlg = pDlg; }

protected:
                    ScHFPage( Window* pParent,
                              const SfxItemSet& rSet,
                              sal_uInt16 nSetId );

    virtual void    ActivatePage();
    virtual void    DeactivatePage();
    virtual void    ActivatePage( const SfxItemSet& rSet );
    virtual int     DeactivatePage( SfxItemSet* pSet = 0 );

private:
    PushButton*         m_pBtnEdit;
    SfxItemSet          aDataSet;
    OUString            aStrPageStyle;
    sal_uInt16          nPageUsage;
    const ScStyleDlg*   pStyleDlg;

#ifdef _TPHF_CXX
private:
    DECL_LINK(BtnHdl, void *);
    DECL_LINK( HFEditHdl, void* );
    DECL_LINK(TurnOnHdl, void *);
#endif
};

//========================================================================

class ScHeaderPage : public ScHFPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

private:
    ScHeaderPage( Window* pParent, const SfxItemSet& rSet );
};

//========================================================================

class ScFooterPage : public ScHFPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

private:
    ScFooterPage( Window* pParent, const SfxItemSet& rSet );
};


#endif // SC_TPHF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
