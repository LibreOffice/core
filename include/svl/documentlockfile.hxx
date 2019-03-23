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

#include <svl/lockfilecommon.hxx>

namespace com { namespace sun { namespace star { namespace io { class XInputStream; } } } }
namespace com { namespace sun { namespace star { namespace io { class XOutputStream; } } } }

namespace svt {

/// Generalized class for LO and MSO lockfile handling.
class SVL_DLLPUBLIC GenDocumentLockFile : public LockFileCommon
{
public:
    /// Specify the lockfile URL directly
    GenDocumentLockFile( const OUString& aURL );
    /// Let the object generate and own URL based on the original file's URL and a prefix
    GenDocumentLockFile( const OUString& aOrigURL, const OUString& aPrefix );
    virtual ~GenDocumentLockFile() override;

    bool CreateOwnLockFile();
    bool OverwriteOwnLockFile();
    /// Delete the Lockfile, if current user is the owner
    virtual void RemoveFile();
    /// Only delete lockfile, disregarding ownership
    void RemoveFileDirectly();

    virtual LockFileEntry GetLockData() = 0;

protected:
    virtual void WriteEntryToStream( const LockFileEntry& aEntry, const css::uno::Reference< css::io::XOutputStream >& xStream ) = 0;
    css::uno::Reference< css::io::XInputStream > OpenStream();
};

/// Class implementing reading and writing LO lockfiles.
class SVL_DLLPUBLIC DocumentLockFile : public GenDocumentLockFile
{
protected:
    virtual void WriteEntryToStream( const LockFileEntry& aEntry, const css::uno::Reference< css::io::XOutputStream >& xStream ) override;

public:
    DocumentLockFile( const OUString& aOrigURL );
    virtual ~DocumentLockFile() override;

    virtual LockFileEntry GetLockData() override;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
