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

#ifndef OOX_OLE_OLEHELPER_HXX
#define OOX_OLE_OLEHELPER_HXX

#include <rtl/ustring.hxx>
#include "oox/helper/binarystreambase.hxx"

namespace oox { class BinaryInputStream; }

namespace oox {
namespace ole {

// ============================================================================

/** Stores data about a StdHlink hyperlink. */
struct StdHlinkInfo
{
    ::rtl::OUString     maTarget;
    ::rtl::OUString     maLocation;
    ::rtl::OUString     maDisplay;
    ::rtl::OUString     maFrame;
};

// ============================================================================

/** Static helper functions for OLE import/export. */
class OleHelper
{
public:
    /** Imports a GUID from the passed binary stream and returns its string representation. */
    static ::rtl::OUString importGuid( BinaryInputStream& rInStrm );

    /** Imports an OLE StdPic picture from the current position of the passed binary stream. */
    static bool         importStdPic( StreamDataSequence& orGraphicData, BinaryInputStream& rInStrm, bool bWithGuid );

    /** Imports an OLE StdHlink from the current position of the passed binary stream. */
    static bool         importStdHlink( StdHlinkInfo& orHlinkInfo, BinaryInputStream& rInStrm, rtl_TextEncoding eTextEnc, bool bWithGuid );

private:
                        OleHelper();        // not implemented
                        ~OleHelper();       // not implemented
};

// ============================================================================

} // namespace ole
} // namespace oox

#endif

