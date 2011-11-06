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


#ifndef _SVT_LISTENERBASE_HXX
#define _SVT_LISTENERBASE_HXX

class SvtBroadcaster;
class SvtListener;

class SvtListenerBase
{
    SvtListenerBase *pNext;
    SvtListenerBase *pLeft, *pRight;
    SvtBroadcaster *pBroadcaster;
    SvtListener *pListener;

public:

    SvtListenerBase( SvtListener& rLst, SvtBroadcaster& rBroadcaster );
    ~SvtListenerBase();

    SvtListenerBase* GetNext() const        { return pNext; }
    void SetNext( SvtListenerBase* p )      { pNext = p; }

    SvtBroadcaster* GetBroadcaster() const  { return pBroadcaster; }
    SvtListener* GetListener() const        { return pListener; }

    SvtListenerBase* GetLeft() const        { return pLeft; }
    SvtListenerBase* GetRight() const       { return pRight; }
};


#endif

