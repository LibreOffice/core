/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#pragma once

#include "scriptdocument.hxx"

class SbMethod;
class SbModule;
class SbxVariable;
class StarBASIC;
class SfxUInt16Item;
class SfxBindings;
class SfxDispatcher;
namespace weld { class Widget; class Window; }

namespace basctl
{
    void            Organize(weld::Window* pParent, sal_Int16 tabId);


    // help methods for the general use:
    SbMethod*       CreateMacro( SbModule* pModule, const OUString& rMacroName );
    void            RunMethod( SbMethod const * pMethod );

    StarBASIC*      FindBasic( const SbxVariable* pVar );
    void            StopBasic();
    long            HandleBasicError( StarBASIC const * pBasic );
    void            BasicStopped( bool* pbAppWindowDisabled = nullptr, bool* pbDispatcherLocked = nullptr, sal_uInt16* pnWaitCount = nullptr,
                            SfxUInt16Item** ppSWActionCount = nullptr, SfxUInt16Item** ppSWLockViewCount = nullptr );

    bool            IsValidSbxName( const OUString& rName );

    BasicManager*   FindBasicManager( StarBASIC const * pLib );

    SfxBindings*    GetBindingsPtr();

    SfxDispatcher*  GetDispatcher ();

    void            InvalidateDebuggerSlots();

    // libraries

    css::uno::Sequence< OUString > GetMergedLibraryNames(
        const css::uno::Reference< css::script::XLibraryContainer >& xModLibContainer,
        const css::uno::Reference< css::script::XLibraryContainer >& xDlgLibContainer );

    /** renames a module

        Will show an error message when renaming fails because the new name is already used.
    */
    bool            RenameModule(
        weld::Widget* pErrorParent, const ScriptDocument& rDocument,
        const OUString& rLibName, const OUString& rOldName, const OUString& rNewName );

    // new methods for macros

    OUString        ChooseMacro(weld::Window* pParent,
        const css::uno::Reference< css::frame::XModel >& rxLimitToDocument, const css::uno::Reference< css::frame::XFrame >& xDocFrame,
        bool bChooseOnly );
    inline OUString ChooseMacro(weld::Window* pParent, const css::uno::Reference<css::frame::XModel>& rLimitToDocument)
    { return ChooseMacro(pParent, rLimitToDocument, css::uno::Reference< css::frame::XFrame >(), false/*bChooseOnly*/); }

    /// @throws css::container::NoSuchElementException
    /// @throws css::uno::RuntimeException
    css::uno::Sequence< OUString > GetMethodNames(
        const ScriptDocument& rDocument, const OUString& rLibName, const OUString& rModName );

    bool            HasMethod(
        const ScriptDocument& rDocument, const OUString& rLibName, const OUString& rModName, const OUString& rMethName );

    // new methods for dialogs

    /** renames a dialog

        Will show an error message when renaming fails because the new name is already used.

        @throws css::container::ElementExistException
        @throws css::container::NoSuchElementException
        @throws css::uno::RuntimeException
    */
    bool            RenameDialog(weld::Widget* pErrorParent, const ScriptDocument& rDocument, const OUString& rLibName, const OUString& rOldName, const OUString& rNewName);

    bool            RemoveDialog( const ScriptDocument& rDocument, const OUString& rLibName, const OUString& rDlgName );

    void            MarkDocumentModified( const ScriptDocument& rDocument );

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
