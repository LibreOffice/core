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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNDOBOOKMARK_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNDOBOOKMARK_HXX

#include <memory>
#include <undobj.hxx>

class SwHistoryBookmark;
class SwHistoryNoTextFieldmark;
class SwHistoryTextFieldmark;

namespace sw {
    namespace mark {
        class IMark;
        class IFieldmark;
    }
}

class SwDoc;

class SwUndoBookmark : public SwUndo
{
    const std::unique_ptr<SwHistoryBookmark> m_pHistoryBookmark;

protected:
    SwUndoBookmark( SwUndoId nUndoId, const ::sw::mark::IMark& );

    void SetInDoc( SwDoc* );
    void ResetInDoc( SwDoc* );

public:
    virtual ~SwUndoBookmark() override;

    /**
       Returns the rewriter for this undo object.

       The rewriter contains the following rule:

           $1 -> <name of bookmark>

       <name of bookmark> is the name of the bookmark whose
       insertion/deletion is recorded by this undo object.

       @return the rewriter for this undo object
     */
    virtual SwRewriter GetRewriter() const override;
};

class SwUndoInsBookmark : public SwUndoBookmark
{
public:
    SwUndoInsBookmark( const ::sw::mark::IMark& );

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
};

class SwUndoDeleteBookmark : public SwUndoBookmark
{
public:
    SwUndoDeleteBookmark( const ::sw::mark::IMark& );

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
};

class SwUndoRenameBookmark : public SwUndo
{
    const OUString m_sOldName;
    const OUString m_sNewName;

public:
    SwUndoRenameBookmark( const OUString& rOldName, const OUString& rNewName, const SwDoc* pDoc );
    virtual ~SwUndoRenameBookmark() override;

private:
    virtual SwRewriter GetRewriter() const override;
    static void Rename( ::sw::UndoRedoContext const &, const OUString& sFrom, const OUString& sTo );
    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
};

/// Handling undo / redo of checkbox and drop-down form field insertion
class SwUndoInsNoTextFieldmark : public SwUndo
{
private:
    const std::unique_ptr<SwHistoryNoTextFieldmark> m_pHistoryNoTextFieldmark;

public:
    SwUndoInsNoTextFieldmark(const ::sw::mark::IFieldmark& rFieldmark);

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
};

/// Handling undo / redo of text form field insertion
class SwUndoInsTextFieldmark : public SwUndo
{
private:
    const std::unique_ptr<SwHistoryTextFieldmark> m_pHistoryTextFieldmark;

public:
    SwUndoInsTextFieldmark(const ::sw::mark::IFieldmark& rFieldmark);

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNDOBOOKMARK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
