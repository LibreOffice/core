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



#ifndef _SD_UNDOLAYER_HXX
#define _SD_UNDOLAYER_HXX

#include "sdundo.hxx"

class SdDrawDocument;
class SdrLayer;

/************************************************************************/

class SdLayerModifyUndoAction : public SdUndoAction
{

public:
    SdLayerModifyUndoAction( SdDrawDocument* _pDoc, SdrLayer* pLayer,
    const String& rOldLayerName, const String& rOldLayerTitle, const String& rOldLayerDesc, bool bOldIsVisible, bool bOldIsLocked, bool bOldIsPrintable,
    const String& rNewLayerName, const String& rNewLayerTitle, const String& rNewLayerDesc, bool bNewIsVisible, bool bNewIsLocked, bool bNewIsPrintable );

    virtual void Undo();
    virtual void Redo();

private:
    SdrLayer* mpLayer;
    String maOldLayerName;
    String maOldLayerTitle;
    String maOldLayerDesc;
    bool mbOldIsVisible;
    bool mbOldIsLocked;
    bool mbOldIsPrintable;
    String maNewLayerName;
    String maNewLayerTitle;
    String maNewLayerDesc;
    bool mbNewIsVisible;
    bool mbNewIsLocked;
    bool mbNewIsPrintable;
};

#endif      // _SD_UNDOLAYER_HXX

