/*************************************************************************
 *
 *  $RCSfile: treeprovider.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2000-11-15 07:14:58 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
//#ifndef __SGI_STL_MEMORY
//#include <stl/memory>
//#endif
//#ifndef _MAP_
//#include <map>
//#endif
//#include <string.h>

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

    struct TreeChangeList;

    //==========================================================================
    //= ITreeProvider
    //==========================================================================
    class ITreeProvider : public ISynchronizedData
    {
    public:
        enum { ALL_LEVELS = -1  };

        virtual ISubtree const* getSubtree( OUString const& aComponentName ) const = 0;
        virtual ISubtree *      requestSubtree( OUString const& aSubtreePath, sal_Int16 nMinLevels = ALL_LEVELS) throw (container::NoSuchElementException) = 0;

        /** returns the node for the given path. In opposite to <method>getSubtree</method>, this methods covers
            leafs, too.
        */
        virtual const INode *   getNode(const OUString& _rPath) = 0;

        virtual void updateTree(TreeChangeList& aChanges) throw (lang::WrappedTargetException, uno::RuntimeException) = 0;
    };

    class ITreeNotifier;

    //==========================================================================
    //= ITreeListener
    //==========================================================================
    class ITreeListener
    {
    public:
          virtual void disposing(ITreeNotifier* pNotifier) = 0;
          virtual void changes(TreeChangeList const& , sal_Bool _bError) = 0;
//        virtual void changes(sal_Int32 _nNotificationId,const ::rtl::OUString& _rNotifyReason) = 0;
    };

    //==========================================================================
    //= ITreeNotifier
    //==========================================================================
    class ITreeNotifier
    {
    public:
        virtual void setListener(ITreeListener* _pListener) = 0;
    };

    //==========================================================================
    //= ITreeProvider2
    //==========================================================================
    // a ITreeProvider which can notify changes that were done
    class ITreeProvider2 : public ITreeProvider
    {
    public:
        virtual void notifyUpdate(TreeChangeList const& aChanges) const throw (uno::RuntimeException) = 0;
    };


    //==========================================================================
    //= ITemplateProvider
    //==========================================================================
    class ITemplateProvider
    {
    public:
        virtual ::std::auto_ptr<INode> createInstance(const ::rtl::OUString& _rTemplateName) throw (uno::Exception) = 0;
    };

    //==========================================================================
    //= ref-counted providers
    //==========================================================================
    class IRefCountedTreeProvider : public ITreeProvider, public IInterface
    {
    };

    class IRefCountedTreeProvider2 : public ITreeProvider2, public IInterface
    {
    };

    class IRefCountedTemplateProvider : public ITemplateProvider, public IInterface
    {
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
            @param      _rNodePath      the path of the node which was updated. If you're interested in the new
                                        node values, you should call getNode on the session.
        */
        virtual void    nodeUpdated(const ::rtl::OUString& _rNodePath, std::auto_ptr<INode> node) = 0;

        /** called whenever another session deleted a new node (assumed the the listener is registered for an ancestor
            of this new node)
            @param      _rNodePath      the path of the node which was deleted.
        */
        virtual void    nodeDeleted(const ::rtl::OUString& _rNodePath, std::auto_ptr<INode> node) = 0;

        /** called whenever another session added a new node (assumed the the listener is registered for an ancestor
            of this new node)
            @param      _rNodePath      the path of the node which was added. If you're interested in the new
                                        node values, you should call getNode on the session.
        */
        virtual void    nodeAdded(const ::rtl::OUString& _rNodePath, std::auto_ptr<INode> node) = 0;
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

