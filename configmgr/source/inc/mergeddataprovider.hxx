/*************************************************************************
 *
 *  $RCSfile: mergeddataprovider.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:18:59 $
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

#ifndef CONFIGMGR_BACKEND_MERGEDDATAPROVIDER_HXX
#define CONFIGMGR_BACKEND_MERGEDDATAPROVIDER_HXX

#ifndef CONFIGMGR_BACKEND_REQUEST_HXX_
#include "request.hxx"
#endif
#ifndef CONFIGMGR_BACKEND_REQUESTTYPES_HXX_
#include "requesttypes.hxx"
#endif

#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif

namespace configmgr
{
// ---------------------------------------------------------------------------
    namespace backend
    {
// ---------------------------------------------------------------------------

    /** Listener interface for receiving notifications
        about changes to previously requested data
    */
    struct SAL_NO_VTABLE INodeDataListener : Refcounted
    {
        /** is called to indicate changes within the data being observed.

            @param _aOriginalRequest
                identifies the data that changed
        */
        virtual void dataChanged(NodeRequest const & _aOriginalRequest) CFG_NOTHROW() = 0;
    };
// ---------------------------------------------------------------------------

    /// Interface providing access to (merged) data for whole components
    struct SAL_NO_VTABLE IComponentDataProvider
    {
        /** loads merged data for a (complete) tree and returns it as return value.

            @param _aRequest
                identifies the component to be loaded

            @returns
                A valid component instance for the given component.

            @throws com::sun::star::uno::Exception
                if the node cannot be retrieved.
                The exact exception being thrown may depend on the underlying backend.

        */
        virtual ComponentResult getComponentData(ComponentRequest const & _aRequest)
            CFG_UNO_THROW_ALL() = 0;
    };

// ---------------------------------------------------------------------------

    /// Interface providing access to template (schema) data
    struct SAL_NO_VTABLE ITemplateDataProvider
    {
        /** loads a given template and returns it as return value

            @param _aRequest
                identifies the template to be loaded

            @returns
                A valid instance of the given template.

                <p> Currently a request with empty template name
                    will retrieve a group node holding all templates
                    of a component.
                </p>

            @throws com::sun::star::uno::Exception
                if the template cannot be retrieved.
                The exact exception being thrown may depend on the underlying backend.
        */
        virtual TemplateResult getTemplateData(TemplateRequest const & _aRequest)
            CFG_UNO_THROW_ALL() = 0;
    };
// ---------------------------------------------------------------------------

    /** Interface providing access to (merged) data for individual nodes
        with optional notification about subsequent changes.

        There is no guarantee, that all changes are notified immediately
        (or even at all).If the provider cannot detect changes
        it may simply ignore a supplied listener.
     */
    struct SAL_NO_VTABLE INodeDataProvider
    {
        /** loads merged data for a (partial) tree and returns it as return value.

            @param _aRequest
                identifies the component to be loaded

            @param _pListener
                a listener to observe subsequent changes to the data requested

                <p> If NULL, no notifications will be sent. </p>

                <p> Otherwise the listener will be notified of changes.
                    The listener must subsequently be removed by calling
                    <member>INodeDataProvider::removeRequestListener</member>.
                    The listener must live at least until it is removed.
                </p>

            @returns
                A valid component instance for the given request.

            @throws com::sun::star::uno::Exception
                if the node cannot be retrieved.
                The exact exception being thrown may depend on the underlying backend.
        */
        virtual ComponentResult getNodeData(ComponentRequest const & _aRequest,
                                            ITemplateDataProvider*   _aTemplateProvider,
                                            INodeDataListener * _pListener = NULL)
            CFG_UNO_THROW_ALL() = 0;

        /** remove a listener registered for a previous request.
            <p>This may also release some open resources for the request.</p>

            @param _pListener
                a listener that was passed to a previous succes
        */
        virtual void removeRequestListener(INodeDataListener * _pListener) CFG_NOTHROW() = 0;
    };
// ---------------------------------------------------------------------------

    /// Interface providing the capability to update node data
    struct SAL_NO_VTABLE INodeUpdateProvider
    {
        /** applies an update to the stored data.

            @param _anUpdate
                identifies the node to be updated and
                describes the changes to be applied.

            @throws com::sun::star::uno::Exception
                if the node cannot be updated.
                The exact exception being thrown may depend on the underlying backend.
        */
        virtual void updateNodeData(UpdateRequest const & _anUpdate)
            CFG_UNO_THROW_ALL() = 0;
    };
// ---------------------------------------------------------------------------

    /// Interface providing access to (merged) default data
    struct SAL_NO_VTABLE IDefaultDataProvider
    {
        /** loads default data for a (partial) tree and returns it as return value

            @param _aRequest
                identifies the node to be loaded

            @returns
                A valid node instance for the default state of the given node.

                <p>May be NULL, if the node exists but has no default equivalent.</p>

            @throws com::sun::star::uno::Exception
                if the default cannot be retrieved.
                The exact exception being thrown may depend on the underlying backend.
        */
        virtual NodeResult getDefaultData(NodeRequest const & _aRequest)
            CFG_UNO_THROW_ALL() = 0;
    };

// ---------------------------------------------------------------------------

    /// Interface providing access to backend meta-data
    struct SAL_NO_VTABLE IDataProviderMetaData
    {
        /** Queries whether default property values are stripped from
            a merged result tree or whether they are returned inline.

            @returns
                <TRUE/>  if default data is stripped from a merged node result, <BR/>
                <FALSE/> if default data is left in the merged node result
        */
        virtual bool isStrippingDefaults() CFG_NOTHROW() = 0;
    };
// ---------------------------------------------------------------------------

    /** Composite interface providing simple direct access to (merged) configuration data
        from some data store.

        <p> Only loading data is supported (no updates or notifications). </p>
    */
    struct IDirectDataProvider
    : Refcounted
    , IComponentDataProvider
    , IDefaultDataProvider
    , ITemplateDataProvider
    {
    };
// ---------------------------------------------------------------------------

    /** Composite interface providing full access to merged configuration data
        from some data store.

        <p> Loading and updating of data is supported.
            Support for notification depends on the backend.
        </p>
    */
    struct IMergedDataProvider
    : Refcounted
    , INodeDataProvider
    , INodeUpdateProvider
    , IDefaultDataProvider
    , ITemplateDataProvider
    , IDataProviderMetaData
    {
    };

// ---------------------------------------------------------------------------
    } // namespace backend

// ---------------------------------------------------------------------------
} // namespace configmgr

#endif

