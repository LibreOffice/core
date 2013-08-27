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

#ifndef _SFX_DOCINSERT_HXX
#define _SFX_DOCINSERT_HXX

#include <tools/errcode.hxx>
#include <tools/link.hxx>
#include <tools/string.hxx>
#include <vector>

#include "sfx2/dllapi.h"

namespace sfx2 { class FileDialogHelper; }
class SfxMedium;
class SfxItemSet;

typedef ::std::vector< SfxMedium* > SfxMediumList;

namespace sfx2 {

class SFX2_DLLPUBLIC DocumentInserter
{
private:
    OUString                m_sDocFactory;
    OUString                m_sFilter;
    Link                    m_aDialogClosedLink;

    sal_Int64 const         m_nDlgFlags;
    ErrCode                 m_nError;

    sfx2::FileDialogHelper* m_pFileDlg;
    SfxItemSet*             m_pItemSet;
    std::vector<OUString> m_pURLList;

    DECL_LINK(DialogClosedHdl, void *);

public:
    DocumentInserter(const OUString& rFactory,
                     bool const bEnableMultiSelection = false);
    ~DocumentInserter();

    void                    StartExecuteModal( const Link& _rDialogClosedLink );
    SfxMedium*              CreateMedium();
    SfxMediumList*          CreateMediumList();

    inline ErrCode          GetError() const { return m_nError; }
    inline OUString         GetFilter() const { return m_sFilter; }
};

} // namespace sfx2

#endif // _SFX_DOCINSERT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
