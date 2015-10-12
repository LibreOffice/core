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

#include <com/sun/star/xml/sax/XFastContextHandler.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <oox/dllapi.h>
#include <memory>

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XLocator; } }
} } }

namespace oox { class SequenceInputStream; }

namespace oox {
namespace core {

class XmlFilterBase;
class FragmentHandler;
struct Relation;
class Relations;

class ContextHandler;
typedef ::rtl::Reference< ContextHandler > ContextHandlerRef;

struct FragmentBaseData;
typedef std::shared_ptr< FragmentBaseData > FragmentBaseDataRef;

typedef ::cppu::WeakImplHelper< ::com::sun::star::xml::sax::XFastContextHandler > ContextHandler_BASE;

class OOX_DLLPUBLIC ContextHandler : public ContextHandler_BASE
{
public:
    explicit            ContextHandler( const ContextHandler& rParent );
    virtual             ~ContextHandler();

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
    OUString     getFragmentPathFromFirstType( const OUString& rType ) const;
    OUString     getFragmentPathFromFirstTypeFromOfficeDoc( const OUString& rType ) const;

    // com.sun.star.xml.sax.XFastContextHandler interface ---------------------

    virtual void SAL_CALL startFastElement( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL startUnknownElement( const OUString& Namespace, const OUString& Name, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL endFastElement( ::sal_Int32 Element ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL endUnknownElement( const OUString& Namespace, const OUString& Name ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createUnknownChildContext( const OUString& Namespace, const OUString& Name, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL characters( const OUString& aChars ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // record context interface -----------------------------------------------

    virtual ContextHandlerRef createRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm );
    virtual void        startRecord( sal_Int32 nRecId, SequenceInputStream& rStrm );
    virtual void        endRecord( sal_Int32 nRecId );

protected:
    /** Helper constructor for the FragmentHandler. */
    explicit            ContextHandler( const FragmentBaseDataRef& rxBaseData );

    void                implSetLocator( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >& rxLocator );

#ifdef _MSC_VER
    ContextHandler() {} // workaround
#endif

private:
    ContextHandler&     operator=( const ContextHandler& ) = delete;

private:
    FragmentBaseDataRef mxBaseData;         ///< Base data of the fragment.
};

} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
