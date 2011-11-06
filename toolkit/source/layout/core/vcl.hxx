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



#ifndef LAYOUT_CORE_VCL_HXX
#define LAYOUT_CORE_VCL_HXX

#include <vcl/dialog.hxx>

#define DECLARE_CLOSING_DIALOG(cls)\
    class Closing##cls : public cls\
    {\
    public:\
        bool mClosing;\
        Closing##cls (Window* parent, WinBits bits);\
        virtual sal_Bool Close ();\
    }

DECLARE_CLOSING_DIALOG (Dialog);
DECLARE_CLOSING_DIALOG (ModalDialog);
DECLARE_CLOSING_DIALOG (ModelessDialog);

#undef DECLARE_CLOSING_DIALOG

#endif /* LAYOUT_CORE_VCL_HXX */
