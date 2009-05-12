/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mergeddataprovider.hxx,v $
 * $Revision: 1.7 $
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

#ifndef CONFIGMGR_BACKEND_MERGEDDATAPROVIDER_HXX
#define CONFIGMGR_BACKEND_MERGEDDATAPROVIDER_HXX

#include "sal/config.h"

#include "salhelper/simplereferenceobject.hxx"

#include "request.hxx"
#include "requesttypes.hxx"
#include "utility.hxx"

namespace configmgr
{
// ---------------------------------------------------------------------------
    namespace backend
    {
// ---------------------------------------------------------------------------
    /** Listener interface for receiving notifications
        about changes to previously requested data
    */
    struct SAL_NO_VTABLE INodeDataListener
    {
        /** is called to indicate changes within the data being observed.

            @param _aOriginalRequest
                identifies the data that changed
        */
        virtual void dataChanged(ComponentRequest const & _aOriginalRequest) SAL_THROW(()) = 0;
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
        virtual ResultHolder< TemplateInstance > getTemplateData(TemplateRequest const & _aRequest)
            SAL_THROW((com::sun::star::uno::Exception)) = 0;
    };
// ---------------------------------------------------------------------------

    /** Composite interface providing full access to merged configuration data
        from some data store.

        <p> Loading and updating of data is supported.
            Support for notification depends on the backend.
        </p>
    */
    struct IMergedDataProvider
    : salhelper::SimpleReferenceObject
    , ITemplateDataProvider
    {
        /** loads merged data for a (partial) tree and returns it as return value.

            @param _aRequest
                identifies the component to be loaded

            @param _pListener
                a listener to observe subsequent changes to the data requested

                <p> If NULL, no notifications will be sent. </p>

                <p> Otherwise the listener will be notified of changes.
                    The listener must subsequently be removed by calling
                    <member>removeRequestListener</member>.
                    The listener must live at least until it is removed.
                </p>

            @returns
                A valid component instance for the given request.

            @throws com::sun::star::uno::Exception
                if the node cannot be retrieved.
                The exact exception being thrown may depend on the underlying backend.
        */
        virtual ResultHolder< ComponentInstance > getNodeData(ComponentRequest const & _aRequest,
                                            ITemplateDataProvider*   _aTemplateProvider,
                                            INodeDataListener * _pListener = NULL)
            SAL_THROW((com::sun::star::uno::Exception)) = 0;

        /** remove a listener registered for a previous request.
            <p>This may also release some open resources for the request.</p>

            @param _pListener
                a listener that was passed to a previous succes

             @param _aRequest
                identifies the component associated with the listener
        */
        virtual void removeRequestListener(INodeDataListener * _pListener,
                                           const ComponentRequest& aRequest) SAL_THROW(()) = 0;

        /** applies an update to the stored data.

            @param _anUpdate
                identifies the node to be updated and
                describes the changes to be applied.

            @throws com::sun::star::uno::Exception
                if the node cannot be updated.
                The exact exception being thrown may depend on the underlying backend.
        */
        virtual void updateNodeData(UpdateRequest const & _anUpdate)
            SAL_THROW((com::sun::star::uno::Exception)) = 0;

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
        virtual ResultHolder< NodeInstance > getDefaultData(NodeRequest const & _aRequest)
            SAL_THROW((com::sun::star::uno::Exception)) = 0;
    };

// ---------------------------------------------------------------------------
    } // namespace backend

// ---------------------------------------------------------------------------
} // namespace configmgr

#endif

