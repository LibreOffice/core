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



#ifndef _SV_ACCMGR_HXX
#define _SV_ACCMGR_HXX

#include <vcl/sv.h>

class ImplAccelList;
class Accelerator;
class KeyCode;

// --------------------
// - ImplAccelManager -
// --------------------

class ImplAccelManager
{
private:
    ImplAccelList*      mpAccelList;
    ImplAccelList*      mpSequenceList;

public:
                        ImplAccelManager()
                        {
                            mpAccelList    = NULL;
                            mpSequenceList = NULL;
                        }
                        ~ImplAccelManager();

    sal_Bool                InsertAccel( Accelerator* pAccel );
    void                RemoveAccel( Accelerator* pAccel );

    void                EndSequence( sal_Bool bCancel = sal_False );
    void                FlushAccel() { EndSequence( sal_True ); }

    sal_Bool                IsAccelKey( const KeyCode& rKeyCode, sal_uInt16 nRepeat );
};

#endif  // _SV_ACCMGR_HXX
