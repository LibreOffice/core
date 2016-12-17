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



#ifndef _AVMEDIA_MEDIACHILD_HXX
#define _AVMEDIA_MEDIACHILD_HXX

#include <sfx2/ctrlitem.hxx>
#include <sfx2/dockwin.hxx>
#include <tools/urlobj.hxx>

// -----------
// - Defines -
// -----------

#define AVMEDIA_MEDIAWINDOW()                                                                                           \
(static_cast< ::avmedia::MediaFloater* >( (                                                                             \
SfxViewFrame::Current() && SfxViewFrame::Current()->GetChildWindow(::avmedia::MediaPlayer::GetChildWindowId())) ?   \
SfxViewFrame::Current()->GetChildWindow(::avmedia::MediaPlayer::GetChildWindowId())->GetWindow() :              \
NULL))

namespace avmedia
{

// ---------------
// - MediaPlayer -
// ---------------

class MediaPlayer : public SfxChildWindow
{
public:
                        MediaPlayer( Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo* );
                        ~MediaPlayer();

                        SFX_DECL_CHILDWINDOW( MediaPlayer );
};

// ----------------
// - MediaFloater -
// ----------------

class MediaWindow;

class MediaFloater : public SfxDockingWindow
{
public:

                            MediaFloater( SfxBindings* pBindings, SfxChildWindow* pCW, Window* pParent );
                            ~MediaFloater();

    void                    setURL( const ::rtl::OUString& rURL, bool bPlayImmediately );
    const ::rtl::OUString&  getURL() const;

    void                    dispatchCurrentURL();

protected:

    virtual void            Resize();
    virtual void            ToggleFloatingMode();

private:

    MediaWindow*            mpMediaWindow;
    Size                    maLastSize;
    long                    mnDummy1;
    long                    mnDummy2;

    void                    implInit();
};

}

#endif
