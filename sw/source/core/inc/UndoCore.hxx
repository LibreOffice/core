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

#ifndef SW_UNDOCORE_HXX
#define SW_UNDOCORE_HXX

#include <undobj.hxx>

#include <calbck.hxx>


class SfxItemSet;
class SwFmtColl;
class SwFmtAnchor;
class SdrMarkList;
class SwUndoDelete;
class SwRedlineSaveData;
class SwFrm;
class SwFmt;

namespace sw {
    class UndoManager;
    class IShellCursorSupplier;
}


typedef SwRedlineSaveData* SwRedlineSaveDataPtr;
SV_DECL_PTRARR_DEL( SwRedlineSaveDatas, SwRedlineSaveDataPtr, 8, 8 )


namespace sw {

class SW_DLLPRIVATE UndoRedoContext
    : public SfxUndoContext
{
public:
    UndoRedoContext(SwDoc & rDoc, IShellCursorSupplier & rCursorSupplier)
        : m_rDoc(rDoc)
        , m_rCursorSupplier(rCursorSupplier)
        , m_pSelFmt(0)
        , m_pMarkList(0)
    { }

    SwDoc & GetDoc() const { return m_rDoc; }

    IShellCursorSupplier & GetCursorSupplier() { return m_rCursorSupplier; }

    void SetSelections(SwFrmFmt *const pSelFmt, SdrMarkList *const pMarkList)
    {
        m_pSelFmt = pSelFmt;
        m_pMarkList = pMarkList;
    }
    void GetSelections(SwFrmFmt *& o_rpSelFmt, SdrMarkList *& o_rpMarkList)
    {
        o_rpSelFmt = m_pSelFmt;
        o_rpMarkList = m_pMarkList;
    }

private:
    SwDoc & m_rDoc;
    IShellCursorSupplier & m_rCursorSupplier;
    SwFrmFmt * m_pSelFmt;
    SdrMarkList * m_pMarkList;
};

class SW_DLLPRIVATE RepeatContext
    : public SfxRepeatTarget
{
public:
    RepeatContext(SwDoc & rDoc, SwPaM & rPaM)
        : m_rDoc(rDoc)
        , m_pCurrentPaM(& rPaM)
        , m_bDeleteRepeated(false)
    { }

    SwDoc & GetDoc() const { return m_rDoc; }

    SwPaM & GetRepeatPaM()
    {
        return *m_pCurrentPaM;
    }

private:
    friend class ::sw::UndoManager;
    friend class ::SwUndoDelete;

    SwDoc & m_rDoc;
    SwPaM * m_pCurrentPaM;
    bool m_bDeleteRepeated; /// has a delete action been repeated?
};

} // namespace sw



class SwUndoFmtColl : public SwUndo, private SwUndRng
{
    String aFmtName;
    SwHistory* pHistory;
    SwFmtColl* pFmtColl;
    // for correct <ReDo(..)> and <Repeat(..)>
    // boolean, which indicates that the attributes are reseted at the nodes
    // before the format has been applied.
    const bool mbReset;
    // boolean, which indicates that the list attributes had been reseted at
    // the nodes before the format has been applied.
    const bool mbResetListAttrs;

    void DoSetFmtColl(SwDoc & rDoc, SwPaM & rPaM);

public:
    SwUndoFmtColl( const SwPaM&, SwFmtColl*,
                   const bool bReset,
                   const bool bResetListAttrs );
    virtual ~SwUndoFmtColl();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    /**
       Returns the rewriter for this undo object.

       The rewriter contains one rule:

           $1 -> <name of format collection>

       <name of format collection> is the name of the format
       collection that is applied by the action recorded by this undo
       object.

       @return the rewriter for this undo object
    */
    virtual SwRewriter GetRewriter() const;

    SwHistory* GetHistory() { return pHistory; }

};


class SwUndoSetFlyFmt : public SwUndo, public SwClient
{
    SwFrmFmt* pFrmFmt;                  // das gespeicherte FlyFormat
    SwFrmFmt* pOldFmt;                  // die alte Fly Vorlage
    SwFrmFmt* pNewFmt;                  // die neue Fly Vorlage
    SfxItemSet* pItemSet;               // die zurueck-/ gesetzten Attribute
    sal_uLong nOldNode, nNewNode;
    xub_StrLen nOldCntnt, nNewCntnt;
    sal_uInt16 nOldAnchorTyp, nNewAnchorTyp;
    sal_Bool bAnchorChgd;

    void PutAttr( sal_uInt16 nWhich, const SfxPoolItem* pItem );
    void Modify( const SfxPoolItem*, const SfxPoolItem* );
    void GetAnchor( SwFmtAnchor& rAnhor, sal_uLong nNode, xub_StrLen nCntnt );

public:
    SwUndoSetFlyFmt( SwFrmFmt& rFlyFmt, SwFrmFmt& rNewFrmFmt );
    virtual ~SwUndoSetFlyFmt();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    virtual SwRewriter GetRewriter() const;
    void DeRegisterFromFormat( SwFmt& );
};


//------------ Undo von verschieben/stufen von Gliederung ----------------

class SwUndoOutlineLeftRight : public SwUndo, private SwUndRng
{
    short nOffset;
public:
    SwUndoOutlineLeftRight( const SwPaM& rPam, short nOffset );

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );
};


const int nUndoStringLength = 20;

/**
   Shortens a string to a maximum length.

   @param rStr      the string to be shortened
   @param nLength   the maximum length for rStr
   @param rFillStr  string to replace cut out characters with

   If rStr has less than nLength characters it will be returned unaltered.

   If rStr has more than nLength characters the following algorithm
   generates the shortened string:

       frontLength = (nLength - length(rFillStr)) / 2
       rearLength = nLength - length(rFillStr) - frontLength
       shortenedString = concat(<first frontLength characters of rStr,
                                rFillStr,
                                <last rearLength characters of rStr>)

   Preconditions:
      - nLength - length(rFillStr) >= 2

   @return the shortened string
 */
String
ShortenString(const String & rStr, xub_StrLen nLength, const String & rFillStr);
/**
   Denotes special characters in a string.

   The rStr is split into parts containing special characters and
   parts not containing special characters. In a part containing
   special characters all characters are equal. These parts are
   maximal.

   @param rStr     the string to denote in

   The resulting string is generated by concatenating the found
   parts. The parts without special characters are surrounded by
   "'". The parts containing special characters are denoted as "n x",
   where n is the length of the part and x is the representation of
   the special character (i. e. "tab(s)").

   @return the denoted string
*/
String DenoteSpecialCharacters(const String & rStr);


#endif // SW_UNDOCORE_HXX

