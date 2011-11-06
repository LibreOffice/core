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


#ifndef _SDRHHCWRAP_HXX_
#define _SDRHHCWRAP_HXX_

#include <svx/svdoutl.hxx>

class SwView;
class SdrTextObj;
class OutlinerView;
class SdrObjListIter;

//////////////////////////////////////////////////////////////////////

class SdrHHCWrapper : public SdrOutliner
{
    // modified version of SdrSpeller

    SwView*             pView;
    SdrTextObj*         pTextObj;
    OutlinerView*       pOutlView;
    sal_Int32           nOptions;
    sal_uInt16          nDocIndex;
    LanguageType        nSourceLang;
    LanguageType        nTargetLang;
    const Font*         pTargetFont;
    sal_Bool            bIsInteractive;

public:
    SdrHHCWrapper( SwView* pVw,
                   LanguageType nSourceLanguage, LanguageType nTargetLanguage,
                   const Font* pTargetFnt,
                   sal_Int32 nConvOptions, sal_Bool bInteractive );

    virtual ~SdrHHCWrapper();

    virtual sal_Bool ConvertNextDocument();
    void    StartTextConversion();
};

//////////////////////////////////////////////////////////////////////

#endif

