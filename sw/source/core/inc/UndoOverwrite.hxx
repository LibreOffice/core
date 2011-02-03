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

#ifndef SW_UNDO_OVERWRITE_HXX
#define SW_UNDO_OVERWRITE_HXX

#include <undobj.hxx>

#include <com/sun/star/uno/Sequence.h>


class SwRedlineSaveDatas;
class SwTxtNode;

namespace utl {
    class TransliterationWrapper;
}


class SwUndoOverwrite: public SwUndo, private SwUndoSaveCntnt
{
    String aDelStr, aInsStr;
    SwRedlineSaveDatas* pRedlSaveData;
    ULONG nSttNode;
    xub_StrLen nSttCntnt;
    BOOL bInsChar : 1;  // no Overwrite, but Insert
    BOOL bGroup : 1;    // TRUE: is already grouped; evaluated in CanGrouping()

public:
    SwUndoOverwrite( SwDoc*, SwPosition&, sal_Unicode cIns );

    virtual ~SwUndoOverwrite();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    // #111827#
    /**
       Returns the rewriter of this undo object.

       The rewriter contains the following rule:

           $1 -> '<overwritten text>'

       <overwritten text> is shortened to nUndoStringLength characters.

       @return the rewriter of this undo object
     */
    virtual SwRewriter GetRewriter() const;

    BOOL CanGrouping( SwDoc*, SwPosition&, sal_Unicode cIns );
};

//--------------------------------------------------------------------

struct _UndoTransliterate_Data;
class SwUndoTransliterate : public SwUndo, public SwUndRng
{
    std::vector< _UndoTransliterate_Data * >    aChanges;
    sal_uInt32 nType;

    void DoTransliterate(SwDoc & rDoc, SwPaM & rPam);

public:
    SwUndoTransliterate( const SwPaM& rPam,
                            const utl::TransliterationWrapper& rTrans );

    virtual ~SwUndoTransliterate();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    void AddChanges( SwTxtNode& rTNd, xub_StrLen nStart, xub_StrLen nLen,
                     ::com::sun::star::uno::Sequence <sal_Int32>& rOffsets );
    BOOL HasData() const { return aChanges.size() > 0; }
};

#endif // SW_UNDO_OVERWRITE_HXX

