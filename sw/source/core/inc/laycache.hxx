/*************************************************************************
 *
 *  $RCSfile: laycache.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fme $ $Date: 2002-09-18 07:18:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _LAYCACHE_HXX
#define _LAYCACHE_HXX


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

#ifndef PRODUCT
    sal_Bool CompareLayout( const SwDoc& rDoc ) const;
#endif
};

#endif
