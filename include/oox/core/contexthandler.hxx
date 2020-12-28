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

#ifndef INCLUDED_OOX_CORE_CONTEXTHANDLER_HXX
#define INCLUDED_OOX_CORE_CONTEXTHANDLER_HXX

#include <memory>
#include <string_view>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/xml/sax/XFastContextHandler.hpp>
#include <cppuhelper/implbase.hxx>
#include <oox/dllapi.h>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace com::sun::star {
    namespace xml::sax { class XFastAttributeList; }
    namespace xml::sax { class XLocator; }
}

namespace oox { class SequenceInputStream; }

namespace oox::core {

class XmlFilterBase;
struct Relation;
class Relations;

class ContextHandler;
typedef ::rtl::Reference< ContextHandler > ContextHandlerRef;

struct FragmentBaseData;
typedef std::shared_ptr< FragmentBaseData > FragmentBaseDataRef;

typedef ::cppu::WeakImplHelper< css::xml::sax::XFastContextHandler > ContextHandler_BASE;

class OOX_DLLPUBLIC ContextHandler : public ContextHandler_BASE
{
public:
    explicit            ContextHandler( const ContextHandler& rParent );
    virtual             ~ContextHandler() override;

    /** Returns the filter instance. */
    XmlFilterBase&      getFilter() const;
    /** Returns the relations of the current fragment. */
    const Relations&    getRelations() const;
    /** Returns the full path of the current fragment. */
    const OUString& getFragmentPath() const;

    /** Returns the full fragment path for the target of the passed relation. */
    OUString     getFragmentPathFromRelation( const Relation& rRelation ) const;
    /** Returns the full fragment path for the passed relation identifier. */
    OUString     getFragmentPathFromRelId( const OUString& rRelId ) const;
    /** Returns the full fragment path for the first relation of the passed type. */
    OUString     getFragmentPathFromFirstType( std::u16string_view rType ) const;
    OUString     getFragmentPathFromFirstTypeFromOfficeDoc( std::u16string_view rType ) const;

    // com.sun.star.xml.sax.XFastContextHandler interface ---------------------

    virtual void SAL_CALL startFastElement( ::sal_Int32 Element, const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs ) override;
    virtual void SAL_CALL startUnknownElement( const OUString& Namespace, const OUString& Name, const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs ) override;
    virtual void SAL_CALL endFastElement( ::sal_Int32 Element ) override;
    virtual void SAL_CALL endUnknownElement( const OUString& Namespace, const OUString& Name ) override;
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs ) override;
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createUnknownChildContext( const OUString& Namespace, const OUString& Name, const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs ) override;
    virtual void SAL_CALL characters( const OUString& aChars ) override;

    // record context interface -----------------------------------------------

    virtual ContextHandlerRef createRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm );
    virtual void        startRecord( sal_Int32 nRecId, SequenceInputStream& rStrm );
    virtual void        endRecord( sal_Int32 nRecId );

protected:
    /** Helper constructor for the FragmentHandler. */
    explicit            ContextHandler( const FragmentBaseDataRef& rxBaseData );

    void                implSetLocator( const css::uno::Reference< css::xml::sax::XLocator >& rxLocator );

#ifdef _MSC_VER
    ContextHandler() {} // workaround
#endif

private:
    ContextHandler&     operator=( const ContextHandler& ) = delete;

private:
    FragmentBaseDataRef mxBaseData;         ///< Base data of the fragment.
};

} // namespace oox::core

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
