/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SwMetadatable.cxx,v $
 * $Revision: 1.1.2.8 $
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

#include "precompiled_sfx2.hxx"

#include <sfx2/Metadatable.hxx>
#include <sfx2/XmlIdRegistry.hxx>

#include <vos/mutex.hxx>
#include <vcl/svapp.hxx> // solarmutex

#include <boost/bind.hpp>

#include <memory>
#include <hash_map>
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

static bool isContentFile(::rtl::OUString const & i_rPath)
{
    return i_rPath.equalsAscii(s_content);
}

static bool isStylesFile (::rtl::OUString const & i_rPath)
{
    return i_rPath.equalsAscii(s_styles);
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
        ::rtl::OUString const& i_rStreamName, ::rtl::OUString const& i_rIdref)
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
        ::rtl::OUString & o_rStream, ::rtl::OUString & o_rIdref) const = 0;

    virtual Metadatable* LookupElement(const ::rtl::OUString & i_rStreamName,
        const ::rtl::OUString & i_rIdref) const = 0;
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
        ::rtl::OUString const& i_rStreamName, ::rtl::OUString const& i_rIdref);

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
        ::rtl::OUString & o_rStream, ::rtl::OUString & o_rIdref) const;

private:

    virtual Metadatable* LookupElement(const ::rtl::OUString & i_rStreamName,
        const ::rtl::OUString & i_rIdref) const;

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
    { OSL_ENSURE(false, "MetadatableUndo::MakeUnoObject"); throw; }
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
    { OSL_ENSURE(false, "MetadatableClipboard::MakeUnoObject"); throw; }
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
        ::rtl::OUString const& i_rStreamName, ::rtl::OUString const& i_rIdref);

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
        ::rtl::OUString & o_rStream, ::rtl::OUString & o_rIdref) const;

    virtual Metadatable* LookupElement(const ::rtl::OUString & i_rStreamName,
        const ::rtl::OUString & i_rIdref) const;

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
    ::rtl::OUString path;
    ::rtl::OUString idref;
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
/*static*/ ::rtl::OUString create_id(const
    ::std::hash_map< ::rtl::OUString, T, ::rtl::OUStringHash > & i_rXmlIdMap)
{
    const ::rtl::OUString prefix( ::rtl::OUString::createFromAscii(s_prefix) );
    typename ::std::hash_map< ::rtl::OUString, T, ::rtl::OUStringHash >
        ::const_iterator iter;
    ::rtl::OUString id;
    do
    {
        const int n( rand() );
        id = prefix + ::rtl::OUString::valueOf(static_cast<sal_Int64>(n));
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
typedef ::std::hash_map< ::rtl::OUString,
    ::std::pair< XmlIdList_t, XmlIdList_t >, ::rtl::OUStringHash > XmlIdMap_t;

/// pointer hash template
template<typename T> struct PtrHash
{
    size_t operator() (T const * i_pT) const
    {
        return reinterpret_cast<size_t>(i_pT);
    }
};

/// element -> (stream name, idref)
typedef ::std::hash_map< const Metadatable*,
    ::std::pair< ::rtl::OUString, ::rtl::OUString>, PtrHash<Metadatable> >
    XmlIdReverseMap_t;

struct XmlIdRegistryDocument::XmlIdRegistry_Impl
{
    XmlIdRegistry_Impl()
        : m_XmlIdMap(), m_XmlIdReverseMap() { }

    bool TryInsertMetadatable(Metadatable& i_xObject,
        const ::rtl::OUString & i_rStream, const ::rtl::OUString & i_rIdref);

    bool LookupXmlId(const Metadatable& i_xObject,
        ::rtl::OUString & o_rStream, ::rtl::OUString & o_rIdref) const;

    Metadatable* LookupElement(const ::rtl::OUString & i_rStreamName,
        const ::rtl::OUString & i_rIdref) const;

    const XmlIdList_t * LookupElementList(
        const ::rtl::OUString & i_rStreamName,
        const ::rtl::OUString & i_rIdref) const;

          XmlIdList_t * LookupElementList(
        const ::rtl::OUString & i_rStreamName,
        const ::rtl::OUString & i_rIdref)
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
    ::rtl::OUString const & i_rStream, Metadatable const& i_rObject)
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
    const ::rtl::OUString & i_rStreamName,
    const ::rtl::OUString & i_rIdref) const
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
    const ::rtl::OUString & i_rStreamName,
    const ::rtl::OUString & i_rIdref) const
{
    if (!isValidXmlId(i_rStreamName, i_rIdref))
    {
        throw lang::IllegalArgumentException(::rtl::OUString::createFromAscii(
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
    ::rtl::OUString & o_rStream, ::rtl::OUString & o_rIdref) const
{
    const XmlIdReverseMap_t::const_iterator iter(
        m_XmlIdReverseMap.find(&i_rObject) );
    if (iter != m_XmlIdReverseMap.end())
    {
        OSL_ENSURE(!iter->second.first.equalsAscii(""),
            "null stream in m_XmlIdReverseMap");
        OSL_ENSURE(!iter->second.second.equalsAscii(""),
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
    const ::rtl::OUString & i_rStreamName, const ::rtl::OUString & i_rIdref)
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
// ???  this is not undoable
//                pList->clear();
//                pList->push_back( &i_rObject );
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
    ::rtl::OUString & o_rStream, ::rtl::OUString & o_rIdref) const
{
    return m_pImpl->LookupXmlId(i_rObject, o_rStream, o_rIdref);
}

Metadatable*
XmlIdRegistryDocument::LookupElement(
    const ::rtl::OUString & i_rStreamName,
    const ::rtl::OUString & i_rIdref) const
{
    return m_pImpl->LookupElement(i_rStreamName, i_rIdref);
}

bool
XmlIdRegistryDocument::TryRegisterMetadatable(Metadatable & i_rObject,
    ::rtl::OUString const& i_rStreamName, ::rtl::OUString const& i_rIdref)
{
    OSL_TRACE("TryRegisterMetadatable: %p (%s#%s)\n", &i_rObject,
        ::rtl::OUStringToOString(i_rStreamName, RTL_TEXTENCODING_UTF8).getStr(),
        ::rtl::OUStringToOString(i_rIdref, RTL_TEXTENCODING_UTF8).getStr());

    OSL_ENSURE(!dynamic_cast<MetadatableUndo*>(&i_rObject),
        "TryRegisterMetadatable called for MetadatableUndo?");
    OSL_ENSURE(!dynamic_cast<MetadatableClipboard*>(&i_rObject),
        "TryRegisterMetadatable called for MetadatableClipboard?");

    if (!isValidXmlId(i_rStreamName, i_rIdref))
    {
        throw lang::IllegalArgumentException(::rtl::OUString::createFromAscii(
            "illegal XmlId"), 0, 0);
    }
    if (i_rObject.IsInContent()
        ?   !isContentFile(i_rStreamName)
        :   !isStylesFile(i_rStreamName))
    {
        throw lang::IllegalArgumentException(::rtl::OUString::createFromAscii(
            "illegal XmlId: wrong stream"), 0, 0);
    }

    ::rtl::OUString old_path;
    ::rtl::OUString old_idref;
    m_pImpl->LookupXmlId(i_rObject, old_path, old_idref);
    if (old_path  == i_rStreamName && old_idref == i_rIdref)
    {
        return (m_pImpl->LookupElement(old_path, old_idref) == &i_rObject);
    }
    XmlIdMap_t::iterator old_id( m_pImpl->m_XmlIdMap.end() );
    if (!old_idref.equalsAscii(""))
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
    OSL_TRACE("RegisterMetadatableAndCreateID: %p\n", &i_rObject);

    OSL_ENSURE(!dynamic_cast<MetadatableUndo*>(&i_rObject),
        "RegisterMetadatableAndCreateID called for MetadatableUndo?");
    OSL_ENSURE(!dynamic_cast<MetadatableClipboard*>(&i_rObject),
        "RegisterMetadatableAndCreateID called for MetadatableClipboard?");

    const bool isInContent( i_rObject.IsInContent() );
    const ::rtl::OUString stream( ::rtl::OUString::createFromAscii(
        isInContent ? s_content : s_styles ) );
    // check if we have a latent xmlid, and if yes, remove it
    ::rtl::OUString old_path;
    ::rtl::OUString old_idref;
    m_pImpl->LookupXmlId(i_rObject, old_path, old_idref);

    XmlIdMap_t::iterator old_id( m_pImpl->m_XmlIdMap.end() );
    if (!old_idref.equalsAscii(""))
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
    const ::rtl::OUString id( create_id(m_pImpl->m_XmlIdMap) );
    OSL_ENSURE(m_pImpl->m_XmlIdMap.find(id) == m_pImpl->m_XmlIdMap.end(),
        "created id is in use");
    m_pImpl->m_XmlIdMap.insert(::std::make_pair(id, isInContent
        ? ::std::make_pair( XmlIdList_t( 1, &i_rObject ), XmlIdList_t() )
        : ::std::make_pair( XmlIdList_t(), XmlIdList_t( 1, &i_rObject ) )));
    m_pImpl->m_XmlIdReverseMap[&i_rObject] = ::std::make_pair(stream, id);
}

void XmlIdRegistryDocument::UnregisterMetadatable(const Metadatable& i_rObject)
{
    OSL_TRACE("UnregisterMetadatable: %p\n", &i_rObject);

    ::rtl::OUString path;
    ::rtl::OUString idref;
    if (!m_pImpl->LookupXmlId(i_rObject, path, idref))
    {
        OSL_ENSURE(false, "unregister: no xml id?");
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
    OSL_TRACE("RemoveXmlIdForElement: %p\n", &i_rObject);

    const XmlIdReverseMap_t::iterator iter(
        m_pImpl->m_XmlIdReverseMap.find(&i_rObject) );
    if (iter != m_pImpl->m_XmlIdReverseMap.end())
    {
        OSL_ENSURE(!iter->second.second.equalsAscii(""),
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

    ::rtl::OUString path;
    ::rtl::OUString idref;
    if (!m_pImpl->LookupXmlId( i_rSource, path, idref ))
    {
        OSL_ENSURE(false, "no xml id?");
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
    OSL_TRACE("CreateUndo: %p\n", &i_rObject);

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
    OSL_TRACE("JoinMetadatables: %p <- %p\n", &i_rMerged, &i_rOther);

    bool mergedOwnsRef;
    ::rtl::OUString path;
    ::rtl::OUString idref;
    if (m_pImpl->LookupXmlId(i_rMerged, path, idref))
    {
        mergedOwnsRef = (m_pImpl->LookupElement(path, idref) == &i_rMerged);
    }
    else
    {
        OSL_ENSURE(false, "JoinMetadatables: no xmlid?");
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
    RMapEntry(::rtl::OUString const& i_rStream,
            ::rtl::OUString const& i_rXmlId,
            ::boost::shared_ptr<MetadatableClipboard> const& i_pLink
                = ::boost::shared_ptr<MetadatableClipboard>())
        :   m_Stream(i_rStream), m_XmlId(i_rXmlId), m_pLink(i_pLink)
        {}
    ::rtl::OUString m_Stream;
    ::rtl::OUString m_XmlId;
    // this would have been an auto_ptr, if only that would have compiled...
    ::boost::shared_ptr<MetadatableClipboard> m_pLink;
};

/// element -> (stream name, idref, source)
typedef ::std::hash_map< const Metadatable*,
    struct RMapEntry,
    PtrHash<Metadatable> >
    ClipboardXmlIdReverseMap_t;

/// Idref -> (content.xml element, styles.xml element)
typedef ::std::hash_map< ::rtl::OUString,
    ::std::pair< Metadatable*, Metadatable* >, ::rtl::OUStringHash >
    ClipboardXmlIdMap_t;

struct XmlIdRegistryClipboard::XmlIdRegistry_Impl
{
    XmlIdRegistry_Impl()
        : m_XmlIdMap(), m_XmlIdReverseMap() { }

    bool TryInsertMetadatable(Metadatable& i_xObject,
        const ::rtl::OUString & i_rStream, const ::rtl::OUString & i_rIdref);

    bool LookupXmlId(const Metadatable& i_xObject,
        ::rtl::OUString & o_rStream, ::rtl::OUString & o_rIdref,
        MetadatableClipboard const* &o_rpLink) const;

    Metadatable* LookupElement(const ::rtl::OUString & i_rStreamName,
        const ::rtl::OUString & i_rIdref) const;

    Metadatable* const* LookupEntry(const ::rtl::OUString & i_rStreamName,
        const ::rtl::OUString & i_rIdref) const;

    Metadatable*      * LookupEntry(const ::rtl::OUString & i_rStreamName,
        const ::rtl::OUString & i_rIdref)
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
    ::rtl::OUString const & i_rStream, Metadatable const& i_rObject)
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
    const ::rtl::OUString & i_rStreamName,
    const ::rtl::OUString & i_rIdref) const
{
    if (!isValidXmlId(i_rStreamName, i_rIdref))
    {
        throw lang::IllegalArgumentException(::rtl::OUString::createFromAscii(
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
    const ::rtl::OUString & i_rStreamName,
    const ::rtl::OUString & i_rIdref) const
{
    Metadatable * const * ppEntry = LookupEntry(i_rStreamName, i_rIdref);
    return ppEntry ? *ppEntry : 0;
}

bool
XmlIdRegistryClipboard::XmlIdRegistry_Impl::LookupXmlId(
    const Metadatable& i_rObject,
    ::rtl::OUString & o_rStream, ::rtl::OUString & o_rIdref,
    MetadatableClipboard const* &o_rpLink) const
{
    const ClipboardXmlIdReverseMap_t::const_iterator iter(
        m_XmlIdReverseMap.find(&i_rObject) );
    if (iter != m_XmlIdReverseMap.end())
    {
        OSL_ENSURE(!iter->second.m_Stream.equalsAscii(""),
            "null stream in m_XmlIdReverseMap");
        OSL_ENSURE(!iter->second.m_XmlId.equalsAscii(""),
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
    const ::rtl::OUString & i_rStreamName, const ::rtl::OUString & i_rIdref)
{
    bool bContent( isContentFile(i_rStreamName) );
    OSL_ENSURE(isContentFile(i_rStreamName) || isStylesFile(i_rStreamName),
        "invalid stream");

    //wntmsci12 won't parse this:
//    Metadatable ** ppEntry( LookupEntry(i_rStreamName, i_rIdref) );
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
    ::rtl::OUString & o_rStream, ::rtl::OUString & o_rIdref) const
{
    const MetadatableClipboard * pLink;
    return m_pImpl->LookupXmlId(i_rObject, o_rStream, o_rIdref, pLink);
}

Metadatable*
XmlIdRegistryClipboard::LookupElement(
    const ::rtl::OUString & i_rStreamName,
    const ::rtl::OUString & i_rIdref) const
{
    return m_pImpl->LookupElement(i_rStreamName, i_rIdref);
}

bool
XmlIdRegistryClipboard::TryRegisterMetadatable(Metadatable & i_rObject,
    ::rtl::OUString const& i_rStreamName, ::rtl::OUString const& i_rIdref)
{
    OSL_TRACE("TryRegisterMetadatable: %p (%s#%s)\n", &i_rObject,
        ::rtl::OUStringToOString(i_rStreamName, RTL_TEXTENCODING_UTF8).getStr(),
        ::rtl::OUStringToOString(i_rIdref, RTL_TEXTENCODING_UTF8).getStr());

    OSL_ENSURE(!dynamic_cast<MetadatableUndo*>(&i_rObject),
        "TryRegisterMetadatable called for MetadatableUndo?");
    OSL_ENSURE(!dynamic_cast<MetadatableClipboard*>(&i_rObject),
        "TryRegisterMetadatable called for MetadatableClipboard?");

    if (!isValidXmlId(i_rStreamName, i_rIdref))
    {
        throw lang::IllegalArgumentException(::rtl::OUString::createFromAscii(
            "illegal XmlId"), 0, 0);
    }
    if (i_rObject.IsInContent()
        ?   !isContentFile(i_rStreamName)
        :   !isStylesFile(i_rStreamName))
    {
        throw lang::IllegalArgumentException(::rtl::OUString::createFromAscii(
            "illegal XmlId: wrong stream"), 0, 0);
    }

    ::rtl::OUString old_path;
    ::rtl::OUString old_idref;
    const MetadatableClipboard * pLink;
    m_pImpl->LookupXmlId(i_rObject, old_path, old_idref, pLink);
    if (old_path  == i_rStreamName && old_idref == i_rIdref)
    {
        return (m_pImpl->LookupElement(old_path, old_idref) == &i_rObject);
    }
    ClipboardXmlIdMap_t::iterator old_id( m_pImpl->m_XmlIdMap.end() );
    if (!old_idref.equalsAscii(""))
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
    OSL_TRACE("RegisterMetadatableAndCreateID: %p\n", &i_rObject);

    OSL_ENSURE(!dynamic_cast<MetadatableUndo*>(&i_rObject),
        "RegisterMetadatableAndCreateID called for MetadatableUndo?");
    OSL_ENSURE(!dynamic_cast<MetadatableClipboard*>(&i_rObject),
        "RegisterMetadatableAndCreateID called for MetadatableClipboard?");

    bool isInContent( i_rObject.IsInContent() );
    ::rtl::OUString stream( ::rtl::OUString::createFromAscii(
        isInContent ? s_content : s_styles ) );

    ::rtl::OUString old_path;
    ::rtl::OUString old_idref;
    LookupXmlId(i_rObject, old_path, old_idref);
    if (!old_idref.equalsAscii("") &&
        (m_pImpl->LookupElement(old_path, old_idref) == &i_rObject))
    {
        return;
    }

    // create id
    const ::rtl::OUString id( create_id(m_pImpl->m_XmlIdMap) );
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
    OSL_TRACE("UnregisterMetadatable: %p\n", &i_rObject);

    ::rtl::OUString path;
    ::rtl::OUString idref;
    const MetadatableClipboard * pLink;
    if (!m_pImpl->LookupXmlId(i_rObject, path, idref, pLink))
    {
        OSL_ENSURE(false, "unregister: no xml id?");
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
    OSL_TRACE("RemoveXmlIdForElement: %p\n", &i_rObject);

    ClipboardXmlIdReverseMap_t::iterator iter(
        m_pImpl->m_XmlIdReverseMap.find(&i_rObject) );
    if (iter != m_pImpl->m_XmlIdReverseMap.end())
    {
        OSL_ENSURE(!iter->second.m_XmlId.equalsAscii(""),
            "null id in m_XmlIdReverseMap");
        m_pImpl->m_XmlIdReverseMap.erase(iter);
    }
}

// -------------------------------------------------------------------

::boost::shared_ptr<MetadatableClipboard>
XmlIdRegistryClipboard::CreateClipboard(const bool i_isInContent)
{
    OSL_TRACE("CreateClipboard: \n");

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
        ::rtl::OUStringToOString(i_rReference.First,
            RTL_TEXTENCODING_UTF8).getStr(),
        ::rtl::OUStringToOString(i_rReference.Second,
            RTL_TEXTENCODING_UTF8).getStr(),
        i_isLatent);

    // N.B.: when copying to the clipboard, the selection is always inserted
    //       into the body, even if the source is a header/footer!
    //       so we do not check whether the stream is right in this function

    if (!isValidXmlId(i_rReference.First, i_rReference.Second))
    {
        throw lang::IllegalArgumentException(::rtl::OUString::createFromAscii(
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
    ::rtl::OUString path;
    ::rtl::OUString idref;
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
    catch (uno::Exception &)
    {
        OSL_ENSURE(false, "Metadatable::RemoveMetadataReference: exception");
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
    if (i_rReference.Second.equalsAscii(""))
    {
        RemoveMetadataReference();
    }
    else
    {
        ::rtl::OUString streamName( i_rReference.First );
        if (streamName.equalsAscii(""))
        {
            // handle empty stream name as auto-detect.
            // necessary for importing flat file format.
            streamName = ::rtl::OUString::createFromAscii(
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
                ::rtl::OUString::createFromAscii("Metadatable::"
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
                bool isLatent( SourceRef.Second.equalsAscii("") );
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
                OSL_ENSURE(false, "neither RegDoc nor RegClp cannot happen");
            }
#if 0
                {
                    //FIXME: do we need this at all???
                    XmlIdRegistryDocument & rRegDoc(
                        dynamic_cast<XmlIdRegistryDocument&>( rReg ) );
                    {
                        if (rRegDoc.TryRegisterMetadatable(*this, SourceRef))
                        {
                            this->m_pReg = &rRegDoc;
                        }
                    }
                }
#endif
        }
    }
    catch (uno::Exception &)
    {
        OSL_ENSURE(false, "Metadatable::RegisterAsCopyOf: exception");
    }
}

::boost::shared_ptr<MetadatableUndo> Metadatable::CreateUndo(
    const bool i_isDelete)
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

            if (i_isDelete)
            {
                RemoveMetadataReference();
            }
            return pUndo;
        }
    }
    catch (uno::Exception &)
    {
        OSL_ENSURE(false, "Metadatable::CreateUndo: exception");
    }
    return ::boost::shared_ptr<MetadatableUndo>();
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
    catch (uno::Exception &)
    {
        OSL_ENSURE(false, "Metadatable::JoinMetadatable: exception");
    }
}


//=============================================================================
// XMetadatable mixin

// ::com::sun::star::rdf::XNode:
::rtl::OUString SAL_CALL MetadatableMixin::getStringValue()
    throw (::com::sun::star::uno::RuntimeException)
{
    return getNamespace() + getLocalName();
}

// ::com::sun::star::rdf::XURI:
::rtl::OUString SAL_CALL MetadatableMixin::getLocalName()
    throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    beans::StringPair mdref( getMetadataReference() );
    if (!mdref.Second.getLength())
    {
        ensureMetadataReference(); // N.B.: side effect!
        mdref = getMetadataReference();
    }
    ::rtl::OUStringBuffer buf;
    buf.append(mdref.First);
    buf.append(static_cast<sal_Unicode>('#'));
    buf.append(mdref.Second);
    return buf.makeStringAndClear();
}

::rtl::OUString SAL_CALL MetadatableMixin::getNamespace()
    throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    const uno::Reference< frame::XModel > xModel( GetModel() );
    const uno::Reference< rdf::XURI > xDMA( xModel, uno::UNO_QUERY_THROW );
    return xDMA->getStringValue();
}

// ::com::sun::star::rdf::XMetadatable:
beans::StringPair SAL_CALL
MetadatableMixin::getMetadataReference()
throw (uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    Metadatable* pObject( GetCoreObject() );
    if (pObject)
    {
        return pObject->GetMetadataReference();
    }
    else
    {
        throw uno::RuntimeException();
    }
}

void SAL_CALL
MetadatableMixin::setMetadataReference(
    const beans::StringPair & i_rReference)
throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    Metadatable* pObject( GetCoreObject() );
    if (pObject)
    {
        return pObject->SetMetadataReference(i_rReference);
    }
    else
    {
        throw uno::RuntimeException();
    }
}

void SAL_CALL MetadatableMixin::ensureMetadataReference()
throw (uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    Metadatable* pObject( GetCoreObject() );
    if (pObject)
    {
        return pObject->EnsureMetadataReference();
    }
    else
    {
        throw uno::RuntimeException();
    }
}

} // namespace sfx2


//=============================================================================

#if OSL_DEBUG_LEVEL > 1

static ::sfx2::XmlIdRegistryDocument s_Reg;
static ::sfx2::XmlIdRegistryClipboard s_RegClip;

class MockMetadatable : public ::sfx2::Metadatable
{
public:
    MockMetadatable(bool i_isInClip = false) :
        m_bInClipboard(i_isInClip), m_bInUndo(false), m_bInContent(true) {}
    bool m_bInClipboard;
    bool m_bInUndo;
    bool m_bInContent;
    virtual bool IsInClipboard() const { return m_bInClipboard; }
    virtual bool IsInUndo() const { return m_bInUndo; }
    virtual bool IsInContent() const { return m_bInContent; }
    virtual ::sfx2::XmlIdRegistry& GetRegistry() { return m_bInClipboard ? static_cast< ::sfx2::XmlIdRegistry&>(s_RegClip) : static_cast< ::sfx2::XmlIdRegistry&>(s_Reg); }
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::rdf::XMetadatable > MakeUnoObject() { return 0; }
};

bool operator==(beans::StringPair p1, beans::StringPair p2)
{
    return p1.First == p2.First && p1.Second == p2.Second;
}

void test()
{
    OSL_TRACE("SwMetadatable test(): start\n");
    MockMetadatable m1;
    MockMetadatable m2;
    MockMetadatable m3;
    MockMetadatable m4;
    MockMetadatable m5;
    ::rtl::OUString empty;
    ::rtl::OUString content( ::rtl::OUString::createFromAscii("content.xml") );
    ::rtl::OUString styles ( ::rtl::OUString::createFromAscii("styles.xml") );
    ::rtl::OUString sid1( ::rtl::OUString::createFromAscii("id1") );
    ::rtl::OUString sid2( ::rtl::OUString::createFromAscii("id2") );
    ::rtl::OUString sid3( ::rtl::OUString::createFromAscii("id3") );
    ::rtl::OUString sid4( ::rtl::OUString::createFromAscii("id4") );
    beans::StringPair id1(content, sid1);
    beans::StringPair id2(content, sid2);
    beans::StringPair id3(content, sid3);
    beans::StringPair id4(styles,  sid4);
    beans::StringPair id3e(empty,  sid3);
    beans::StringPair id4e(empty,  sid4);
    m1.SetMetadataReference(id1);
    OSL_ENSURE(m1.GetMetadataReference() == id1, "set failed");
    try {
        m2.SetMetadataReference(id1);
        OSL_ENSURE(false, "set duplicate succeeded");
    } catch (lang::IllegalArgumentException) { }
    m1.SetMetadataReference(id1);
    OSL_ENSURE(m1.GetMetadataReference() == id1, "set failed (existing)");
    m1.EnsureMetadataReference();
    OSL_ENSURE(m1.GetMetadataReference() == id1, "ensure failed (existing)");

    m2.EnsureMetadataReference();
    beans::StringPair m2id(m2.GetMetadataReference());
    OSL_ENSURE(m2id.Second.getLength(), "ensure failed");
    m2.EnsureMetadataReference();
    OSL_ENSURE(m2.GetMetadataReference() == m2id, "ensure failed (idempotent)");

    m1.m_bInUndo = true;
    OSL_ENSURE(!m1.GetMetadataReference().Second.getLength(), "move to undo failed");

    m1.m_bInUndo = false;
    OSL_ENSURE(m1.GetMetadataReference() == id1, "move from undo failed");

    m1.m_bInUndo = true;
    try {
        m2.SetMetadataReference(id1); // steal!
    } catch (lang::IllegalArgumentException &) {
        OSL_ENSURE(false, "set duplicate to undo failed");
    }
    m1.m_bInUndo = false;
    OSL_ENSURE(!m1.GetMetadataReference().Second.getLength(), "move from undo: duplicate");

    m3.RegisterAsCopyOf(m2);
    OSL_ENSURE(m2.GetMetadataReference() == id1, "copy: source");
    OSL_ENSURE(!m3.GetMetadataReference().Second.getLength(), "copy: duplicate");
    m4.RegisterAsCopyOf(m3);
    OSL_ENSURE(m2.GetMetadataReference() == id1, "copy: source");
    OSL_ENSURE(!m3.GetMetadataReference().Second.getLength(), "copy: duplicate");
    OSL_ENSURE(!m4.GetMetadataReference().Second.getLength(), "copy: duplicate");
    m2.m_bInUndo = true;
    OSL_ENSURE(m3.GetMetadataReference() == id1, "duplicate to undo");
    OSL_ENSURE(!m2.GetMetadataReference().Second.getLength(), "duplicate to undo");
    m2.m_bInUndo = false;
    OSL_ENSURE(m2.GetMetadataReference() == id1, "duplicate from undo");
    OSL_ENSURE(!m3.GetMetadataReference().Second.getLength(), "duplicate from undo");

    m4.EnsureMetadataReference(); // new!
    beans::StringPair m4id(m4.GetMetadataReference());
    OSL_ENSURE(m4id.Second.getLength() && !(m4id == id1), "ensure on duplicate");

    MockMetadatable mc1(true); // in clipboard
    MockMetadatable mc2(true);
    MockMetadatable mc3(true);
    MockMetadatable mc4(true);
    MockMetadatable m2p;
    MockMetadatable m3p;

    mc1.SetMetadataReference(id2);
    OSL_ENSURE(mc1.GetMetadataReference() == id2, "set failed");
    try {
        mc2.SetMetadataReference(id2);
        OSL_ENSURE(false, "set duplicate succeeded");
    } catch (lang::IllegalArgumentException) { }
    mc1.SetMetadataReference(id2);
    OSL_ENSURE(mc1.GetMetadataReference() == id2, "set failed (existing)");
    mc1.EnsureMetadataReference();
    OSL_ENSURE(mc1.GetMetadataReference() == id2, "ensure failed (existing)");
    mc2.EnsureMetadataReference();
    beans::StringPair mc2id(mc2.GetMetadataReference());
    OSL_ENSURE(mc2id.Second.getLength(), "ensure failed");
    mc2.EnsureMetadataReference();
    OSL_ENSURE(mc2.GetMetadataReference() == mc2id, "ensure failed (idempotent)");
    mc2.RemoveMetadataReference();
    OSL_ENSURE(!mc2.GetMetadataReference().Second.getLength(), "remove failed");

    // set up mc2 as copy of m2 and mc3 as copy of m3
    mc3.RegisterAsCopyOf(m3);
    OSL_ENSURE(!mc3.GetMetadataReference().Second.getLength() , "copy to clipboard (latent)");
    mc2.RegisterAsCopyOf(m2);
    OSL_ENSURE(mc2.GetMetadataReference() == id1, "copy to clipboard (non-latent)");
    // paste mc2 to m2p and mc3 to m3p
    m2p.RegisterAsCopyOf(mc2);
    OSL_ENSURE(!m2p.GetMetadataReference().Second.getLength() , "paste from clipboard (non-latent)");
    m3p.RegisterAsCopyOf(mc3);
    OSL_ENSURE(!m3p.GetMetadataReference().Second.getLength() , "paste from clipboard (latent)");
    // delete m2, m2p, m3
    m2.RemoveMetadataReference();
    OSL_ENSURE(!m2.GetMetadataReference().Second.getLength(), "remove failed");
    OSL_ENSURE(m2p.GetMetadataReference() == id1, "paste-remove (non-latent)");
    m2p.RemoveMetadataReference();
    OSL_ENSURE(!m2p.GetMetadataReference().Second.getLength(), "remove failed");
    OSL_ENSURE(m3.GetMetadataReference() == id1, "paste-remove2 (non-latent)");
    m3.RemoveMetadataReference();
    OSL_ENSURE(!m3.GetMetadataReference().Second.getLength(), "remove failed");
    OSL_ENSURE(m3p.GetMetadataReference() == id1, "paste-remove (latent)");
    // delete mc2
    mc2.SetMetadataReference(beans::StringPair());
    OSL_ENSURE(!mc3.GetMetadataReference().Second.getLength() , "in clipboard becomes non-latent");
    // paste mc2
    m2p.RegisterAsCopyOf(mc2);
    OSL_ENSURE(!m2p.GetMetadataReference().Second.getLength(), "remove-paste");
    OSL_ENSURE(m3p.GetMetadataReference() == id1, "remove-paste (stolen)");

    // auto-detect stream
    m5.SetMetadataReference(id3e);
    OSL_ENSURE(m5.GetMetadataReference() == id3, "auto-detect (content)");
    m5.m_bInContent = false;
    m5.SetMetadataReference(id4e);
    OSL_ENSURE(m5.GetMetadataReference() == id4, "auto-detect (styles)");

    OSL_TRACE("sfx2::Metadatable test(): finished\n");
}

struct Test { Test() { test(); }  };
static Test s_test;


#include <stdio.h>

static void dump(sfx2::XmlIdList_t * pList)
#ifdef GCC
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

