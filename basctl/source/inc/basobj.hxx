/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: basobj.hxx,v $
 * $Revision: 1.3 $
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
#include <svtools/lstner.hxx>

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

extern "C" {
    rtl_uString* basicide_choose_macro( void* pOnlyInDocument_AsXModel, BOOL bChooseOnly, rtl_uString* pMacroDesc );
    void basicide_macro_organizer( INT16 nTabId );
}

extern "C" {
    long basicide_handle_basic_error( void* pPtr );
}


class BasicIDE
{
protected:
    static BOOL         StringCompareLessThan( const String& rStr1, const String& rStr2 )
                            { return (rStr1.CompareIgnoreCaseToAscii( rStr2 ) == COMPARE_LESS); }
public:
                        BasicIDE();
                        ~BasicIDE();

    static SfxMacro*    CreateMacro();
    static void         Organize( INT16 tabId );

    static USHORT       GetBasicDialogCount();
    static void         IncBasicDialogCount();
    static void         DecBasicDialogCount();


    // Hilfsmethoden fuer den allg. Gebrauch:
    static SbMethod*    FindMacro( SbModule* pModule, const String& rMacroName );
    static SbMethod*    CreateMacro( SbModule* pModule, const String& rMacroName );
    static void         RunMethod( SbMethod* pMethod );

    static StarBASIC*   FindBasic( const SbxVariable* pVar );
    static void         StopBasic();
    static long         HandleBasicError( StarBASIC* pBasic );
    static void         BasicStopped( BOOL* pbAppWindowDisabled = 0, BOOL* pbDispatcherLocked = 0, USHORT* pnWaitCount = 0,
                            SfxUInt16Item** ppSWActionCount = 0, SfxUInt16Item** ppSWLockViewCount = 0 );

    static BOOL         IsValidSbxName( const String& rName );

    static BasicManager*        FindBasicManager( StarBASIC* pLib );

    static SfxBindings*         GetBindingsPtr();

    static void         InvalidateDebuggerSlots();

    // libraries

    static ::com::sun::star::uno::Sequence< ::rtl::OUString > GetMergedLibraryNames(
        const ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer >& xModLibContainer,
        const ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer >& xDlgLibContainer );

    // new methods for modules

    static ::rtl::OUString      GetModule(
        const ScriptDocument& rDocument, const String& rLibName, const String& rModName )
        throw( ::com::sun::star::container::NoSuchElementException );

    /** renames a module

        Will show an error message when renaming fails because the new name is already used.
    */
    static bool                 RenameModule(
        Window* pErrorParent, const ScriptDocument& rDocument,
        const String& rLibName, const String& rOldName, const String& rNewName );

    // new methods for macros

    static ::rtl::OUString      ChooseMacro( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rxLimitToDocument,
        BOOL bChooseOnly, const ::rtl::OUString& rMacroDesc );

    static ::com::sun::star::uno::Sequence< ::rtl::OUString > GetMethodNames(
        const ScriptDocument& rDocument, const String& rLibName, const String& rModName )
        throw( ::com::sun::star::container::NoSuchElementException );

    static BOOL                 HasMethod(
        const ScriptDocument& rDocument, const String& rLibName, const String& rModName, const String& rMethName );

    // new methods for dialogs

    static ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStreamProvider > GetDialog(
        const ScriptDocument& rDocument, const String& rLibName, const String& rDlgName )
        throw( ::com::sun::star::container::NoSuchElementException );

    /** renames a dialog

        Will show an error message when renaming fails because the new name is already used.
    */
    static bool                 RenameDialog(
        Window* pErrorParent, const ScriptDocument& rDocument, const String& rLibName, const String& rOldName, const String& rNewName )
        throw( ::com::sun::star::container::ElementExistException, ::com::sun::star::container::NoSuchElementException );

    static bool                 RemoveDialog(
        const ScriptDocument& rDocument, const String& rLibName, const String& rDlgName );

    static void                 MarkDocumentModified( const ScriptDocument& rDocument );
};

#endif  // _BASOBJ_HXX
