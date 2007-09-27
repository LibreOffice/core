/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ddefld.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 07:58:48 $
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
#ifndef _DDEFLD_HXX
#define _DDEFLD_HXX

#ifndef _LNKBASE_HXX //autogen
#include <sfx2/lnkbase.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

#ifndef _FLDBAS_HXX
#include "fldbas.hxx"
#endif

class SwDoc;

/*--------------------------------------------------------------------
    Beschreibung: FieldType fuer DDE
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwDDEFieldType : public SwFieldType
{
    String aName;
    String aExpansion;

    ::sfx2::SvBaseLinkRef refLink;
    SwDoc* pDoc;

    USHORT nRefCnt;
    BOOL bCRLFFlag : 1;
    BOOL bDeleted : 1;

    SW_DLLPRIVATE void _RefCntChgd();

public:
    SwDDEFieldType( const String& rName, const String& rCmd,
                    USHORT = sfx2::LINKUPDATE_ONCALL );
    ~SwDDEFieldType();

    const String& GetExpansion() const          { return aExpansion; }
    void SetExpansion( const String& rStr )     { aExpansion = rStr,
                                                  bCRLFFlag = FALSE; }

    virtual SwFieldType* Copy() const;
    virtual const String& GetName() const;

    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, USHORT nWhich ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, USHORT nWhich );

    String GetCmd() const;
    void SetCmd( const String& rStr );

    USHORT GetType() const          { return refLink->GetUpdateMode();  }
    void SetType( USHORT nType )    { refLink->SetUpdateMode( nType );  }

    BOOL IsDeleted() const          { return bDeleted; }
    void SetDeleted( BOOL b )       { bDeleted = b; }

    void UpdateNow()                { refLink->Update(); }
    void Disconnect()               { refLink->Disconnect(); }

    const ::sfx2::SvBaseLink& GetBaseLink() const    { return *refLink; }
          ::sfx2::SvBaseLink& GetBaseLink()          { return *refLink; }

    const SwDoc* GetDoc() const     { return pDoc; }
          SwDoc* GetDoc()           { return pDoc; }
    void SetDoc( SwDoc* pDoc );

    void IncRefCnt() {  if( !nRefCnt++ && pDoc ) _RefCntChgd(); }
    void DecRefCnt() {  if( !--nRefCnt && pDoc ) _RefCntChgd(); }

    void SetCRLFDelFlag( BOOL bFlag = TRUE )    { bCRLFFlag = bFlag; }
};

/*--------------------------------------------------------------------
    Beschreibung: DDE-Feld
 --------------------------------------------------------------------*/

class SwDDEField : public SwField
{
public:
    SwDDEField(SwDDEFieldType*);
    ~SwDDEField();

    virtual String   Expand() const;
    virtual SwField* Copy() const;

    // ueber Typen Parameter ermitteln
    // Name kann nicht geaendert werden
    virtual const String& GetPar1() const;

    // Commando
    virtual String  GetPar2() const;
    virtual void    SetPar2(const String& rStr);
};


#endif // _DDEFLD_HXX
