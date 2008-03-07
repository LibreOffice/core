/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chgviset.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 12:15:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef SC_CHGVISET_HXX
#define SC_CHGVISET_HXX

#ifndef _DATETIME_HXX //autogen
#include <tools/datetime.hxx>
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"
#endif

enum ScChgsDateMode{    SCDM_DATE_BEFORE=0,SCDM_DATE_SINCE=1,SCDM_DATE_EQUAL=2,
                        SCDM_DATE_NOTEQUAL=3,SCDM_DATE_BETWEEN=4, SCDM_DATE_SAVE=5,
                        SCDM_NO_DATEMODE=6};

namespace utl {
    class TextSearch;
}

class ScDocument;

class ScChangeViewSettings
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


    void                Load( SvStream& rStream, USHORT nVer );
    void                Store( SvStream& rStream ) const;

    ScChangeViewSettings&   operator=   ( const ScChangeViewSettings& r );

                        /// Adjust dates according to selected DateMode
    void                AdjustDateMode( const ScDocument& rDoc );

    bool                HasActionRange() const { return mbIsActionRange; }
    void                SetHasActionRange( bool nFlag = true ) { mbIsActionRange = nFlag; }
    void                GetTheActionRange( ULONG& nFirst, ULONG& nLast ) const { nFirst = mnFirstAction; nLast = mnLastAction; }
    void                SetTheActionRange( ULONG nFirst, ULONG nLast ) { mnFirstAction = nFirst; mnLastAction = nLast; }
};



#endif

