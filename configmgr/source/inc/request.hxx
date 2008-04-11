/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: request.hxx,v $
 * $Revision: 1.6 $
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

#ifndef CONFIGMGR_BACKEND_REQUEST_HXX_
#define CONFIGMGR_BACKEND_REQUEST_HXX_

#include "requestoptions.hxx"
#include "requesttypes.hxx"
#include "configpath.hxx"

namespace configmgr
{
// ---------------------------------------------------------------------------
    namespace backend
    {
// ---------------------------------------------------------------------------
        using configuration::AbsolutePath;
        using configuration::Name;
// ---------------------------------------------------------------------------

        class NodeRequest
        {
            AbsolutePath    m_aNodePath;
            RequestOptions  m_aOptions;
        public:
            NodeRequest(AbsolutePath const& _aNodePath, RequestOptions const & _aOptions)
            : m_aNodePath(_aNodePath)
            , m_aOptions(_aOptions)
            {
            }

            AbsolutePath    const & getPath()       const { return m_aNodePath; }
            RequestOptions  const & getOptions()    const { return m_aOptions; }
        };
// ---------------------------------------------------------------------------

        class ComponentRequest
        {
            Name    m_aComponentName;
            RequestOptions  m_aOptions;
            bool            m_bForcedReload;
        public:
            ComponentRequest(Name const& _aComponentName, RequestOptions const & _aOptions)
            : m_aComponentName(_aComponentName)
            , m_aOptions(_aOptions)
            , m_bForcedReload(false)
            {
            }

            Name            const & getComponentName()  const { return m_aComponentName; }
            RequestOptions  const & getOptions()        const { return m_aOptions; }

            bool isForcingReload() const { return m_bForcedReload; }
            void forceReload(bool _bForce = true) { m_bForcedReload = _bForce; }
     };
// ---------------------------------------------------------------------------

        class TemplateRequest
        {
            Name    m_aComponentName;
            Name    m_aTemplateName;

        public:
            static
            TemplateRequest forComponent(Name const & _aComponentName)
            {
                return TemplateRequest( Name(), _aComponentName);
            }

            explicit
            TemplateRequest(Name const & _aTemplateName, Name const & _aComponentName)
            : m_aComponentName(_aComponentName)
            , m_aTemplateName(_aTemplateName)
            {}

            bool isComponentRequest() const { return m_aTemplateName.isEmpty(); }
            Name getTemplateName()      const { return m_aTemplateName; }
            Name getComponentName()     const { return m_aComponentName; }

            static RequestOptions getOptions()
            { return RequestOptions::forAllLocales(); }
        };

        inline ComponentRequest getComponentRequest(TemplateRequest const & _aTR)
        { return ComponentRequest(_aTR.getComponentName(), _aTR.getOptions()); }
// ---------------------------------------------------------------------------

        class UpdateRequest
        {
            typedef rtl::OUString RequestId;

            ConstUpdateInstance  m_aUpdate;
            RequestOptions  m_aOptions;
            RequestId       m_aRQID;
        public:
            explicit
            UpdateRequest(  UpdateInstance const & _aUpdate,
                            RequestOptions const & _aOptions)
            : m_aUpdate(_aUpdate)
            , m_aOptions(_aOptions)
            {}

            explicit
            UpdateRequest(  ConstUpdateInstance const & _aUpdate,
                            RequestOptions const & _aOptions)
            : m_aUpdate(_aUpdate)
            , m_aOptions(_aOptions)
            {}

            explicit
            UpdateRequest(  ConstUpdateInstance::Data _aUpdateData,
                            AbsolutePath const & _aRootpath,
                            RequestOptions const & _aOptions)
            : m_aUpdate(_aUpdateData, _aRootpath)
            , m_aOptions(_aOptions)
            {}

            bool isSyncRequired() const { return !m_aOptions.isAsyncEnabled(); }

            RequestOptions const & getOptions()  const { return m_aOptions; }
            NodePath const & getUpdateRoot()     const { return m_aUpdate.root(); }

            ConstUpdateInstance const & getUpdate()   const { return m_aUpdate; }
            ConstUpdateInstance::Data getUpdateData() const { return m_aUpdate.data(); }

            void setRequestId(RequestId const & _aRQID) { m_aRQID = _aRQID; }
            RequestId getRequestId() const { return m_aRQID; }
        };

        inline ComponentRequest getComponentRequest(UpdateRequest const & _aUR)
        { return ComponentRequest(_aUR.getUpdateRoot().getModuleName(), _aUR.getOptions()); }
// ---------------------------------------------------------------------------
    } // namespace
// ---------------------------------------------------------------------------
} // namespace

#endif
