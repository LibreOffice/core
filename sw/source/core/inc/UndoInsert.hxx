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

#ifndef SW_UNDO_INSERT_HXX
#define SW_UNDO_INSERT_HXX

#include <undobj.hxx>

#include <tools/mempool.hxx>

#include <swtypes.hxx>
#include <IDocumentContentOperations.hxx>


class Graphic;
class SwGrfNode;
class SwUndoDelete;
class SwUndoFmtAttr;


class SwUndoInsert: public SwUndo, private SwUndoSaveCntnt
{
    SwPosition *pPos;                   // Inhalt fuers Redo
    String *pTxt, *pUndoTxt;
    SwRedlineData* pRedlData;
    ULONG nNode;
    xub_StrLen nCntnt, nLen;
    BOOL bIsWordDelim : 1;
    BOOL bIsAppend : 1;

    const IDocumentContentOperations::InsertFlags m_nInsertFlags;

    friend class SwDoc;     // eigentlich nur SwDoc::Insert( String )
    BOOL CanGrouping( sal_Unicode cIns );
    BOOL CanGrouping( const SwPosition& rPos );

    SwDoc * pDoc;

    void Init(const SwNodeIndex & rNode);
    String * GetTxtFromDoc() const;

public:
    SwUndoInsert( const SwNodeIndex& rNode, xub_StrLen nCntnt, xub_StrLen nLen,
                  const IDocumentContentOperations::InsertFlags nInsertFlags,
                  BOOL bWDelim = TRUE );
    SwUndoInsert( const SwNodeIndex& rNode );
    virtual ~SwUndoInsert();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    // #111827#
    /**
       Returns rewriter for this undo object.

       The returned rewriter has the following rule:

           $1 -> '<inserted text>'

       <inserted text> is shortened to a length of nUndoStringLength.

       @return rewriter for this undo object
     */
    virtual SwRewriter GetRewriter() const;


    DECL_FIXEDMEMPOOL_NEWDEL(SwUndoInsert)
};


//--------------------------------------------------------------------

SwRewriter SW_DLLPRIVATE
MakeUndoReplaceRewriter(ULONG const ocurrences,
    ::rtl::OUString const& sOld, ::rtl::OUString const& sNew);

class SwUndoReplace
    : public SwUndo
{
public:
    SwUndoReplace(SwPaM const& rPam,
            ::rtl::OUString const& rInsert, bool const bRegExp);

    virtual ~SwUndoReplace();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    // #111827#
    /**
       Returns the rewriter of this undo object.

       If this undo object represents several replacements the
       rewriter contains the following rules:

           $1 -> <number of replacements>
           $2 -> occurrences of
           $3 -> <replaced text>

       If this undo object represents one replacement the rewriter
       contains these rules:

           $1 -> <replaced text>
           $2 -> "->"                   (STR_YIELDS)
           $3 -> <replacing text>

       @return the rewriter of this undo object
    */
    virtual SwRewriter GetRewriter() const;

    void SetEnd( const SwPaM& rPam );

private:
    class Impl;
    ::std::auto_ptr<Impl> m_pImpl;
};


//--------------------------------------------------------------------

class SwUndoReRead : public SwUndo
{
    Graphic *pGrf;
    String *pNm, *pFltr;
    ULONG nPos;
    USHORT nMirr;

    void SaveGraphicData( const SwGrfNode& );
    void SetAndSave( ::sw::UndoRedoContext & );

public:
    SwUndoReRead( const SwPaM& rPam, const SwGrfNode& pGrfNd );

    virtual ~SwUndoReRead();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
};

//--------------------------------------------------------------------

class SwUndoInsertLabel : public SwUndo
{
    union {
        struct {
            // fuer NoTxtFrms
            SwUndoInsLayFmt* pUndoFly;
            SwUndoFmtAttr* pUndoAttr;
        } OBJECT;
        struct {
            // fuer Tabelle/TextRahmen
            SwUndoDelete* pUndoInsNd;
            ULONG nNode;
        } NODE;
    };

    String sText;
    // --> PB 2005-01-06 #i39983# the separator is drawn with a character style
    String sSeparator;
    // <--
    String sNumberSeparator;
    String sCharacterStyle;
    // OD 2004-04-15 #i26791# - re-store of drawing object position no longer needed
    USHORT nFldId;
    SwLabelType eType;
    BYTE nLayerId;              // fuer Zeichen-Objekte
    BOOL bBefore        :1;
    BOOL bUndoKeep      :1;
    BOOL bCpyBrd        :1;

public:
    SwUndoInsertLabel( const SwLabelType eTyp, const String &rText,
    // --> PB 2005-01-06 #i39983# the separator is drawn with a character style
                        const String& rSeparator,
    // <--
                        const String& rNumberSeparator, //#i61007# order of captions
                        const BOOL bBefore, const USHORT nId,
                        const String& rCharacterStyle,
                        const BOOL bCpyBrd );
    virtual ~SwUndoInsertLabel();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    // #111827#
    /**
       Returns the rewriter of this undo object.

       The rewriter contains this rule:

           $1 -> '<text of inserted label>'

       <text of inserted label> is shortened to nUndoStringLength
       characters.

       @return the rewriter of this undo object
     */
    virtual SwRewriter GetRewriter() const;

    void SetNodePos( ULONG nNd )
        { if( LTYPE_OBJECT != eType ) NODE.nNode = nNd; }

    void SetUndoKeep()  { bUndoKeep = TRUE; }
    void SetFlys( SwFrmFmt& rOldFly, SfxItemSet& rChgSet, SwFrmFmt& rNewFly );
    void SetDrawObj( BYTE nLayerId );
};

#endif // SW_UNDO_INSERT_HXX

