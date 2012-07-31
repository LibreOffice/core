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
#ifndef _BASOBJ_HXX
#define _BASOBJ_HXX

#include "scriptdocument.hxx"

#include <svl/lstner.hxx>

class SbMethod;
class SbModule;
class SbxVariable;
class StarBASIC;
class String;
class SfxUInt16Item;
class SfxBindings;
class Window;

namespace BasicIDE
{
    void            Organize( sal_Int16 tabId );


    // help methods for the general use:
    SbMethod*   CreateMacro( SbModule* pModule, const String& rMacroName );
    void            RunMethod( SbMethod* pMethod );

    StarBASIC*  FindBasic( const SbxVariable* pVar );
    void            StopBasic();
    long         HandleBasicError( StarBASIC* pBasic );
    void            BasicStopped( bool* pbAppWindowDisabled = 0, bool* pbDispatcherLocked = 0, sal_uInt16* pnWaitCount = 0,
                            SfxUInt16Item** ppSWActionCount = 0, SfxUInt16Item** ppSWLockViewCount = 0 );

    sal_Bool            IsValidSbxName( const String& rName );

    BasicManager*       FindBasicManager( StarBASIC* pLib );

    SfxBindings*        GetBindingsPtr();

    void            InvalidateDebuggerSlots();

    // libraries

    ::com::sun::star::uno::Sequence< ::rtl::OUString > GetMergedLibraryNames(
        const ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer >& xModLibContainer,
        const ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer >& xDlgLibContainer );

    // new methods for modules

    ::rtl::OUString     GetModule(
        const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, const ::rtl::OUString& rModName )
        throw( ::com::sun::star::container::NoSuchElementException );

    /** renames a module

        Will show an error message when renaming fails because the new name is already used.
    */
    bool                 RenameModule(
        Window* pErrorParent, const ScriptDocument& rDocument,
        const ::rtl::OUString& rLibName, const ::rtl::OUString& rOldName, const ::rtl::OUString& rNewName );

    // new methods for macros

    ::rtl::OUString     ChooseMacro( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rxLimitToDocument,
        sal_Bool bChooseOnly, const ::rtl::OUString& rMacroDesc );

    ::com::sun::star::uno::Sequence< ::rtl::OUString > GetMethodNames(
        const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, const ::rtl::OUString& rModName )
        throw( ::com::sun::star::container::NoSuchElementException );

    bool                    HasMethod(
        const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, const ::rtl::OUString& rModName, const ::rtl::OUString& rMethName );

    // new methods for dialogs

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStreamProvider > GetDialog(
        const ScriptDocument& rDocument, const String& rLibName, const String& rDlgName )
        throw( ::com::sun::star::container::NoSuchElementException );

    /** renames a dialog

        Will show an error message when renaming fails because the new name is already used.
    */
    bool                 RenameDialog(
        Window* pErrorParent, const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, const ::rtl::OUString& rOldName, const ::rtl::OUString& rNewName )
        throw( ::com::sun::star::container::ElementExistException, ::com::sun::star::container::NoSuchElementException );

    bool                 RemoveDialog(
        const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, const ::rtl::OUString& rDlgName );

    void                MarkDocumentModified( const ScriptDocument& rDocument );
}

#endif  // _BASOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
