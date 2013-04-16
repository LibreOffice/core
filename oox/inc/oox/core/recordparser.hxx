/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
