/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsCacheCompactor.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 06:09:56 $
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

#ifndef SD_SLIDESORTER_CACHE_COMPACTOR_HXX
#define SD_SLIDESORTER_CACHE_COMPACTOR_HXX

#include <sal/types.h>

namespace sd { namespace slidesorter { namespace cache {

class BitmapCache;

/** This trivial compaction operation does not do anything.  It especially
    does not remove or scale down older bitmaps.  Use this operator for
    debugging, experiments, or when memory consumption is of no concern.
*/
class NoCompaction
{
public:
    void operator() (BitmapCache& rCache, sal_Int32 nMaximalSize);
};




/** Make room for new bitmaps by removing seldomly used ones.
*/
class CompactionByRemoval
{
public:
    void operator() (BitmapCache& rCache, sal_Int32 nMaximalSize);
};




/** Make room for new new bitmaps by reducing the resolution of old ones.
*/
class CompactionByReduction
{
public:
    void operator() (BitmapCache& rCache, sal_Int32 nMaximalSize);
};

} } } // end of namespace ::sd::slidesorter::cache

#endif
