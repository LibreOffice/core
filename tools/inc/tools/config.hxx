/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: config.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:09:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _CONFIG_HXX
#define _CONFIG_HXX

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

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
