/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: translatechanges.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:09:45 $
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

#ifndef CONFIGMGR_API_TRANSLATECHANGES_HXX_
#define CONFIGMGR_API_TRANSLATECHANGES_HXX_

#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/util/XChangesListener.hpp>

namespace configmgr
{
// ---------------------------------------------------------------------------------------------------
    namespace css = ::com::sun::star;
    namespace uno       = css::uno;
    namespace lang      = css::lang;
    namespace util      = css::util;
    namespace beans     = css::beans;
    namespace container = css::container;
// ---------------------------------------------------------------------------------------------------

    namespace configuration
    {
        class NodeChangeInformation;
        class NodeChangeData;
        class NodeChangeLocation;

        //class NodeChange;
        //class NodeChanges;
        class Tree;
        class TreeRef;
        class NodeRef;
        class NodeID;
        class RelativePath;
    }
// ---------------------------------------------------------------------------------------------------

    namespace configapi
    {
        class NotifierImpl;
        class Factory;

        struct UnoChange { uno::Any newValue, oldValue; };

    //interpreting NodeChanges
        // resolve the relative path from a given base tree (root) to the changed node
        bool resolveChangeLocation( configuration::RelativePath& aPath,
                                    configuration::NodeChangeLocation const& aChange,
                                    configuration::Tree const& aBaseTree);
        // resolve the relative path from a given base node to the changed node
        bool resolveChangeLocation( configuration::RelativePath& aPath,
                                    configuration::NodeChangeLocation const& aChange,
                                    configuration::Tree const& aBaseTree,
                                    configuration::NodeRef const& aBaseNode);

        // change path and base settings to start from the given base tree (root)
        bool rebaseChange(  configuration::NodeChangeLocation& aChange,
                            configuration::TreeRef const& _aBaseTreeRef);
        // change path and base settings to start from the given base node
        bool rebaseChange(  configuration::NodeChangeLocation& aChange,
                            configuration::TreeRef const& _aBaseTreeRef,
                            configuration::NodeRef const& aBaseNode);
        // resolve non-uno elements to Uno Objects
        bool resolveUnoObjects(UnoChange& aUnoChange,
                               configuration::NodeChangeData const& aChange,
                               Factory& rFactory);
        // resolve non-uno elements to Uno Objects inplace
        bool resolveToUno(configuration::NodeChangeData& aChange,
                          Factory& rFactory);

    // building events
        /// find the sending api object
        void fillEventSource(lang::EventObject& rEvent, configuration::Tree const& aTree, configuration::NodeRef const& aNode, Factory& rFactory);

        /// fill a change info from a NodeChangeInfo
        void fillChange(util::ElementChange& rChange,
                        configuration::NodeChangeInformation const& aInfo,
                        configuration::Tree const& aBaseTree,
                        Factory& rFactory);
        /// fill a change info from a NodeChangeInfo
        void fillChange(util::ElementChange& rChange,
                        configuration::NodeChangeInformation const& aInfo,
                        configuration::Tree const& aBaseTree,
                        configuration::NodeRef const& aBaseNode,
                        Factory& rFactory);
        /// fill a change info from a NodeChangeInfo (base,path and uno objects are assumed to be resolved already)
        void fillChangeFromResolved(util::ElementChange& rChange, configuration::NodeChangeInformation const& aInfo);

        /// fill a event from a NodeChangeInfo
        bool fillEventData(container::ContainerEvent& rEvent, configuration::NodeChangeInformation const& aInfo, Factory& rFactory);
        /// fill a event from a NodeChangeInfo (uno objects are assumed to be resolved already)
        bool fillEventDataFromResolved(container::ContainerEvent& rEvent, configuration::NodeChangeInformation const& aInfo);
        /// fill a event from a NodeChangeInfo(uno objects are assumed to be resolved already) - returns false if this isn't a property change
        bool fillEventData(beans::PropertyChangeEvent& rEvent, configuration::NodeChangeInformation const& aInfo, Factory& rFactory, bool bMore);
        /// fill a event from a NodeChangeInfo(uno objects are assumed to be resolved already) - returns false if this isn't a property change
        bool fillEventDataFromResolved(beans::PropertyChangeEvent& rEvent, configuration::NodeChangeInformation const& aInfo, bool bMore);
    }
// ---------------------------------------------------------------------------------------------------
}

#endif // CONFIGMGR_API_TRANSLATECHANGES_HXX_
