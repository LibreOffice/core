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



#ifndef SC_WARNBOX_HXX
#define SC_WARNBOX_HXX

#include <vcl/msgbox.hxx>


// ============================================================================

/** Message box with warning image and "Do not show again" checkbox. */
class ScCbWarningBox : public WarningBox
{
public:
    /** @param rMsgStr  Resource ID for the message text.
        @param bDefYes  true = "Yes" focused, false = "No" focused. */
                                ScCbWarningBox( Window* pParent, const String& rMsgStr, bool bDefYes = true );

    /** Opens dialog if IsDialogEnabled() returns true.
        @descr  If after executing the dialog the checkbox "Do not show again" is set,
                the method DisableDialog() will be called. */
    virtual sal_Int16           Execute();

    /** Called before executing the dialog. If this method returns false, the dialog will not be opened. */
    virtual bool                IsDialogEnabled();
    /** Called, when dialog is exited and the option "Do not show again" is set. */
    virtual void                DisableDialog();
};


// ----------------------------------------------------------------------------

/** Warning box for "Replace cell contents?". */
class ScReplaceWarnBox : public ScCbWarningBox
{
public:
                                ScReplaceWarnBox( Window* pParent );

    /** Reads the configuration key "ReplaceCellsWarning". */
    virtual bool                IsDialogEnabled();
    /** Sets the configuration key "ReplaceCellsWarning" to false. */
    virtual void                DisableDialog();
};


// ============================================================================

#endif

