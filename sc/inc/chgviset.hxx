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
#ifndef INCLUDED_SC_INC_CHGVISET_HXX
#define INCLUDED_SC_INC_CHGVISET_HXX

#include <tools/datetime.hxx>
#include <svx/ctredlin.hxx>
#include "rangelst.hxx"
#include "scdllapi.h"

namespace utl {
    class TextSearch;
}

class ScDocument;

class SC_DLLPUBLIC ScChangeViewSettings
{
private:

    utl::TextSearch*    pCommentSearcher;
    DateTime            aFirstDateTime;
    DateTime            aLastDateTime;
    OUString            aAuthorToShow;
    OUString            aComment;
    ScRangeList         aRangeList;
    SvxRedlinDateMode   eDateMode;
    bool                bShowIt;
    bool                bIsDate;
    bool                bIsAuthor;
    bool                bIsComment;
    bool                bIsRange;
    bool                bEveryoneButMe;
    bool                bShowAccepted;
    bool                bShowRejected;
    bool                mbIsActionRange;
    sal_uLong           mnFirstAction;
    sal_uLong           mnLastAction;

public:

    ScChangeViewSettings()
        : pCommentSearcher(nullptr)
        , aFirstDateTime(DateTime::EMPTY)
        , aLastDateTime(DateTime::EMPTY)
        , eDateMode(SvxRedlinDateMode::BEFORE)
        , bShowIt(false)
        , bIsDate(false)
        , bIsAuthor(false)
        , bIsComment(false)
        , bIsRange(false)
        , bEveryoneButMe(false)
        , bShowAccepted(false)
        , bShowRejected(false)
        , mbIsActionRange(false)
        , mnFirstAction(0)
        , mnLastAction(0)
    {
    }

    ScChangeViewSettings( const ScChangeViewSettings& r );

    ~ScChangeViewSettings();

    bool                ShowChanges() const {return bShowIt;}
    void                SetShowChanges(bool bFlag) {bShowIt=bFlag;}

    bool                HasDate() const {return bIsDate;}
    void                SetHasDate(bool bFlag) {bIsDate=bFlag;}

    void                SetTheDateMode(SvxRedlinDateMode eDatMod){ eDateMode=eDatMod; }
    SvxRedlinDateMode   GetTheDateMode() const { return eDateMode; }

    void                SetTheFirstDateTime(const DateTime& aDateTime) {aFirstDateTime=aDateTime;}
    const DateTime&     GetTheFirstDateTime()const {return aFirstDateTime;}

    void                SetTheLastDateTime(const DateTime& aDateTime) {aLastDateTime=aDateTime;}
    const DateTime&     GetTheLastDateTime()const {return aLastDateTime;}

    bool                HasAuthor() const {return bIsAuthor;}
    void                SetHasAuthor(bool bFlag) {bIsAuthor=bFlag;}

    OUString            GetTheAuthorToShow()const {return aAuthorToShow;}
    void                SetTheAuthorToShow(const OUString& aString){aAuthorToShow=aString;}

    bool                HasComment() const {return bIsComment;}
    void                SetHasComment(bool bFlag) {bIsComment=bFlag;}

    OUString            GetTheComment()const {return aComment;}
    void                SetTheComment(const OUString& aString);

    bool                IsValidComment(const OUString* pCommentStr) const;

    bool                IsEveryoneButMe() const {return bEveryoneButMe;}

    bool                HasRange() const {return bIsRange;}
    void                SetHasRange(bool bFlag) {bIsRange=bFlag;}

    const ScRangeList&  GetTheRangeList()const {return aRangeList;}
    void                SetTheRangeList(const ScRangeList& aRl){aRangeList=aRl;}

    bool                IsShowAccepted() const { return bShowAccepted; }
    void                SetShowAccepted( bool bVal ) { bShowAccepted = bVal; }

    bool                IsShowRejected() const { return bShowRejected; }
    void                SetShowRejected( bool bVal ) { bShowRejected = bVal; }

    ScChangeViewSettings&   operator=   ( const ScChangeViewSettings& r );

                        /// Adjust dates according to selected DateMode
    void                AdjustDateMode( const ScDocument& rDoc );

    bool                HasActionRange() const { return mbIsActionRange; }
    void                SetHasActionRange( bool bFlag = true ) { mbIsActionRange = bFlag; }
    void                GetTheActionRange( sal_uLong& nFirst, sal_uLong& nLast ) const { nFirst = mnFirstAction; nLast = mnLastAction; }
    void                SetTheActionRange( sal_uLong nFirst, sal_uLong nLast ) { mnFirstAction = nFirst; mnLastAction = nLast; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
