/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: confignode.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:37:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _UNOTOOLS_CONFIGNODE_HXX_
#define _UNOTOOLS_CONFIGNODE_HXX_

#ifndef INCLUDED_UNOTOOLSDLLAPI_H
#include "unotools/unotoolsdllapi.h"
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif
#ifndef _UNOTOOLS_EVENTLISTENERADAPTER_HXX_
#include <unotools/eventlisteneradapter.hxx>
#endif

//........................................................................
namespace utl
{
//........................................................................

    //========================================================================
    //= OConfigurationNode
    //========================================================================
    class OConfigurationTreeRoot;
    /** a small wrapper around a configuration node.<p/>
        Nodes in the terminology used herein are <em>inner</em> nodes of a configuration
        tree, which means <em>no leafs</em>.
    */
    class UNOTOOLS_DLLPUBLIC OConfigurationNode : public ::utl::OEventListenerAdapter
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XHierarchicalNameAccess >
                    m_xHierarchyAccess;     /// accessing children grandchildren (mandatory interface of our UNO object)
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                    m_xDirectAccess;        /// accessing children  (mandatory interface of our UNO object)
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace >
                    m_xReplaceAccess;       /// replacing child values
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                    m_xContainerAccess;     /// modifying set nodes  (optional interface of our UNO object)
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                    m_xProvider;            /// the configuration provider which supplied the nodes of the hierarchy we're a part of
        sal_Bool    m_bEscapeNames;         /// escape names before accessing children ?

        ::rtl::OUString
                    m_sCompletePath;

        OConfigurationNode  insertNode(const ::rtl::OUString& _rName,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xNode) const throw();
    protected:
        /// constructs a node object with an interface representing a node
        OConfigurationNode(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxNode,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxProvider);

    public:
        /// constructs an empty and invalid node object
        OConfigurationNode() :m_bEscapeNames(sal_False) { }
        /// copy ctor
        OConfigurationNode(const OConfigurationNode& _rSource);

        /// assigment
        const OConfigurationNode& operator=(const OConfigurationNode& _rSource);

        /// dtor
        ~OConfigurationNode();

        /** open a sub node
            @param      _rPath      access path of the to-be-opened sub node. May be a hierarchical path.
        */
        OConfigurationNode  openNode(const ::rtl::OUString& _rPath) const throw();

        OConfigurationNode  openNode( const sal_Char* _pAsciiPath ) const
        {
            return openNode( ::rtl::OUString::createFromAscii( _pAsciiPath ) );
        }

        /** create a new child node

            If the object represents a set node, this method may be used to create a new child. For non-set-nodes, the
            method will fail.<br/>
            Unless the respective operations on the pure configuration API, the to-be-created node immediately
            becomes a part of it's hierarchy, no explicit insertion is necessary.
            @param      _rName      name for the new child. Must be level-1-depth.
        */
        OConfigurationNode  createNode(const ::rtl::OUString& _rName) const throw();

        OConfigurationNode  createNode( const sal_Char* _pAsciiName ) const
        {
            return createNode( ::rtl::OUString::createFromAscii( _pAsciiName ) );
        }

        /** appends a node under a new name

            If the object represents a set node, this method may be used to create a new child. For non-set-nodes, the
            method will fail.<br/>
            Unless the respective operations on the pure configuration API, the to-be-created node immediately
            becomes a part of it's hierarchy, no explicit insertion is necessary.
            @param      _rName      name for the new child. Must be level-1-depth.
            @param      _aNewNode   the node which should be appended
        */
        OConfigurationNode  appendNode(const ::rtl::OUString& _rName,const OConfigurationNode& _aNewNode) const throw();

        OConfigurationNode  appendNode( const sal_Char* _pAsciiName, const OConfigurationNode& _aNewNode ) const
        {
            return appendNode( ::rtl::OUString::createFromAscii( _pAsciiName ), _aNewNode );
        }

        /** remove an existent child nod

            If the object represents a set node, this method may be used to delete an existent child. For non-set-nodes,
            the method will fail.
        */
        sal_Bool            removeNode(const ::rtl::OUString& _rName) const throw();

        sal_Bool            removeNode( const sal_Char* _pAsciiName ) const
        {
            return removeNode( ::rtl::OUString::createFromAscii( _pAsciiName ) );
        }

        /** retrieves the content of a descendant

            the returned value may contain anything from an interface (if <arg>_rPath</arg> refers to inner node of
            the configuration tree) to any explicit value (e.g. string, integer) or even void.<br/>
            Unfortunately, this implies that if a void value is returned, you won't have a clue if this means
            "the path does not exist" (besides the assertion made :), or if the value is really void.
        */
        ::com::sun::star::uno::Any
                            getNodeValue(const ::rtl::OUString& _rPath) const throw();

        ::com::sun::star::uno::Any
                            getNodeValue( const sal_Char* _pAsciiPath ) const
        {
            return getNodeValue( ::rtl::OUString::createFromAscii( _pAsciiPath ) );
        }

        /** write a node value<p/>
            The value given is written into the node specified by the given relative path.<br/>
            In opposite to <method>getNodeValue</method>, _rName must refer to a leaf in the configuration tree, not an inner
            node.
            @return     sal_True if and only if the write was successfull.
        */
        sal_Bool            setNodeValue(const ::rtl::OUString& _rPath, const ::com::sun::star::uno::Any& _rValue) const throw();

        sal_Bool            setNodeValue( const sal_Char* _pAsciiPath, const ::com::sun::star::uno::Any& _rValue ) const
        {
            return setNodeValue( ::rtl::OUString::createFromAscii( _pAsciiPath ), _rValue );
        }

        /// return the names of the existing children
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                            getNodeNames() const throw();

        /** enables or disables name escaping when accessing direct children<p/>
            Escaping is disabled by default, usually you enable it for set nodes (e.g. with calling setEscape(isSetNode)).
            Once escaping is enabled, you should not access indirect children (e.g. openNode("child/grandchild"), 'cause
            escaping for such names may not be supported by the underlying API objects.
            @see getEscape
        */
        void        setEscape(sal_Bool _bEnable = sal_True);
        /** get the flag specifying the current escape behaviour
            @see setEscape
        */
        sal_Bool    getEscape() const { return m_bEscapeNames; }

        /** clone the object, creating a new hierarchy where the root represents the same node as the object
            where this method is called does.<p/>
            The new hierarchy does not share any updates with the one the object is a part of, i.e. any changes made
            in the existing hierarchy are not visible to the newly created one until committed (and vice versa).
            @param      _rRoot      out parameter. upon return, contains the object representing the root of the new hierarchy.
        */
        OConfigurationTreeRoot cloneAsRoot() const throw();

        /// invalidate the object
        virtual void clear() throw();

        // -----------------------
        // meta informations about the node

        /// checks whether or not the object represents a set node.
        sal_Bool isSetNode() const;

        /// checks whether or not a direct child with a given name exists
        sal_Bool hasByName(const ::rtl::OUString& _rName) const throw();
        sal_Bool hasByName( const sal_Char* _pAsciiName ) const { return hasByName( ::rtl::OUString::createFromAscii( _pAsciiName ) ); }

        /// checks whether or not a descendent (no matter if direct or indirect) with the given name exists
        sal_Bool hasByHierarchicalName( const ::rtl::OUString& _rName ) const throw();
        sal_Bool hasByHierarchicalName( const sal_Char* _pAsciiName ) const { return hasByHierarchicalName( ::rtl::OUString::createFromAscii( _pAsciiName ) ); }

        /// check if the objects represents a valid configuration node
        sal_Bool isValid() const { return m_xHierarchyAccess.is(); }

        /// check whether the object is read-only of updatable
        sal_Bool isReadonly() const { return !m_xReplaceAccess.is(); }

    protected:
        // OEventListenerAdapter
        virtual void _disposing( const ::com::sun::star::lang::EventObject& _rSource );

    protected:
        enum NAMEORIGIN
        {
            NO_CONFIGURATION,       /// the name came from a configuration node
            NO_CALLER               /// the name came from a client of this class
        };
        ::rtl::OUString normalizeName(const ::rtl::OUString& _rName, NAMEORIGIN _eOrigin) const;
    };

    //========================================================================
    //= OConfigurationTreeRoot
    //========================================================================
    /** a specialized version of a OConfigurationNode, representing the root
        of a configuration sub tree<p/>
        Only this class is able to commit any changes made any any OConfigurationNode
        objects.
    */
    class UNOTOOLS_DLLPUBLIC OConfigurationTreeRoot : public OConfigurationNode
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XChangesBatch >
                                m_xCommitter;
    protected:
        /** ctor<p/>
        */
        OConfigurationTreeRoot(
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XChangesBatch >& _rxRootNode,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxProvider);

        /** ctor for a readonly node
        */
        OConfigurationTreeRoot(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxRootNode,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxProvider);

    public:
        /// modes to used when creating a top-level node object
        enum CREATION_MODE
        {
            CM_READONLY,            /// open the node (i.e. sub tree) for read access only
            CM_UPDATABLE,           /// open the node (i.e. sub tree) for read and write access
            CM_PREFER_UPDATABLE     /// open the node (i.e. sub tree) for read and write access, if this fails, open it for read acces
        };

    public:
        /** default ctor<p/>
            The object constructed here is invalid (i.e. <method>isValid</method> will return sal_False).
        */
        OConfigurationTreeRoot() :OConfigurationNode() { }

        /// copy ctor
        OConfigurationTreeRoot(const OConfigurationTreeRoot& _rSource)
            :OConfigurationNode(_rSource), m_xCommitter(_rSource.m_xCommitter) { }

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
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxConfProvider,
                const ::rtl::OUString& _rPath,
                sal_Int32 _nDepth = -1,
                CREATION_MODE _eMode = CM_PREFER_UPDATABLE,
                sal_Bool _bLazyWrite = sal_True
            );

        /** open a new top-level configuration node<p/>
            opens a new node which is the root if an own configuration sub tree. This is what "top level" means: The
            node does not have a parent. It does not mean that the node represents a module tree (like org.openoffice.Office.Writer
            or such).<br/>
            In opposite to <method>createWithProvider</method>, createWithProvider expects a service factory. This factory
            is used to create a configuration provider, and this provider is used to retrieve the node
            @see    createWithProvider
            @param      _rxORB          service factory to use to create the configuration provider.
            @param      _rPath          path to the node the object should represent
            @param      _nDepth         depth for node retrieval
            @param      _eMode          specifies which privileges should be applied when retrieving the node
        */
        static OConfigurationTreeRoot createWithServiceFactory(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::rtl::OUString& _rPath, sal_Int32 _nDepth = -1, CREATION_MODE _eMode = CM_PREFER_UPDATABLE, sal_Bool _bLazyWrite = sal_True);

        /** tolerant version of the <member>createWithServiceFactory</member>

            <p>No assertions are thrown in case of an failure to initialize the configuration service, but once
            the configuration could be initialized, errors in the creation of the specific node (e.g. because the
            given node path does not exist) are still asserted.</p>
        */
        static OConfigurationTreeRoot tryCreateWithServiceFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::rtl::OUString& _rPath, sal_Int32 _nDepth = -1, CREATION_MODE _eMode = CM_PREFER_UPDATABLE, sal_Bool _bLazyWrite = sal_True );

        /** commit all changes made on the subtree the object is the root for<p/>
            All changes made on any <type>OConfigurationNode</type> object retrieved (maybe indirect) from this root
            object are committed when calling this method.
            @return     sal_True if and only if the commit was successfull
        */
        sal_Bool commit() const throw();

        /// invalidate the object
        virtual void clear() throw();
    };

//........................................................................
}   // namespace utl
//........................................................................

#endif // _UNOTOOLS_CONFIGNODE_HXX_

