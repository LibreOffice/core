/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlfilterbase.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:37:20 $
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

#ifndef OOX_CORE_XMLFILTERBASE_HXX
#define OOX_CORE_XMLFILTERBASE_HXX

#include <rtl/ref.hxx>
#include "oox/vml/drawing.hxx"
#include "oox/core/filterbase.hxx"
#include "oox/core/relations.hxx"

namespace com { namespace sun { namespace star {
    namespace container { class XNameContainer; }
    namespace xml { namespace sax { class XLocator; } }
    namespace xml { namespace sax { class XFastDocumentHandler; } }
} } }

namespace oox {
namespace core {

class FragmentHandler;

// ============================================================================

struct XmlFilterBaseImpl;

class XmlFilterBase : public FilterBase
{
public:
    explicit            XmlFilterBase(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory );

    virtual             ~XmlFilterBase();

    /** Has to be implemented by each filter to resolve scheme colors. */
    virtual sal_Int32   getSchemeClr( sal_Int32 nColorSchemeToken ) const = 0;

    /** Has to be implemented by each filter to return drawings collection. */
    virtual const ::oox::vml::DrawingPtr getDrawings() = 0;

    // ------------------------------------------------------------------------

    /** Returns the fragment path for the passed type, used for fragments
        referred by the root relations. */
    ::rtl::OUString     getFragmentPathFromType( const ::rtl::OUString& rType );

    /** Imports a fragment using the passed fragment handler, which contains
        the full path to the fragment stream.

        @return  True, if the fragment could be imported.
     */
    bool                importFragment( const ::rtl::Reference< FragmentHandler >& rxHandler );

    /** Imports the relations fragment associated with the specified fragment.

        @return  The relations collection of the specified fragment.
     */
    RelationsRef        importRelations( const ::rtl::OUString& rFragmentPath );

    /** Copies the picture element specified with rPicturePath from the source
        document to the target models picture substorage.

        @return  The URL of the picture in the target models storage.
     */
    ::rtl::OUString     copyPictureStream( const ::rtl::OUString& rPicturePath );

    /** needs to be holded throughout the whole import, allows to create new arrow names... */
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >&
                        getMarkerTable() const;

private:
    virtual StorageRef  implCreateStorage(
                            ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStream,
                            ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& rxOutStream ) const;

private:
    ::std::auto_ptr< XmlFilterBaseImpl > mxImpl;
};

typedef ::rtl::Reference< XmlFilterBase > XmlFilterRef;

// ============================================================================

} // namespace core
} // namespace oox

#endif

