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

#include "oox/ole/vbahelper.hxx"
#include <rtl/ustrbuf.hxx>
#include "oox/helper/binaryinputstream.hxx"

namespace oox {
namespace ole {



using namespace ::com::sun::star::uno;



bool VbaHelper::readDirRecord( sal_uInt16& rnRecId, StreamDataSequence& rRecData, BinaryInputStream& rInStrm )
{
    // read the record header
    sal_Int32 nRecSize;
    rInStrm >> rnRecId >> nRecSize;
    // for no obvious reason, PROJECTVERSION record contains size field of 4, but is 6 bytes long
    if( rnRecId == VBA_ID_PROJECTVERSION )
    {
        OSL_ENSURE( nRecSize == 4, "VbaHelper::readDirRecord - unexpected record size for PROJECTVERSION" );
        nRecSize = 6;
    }
    // read the record contents into the passed sequence
    return !rInStrm.isEof() && (rInStrm.readData( rRecData, nRecSize ) == nRecSize);
}

bool VbaHelper::extractKeyValue( OUString& rKey, OUString& rValue, const OUString& rKeyValue )
{
    sal_Int32 nEqSignPos = rKeyValue.indexOf( '=' );
    if( nEqSignPos > 0 )
    {
        rKey = rKeyValue.copy( 0, nEqSignPos ).trim();
        rValue = rKeyValue.copy( nEqSignPos + 1 ).trim();
        return !rKey.isEmpty() && !rValue.isEmpty();
    }
    return false;
}



} // namespace ole
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
