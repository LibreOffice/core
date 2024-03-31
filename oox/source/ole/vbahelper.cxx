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

#include <oox/ole/vbahelper.hxx>
#include <osl/diagnose.h>
#include <o3tl/string_view.hxx>
#include <oox/helper/binaryinputstream.hxx>

namespace oox::ole {

bool VbaHelper::readDirRecord( sal_uInt16& rnRecId, StreamDataSequence& rRecData, BinaryInputStream& rInStrm )
{
    // read the record header
    sal_Int32 nRecSize;
    rnRecId = rInStrm.readuInt16();
    nRecSize = rInStrm.readInt32();
    // for no obvious reason, PROJECTVERSION record contains size field of 4, but is 6 bytes long
    if( rnRecId == VBA_ID_PROJECTVERSION )
    {
        OSL_ENSURE( nRecSize == 4, "VbaHelper::readDirRecord - unexpected record size for PROJECTVERSION" );
        nRecSize = 6;
    }
    // read the record contents into the passed sequence
    return !rInStrm.isEof() && (rInStrm.readData( rRecData, nRecSize ) == nRecSize);
}

bool VbaHelper::extractKeyValue( OUString& rKey, OUString& rValue, std::u16string_view rKeyValue )
{
    size_t nEqSignPos = rKeyValue.find( '=' );
    if( nEqSignPos > 0 && nEqSignPos != std::u16string_view::npos )
    {
        rKey = o3tl::trim(rKeyValue.substr( 0, nEqSignPos ));
        rValue = o3tl::trim(rKeyValue.substr( nEqSignPos + 1 ));
        return !rKey.isEmpty() && !rValue.isEmpty();
    }
    return false;
}

} // namespace oox::ole

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
