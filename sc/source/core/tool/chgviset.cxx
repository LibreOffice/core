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

#include <unotools/textsearch.hxx>

#include "chgviset.hxx"
#include "rechead.hxx"
#include "chgtrack.hxx"
#include "document.hxx"

ScChangeViewSettings::~ScChangeViewSettings()
{
}

ScChangeViewSettings::ScChangeViewSettings( const ScChangeViewSettings& r ):
    pCommentSearcher(nullptr),
    aFirstDateTime( DateTime::EMPTY ),
    aLastDateTime( DateTime::EMPTY )
{
    SetTheComment(r.aComment);

    aFirstDateTime  =r.aFirstDateTime;
    aLastDateTime   =r.aLastDateTime;
    aAuthorToShow   =r.aAuthorToShow;
    aRangeList      =r.aRangeList;
    eDateMode       =r.eDateMode;
    bShowIt         =r.bShowIt;
    bIsDate         =r.bIsDate;
    bIsAuthor       =r.bIsAuthor;
    bIsComment      =r.bIsComment;
    bIsRange        =r.bIsRange;
    bEveryoneButMe  =r.bEveryoneButMe;
    bShowAccepted   =r.bShowAccepted;
    bShowRejected   =r.bShowRejected;
    mbIsActionRange = r.mbIsActionRange;
    mnFirstAction   = r.mnFirstAction;
    mnLastAction    = r.mnLastAction;

}

ScChangeViewSettings& ScChangeViewSettings::operator=( const ScChangeViewSettings& r )
{
    pCommentSearcher = nullptr;
    SetTheComment(r.aComment);

    aFirstDateTime  =r.aFirstDateTime;
    aLastDateTime   =r.aLastDateTime;
    aAuthorToShow   =r.aAuthorToShow;
    aRangeList      =r.aRangeList;
    eDateMode       =r.eDateMode;
    bShowIt         =r.bShowIt;
    bIsDate         =r.bIsDate;
    bIsAuthor       =r.bIsAuthor;
    bIsComment      =r.bIsComment;
    bIsRange        =r.bIsRange;
    bEveryoneButMe  =r.bEveryoneButMe;
    bShowAccepted   =r.bShowAccepted;
    bShowRejected   =r.bShowRejected;
    mbIsActionRange = r.mbIsActionRange;
    mnFirstAction   = r.mnFirstAction;
    mnLastAction    = r.mnLastAction;

    return *this;
}

bool ScChangeViewSettings::IsValidComment(const OUString* pCommentStr) const
{
    bool bTheFlag = true;

    if(pCommentSearcher)
    {
        sal_Int32 nStartPos = 0;
        sal_Int32 nEndPos = pCommentStr->getLength();
        bTheFlag = pCommentSearcher->SearchForward(*pCommentStr, &nStartPos, &nEndPos);
    }
    return bTheFlag;
}

void ScChangeViewSettings::SetTheComment(const OUString& rString)
{
    aComment = rString;
    pCommentSearcher.reset();

    if(!rString.isEmpty())
    {
        utl::SearchParam aSearchParam( rString,
            utl::SearchParam::SearchType::Regexp,false );

        pCommentSearcher.reset( new utl::TextSearch( aSearchParam, *ScGlobal::pCharClass ) );
    }
}

void ScChangeViewSettings::AdjustDateMode( const ScDocument& rDoc )
{
    switch ( eDateMode )
    {   // corresponds with ScViewUtil::IsActionShown
        case SvxRedlinDateMode::EQUAL :
        case SvxRedlinDateMode::NOTEQUAL :
            aFirstDateTime.SetTime( 0 );
            aLastDateTime = aFirstDateTime;
            aLastDateTime.SetTime( 23595999 );
            break;
        case SvxRedlinDateMode::SAVE:
        {
            const ScChangeAction* pLast = nullptr;
            ScChangeTrack* pTrack = rDoc.GetChangeTrack();
            if ( pTrack )
            {
                pLast = pTrack->GetLastSaved();
                if ( pLast )
                {
                    aFirstDateTime = pLast->GetDateTime();

                    // Set the next minute as the start time and assume that
                    // the document isn't saved, reloaded, edited and filter set
                    // all together during the gap between those two times.
                    aFirstDateTime += tools::Time( 0, 1 );
                    aFirstDateTime.SetSec(0);
                    aFirstDateTime.SetNanoSec(0);
                }
            }
            if ( !pLast )
            {
                aFirstDateTime.SetDate( 18990101 );
                aFirstDateTime.SetTime( 0 );
            }
            aLastDateTime = Date( Date::SYSTEM );
            aLastDateTime.AddYears( 100 );
        }
        break;
        default:
        {
            // added to avoid warnings
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
