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



#ifndef _SD_UNPRLOUT_HXX
#define _SD_UNPRLOUT_HXX

#include "sdundo.hxx"
#include "pres.hxx"                 // enum AutoLayout

class SdDrawDocument;
class SdPage;

class SdPresentationLayoutUndoAction : public SdUndoAction
{
    String          aOldLayoutName;
    String          aNewLayoutName;
    AutoLayout      eOldAutoLayout;
    AutoLayout      eNewAutoLayout;
    bool            bSetAutoLayout;     // true: AutoLayout aendern
    SdPage*         pPage;
    String          aComment;

public:
    SdPresentationLayoutUndoAction(SdDrawDocument* pTheDoc,
                         String          aTheOldLayoutName,
                         String          aTheNewLayoutName,
                         AutoLayout      eTheOldAutoLayout,
                         AutoLayout      eTheNewAutoLayout,
                         bool            bSet,
                         SdPage*         pThePage);

    virtual ~SdPresentationLayoutUndoAction();
    virtual void Undo();
    virtual void Redo();

    virtual String GetComment() const;
};

#endif      // _SD_UNPRLOUT_HXX

