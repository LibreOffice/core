/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: resid.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:16:44 $
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

#ifndef _TOOLS_RESID_HXX
#define _TOOLS_RESID_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

struct RSHEADER_TYPE;
typedef sal_uInt32 RESOURCE_TYPE;
#define RSC_NOTYPE              0x100
#define RSC_DONTRELEASE         (sal_uInt32(1 << 31))

class ResMgr;

//---------
//- ResId -
//---------

class ResId
{
    RSHEADER_TYPE*  pResource;     // Ist pResource == 0, dann nResId gueltig,
                                   // sonst zeigt pResource auf die Resource
                                   // Hoechstes Bit entscheidet ueber Freigabe
                                   // der Resource in Increment
    sal_uInt32      nResId;        // Resource Identifier
    RESOURCE_TYPE   nRT;           // ResourceTyp zum Laden und Typ
    ResMgr *        pResMgr;       // Ueber diesen ResMgr addressieren
    RESOURCE_TYPE   nRT2;          // ResourceTyp zum Laden, "uberschreibt nRT

public:
    ULONG           aWinBits;

                    ResId( RSHEADER_TYPE * pRc )
                    {
                        nResId      = 0;
                        pResource   = pRc;
                        nRT = nRT2  = RSC_NOTYPE;
                        pResMgr     = NULL;
                    }
                    ResId( sal_uInt32 nId, ResMgr * pMgr = NULL )
                    {
                        nResId      = nId;
                        pResource   = NULL;
                        nRT = nRT2  = RSC_NOTYPE;
                        pResMgr     = pMgr;
                    }
                    // backwards compatibility; avoid ambiguities
                    ResId( USHORT nId, ResMgr* pMgr = NULL )
                    {
                        nResId      = sal_uInt32(nId);
                        pResource   = NULL;
                        nRT = nRT2  = RSC_NOTYPE;
                        pResMgr     = pMgr;
                    }
                    ResId( int nId, ResMgr* pMgr = NULL )
                    {
                        nResId      = sal_uInt32(nId);
                        pResource   = NULL;
                        nRT = nRT2  = RSC_NOTYPE;
                        pResMgr     = pMgr;
                    }
                    ResId( long nId, ResMgr* pMgr = NULL )
                    {
                        nResId      = sal_uInt32(nId);
                        pResource   = NULL;
                        nRT = nRT2  = RSC_NOTYPE;
                        pResMgr     = pMgr;
                    }

    RESOURCE_TYPE   GetRT() const { return( nRT ); }
    const ResId &   SetRT( RESOURCE_TYPE nTyp ) const
                    /*  [Beschreibung]

                        Setzt den Typ, wenn er vorher noch nicht gesetzt
                        wurde. Er kann mit <ResId::GetRT()> abgefragt werden.

                        [Anmerkung]

                        Die Methode ist const, um die Compiler-Warning
                        zu umgehen.

                        [Beispiel]

                        ResId aId( 1000 );
                        aId.SetRT( RSC_WINDOW );    // setzt den Typ Window
                        aId.SetRT( RSC_BUTTON );    // setzt nicht mehr Typ
                        //aId.GetRT() == RSC_WINDOW ist wahr

                        [Querverweise]

                        <ResId::GetRT2()>, <ResId::GetRT()>
                    */
                    {
                        if( RSC_NOTYPE == nRT )
                            ((ResId *)this)->nRT = nTyp;
                        return *this;
                    }
    RESOURCE_TYPE   GetRT2() const
                    /*  [Beschreibung]

                        Gibt den Typ zur"uck, der mit <ResId::SetRT2()>
                        gesetzt wurde. Wurde damit kein Typ gesetzt,
                        dann wird der Typ zur"uckgegeben, der mit
                        <ResId::SetRT()> gesetzt wurde. Ansonsten wird
                        RSC_NOTYPE zur"uckgegeben.

                        [Anmerkung]

                        Ein zweiter Ressourcetyp ist notwendig, da
                        der erste in StarView zum Erzeugen der
                        Systemfenster in der Klasse <Window> ben"otigt wird
                        und deshalb nur bekannte Typen enthalten darf.
                    */
                    {
                        if( RSC_NOTYPE == nRT2 )
                            return( nRT );
                        else
                            return( nRT2 );
                    }
    const ResId &   SetRT2( RESOURCE_TYPE nTyp ) const
                    /*  [Beschreibung]

                        Setzt den Typ, wenn er vorher noch nicht gesetzt
                        wurde. Er kann mit <ResId::GetRT2()> abgefragt werden.

                        [Anmerkung]

                        SetRT2() darf nur gerufen werden, wenn keine
                        abgeleitete Klasse in ihrem Ressource-Konstruktor
                        <ResId::SetRT()> ruft.
                    */
                    {
                        if( RSC_NOTYPE == nRT2 )
                            ((ResId *)this)->nRT2 = nTyp;
                        return *this;
                    }

    ResMgr *        GetResMgr() const { return pResMgr; }
    const ResId &   SetResMgr( ResMgr * pMgr ) const
                    {
                        // const, wegen Compiler warning
                        ((ResId *)this)->pResMgr = pMgr;
                        return *this;
                    }

    const ResId &  SetAutoRelease(BOOL bRelease) const
                   {
                       if( bRelease )
                           ((ResId *)this)->nResId &= ~RSC_DONTRELEASE;
                       else
                           ((ResId *)this)->nResId |= RSC_DONTRELEASE;
                       return *this;
                   }

    BOOL           IsAutoRelease()  const
                   { return !(nResId & RSC_DONTRELEASE); }

    sal_uInt32     GetId()          const { return nResId & ~RSC_DONTRELEASE; }
    RSHEADER_TYPE* GetpResource()   const { return pResource; }
};

#endif // _RESID_HXX
