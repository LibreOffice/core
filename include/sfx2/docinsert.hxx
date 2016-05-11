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

#ifndef INCLUDED_SFX2_DOCINSERT_HXX
#define INCLUDED_SFX2_DOCINSERT_HXX

#include <rtl/ustring.hxx>
#include <sfx2/dllapi.h>
#include <tools/errcode.hxx>
#include <tools/link.hxx>
#include <vector>

namespace sfx2 { class FileDialogHelper; }
class SfxMedium;
class SfxItemSet;
enum class FileDialogFlags;

typedef ::std::vector< SfxMedium* > SfxMediumList;

namespace sfx2 {

class SFX2_DLLPUBLIC DocumentInserter
{
private:
    OUString                m_sDocFactory;
    OUString                m_sFilter;
    Link<sfx2::FileDialogHelper*,void> m_aDialogClosedLink;

    FileDialogFlags const m_nDlgFlags;
    ErrCode                 m_nError;

    sfx2::FileDialogHelper* m_pFileDlg;
    SfxItemSet*             m_pItemSet;
    std::vector<OUString>   m_pURLList;

    DECL_LINK_TYPED(DialogClosedHdl, sfx2::FileDialogHelper*, void);

public:
    DocumentInserter(const OUString& rFactory,
                     bool const bEnableMultiSelection = false);
    ~DocumentInserter();

    void                    StartExecuteModal( const Link<sfx2::FileDialogHelper*,void>& _rDialogClosedLink );
    SfxMedium*              CreateMedium();
    SfxMediumList*          CreateMediumList();
};

} // namespace sfx2

#endif // INCLUDED_SFX2_DOCINSERT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
