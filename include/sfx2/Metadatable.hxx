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
#ifndef METADATABLE_HXX
#define METADATABLE_HXX

#include <sal/config.h>

#include <sfx2/dllapi.h>

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/rdf/XMetadatable.hpp>

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>


namespace com { namespace sun { namespace star {
    namespace frame { class XModel; }
} } }

namespace sfx2 {
    class IXmlIdRegistry;
}

namespace sfx2 {

class XmlIdRegistry;
class MetadatableUndo;


// XML ID handling ---------------------------------------------------


/// create a sfx2::XmlIdRegistryDocument or a sfx2::XmlIdRegistryClipboard
SFX2_DLLPUBLIC ::sfx2::IXmlIdRegistry *
createXmlIdRegistry(const bool i_DocIsClipboard);


/** base class for core objects that may have xml:id.

    <p>The interface of this class consists of 3 parts:
    <ul><li>implementations that are used by the <type>MetadatableMixin</type>
            below</li>
        <li>hooks to be called by the sw core whenever actions that are
            relevant to the uniqueness of xml:ids are taken (copying,
            splitting, merging, deletion, undo, etc.)</li>
        <li>abstract methods that are called by the implementation of the
            previous hooks</li></ul>
    </p>
 */
class SFX2_DLLPUBLIC Metadatable : private boost::noncopyable
{

public:
    Metadatable() : m_pReg(0) {}

    // destructor calls RemoveMetadataReference
    virtual ~Metadatable();

    // for MetadatableMixin ----------------------------------------------

    ::com::sun::star::beans::StringPair GetMetadataReference() const;
    void SetMetadataReference(
        const ::com::sun::star::beans::StringPair & i_rReference);
    void EnsureMetadataReference();

    // hooks -------------------------------------------------------------

    // called from dtor!
    void RemoveMetadataReference();

    /** register this as a copy of i_rSource */
    void RegisterAsCopyOf(Metadatable const & i_rSource,
        const bool i_bCopyPrecedesSource = false);

    /** create an Undo Metadatable, which remembers this' reference */
    ::boost::shared_ptr<MetadatableUndo> CreateUndo() const;
    ::boost::shared_ptr<MetadatableUndo> CreateUndoForDelete();

    /** restore this from Undo Metadatable */
    void RestoreMetadata(::boost::shared_ptr<MetadatableUndo> const& i_pUndo);

    /** merge this and i_rOther into this */
    void JoinMetadatable(Metadatable const & i_rOther,
        const bool i_isMergedEmpty, const bool i_isOtherEmpty);

    // abstract methods --------------------------------------------------

    /** get the registry from the SwDoc */
    virtual ::sfx2::IXmlIdRegistry& GetRegistry() = 0;

    /** is this in a clipboard document? */
    virtual bool IsInClipboard() const = 0;

    /** is this in undo array? */
    virtual bool IsInUndo() const = 0;

    /** which stream is this in? true: content.xml; false: styles.xml */
    virtual bool IsInContent() const = 0;

    /** create XMetadatable from this.
        note: if IsInUndo or IsInClipboard return true,
        MakeUnoObject <em>must not</em> be called!
     */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::rdf::XMetadatable > MakeUnoObject() = 0;

private:
    friend class MetadatableClipboard;
    friend class MetadatableUndo;

    // note that Reg may be a XmlIdRegistryDocument or a XmlIdRegistryClipboard
    XmlIdRegistry* m_pReg; // null => no XmlId
};


/** base class for UNO objects that implement <type>XMetadatable</type>.

    <p>An instance of this base class is associated with an instance of
       <type>Metadatable</type>.</p>
 */
class SFX2_DLLPUBLIC MetadatableMixin :
    public ::cppu::WeakImplHelper1<
        ::com::sun::star::rdf::XMetadatable>
{

public:
    MetadatableMixin() {};

    virtual ~MetadatableMixin() {}

    // ::com::sun::star::rdf::XNode:
    virtual OUString SAL_CALL getStringValue()
        throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::rdf::XURI:
    virtual OUString SAL_CALL getLocalName()
        throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getNamespace()
        throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::rdf::XMetadatable:
    virtual ::com::sun::star::beans::StringPair SAL_CALL getMetadataReference()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMetadataReference(
        const ::com::sun::star::beans::StringPair & i_rReference)
        throw (::com::sun::star::uno::RuntimeException,
            ::com::sun::star::lang::IllegalArgumentException);
    virtual void SAL_CALL ensureMetadataReference()
        throw (::com::sun::star::uno::RuntimeException);

protected:
    /// get the core object corresponding to this UNO object.
    virtual Metadatable * GetCoreObject() = 0;
    /// get the <type>XModel</type> for the document
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
        GetModel() = 0;

};

} // namespace sfx2

#endif // METADATABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
