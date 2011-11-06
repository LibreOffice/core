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



#ifndef OOX_OLE_VBAHELPER_HXX
#define OOX_OLE_VBAHELPER_HXX

#include "oox/helper/binarystreambase.hxx"

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
const sal_uInt16 VBA_ID_PROJECTNAME             = 0x0004;
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

private:
                        VbaHelper();
                        ~VbaHelper();
};

// ============================================================================

} // namespace ole
} // namespace oox

#endif
