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

#ifndef OOX_CORE_RECORDPARSER_HXX
#define OOX_CORE_RECORDPARSER_HXX

#include <map>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <rtl/ref.hxx>
#include "oox/helper/binaryinputstream.hxx"
#include "oox/core/fragmenthandler.hxx"

namespace oox {
namespace core {

namespace prv { class Locator; }
namespace prv { class ContextStack; }

// ============================================================================

struct RecordInputSource
{
    BinaryInputStreamRef mxInStream;
    OUString     maPublicId;
    OUString     maSystemId;
};

// ============================================================================

class RecordParser
{
public:
                        RecordParser();
    virtual             ~RecordParser();

    void                setFragmentHandler( const ::rtl::Reference< FragmentHandler >& rxHandler );

    void                parseStream( const RecordInputSource& rInputSource )
                            throw(  ::com::sun::star::xml::sax::SAXException,
                                    ::com::sun::star::io::IOException,
                                    ::com::sun::star::uno::RuntimeException );

    const RecordInputSource& getInputSource() const { return maSource; }

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
