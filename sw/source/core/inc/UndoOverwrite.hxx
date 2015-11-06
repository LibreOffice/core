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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNDOOVERWRITE_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNDOOVERWRITE_HXX

#include <com/sun/star/uno/Sequence.h>
#include <rtl/ustring.hxx>
#include <undobj.hxx>

class SwRedlineSaveDatas;
class SwTextNode;

namespace utl {
    class TransliterationWrapper;
}

class SwUndoOverwrite: public SwUndo, private SwUndoSaveContent
{
    OUString aDelStr, aInsStr;
    SwRedlineSaveDatas* pRedlSaveData;
    sal_uLong nSttNode;
    sal_Int32 nSttContent;
    bool bInsChar : 1;  // no Overwrite, but Insert
    bool bGroup : 1;    // TRUE: is already grouped; evaluated in CanGrouping()

public:
    SwUndoOverwrite( SwDoc*, SwPosition&, sal_Unicode cIns );

    virtual ~SwUndoOverwrite();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    /**
       Returns the rewriter of this undo object.

       The rewriter contains the following rule:

           $1 -> '<overwritten text>'

       <overwritten text> is shortened to nUndoStringLength characters.

       @return the rewriter of this undo object
     */
    virtual SwRewriter GetRewriter() const override;

    bool CanGrouping( SwDoc*, SwPosition&, sal_Unicode cIns );
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

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    void AddChanges( SwTextNode& rTNd, sal_Int32 nStart, sal_Int32 nLen,
                     css::uno::Sequence <sal_Int32>& rOffsets );
    bool HasData() const { return aChanges.size() > 0; }
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNDOOVERWRITE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
