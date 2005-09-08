/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pntlock.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:44:21 $
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

#ifndef SC_PNTLOCK_HXX
#define SC_PNTLOCK_HXX

#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"
#endif

class ScPaintLockData
{
private:
    ScRangeListRef  xRangeList;
    USHORT          nMode;
    USHORT          nLevel;
    USHORT          nDocLevel;
    USHORT          nParts;
    BOOL            bModified;

public:
                    ScPaintLockData(USHORT nNewMode);
                    ~ScPaintLockData();

    void            AddRange( const ScRange& rRange, USHORT nP );

    void            SetModified()   { bModified = TRUE; }
    void            IncLevel(sal_Bool bDoc)
                        { if (bDoc) ++nDocLevel; else ++nLevel; }
    void            DecLevel(sal_Bool bDoc)
                        { if (bDoc) --nDocLevel; else --nLevel; }

    const ScRangeListRef&   GetRangeList() const            { return xRangeList; }
    USHORT                  GetParts() const                { return nParts; }
    USHORT                  GetLevel(sal_Bool bDoc) const
                                { return bDoc ? nDocLevel : nLevel; }
    BOOL                    GetModified() const             { return bModified; }

                    // fuer Wiederherstellen nach Reset
    void            SetLevel(USHORT nNew, sal_Bool bDoc)
                        { if (bDoc) nDocLevel = nNew; else nLevel = nNew; }
};

#endif

