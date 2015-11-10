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
#ifndef INCLUDED_BASCTL_SOURCE_INC_BASOBJ_HXX
#define INCLUDED_BASCTL_SOURCE_INC_BASOBJ_HXX

#include "scriptdocument.hxx"

#include <svl/lstner.hxx>

class SbMethod;
class SbModule;
class SbxVariable;
class StarBASIC;
class SfxUInt16Item;
class SfxBindings;
class SfxDispatcher;
namespace vcl { class Window; }

namespace basctl
{
    void            Organize( sal_Int16 tabId );


    // help methods for the general use:
    SbMethod*       CreateMacro( SbModule* pModule, const OUString& rMacroName );
    void            RunMethod( SbMethod* pMethod );

    StarBASIC*      FindBasic( const SbxVariable* pVar );
    void            StopBasic();
    long            HandleBasicError( StarBASIC* pBasic );
    void            BasicStopped( bool* pbAppWindowDisabled = nullptr, bool* pbDispatcherLocked = nullptr, sal_uInt16* pnWaitCount = nullptr,
                            SfxUInt16Item** ppSWActionCount = nullptr, SfxUInt16Item** ppSWLockViewCount = nullptr );

    bool            IsValidSbxName( const OUString& rName );

    BasicManager*   FindBasicManager( StarBASIC* pLib );

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
        vcl::Window* pErrorParent, const ScriptDocument& rDocument,
        const OUString& rLibName, const OUString& rOldName, const OUString& rNewName );

    // new methods for macros

    OUString        ChooseMacro(
        const css::uno::Reference< css::frame::XModel >& rxLimitToDocument,
        bool bChooseOnly, const OUString& rMacroDesc );

    css::uno::Sequence< OUString > GetMethodNames(
        const ScriptDocument& rDocument, const OUString& rLibName, const OUString& rModName )
        throw (css::container::NoSuchElementException, css::uno::RuntimeException, std::exception);

    bool            HasMethod(
        const ScriptDocument& rDocument, const OUString& rLibName, const OUString& rModName, const OUString& rMethName );

    // new methods for dialogs

    /** renames a dialog

        Will show an error message when renaming fails because the new name is already used.
    */
    bool            RenameDialog(
        vcl::Window* pErrorParent, const ScriptDocument& rDocument, const OUString& rLibName, const OUString& rOldName, const OUString& rNewName )
        throw (css::container::ElementExistException, css::container::NoSuchElementException, css::uno::RuntimeException, std::exception);

    bool            RemoveDialog( const ScriptDocument& rDocument, const OUString& rLibName, const OUString& rDlgName );

    void            MarkDocumentModified( const ScriptDocument& rDocument );

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_INC_BASOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
