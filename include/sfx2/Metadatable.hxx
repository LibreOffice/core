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
#ifndef INCLUDED_SFX2_METADATABLE_HXX
#define INCLUDED_SFX2_METADATABLE_HXX

#include <sal/config.h>

#include <sfx2/dllapi.h>

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/rdf/XMetadatable.hpp>

#include <memory>


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
    <ul><li>implementations that are used by the MetadatableMixin
            below</li>
        <li>hooks to be called by the sw core whenever actions that are
            relevant to the uniqueness of xml:ids are taken (copying,
            splitting, merging, deletion, undo, etc.)</li>
        <li>abstract methods that are called by the implementation of the
            previous hooks</li></ul>
    </p>
 */
class SFX2_DLLPUBLIC Metadatable
{
public:
    Metadatable() : m_pReg(nullptr) {}

    // destructor calls RemoveMetadataReference
    virtual ~Metadatable();

    // for MetadatableMixin ----------------------------------------------

    css::beans::StringPair GetMetadataReference() const;
    void SetMetadataReference( const css::beans::StringPair & i_rReference);
    void EnsureMetadataReference();

    // hooks -------------------------------------------------------------

    // called from dtor!
    void RemoveMetadataReference();

    /** register this as a copy of i_rSource */
    void RegisterAsCopyOf(Metadatable const & i_rSource,
        const bool i_bCopyPrecedesSource = false);

    /** create an Undo Metadatable, which remembers this' reference */
    std::shared_ptr<MetadatableUndo> CreateUndo() const;
    std::shared_ptr<MetadatableUndo> CreateUndoForDelete();

    /** restore this from Undo Metadatable */
    void RestoreMetadata(std::shared_ptr<MetadatableUndo> const& i_pUndo);

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
    virtual css::uno::Reference< css::rdf::XMetadatable > MakeUnoObject() = 0;

private:
    Metadatable(const Metadatable&) = delete;
    Metadatable& operator=(const Metadatable&) = delete;

    friend class MetadatableClipboard;
    friend class MetadatableUndo;

    // note that Reg may be a XmlIdRegistryDocument or a XmlIdRegistryClipboard
    XmlIdRegistry* m_pReg; // null => no XmlId
};


/** base class for UNO objects that implement XMetadatable.

    <p>An instance of this base class is associated with an instance of
       Metadatable.</p>
 */
class SFX2_DLLPUBLIC MetadatableMixin :
    public ::cppu::WeakImplHelper1<
        css::rdf::XMetadatable>
{

public:
    MetadatableMixin() {};

    virtual ~MetadatableMixin() {}

    // css::rdf::XNode:
    virtual OUString SAL_CALL getStringValue()
        throw (css::uno::RuntimeException, std::exception) override;

    // css::rdf::XURI:
    virtual OUString SAL_CALL getLocalName()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getNamespace()
        throw (css::uno::RuntimeException, std::exception) override;

    // css::rdf::XMetadatable:
    virtual css::beans::StringPair SAL_CALL getMetadataReference()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMetadataReference(
        const css::beans::StringPair & i_rReference)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException, std::exception) override;
    virtual void SAL_CALL ensureMetadataReference()
        throw (css::uno::RuntimeException, std::exception) override;

protected:
    /// get the core object corresponding to this UNO object.
    virtual Metadatable * GetCoreObject() = 0;
    /// get the XModel for the document
    virtual css::uno::Reference< css::frame::XModel >
        GetModel() = 0;

};

} // namespace sfx2

#endif // INCLUDED_SFX2_METADATABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
