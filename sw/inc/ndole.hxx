/*************************************************************************
 *
 *  $RCSfile: ndole.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-08 21:17:20 $
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
#ifndef _NDOLE_HXX
#define _NDOLE_HXX

#ifndef _NDNOTXT_HXX
#include <ndnotxt.hxx>
#endif

class SwGrfFmtColl;
class SwDoc;
class SvInPlaceObjectRef;
class SvInPlaceObject;
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
    SvInPlaceObjectRef *pOLERef;    //new/delete, damit so2.hxx wegfaellt.
    String aName;

    SwOLEObj( const SwOLEObj& rObj );   //nicht erlaubt.
    SwOLEObj();

    void SetNode( SwOLENode* pNode );

public:
    SwOLEObj( SvInPlaceObject *pObj );
    SwOLEObj( const String &rName );
    ~SwOLEObj();

    // OLE-Object aus dem "Speicher" entfernen
    void Unload();
    BOOL RemovedFromLRU();

#ifndef _FESHVIEW_ONLY_INLINE_NEEDED
    SvInPlaceObjectRef GetOleRef();
    const String &GetName() const { return aName; }

    BOOL IsOleRef() const;  //Damit das Objekt nicht unnoetig geladen werden muss.
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
    String sChartTblName;       // bei Chart Objecten: Name der ref. Tabelle
    BOOL   bOLESizeInvalid;     //Soll beim SwDoc::PrtOLENotify beruecksichtig
                                //werden (zum Beispiel kopiert). Ist nicht
                                //Persistent.

    SwOLENode(  const SwNodeIndex &rWhere,
                SvInPlaceObject *,
                SwGrfFmtColl *pGrfColl,
                SwAttrSet* pAutoAttr = 0 );

    SwOLENode(  const SwNodeIndex &rWhere,
                const String &rName,
                SwGrfFmtColl *pGrfColl,
                SwAttrSet* pAutoAttr = 0 );

    // aOLEObj besitzt einen privaten Copy-CTOR, wir brauchen auch einen:
    SwOLENode( const SwOLENode & );

public:
    const SwOLEObj& GetOLEObj() const { return aOLEObj; }
          SwOLEObj& GetOLEObj()       { return aOLEObj; }

    virtual SwCntntNode *SplitNode( const SwPosition & );
        // steht in ndcopy.cxx
    virtual SwCntntNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const;

    virtual Size GetTwipSize() const;

    virtual BOOL SavePersistentData();
    virtual BOOL RestorePersistentData();

    BOOL IsInGlobalDocSection() const;
    BOOL IsOLEObjectDeleted() const;

    BOOL IsOLESizeInvalid() const   { return bOLESizeInvalid; }
    void SetOLESizeInvalid( BOOL b ){ bOLESizeInvalid = b; }

    // OLE-Object aus dem "Speicher" entfernen
    inline void Unload() { aOLEObj.Unload(); }

#ifndef _FESHVIEW_ONLY_INLINE_NEEDED
    const String& GetChartTblName() const       { return sChartTblName; }
    void SetChartTblName( const String& rNm )   { sChartTblName = rNm; }
#endif
};

// Inline Metoden aus Node.hxx - erst hier ist der TxtNode bekannt !!
inline SwOLENode *SwNode::GetOLENode()
{
     return ND_OLENODE == nNodeType ? (SwOLENode*)this : 0;
}
inline const SwOLENode *SwNode::GetOLENode() const
{
     return ND_OLENODE == nNodeType ? (const SwOLENode*)this : 0;
}


#endif  // _NDOLE_HXX

