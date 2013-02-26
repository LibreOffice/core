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
#ifndef _TOOLS_INTERNAL_HXX
#define _TOOLS_INTERNAL_HXX

// This header contains classes we're trying to kill and use only
// internally inside tools

#include <tools/fsys.hxx>

struct FileCopier_Impl;

// FSysExact
enum FSysExact
{
    FSYS_NOTEXACT,
    FSYS_EXACT
};

// FileCopier (a private impl. detail of tools/)

class FileCopier
{
    DirEntry            aSource;
    DirEntry            aTarget;
    sal_uIntPtr         nBytesTotal;
    sal_uIntPtr         nBytesCopied;
    Link                aProgressLink;
    sal_uInt16          nBlockSize;
    FileCopier_Impl*    pImp;

private:
    TOOLS_DLLPRIVATE FSysError DoCopy_Impl(
        const DirEntry &rSource, const DirEntry &rTarget );

protected:
    virtual sal_Bool    Progress();
    virtual ErrCode     Error( ErrCode eErr,
                               const DirEntry *pSource, const DirEntry *pTarget );

public:
                        FileCopier( const DirEntry &rSource,
                                    const DirEntry &rTarget );
                        FileCopier( const FileCopier &rCopier );
                        virtual ~FileCopier();

    FileCopier&         operator =( const FileCopier &rCopier );

    void                SetBlockSize( sal_uInt16 nBytes ) { nBlockSize = nBytes; }
    sal_uInt16          GetBlockSize() const { return nBlockSize; }

    sal_uIntPtr         GetBytesTotal() const { return nBytesTotal; }
    sal_uIntPtr         GetBytesCopied() const { return nBytesCopied; }

    void                SetSource( const DirEntry &rSource ) { aSource = rSource; }
    void                SetTarget( const DirEntry &rTarget ) { aTarget = rTarget; }
    const DirEntry&     GetSource() const { return aSource; }
    const DirEntry&     GetTarget() const { return aTarget; }

    FSysError           Execute( FSysAction nActions = FSYS_ACTION_STANDARD );
    FSysError           ExecuteExact( FSysAction nActions = FSYS_ACTION_STANDARD,
                                      FSysExact  eExact = FSYS_NOTEXACT);

    void                SetProgressHdl( const Link& rLink ) { aProgressLink = rLink; }
    const Link&         GetProgressHdl() const { return aProgressLink; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
