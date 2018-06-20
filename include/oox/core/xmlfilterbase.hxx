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

#ifndef INCLUDED_OOX_CORE_XMLFILTERBASE_HXX
#define INCLUDED_OOX_CORE_XMLFILTERBASE_HXX

#include <memory>
#include <vector>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <oox/core/filterbase.hxx>
#include <oox/core/relations.hxx>
#include <oox/dllapi.h>
#include <oox/helper/storagebase.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace com { namespace sun { namespace star {
    namespace document { class XDocumentProperties; }
    namespace io { class XInputStream; }
    namespace io { class XOutputStream; }
    namespace io { class XStream; }
    namespace text { class XText; }
    namespace text { class XTextCursor; }
    namespace text { class XTextField; }
    namespace uno { class XComponentContext; }
    namespace xml { namespace dom { class XDocument; } }
    namespace xml { namespace sax { class XFastSAXSerializable; } }
} } }

namespace oox {
    namespace drawingml { class Theme; }
    namespace drawingml { namespace chart { class ChartConverter; } }
    namespace drawingml { namespace table {
        class TableStyleList;
        typedef std::shared_ptr< TableStyleList > TableStyleListPtr;
    } }
    namespace vml { class Drawing; }
}

namespace rtl { template <class reference_type> class Reference; }

namespace sax_fastparser {
    class FastSerializerHelper;

    typedef std::shared_ptr< FastSerializerHelper > FSHelperPtr;
}

namespace utl { class MediaDescriptor; }

namespace oox {
namespace core {

class FragmentHandler;
class FastParser;

struct TextField {
    css::uno::Reference< css::text::XText >       xText;
    css::uno::Reference< css::text::XTextCursor > xTextCursor;
    css::uno::Reference< css::text::XTextField >  xTextField;
};
typedef std::vector< TextField > TextFieldStack;

struct XmlFilterBaseImpl;

class OOX_DLLPUBLIC XmlFilterBase : public FilterBase
{
public:
    /// @throws css::uno::RuntimeException
    explicit            XmlFilterBase(
                            const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual             ~XmlFilterBase() override;

    /** Has to be implemented by each filter, returns the current theme. */
    virtual const ::oox::drawingml::Theme* getCurrentTheme() const = 0;

    /** Has to be implemented by each filter to return the collection of VML shapes. */
    virtual ::oox::vml::Drawing* getVmlDrawing() = 0;

    /** Has to be implemented by each filter, returns a filter-specific chart
        converter object, that should be global per imported document. */
    virtual ::oox::drawingml::chart::ChartConverter* getChartConverter() = 0;

    /** Helper to switch chart data table - specifically for xlsx imports */
    virtual void useInternalChartDataTable( bool /*bInternal*/ ) { }

    /** Has to be implemented by each filter to return the table style list. */
    virtual const ::oox::drawingml::table::TableStyleListPtr getTableStyles() = 0;


    OUString getFragmentPathFromFirstTypeFromOfficeDoc( const OUString& rPart );

    /** Imports a fragment using the passed fragment handler, which contains
        the full path to the fragment stream.

        @return  True, if the fragment could be imported.
     */
    bool importFragment( const rtl::Reference<FragmentHandler>& rxHandler );
    bool importFragment( const rtl::Reference<FragmentHandler>& rxHandler, FastParser& rParser );

    /** Imports a fragment into an xml::dom::XDocument.

        @param rFragmentPath path to fragment

        @return a non-empty reference to the XDocument, if the
        fragment could be imported.
     */
    css::uno::Reference< css::xml::dom::XDocument> importFragment( const OUString& rFragmentPath );

    /** Imports a fragment from an xml::dom::XDocument using the
        passed fragment handler

        @param rxHandler fragment handler; path to fragment is
        ignored, input source is the rxSerializer

        @param rxSerializer usually retrieved from a
        xml::dom::XDocument, will get serialized into rxHandler

        @return true, if the fragment could be imported.
     */
    bool importFragment( const ::rtl::Reference< FragmentHandler >& rxHandler,
                         const css::uno::Reference< css::xml::sax::XFastSAXSerializable >& rxSerializer );

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
    OUString     addRelation( const OUString& rType, const OUString& rTarget );

    /** Adds new relation to part's relations.

        @param rPartName
            Part name the relations are related to. The relations will be stored in <rPartName::path>/_rels/<rPartName::name>.rels.

        @param rType
            Relation type.

        @param rTarget
            Relation target.

        @return  Added relation Id.
     */
    OUString     addRelation( const css::uno::Reference< css::io::XOutputStream >& rOutputStream, const OUString& rType, const OUString& rTarget, bool bExternal = false );

    /** Returns a stack of used textfields, used by the pptx importer to replace links to slidepages with the real page name */
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
    css::uno::Reference< css::io::XOutputStream >
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
    sal_Int32 GetUniqueId() { return mnMaxDocId++; }

    /** Write the document properties into into the current OPC package.

        @param xProperties  The document properties to export.
     */
    void exportDocumentProperties( const css::uno::Reference< css::document::XDocumentProperties >& xProperties, bool bSecurityOptOpenReadOnly );

    /** Write the customXml entries we are preserving (xlsx and pptx only). */
    void exportCustomFragments();

    /** Read the document properties and also the customXml entries (xlsx and pptx only). */
    void importDocumentProperties();

    static void putPropertiesToDocumentGrabBag(const css::uno::Reference<css::lang::XComponent>& xDstDoc,
                                               const comphelper::SequenceAsHashMap& rProperties);

    static FastParser* createParser();

    bool isMSO2007Document() const;

    /// Signal that an MSO 2007-created SmartArt was found, need to warn the
    /// user about it.
    void setMissingExtDrawing();

    void checkDocumentProperties(
            const css::uno::Reference<css::document::XDocumentProperties>& xDocProps);

    OUString getNamespaceURL(sal_Int32 nNSID) const;

protected:
    virtual css::uno::Reference< css::io::XInputStream >
        implGetInputStream( utl::MediaDescriptor& rMediaDesc ) const override;

    virtual css::uno::Reference< css::io::XStream >
        implGetOutputStream( utl::MediaDescriptor& rMediaDesc ) const override;

    virtual bool implFinalizeExport( utl::MediaDescriptor& rMediaDescriptor ) override;

private:
    virtual StorageRef  implCreateStorage(
                            const css::uno::Reference< css::io::XInputStream >& rxInStream ) const override;
    virtual StorageRef  implCreateStorage(
                            const css::uno::Reference< css::io::XStream >& rxOutStream ) const override;

    void importCustomFragments(css::uno::Reference<css::embed::XStorage> const & xDocumentStorage);

private:
    ::std::unique_ptr< XmlFilterBaseImpl > mxImpl;
    sal_Int32 mnRelId;
    sal_Int32 mnMaxDocId;
    bool mbMSO2007;
protected:
    bool mbMissingExtDrawing;
};

} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
