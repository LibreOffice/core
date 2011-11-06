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



#include <vcl.hxx>

#include <sal/types.h>
#include <vcl/button.hxx>

static PushButton* get_button (Dialog const* dialog, sal_uInt32 type)
{
    Window* child = dialog->GetWindow (WINDOW_FIRSTCHILD);
    while (child)
    {
        if (child->GetType () == type)
            return static_cast <PushButton*> (child);
        child = child->GetWindow (WINDOW_NEXT);
    }

    return 0;
}

#define IMPLEMENT_CLOSING_DIALOG(cls)\
    Closing##cls::Closing##cls (Window* parent, WinBits bits)\
    : cls (parent, bits)\
        , mClosing (false)\
    {\
    }\
    sal_Bool Closing##cls::Close ()\
    {\
        if (mClosing)\
            EndDialog (false);\
        else if (PushButton *cancel = get_button (this, WINDOW_CANCELBUTTON))\
            cancel->Click ();\
        else if (PushButton *ok = get_button (this, WINDOW_OKBUTTON))\
            ok->Click ();\
        mClosing = true;\
        return false;\
    }

IMPLEMENT_CLOSING_DIALOG (Dialog);
IMPLEMENT_CLOSING_DIALOG (ModelessDialog);
IMPLEMENT_CLOSING_DIALOG (ModalDialog);
