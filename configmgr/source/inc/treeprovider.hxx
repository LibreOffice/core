/*************************************************************************
 *
 *  $RCSfile: treeprovider.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: jb $ $Date: 2001-07-05 17:05:46 $
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

#ifndef _CONFIGMGR_COMMONTYPES_HXX_
#include "commontypes.hxx"
#endif
#ifndef _CONFIGMGR_SYNCHRONIZE_HXX_
#include "synchronize.hxx"
#endif

#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
#endif
#ifndef CONFIGMGR_MISC_OPTIONS_HXX_
#include <options.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif

#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif


namespace configmgr
{

    namespace uno = com::sun::star::uno;
    using ::rtl::OUString;

////////////////////////////////////////////////////////////////////////////////
    namespace configuration
    {
        class Name;
        class AbsolutePath;
    }
    //-------------------------
    class ISubtree;
    struct TreeChangeList;

    //==========================================================================
    //= ITreeProvider
    //==========================================================================
    class ITreeProvider
    {
    public:
        typedef configuration::AbsolutePath AbsolutePath;

        enum { ALL_LEVELS = -1  };

        /// load the tree named by a path using certain options and requiring a specific loading depth - return it yielding ownership
        virtual std::auto_ptr<ISubtree> loadSubtree(AbsolutePath const& aSubtreePath,
                                                      const vos::ORef < OOptions >& _xOptions,
                                                      sal_Int16 nMinLevels = ALL_LEVELS) throw (uno::Exception) = 0;

        /// update the stored data according to a changes list
        virtual void updateTree(TreeChangeList& aChanges) throw (uno::Exception) = 0;

    };

    //==========================================================================
    //= ITreeManager
    //==========================================================================
    // a ITreeProvider which can notify changes that were done, and manages the lifetime of subtrees

    class ITreeManager : public ISynchronizedData
    {
    public:
        typedef configuration::AbsolutePath AbsolutePath;

         enum { ALL_LEVELS = ITreeProvider::ALL_LEVELS  };

       /** request that the tree named by a path is added to the collection of managed trees
            respecting certain options and requiring a specific loading depth.
            Return a reference to that managed tree.
            The reference must later be released by calling releaseSubtree with the same path and options.
        */
        virtual ISubtree * requestSubtree(AbsolutePath const& aSubtreePath,
                                          const vos::ORef < OOptions >& _xOptions,
                                          sal_Int16 nMinLevels = ALL_LEVELS) throw (uno::Exception) = 0;

        /** request that the tree named by a path is added to the collection of managed trees
            respecting certain options and requiring a specific loading depth.
        */
        virtual void fetchSubtree(AbsolutePath const& aSubtreePath,
                                  const vos::ORef < OOptions >& _xOptions,
                                  sal_Int16 nMinLevels = ALL_LEVELS) throw() = 0;

        /// update the managed data according to a changes list - update the changes list accordingly with old values
        virtual void updateTree(TreeChangeList& aChanges) throw (uno::Exception) = 0;

        // notification
        virtual void notifyUpdate(TreeChangeList const& aChanges ) throw (uno::RuntimeException) = 0;

        // bookkeeping support
        virtual void releaseSubtree(AbsolutePath const& aSubtreePath,
                                    const vos::ORef < OOptions >& _xOptions ) throw () = 0;

        /** data for the given options may not be used any more
            <p>all clients of such data must be disposed</p>
            <p>If the locale is not set, the whole user has become invalid</p>
        */
        virtual void disposeData(const vos::ORef < OOptions >& _xOptions) throw () = 0;


    };


    //==========================================================================
    //= ITemplateProvider
    //==========================================================================
    class ITemplateProvider
    {
    public:
        typedef configuration::Name         Name;

        virtual ::std::auto_ptr<INode> requestTemplateInstance(
                                            Name const& aName, Name const& aModule,
                                            const vos::ORef < OOptions >& _xOptions
                                        ) throw (uno::Exception) = 0;

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

