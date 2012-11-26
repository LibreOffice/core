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


#ifndef _SVT_LISTENERITER_HXX
#define _SVT_LISTENERITER_HXX

#include "svl/svldllapi.h"
#include <typeinfo>

class SvtListener;
class SvtListenerBase;
class SvtBroadcaster;

//-------------------------------------------------------------------------

typedef bool (*ConvertToSvtListener)( const SvtListener* );
template<class T> bool _IsSvtListener(const SvtListener* pListener)
{
    return 0 != dynamic_cast<const T*>(pListener);
}

//-------------------------------------------------------------------------

class SVL_DLLPUBLIC SvtListenerIter
{
    friend class SvtListenerBase;

    SvtBroadcaster& rRoot;
    SvtListenerBase *pAkt, *pDelNext;

    // for the update of all iterator's, if a listener is added or removed
    // at the same time.
    static SvtListenerIter *pListenerIters;
    SvtListenerIter *pNxtIter;
    ConvertToSvtListener pSrchFunction;           // fuer First/Next - suche diesen Type

    SVL_DLLPRIVATE static void RemoveListener( SvtListenerBase& rDel,
                                               SvtListenerBase* pNext );

public:
    SvtListenerIter( SvtBroadcaster& );
    ~SvtListenerIter();

    const SvtBroadcaster& GetBroadcaster() const    { return rRoot; }
          SvtBroadcaster& GetBroadcaster()          { return rRoot; }

    SvtListener* GoNext();          // to the next
    SvtListener* GoPrev();          // to the previous

    SvtListener* GoStart();         // to the start of the list
    SvtListener* GoEnd();           // to the end of the list

    SvtListener* GoRoot();          // to the root
    SvtListener* GetCurr() const;   // returns the current

    int IsChanged() const       { return pDelNext != pAkt; }

    SvtListener* First( ConvertToSvtListener rConvert );
    SvtListener* Next();
};


#endif

