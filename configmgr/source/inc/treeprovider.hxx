/*************************************************************************
 *
 *  $RCSfile: treeprovider.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-21 12:16:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/* PLEASE DON'T DELETE ANY COMMENT LINES, ALSO IT'S UNNECESSARY. */


#ifndef CONFIGMGR_TREEPROVIDER_HXX
#define CONFIGMGR_TREEPROVIDER_HXX

#include "change.hxx"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _CONFIGMGR_COMMONTYPES_HXX_
#include "commontypes.hxx"
#endif
#ifndef _CONFIGMGR_SYNCHRONIZE_HXX_
#include "synchronize.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_TYPE_HXX_
#include <com/sun/star/uno/Type.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_WRAPPEDTARGETEXCEPTION_HPP_
#include <com/sun/star/lang/WrappedTargetException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_NOSUCHELEMENTEXCEPTION_HPP_
#include <com/sun/star/container/NoSuchElementException.hpp>
#endif
#ifndef CONFIGMGR_MISC_OPTIONS_HXX_
#include <options.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef CONFIGMGR_CONFNAME_HXX_
#include "confname.hxx"
#endif

namespace configmgr
{

    namespace uno = com::sun::star::uno;
    namespace lang = com::sun::star::lang;
    namespace container = com::sun::star::container;
    using ::rtl::OUString;

////////////////////////////////////////////////////////////////////////////////


    //==========================================================================
    //= TreeChangeList
    //==========================================================================
    struct TreeChangeList
    {
        vos::ORef < OOptions > m_xOptions;       // options for the tree that is concerned by these changes
        ConfigurationName pathToRoot;            // absolute path to the root of the whole to-be-updated subtree
        SubtreeChange root;                      // changes made within this sub tree
        // TreeChangeList(): root(::rtl::OUString(), configuration::Attributes()){}

        TreeChangeList(const vos::ORef < OOptions >& _xOptions, const rtl::OUString& _rPathToRoot, const SubtreeChange& _aSubtree)
                : m_xOptions(_xOptions),
                  pathToRoot(_rPathToRoot, ConfigurationName::Absolute()),
                  root(_aSubtree)   /* EXPENSIVE!!! (deep copy) */
            {}

        // TreeChangeList(const vos::ORef < OOptions >& _xOptions, const rtl::OUString& _rPathToRoot, auto_ptr<SubtreeChange> _pSubtreeChange)
        //         : m_xOptions(_xOptions),
        //           pathToRoot(_rPathToRoot),
        //           root(*_pSubtreeChange.release()) /* EXPENSIVE!!! (deep copy) */
        //     {}
        /** ctor
        @param      _rPathToRoot        path to the root of the whole to-be-updated subtree
        @param      _rLocalName         relative path within the to-be-updated subtree
        */
        TreeChangeList( const vos::ORef < OOptions >& _xOptions,
                        const rtl::OUString& _rPathToRoot,
                        const rtl::OUString& _rLocalName,
                        const configuration::Attributes& _rAttr)
                : m_xOptions(_xOptions)
                , pathToRoot(_rPathToRoot, ConfigurationName::Absolute())
                , root(_rLocalName, _rAttr)
        {}

        /** ctor
        @param      _rPathToRoot        path to the root of the whole to-be-updated subtree
        @param      _rLocalName         relative path within the to-be-updated subtree
        */
        TreeChangeList( const vos::ORef < OOptions >& _xOptions,
                        const rtl::OUString& _rPathToRoot,
                        const rtl::OUString& _rLocalName,
                        const rtl::OUString& _rChildTemplateName,
                        const rtl::OUString& _rChildTemplateModule,
                        const configuration::Attributes& _rAttr)
                : m_xOptions(_xOptions)
                , pathToRoot(_rPathToRoot, ConfigurationName::Absolute())
                , root(_rLocalName, _rChildTemplateName, _rChildTemplateModule, _rAttr)
        {}

        /** ctor
        @param      _rPathToRoot        path to the root of the whole to-be-updated subtree
        @param      _rLocalName         relative path within the to-be-updated subtree
        */
        TreeChangeList( const vos::ORef < OOptions >& _xOptions,
                        const rtl::OUString& _rPathToRoot,
                        const ISubtree& _rTree)
                : m_xOptions(_xOptions)
                , pathToRoot(_rPathToRoot, ConfigurationName::Absolute())
                , root(_rTree)
        {
            OSL_ENSURE(false, "Test only, because deep copy of subtreechange is very expensive.");
        }

        /** ctor
        @param      _rTreeList          list to initialize the path, no childs are copied
        */
        TreeChangeList( const TreeChangeList& _rTree, SubtreeChange::NoChildCopy _rNoCopy)
            : m_xOptions(_rTree.m_xOptions)
            , pathToRoot(_rTree.pathToRoot)
            , root(_rTree.root, _rNoCopy)
        {}

        /// get the module these changes belong to
        OUString getModuleName() const;
    };

    //==========================================================================
    //= ITreeProvider
    //==========================================================================
    class ITreeProvider : public ISynchronizedData
    {
    public:
        enum { ALL_LEVELS = -1  };

        virtual ISubtree * requestSubtree(OUString const& aSubtreePath,
                                          const vos::ORef < OOptions >& _xOptions,
                                          sal_Int16 nMinLevels = ALL_LEVELS) throw (uno::Exception) = 0;
        virtual void updateTree(TreeChangeList& aChanges) throw (uno::Exception) = 0;

    };

    //==========================================================================
    //= ITreeManager
    //==========================================================================
    // a ITreeProvider which can notify changes that were done, and manages the lifetime of subtrees

    class ITreeManager : public ITreeProvider
    {
    public:
        // notification
        virtual void notifyUpdate(TreeChangeList const& aChanges ) throw (uno::RuntimeException) = 0;

        // bookkeeping support
        virtual void releaseSubtree(OUString const& aSubtreePath,
                                    const vos::ORef < OOptions >& _xOptions ) throw () = 0;

        /** data for the given options may not be used any more
            <p>all clients of such data must be disposed</p>
            <p>If the locale is not set, the whole user has become invalid</p>
        */
        virtual void disposeData(const vos::ORef < OOptions >& _xOptions) throw () = 0;


        /** initiates the fetch of a Subtree */
        virtual void fetchSubtree(OUString const& aSubtreePath,
                                  const vos::ORef < OOptions >& _xOptions,
                                  sal_Int16 nMinLevels = ALL_LEVELS) throw() = 0;
    };


    //==========================================================================
    //= ITemplateProvider
    //==========================================================================
    class ITemplateProvider
    {
    public:
        virtual ::std::auto_ptr<INode> createInstance(const ::rtl::OUString& _rTemplateName) throw (uno::Exception) = 0;
    };

////////////////////////////////////////////////////////////////////////////////

    //==========================================================================
    //= INotifyListener
    //==========================================================================
    /** a listener on configuration nodes. able to receive all changes in one or more
        specific registry sub trees.
    */
    class INotifyListener : public IInterface
    {
    public:
        /** called whenever another session modified a node which the listener is registered for
            @param      _rChanges       The list of changes for a node.
        */
        virtual void    nodeUpdated(TreeChangeList& _rChanges) = 0;
    };

    //==========================================================================
    //= INotifyBroadcaster
    //==========================================================================
    /** a broadcaster of changes on configuration nodes. Able to broadcast all changes in one or more
        specific registry sub trees.

    */
    class INotifyBroadcaster
    {
    public:

        /** registers a listener for node changes.
        */
        virtual void    setNotifyListener(const ::vos::ORef< INotifyListener >& _rListener) = 0;
    };

} // namespace configmgr

#endif

