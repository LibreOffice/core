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


#ifndef _SFXBRDCST_HXX
#define _SFXBRDCST_HXX

#include "svl/svldllapi.h"
#include <svl/svarray.hxx>

class SfxListener;
class SfxHint;

#ifndef _SFX_BRDCST_CXX
typedef SvPtrarr SfxListenerArr_Impl;
#endif

//-------------------------------------------------------------------------

class SVL_DLLPUBLIC SfxBroadcaster
{
friend class SfxListener;

    SfxListenerArr_Impl     aListeners;

private:
    sal_Bool         AddListener( SfxListener& rListener );
    void                    RemoveListener( SfxListener& rListener );
    const SfxBroadcaster&   operator=(const SfxBroadcaster &); // verboten

protected:
    void                    Forward(SfxBroadcaster& rBC, const SfxHint& rHint);
    virtual void            ListenersGone();

public:
                            SfxBroadcaster();
                            SfxBroadcaster( const SfxBroadcaster &rBC );
    virtual                 ~SfxBroadcaster();

    void                    Broadcast( const SfxHint &rHint );
    void                    BroadcastDelayed( const SfxHint& rHint );
    void                    BroadcastInIdle( const SfxHint& rHint );

    sal_Bool                    HasListeners() const;
    sal_uInt16                  GetListenerCount() const { return aListeners.Count(); }
    SfxListener*            GetListener( sal_uInt16 nNo ) const
                            { return (SfxListener*) aListeners[nNo]; }
};

#endif
