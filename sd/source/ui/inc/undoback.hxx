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



#ifndef _SD_UNDOBACK_HXX
#define _SD_UNDOBACK_HXX

#include "sdundo.hxx"

class SdDrawDocument;
class SdPage;
class SfxItemSet;

// -----------------------------
// - SdBackgroundObjUndoAction -
// -----------------------------

class SdBackgroundObjUndoAction : public SdUndoAction
{
private:

    SdPage&                 mrPage;
    SfxItemSet*             mpItemSet;

    void                    ImplRestoreBackgroundObj();

public:

                            SdBackgroundObjUndoAction(
                                SdDrawDocument& rDoc,
                                SdPage& rPage,
                                const SfxItemSet& rItenSet);
    virtual                 ~SdBackgroundObjUndoAction();

    virtual void            Undo();
    virtual void            Redo();

    virtual SdUndoAction*   Clone() const;
};

#endif // _SD_UNDOBACK_HXX
