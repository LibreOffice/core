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


#ifndef _SFXLSTNER_HXX
#define _SFXLSTNER_HXX

#include "svl/svldllapi.h"
#include <svl/svarray.hxx>

class SfxBroadcaster;
class SfxHint;

#ifndef _SFX_LSTNER_CXX
typedef SvPtrarr SfxBroadcasterArr_Impl;
#endif

#define SFX_NOTIFY( rBC, rBCT, rHint, rHintT ) \
        Notify( rBC, rHint )

//-------------------------------------------------------------------------

class SVL_DLLPUBLIC SfxListener
{
    SfxBroadcasterArr_Impl aBCs;

private:
    const SfxListener&  operator=(const SfxListener &); // n.i., ist verboten

public:
                        SfxListener();
                        SfxListener( const SfxListener &rCopy );
    virtual             ~SfxListener();

    sal_Bool                StartListening( SfxBroadcaster& rBroadcaster, sal_Bool bPreventDups = sal_False );
    sal_Bool                EndListening( SfxBroadcaster& rBroadcaster, sal_Bool bAllDups = sal_False );
    void                EndListening( sal_uInt16 nNo );
    void                EndListeningAll();
    sal_Bool                IsListening( SfxBroadcaster& rBroadcaster ) const;

    sal_uInt16              GetBroadcasterCount() const
                        { return aBCs.Count(); }
    SfxBroadcaster*     GetBroadcasterJOE( sal_uInt16 nNo ) const
                        { return (SfxBroadcaster*) aBCs.GetObject(nNo); }

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

#ifndef _NOTIFY_HXX
    void RemoveBroadcaster_Impl( SfxBroadcaster& rBC );
#endif
};

#endif
