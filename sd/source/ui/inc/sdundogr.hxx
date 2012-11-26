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



#ifndef _SD_SDUNDOGR_HXX
#define _SD_SDUNDOGR_HXX

#include <tools/contnr.hxx>
#include "sdundo.hxx"
#include "sddllapi.h"

class SD_DLLPUBLIC SdUndoGroup : public SdUndoAction
{
    Container      aCtn;
public:
                   SdUndoGroup(SdDrawDocument* pSdDrawDocument)
                              : SdUndoAction(pSdDrawDocument),
                                aCtn(16, 16, 16) {}
    virtual       ~SdUndoGroup();

    virtual sal_Bool   Merge( SfxUndoAction* pNextAction );

    virtual void   Undo();
    virtual void   Redo();

    void           AddAction(SdUndoAction* pAction);
    sal_uLong          Count() const { return aCtn.Count(); }

};

#endif     // _SD_SDUNDOGR_HXX
