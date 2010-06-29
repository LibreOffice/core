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

#ifndef OOX_OLE_VBAHELPER_HXX
#define OOX_OLE_VBAHELPER_HXX

#include "oox/helper/binarystreambase.hxx"

namespace com { namespace sun { namespace star {
    namespace container { class XNameContainer; }
    namespace document { class XEventsSupplier; }
} } }

namespace oox { class BinaryInputStream; }

namespace oox {
namespace ole {

// Directory stream record identifiers ========================================

const sal_uInt16 VBA_ID_MODULECOOKIE            = 0x002C;
const sal_uInt16 VBA_ID_MODULEDOCSTRING         = 0x001C;
const sal_uInt16 VBA_ID_MODULEDOCSTRINGUNICODE  = 0x0048;
const sal_uInt16 VBA_ID_MODULEEND               = 0x002B;
const sal_uInt16 VBA_ID_MODULEHELPCONTEXT       = 0x001E;
const sal_uInt16 VBA_ID_MODULENAME              = 0x0019;
const sal_uInt16 VBA_ID_MODULENAMEUNICODE       = 0x0047;
const sal_uInt16 VBA_ID_MODULEOFFSET            = 0x0031;
const sal_uInt16 VBA_ID_MODULEPRIVATE           = 0x0028;
const sal_uInt16 VBA_ID_MODULEREADONLY          = 0x0025;
const sal_uInt16 VBA_ID_MODULESTREAMNAME        = 0x001A;
const sal_uInt16 VBA_ID_MODULESTREAMNAMEUNICODE = 0x0032;
const sal_uInt16 VBA_ID_MODULETYPEDOCUMENT      = 0x0022;
const sal_uInt16 VBA_ID_MODULETYPEPROCEDURAL    = 0x0021;
const sal_uInt16 VBA_ID_PROJECTCODEPAGE         = 0x0003;
const sal_uInt16 VBA_ID_PROJECTEND              = 0x0010;
const sal_uInt16 VBA_ID_PROJECTMODULES          = 0x000F;
const sal_uInt16 VBA_ID_PROJECTVERSION          = 0x0009;

// ============================================================================

/** Static helper functions for the VBA filters. */
class VbaHelper
{
public:
    /** Returns the full Basic script URL from a VBA module and macro name.
        The script is assumed to be in a document library. */
    static ::rtl::OUString getBasicScriptUrl(
                            const ::rtl::OUString& rLibraryName,
                            const ::rtl::OUString& rModuleName,
                            const ::rtl::OUString& rMacroName );

    /** Reads the next record from the VBA directory stream 'dir'.

        @param rnRecId  (out parameter) The record identifier of the new record.
        @param rRecData  (out parameter) The contents of the new record.
        @param rInStrm  The 'dir' stream.

        @return  True = next record successfully read. False on any error, or
            if the stream is EOF.
     */
    static bool         readDirRecord(
                            sal_uInt16& rnRecId,
                            StreamDataSequence& rRecData,
                            BinaryInputStream& rInStrm );

    /** Extracts a key/value pair from a string separated by an equality sign.

        @param rKey  (out parameter) The key before the separator.
        @param rValue  (out parameter) The value following the separator.
        @param rCodeLine  The source key/value pair.

        @return  True = Equality sign separator found, and the returned key and
            value are not empty. False otherwise.
     */
    static bool         extractKeyValue(
                            ::rtl::OUString& rKey,
                            ::rtl::OUString& rValue,
                            const ::rtl::OUString& rKeyValue );

    /** Removes whitespace characters from the beginning of the passed string.

        @param rCodeLine  (in/out parameter) The string to be modified.

        @return  True = at least one whitespace character found and removed
            from rCodeLine. False = rCodeLine is empty or does not start with
            a whitespace character.
     */
    static bool         eatWhitespace( ::rtl::OUString& rCodeLine );

    /** Removes the passed keyword from the beginning of the passed string.

        @param rCodeLine  (in/out parameter) The string to be modified.

        @param rKeyword  The keyword to be removed from the beginning of the
            rCodeLine string.

        @return  True = rCodeLine starts with the passed keyword (case
            insensitive), and is followed by whitespace characters, or it ends
            right after the keyword. The keyword and the following whitespace
            characters have been removed from rCodeLine. False = rCodeLine is
            empty or does not start with the specified keyword, or the keyword
            is not followed by whitespace characters.
     */
    static bool         eatKeyword( ::rtl::OUString& rCodeLine, const ::rtl::OUString& rKeyword );

    /** Returns the VBA source code of the specified module, or an empty
        string, if the module does not exist.

        @param rxBasicLib  The container for all VBA code modules.
        @param rModuleName  The name of the VBA code module.
     */
    static ::rtl::OUString getSourceCode(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& rxBasicLib,
                            const ::rtl::OUString& rModuleName );

    /** Checks, if a macro with the specified name exists in the passed VBA
        source code.

        @param rSourceCode  The VBA source code.
        @param rMacroName  The name of the macro.
     */
    static bool         hasMacro(
                            const ::rtl::OUString& rSourceCode,
                            const ::rtl::OUString& rMacroName );

    /** Checks, if a macro with the specified name exists in the specified
        module.

        @param rxBasicLib  The container for all VBA code modules.
        @param rModuleName  The name of the VBA module to check for the macro.
        @param rMacroName  The name of the macro.
     */
    static bool         hasMacro(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& rxBasicLib,
                            const ::rtl::OUString& rModuleName,
                            const ::rtl::OUString& rMacroName );

    /** Tries to insert a VBA macro into the specified code module.

        @descr  If the specified macro does not exist, it will be generated as
            following, using the passed parameters. If the parameter rMacroType
            is left empty, a sub procedure macro will be generated:

            Private Sub <rMacroName> ( <rMacroArgs> )
                <rMacroCode>
            End Sub

            If the parameter rMacroType is not empty, a function macro
            will be generated. Note that the parameter rMacroCode has to
            provide the code that returns the function value.

            Private Function <rMacroName> ( <rMacroArgs> ) As <rMacroType>
                <rMacroCode>
            End Function

            The source code in rMacroCode may contain a special placeholder
            $MACRO that will be replaced by the macro name passed in rMacroName
            before the macro will be inserted into the module.

        @param rModuleName  The name of the VBA module to be used.
        @param rMacroName  The name of the VBA macro to be inserted.
        @param rMacroArgs  The argument list of the VBA macro.
        @param rMacroType  Macro return type (empty for sub procedure).
        @param rMacroCode  The VBA source code for the macro.

        @return  True, if the specified VBA macro has been inserted. False, if
            there already exists a macro with the specified name, or if any
            error has occurred, for example, Office configuration forbids to
            generate executable VBA code or the specified module does not
            exist.
     */
    static bool         insertMacro(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& rxBasicLib,
                            const ::rtl::OUString& rModuleName,
                            const ::rtl::OUString& rMacroName,
                            const ::rtl::OUString& rMacroArgs,
                            const ::rtl::OUString& rMacroType,
                            const ::rtl::OUString& rMacroCode );

    /** Tries to attach a VBA macro to an event of the passed events supplier.

        @descr  The function checks if the specified macro exists and attaches
            it to the event of the passed events supplier.

        @param rxEventsSupp  The events supplier for the event to be attached.
        @param rEventName  The event name used in the office API.
        @param rLibraryName  The name of the Basic library containing the macro.
        @param rModuleName  The name of the VBA module containing the macro.
        @param rMacroName  The name of the VBA macro to attach to the event.
     */
    static bool         attachMacroToEvent(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventsSupplier >& rxEventsSupp,
                            const ::rtl::OUString& rEventName,
                            const ::rtl::OUString& rLibraryName,
                            const ::rtl::OUString& rModuleName,
                            const ::rtl::OUString& rMacroName );

private:
                        VbaHelper();
                        ~VbaHelper();
};

// ============================================================================

} // namespace ole
} // namespace oox

#endif
