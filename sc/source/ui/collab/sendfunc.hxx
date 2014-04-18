/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_COLLAB_SENDFUNC_HXX
#define INCLUDED_SC_SOURCE_UI_COLLAB_SENDFUNC_HXX

#include <sal/config.h>

#include "docfunc.hxx"
class ScCollaboration;
class ScChangeOpWriter;

class ScDocFuncSend : public ScDocFunc
{
    ScDocFuncDirect*    mpDirect;
    ScCollaboration*    mpCollaboration;

    friend class ScCollaboration;
    void                RecvMessage( const OString &rString );
    void                SendMessage( ScChangeOpWriter &rOp );
public:
    // FIXME: really ScDocFunc should be an abstract base, so
    // we don't need the rDocSh hack/pointer
    ScDocFuncSend( ScDocShell& rDocSh, ScDocFuncDirect* pDirect, ScCollaboration* pCollaboration );
    virtual ~ScDocFuncSend();

    virtual void        EnterListAction( sal_uInt16 nNameResId );
    virtual void        EndListAction();

    virtual sal_Bool    SetNormalString( bool& o_rbNumFmtSet, const ScAddress& rPos, const OUString& rText, sal_Bool bApi );
    virtual bool SetValueCell( const ScAddress& rPos, double fVal, bool bInteraction );
    virtual bool SetValueCells( const ScAddress& rPos, const std::vector<double>& aVals, bool bInteraction );
    virtual bool SetStringCell( const ScAddress& rPos, const OUString& rStr, bool bInteraction );
    virtual bool SetEditCell( const ScAddress& rPos, const EditTextObject& rStr, bool bInteraction );
    virtual bool SetFormulaCell( const ScAddress& rPos, ScFormulaCell* pCell, bool bInteraction );
    virtual bool PutData( const ScAddress& rPos, ScEditEngineDefaulter& rEngine, bool bApi );
    virtual bool SetCellText(
        const ScAddress& rPos, const OUString& rText, bool bInterpret, bool bEnglish,
        bool bApi, const formula::FormulaGrammar::Grammar eGrammar );
    virtual bool        ShowNote( const ScAddress& rPos, bool bShow = true );
    virtual bool        SetNoteText( const ScAddress& rPos, const OUString& rNoteText, sal_Bool bApi );
    virtual sal_Bool    RenameTable( SCTAB nTab, const OUString& rName, sal_Bool bRecord, sal_Bool bApi );
    virtual sal_Bool    ApplyAttributes( const ScMarkData& rMark, const ScPatternAttr& rPattern,
                                         sal_Bool bRecord, sal_Bool bApi );
    virtual sal_Bool    ApplyStyle( const ScMarkData& rMark, const OUString& rStyleName,
                                    sal_Bool bRecord, sal_Bool bApi );
    virtual sal_Bool    MergeCells( const ScCellMergeOption& rOption, sal_Bool bContents,
                                    sal_Bool bRecord, sal_Bool bApi );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
