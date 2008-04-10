/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fdumperdff.hxx,v $
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

#ifndef SC_FDUMPERDFF_HXX
#define SC_FDUMPERDFF_HXX

#include "fdumper.hxx"

#if SCF_INCL_DUMPER

namespace scf {
namespace dump {

// ============================================================================
// ============================================================================

class DffRecordHeaderObject : public RecordHeaderBase
{
public:
    explicit            DffRecordHeaderObject( const InputObjectBase& rParent );

    inline sal_uInt16   GetRecId() const { return mnRecId; }
    inline sal_uInt32   GetRecSize() const { return mnRecSize; }
    inline sal_Size     GetBodyStart() const { return mnBodyStart; }
    inline sal_Size     GetBodyEnd() const { return mnBodyEnd; }

    inline sal_uInt16   GetVer() const { return mnVer; }
    inline sal_uInt16   GetInst() const { return mnInst; }

    inline bool         HasRecName() const { return GetRecNames()->HasName( mnRecId ); }

protected:
    virtual bool        ImplIsValid() const;
    virtual void        ImplDumpBody();

private:
    NameListRef         mxRecInst;
    sal_Size            mnBodyStart;
    sal_Size            mnBodyEnd;
    sal_uInt32          mnRecSize;
    sal_uInt16          mnRecId;
    sal_uInt16          mnVer;
    sal_uInt16          mnInst;
};

// ============================================================================
// ============================================================================

class DffDumpObject : public InputObjectBase
{
public:
    explicit            DffDumpObject( const InputObjectBase& rParent );
    virtual             ~DffDumpObject();

    void                DumpDffClientPos( const sal_Char* pcName, sal_Int32 nSubScale );
    void                DumpDffClientRect();

protected:
    virtual bool        ImplIsValid() const;
    virtual void        ImplDumpBody();

private:
    void                ConstructOwn();

    void                DumpRecordBody();

    void                DumpDffOptRec();
    sal_uInt16          DumpDffOptPropHeader();
    void                DumpDffOptPropValue( sal_uInt16 nPropId, sal_uInt32 nValue );

private:
    typedef ScfRef< DffRecordHeaderObject > DffRecHeaderObjRef;
    DffRecHeaderObjRef  mxHdrObj;
};

typedef ScfRef< DffDumpObject > DffDumpObjectRef;

// ============================================================================
// ============================================================================

} // namespace dump
} // namespace scf

#endif
#endif

