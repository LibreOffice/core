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


#ifndef SC_IANYREFDIALOG_HXX_INCLUDED
#define SC_IANYREFDIALOG_HXX_INCLUDED

#include <formula/IControlReferenceHandler.hxx>

class ScRange;
class ScDocument;
class ScTabViewShell;
class SfxObjectShell;
namespace formula
{
    class RefEdit;
    class RefButton;
}
class SAL_NO_VTABLE IAnyRefDialog : public formula::IControlReferenceHandler
{
public:
    virtual ~IAnyRefDialog(){}

    //virtual void ShowReference(const String& _sRef) = 0;
    //virtual void HideReference( sal_Bool bDoneRefMode = sal_True ) = 0;
    //virtual void ReleaseFocus( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL ) = 0;
    //virtual void ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL ) = 0;


    virtual void SetReference( const ScRange& rRef, ScDocument* pDoc ) = 0;
    virtual void RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL ) = 0;
    virtual void RefInputDone( sal_Bool bForced = sal_False ) = 0;
    virtual sal_Bool IsTableLocked() const = 0;
    virtual sal_Bool IsRefInputMode() const = 0;

    virtual sal_Bool IsDocAllowed( SfxObjectShell* pDocSh ) const = 0;
    virtual void AddRefEntry() = 0;
    virtual void SetActive() = 0;
    virtual void ViewShellChanged( ScTabViewShell* pScViewShell ) = 0;
};

#endif // SC_IANYREFDIALOG_HXX_INCLUDED
