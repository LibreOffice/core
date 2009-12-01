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
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include "oox/drawingml/table/tablestylelist.hxx"
#include "oox/core/filterbase.hxx"
#include "oox/core/relations.hxx"
#include <oox/dllapi.h>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XText.hpp>

namespace com { namespace sun { namespace star {
    namespace container { class XNameContainer; }
    namespace xml { namespace sax { class XLocator; } }
    namespace xml { namespace sax { class XFastDocumentHandler; } }
} } }

namespace oox { namespace drawingml { class Theme; } }
namespace oox { namespace drawingml { namespace chart { class ChartConverter; } } }
namespace oox { namespace vml { class Drawing; } }

namespace sax_fastparser {
    class FastSerializerHelper;

    typedef boost::shared_ptr< FastSerializerHelper > FSHelperPtr;
}

namespace oox {
namespace core {

class FragmentHandler;

struct TextField {
    com::sun::star::uno::Reference< com::sun::star::text::XText > xText;
    com::sun::star::uno::Reference< com::sun::star::text::XTextCursor > xTextCursor;
    com::sun::star::uno::Reference< com::sun::star::text::XTextField > xTextField;
};
typedef std::vector< TextField > TextFieldStack;

// ============================================================================

struct XmlFilterBaseImpl;

class OOX_DLLPUBLIC XmlFilterBase : public FilterBase
{
public:
    explicit            XmlFilterBase(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxGlobalFactory );

    virtual             ~XmlFilterBase();

    /** Has to be implemented by each filter, returns the current theme. */
    virtual const ::oox::drawingml::Theme*
                        getCurrentTheme() const = 0;

    /** Has to be implemented by each filter to resolve scheme colors. */
    virtual sal_Int32   getSchemeClr( sal_Int32 nColorSchemeToken ) const = 0;

    /** Has to be implemented by each filter to return the collection of VML shapes. */
    virtual ::oox::vml::Drawing* getVmlDrawing() = 0;

    /** Has to be implemented by each filter, returns a filter-specific chart
        converter object, that should be global per imported document. */
    virtual ::oox::drawingml::chart::ChartConverter& getChartConverter() = 0;

    /** Has to be implemented by each filter to return the table style list. */
    virtual const ::oox::drawingml::table::TableStyleListPtr getTableStyles() = 0;

    // ------------------------------------------------------------------------

    /** Returns the fragment path from the first relation of the passed type,
        used for fragments referred by the root relations. */
    ::rtl::OUString     getFragmentPathFromFirstType( const ::rtl::OUString& rType );

    /** Imports a fragment using the passed fragment handler, which contains
        the full path to the fragment stream.

        @return  True, if the fragment could be imported.
     */
    bool                importFragment( const ::rtl::Reference< FragmentHandler >& rxHandler );

    /** Imports the relations fragment associated with the specified fragment.

        @return  The relations collection of the specified fragment.
     */
    RelationsRef        importRelations( const ::rtl::OUString& rFragmentPath );

    /** Adds new relation.

        @param rType
            Relation type.

        @param rTarget
            Relation target.

        @return  Added relation Id.
     */
    ::rtl::OUString     addRelation( const ::rtl::OUString& rType, const ::rtl::OUString& rTarget, bool bExternal = false );

    /** Adds new relation to part's relations.

        @param rPartName
            Part name the relations are related to. The relations will be stored in <rPartName::path>/_rels/<rPartName::name>.rels.

        @param rType
            Relation type.

        @param rTarget
            Relation target.

        @return  Added relation Id.
     */
    ::rtl::OUString     addRelation( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > xOutputStream, const ::rtl::OUString& rType, const ::rtl::OUString& rTarget, bool bExternal = false );

    /** Returns a stack of used textfields, used by the pptx importer to replace links to slidepages with rhe real page name */
    TextFieldStack& getTextFieldStack() const;

    /** Opens and returns the specified output stream from the base storage with specified media type.

        @param rStreamName
            The name of the embedded storage stream. The name may contain
            slashes to open streams from embedded substorages. If base stream
            access has been enabled in the storage, the base stream can be
            accessed by passing an empty string as stream name.

        @param rMediaType
            The media type string, used in [Content_Types].xml stream in base
            storage.

        @return The opened output stream.
     */
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
                        openFragmentStream(
                            const ::rtl::OUString& rStreamName,
                            const ::rtl::OUString& rMediaType );

    /** Opens specified output stream from the base storage with specified
        media type and returns new fast serializer for that stream.

        @param rStreamName
            The name of the embedded storage stream. The name may contain
            slashes to open streams from embedded substorages. If base stream
            access has been enabled in the storage, the base stream can be
            accessed by passing an empty string as stream name.

        @param rMediaType
            The media type string, used in [Content_Types].xml stream in base
            storage.

        @return newly created serializer helper.
     */
    ::sax_fastparser::FSHelperPtr
                        openFragmentStreamWithSerializer(
                            const ::rtl::OUString& rStreamName,
                            const ::rtl::OUString& rMediaType );

    /** Returns new unique ID for exported document.

        @return newly created ID.
     */
    inline sal_Int32 GetUniqueId() { return mnMaxDocId++; }
    inline ::rtl::OString GetUniqueIdOString() { return ::rtl::OString::valueOf( mnMaxDocId++ ); }
    inline ::rtl::OUString GetUniqueIdOUString() { return ::rtl::OUString::valueOf( mnMaxDocId++ ); }

private:
    virtual StorageRef  implCreateStorage(
                            ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStream,
                            ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& rxOutStream ) const;

private:
    ::std::auto_ptr< XmlFilterBaseImpl > mxImpl;
    sal_Int32 mnRelId;
    sal_Int32 mnMaxDocId;
};

typedef ::rtl::Reference< XmlFilterBase > XmlFilterRef;

// ============================================================================

} // namespace core
} // namespace oox

#endif

