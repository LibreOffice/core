/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docpool.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:33:14 $
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

#ifndef SC_SCDOCPOL_HXX
#define SC_SCDOCPOL_HXX

#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif

class ScStyleSheet;

//------------------------------------------------------------------------

class ScDocumentPool: public SfxItemPool
{
    SfxPoolItem**   ppPoolDefaults;
    SfxItemPool*    pSecondary;
    static USHORT*  pVersionMap1;
    static USHORT*  pVersionMap2;
    static USHORT*  pVersionMap3;
    static USHORT*  pVersionMap4;
    static USHORT*  pVersionMap5;
    static USHORT*  pVersionMap6;
    static USHORT*  pVersionMap7;
    static USHORT*  pVersionMap8;
    static USHORT*  pVersionMap9;
    static USHORT*  pVersionMap10;

public:
            ScDocumentPool( SfxItemPool* pSecPool = NULL, BOOL bLoadRefCounts = FALSE );
            ~ScDocumentPool();

    virtual SfxItemPool*        Clone() const;
    virtual SfxMapUnit          GetMetric( USHORT nWhich ) const;

    virtual const SfxPoolItem&  Put( const SfxPoolItem&, USHORT nWhich = 0 );
    virtual void                Remove( const SfxPoolItem& );
    static void                 CheckRef( const SfxPoolItem& );
    void                        MyLoadCompleted();

    void StyleDeleted( ScStyleSheet* pStyle );      // Loeschen von Vorlagen im Organizer
    virtual SfxItemPresentation     GetPresentation(
                                        const SfxPoolItem&  rItem,
                                        SfxItemPresentation ePresentation,
                                        SfxMapUnit          ePresentationMetric,
                                        String&             rText,
                                        const IntlWrapper* pIntl = 0 ) const;

    static void InitVersionMaps();
    static void DeleteVersionMaps();
};




#endif
