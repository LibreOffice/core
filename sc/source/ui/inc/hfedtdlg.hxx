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

#ifndef SC_HFEDTDLG_HXX
#define SC_HFEDTDLG_HXX

#include <sfx2/tabdlg.hxx>
#include <svx/pageitem.hxx>


#include "sc.hrc"




class ScHFEditDlg : public SfxTabDialog
{
    SvxNumType eNumType;
protected:
    ScHFEditDlg(SfxViewFrame* pFrame, Window* pParent,
        const SfxItemSet& rCoreSet, const OUString& rPageStyle,
        const OString& rID, const OUString& rUIXMLDescription );
public:
    virtual void PageCreated( sal_uInt16 nId, SfxTabPage& rPage );
};

class ScHFEditHeaderDlg : public ScHFEditDlg
{
public:
    ScHFEditHeaderDlg(SfxViewFrame* pFrame, Window* pParent,
        const SfxItemSet& rCoreSet, const OUString& rPageStyle);
};

class ScHFEditFooterDlg : public ScHFEditDlg
{
public:
    ScHFEditFooterDlg(SfxViewFrame* pFrame, Window* pParent,
        const SfxItemSet& rCoreSet, const OUString& rPageStyle);
};

class ScHFEditLeftHeaderDlg : public ScHFEditDlg
{
public:
    ScHFEditLeftHeaderDlg(SfxViewFrame* pFrame, Window* pParent,
        const SfxItemSet& rCoreSet, const OUString& rPageStyle);
};

class ScHFEditRightHeaderDlg : public ScHFEditDlg
{
public:
    ScHFEditRightHeaderDlg(SfxViewFrame* pFrame, Window* pParent,
        const SfxItemSet& rCoreSet, const OUString& rPageStyle);
};

class ScHFEditLeftFooterDlg : public ScHFEditDlg
{
public:
    ScHFEditLeftFooterDlg(SfxViewFrame* pFrame, Window* pParent,
        const SfxItemSet& rCoreSet, const OUString& rPageStyle);
};

class ScHFEditRightFooterDlg : public ScHFEditDlg
{
public:
    ScHFEditRightFooterDlg(SfxViewFrame* pFrame, Window* pParent,
        const SfxItemSet& rCoreSet, const OUString& rPageStyle);
};

class ScHFEditSharedHeaderDlg : public ScHFEditDlg
{
public:
    ScHFEditSharedHeaderDlg(SfxViewFrame* pFrame, Window* pParent,
        const SfxItemSet& rCoreSet, const OUString& rPageStyle);
};

class ScHFEditSharedFooterDlg : public ScHFEditDlg
{
public:
    ScHFEditSharedFooterDlg(SfxViewFrame* pFrame, Window* pParent,
        const SfxItemSet& rCoreSet, const OUString& rPageStyle);
};

class ScHFEditAllDlg : public ScHFEditDlg
{
public:
    ScHFEditAllDlg(SfxViewFrame* pFrame, Window* pParent,
        const SfxItemSet& rCoreSet, const OUString& rPageStyle);
};

class ScHFEditActiveDlg : public ScHFEditDlg
{
public:
    ScHFEditActiveDlg(SfxViewFrame* pFrame, Window* pParent,
        const SfxItemSet& rCoreSet, const OUString& rPageStyle);
};

#endif // SC_HFEDTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
