/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xmlfilterbase.hxx,v $
 * $Revision: 1.7 $
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

#ifndef OOX_CORE_XMLFILTERBASE_HXX
#define OOX_CORE_XMLFILTERBASE_HXX

#include <rtl/ref.hxx>
#include "oox/vml/drawing.hxx"
#include "oox/drawingml/table/tablestylelist.hxx"
#include "oox/core/filterbase.hxx"
#include "oox/core/relations.hxx"

namespace com { namespace sun { namespace star {
    namespace container { class XNameContainer; }
    namespace xml { namespace sax { class XLocator; } }
    namespace xml { namespace sax { class XFastDocumentHandler; } }
} } }

namespace oox { namespace drawingml { class Theme; } }
namespace oox { namespace drawingml { namespace chart { class ChartConverter; } } }

namespace oox {
namespace core {

class FragmentHandler;
class ModelObjectContainer;

// ============================================================================

struct XmlFilterBaseImpl;

class XmlFilterBase : public FilterBase
{
public:
    explicit            XmlFilterBase(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory );

    virtual             ~XmlFilterBase();

    /** Has to be implemented by each filter, returns the current theme. */
    virtual const ::oox::drawingml::Theme*
                        getCurrentTheme() const = 0;

    /** Has to be implemented by each filter to resolve scheme colors. */
    virtual sal_Int32   getSchemeClr( sal_Int32 nColorSchemeToken ) const = 0;

    /** Has to be implemented by each filter to return drawings collection. */
    virtual const ::oox::vml::DrawingPtr
                        getDrawings() = 0;

    /** Has to be implemented by each filter, returns a filter-specific chart
        converter object, that should be global per imported document. */
    virtual ::oox::drawingml::chart::ChartConverter&
                        getChartConverter() = 0;

    /** Has to be implemented by each filter to return the table style list. */
    virtual const ::oox::drawingml::table::TableStyleListPtr getTableStyles() = 0;

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

    /** Returns object containers for various named drawing objects for the imported document. */
    ModelObjectContainer& getModelObjectContainer() const;

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

