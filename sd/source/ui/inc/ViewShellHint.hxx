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



#ifndef SD_VIEW_SHELL_HINT_HXX
#define SD_VIEW_SHELL_HINT_HXX

#include <svl/hint.hxx>

namespace sd {

/** Local derivation of the SfxHint class that defines some hint ids that
    are used by the ViewShell class and its descendants.
*/
class ViewShellHint
    : public SfxHint
{
public:
    enum HintId {
        // Indicate that a page resize is about to begin.
        HINT_PAGE_RESIZE_START,
        // Indicate that a page resize has been completed.
        HINT_PAGE_RESIZE_END,
        // Indicate that an edit mode change is about to begin.
        HINT_CHANGE_EDIT_MODE_START,
        // Indicate that an edit mode change has been completed.
        HINT_CHANGE_EDIT_MODE_END,

        HINT_COMPLEX_MODEL_CHANGE_START,
        HINT_COMPLEX_MODEL_CHANGE_END
    };

    ViewShellHint (HintId nHintId);

    HintId GetHintId (void) const;

private:
    HintId meHintId;
};

} // end of namespace sd

#endif
