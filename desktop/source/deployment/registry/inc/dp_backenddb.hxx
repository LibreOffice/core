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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_REGISTRY_INC_DP_BACKENDDB_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_REGISTRY_INC_DP_BACKENDDB_HXX

#include <rtl/ustring.hxx>
#include <list>
#include <vector>

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

    BackendDb(BackendDb const &) = delete;
    BackendDb &  operator = (BackendDb const &) = delete;

protected:
    const css::uno::Reference<css::uno::XComponentContext> m_xContext;
    OUString m_urlDb;

protected:

    /* caller must make sure that only one thread accesses the function
     */
    css::uno::Reference<css::xml::dom::XDocument> const & getDocument();

    /* the namespace prefix is "reg" (without quotes)
     */
    css::uno::Reference<css::xml::xpath::XXPathAPI> const & getXPathAPI();
    void save();
    void removeElement(OUString const & sXPathExpression);

    css::uno::Reference<css::xml::dom::XNode> getKeyElement(
        OUString const & url);

    void writeSimpleList(
        ::std::list< OUString> const & list,
        OUString const & sListTagName,
        OUString const & sMemberTagName,
        css::uno::Reference<css::xml::dom::XNode> const & xParent);

    void writeVectorOfPair(
        ::std::vector< ::std::pair< OUString, OUString > > const & vecPairs,
        OUString const & sVectorTagName,
        OUString const & sPairTagName,
        OUString const & sFirstTagName,
        OUString const & sSecondTagName,
        css::uno::Reference<css::xml::dom::XNode> const & xParent);

    void writeSimpleElement(
        OUString const & sElementName, OUString const & value,
        css::uno::Reference<css::xml::dom::XNode> const & xParent);

    css::uno::Reference<css::xml::dom::XNode> writeKeyElement(
        OUString const & url);

    OUString readSimpleElement(
        OUString const & sElementName,
        css::uno::Reference<css::xml::dom::XNode> const & xParent);

    ::std::vector< ::std::pair< OUString, OUString > >
    readVectorOfPair(
        css::uno::Reference<css::xml::dom::XNode> const & parent,
        OUString const & sListTagName,
        OUString const & sPairTagName,
        OUString const & sFirstTagName,
        OUString const & sSecondTagName);

    ::std::list< OUString> readList(
        css::uno::Reference<css::xml::dom::XNode> const & parent,
        OUString const & sListTagName,
        OUString const & sMemberTagName);

    /* returns the values of one particularly child element of all key elements.
     */
    ::std::list< OUString> getOneChildFromAllEntries(
        OUString const & sElementName);


    /*  returns the namespace which is to be written as xmlns attribute
        into the root element.
     */
    virtual OUString getDbNSName()=0;
    /* return the namespace prefix which is to be registered with the XPath API.

       The prefix can then be used in XPath expressions.
    */
    virtual OUString getNSPrefix()=0;
    /* returns the name of the root element without any namespace prefix.
     */
    virtual OUString getRootElementName()=0;
    /* returns the name of xml element for each entry
     */
    virtual OUString getKeyElementName()=0;

public:
    BackendDb(css::uno::Reference<css::uno::XComponentContext> const &  xContext,
              OUString const & url);
    virtual ~BackendDb() {};

    void removeEntry(OUString const & url);

    /* This is called to write the "revoked" attribute to the entry.
       This is done when XPackage::revokePackage is called.
    */
    void revokeEntry(OUString const & url);

    /* returns false if the entry does not exist yet.
     */
    bool activateEntry(OUString const & url);

    bool hasActiveEntry(OUString const & url);

};

class RegisteredDb: public BackendDb
{

public:
    RegisteredDb( css::uno::Reference<css::uno::XComponentContext> const &  xContext,
                  OUString const & url);
    virtual ~RegisteredDb() {};


    void addEntry(OUString const & url);
};

}
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
