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


#include <sfx2/Metadatable.hxx>
#include <sfx2/XmlIdRegistry.hxx>

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx> // solarmutex

#include <rtl/random.h>

#include <boost/bind.hpp>

#include <memory>
#include <boost/unordered_map.hpp>
#include <list>
#include <algorithm>
#if OSL_DEBUG_LEVEL > 0
#include <typeinfo>
#endif


/** XML ID handling.

    There is an abstract base class <type>XmlIdRegistry</type>, with
    2 subclasses <type>XmlIdRegistryDocument</type> for "normal" documents,
    and <type>XmlIdRegistryClipboard</type> for clipboard documents.
    These classes are responsible for managing XML IDs for all elements
    of the model. Only the implementation of the <type>Metadatable</type>
    base class needs to know the registries, so they are not in the header.

    The handling of XML IDs differs between clipboard and non-clipboard
    documents in several aspects. Most importantly, non-clipboard documents
    can have several elements associated with one XML ID.
    This is necessary because of the weird undo implementation:
    deleting a text node moves the deleted node to the undo array, but
    executing undo will then create a <em>copy</em> of that node in the
    document array. These 2 nodes must have the same XML ID, because
    we cannot know whether the user will do a redo next, or something else.

    Because we need to have a mechanism for several objects per XML ID anyway,
    we use that also to enable some usability features:
    The document registry has a list of Metadatables per XML ID.
    This list is sorted by priority, i.e., the first element has highest
    priority. When inserting copies, care must be taken that they are inserted
    at the right position: either before or after the source.
    This is done by <method>Metadatable::RegisterAsCopyOf</method>.
    When a text node is split, then both resulting text nodes are inserted
    into the list. If the user then deletes one text node, the other one
    will have the XML ID.
    Also, when a Metadatable is copied to the clipboard and then pasted,
    the copy is inserted into the list. If the user then deletes the source,
    the XML ID is not lost.
    The goal is that it should be hard to lose an XML ID by accident, which
    is especially important as long as we do not have an UI that displays them.

    There are two subclasses of <type>Metadatable</type>:
    <ul><li><type>MetadatableClipboard</type>: for copies in the clipboard</li>
        <li><type>MetadatableUndo</type>: for undo, because a Metadatable
        may be destroyed on delete and a new one created on undo.</li></ul>
    These serve only to track the position in an XML ID list in a document
    registry, so that future actions can insert objects at the right position.
    Unfortunately, inserting dummy objects seems to be necessary:
    <ul><li>it is not sufficent to just remember the saved id, because then
            the relative priorities might change when executing the undo</li>
        <li>it is not sufficient to record the position as an integer, because
            if we delete a text node and then undo, the node will be copied(!),
            and we will have one more node in the list.<li>
        <li>it is not sufficient to record the pointer of the previous/next
            Metadatable, because if we delete a text node, undo, and then
            do something to clear the redo array, the original text node is
            destroyed, and is replaced by the copy created by undo</li></ul>

    If content from a non-clipboard document is copied into a clipboard
    document, a dummy <type>MetadatableClipboard</type> is inserted into the
    non-clipboard document registry in order to track the position of the
    source element.  When the clipboard content is pasted back into the source
    document, this dummy object is used to associate the pasted element with
    that same XML ID.

    If a <type>Metadatable</type> is deleted or merged,
    <method>Metadatable::CreateUndo</method> is called, and returns a
    <type>MetadatableUndo<type> instance, which can be used to undo the action
    by passing it to <method>Metadatable::RestoreMetadata</method>.

    @author mst
 */


using namespace ::com::sun::star;

using ::sfx2::isValidXmlId;


namespace sfx2 {

static const char s_content [] = "content.xml";
static const char s_styles  [] = "styles.xml";
static const char s_prefix  [] = "id";  // prefix for generated xml:id

static bool isContentFile(OUString const & i_rPath)
{
    return i_rPath == s_content;
}

static bool isStylesFile (OUString const & i_rPath)
{
    return i_rPath == s_styles;
}


//=============================================================================
// XML ID handling ---------------------------------------------------

/** handles registration of XMetadatable.

    This class is responsible for guaranteeing that XMetadatable objects
    always have XML IDs that are unique within a stream.

    This is an abstract base class; see subclasses XmlIdRegistryDocument and
    XmlIdRegistryClipboard.

    @see SwDoc::GetXmlIdRegistry
    @see SwDocShell::GetXmlIdRegistry
 */
class XmlIdRegistry : public sfx2::IXmlIdRegistry
{

public:
    XmlIdRegistry();

    virtual ~XmlIdRegistry();

    /** get the ODF element with the given metadata reference. */
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::rdf::XMetadatable > SAL_CALL
        GetElementByMetadataReference(
            const ::com::sun::star::beans::StringPair & i_rReference) const;

    /** register an ODF element at a newly generated, unique metadata reference.

        <p>
        Find a fresh XML ID, and register it for the element.
        The generated ID does not occur in any stream of the document.
        </p>
     */
    virtual void RegisterMetadatableAndCreateID(Metadatable& i_xObject) = 0;

    /** try to register an ODF element at a given XML ID, or update its
        registation to a different XML ID.

        <p>
        If the given new metadata reference is not already occupied in the
        document, unregister the element at its old metadata reference if
        it has one, and register the new metadata reference for the element.
        Note that this method only ensures that XML IDs are unique per stream,
        so using the same XML ID in both content.xml and styles.xml is allowed.
        </p>

        @returns
            true iff the element has successfully been registered
     */
    virtual bool TryRegisterMetadatable(Metadatable& i_xObject,
        OUString const& i_rStreamName, OUString const& i_rIdref)
        = 0;

    /** unregister an ODF element.

        <p>
        Unregister the element at its metadata reference.
        Does not remove the metadata reference from the element.
        </p>

        @see RemoveXmlIdForElement
     */
    virtual void UnregisterMetadatable(Metadatable const&) = 0;

    /** get the metadata reference for the given element. */
    ::com::sun::star::beans::StringPair
        GetXmlIdForElement(Metadatable const&) const;

    /** remove the metadata reference for the given element. */
    virtual void RemoveXmlIdForElement(Metadatable const&) = 0;

protected:

    virtual bool LookupXmlId(const Metadatable& i_xObject,
        OUString & o_rStream, OUString & o_rIdref) const = 0;

    virtual Metadatable* LookupElement(const OUString & i_rStreamName,
        const OUString & i_rIdref) const = 0;
};

// XmlIdRegistryDocument ---------------------------------------------

/** non-clipboard documents */
class XmlIdRegistryDocument : public XmlIdRegistry
{

public:
    XmlIdRegistryDocument();

    virtual ~XmlIdRegistryDocument();

    virtual void RegisterMetadatableAndCreateID(Metadatable& i_xObject);

    virtual bool TryRegisterMetadatable(Metadatable& i_xObject,
        OUString const& i_rStreamName, OUString const& i_rIdref);

    virtual void UnregisterMetadatable(Metadatable const&);

    virtual void RemoveXmlIdForElement(Metadatable const&);

    /** register i_rCopy as a copy of i_rSource,
        with precedence iff i_bCopyPrecedesSource is true */
    void RegisterCopy(Metadatable const& i_rSource, Metadatable & i_rCopy,
        const bool i_bCopyPrecedesSource);

    /** create a Undo Metadatable for i_rObject. */
    ::boost::shared_ptr<MetadatableUndo> CreateUndo(
        Metadatable const& i_rObject);

    /** merge i_rMerged and i_rOther into i_rMerged. */
    void JoinMetadatables(Metadatable & i_rMerged, Metadatable const& i_rOther);

    // unfortunately public, Metadatable::RegisterAsCopyOf needs this
    virtual bool LookupXmlId(const Metadatable& i_xObject,
        OUString & o_rStream, OUString & o_rIdref) const;

private:

    virtual Metadatable* LookupElement(const OUString & i_rStreamName,
        const OUString & i_rIdref) const;

    struct XmlIdRegistry_Impl;
    ::std::auto_ptr<XmlIdRegistry_Impl> m_pImpl;
};

// MetadatableUndo ---------------------------------------------------

/** the horrible Undo Metadatable: is inserted into lists to track position */
class MetadatableUndo : public Metadatable
{
    /// as determined by the stream of the source in original document
    const bool m_isInContent;
public:
    MetadatableUndo(const bool i_isInContent)
        : m_isInContent(i_isInContent) { }
    virtual ::sfx2::XmlIdRegistry& GetRegistry()
    {
        // N.B. for Undo, m_pReg is initialized by registering this as copy in
        // CreateUndo; it is never cleared
        OSL_ENSURE(m_pReg, "no m_pReg in MetadatableUndo ?");
        return *m_pReg;
    }
    virtual bool IsInClipboard() const { return false; }
    virtual bool IsInUndo() const { return true; }
    virtual bool IsInContent() const { return m_isInContent; }
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::rdf::XMetadatable > MakeUnoObject()
    { OSL_FAIL("MetadatableUndo::MakeUnoObject"); throw; }
};

// MetadatableClipboard ----------------------------------------------

/** the horrible Clipboard Metadatable: inserted into lists to track position */
class MetadatableClipboard : public Metadatable
{
    /// as determined by the stream of the source in original document
    const bool m_isInContent;
public:
    MetadatableClipboard(const bool i_isInContent)
        : m_isInContent(i_isInContent) { }
    virtual ::sfx2::XmlIdRegistry& GetRegistry()
    {
    // N.B. for Clipboard, m_pReg is initialized by registering this as copy in
    // RegisterAsCopyOf; it is only cleared by OriginNoLongerInBusinessAnymore
        OSL_ENSURE(m_pReg, "no m_pReg in MetadatableClipboard ?");
        return *m_pReg;
    }
    virtual bool IsInClipboard() const { return true; }
    virtual bool IsInUndo() const { return false; }
    virtual bool IsInContent() const { return m_isInContent; }
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::rdf::XMetadatable > MakeUnoObject()
    { OSL_FAIL("MetadatableClipboard::MakeUnoObject"); throw; }
    void OriginNoLongerInBusinessAnymore() { m_pReg = 0; }
};

// XmlIdRegistryClipboard --------------------------------------------

class XmlIdRegistryClipboard : public XmlIdRegistry
{

public:
    XmlIdRegistryClipboard();
    virtual ~XmlIdRegistryClipboard();

    virtual void RegisterMetadatableAndCreateID(Metadatable& i_xObject);

    virtual bool TryRegisterMetadatable(Metadatable& i_xObject,
        OUString const& i_rStreamName, OUString const& i_rIdref);

    virtual void UnregisterMetadatable(Metadatable const&);

    virtual void RemoveXmlIdForElement(Metadatable const&);

    /** register i_rCopy as a copy of i_rSource */
    MetadatableClipboard & RegisterCopyClipboard(Metadatable & i_rCopy,
        beans::StringPair const & i_rReference,
        const bool i_isLatent);

    /** get the Metadatable that links i_rObject to its origin registry */
    MetadatableClipboard const* SourceLink(Metadatable const& i_rObject);

private:
    virtual bool LookupXmlId(const Metadatable& i_xObject,
        OUString & o_rStream, OUString & o_rIdref) const;

    virtual Metadatable* LookupElement(const OUString & i_rStreamName,
        const OUString & i_rIdref) const;

    /** create a Clipboard Metadatable for i_rObject. */
    ::boost::shared_ptr<MetadatableClipboard> CreateClipboard(
        const bool i_isInContent);

    struct XmlIdRegistry_Impl;
    ::std::auto_ptr<XmlIdRegistry_Impl> m_pImpl;
};


//=============================================================================
// XmlIdRegistry

::sfx2::IXmlIdRegistry * createXmlIdRegistry(const bool i_DocIsClipboard)
{
    return i_DocIsClipboard
        ? static_cast<XmlIdRegistry*>( new XmlIdRegistryClipboard )
        : static_cast<XmlIdRegistry*>( new XmlIdRegistryDocument );
}

XmlIdRegistry::XmlIdRegistry()
{
}

XmlIdRegistry::~XmlIdRegistry()
{
}

::com::sun::star::uno::Reference< ::com::sun::star::rdf::XMetadatable > SAL_CALL
XmlIdRegistry::GetElementByMetadataReference(
    const beans::StringPair & i_rReference) const
{
    Metadatable* pObject( LookupElement(i_rReference.First,
        i_rReference.Second) );
    return pObject ? pObject->MakeUnoObject() : 0;
}

beans::StringPair
XmlIdRegistry::GetXmlIdForElement(const Metadatable& i_rObject) const
{
    OUString path;
    OUString idref;
    if (LookupXmlId(i_rObject, path, idref))
    {
        if (LookupElement(path, idref) == &i_rObject)
        {
            return beans::StringPair(path, idref);
        }
    }
    return beans::StringPair();
}


/// generate unique xml:id
template< typename T >
/*static*/ OUString create_id(const
    ::boost::unordered_map< OUString, T, OUStringHash > & i_rXmlIdMap)
{
    static rtlRandomPool s_Pool( rtl_random_createPool() );
    const OUString prefix(s_prefix);
    typename ::boost::unordered_map< OUString, T, OUStringHash >
        ::const_iterator iter;
    OUString id;
    do
    {
        sal_Int32 n;
        rtl_random_getBytes(s_Pool, & n, sizeof(n));
        id = prefix + OUString::number(abs(n));
        iter = i_rXmlIdMap.find(id);
    }
    while (iter != i_rXmlIdMap.end());
    return id;
}

//=============================================================================
// Document XML ID Registry (_Impl)

/// element list
typedef ::std::list< Metadatable* > XmlIdList_t;

/// Idref -> (content.xml element list, styles.xml element list)
typedef ::boost::unordered_map< OUString,
    ::std::pair< XmlIdList_t, XmlIdList_t >, OUStringHash > XmlIdMap_t;

/// pointer hash template
template<typename T> struct PtrHash
{
    size_t operator() (T const * i_pT) const
    {
        return reinterpret_cast<size_t>(i_pT);
    }
};

/// element -> (stream name, idref)
typedef ::boost::unordered_map< const Metadatable*,
    ::std::pair< OUString, OUString>, PtrHash<Metadatable> >
    XmlIdReverseMap_t;

struct XmlIdRegistryDocument::XmlIdRegistry_Impl
{
    XmlIdRegistry_Impl()
        : m_XmlIdMap(), m_XmlIdReverseMap() { }

    bool TryInsertMetadatable(Metadatable& i_xObject,
        const OUString & i_rStream, const OUString & i_rIdref);

    bool LookupXmlId(const Metadatable& i_xObject,
        OUString & o_rStream, OUString & o_rIdref) const;

    Metadatable* LookupElement(const OUString & i_rStreamName,
        const OUString & i_rIdref) const;

    const XmlIdList_t * LookupElementList(
        const OUString & i_rStreamName,
        const OUString & i_rIdref) const;

          XmlIdList_t * LookupElementList(
        const OUString & i_rStreamName,
        const OUString & i_rIdref)
    {
        return const_cast<XmlIdList_t*>(
            const_cast<const XmlIdRegistry_Impl*>(this)
                ->LookupElementList(i_rStreamName, i_rIdref));
    }

    XmlIdMap_t m_XmlIdMap;
    XmlIdReverseMap_t m_XmlIdReverseMap;
};

// -------------------------------------------------------------------

static void
rmIter(XmlIdMap_t & i_rXmlIdMap, XmlIdMap_t::iterator const& i_rIter,
    OUString const & i_rStream, Metadatable const& i_rObject)
{
    if (i_rIter != i_rXmlIdMap.end())
    {
        XmlIdList_t & rList( isContentFile(i_rStream)
            ? i_rIter->second.first : i_rIter->second.second );
        rList.remove(&const_cast<Metadatable&>(i_rObject));
        if (i_rIter->second.first.empty() && i_rIter->second.second.empty())
        {
            i_rXmlIdMap.erase(i_rIter);
        }
    }
}

// -------------------------------------------------------------------

const XmlIdList_t *
XmlIdRegistryDocument::XmlIdRegistry_Impl::LookupElementList(
    const OUString & i_rStreamName,
    const OUString & i_rIdref) const
{
    const XmlIdMap_t::const_iterator iter( m_XmlIdMap.find(i_rIdref) );
    if (iter != m_XmlIdMap.end())
    {
        OSL_ENSURE(!iter->second.first.empty() || !iter->second.second.empty(),
            "null entry in m_XmlIdMap");
        return (isContentFile(i_rStreamName))
            ?  &iter->second.first
            :  &iter->second.second;
    }
    else
    {
        return 0;
    }
}

Metadatable*
XmlIdRegistryDocument::XmlIdRegistry_Impl::LookupElement(
    const OUString & i_rStreamName,
    const OUString & i_rIdref) const
{
    if (!isValidXmlId(i_rStreamName, i_rIdref))
    {
        throw lang::IllegalArgumentException(OUString(
            "illegal XmlId"), 0, 0);
    }

    const XmlIdList_t * pList( LookupElementList(i_rStreamName, i_rIdref) );
    if (pList)
    {
        const XmlIdList_t::const_iterator iter(
            ::std::find_if(pList->begin(), pList->end(),
                ::boost::bind(
                    ::std::logical_not<bool>(),
                        ::boost::bind(
                            ::std::logical_or<bool>(),
                                ::boost::bind( &Metadatable::IsInUndo, _1 ),
                                ::boost::bind( &Metadatable::IsInClipboard, _1 )
            ) ) ) );
        if (iter != pList->end())
        {
            return *iter;
        }
    }
    return 0;
}

bool
XmlIdRegistryDocument::XmlIdRegistry_Impl::LookupXmlId(
    const Metadatable& i_rObject,
    OUString & o_rStream, OUString & o_rIdref) const
{
    const XmlIdReverseMap_t::const_iterator iter(
        m_XmlIdReverseMap.find(&i_rObject) );
    if (iter != m_XmlIdReverseMap.end())
    {
        OSL_ENSURE(!iter->second.first.isEmpty(),
            "null stream in m_XmlIdReverseMap");
        OSL_ENSURE(!iter->second.second.isEmpty(),
            "null id in m_XmlIdReverseMap");
        o_rStream = iter->second.first;
        o_rIdref  = iter->second.second;
        return true;
    }
    else
    {
        return false;
    }
}

bool
XmlIdRegistryDocument::XmlIdRegistry_Impl::TryInsertMetadatable(
    Metadatable & i_rObject,
    const OUString & i_rStreamName, const OUString & i_rIdref)
{
    const bool bContent( isContentFile(i_rStreamName) );
    OSL_ENSURE(isContentFile(i_rStreamName) || isStylesFile(i_rStreamName),
        "invalid stream");

    XmlIdList_t * pList( LookupElementList(i_rStreamName, i_rIdref) );
    if (pList)
    {
        if (pList->empty())
        {
            pList->push_back( &i_rObject );
            return true;
        }
        else
        {
            // this is only called from TryRegister now, so check
            // if all elements in the list are deleted (in undo) or
            // placeholders, then "steal" the id from them
            if ( pList->end() == ::std::find_if(pList->begin(), pList->end(),
                ::boost::bind(
                    ::std::logical_not<bool>(),
                        ::boost::bind(
                            ::std::logical_or<bool>(),
                                ::boost::bind( &Metadatable::IsInUndo, _1 ),
                                ::boost::bind( &Metadatable::IsInClipboard, _1 )
                ) ) ) )
            {
                pList->push_front( &i_rObject );
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    else
    {
        m_XmlIdMap.insert(::std::make_pair(i_rIdref, bContent
            ? ::std::make_pair( XmlIdList_t( 1, &i_rObject ), XmlIdList_t() )
            : ::std::make_pair( XmlIdList_t(), XmlIdList_t( 1, &i_rObject ) )));
        return true;
    }
}

//=============================================================================
// Document XML ID Registry


XmlIdRegistryDocument::XmlIdRegistryDocument()
    :   m_pImpl( new XmlIdRegistry_Impl )
{
}

static void
removeLink(Metadatable* i_pObject)
{
    OSL_ENSURE(i_pObject, "null in list ???");
    if (!i_pObject) return;
    if (i_pObject->IsInClipboard())
    {
        MetadatableClipboard* pLink(
            dynamic_cast<MetadatableClipboard*>( i_pObject ) );
        OSL_ENSURE(pLink, "IsInClipboard, but no MetadatableClipboard ?");
        if (pLink)
        {
            pLink->OriginNoLongerInBusinessAnymore();
        }
    }
}

XmlIdRegistryDocument::~XmlIdRegistryDocument()
{
    // notify all list elements that are actually in the clipboard
    for (XmlIdMap_t::iterator iter(m_pImpl->m_XmlIdMap.begin());
        iter != m_pImpl->m_XmlIdMap.end(); ++iter)
    {
        ::std::for_each(iter->second.first.begin(), iter->second.first.end(),
            removeLink);
        ::std::for_each(iter->second.second.begin(), iter->second.second.end(),
            removeLink);
    }
}

bool
XmlIdRegistryDocument::LookupXmlId(
    const Metadatable& i_rObject,
    OUString & o_rStream, OUString & o_rIdref) const
{
    return m_pImpl->LookupXmlId(i_rObject, o_rStream, o_rIdref);
}

Metadatable*
XmlIdRegistryDocument::LookupElement(
    const OUString & i_rStreamName,
    const OUString & i_rIdref) const
{
    return m_pImpl->LookupElement(i_rStreamName, i_rIdref);
}

bool
XmlIdRegistryDocument::TryRegisterMetadatable(Metadatable & i_rObject,
    OUString const& i_rStreamName, OUString const& i_rIdref)
{
    OSL_TRACE("TryRegisterMetadatable: %p (%s#%s)\n", &i_rObject,
        OUStringToOString(i_rStreamName, RTL_TEXTENCODING_UTF8).getStr(),
        OUStringToOString(i_rIdref, RTL_TEXTENCODING_UTF8).getStr());

    OSL_ENSURE(!dynamic_cast<MetadatableUndo*>(&i_rObject),
        "TryRegisterMetadatable called for MetadatableUndo?");
    OSL_ENSURE(!dynamic_cast<MetadatableClipboard*>(&i_rObject),
        "TryRegisterMetadatable called for MetadatableClipboard?");

    if (!isValidXmlId(i_rStreamName, i_rIdref))
    {
        throw lang::IllegalArgumentException(OUString(
            "illegal XmlId"), 0, 0);
    }
    if (i_rObject.IsInContent()
        ?   !isContentFile(i_rStreamName)
        :   !isStylesFile(i_rStreamName))
    {
        throw lang::IllegalArgumentException(OUString(
            "illegal XmlId: wrong stream"), 0, 0);
    }

    OUString old_path;
    OUString old_idref;
    m_pImpl->LookupXmlId(i_rObject, old_path, old_idref);
    if (old_path  == i_rStreamName && old_idref == i_rIdref)
    {
        return (m_pImpl->LookupElement(old_path, old_idref) == &i_rObject);
    }
    XmlIdMap_t::iterator old_id( m_pImpl->m_XmlIdMap.end() );
    if (!old_idref.isEmpty())
    {
        old_id = m_pImpl->m_XmlIdMap.find(old_idref);
        OSL_ENSURE(old_id != m_pImpl->m_XmlIdMap.end(), "old id not found");
    }
    if (m_pImpl->TryInsertMetadatable(i_rObject, i_rStreamName, i_rIdref))
    {
        rmIter(m_pImpl->m_XmlIdMap, old_id, old_path, i_rObject);
        m_pImpl->m_XmlIdReverseMap[&i_rObject] =
            ::std::make_pair(i_rStreamName, i_rIdref);
        return true;
    }
    else
    {
        return false;
    }
}

void
XmlIdRegistryDocument::RegisterMetadatableAndCreateID(Metadatable & i_rObject)
{
    OSL_TRACE("RegisterMetadatableAndCreateID: %p", &i_rObject);

    OSL_ENSURE(!dynamic_cast<MetadatableUndo*>(&i_rObject),
        "RegisterMetadatableAndCreateID called for MetadatableUndo?");
    OSL_ENSURE(!dynamic_cast<MetadatableClipboard*>(&i_rObject),
        "RegisterMetadatableAndCreateID called for MetadatableClipboard?");

    const bool isInContent( i_rObject.IsInContent() );
    const OUString stream( OUString::createFromAscii(
        isInContent ? s_content : s_styles ) );
    // check if we have a latent xmlid, and if yes, remove it
    OUString old_path;
    OUString old_idref;
    m_pImpl->LookupXmlId(i_rObject, old_path, old_idref);

    XmlIdMap_t::iterator old_id( m_pImpl->m_XmlIdMap.end() );
    if (!old_idref.isEmpty())
    {
        old_id = m_pImpl->m_XmlIdMap.find(old_idref);
        OSL_ENSURE(old_id != m_pImpl->m_XmlIdMap.end(), "old id not found");
        if (m_pImpl->LookupElement(old_path, old_idref) == &i_rObject)
        {
            return;
        }
        else
        {
            // remove latent xmlid
            rmIter(m_pImpl->m_XmlIdMap, old_id, old_path, i_rObject);
        }
    }

    // create id
    const OUString id( create_id(m_pImpl->m_XmlIdMap) );
    OSL_ENSURE(m_pImpl->m_XmlIdMap.find(id) == m_pImpl->m_XmlIdMap.end(),
        "created id is in use");
    m_pImpl->m_XmlIdMap.insert(::std::make_pair(id, isInContent
        ? ::std::make_pair( XmlIdList_t( 1, &i_rObject ), XmlIdList_t() )
        : ::std::make_pair( XmlIdList_t(), XmlIdList_t( 1, &i_rObject ) )));
    m_pImpl->m_XmlIdReverseMap[&i_rObject] = ::std::make_pair(stream, id);
}

void XmlIdRegistryDocument::UnregisterMetadatable(const Metadatable& i_rObject)
{
    OSL_TRACE("UnregisterMetadatable: %p", &i_rObject);

    OUString path;
    OUString idref;
    if (!m_pImpl->LookupXmlId(i_rObject, path, idref))
    {
        OSL_FAIL("unregister: no xml id?");
        return;
    }
    const XmlIdMap_t::iterator iter( m_pImpl->m_XmlIdMap.find(idref) );
    if (iter != m_pImpl->m_XmlIdMap.end())
    {
        rmIter(m_pImpl->m_XmlIdMap, iter, path, i_rObject);
    }
}

void XmlIdRegistryDocument::RemoveXmlIdForElement(const Metadatable& i_rObject)
{
    OSL_TRACE("RemoveXmlIdForElement: %p", &i_rObject);

    const XmlIdReverseMap_t::iterator iter(
        m_pImpl->m_XmlIdReverseMap.find(&i_rObject) );
    if (iter != m_pImpl->m_XmlIdReverseMap.end())
    {
        OSL_ENSURE(!iter->second.second.isEmpty(),
            "null id in m_XmlIdReverseMap");
        m_pImpl->m_XmlIdReverseMap.erase(iter);
    }
}

// -------------------------------------------------------------------

void XmlIdRegistryDocument::RegisterCopy(Metadatable const& i_rSource,
    Metadatable & i_rCopy, const bool i_bCopyPrecedesSource)
{
    OSL_TRACE("RegisterCopy: %p -> %p (%d)\n",
        &i_rSource, &i_rCopy, i_bCopyPrecedesSource);

    // potential sources: clipboard, undo array, splitNode
    // assumption: stream change can only happen via clipboard, and is handled
    // by Metadatable::RegisterAsCopyOf
    OSL_ENSURE(i_rSource.IsInUndo() || i_rCopy.IsInUndo() ||
        (i_rSource.IsInContent() == i_rCopy.IsInContent()),
        "RegisterCopy: not in same stream?");

    OUString path;
    OUString idref;
    if (!m_pImpl->LookupXmlId( i_rSource, path, idref ))
    {
        OSL_FAIL("no xml id?");
        return;
    }
    XmlIdList_t * pList ( m_pImpl->LookupElementList(path, idref) );
    OSL_ENSURE( ::std::find( pList->begin(), pList->end(), &i_rCopy )
        == pList->end(), "copy already registered???");
    XmlIdList_t::iterator srcpos(
        ::std::find( pList->begin(), pList->end(), &i_rSource ) );
    OSL_ENSURE(srcpos != pList->end(), "source not in list???");
    if (srcpos == pList->end())
    {
        return;
    }
    if (i_bCopyPrecedesSource)
    {
        pList->insert( srcpos, &i_rCopy );
    }
    else
    {
        // for undo push_back does not work! must insert right after source
        pList->insert( ++srcpos, &i_rCopy );
    }
    m_pImpl->m_XmlIdReverseMap.insert(::std::make_pair(&i_rCopy,
        ::std::make_pair(path, idref)));
}

::boost::shared_ptr<MetadatableUndo>
XmlIdRegistryDocument::CreateUndo(Metadatable const& i_rObject)
{
    OSL_TRACE("CreateUndo: %p", &i_rObject);

    return ::boost::shared_ptr<MetadatableUndo>(
                new MetadatableUndo(i_rObject.IsInContent()) );
}

/*
i_rMerged is both a source and the target node of the merge
i_rOther is the other source, and will be deleted after the merge

dimensions: none|latent|actual empty|nonempty
i_rMerged(1)    i_rOther(2)        result
     *|empty         *|empty    => 1|2 (arbitrary)
     *|empty         *|nonempty => 2
     *|nonempty      *|empty    => 1
  none|nonempty   none|nonempty => none
  none|nonempty latent|nonempty => 2
latent|nonempty   none|nonempty => 1
latent|nonempty latent|nonempty => 1|2
     *|nonempty actual|nonempty => 2
actual|nonempty      *|nonempty => 1
actual|nonempty actual|nonempty => 1|2
*/
void
XmlIdRegistryDocument::JoinMetadatables(
    Metadatable & i_rMerged, Metadatable const & i_rOther)
{
    OSL_TRACE("JoinMetadatables: %p <- %p", &i_rMerged, &i_rOther);

    bool mergedOwnsRef;
    OUString path;
    OUString idref;
    if (m_pImpl->LookupXmlId(i_rMerged, path, idref))
    {
        mergedOwnsRef = (m_pImpl->LookupElement(path, idref) == &i_rMerged);
    }
    else
    {
        OSL_FAIL("JoinMetadatables: no xmlid?");
        return;
    }
    if (!mergedOwnsRef)
    {
        i_rMerged.RemoveMetadataReference();
        i_rMerged.RegisterAsCopyOf(i_rOther, true);
        return;
    }
    // other cases: merged has actual ref and is nonempty,
    // other has latent/actual ref and is nonempty: other loses => nothing to do
}


//=============================================================================
// Clipboard XML ID Registry (_Impl)

struct RMapEntry
{
    RMapEntry() : m_pLink() { }
    RMapEntry(OUString const& i_rStream,
            OUString const& i_rXmlId,
            ::boost::shared_ptr<MetadatableClipboard> const& i_pLink
                = ::boost::shared_ptr<MetadatableClipboard>())
        :   m_Stream(i_rStream), m_XmlId(i_rXmlId), m_pLink(i_pLink)
        {}
    OUString m_Stream;
    OUString m_XmlId;
    // this would have been an auto_ptr, if only that would have compiled...
    ::boost::shared_ptr<MetadatableClipboard> m_pLink;
};

/// element -> (stream name, idref, source)
typedef ::boost::unordered_map< const Metadatable*,
    struct RMapEntry,
    PtrHash<Metadatable> >
    ClipboardXmlIdReverseMap_t;

/// Idref -> (content.xml element, styles.xml element)
typedef ::boost::unordered_map< OUString,
    ::std::pair< Metadatable*, Metadatable* >, OUStringHash >
    ClipboardXmlIdMap_t;

struct XmlIdRegistryClipboard::XmlIdRegistry_Impl
{
    XmlIdRegistry_Impl()
        : m_XmlIdMap(), m_XmlIdReverseMap() { }

    bool TryInsertMetadatable(Metadatable& i_xObject,
        const OUString & i_rStream, const OUString & i_rIdref);

    bool LookupXmlId(const Metadatable& i_xObject,
        OUString & o_rStream, OUString & o_rIdref,
        MetadatableClipboard const* &o_rpLink) const;

    Metadatable* LookupElement(const OUString & i_rStreamName,
        const OUString & i_rIdref) const;

    Metadatable* const* LookupEntry(const OUString & i_rStreamName,
        const OUString & i_rIdref) const;

    Metadatable*      * LookupEntry(const OUString & i_rStreamName,
        const OUString & i_rIdref)
    {
        return const_cast<Metadatable**>(
            const_cast<const XmlIdRegistry_Impl*>(this)
                ->LookupEntry(i_rStreamName, i_rIdref));
    }

    ClipboardXmlIdMap_t m_XmlIdMap;
    ClipboardXmlIdReverseMap_t m_XmlIdReverseMap;
};

// -------------------------------------------------------------------

static void
rmIter(ClipboardXmlIdMap_t & i_rXmlIdMap,
    ClipboardXmlIdMap_t::iterator const& i_rIter,
    OUString const & i_rStream, Metadatable const& i_rObject)
{
    if (i_rIter != i_rXmlIdMap.end())
    {
        Metadatable *& rMeta = isContentFile(i_rStream)
            ? i_rIter->second.first : i_rIter->second.second;
        if (rMeta == &i_rObject)
        {
            rMeta = 0;
        }
        if (!i_rIter->second.first && !i_rIter->second.second)
        {
            i_rXmlIdMap.erase(i_rIter);
        }
    }
}

// -------------------------------------------------------------------

Metadatable* const*
XmlIdRegistryClipboard::XmlIdRegistry_Impl::LookupEntry(
    const OUString & i_rStreamName,
    const OUString & i_rIdref) const
{
    if (!isValidXmlId(i_rStreamName, i_rIdref))
    {
        throw lang::IllegalArgumentException(OUString(
            "illegal XmlId"), 0, 0);
    }

    const ClipboardXmlIdMap_t::const_iterator iter( m_XmlIdMap.find(i_rIdref) );
    if (iter != m_XmlIdMap.end())
    {
        OSL_ENSURE(iter->second.first || iter->second.second,
            "null entry in m_XmlIdMap");
        return (isContentFile(i_rStreamName))
            ?  &iter->second.first
            :  &iter->second.second;
    }
    else
    {
        return 0;
    }
}

Metadatable*
XmlIdRegistryClipboard::XmlIdRegistry_Impl::LookupElement(
    const OUString & i_rStreamName,
    const OUString & i_rIdref) const
{
    Metadatable * const * ppEntry = LookupEntry(i_rStreamName, i_rIdref);
    return ppEntry ? *ppEntry : 0;
}

bool
XmlIdRegistryClipboard::XmlIdRegistry_Impl::LookupXmlId(
    const Metadatable& i_rObject,
    OUString & o_rStream, OUString & o_rIdref,
    MetadatableClipboard const* &o_rpLink) const
{
    const ClipboardXmlIdReverseMap_t::const_iterator iter(
        m_XmlIdReverseMap.find(&i_rObject) );
    if (iter != m_XmlIdReverseMap.end())
    {
        OSL_ENSURE(!iter->second.m_Stream.isEmpty(),
            "null stream in m_XmlIdReverseMap");
        OSL_ENSURE(!iter->second.m_XmlId.isEmpty(),
            "null id in m_XmlIdReverseMap");
        o_rStream = iter->second.m_Stream;
        o_rIdref  = iter->second.m_XmlId;
        o_rpLink  = iter->second.m_pLink.get();
        return true;
    }
    else
    {
        return false;
    }
}

bool
XmlIdRegistryClipboard::XmlIdRegistry_Impl::TryInsertMetadatable(
    Metadatable & i_rObject,
    const OUString & i_rStreamName, const OUString & i_rIdref)
{
    bool bContent( isContentFile(i_rStreamName) );
    OSL_ENSURE(isContentFile(i_rStreamName) || isStylesFile(i_rStreamName),
        "invalid stream");

    Metadatable ** ppEntry = LookupEntry(i_rStreamName, i_rIdref);
    if (ppEntry)
    {
        if (*ppEntry)
        {
            return false;
        }
        else
        {
            *ppEntry = &i_rObject;
            return true;
        }
    }
    else
    {
        m_XmlIdMap.insert(::std::make_pair(i_rIdref, bContent
            ? ::std::make_pair( &i_rObject, static_cast<Metadatable*>(0) )
            : ::std::make_pair( static_cast<Metadatable*>(0), &i_rObject )));
        return true;
    }
}

//=============================================================================
// Clipboard XML ID Registry


XmlIdRegistryClipboard::XmlIdRegistryClipboard()
    :   m_pImpl( new XmlIdRegistry_Impl )
{
}

XmlIdRegistryClipboard::~XmlIdRegistryClipboard()
{
}

bool
XmlIdRegistryClipboard::LookupXmlId(
    const Metadatable& i_rObject,
    OUString & o_rStream, OUString & o_rIdref) const
{
    const MetadatableClipboard * pLink;
    return m_pImpl->LookupXmlId(i_rObject, o_rStream, o_rIdref, pLink);
}

Metadatable*
XmlIdRegistryClipboard::LookupElement(
    const OUString & i_rStreamName,
    const OUString & i_rIdref) const
{
    return m_pImpl->LookupElement(i_rStreamName, i_rIdref);
}

bool
XmlIdRegistryClipboard::TryRegisterMetadatable(Metadatable & i_rObject,
    OUString const& i_rStreamName, OUString const& i_rIdref)
{
    OSL_TRACE("TryRegisterMetadatable: %p (%s#%s)\n", &i_rObject,
        OUStringToOString(i_rStreamName, RTL_TEXTENCODING_UTF8).getStr(),
        OUStringToOString(i_rIdref, RTL_TEXTENCODING_UTF8).getStr());

    OSL_ENSURE(!dynamic_cast<MetadatableUndo*>(&i_rObject),
        "TryRegisterMetadatable called for MetadatableUndo?");
    OSL_ENSURE(!dynamic_cast<MetadatableClipboard*>(&i_rObject),
        "TryRegisterMetadatable called for MetadatableClipboard?");

    if (!isValidXmlId(i_rStreamName, i_rIdref))
    {
        throw lang::IllegalArgumentException(OUString(
            "illegal XmlId"), 0, 0);
    }
    if (i_rObject.IsInContent()
        ?   !isContentFile(i_rStreamName)
        :   !isStylesFile(i_rStreamName))
    {
        throw lang::IllegalArgumentException(OUString(
            "illegal XmlId: wrong stream"), 0, 0);
    }

    OUString old_path;
    OUString old_idref;
    const MetadatableClipboard * pLink;
    m_pImpl->LookupXmlId(i_rObject, old_path, old_idref, pLink);
    if (old_path  == i_rStreamName && old_idref == i_rIdref)
    {
        return (m_pImpl->LookupElement(old_path, old_idref) == &i_rObject);
    }
    ClipboardXmlIdMap_t::iterator old_id( m_pImpl->m_XmlIdMap.end() );
    if (!old_idref.isEmpty())
    {
        old_id = m_pImpl->m_XmlIdMap.find(old_idref);
        OSL_ENSURE(old_id != m_pImpl->m_XmlIdMap.end(), "old id not found");
    }
    if (m_pImpl->TryInsertMetadatable(i_rObject, i_rStreamName, i_rIdref))
    {
        rmIter(m_pImpl->m_XmlIdMap, old_id, old_path, i_rObject);
        m_pImpl->m_XmlIdReverseMap[&i_rObject] =
            RMapEntry(i_rStreamName, i_rIdref);
        return true;
    }
    else
    {
        return false;
    }
}

void
XmlIdRegistryClipboard::RegisterMetadatableAndCreateID(Metadatable & i_rObject)
{
    OSL_TRACE("RegisterMetadatableAndCreateID: %p", &i_rObject);

    OSL_ENSURE(!dynamic_cast<MetadatableUndo*>(&i_rObject),
        "RegisterMetadatableAndCreateID called for MetadatableUndo?");
    OSL_ENSURE(!dynamic_cast<MetadatableClipboard*>(&i_rObject),
        "RegisterMetadatableAndCreateID called for MetadatableClipboard?");

    bool isInContent( i_rObject.IsInContent() );
    OUString stream( OUString::createFromAscii(
        isInContent ? s_content : s_styles ) );

    OUString old_path;
    OUString old_idref;
    LookupXmlId(i_rObject, old_path, old_idref);
    if (!old_idref.isEmpty() &&
        (m_pImpl->LookupElement(old_path, old_idref) == &i_rObject))
    {
        return;
    }

    // create id
    const OUString id( create_id(m_pImpl->m_XmlIdMap) );
    OSL_ENSURE(m_pImpl->m_XmlIdMap.find(id) == m_pImpl->m_XmlIdMap.end(),
        "created id is in use");
    m_pImpl->m_XmlIdMap.insert(::std::make_pair(id, isInContent
        ? ::std::make_pair( &i_rObject, static_cast<Metadatable*>(0) )
        : ::std::make_pair( static_cast<Metadatable*>(0), &i_rObject )));
    // N.B.: if i_rObject had a latent XmlId, then we implicitly delete the
    // MetadatableClipboard and thus the latent XmlId here
    m_pImpl->m_XmlIdReverseMap[&i_rObject] = RMapEntry(stream, id);
}

void XmlIdRegistryClipboard::UnregisterMetadatable(const Metadatable& i_rObject)
{
    OSL_TRACE("UnregisterMetadatable: %p", &i_rObject);

    OUString path;
    OUString idref;
    const MetadatableClipboard * pLink;
    if (!m_pImpl->LookupXmlId(i_rObject, path, idref, pLink))
    {
        OSL_FAIL("unregister: no xml id?");
        return;
    }
    const ClipboardXmlIdMap_t::iterator iter( m_pImpl->m_XmlIdMap.find(idref) );
    if (iter != m_pImpl->m_XmlIdMap.end())
    {
        rmIter(m_pImpl->m_XmlIdMap, iter, path, i_rObject);
    }
}


void XmlIdRegistryClipboard::RemoveXmlIdForElement(const Metadatable& i_rObject)
{
    OSL_TRACE("RemoveXmlIdForElement: %p", &i_rObject);

    ClipboardXmlIdReverseMap_t::iterator iter(
        m_pImpl->m_XmlIdReverseMap.find(&i_rObject) );
    if (iter != m_pImpl->m_XmlIdReverseMap.end())
    {
        OSL_ENSURE(!iter->second.m_XmlId.isEmpty(),
            "null id in m_XmlIdReverseMap");
        m_pImpl->m_XmlIdReverseMap.erase(iter);
    }
}

// -------------------------------------------------------------------

::boost::shared_ptr<MetadatableClipboard>
XmlIdRegistryClipboard::CreateClipboard(const bool i_isInContent)
{
    OSL_TRACE("CreateClipboard:");

    return ::boost::shared_ptr<MetadatableClipboard>(
        new MetadatableClipboard(i_isInContent) );
}

MetadatableClipboard &
XmlIdRegistryClipboard::RegisterCopyClipboard(Metadatable & i_rCopy,
    beans::StringPair const & i_rReference,
    const bool i_isLatent)
{
    OSL_TRACE("RegisterCopyClipboard: %p -> "/*"%p"*/"(%s#%s) (%d)\n",
        /*&i_rSource,*/ &i_rCopy,
        OUStringToOString(i_rReference.First,
            RTL_TEXTENCODING_UTF8).getStr(),
        OUStringToOString(i_rReference.Second,
            RTL_TEXTENCODING_UTF8).getStr(),
        i_isLatent);

    // N.B.: when copying to the clipboard, the selection is always inserted
    //       into the body, even if the source is a header/footer!
    //       so we do not check whether the stream is right in this function

    if (!isValidXmlId(i_rReference.First, i_rReference.Second))
    {
        throw lang::IllegalArgumentException(OUString(
            "illegal XmlId"), 0, 0);
    }

    if (!i_isLatent)
    {
        // this should succeed assuming clipboard has a single source document
        const bool success( m_pImpl->TryInsertMetadatable(i_rCopy,
                i_rReference.First, i_rReference.Second) );
        OSL_ENSURE(success, "RegisterCopyClipboard: TryInsert failed?");
        (void) success;
    }
    const ::boost::shared_ptr<MetadatableClipboard> pLink(
        CreateClipboard( isContentFile(i_rReference.First)) );
    m_pImpl->m_XmlIdReverseMap.insert(::std::make_pair(&i_rCopy,
        RMapEntry(i_rReference.First, i_rReference.Second, pLink)));
    return *pLink.get();
}

MetadatableClipboard const*
XmlIdRegistryClipboard::SourceLink(Metadatable const& i_rObject)
{
    OUString path;
    OUString idref;
    const MetadatableClipboard * pLink( 0 );
    m_pImpl->LookupXmlId(i_rObject, path, idref, pLink);
    return pLink;
}


//=============================================================================
// Metadatable mixin


Metadatable::~Metadatable()
{
    RemoveMetadataReference();
}

void Metadatable::RemoveMetadataReference()
{
    try
    {
        if (m_pReg)
        {
            m_pReg->UnregisterMetadatable( *this );
            m_pReg->RemoveXmlIdForElement( *this );
            m_pReg = 0;
        }
    }
    catch (const uno::Exception &)
    {
        OSL_FAIL("Metadatable::RemoveMetadataReference: exception");
    }
}

// ::com::sun::star::rdf::XMetadatable:
beans::StringPair
Metadatable::GetMetadataReference() const
{
    if (m_pReg)
    {
        return m_pReg->GetXmlIdForElement(*this);
    }
    return beans::StringPair();
}

void
Metadatable::SetMetadataReference(
    const ::com::sun::star::beans::StringPair & i_rReference)
{
    if (i_rReference.Second.isEmpty())
    {
        RemoveMetadataReference();
    }
    else
    {
        OUString streamName( i_rReference.First );
        if (streamName.isEmpty())
        {
            // handle empty stream name as auto-detect.
            // necessary for importing flat file format.
            streamName = OUString::createFromAscii(
                            IsInContent() ? s_content : s_styles );
        }
        XmlIdRegistry & rReg( dynamic_cast<XmlIdRegistry&>( GetRegistry() ) );
        if (rReg.TryRegisterMetadatable(*this, streamName, i_rReference.Second))
        {
            m_pReg = &rReg;
        }
        else
        {
            throw lang::IllegalArgumentException(
                OUString("Metadatable::"
                    "SetMetadataReference: argument is invalid"), /*this*/0, 0);
        }
    }
}

void Metadatable::EnsureMetadataReference()
{
    XmlIdRegistry& rReg(
        m_pReg ? *m_pReg : dynamic_cast<XmlIdRegistry&>( GetRegistry() ) );
    rReg.RegisterMetadatableAndCreateID( *this );
    m_pReg = &rReg;
}

const ::sfx2::IXmlIdRegistry& GetRegistryConst(Metadatable const& i_rObject)
{
    return const_cast< Metadatable& >( i_rObject ).GetRegistry();
}

void
Metadatable::RegisterAsCopyOf(Metadatable const & i_rSource,
    const bool i_bCopyPrecedesSource)
{
    OSL_ENSURE(typeid(*this) == typeid(i_rSource)
        || typeid(i_rSource) == typeid(MetadatableUndo)
        || typeid(*this)     == typeid(MetadatableUndo)
        || typeid(i_rSource) == typeid(MetadatableClipboard)
        || typeid(*this)     == typeid(MetadatableClipboard),
        "RegisterAsCopyOf element with different class?");
    OSL_ENSURE(!this->m_pReg, "RegisterAsCopyOf called on element with XmlId?");

    if (this->m_pReg)
    {
        RemoveMetadataReference();
    }

    try
    {
        if (i_rSource.m_pReg)
        {
            XmlIdRegistry & rReg(
                dynamic_cast<XmlIdRegistry&>( GetRegistry() ) );
            if (i_rSource.m_pReg == &rReg)
            {
                OSL_ENSURE(!IsInClipboard(),
                    "RegisterAsCopy: both in clipboard?");
                if (!IsInClipboard())
                {
                    XmlIdRegistryDocument & rRegDoc(
                        dynamic_cast<XmlIdRegistryDocument&>( rReg ) );
                    rRegDoc.RegisterCopy(i_rSource, *this,
                        i_bCopyPrecedesSource);
                    this->m_pReg = &rRegDoc;
                }
                return;
            }
            // source is in different document
            XmlIdRegistryDocument  * pRegDoc(
                dynamic_cast<XmlIdRegistryDocument *>(&rReg) );
            XmlIdRegistryClipboard * pRegClp(
                dynamic_cast<XmlIdRegistryClipboard*>(&rReg) );

            if (pRegClp)
            {
                beans::StringPair SourceRef(
                    i_rSource.m_pReg->GetXmlIdForElement(i_rSource) );
                bool isLatent( SourceRef.Second.isEmpty() );
                XmlIdRegistryDocument * pSourceRegDoc(
                    dynamic_cast<XmlIdRegistryDocument*>(i_rSource.m_pReg) );
                OSL_ENSURE(pSourceRegDoc, "RegisterAsCopyOf: 2 clipboards?");
                if (!pSourceRegDoc) return;
                // this is a copy _to_ the clipboard
                if (isLatent)
                {
                    pSourceRegDoc->LookupXmlId(i_rSource,
                        SourceRef.First, SourceRef.Second);
                }
                Metadatable & rLink(
                    pRegClp->RegisterCopyClipboard(*this, SourceRef, isLatent));
                this->m_pReg = pRegClp;
                // register as copy in the non-clipboard registry
                pSourceRegDoc->RegisterCopy(i_rSource, rLink,
                    false); // i_bCopyPrecedesSource);
                rLink.m_pReg = pSourceRegDoc;
            }
            else if (pRegDoc)
            {
                XmlIdRegistryClipboard * pSourceRegClp(
                    dynamic_cast<XmlIdRegistryClipboard*>(i_rSource.m_pReg) );
                OSL_ENSURE(pSourceRegClp,
                    "RegisterAsCopyOf: 2 non-clipboards?");
                if (!pSourceRegClp) return;
                const MetadatableClipboard * pLink(
                    pSourceRegClp->SourceLink(i_rSource) );
                // may happen if src got its id via UNO call
                if (!pLink) return;
                // only register copy if clipboard content is from this SwDoc!
                if (pLink && (&GetRegistryConst(*pLink) == pRegDoc))
                {
                    // this is a copy _from_ the clipboard; check if the
                    // element is still in the same stream
                    // N.B.: we check the stream of pLink, not of i_rSource!
                    bool srcInContent( pLink->IsInContent() );
                    bool tgtInContent( this->IsInContent() );
                    if (srcInContent == tgtInContent)
                    {
                        pRegDoc->RegisterCopy(*pLink, *this,
                            true); // i_bCopyPrecedesSource);
                        this->m_pReg = pRegDoc;
                    }
                    // otherwise: stream change! do not register!
                }
            }
            else
            {
                OSL_FAIL("neither RegDoc nor RegClp cannot happen");
            }
        }
    }
    catch (const uno::Exception &)
    {
        OSL_FAIL("Metadatable::RegisterAsCopyOf: exception");
    }
}

::boost::shared_ptr<MetadatableUndo> Metadatable::CreateUndo() const
{
    OSL_ENSURE(!IsInUndo(), "CreateUndo called for object in undo?");
    OSL_ENSURE(!IsInClipboard(), "CreateUndo called for object in clipboard?");
    try
    {
        if (!IsInClipboard() && !IsInUndo() && m_pReg)
        {
            XmlIdRegistryDocument * pRegDoc(
                dynamic_cast<XmlIdRegistryDocument*>( m_pReg ) );
            ::boost::shared_ptr<MetadatableUndo> pUndo(
                pRegDoc->CreateUndo(*this) );
            pRegDoc->RegisterCopy(*this, *pUndo, false);
            pUndo->m_pReg = pRegDoc;
            return pUndo;
        }
    }
    catch (const uno::Exception &)
    {
        OSL_FAIL("Metadatable::CreateUndo: exception");
    }
    return ::boost::shared_ptr<MetadatableUndo>();
}

::boost::shared_ptr<MetadatableUndo> Metadatable::CreateUndoForDelete()
{
    ::boost::shared_ptr<MetadatableUndo> const pUndo( CreateUndo() );
    RemoveMetadataReference();
    return pUndo;
}

void Metadatable::RestoreMetadata(
    ::boost::shared_ptr<MetadatableUndo> const& i_pUndo)
{
    OSL_ENSURE(!IsInUndo(), "RestoreMetadata called for object in undo?");
    OSL_ENSURE(!IsInClipboard(),
        "RestoreMetadata called for object in clipboard?");
    if (IsInClipboard() || IsInUndo()) return;
    RemoveMetadataReference();
    if (i_pUndo)
    {
        this->RegisterAsCopyOf(*i_pUndo, true);
    }
}

void
Metadatable::JoinMetadatable(Metadatable const & i_rOther,
    const bool i_isMergedEmpty, const bool i_isOtherEmpty)
{
    OSL_ENSURE(!IsInUndo(), "JoinMetadatables called for object in undo?");
    OSL_ENSURE(!IsInClipboard(),
        "JoinMetadatables called for object in clipboard?");
    if (IsInClipboard() || IsInUndo()) return;

    if (i_isOtherEmpty && !i_isMergedEmpty)
    {
        // other is empty, thus loses => nothing to do
        return;
    }
    if (i_isMergedEmpty && !i_isOtherEmpty)
    {
        this->RemoveMetadataReference();
        this->RegisterAsCopyOf(i_rOther, true);
        return;
    }

    if (!i_rOther.m_pReg)
    {
        // other doesn't have xmlid, thus loses => nothing to do
        return;
    }
    if (!m_pReg)
    {
        this->RegisterAsCopyOf(i_rOther, true);
        // assumption: i_rOther will be deleted, so don't unregister it here
        return;
    }
    try
    {
        XmlIdRegistryDocument * pRegDoc(
            dynamic_cast<XmlIdRegistryDocument*>( m_pReg ) );
        OSL_ENSURE(pRegDoc, "JoinMetadatable: no pRegDoc?");
        if (pRegDoc)
        {
            pRegDoc->JoinMetadatables(*this, i_rOther);
        }
    }
    catch (const uno::Exception &)
    {
        OSL_FAIL("Metadatable::JoinMetadatable: exception");
    }
}


//=============================================================================
// XMetadatable mixin

// ::com::sun::star::rdf::XNode:
OUString SAL_CALL MetadatableMixin::getStringValue()
    throw (::com::sun::star::uno::RuntimeException)
{
    return getNamespace() + getLocalName();
}

// ::com::sun::star::rdf::XURI:
OUString SAL_CALL MetadatableMixin::getLocalName()
    throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    beans::StringPair mdref( getMetadataReference() );
    if (mdref.Second.isEmpty())
    {
        ensureMetadataReference(); // N.B.: side effect!
        mdref = getMetadataReference();
    }
    OUStringBuffer buf;
    buf.append(mdref.First);
    buf.append(static_cast<sal_Unicode>('#'));
    buf.append(mdref.Second);
    return buf.makeStringAndClear();
}

OUString SAL_CALL MetadatableMixin::getNamespace()
    throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    const uno::Reference< frame::XModel > xModel( GetModel() );
    const uno::Reference< rdf::XURI > xDMA( xModel, uno::UNO_QUERY_THROW );
    return xDMA->getStringValue();
}

// ::com::sun::star::rdf::XMetadatable:
beans::StringPair SAL_CALL
MetadatableMixin::getMetadataReference()
throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Metadatable *const pObject( GetCoreObject() );
    if (!pObject)
    {
        throw uno::RuntimeException(
            OUString(
                "MetadatableMixin: cannot get core object; not inserted?"),
            *this);
    }
    return pObject->GetMetadataReference();
}

void SAL_CALL
MetadatableMixin::setMetadataReference(
    const beans::StringPair & i_rReference)
throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    SolarMutexGuard aGuard;

    Metadatable *const pObject( GetCoreObject() );
    if (!pObject)
    {
        throw uno::RuntimeException(
            OUString(
                "MetadatableMixin: cannot get core object; not inserted?"),
            *this);
    }
    return pObject->SetMetadataReference(i_rReference);
}

void SAL_CALL MetadatableMixin::ensureMetadataReference()
throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Metadatable *const pObject( GetCoreObject() );
    if (!pObject)
    {
        throw uno::RuntimeException(
            OUString(
                "MetadatableMixin: cannot get core object; not inserted?"),
            *this);
    }
    return pObject->EnsureMetadataReference();
}

} // namespace sfx2


//=============================================================================

#if OSL_DEBUG_LEVEL > 1

#include <stdio.h>

static void dump(sfx2::XmlIdList_t * pList)
#ifdef __GNUC__
__attribute__ ((unused))
#endif
;
static void dump(sfx2::XmlIdList_t * pList)
{
    fprintf(stderr, "\nXmlIdList(%p):  ", pList);
    for (sfx2::XmlIdList_t::iterator i = pList->begin(); i != pList->end(); ++i)
    {
        fprintf(stderr, "%p  ", *i);
    }
    fprintf(stderr, "\n");
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
