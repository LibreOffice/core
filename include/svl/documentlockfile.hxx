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

#ifndef INCLUDED_SVL_DOCUMENTLOCKFILE_HXX
#define INCLUDED_SVL_DOCUMENTLOCKFILE_HXX

#include <svl/svldllapi.h>

#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>

#include <svl/lockfilecommon.hxx>

namespace svt {

class SVL_DLLPUBLIC DocumentLockFile : public LockFileCommon
{
    // the workaround for automated testing!
    static bool m_bAllowInteraction;

    css::uno::Reference< css::io::XInputStream > OpenStream();

    void WriteEntryToStream( const LockFileEntry& aEntry, css::uno::Reference< css::io::XOutputStream > xStream );

public:
    DocumentLockFile( const OUString& aOrigURL );
    ~DocumentLockFile();

    bool CreateOwnLockFile();
    LockFileEntry GetLockData();
    bool OverwriteOwnLockFile();
    void RemoveFile();

    static bool IsInteractionAllowed() { return m_bAllowInteraction; }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
