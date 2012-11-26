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



#ifndef _SD_SDUNDO_HXX
#define _SD_SDUNDO_HXX

#include <svl/undo.hxx>
#include "sddllapi.h"

class SdDrawDocument;

class SD_DLLPUBLIC SdUndoAction : public SfxUndoAction
{
public:
                            SdUndoAction(SdDrawDocument* pSdDrawDocument)
                                : mpDoc(pSdDrawDocument)  {}
    virtual                 ~SdUndoAction() {}

    void                    SetComment(String& rStr) { maComment = rStr; }
    virtual String          GetComment() const { return maComment; }
    virtual SdUndoAction*   Clone() const { return NULL; }

protected:
    SdDrawDocument* mpDoc;
    String maComment;
};

#endif     // _SD_SDUNDO_HXX
