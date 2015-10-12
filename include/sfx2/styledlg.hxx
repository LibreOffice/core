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
#ifndef INCLUDED_SFX2_STYLEDLG_HXX
#define INCLUDED_SFX2_STYLEDLG_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>

#include <sfx2/tabdlg.hxx>

class SfxStyleSheetBase;
class SfxStyleSheetBasePool;

class SFX2_DLLPUBLIC SfxStyleDialog: public SfxTabDialog
{
private:
    SfxStyleSheetBase*          pStyle;
    DECL_DLLPRIVATE_LINK_TYPED( CancelHdl, Button *, void );
    sal_uInt16 m_nOrganizerId;
protected:
    virtual void   RefreshInputSet() override;

public:
    SfxStyleDialog(vcl::Window* pParent, const OUString& rID,
        const OUString& rUIXMLDescription, SfxStyleSheetBase&);

    virtual ~SfxStyleDialog();
    virtual void dispose() override;

    SfxStyleSheetBase&          GetStyleSheet() { return *pStyle; }
    const SfxStyleSheetBase&    GetStyleSheet() const { return *pStyle; }

    virtual short               Ok() override;

    static OUString GenerateUnusedName(SfxStyleSheetBasePool &rPool);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
