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

#ifndef INCLUDED_SW_INC_FMTMETA_HXX
#define INCLUDED_SW_INC_FMTMETA_HXX

#include "calbck.hxx"

#include <cppuhelper/weakref.hxx>

#include <svl/poolitem.hxx>
#include <sfx2/Metadatable.hxx>

#include <memory>
#include <vector>

namespace com { namespace sun { namespace star {
    namespace document {
        class XDocumentProperties;
    }
    namespace text {
        class XTextField;
    }
}}}

/**
 * The classes that make up a meta entity are:
 * <dl>
 *   <dt>SwTextMeta</dt><dd>the text hint</dd>
 *   <dt>SwFormatMeta</dt><dd>the pool item</dd>
 *   <dt>sw::Meta</dt><dd>the metadatable entity itself</dd>
 *   <dt>SwXMeta</dt><dd>the UNO wrapper object</dd>
 * </dl>
 *
 * The text hint contains the pool item (as usual) and has a pointer to the
 * text node at which it is attached.
 * The pool item has a shared pointer to the metadatable entity, and a reverse
 * pointer to the text attribute at which it is attached.
 * The pool item is non-poolable; it may only be attached to one text
 * attribute.
 * Of all the pool items that refer to a metadatable entity, only one may be
 * in the document content at any time. Others may be in the undo array, or in
 * undo objects.
 * The metadatable entity has a reverse pointer to the pool item that is
 * currently in the document. It also registers as a client at the text node
 * at which it is attached via this pool item and its text attribute.
 * The UNO wrapper object registers as a client at the metadatable entity.
 *
 * Copying the metadatable entity proceeds in the following way:
 * <ol>
 *   <li>The pool item is cloned (because it is non-poolable); the clone
 *       points to the same metadatable entity, but the metadatable entity's
 *       reverse pointer is unchanged.</li>
 *   <li>The DoCopy() method is called at the new pool item:
 *       it will clone the metadatable entity (using RegisterAsCopyOf).
 *       This is necessary, because first, a metadatable entity may
 *       only be inserted once into a document, and second, the copy may be
 *       inserted into a different document than the source document!</li>
 *   <li>A new text hint is created, taking over the new pool item.</li>
 *   <li>The text hint is inserted into the hints array of some text node.</li>
 * </ol>
 */

class SwTextMeta;
class SwXMeta;
class SwXMetaField;
class SwTextNode;
class SwDoc;
namespace sw {
    class Meta;
    class MetaFieldManager;
}

class SwFormatMeta
    : public SfxPoolItem
{
private:
    friend class SwTextMeta; ///< needs SetTextAttr, DoCopy
    friend class ::sw::Meta; ///< needs m_pTextAttr

    std::shared_ptr< ::sw::Meta > m_pMeta;
    SwTextMeta * m_pTextAttr;

    SwTextMeta * GetTextAttr() { return m_pTextAttr; }
    void SetTextAttr(SwTextMeta * const i_pTextAttr);

    /// this method <em>must</em> be called when the hint is actually copied
    void DoCopy(::sw::MetaFieldManager & i_rTargetDocManager,
        SwTextNode & i_rTargetTextNode);

    explicit SwFormatMeta( const sal_uInt16 i_nWhich );

public:
    /// takes ownership
    explicit SwFormatMeta( std::shared_ptr< ::sw::Meta > const & i_pMeta,
                        const sal_uInt16 i_nWhich );
    virtual ~SwFormatMeta() override;

    /// SfxPoolItem
    virtual bool             operator==( const SfxPoolItem & ) const override;
    virtual SfxPoolItem *    Clone( SfxItemPool *pPool = nullptr ) const override;

    /// notify clients registered at m_pMeta that this meta is being (re-)moved
    void NotifyChangeTextNode(SwTextNode *const pTextNode);
    static SwFormatMeta * CreatePoolDefault( const sal_uInt16 i_nWhich );
    ::sw::Meta * GetMeta() { return m_pMeta.get(); }
};

namespace sw {

class Meta
    : public ::sfx2::Metadatable
    , public SwModify
    , public sw::BroadcasterMixin
{
    friend class ::SwFormatMeta; ///< SetFormatMeta, NotifyChangeTextNode
    friend class ::SwXMeta;   ///< GetTextNode, GetTextAttr, Get/SetXMeta

    css::uno::WeakReference<
        css::rdf::XMetadatable> m_wXMeta;

    SwFormatMeta * m_pFormat;
    SwTextNode * m_pTextNode;

protected:

    SwTextMeta * GetTextAttr() const;
    SwTextNode * GetTextNode() const { return m_pTextNode;} ///< @return 0 if not in document (undo)

    SwFormatMeta * GetFormatMeta() const { return m_pFormat; }
    void SetFormatMeta( SwFormatMeta * const i_pFormat ) { m_pFormat = i_pFormat; };

    void NotifyChangeTextNode(SwTextNode *const pTextNode);

    css::uno::WeakReference<css::rdf::XMetadatable> const& GetXMeta() const
            { return m_wXMeta; }
    void SetXMeta(css::uno::Reference<css::rdf::XMetadatable> const& xMeta)
            { m_wXMeta = xMeta; }

    /// SwClient
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew ) override;

public:
    explicit Meta(SwFormatMeta * const i_pFormat);
    virtual ~Meta() override;

    /// sfx2::Metadatable
    virtual ::sfx2::IXmlIdRegistry& GetRegistry() override;
    virtual bool IsInClipboard() const override;
    virtual bool IsInUndo() const override;
    virtual bool IsInContent() const override;
    virtual css::uno::Reference< css::rdf::XMetadatable > MakeUnoObject() override;
};

class MetaField
    : public Meta
{
private:
    friend class ::SwFormatMeta;
    friend class ::SwXMetaField;
    friend class ::sw::MetaFieldManager;

    sal_uInt32 m_nNumberFormat;
    bool       m_bIsFixedLanguage;

    sal_uInt32 GetNumberFormat(OUString const & rContent) const;
    void SetNumberFormat(sal_uInt32 nNumberFormat);
    bool IsFixedLanguage() const    { return m_bIsFixedLanguage; }
    void SetIsFixedLanguage(bool b) { m_bIsFixedLanguage = b; }

    explicit MetaField(SwFormatMeta * const i_pFormat,
            const sal_uInt32 nNumberFormat,
            const bool bIsFixedLanguage );

public:
    /// get prefix/suffix from the RDF repository. @throws RuntimeException
    void GetPrefixAndSuffix(
        OUString *const o_pPrefix, OUString *const o_pSuffix);
};

    /// knows all meta-fields in the document.
class SW_DLLPUBLIC MetaFieldManager
{
private:
    typedef std::vector< std::weak_ptr<MetaField> > MetaFieldList_t;
    MetaFieldList_t m_MetaFields;
    /// Document properties of a clipboard document, empty for non-clipboard documents.
    css::uno::Reference<css::document::XDocumentProperties> m_xDocumentProperties;

    MetaFieldManager(MetaFieldManager const&) = delete;
    MetaFieldManager& operator=(MetaFieldManager const&) = delete;

public:
    MetaFieldManager();
    std::shared_ptr<MetaField> makeMetaField(
                SwFormatMeta * const i_pFormat = nullptr,
                const sal_uInt32 nNumberFormat = SAL_MAX_UINT32,
                const bool bIsFixedLanguage = false );
    /// get all meta fields
    std::vector< css::uno::Reference<css::text::XTextField> > getMetaFields();
    /// Copy document properties from rSource to m_xDocumentProperties.
    void copyDocumentProperties(const SwDoc& rSource);
    const css::uno::Reference<css::document::XDocumentProperties>& getDocumentProperties();
};

} // namespace sw

#endif // INCLUDED_SW_INC_FMTMETA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
