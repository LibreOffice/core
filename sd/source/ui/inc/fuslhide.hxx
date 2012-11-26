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



#ifndef SD_FU_SLIDE_HIDE_HXX
#define SD_FU_SLIDE_HIDE_HXX

#include "fuslid.hxx"

namespace sd {

/*************************************************************************
|*
|* selektierte Dias in der Show zeigen / nicht zeigen
|*
\************************************************************************/

class FuSlideHide
    : public FuSlide
{
public:
    static FunctionReference Create( SlideViewShell* pViewSh, ::sd::Window* pWin, SlideView* pView, SdDrawDocument* pDoc, SfxRequest& rReq );
    virtual void DoExecute( SfxRequest& rReq );

protected:
    FuSlideHide (
        ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);
    virtual ~FuSlideHide (void);
};

} // end of namespace sd

#endif

