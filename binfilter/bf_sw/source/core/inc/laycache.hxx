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
#ifndef _LAYCACHE_HXX
#define _LAYCACHE_HXX
namespace binfilter {


class SwDoc;
class SwLayCacheImpl;

/*************************************************************************
 *                      class SwLayoutCache
 *
 * This class allows to save layout information in the file and it contains
 * this information after loading of a file.
 * Call Write(..) with a stream and the document to save and the page break
 * information of the document will be written.
 * Call Read(..) with a stream and the member pLayCacheImpl will
 * read the information from the stream and store it in an internal structur.
 * There's a simple locking mechanism at these classes,
 * if somebody reads the information, he increments the lock count by 1,
 * during the Read(..) function the lock count will set to $8000.
 *
 **************************************************************************/

class SwLayoutCache
{
    SwLayCacheImpl *pImpl;
    USHORT nLockCount;
public:
    SwLayoutCache() : pImpl( NULL ), nLockCount( 0 ) {}
    ~SwLayoutCache();

    void Read( SvStream &rStream );
    void Write( SvStream &rStream, const SwDoc& rDoc );

    void ClearImpl();
    sal_Bool IsLocked() const { return nLockCount > 0; }
    USHORT& GetLockCount() { return nLockCount; }
    SwLayCacheImpl *LockImpl()
        { if( nLockCount & 0x8000 ) return NULL;
          if ( pImpl )
            ++nLockCount;
          return pImpl; }
    void UnlockImpl() { --nLockCount; }

#ifdef DBG_UTIL
#endif
};

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
