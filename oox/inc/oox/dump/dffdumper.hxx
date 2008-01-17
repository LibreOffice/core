/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dffdumper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

