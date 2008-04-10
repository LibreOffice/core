/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dffdumper.hxx,v $
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

#ifndef OOX_DUMP_DFFDUMPER_HXX
#define OOX_DUMP_DFFDUMPER_HXX

#include "oox/dump/dumperbase.hxx"

#if OOX_INCLUDE_DUMPER

namespace oox {
namespace dump {

// ============================================================================

class DffRecordHeaderObject : public RecordHeaderBase< sal_uInt16, sal_uInt32 >
{
public:
    explicit            DffRecordHeaderObject( const InputObjectBase& rParent );

    inline sal_Int64    getBodyStart() const { return mnBodyStart; }
    inline sal_Int64    getBodyEnd() const { return mnBodyEnd; }
    inline sal_uInt16   getVer() const { return mnInstVer & 0x000F; }
    inline sal_uInt16   getInst() const { return (mnInstVer & 0xFFF0) >> 4; }

protected:
    virtual bool        implIsValid() const;
    virtual bool        implReadHeader( sal_Int64& ornRecPos, sal_uInt16& ornRecId, sal_uInt32& ornRecSize );
    virtual void        implWriteExtHeader();

private:
    NameListRef         mxRecInst;
    sal_Int64           mnBodyStart;
    sal_Int64           mnBodyEnd;
    sal_uInt16          mnInstVer;
};

// ============================================================================

class DffDumpObject : public InputObjectBase
{
public:
    explicit            DffDumpObject( const InputObjectBase& rParent );
    virtual             ~DffDumpObject();

    void                dumpDffClientPos( const sal_Char* pcName, sal_Int32 nSubScale );
    void                dumpDffClientRect();

protected:
    virtual bool        implIsValid() const;
    virtual void        implDump();

private:
    void                dumpRecordBody();

    void                dumpDffOptRec();
    sal_uInt16          dumpDffOptPropHeader();
    void                dumpDffOptPropValue( sal_uInt16 nPropId, sal_uInt32 nValue );

private:
    typedef ::boost::shared_ptr< DffRecordHeaderObject > DffRecHeaderObjRef;
    DffRecHeaderObjRef  mxHdrObj;
};

typedef ::boost::shared_ptr< DffDumpObject > DffDumpObjectRef;

// ============================================================================

} // namespace dump
} // namespace oox

#endif
#endif

