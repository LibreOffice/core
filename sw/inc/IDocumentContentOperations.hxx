/*************************************************************************
 *
 *  $RCSfile: IDocumentContentOperations.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 15:11:48 $
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
        DOC_MOVEREDLINES = 0x04
    };

 public:
    /** Kopieren eines Bereiches im oder in ein anderes Dokument !
        Die Position kann auch im Bereich liegen !!
    */
    virtual bool Copy(SwPaM&, SwPosition&) const = 0;

    /** Loesche die Section, in der der Node steht.
    */
    virtual void DeleteSection(SwNode* pNode) = 0;

    /** loeschen eines BereichesSwFlyFrmFmt
    */
    virtual bool Delete(SwPaM&) = 0;

    /** loeschen gesamter Absaetze
    */
    virtual bool DelFullPara(SwPaM&) = 0;

    /** komplett loeschen eines Bereiches
    */
    virtual bool DeleteAndJoin(SwPaM&) = 0;

    /** verschieben eines Bereiches
    */
    virtual bool Move(SwPaM&, SwPosition&, SwMoveFlags) = 0;

    /** verschieben ganzer Nodes
    */
    virtual bool Move(SwNodeRange&, SwNodeIndex&, SwMoveFlags) = 0;

    /** verschieben eines Bereiches
    */
    virtual bool MoveAndJoin(SwPaM&, SwPosition&, SwMoveFlags) = 0;

    /** Ueberschreiben eines einzelnen Zeichens. rRg.Start() bezeichnet
        den Node und die Position in dem Node, an der eingefuegt wird,
    */
    virtual bool Overwrite(const SwPaM &rRg, sal_Unicode c) = 0;

    /** Ueberschreiben eines Strings in einem bestehenden Textnode.
    */
    virtual bool Overwrite(const SwPaM &rRg, const String& rStr) = 0;

    /** Einfuegen eines einzelnen Zeichens. rRg.Start() bezeichnet
        den Node und die Position in dem Node, an der eingefuegt wird.
    */
    virtual bool Insert(const SwPaM &rRg, sal_Unicode c) = 0;

    /** Einfuegen eines Strings in einen bestehenden
        Textnode. Der Text wird kopiert.
    */
    virtual bool Insert(const SwPaM &rRg, const String&, bool bHintExpand) = 0;

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

    virtual SwFlyFrmFmt* InsertOLE(const SwPaM &rRg, const String& rObjName, const SfxItemSet* pFlyAttrSet,
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
    virtual bool Replace(SwPaM& rPam, const String& rNewStr, bool bRegExpRplc) = 0;

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
    virtual bool Insert(const SwPaM &rRg, const SfxPoolItem&, sal_uInt16 nFlags) = 0;

    /**
    */
    virtual bool Insert(const SwPaM &rRg, const SfxItemSet&, sal_uInt16 nFlags) = 0;

    /** Removes any leading white space from the paragraph
    */
    virtual void RemoveLeadingWhiteSpace(const SwPosition & rPos ) = 0;

 protected:
    virtual ~IDocumentContentOperations() {};
};

#endif // IDOCUMENTCONTENTOPERATIONS_HXX_INCLUDED

