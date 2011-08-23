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
#ifndef _PSEUDO_HXX
#define _PSEUDO_HXX

#include <tools/unqid.hxx>
#include <bf_so3/iface.hxx>
#include <bf_svtools/ownlist.hxx>

#include "bf_so3/so3dllapi.h"

class  Menu;

namespace binfilter {

class  SvBaseLink;
class  SvEmbeddedClient;
class  ImpOleObject;

//=========================================================================
class SO3_DLLPUBLIC SvVerb
/*	[Beschreibung]

    Die Klasse beschreibt einen Befehl, der an ein SvPseudoObject
    geschickt werden kann.

    [Querverweise]

    <SvPseudoObject::DoVerb>
*/
{
private:
    long            nId;
    String          aName;
    UniqueItemId    aMenuId;
    BOOL    		bOnMenu : 1,
                    bConst  : 1;
public:
                    SvVerb( long nIdP, const String & rNameP,
                            BOOL bConstP = FALSE, BOOL bOnMenuP = TRUE );
                    SvVerb( const SvVerb & );
    SvVerb &        operator = ( const SvVerb & );
                    ~SvVerb();

    const String &  GetName() const { return aName; }
    long            GetId() const { return nId; }
    USHORT          GetMenuId() const { return (USHORT)aMenuId.GetId(); }
    BOOL            IsConst() const { return bConst; }
    BOOL            IsOnMenu() const { return bOnMenu; }
};

//=========================================================================
class SO3_DLLPUBLIC SvVerbList
{
    PRV_SV_DECL_OWNER_LIST(SvVerbList,SvVerb)
};

//=========================================================================
// Muessen mit OLE2 Werten uebereinstimmen
#define SVVERB_SHOW     	-1
#define SVVERB_OPEN     	-2
#define SVVERB_HIDE     	-3
#define SVVERB_UIACTIVATE	-4
#define SVVERB_IPACTIVATE   -5

// Eigene Verbs
#define SVVERB_PROPS		1

// Muessen mit OLE2 Werten uebereinstimmen
#define SVOBJ_MISCSTATUS_SERVERRESIZE 			1
#define SVOBJ_MISCSTATUS_NOTREPLACE   			4
#define SVOBJ_MISCSTATUS_CANTLINKINSIDE			16
#define SVOBJ_MISCSTATUS_LINK         			64
#define SVOBJ_MISCSTATUS_INSIDEOUT    			128
#define SVOBJ_MISCSTATUS_ACTIVATEWHENVISIBLE    256
#define SVOBJ_MISCSTATUS_NOTRESIZEABLE			512
#define SVOBJ_MISCSTATUS_ALWAYSACTIVATE			1024
#define SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE	2048
#define SVOBJ_MISCSTATUS_SPECIALOBJECT          4096

class SO3_DLLPUBLIC SvPseudoObject : virtual public SvObject
/*	[Beschreibung]

    Diese Klasse stellt die Basisschnittstelle fuer Klassen zur Verfuegung,
    deren Instanzen von aussen, z.B. via Link oder als Ole-Objekt,
    angesprochen werden kann. Sie besitzt folgende Eigenschaften:
    -Informationen (z.B. den Typ) ueber das Objekt liefern.
    -Daten- und Benachrichtigungsquelle ueber die Klassen
     <SvDataObject> und <SvAdvise>
    -Das Objekt anzeigen oder im Server aktivieren.
*/
{
friend class ImpOleObject;
private:
    SvVerbList *	pVerbs;          	// Liste der Verben
    BOOL			bDeleteVerbs;		// TRUE, wenn pVerbs loeschen
protected:
                    ~SvPseudoObject();

    virtual ErrCode Verb( long nVerbPos, SvEmbeddedClient * pCallerClient,
                        Window * pWin, const Rectangle * pWorkAreaPixel );
    virtual BOOL	Close();
public:
    SvPseudoObject();

    SO2_DECL_STANDARD_CLASS_DLL( SvPseudoObject, SOAPP )

    virtual void    FillClass( SvGlobalName * pClassName,
                               ULONG * pFormat,
                               String * pAppName,
                               String * pFullTypeName,
                               String * pShortTypeName,
                               long nFileFormat = SOFFICE_FILEFORMAT_CURRENT ) const;
    SvGlobalName    GetClassName() const;

                    // Befehle
    void            SetVerbList( SvVerbList * pVerbsP, BOOL bDeleteVerbs=FALSE );
    virtual const SvVerbList & GetVerbList() const;

    virtual ULONG	GetMiscStatus() const;

};

#ifndef SO2_DECL_SVPSEUDOOBJECT_DEFINED
#define SO2_DECL_SVPSEUDOOBJECT_DEFINED
SO2_DECL_REF(SvPseudoObject)
#endif
SO2_IMPL_REF( SvPseudoObject )

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
