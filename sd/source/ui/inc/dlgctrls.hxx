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




#ifndef SD_DLGCTRLS_HXX
#define SD_DLGCTRLS_HXX

#include "TransitionPreset.hxx"
#include <vcl/lstbox.hxx>

#ifndef _SD_SDRESID_HXX
#include "sdresid.hxx"
#endif
#include "fadedef.h"
#include "sddllapi.h"

/*************************************************************************
|*
|* FadeEffectLB
|*
\************************************************************************/

struct FadeEffectLBImpl;

class SD_DLLPUBLIC FadeEffectLB : public ListBox
{
public:
                                FadeEffectLB( Window* pParent, SdResId Id );
                                ~FadeEffectLB();
    virtual void                Fill();

/*  void                        selectEffectFromPage( SdPage* pPage ); */
    void                        applySelected( SdPage* pSlide ) const;

    FadeEffectLBImpl*           mpImpl;
};

#endif // SD_DLGCTRLS_HXX
