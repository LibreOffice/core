/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: olestoragedumper.hxx,v $
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

#ifndef OOX_DUMP_OLESTORAGEDUMPER_HXX
#define OOX_DUMP_OLESTORAGEDUMPER_HXX

#include "oox/helper/storagebase.hxx"
#include "oox/dump/dumperbase.hxx"

#if OOX_INCLUDE_DUMPER

namespace com { namespace sun { namespace star {
    namespace io { class XInputStream; }
} } }

namespace oox {
namespace dump {

// ============================================================================

class OlePropertyStreamObject : public InputStreamObject
{
public:
    explicit            OlePropertyStreamObject( const ObjectBase& rParent, const ::rtl::OUString& rOutFileName, BinaryInputStreamRef xStrm );

protected:
    virtual void        implDump();

private:
    void                dumpSection( const ::rtl::OUString& rGuid, sal_uInt32 nStartPos );

    void                dumpProperty( sal_Int32 nPropId, sal_uInt32 nStartPos );
    void                dumpCodePageProperty( sal_uInt32 nStartPos );
    void                dumpDictionaryProperty( sal_uInt32 nStartPos );

    void                dumpPropertyContents( sal_Int32 nPropId );
    void                dumpPropertyValue( sal_Int32 nPropId, sal_Int32 nBaseType );

    sal_Int32           dumpPropertyType();
    void                dumpBlob( const sal_Char* pcName );
    ::rtl::OUString     dumpString8( const sal_Char* pcName );
    ::rtl::OUString     dumpCharArray8( const sal_Char* pcName, sal_Int32 nCharCount );
    ::rtl::OUString     dumpString16( const sal_Char* pcName );
    ::rtl::OUString     dumpCharArray16( const sal_Char* pcName, sal_Int32 nCharCount );
    ::com::sun::star::util::DateTime dumpFileTime( const sal_Char* pcName );

    bool                startElement( sal_uInt32 nStartPos );
    void                writeSectionHeader( const ::rtl::OUString& rGuid, sal_uInt32 nStartPos );
    void                writePropertyHeader( sal_Int32 nPropId, sal_uInt32 nStartPos );

private:
    NameListRef         mxPropIds;
    rtl_TextEncoding    meTextEnc;
    bool                mbIsUnicode;
};

// ============================================================================

} // namespace dump
} // namespace oox

#endif
#endif

