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
#ifndef SC_CHGVISET_HXX
#define SC_CHGVISET_HXX

#include <tools/datetime.hxx>
#include "rangelst.hxx"
#include "scdllapi.h"

enum ScChgsDateMode{    SCDM_DATE_BEFORE=0,SCDM_DATE_SINCE=1,SCDM_DATE_EQUAL=2,
                        SCDM_DATE_NOTEQUAL=3,SCDM_DATE_BETWEEN=4, SCDM_DATE_SAVE=5,
                        SCDM_NO_DATEMODE=6};

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
    ScChgsDateMode      eDateMode;
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
        : pCommentSearcher(NULL)
        , aFirstDateTime(DateTime::EMPTY)
        , aLastDateTime(DateTime::EMPTY)
        , eDateMode(SCDM_DATE_BEFORE)
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
    void                SetShowChanges(bool nFlag) {bShowIt=nFlag;}

    bool                HasDate() const {return bIsDate;}
    void                SetHasDate(bool nFlag) {bIsDate=nFlag;}

    void                SetTheDateMode(ScChgsDateMode eDatMod){ eDateMode=eDatMod; }
    ScChgsDateMode      GetTheDateMode() const { return eDateMode; }

    void                SetTheFirstDateTime(const DateTime& aDateTime) {aFirstDateTime=aDateTime;}
    const DateTime&     GetTheFirstDateTime()const {return aFirstDateTime;}

    void                SetTheLastDateTime(const DateTime& aDateTime) {aLastDateTime=aDateTime;}
    const DateTime&     GetTheLastDateTime()const {return aLastDateTime;}


    bool                HasAuthor() const {return bIsAuthor;}
    void                SetHasAuthor(bool nFlag) {bIsAuthor=nFlag;}

    OUString            GetTheAuthorToShow()const {return aAuthorToShow;}
    void                SetTheAuthorToShow(const OUString& aString){aAuthorToShow=aString;}

    bool                HasComment() const {return bIsComment;}
    void                SetHasComment(bool nFlag) {bIsComment=nFlag;}

    OUString            GetTheComment()const {return aComment;}
    void                SetTheComment(const OUString& aString);

    bool                IsValidComment(const OUString* pCommentStr) const;

    bool                IsEveryoneButMe() const {return bEveryoneButMe;}
    void                SetEveryoneButMe(bool nFlag) {bEveryoneButMe=nFlag;}


    bool                HasRange() const {return bIsRange;}
    void                SetHasRange(bool nFlag) {bIsRange=nFlag;}

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
    void                SetHasActionRange( bool nFlag = true ) { mbIsActionRange = nFlag; }
    void                GetTheActionRange( sal_uLong& nFirst, sal_uLong& nLast ) const { nFirst = mnFirstAction; nLast = mnLastAction; }
    void                SetTheActionRange( sal_uLong nFirst, sal_uLong nLast ) { mnFirstAction = nFirst; mnLastAction = nLast; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
