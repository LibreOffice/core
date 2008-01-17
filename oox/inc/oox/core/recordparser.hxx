/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: recordparser.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:44 $
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

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <rtl/ref.hxx>
#include "oox/helper/binaryinputstream.hxx"
#include "oox/core/recordinfoprovider.hxx"

namespace oox {
namespace core {

class FragmentHandler;
class RecordContext;

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
    void                setRecordInfoProvider( const RecordInfoProviderRef& rxProvider );

    void                parseStream( const RecordInputSource& rInputSource )
                            throw(  ::com::sun::star::xml::sax::SAXException,
                                    ::com::sun::star::io::IOException,
                                    ::com::sun::star::uno::RuntimeException );

    inline const RecordInputSource& getInputSource() const { return maSource; }

    /** Helper for broken OOBIN streams, pushes a new context onto the stack. */
    void                pushContext( sal_Int32 nRecId, const ::rtl::Reference< RecordContext >& rxContext );
    /** Helper for broken OOBIN streams, pops the current context from stack. */
    void                popContext();

private:
    RecordInputSource   maSource;
    ::rtl::Reference< FragmentHandler > mxHandler;
    RecordInfoProviderRef mxRecInfoProvider;
    ::rtl::Reference< prv::Locator > mxLocator;
    ::std::auto_ptr< prv::ContextStack > mxStack;
};

// ============================================================================

} // namespace core
} // namespace oox

#endif

