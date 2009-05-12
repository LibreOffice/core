/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: translatechanges.hxx,v $
 * $Revision: 1.11 $
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

#ifndef CONFIGMGR_API_TRANSLATECHANGES_HXX_
#define CONFIGMGR_API_TRANSLATECHANGES_HXX_

#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include "rtl/ref.hxx"

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
        // resolve the relative path from a given base node to the changed node
        bool resolveChangeLocation( configuration::RelativePath& aPath,
                                    configuration::NodeChangeLocation const& aChange,
                                    rtl::Reference< configuration::Tree > const& aBaseTree,
                                    configuration::NodeRef const& aBaseNode);

        // change path and base settings to start from the given base tree (root)
        bool rebaseChange(  configuration::NodeChangeLocation& aChange,
                            rtl::Reference< configuration::Tree > const& _aBaseTreeRef);
        // change path and base settings to start from the given base node
        bool rebaseChange(  configuration::NodeChangeLocation& aChange,
                            rtl::Reference< configuration::Tree > const& _aBaseTreeRef,
                            configuration::NodeRef const& aBaseNode);
        // resolve non-uno elements to Uno Objects
        bool resolveUnoObjects(UnoChange& aUnoChange,
                               configuration::NodeChangeData const& aChange,
                               Factory& rFactory);
        // resolve non-uno elements to Uno Objects inplace
        bool resolveToUno(configuration::NodeChangeData& aChange,
                          Factory& rFactory);

    // building events

        /// fill a change info from a NodeChangeInfo
        void fillChange(util::ElementChange& rChange,
                        configuration::NodeChangeInformation const& aInfo,
                        rtl::Reference< configuration::Tree > const& aBaseTree,
                        Factory& rFactory);
        /// fill a change info from a NodeChangeInfo
        void fillChange(util::ElementChange& rChange,
                        configuration::NodeChangeInformation const& aInfo,
                        rtl::Reference< configuration::Tree > const& aBaseTree,
                        configuration::NodeRef const& aBaseNode,
                        Factory& rFactory);
        /// fill a change info from a NodeChangeInfo (base,path and uno objects are assumed to be resolved already)
        void fillChangeFromResolved(util::ElementChange& rChange, configuration::NodeChangeInformation const& aInfo);

        /// fill a event from a NodeChangeInfo (uno objects are assumed to be resolved already)
        bool fillEventDataFromResolved(container::ContainerEvent& rEvent, configuration::NodeChangeInformation const& aInfo);
        /// fill a event from a NodeChangeInfo(uno objects are assumed to be resolved already) - returns false if this isn't a property change
        bool fillEventDataFromResolved(beans::PropertyChangeEvent& rEvent, configuration::NodeChangeInformation const& aInfo, bool bMore);
    }
// ---------------------------------------------------------------------------------------------------
}

#endif // CONFIGMGR_API_TRANSLATECHANGES_HXX_
