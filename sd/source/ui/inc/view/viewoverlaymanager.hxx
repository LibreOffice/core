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



#ifndef _SD_VIEWOVERLAYMANAGER_HXX
#define _SD_VIEWOVERLAYMANAGER_HXX

#include <vcl/image.hxx>
#include <svl/lstner.hxx>
#include "EventMultiplexer.hxx"
#include "ViewShellBase.hxx"
#include "res_bmp.hrc"

namespace sd
{

typedef std::vector< rtl::Reference< SmartTag > > ViewTagVector;

class ViewOverlayManager : public SfxListener
{
public:
    ViewOverlayManager( ViewShellBase& rViewShellBase );
    virtual ~ViewOverlayManager();

    void onZoomChanged();
    void UpdateTags();

    DECL_LINK(EventMultiplexerListener, tools::EventMultiplexerEvent*);
    DECL_LINK(UpdateTagsHdl, void *);

    bool CreateTags();
    bool DisposeTags();

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

private:
    ViewShellBase& mrBase;
    sal_uLong mnUpdateTagsEvent;

    ViewTagVector   maTagVector;
};

}

#endif // _SD_VIEWOVERLAYMANAGER_HXX
