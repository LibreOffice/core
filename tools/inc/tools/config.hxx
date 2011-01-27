/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _CONFIG_HXX
#define _CONFIG_HXX

#include "tools/toolsdllapi.h"
#include <tools/string.hxx>

struct ImplConfigData;
struct ImplGroupData;

// ----------
// - Config -
// ----------

class TOOLS_DLLPUBLIC Config
{
private:
    XubString           maFileName;
    ByteString          maGroupName;
    ImplConfigData*     mpData;
    ImplGroupData*      mpActGroup;
    ULONG               mnDataUpdateId;
    USHORT              mnLockCount;
    BOOL                mbPersistence;
    BOOL                mbDummy1;

#ifdef _CONFIG_CXX
    TOOLS_DLLPRIVATE BOOL               ImplUpdateConfig() const;
    TOOLS_DLLPRIVATE ImplGroupData*     ImplGetGroup() const;
#endif

public:
                        Config();
                        Config( const XubString& rFileName );
                        ~Config();

    const XubString&    GetPathName() const { return maFileName; }
    static XubString    GetDefDirectory();
    static XubString    GetConfigName( const XubString& rPath, const XubString& rBaseName );

    void                SetGroup( const ByteString& rGroup );
    const ByteString&   GetGroup() const { return maGroupName; }
    void                DeleteGroup( const ByteString& rGroup );
    ByteString          GetGroupName( USHORT nGroup ) const;
    USHORT              GetGroupCount() const;
    BOOL                HasGroup( const ByteString& rGroup ) const;

    ByteString          ReadKey( const ByteString& rKey ) const;
    UniString           ReadKey( const ByteString& rKey, rtl_TextEncoding eEncoding ) const;
    ByteString          ReadKey( const ByteString& rKey, const ByteString& rDefault ) const;
    void                WriteKey( const ByteString& rKey, const ByteString& rValue );
    void                WriteKey( const ByteString& rKey, const UniString& rValue, rtl_TextEncoding eEncoding );
    void                DeleteKey( const ByteString& rKey );
    ByteString          GetKeyName( USHORT nKey ) const;
    ByteString          ReadKey( USHORT nKey ) const;
    USHORT              GetKeyCount() const;

    void                EnterLock();
    void                LeaveLock();
    BOOL                IsLocked() const { return (mnLockCount != 0); }
    BOOL                Update();
    void                Flush();

    void                EnablePersistence( BOOL bPersistence = TRUE )
                            { mbPersistence = bPersistence; }
    BOOL                IsPersistenceEnabled() const { return mbPersistence; }

    void                SetLineEnd( LineEnd eLineEnd );
    LineEnd             GetLineEnd() const;

private:
    TOOLS_DLLPRIVATE                Config( const Config& rConfig );
    TOOLS_DLLPRIVATE Config&            operator = ( const Config& rConfig );
};

#endif // _SV_CONFIG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
