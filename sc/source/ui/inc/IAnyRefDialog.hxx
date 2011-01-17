/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
