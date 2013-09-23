/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef SW_UNDO_OVERWRITE_HXX
#define SW_UNDO_OVERWRITE_HXX

#include <com/sun/star/uno/Sequence.h>
#include <tools/string.hxx>
#include <undobj.hxx>

class SwRedlineSaveDatas;
class SwTxtNode;

namespace utl {
    class TransliterationWrapper;
}

class SwUndoOverwrite: public SwUndo, private SwUndoSaveCntnt
{
    String aDelStr, aInsStr;
    SwRedlineSaveDatas* pRedlSaveData;
    sal_uLong nSttNode;
    xub_StrLen nSttCntnt;
    sal_Bool bInsChar : 1;  // no Overwrite, but Insert
    sal_Bool bGroup : 1;    // TRUE: is already grouped; evaluated in CanGrouping()

public:
    SwUndoOverwrite( SwDoc*, SwPosition&, sal_Unicode cIns );

    virtual ~SwUndoOverwrite();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    /**
       Returns the rewriter of this undo object.

       The rewriter contains the following rule:

           $1 -> '<overwritten text>'

       <overwritten text> is shortened to nUndoStringLength characters.

       @return the rewriter of this undo object
     */
    virtual SwRewriter GetRewriter() const;

    sal_Bool CanGrouping( SwDoc*, SwPosition&, sal_Unicode cIns );
};

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
    sal_Bool HasData() const { return aChanges.size() > 0; }
};

#endif // SW_UNDO_OVERWRITE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
