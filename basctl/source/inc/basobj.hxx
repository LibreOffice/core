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
#ifndef _BASOBJ_HXX
#define _BASOBJ_HXX

#include "scriptdocument.hxx"
#include <tools/string.hxx>
#include <svl/lstner.hxx>

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAMPROVIDER_HXX_
#include <com/sun/star/io/XInputStreamProvider.hpp>
#endif
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/frame/XModel.hpp>

class SfxMacro;
class SbMethod;
class SbModule;
class SbxObject;
class SbxVariable;
class StarBASIC;
class BasicManager;
class SfxUInt16Item;
class SfxBindings;
class Window;
struct BasicIDE_Impl;

namespace BasicIDE
{
    void            Organize( sal_Int16 tabId );


    // Hilfsmethoden fuer den allg. Gebrauch:
    SbMethod*   CreateMacro( SbModule* pModule, const String& rMacroName );
    void            RunMethod( SbMethod* pMethod );

    StarBASIC*  FindBasic( const SbxVariable* pVar );
    void            StopBasic();
    long         HandleBasicError( StarBASIC* pBasic );
    void            BasicStopped( sal_Bool* pbAppWindowDisabled = 0, sal_Bool* pbDispatcherLocked = 0, sal_uInt16* pnWaitCount = 0,
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
        const ScriptDocument& rDocument, const String& rLibName, const String& rModName )
        throw( ::com::sun::star::container::NoSuchElementException );

    /** renames a module

        Will show an error message when renaming fails because the new name is already used.
    */
    bool                 RenameModule(
        Window* pErrorParent, const ScriptDocument& rDocument,
        const String& rLibName, const String& rOldName, const String& rNewName );

    // new methods for macros

    ::rtl::OUString     ChooseMacro( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rxLimitToDocument,
        sal_Bool bChooseOnly, const ::rtl::OUString& rMacroDesc );

    ::com::sun::star::uno::Sequence< ::rtl::OUString > GetMethodNames(
        const ScriptDocument& rDocument, const String& rLibName, const String& rModName )
        throw( ::com::sun::star::container::NoSuchElementException );

    sal_Bool                    HasMethod(
        const ScriptDocument& rDocument, const String& rLibName, const String& rModName, const String& rMethName );

    // new methods for dialogs

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStreamProvider > GetDialog(
        const ScriptDocument& rDocument, const String& rLibName, const String& rDlgName )
        throw( ::com::sun::star::container::NoSuchElementException );

    /** renames a dialog

        Will show an error message when renaming fails because the new name is already used.
    */
    bool                 RenameDialog(
        Window* pErrorParent, const ScriptDocument& rDocument, const String& rLibName, const String& rOldName, const String& rNewName )
        throw( ::com::sun::star::container::ElementExistException, ::com::sun::star::container::NoSuchElementException );

    bool                 RemoveDialog(
        const ScriptDocument& rDocument, const String& rLibName, const String& rDlgName );

    void                MarkDocumentModified( const ScriptDocument& rDocument );
}

#endif  // _BASOBJ_HXX
