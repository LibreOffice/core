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



#ifndef _SD_UNDOHEADERFOOTER_HXX
#define _SD_UNDOHEADERFOOTER_HXX

#include <tools/gen.hxx>
#include "sdundo.hxx"
#include "sdpage.hxx"
#include "sddllapi.h"

class SdDrawDocument;

/************************************************************************/

class SD_DLLPUBLIC SdHeaderFooterUndoAction : public SdUndoAction
{
    SdPage*     mpPage;

    const sd::HeaderFooterSettings  maOldSettings;
    const sd::HeaderFooterSettings  maNewSettings;

public:
    SdHeaderFooterUndoAction( SdDrawDocument* pDoc, SdPage* pPage, const sd::HeaderFooterSettings& rNewSettings );
    virtual ~SdHeaderFooterUndoAction();

    virtual void Undo();
    virtual void Redo();
};

#endif      // _SD_UNDOHEADERFOOTER_HXX

