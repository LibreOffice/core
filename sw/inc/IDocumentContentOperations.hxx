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



#ifndef IDOCUMENTCONTENTOPERATIONS_HXX_INCLUDED
#define IDOCUMENTCONTENTOPERATIONS_HXX_INCLUDED

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

class SwPaM;
struct SwPosition;
class SwNode;
class SwNodeRange;
class String;
class Graphic;
class SfxItemSet;
class SfxPoolItem;
class GraphicObject;
class SdrObject;
class SwFrmFmt;
class SwDrawFrmFmt;
class SwFlyFrmFmt;
class SwNodeIndex;
class SwFmtFld;

namespace utl { class TransliterationWrapper; }
namespace svt { class EmbeddedObjectRef; }

/** Text operation/manipulation interface
*/
class IDocumentContentOperations
{
public:
    enum SwMoveFlags
    {
        DOC_MOVEDEFAULT = 0x00,
        DOC_MOVEALLFLYS = 0x01,
        DOC_CREATEUNDOOBJ = 0x02,
        DOC_MOVEREDLINES = 0x04,
        DOC_NO_DELFRMS = 0x08
    };

    // constants for inserting text
    enum InsertFlags
    {   INS_DEFAULT         = 0x00 // no extras
    ,   INS_EMPTYEXPAND     = 0x01 // expand empty hints at insert position
    ,   INS_NOHINTEXPAND    = 0x02 // do not expand any hints at insert pos
    ,   INS_FORCEHINTEXPAND = 0x04 // expand all hints at insert position
    };

public:
    /** Kopieren eines Bereiches im oder in ein anderes Dokument !
    Die Position kann auch im Bereich liegen !!
    */
    virtual bool CopyRange(SwPaM&, SwPosition&, const bool bCopyAll ) const = 0;

    /** Loesche die Section, in der der Node steht.
    */
    virtual void DeleteSection(SwNode* pNode) = 0;

    /** loeschen eines BereichesSwFlyFrmFmt
    */
    virtual bool DeleteRange(SwPaM&) = 0;

    /** loeschen gesamter Absaetze
    */
    virtual bool DelFullPara(SwPaM&) = 0;

    /** complete delete of a given PaM

    OD 2009-08-20 #i100466#
    Add optional parameter <bForceJoinNext>, default value <false>
    Needed for hiding of deletion redlines
    */
    virtual bool DeleteAndJoin( SwPaM&,
        const bool bForceJoinNext = false ) = 0;

    /** verschieben eines Bereiches
    */
    virtual bool MoveRange(SwPaM&, SwPosition&, SwMoveFlags) = 0;

    /** verschieben ganzer Nodes
    */
    virtual bool MoveNodeRange(SwNodeRange&, SwNodeIndex&, SwMoveFlags) = 0;

    /** verschieben eines Bereiches
    */
    virtual bool MoveAndJoin(SwPaM&, SwPosition&, SwMoveFlags) = 0;

    /** Ueberschreiben eines Strings in einem bestehenden Textnode.
    */
    virtual bool Overwrite(const SwPaM &rRg, const String& rStr) = 0;

    /** Insert string into existing text node at position rRg.Point().
    */
    virtual bool InsertString(const SwPaM &rRg, const String&,
        const enum InsertFlags nInsertMode = INS_EMPTYEXPAND ) = 0;

    /** change text to Upper/Lower/Hiragana/Katagana/...
    */
    virtual void TransliterateText(const SwPaM& rPaM, utl::TransliterationWrapper&) = 0;

    /** Einfuegen einer Grafik, Formel. Die XXXX werden kopiert.
    */
    virtual SwFlyFrmFmt* Insert(const SwPaM &rRg, const String& rGrfName, const String& rFltName, const Graphic* pGraphic,
        const SfxItemSet* pFlyAttrSet, const SfxItemSet* pGrfAttrSet, SwFrmFmt*) = 0;

    /**
    */
    virtual SwFlyFrmFmt* Insert(const SwPaM& rRg, const GraphicObject& rGrfObj, const SfxItemSet* pFlyAttrSet,
        const SfxItemSet* pGrfAttrSet, SwFrmFmt*) = 0;

    /** austauschen einer Grafik (mit Undo)
    */
    virtual void ReRead(SwPaM&, const String& rGrfName, const String& rFltName, const Graphic* pGraphic, const GraphicObject* pGrfObj) = 0;

    /** Einfuegen eines DrawObjectes. Das Object muss bereits im DrawModel
    angemeldet sein.
    */
    virtual SwDrawFrmFmt* Insert(const SwPaM &rRg, SdrObject& rDrawObj, const SfxItemSet* pFlyAttrSet, SwFrmFmt*) = 0;

    /** Einfuegen von OLE-Objecten.
    */
    virtual SwFlyFrmFmt* Insert(const SwPaM &rRg, const svt::EmbeddedObjectRef& xObj, const SfxItemSet* pFlyAttrSet,
        const SfxItemSet* pGrfAttrSet, SwFrmFmt*) = 0;

    virtual SwFlyFrmFmt* InsertOLE(const SwPaM &rRg, const String& rObjName, sal_Int64 nAspect, const SfxItemSet* pFlyAttrSet,
        const SfxItemSet* pGrfAttrSet, SwFrmFmt*) = 0;

    /** Aufspalten eines Nodes an rPos (nur fuer den TxtNode implementiert)
    */
    virtual bool SplitNode(const SwPosition &rPos, bool bChkTableStart) = 0;

    /**
    */
    virtual bool AppendTxtNode(SwPosition& rPos) = 0;

    /** Ersetz einen selektierten Bereich in einem TextNode mit dem
    String. Ist fuers Suchen&Ersetzen gedacht.
    bRegExpRplc - ersetze Tabs (\\t) und setze den gefundenen String
    ein ( nicht \& )
    z.B.: Fnd: "zzz", Repl: "xx\t\\t..&..\&"
    --> "xx\t<Tab>..zzz..&"
    */
    virtual bool ReplaceRange(SwPaM& rPam, const String& rNewStr,
        const bool bRegExReplace) = 0;

    /** Einfuegen eines Attributs. Erstreckt sich rRg ueber
    mehrere Nodes, wird das Attribut aufgespaltet, sofern
    dieses Sinn macht. Nodes, in denen dieses Attribut keinen
    Sinn macht, werden ignoriert.  In vollstaendig in der
    Selektion eingeschlossenen Nodes wird das Attribut zu
    harter Formatierung, in den anderen (Text-)Nodes wird das
    Attribut in das Attributearray eingefuegt. Bei einem
    Zeichenattribut wird ein "leerer" Hint eingefuegt,
    wenn keine Selektion
    vorliegt; andernfalls wird das Attribut als harte
    Formatierung dem durch rRg.Start() bezeichneten Node
    hinzugefuegt.  Wenn das Attribut nicht eingefuegt werden
    konnte, liefert die Methode sal_False.
    */
    //Modify here for #119405, by chengjh, 2012-08-16
    //Add a para for the char attribute exp...
    virtual bool InsertPoolItem(
        const SwPaM &rRg,
        const SfxPoolItem&,
        const sal_uInt16 nFlags,
        const bool bExpandCharToPara=false ) = 0;
    //End

    /**
    */
    virtual bool InsertItemSet (const SwPaM &rRg, const SfxItemSet&,
        const sal_uInt16 nFlags) = 0;

    /** Removes any leading white space from the paragraph
    */
    virtual void RemoveLeadingWhiteSpace(const SwPosition & rPos ) = 0;

protected:
    virtual ~IDocumentContentOperations() {};
};

#endif // IDOCUMENTCONTENTOPERATIONS_HXX_INCLUDED

