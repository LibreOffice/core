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

#ifndef OOX_CORE_RECORDPARSER_HXX
#define OOX_CORE_RECORDPARSER_HXX

#include <map>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <rtl/ref.hxx>
#include "oox/helper/binaryinputstream.hxx"

namespace oox {
namespace core {

class FragmentHandler;
struct RecordInfo;

namespace prv { class Locator; }
namespace prv { class ContextStack; }

// ============================================================================

struct RecordInputSource
{
    BinaryInputStreamRef mxInStream;
    ::rtl::OUString     maPublicId;
    ::rtl::OUString     maSystemId;
};

// ============================================================================

class RecordParser
{
public:
    explicit            RecordParser();
    virtual             ~RecordParser();

    void                setFragmentHandler( const ::rtl::Reference< FragmentHandler >& rxHandler );

    void                parseStream( const RecordInputSource& rInputSource )
                            throw(  ::com::sun::star::xml::sax::SAXException,
                                    ::com::sun::star::io::IOException,
                                    ::com::sun::star::uno::RuntimeException );

    inline const RecordInputSource& getInputSource() const { return maSource; }

private:
    /** Returns a RecordInfo struct that contains the passed record identifier
        as context start identifier. */
    const RecordInfo*   getStartRecordInfo( sal_Int32 nRecId ) const;
    /** Returns a RecordInfo struct that contains the passed record identifier
        as context end identifier. */
    const RecordInfo*   getEndRecordInfo( sal_Int32 nRecId ) const;

private:
    typedef ::std::map< sal_Int32, RecordInfo > RecordInfoMap;

    RecordInputSource   maSource;
    ::rtl::Reference< FragmentHandler > mxHandler;
    ::rtl::Reference< prv::Locator > mxLocator;
    ::std::auto_ptr< prv::ContextStack > mxStack;
    RecordInfoMap       maStartMap;
    RecordInfoMap       maEndMap;
};

// ============================================================================

} // namespace core
} // namespace oox

#endif

