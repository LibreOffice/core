/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _NDOLE_HXX
#define _NDOLE_HXX

#include <ndnotxt.hxx>

#include <svtools/embedhlp.hxx>

class SwGrfFmtColl;
class SwDoc;
class SwOLENode;

class SwOLEListener_Impl;
class SwEmbedObjectLink;
class SW_DLLPUBLIC SwOLEObj
{
    friend class SwOLENode;

    const SwOLENode* pOLENd;
    SwOLEListener_Impl* pListener;

    //Entweder Ref oder Name sind bekannt, wenn nur der Name bekannt ist, wird
    //dir Ref bei Anforderung durch GetOleRef() vom Sfx besorgt.
    svt::EmbeddedObjectRef xOLERef;
    String aName;

    SwOLEObj( const SwOLEObj& rObj );   //nicht erlaubt.
    SwOLEObj();

    void SetNode( SwOLENode* pNode );

public:
    SwOLEObj( const svt::EmbeddedObjectRef& pObj );
    SwOLEObj( const String &rName, sal_Int64 nAspect );
    ~SwOLEObj();

    sal_Bool UnloadObject();
    static sal_Bool UnloadObject( ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject > xObj,
                                const SwDoc* pDoc,
                                sal_Int64 nAspect );

    String GetDescription();

#ifndef _FESHVIEW_ONLY_INLINE_NEEDED
    const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > GetOleRef();
    svt::EmbeddedObjectRef& GetObject();
    const String& GetCurrentPersistName() const { return aName; }
    String GetStyleString();
    sal_Bool IsOleRef() const;  //Damit das Objekt nicht unnoetig geladen werden muss.
#endif
};


// --------------------
// SwOLENode
// --------------------

class SW_DLLPUBLIC SwOLENode: public SwNoTxtNode
{
    friend class SwNodes;
    mutable SwOLEObj aOLEObj;
    Graphic*    pGraphic;
    String sChartTblName;       // bei Chart Objecten: Name der ref. Tabelle
    sal_Bool   bOLESizeInvalid;     //Soll beim SwDoc::PrtOLENotify beruecksichtig
                                //werden (zum Beispiel kopiert). Ist nicht
                                //Persistent.

    SwEmbedObjectLink*  mpObjectLink;
    String maLinkURL;

    SwOLENode(  const SwNodeIndex &rWhere,
                const svt::EmbeddedObjectRef&,
                SwGrfFmtColl *pGrfColl,
                SwAttrSet* pAutoAttr = 0 );

    SwOLENode(  const SwNodeIndex &rWhere,
                const String &rName,
                sal_Int64 nAspect,
                SwGrfFmtColl *pGrfColl,
                SwAttrSet* pAutoAttr = 0 );

    // aOLEObj besitzt einen privaten Copy-CTOR, wir brauchen auch einen:
    SwOLENode( const SwOLENode & );

    using SwNoTxtNode::GetGraphic;

public:
    const SwOLEObj& GetOLEObj() const { return aOLEObj; }
          SwOLEObj& GetOLEObj()       { return aOLEObj; }
    ~SwOLENode();

    virtual SwCntntNode *SplitCntntNode( const SwPosition & );
        // steht in ndcopy.cxx
    virtual SwCntntNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const;

    virtual Size GetTwipSize() const;

    Graphic* GetGraphic();

    Graphic* GetHCGraphic(); // tries to retrieve HighContrast representation if possible
    void GetNewReplacement();

    virtual sal_Bool SavePersistentData();
    virtual sal_Bool RestorePersistentData();

    sal_Bool IsInGlobalDocSection() const;
    sal_Bool IsOLEObjectDeleted() const;

    sal_Bool IsOLESizeInvalid() const   { return bOLESizeInvalid; }
    void SetOLESizeInvalid( sal_Bool b ){ bOLESizeInvalid = b; }

    sal_Int64 GetAspect() const { return aOLEObj.GetObject().GetViewAspect(); }
    void SetAspect( sal_Int64 nAspect) { aOLEObj.GetObject().SetViewAspect( nAspect ); }

    // OLE-Object aus dem "Speicher" entfernen
    // inline void Unload() { aOLEObj.Unload(); }
    String GetDescription() const { return aOLEObj.GetDescription(); }

    sal_Bool UpdateLinkURL_Impl();
    void BreakFileLink_Impl();
    void DisconnectFileLink_Impl();

    void CheckFileLink_Impl();

    // --> OD 2009-03-05 #i99665#
    bool IsChart() const;
    // <--

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
