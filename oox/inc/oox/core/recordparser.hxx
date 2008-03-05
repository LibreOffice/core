/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: recordparser.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:36:10 $
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

