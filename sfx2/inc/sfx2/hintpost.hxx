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


#ifndef _SFXHINTPOST_HXX
#define _SFXHINTPOST_HXX

#include <sfx2/genlink.hxx>
#include <tools/ref.hxx>

//===================================================================

class SfxHint;

//-------------------------------------------------------------------

class SfxHintPoster: public SvRefBase

/*  [Beschreibung]

    Mit Instanzen dieser Klasse k"onnen eindeutige Events per PostUserEvent
    "uber die StarView-Application verschickt werden. Wenn das User-Event
    ausgel"ost wird, wird der Handler <Event()> gerufen, dessen
    Basisimplementierung den mit <SetEventHdl()> angegbenen Link ruft.

    Die Instanz wird via Ref-Count mindestens solange gehalten, wie
    ein ggf. abgeschicktes Event noch nicht angekommen ist. Sollte das
    Ziel vorher sterben, ist die Verbindung zuvor mit 'SetEventHdl(GenLink())'
    zu kappen.
*/

{
    sal_uIntPtr         nId;
    GenLink         aLink;

private:
    void            RegisterEvent();
//#if 0 // _SOLAR__PRIVATE
                    DECL_LINK( DoEvent_Impl, SfxHint * );
//#endif

protected:
    virtual         ~SfxHintPoster();
    virtual void    Event( SfxHint* pPostedHint );

public:
                    SfxHintPoster();
                    SfxHintPoster( const GenLink& rLink );

    void            Post( SfxHint* pHint = 0 );
    void            SetEventHdl( const GenLink& rLink );
};

//-------------------------------------------------------------------

SV_DECL_IMPL_REF(SfxHintPoster);

#endif
