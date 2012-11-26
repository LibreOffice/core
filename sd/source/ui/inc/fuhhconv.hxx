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



#ifndef _SD_FUHHCONV_HXX
#define _SD_FUHHCONV_HXX

#ifndef _SD_FUPOOR_HXX
#include "fupoor.hxx"
#endif

namespace sd {

class Outliner;


class FuHangulHanjaConversion : public FuPoor
{
 public:
    static FunctionReference Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq );

    void StartConversion( sal_Int16 nSourceLanguage,  sal_Int16 nTargetLanguage,
        const Font *pTargetFont, sal_Int32 nOptions, bool bIsInteractive );

    void StartChineseConversion();

    void ConvertStyles( sal_Int16 nTargetLanguage, const Font *pTargetFont );

    Outliner* GetOutliner() const { return pSdOutliner; }

 protected:
    ~FuHangulHanjaConversion();

    Outliner*   pSdOutliner;
    bool            bOwnOutliner;

private:
    FuHangulHanjaConversion (
        ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq );

};

#endif // _SD_FUHHCONV_HXX


} // end of namespace sd
