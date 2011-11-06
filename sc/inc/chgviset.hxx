/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
    sal_Bool                bShowIt;
    sal_Bool                bIsDate;
    sal_Bool                bIsAuthor;
    sal_Bool                bIsComment;
    sal_Bool                bIsRange;
    sal_Bool                bEveryoneButMe;
    sal_Bool                bShowAccepted;
    sal_Bool                bShowRejected;
    bool                mbIsActionRange;
    sal_uLong               mnFirstAction;
    sal_uLong               mnLastAction;

public:

                        ScChangeViewSettings()
                        {
                            pCommentSearcher=NULL;
                            bIsDate=sal_False;
                            bIsAuthor=sal_False;
                            bIsRange=sal_False;
                            bIsComment=sal_False;
                            bShowIt=sal_False;
                            eDateMode=SCDM_DATE_BEFORE;
                            bEveryoneButMe=sal_False;
                            bShowAccepted=sal_False;
                            bShowRejected=sal_False;
                            mbIsActionRange = false;
                        }

                        ScChangeViewSettings( const ScChangeViewSettings& r );

                        ~ScChangeViewSettings();

    sal_Bool                ShowChanges() const {return bShowIt;}
    void                SetShowChanges(sal_Bool nFlag=sal_True){bShowIt=nFlag;}

    sal_Bool                HasDate() const {return bIsDate;}
    void                SetHasDate(sal_Bool nFlag=sal_True) {bIsDate=nFlag;}

    void                SetTheDateMode(ScChgsDateMode eDatMod){ eDateMode=eDatMod; }
    ScChgsDateMode      GetTheDateMode() const { return eDateMode; }

    void                SetTheFirstDateTime(const DateTime& aDateTime) {aFirstDateTime=aDateTime;}
    const DateTime&     GetTheFirstDateTime()const {return aFirstDateTime;}

    void                SetTheLastDateTime(const DateTime& aDateTime) {aLastDateTime=aDateTime;}
    const DateTime&     GetTheLastDateTime()const {return aLastDateTime;}


    sal_Bool                HasAuthor() const {return bIsAuthor;}
    void                SetHasAuthor(sal_Bool nFlag=sal_True) {bIsAuthor=nFlag;}

    String              GetTheAuthorToShow()const {return aAuthorToShow;}
    void                SetTheAuthorToShow(const String& aString){aAuthorToShow=aString;}

    sal_Bool                HasComment() const {return bIsComment;}
    void                SetHasComment(sal_Bool nFlag=sal_True) {bIsComment=nFlag;}

    String              GetTheComment()const {return aComment;}
    void                SetTheComment(const String& aString);

    sal_Bool                IsValidComment(const String* pCommentStr) const;

    sal_Bool                IsEveryoneButMe() const {return bEveryoneButMe;}
    void                SetEveryoneButMe(sal_Bool nFlag=sal_True) {bEveryoneButMe=nFlag;}


    sal_Bool                HasRange() const {return bIsRange;}
    void                SetHasRange(sal_Bool nFlag=sal_True) {bIsRange=nFlag;}

    const ScRangeList&  GetTheRangeList()const {return aRangeList;}
    void                SetTheRangeList(const ScRangeList& aRl){aRangeList=aRl;}

    sal_Bool                IsShowAccepted() const { return bShowAccepted; }
    void                SetShowAccepted( sal_Bool bVal ) { bShowAccepted = bVal; }

    sal_Bool                IsShowRejected() const { return bShowRejected; }
    void                SetShowRejected( sal_Bool bVal ) { bShowRejected = bVal; }

    ScChangeViewSettings&   operator=   ( const ScChangeViewSettings& r );

                        /// Adjust dates according to selected DateMode
    void                AdjustDateMode( const ScDocument& rDoc );

    bool                HasActionRange() const { return mbIsActionRange; }
    void                SetHasActionRange( bool nFlag = true ) { mbIsActionRange = nFlag; }
    void                GetTheActionRange( sal_uLong& nFirst, sal_uLong& nLast ) const { nFirst = mnFirstAction; nLast = mnLastAction; }
    void                SetTheActionRange( sal_uLong nFirst, sal_uLong nLast ) { mnFirstAction = nFirst; mnLastAction = nLast; }
};



#endif

