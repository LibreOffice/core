/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#if ! defined INCLUDED_DP_BACKENDDB_HXX
#define INCLUDED_DP_BACKENDDB_HXX

#include "rtl/ustring.hxx"
#include <list>
#include <vector>

namespace css = ::com::sun::star;

namespace com { namespace sun { namespace star {
        namespace uno {
        class XComponentContext;
        }
        namespace xml { namespace dom {
            class XDocument;
            class XNode;
        }}
        namespace xml { namespace xpath {
            class XXPathAPI;
        }}
}}}

namespace dp_registry {
namespace backend {

class BackendDb
{
private:

    css::uno::Reference<css::xml::dom::XDocument> m_doc;
    css::uno::Reference<css::xml::xpath::XXPathAPI> m_xpathApi;

    BackendDb(BackendDb const &);
    BackendDb &  operator = (BackendDb const &);

protected:
    const css::uno::Reference<css::uno::XComponentContext> m_xContext;
    ::rtl::OUString m_urlDb;

protected:

    /* caller must make sure that only one thread accesses the function
     */
    css::uno::Reference<css::xml::dom::XDocument> getDocument();

    /* the namespace prefix is "reg" (without quotes)
     */
    css::uno::Reference<css::xml::xpath::XXPathAPI> getXPathAPI();
    void save();
    void removeElement(::rtl::OUString const & sXPathExpression);

    css::uno::Reference<css::xml::dom::XNode> getKeyElement(
        ::rtl::OUString const & url);

    void writeSimpleList(
        ::std::list< ::rtl::OUString> const & list,
        ::rtl::OUString const & sListTagName,
        ::rtl::OUString const & sMemberTagName,
        css::uno::Reference<css::xml::dom::XNode> const & xParent);

    void writeVectorOfPair(
        ::std::vector< ::std::pair< ::rtl::OUString, ::rtl::OUString > > const & vecPairs,
        ::rtl::OUString const & sVectorTagName,
        ::rtl::OUString const & sPairTagName,
        ::rtl::OUString const & sFirstTagName,
        ::rtl::OUString const & sSecondTagName,
        css::uno::Reference<css::xml::dom::XNode> const & xParent);

    void writeSimpleElement(
        ::rtl::OUString const & sElementName, ::rtl::OUString const & value,
        css::uno::Reference<css::xml::dom::XNode> const & xParent);

    css::uno::Reference<css::xml::dom::XNode> writeKeyElement(
        ::rtl::OUString const & url);

    ::rtl::OUString readSimpleElement(
        ::rtl::OUString const & sElementName,
        css::uno::Reference<css::xml::dom::XNode> const & xParent);

    ::std::vector< ::std::pair< ::rtl::OUString, ::rtl::OUString > >
    readVectorOfPair(
        css::uno::Reference<css::xml::dom::XNode> const & parent,
        ::rtl::OUString const & sListTagName,
        ::rtl::OUString const & sPairTagName,
        ::rtl::OUString const & sFirstTagName,
        ::rtl::OUString const & sSecondTagName);

    ::std::list< ::rtl::OUString> readList(
        css::uno::Reference<css::xml::dom::XNode> const & parent,
        ::rtl::OUString const & sListTagName,
        ::rtl::OUString const & sMemberTagName);

    /* returns the values of one particulary child element of all key elements.
     */
    ::std::list< ::rtl::OUString> getOneChildFromAllEntries(
        ::rtl::OUString const & sElementName);


    /*  returns the namespace which is to be written as xmlns attribute
        into the root element.
     */
    virtual ::rtl::OUString getDbNSName()=0;
    /* return the namespace prefix which is to be registered with the XPath API.

       The prefix can then be used in XPath expressions.
    */
    virtual ::rtl::OUString getNSPrefix()=0;
    /* returns the name of the root element without any namespace prefix.
     */
    virtual ::rtl::OUString getRootElementName()=0;
    /* returns the name of xml element for each entry
     */
    virtual ::rtl::OUString getKeyElementName()=0;



public:
    BackendDb(css::uno::Reference<css::uno::XComponentContext> const &  xContext,
              ::rtl::OUString const & url);
    virtual ~BackendDb() {};

    void removeEntry(::rtl::OUString const & url);

    /* This is called to write the "revoked" attribute to the entry.
       This is done when XPackage::revokePackage is called.
    */
    void revokeEntry(::rtl::OUString const & url);

    /* returns false if the entry does not exist yet.
     */
    bool activateEntry(::rtl::OUString const & url);

    bool hasActiveEntry(::rtl::OUString const & url);

};

class RegisteredDb: public BackendDb
{

public:
    RegisteredDb( css::uno::Reference<css::uno::XComponentContext> const &  xContext,
                  ::rtl::OUString const & url);
    virtual ~RegisteredDb() {};


    virtual void addEntry(::rtl::OUString const & url);
    virtual bool getEntry(::rtl::OUString const & url);

};


}
}
#endif

