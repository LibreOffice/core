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
#ifndef _LINKMGR_HXX
#define _LINKMGR_HXX

#include <bf_so3/linksrc.hxx>
#include <tools/string.hxx>
#include <bf_svtools/svarray.hxx>

#include "bf_so3/so3dllapi.h"

namespace binfilter
{

class SvPersist;
class SvBaseLink;
class SvBaseLinkRef;

typedef SvBaseLinkRef* SvBaseLinkRefPtr;
SV_DECL_PTRARR( SvBaseLinks, SvBaseLinkRefPtr, 1, 1 )

typedef SvLinkSource* SvLinkSourcePtr;
SV_DECL_PTRARR( SvLinkSources, SvLinkSourcePtr, 1, 1 )

class SO3_DLLPUBLIC SvLinkManager
{
private:
    SvBaseLinks		aLinkTbl;
    SvLinkSources    aServerTbl;
    SvPersist	    *pPersist; // LinkMgr muss vor SvPersist freigegeben werden
protected:
    BOOL 		InsertLink( SvBaseLink* pLink, USHORT nObjType, USHORT nUpdateType,
                            const String* pName = 0 );
public:
                SvLinkManager();
                virtual ~SvLinkManager();

    SvPersist*	GetPersist() const 				{ return pPersist; }
    void        SetPersist( SvPersist * p ) 	{ pPersist = p; }

    void		Remove( SvBaseLink *pLink );
    void		Remove( USHORT nPos, USHORT nCnt = 1 );
    BOOL		Insert( SvBaseLink* pLink );

                // den Link mit einem SvLinkSource verbinden und in die Liste eintragen
    BOOL 		InsertDDELink( SvBaseLink*,
                        const String& rServer,
                        const String& rTopic,
                        const String& rItem );

                // falls am Link schon alles eingestellt ist !
    BOOL 		InsertDDELink( SvBaseLink* );

                // erfrage die Strings fuer den Dialog
    virtual BOOL GetDisplayNames( const SvBaseLink *,
                                    String* pType,
                                    String* pFile = 0,
                                    String* pLink = 0,
                                    String* pFilter = 0 ) const;

    virtual SvLinkSourceRef CreateObj( SvBaseLink * );

    void 		UpdateAllLinks( BOOL bCallErrHdl,
                                BOOL bUpdateGrfLinks );

                // Liste aller Links erfragen (z.B. fuer Verknuepfungs-Dialog)
    const 		SvBaseLinks& GetLinks() const { return aLinkTbl; }

    // ----------------- Serverseitige Verwaltung --------------------

                // Liste der zu serviereden Links erfragen
    const SvLinkSources& GetServers() const { return aServerTbl; }
                // einen zu servierenden Link eintragen/loeschen
    BOOL 		InsertServer( SvLinkSource* rObj );
    void 		RemoveServer( SvLinkSource* rObj );
    void 		RemoveServer( USHORT nPos, USHORT nCnt = 1 )
                {	aServerTbl.Remove( nPos, nCnt ); }

private:
                // diese Methoden gibts nicht!
    SO3_DLLPRIVATE 			SvLinkManager( const SvLinkManager& );
    SO3_DLLPRIVATE 			SvLinkManager& operator=( const SvLinkManager& );
};

// Trenner im LinkName fuer die DDE-/File-/Grafik- Links
// (nur wer es braucht, um einen SvLinkName zusammenzubasteln)
const sal_Char cTokenSeperator = '\xff';

// erzeuge einen String fuer den SvLinkName. Fuer
// - DDE die ersten 3 Strings, (Server, Topic, Item)
// - File-/Grf-LinkNms die letzen 3 Strings (FileName, Bereich, Filter)
void SO3_DLLPUBLIC MakeLnkName( String& rName,
                 const String* pType,		// kann auch 0 sein !!
                 const String& rFile,
                 const String& rLink,
                 const String* pFilter = 0 );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
