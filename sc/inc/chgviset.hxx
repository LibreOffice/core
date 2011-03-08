/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef SC_CHGVISET_HXX
#define SC_CHGVISET_HXX

#include <tools/datetime.hxx>
#include <tools/string.hxx>
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
    String              aAuthorToShow;
    String              aComment;
    ScRangeList         aRangeList;
    ScChgsDateMode      eDateMode;
    BOOL                bShowIt;
    BOOL                bIsDate;
    BOOL                bIsAuthor;
    BOOL                bIsComment;
    BOOL                bIsRange;
    BOOL                bEveryoneButMe;
    BOOL                bShowAccepted;
    BOOL                bShowRejected;
    bool                mbIsActionRange;
    ULONG               mnFirstAction;
    ULONG               mnLastAction;

public:

                        ScChangeViewSettings()
                        {
                            pCommentSearcher=NULL;
                            bIsDate=FALSE;
                            bIsAuthor=FALSE;
                            bIsRange=FALSE;
                            bIsComment=FALSE;
                            bShowIt=FALSE;
                            eDateMode=SCDM_DATE_BEFORE;
                            bEveryoneButMe=FALSE;
                            bShowAccepted=FALSE;
                            bShowRejected=FALSE;
                            mbIsActionRange = false;
                        }

                        ScChangeViewSettings( const ScChangeViewSettings& r );

                        ~ScChangeViewSettings();

    BOOL                ShowChanges() const {return bShowIt;}
    void                SetShowChanges(BOOL nFlag=TRUE){bShowIt=nFlag;}

    BOOL                HasDate() const {return bIsDate;}
    void                SetHasDate(BOOL nFlag=TRUE) {bIsDate=nFlag;}

    void                SetTheDateMode(ScChgsDateMode eDatMod){ eDateMode=eDatMod; }
    ScChgsDateMode      GetTheDateMode() const { return eDateMode; }

    void                SetTheFirstDateTime(const DateTime& aDateTime) {aFirstDateTime=aDateTime;}
    const DateTime&     GetTheFirstDateTime()const {return aFirstDateTime;}

    void                SetTheLastDateTime(const DateTime& aDateTime) {aLastDateTime=aDateTime;}
    const DateTime&     GetTheLastDateTime()const {return aLastDateTime;}


    BOOL                HasAuthor() const {return bIsAuthor;}
    void                SetHasAuthor(BOOL nFlag=TRUE) {bIsAuthor=nFlag;}

    String              GetTheAuthorToShow()const {return aAuthorToShow;}
    void                SetTheAuthorToShow(const String& aString){aAuthorToShow=aString;}

    BOOL                HasComment() const {return bIsComment;}
    void                SetHasComment(BOOL nFlag=TRUE) {bIsComment=nFlag;}

    String              GetTheComment()const {return aComment;}
    void                SetTheComment(const String& aString);

    BOOL                IsValidComment(const String* pCommentStr) const;

    BOOL                IsEveryoneButMe() const {return bEveryoneButMe;}
    void                SetEveryoneButMe(BOOL nFlag=TRUE) {bEveryoneButMe=nFlag;}


    BOOL                HasRange() const {return bIsRange;}
    void                SetHasRange(BOOL nFlag=TRUE) {bIsRange=nFlag;}

    const ScRangeList&  GetTheRangeList()const {return aRangeList;}
    void                SetTheRangeList(const ScRangeList& aRl){aRangeList=aRl;}

    BOOL                IsShowAccepted() const { return bShowAccepted; }
    void                SetShowAccepted( BOOL bVal ) { bShowAccepted = bVal; }

    BOOL                IsShowRejected() const { return bShowRejected; }
    void                SetShowRejected( BOOL bVal ) { bShowRejected = bVal; }

    ScChangeViewSettings&   operator=   ( const ScChangeViewSettings& r );

                        /// Adjust dates according to selected DateMode
    void                AdjustDateMode( const ScDocument& rDoc );

    bool                HasActionRange() const { return mbIsActionRange; }
    void                SetHasActionRange( bool nFlag = true ) { mbIsActionRange = nFlag; }
    void                GetTheActionRange( ULONG& nFirst, ULONG& nLast ) const { nFirst = mnFirstAction; nLast = mnLastAction; }
    void                SetTheActionRange( ULONG nFirst, ULONG nLast ) { mnFirstAction = nFirst; mnLastAction = nLast; }
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
