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
#ifndef INCLUDED_UNOTOOLS_CONFIGNODE_HXX
#define INCLUDED_UNOTOOLS_CONFIGNODE_HXX

#include <unotools/unotoolsdllapi.h>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <unotools/eventlisteneradapter.hxx>

namespace comphelper
{
    class ComponentContext;
}

namespace utl
{

    //= OConfigurationNode

    class OConfigurationTreeRoot;
    /** a small wrapper around a configuration node.<p/>
        Nodes in the terminology used herein are <em>inner</em> nodes of a configuration
        tree, which means <em>no leafs</em>.
    */
    class UNOTOOLS_DLLPUBLIC OConfigurationNode : public ::utl::OEventListenerAdapter
    {
    private:
        css::uno::Reference< css::container::XHierarchicalNameAccess >
                    m_xHierarchyAccess;     /// accessing children grandchildren (mandatory interface of our UNO object)
        css::uno::Reference< css::container::XNameAccess >
                    m_xDirectAccess;        /// accessing children  (mandatory interface of our UNO object)
        css::uno::Reference< css::container::XNameReplace >
                    m_xReplaceAccess;       /// replacing child values
        css::uno::Reference< css::container::XNameContainer >
                    m_xContainerAccess;     /// modifying set nodes  (optional interface of our UNO object)
        bool        m_bEscapeNames;         /// escape names before accessing children ?

        OConfigurationNode  insertNode(const OUString& _rName,const css::uno::Reference< css::uno::XInterface >& _xNode) const throw();

    protected:
        /// constructs a node object with an interface representing a node
        OConfigurationNode(
            const css::uno::Reference< css::uno::XInterface >& _rxNode
        );

        const css::uno::Reference< css::container::XNameAccess >&
            getUNONode() const { return m_xDirectAccess; }

    public:
        /// constructs an empty and invalid node object
        OConfigurationNode() :m_bEscapeNames(false) { }
        /// copy ctor
        OConfigurationNode(const OConfigurationNode& _rSource);
        /// move ctor
        OConfigurationNode(OConfigurationNode&& _rSource);

        /// assignment
        OConfigurationNode& operator=(const OConfigurationNode& _rSource);
        OConfigurationNode& operator=(OConfigurationNode&& _rSource);

        /// returns the local name of the node
        OUString     getLocalName() const;

        /** open a sub node
            @param      _rPath      access path of the to-be-opened sub node. May be a hierarchical path.
        */
        OConfigurationNode  openNode(const OUString& _rPath) const throw();

        OConfigurationNode  openNode( const sal_Char* _pAsciiPath ) const
        {
            return openNode( OUString::createFromAscii( _pAsciiPath ) );
        }

        /** create a new child node

            If the object represents a set node, this method may be used to create a new child. For non-set-nodes, the
            method will fail.<br/>
            Unless the respective operations on the pure configuration API, the to-be-created node immediately
            becomes a part of its hierarchy, no explicit insertion is necessary.
            @param      _rName      name for the new child. Must be level-1-depth.
        */
        OConfigurationNode  createNode(const OUString& _rName) const throw();

        /** remove an existent child nod

            If the object represents a set node, this method may be used to delete an existent child. For non-set-nodes,
            the method will fail.
        */
        bool            removeNode(const OUString& _rName) const throw();

        /** retrieves the content of a descendant

            the returned value may contain anything from an interface (if <arg>_rPath</arg> refers to inner node of
            the configuration tree) to any explicit value (e.g. string, integer) or even void.<br/>
            Unfortunately, this implies that if a void value is returned, you won't have a clue if this means
            "the path does not exist" (besides the assertion made :), or if the value is really void.
        */
        css::uno::Any       getNodeValue(const OUString& _rPath) const throw();

        css::uno::Any       getNodeValue( const sal_Char* _pAsciiPath ) const
        {
            return getNodeValue( OUString::createFromAscii( _pAsciiPath ) );
        }

        /** write a node value<p/>
            The value given is written into the node specified by the given relative path.<br/>
            In opposite to <method>getNodeValue</method>, _rName must refer to a leaf in the configuration tree, not an inner
            node.
            @return     sal_True if and only if the write was successful.
        */
        bool            setNodeValue(const OUString& _rPath, const css::uno::Any& _rValue) const throw();

        bool            setNodeValue( const sal_Char* _pAsciiPath, const css::uno::Any& _rValue ) const
        {
            return setNodeValue( OUString::createFromAscii( _pAsciiPath ), _rValue );
        }

        /// return the names of the existing children
        css::uno::Sequence< OUString >
                            getNodeNames() const throw();

        /** get the flag specifying the current escape behaviour
            @see setEscape
        */
        bool    getEscape() const { return m_bEscapeNames; }

        /// invalidate the object
        virtual void clear() throw();

        // meta information about the node

        /// checks whether or not the object represents a set node.
        bool isSetNode() const;

        /// checks whether or not a direct child with a given name exists
        bool hasByName(const OUString& _rName) const throw();
        bool hasByName( const sal_Char* _pAsciiName ) const { return hasByName( OUString::createFromAscii( _pAsciiName ) ); }

        /// checks whether or not a descendent (no matter if direct or indirect) with the given name exists
        bool hasByHierarchicalName( const OUString& _rName ) const throw();

        /// check if the objects represents a valid configuration node
        bool isValid() const { return m_xHierarchyAccess.is(); }

        /// check whether the object is read-only of updatable
        bool isReadonly() const { return !m_xReplaceAccess.is(); }

    protected:
        // OEventListenerAdapter
        virtual void _disposing( const css::lang::EventObject& _rSource ) override;

    protected:
        enum NAMEORIGIN
        {
            NO_CONFIGURATION,       /// the name came from a configuration node
            NO_CALLER               /// the name came from a client of this class
        };
        OUString normalizeName(const OUString& _rName, NAMEORIGIN _eOrigin) const;
    };

    //= OConfigurationTreeRoot

    /** a specialized version of a OConfigurationNode, representing the root
        of a configuration sub tree<p/>
        Only this class is able to commit any changes made any any OConfigurationNode
        objects.
    */
    class UNOTOOLS_DLLPUBLIC OConfigurationTreeRoot : public OConfigurationNode
    {
        css::uno::Reference< css::util::XChangesBatch >
                                m_xCommitter;
    protected:
        /** ctor for a readonly node
        */
        OConfigurationTreeRoot(
            const css::uno::Reference< css::uno::XInterface >& _rxRootNode
        );

    public:
        /// modes to use when creating a top-level node object
        enum CREATION_MODE
        {
            /// open the node (i.e. sub tree) for read access only
            CM_READONLY,
            /// open the node (i.e. sub tree) for read and write access, fall back to read-only if write access is not possible
            CM_UPDATABLE
        };

    public:
        /** default ctor<p/>
            The object constructed here is invalid (i.e. <method>isValid</method> will return sal_False).
        */
        OConfigurationTreeRoot() :OConfigurationNode() { }

        /** creates a configuration tree for the given path in the given mode
        */
        OConfigurationTreeRoot(
            const css::uno::Reference<css::uno::XComponentContext> & i_rContext,
            const OUString& i_rNodePath,
            const bool i_bUpdatable
        );

        /** open a new top-level configuration node

            opens a new node which is the root if an own configuration sub tree. This is what "top level" means: The
            node does not have a parent. It does not mean that the node represents a module tree (like org.openoffice.Office.Writer
            or such).<br/>
            In opposite to <method>createWithServiceFactory</method>, createWithProvider expects a configuration provider
            to work with.

            @param      _rxConfProvider configuration provider to use when retrieving the node.
            @param      _rPath          path to the node the object should represent
            @param      _nDepth         depth for node retrieval
            @param      _eMode          specifies which privileges should be applied when retrieving the node

            @see    createWithServiceFactory
        */
        static OConfigurationTreeRoot createWithProvider(
                const css::uno::Reference< css::lang::XMultiServiceFactory >& _rxConfProvider,
                const OUString& _rPath,
                sal_Int32 _nDepth,
                CREATION_MODE _eMode
            );

        /** open a new top-level configuration node<p/>
            opens a new node which is the root if an own configuration sub tree. This is what "top level" means: The
            node does not have a parent. It does not mean that the node represents a module tree (like org.openoffice.Office.Writer
            or such).<br/>
            In opposite to <method>createWithProvider</method>, createWithProvider expects a service factory. This factory
            is used to create a configuration provider, and this provider is used to retrieve the node
            @see    createWithProvider
            @param      _rxContext      service factory to use to create the configuration provider.
            @param      _rPath          path to the node the object should represent
            @param      _nDepth         depth for node retrieval
            @param      _eMode          specifies which privileges should be applied when retrieving the node
        */
        static OConfigurationTreeRoot createWithComponentContext(const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
            const OUString& _rPath, sal_Int32 _nDepth = -1, CREATION_MODE _eMode = CM_UPDATABLE);

        /** tolerant version of the <member>createWithServiceFactory</member>

            <p>No assertions are thrown in case of an failure to initialize the configuration service, but once
            the configuration could be initialized, errors in the creation of the specific node (e.g. because the
            given node path does not exist) are still asserted.</p>
        */
        static OConfigurationTreeRoot tryCreateWithComponentContext( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
            const OUString& _rPath, sal_Int32 _nDepth = -1, CREATION_MODE _eMode = CM_UPDATABLE );

        /** commit all changes made on the subtree the object is the root for<p/>
            All changes made on any OConfigurationNode object retrieved (maybe indirect) from this root
            object are committed when calling this method.
            @return     sal_True if and only if the commit was successful
        */
        bool commit() const throw();

        /// invalidate the object
        virtual void clear() throw() override;
    };

}   // namespace utl

#endif // INCLUDED_UNOTOOLS_CONFIGNODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
