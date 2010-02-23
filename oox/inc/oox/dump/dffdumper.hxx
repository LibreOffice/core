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
    inline sal_uInt16   getVer() const { return mnInstVer & 0x000F; }
    inline sal_uInt16   getInst() const { return (mnInstVer & 0xFFF0) >> 4; }
    inline bool         isContainer() const { return getVer() == 15; }

protected:
    inline explicit     DffStreamObject() {}

    using               SequenceRecordObjectBase::construct;
    void                construct( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, const ::rtl::OUString& rSysFileName );
    void                construct( const OutputObjectBase& rParent, const BinaryInputStreamRef& rxStrm );

    virtual bool        implReadRecordHeader( BinaryInputStream& rBaseStrm, sal_Int64& ornRecId, sal_Int64& ornRecSize );
    virtual void        implWriteExtHeader();
    virtual void        implDumpRecordBody();
    virtual void        implDumpClientAnchor();

private:
    void                constructDffObj();

    sal_uInt16          dumpDffOptPropHeader();
    void                dumpDffOptPropValue( sal_uInt16 nPropId, sal_uInt32 nValue );

private:
    sal_uInt16          mnInstVer;
    sal_Int32           mnRealSize;
};

// ============================================================================

} // namespace dump
} // namespace oox

#endif
#endif

