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

#ifndef INCLUDED_SD_SOURCE_UI_INC_OUTLINEBULLETDLG_HXX
#define INCLUDED_SD_SOURCE_UI_INC_OUTLINEBULLETDLG_HXX

#include <sfx2/tabdlg.hxx>

namespace sd {

class View;

/**
 * Bullet-Tab-Dialog
 */
class OutlineBulletDlg
    : public SfxTabDialog
{
public:
    OutlineBulletDlg (
        vcl::Window* pParent,
        const SfxItemSet* pAttr,
        ::sd::View* pView );
    virtual ~OutlineBulletDlg ();
    virtual void dispose() override;

    const SfxItemSet* GetOutputItemSet() const;

protected:
    virtual void PageCreated( sal_uInt16 nId, SfxTabPage &rPage ) override;

private:
    using SfxTabDialog::GetOutputItemSet;

    SfxItemSet  aInputSet;
    SfxItemSet  *pOutputSet;
    sal_uInt16  m_nOptionsId;
    sal_uInt16  m_nPositionId;
    bool    bTitle;
    ::sd::View  *pSdView;
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
