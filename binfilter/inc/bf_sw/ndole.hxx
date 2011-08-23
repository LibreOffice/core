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
#ifndef _NDOLE_HXX
#define _NDOLE_HXX

#include <ndnotxt.hxx>

namespace binfilter {

class SvInPlaceObjectRef; 
class SvInPlaceObject; 

class SwGrfFmtColl;
class SwDoc;
class SwOLENode;
class SwOLELink;
class SwOLELRUCache;


class SwOLEObj
{
    friend class SwOLENode;

    static SwOLELRUCache* pOLELRU_Cache;

    const SwOLENode* pOLENd;

    //Entweder Ref oder Name sind bekannt, wenn nur der Name bekannt ist, wird
    //dir Ref bei Anforderung durch GetOleRef() vom Sfx besorgt.
    SvInPlaceObjectRef *pOLERef;	//new/delete, damit so2.hxx wegfaellt.
    String aName;

    SwOLEObj( const SwOLEObj& rObj );	//nicht erlaubt.
    SwOLEObj();

    void SetNode( SwOLENode* pNode );

public:
    SwOLEObj( SvInPlaceObject *pObj );
    SwOLEObj( const String &rName );
    ~SwOLEObj();

     BOOL RemovedFromLRU();

#ifndef _FESHVIEW_ONLY_INLINE_NEEDED
    SvInPlaceObjectRef GetOleRef();
    const String &GetName() const { return aName; }

    BOOL IsOleRef() const;	//Damit das Objekt nicht unnoetig geladen werden muss.
#endif
};


// --------------------
// SwOLENode
// --------------------

struct SwPersistentOleData;

class SwOLENode: public SwNoTxtNode
{
    friend class SwNodes;
    SwOLEObj aOLEObj;
    SwPersistentOleData* pSavedData;
    String sChartTblName;		// bei Chart Objecten: Name der ref. Tabelle
    BOOL   bOLESizeInvalid;		//Soll beim SwDoc::PrtOLENotify beruecksichtig
                                //werden (zum Beispiel kopiert). Ist nicht
                                //Persistent.

    SwOLENode(	const SwNodeIndex &rWhere,
                SvInPlaceObject *,
                SwGrfFmtColl *pGrfColl,
                SwAttrSet* pAutoAttr = 0 );

    SwOLENode(	const SwNodeIndex &rWhere,
                const String &rName,
                SwGrfFmtColl *pGrfColl,
                SwAttrSet* pAutoAttr = 0 );

    // aOLEObj besitzt einen privaten Copy-CTOR, wir brauchen auch einen:
    SwOLENode( const SwOLENode & );

public:
    const SwOLEObj& GetOLEObj() const { return aOLEObj; }
          SwOLEObj& GetOLEObj()		  { return aOLEObj; }

    virtual SwCntntNode *SplitNode( const SwPosition & );
        // steht in ndcopy.cxx
    virtual SwCntntNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const;

    virtual Size GetTwipSize() const;


    BOOL IsInGlobalDocSection() const;
    BOOL IsOLEObjectDeleted() const;

    BOOL IsOLESizeInvalid() const	{ return bOLESizeInvalid; }
    void SetOLESizeInvalid( BOOL b ){ bOLESizeInvalid = b; }

#ifndef _FESHVIEW_ONLY_INLINE_NEEDED
    const String& GetChartTblName() const 		{ return sChartTblName; }
    void SetChartTblName( const String& rNm ) 	{ sChartTblName = rNm; }
#endif
};


// Inline Metoden aus Node.hxx - erst hier ist der TxtNode bekannt !!
#if !(defined(MACOSX) && ( __GNUC__ < 3 ))
// GrP moved to gcc_outl.cxx; revisit with gcc3
inline SwOLENode *SwNode::GetOLENode()
{
     return ND_OLENODE == nNodeType ? (SwOLENode*)this : 0;
}
inline const SwOLENode *SwNode::GetOLENode() const
{
     return ND_OLENODE == nNodeType ? (const SwOLENode*)this : 0;
}
#endif

} //namespace binfilter
#endif	// _NDOLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
