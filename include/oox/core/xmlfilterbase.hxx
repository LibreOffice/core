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

#ifndef OOX_CORE_XMLFILTERBASE_HXX
#define OOX_CORE_XMLFILTERBASE_HXX

#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <rtl/ref.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include "oox/core/filterbase.hxx"
#include "oox/core/relations.hxx"
#include "oox/drawingml/table/tablestylelist.hxx"
#include "oox/dllapi.h"

namespace com { namespace sun { namespace star {
    namespace container { class XNameContainer; }
    namespace document { class XDocumentProperties; }
    namespace xml { namespace dom { class XDocument; } }
    namespace xml { namespace sax { class XLocator; } }
    namespace xml { namespace sax { class XFastDocumentHandler; } }
    namespace xml { namespace sax { class XFastSAXSerializable; } }
} } }

namespace oox {
    namespace drawingml { class Theme; }
    namespace drawingml { namespace chart { class ChartConverter; } }
    namespace vml { class Drawing; }
}

namespace sax_fastparser {
    class FastSerializerHelper;

    typedef boost::shared_ptr< FastSerializerHelper > FSHelperPtr;
}

namespace oox {
namespace core {

class FragmentHandler;

// ============================================================================

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
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext )
                            throw( ::com::sun::star::uno::RuntimeException );

    virtual             ~XmlFilterBase();

    /** Has to be implemented by each filter, returns the current theme. */
    virtual const ::oox::drawingml::Theme*
                        getCurrentTheme() const = 0;

    /** Has to be implemented by each filter to return the collection of VML shapes. */
    virtual ::oox::vml::Drawing* getVmlDrawing() = 0;

    /** Has to be implemented by each filter, returns a filter-specific chart
        converter object, that should be global per imported document. */
    virtual ::oox::drawingml::chart::ChartConverter* getChartConverter() = 0;

    /** Helper to switch chart data table - specifically for xlsx imports */
     virtual void useInternalChartDataTable( bool /*bInternal*/ ) { }

    /** Has to be implemented by each filter to return the table style list. */
    virtual const ::oox::drawingml::table::TableStyleListPtr getTableStyles() = 0;

    // ------------------------------------------------------------------------

    /** Returns the fragment path from the first relation of the passed type,
        used for fragments referred by the root relations. */
    OUString     getFragmentPathFromFirstType( const OUString& rType );

    /** Imports a fragment using the passed fragment handler, which contains
        the full path to the fragment stream.

        @return  True, if the fragment could be imported.
     */
    bool                importFragment( const ::rtl::Reference< FragmentHandler >& rxHandler );

    /** Imports a fragment into an xml::dom::XDocument.

        @param rFragmentPath path to fragment

        @return a non-empty reference to the XDocument, if the
        fragment could be imported.
     */
    ::com::sun::star::uno::Reference<
       ::com::sun::star::xml::dom::XDocument> importFragment( const OUString& rFragmentPath );

    /** Imports a fragment from an xml::dom::XDocument using the
        passed fragment handler

        @param rxHandler fragment handler; path to fragment is
        ignored, input source is the rxSerializer

        @param rxSerializer usually retrieved from a
        xml::dom::XDocument, will get serialized into rxHandler

        @return true, if the fragment could be imported.
     */
    bool importFragment( const ::rtl::Reference< FragmentHandler >& rxHandler,
                         const ::com::sun::star::uno::Reference<
                               ::com::sun::star::xml::sax::XFastSAXSerializable >& rxSerializer );

    /** Imports the relations fragment associated with the specified fragment.

        @return  The relations collection of the specified fragment.
     */
    RelationsRef        importRelations( const OUString& rFragmentPath );

    /** Adds new relation.

        @param rType
            Relation type.

        @param rTarget
            Relation target.

        @return  Added relation Id.
     */
    OUString     addRelation( const OUString& rType, const OUString& rTarget, bool bExternal = false );

    /** Adds new relation to part's relations.

        @param rPartName
            Part name the relations are related to. The relations will be stored in <rPartName::path>/_rels/<rPartName::name>.rels.

        @param rType
            Relation type.

        @param rTarget
            Relation target.

        @return  Added relation Id.
     */
    OUString     addRelation( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > xOutputStream, const OUString& rType, const OUString& rTarget, bool bExternal = false );

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
                            const OUString& rStreamName,
                            const OUString& rMediaType );

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
                            const OUString& rStreamName,
                            const OUString& rMediaType );

    /** Returns new unique ID for exported document.

        @return newly created ID.
     */
    inline sal_Int32 GetUniqueId() { return mnMaxDocId++; }
    inline OString GetUniqueIdOString() { return OString::valueOf( mnMaxDocId++ ); }
    inline OUString GetUniqueIdOUString() { return OUString::valueOf( mnMaxDocId++ ); }

    /** Write the document properties into into the current OPC package.

        @param xProperties  The document properties to export.

        @return *this
     */
    XmlFilterBase& exportDocumentProperties( ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentProperties > xProperties );

    OUString getNamespaceURL( const OUString& rPrefix );

    sal_Int32 getNamespaceId( const OUString& rUrl );

    void importDocumentProperties();

protected:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
        implGetInputStream( ::comphelper::MediaDescriptor& rMediaDesc ) const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >
        implGetOutputStream( ::comphelper::MediaDescriptor& rMediaDesc ) const;

    virtual bool implFinalizeExport(  ::comphelper::MediaDescriptor& rMediaDescriptor );

private:
    virtual StorageRef  implCreateStorage(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStream ) const;
    virtual StorageRef  implCreateStorage(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& rxOutStream ) const;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
