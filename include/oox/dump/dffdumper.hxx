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

#ifndef OOX_DUMP_DFFDUMPER_HXX
#define OOX_DUMP_DFFDUMPER_HXX

#include "oox/dump/dumperbase.hxx"

#if OOX_INCLUDE_DUMPER

namespace oox {
namespace dump {

// ============================================================================

class DffStreamObject : public SequenceRecordObjectBase
{
public:
    sal_uInt16   getVer() const { return mnInstVer & 0x000F; }
    sal_uInt16   getInst() const { return (mnInstVer & 0xFFF0) >> 4; }
    bool         isContainer() const { return getVer() == 15; }

protected:
                        DffStreamObject() {}

    using               SequenceRecordObjectBase::construct;

    virtual bool        implReadRecordHeader( BinaryInputStream& rBaseStrm, sal_Int64& ornRecId, sal_Int64& ornRecSize );
    virtual void        implWriteExtHeader();
    virtual void        implDumpRecordBody();
    virtual void        implDumpClientAnchor();

private:
    sal_uInt32          dumpDffSimpleColor( const String& rName );

    void                dumpDffOpt();
    sal_uInt16          dumpDffOptPropHeader();

private:
    ItemFormatMap       maSimpleProps;
    ItemFormatMap       maComplexProps;
    sal_uInt16          mnInstVer;
    sal_Int32           mnRealSize;
};

// ============================================================================

} // namespace dump
} // namespace oox

#endif
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
