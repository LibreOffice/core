/*************************************************************************
 *
 *  $RCSfile: request.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2002-03-12 14:10:33 $
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

#ifndef CONFIGMGR_BACKEND_REQUEST_HXX_
#define CONFIGMGR_BACKEND_REQUEST_HXX_

#ifndef CONFIGMGR_MISC_REQUESTOPTIONS_HXX_
#include "requestoptions.hxx"
#endif
#ifndef CONFIGMGR_BACKEND_REQUESTTYPES_HXX_
#include "requesttypes.hxx"
#endif
#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif

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

        public:
            ComponentRequest(Name const& _aComponentName, RequestOptions const & _aOptions)
            : m_aComponentName(_aComponentName)
            , m_aOptions(_aOptions)
            {
            }

            Name            const & getComponentName()  const { return m_aComponentName; }
            RequestOptions  const & getOptions()        const { return m_aOptions; }
        };
// ---------------------------------------------------------------------------

        class TemplateRequest
        {
            Name    m_aComponentName;
            Name    m_aTemplateName;

        public:
            explicit
            TemplateRequest(Name const & _aComponentName)
            : m_aComponentName(_aComponentName)
            , m_aTemplateName()
            {}

            explicit
            TemplateRequest(Name const & _aComponentName, Name const & _aTemplateName)
            : m_aComponentName(_aComponentName)
            , m_aTemplateName(_aTemplateName)
            {}

            bool isComponentRequest() const { return m_aTemplateName.isEmpty(); }
            Name getTemplateName()      const { return m_aTemplateName; }
            Name getComponentName()     const { return m_aComponentName; }

            static RequestOptions getOptions();
        };

        inline ComponentRequest getComponentRequest(TemplateRequest const & _aTR)
        { return ComponentRequest(_aTR.getComponentName(), _aTR.getOptions()); }
// ---------------------------------------------------------------------------

        class UpdateRequest
        {
            UpdateInstance  m_aUpdate;
            RequestOptions  m_aOptions;
            bool            m_bForceFlush;
        public:
            explicit
            UpdateRequest(UpdateInstance & _aUpdate, RequestOptions const & _aOptions)
            : m_aUpdate(_aUpdate)
            , m_aOptions(_aOptions)
            , m_bForceFlush( _aOptions.isForcingReload() )
            {}

            explicit
            UpdateRequest(AbsolutePath const & _aRootpath, UpdateInstance::Data & _aUpdateData, RequestOptions const & _aOptions)
            : m_aUpdate(_aRootpath,_aUpdateData)
            , m_aOptions(_aOptions)
            , m_bForceFlush( _aOptions.isForcingReload() )
            {}

            void forceFlush() { m_bForceFlush = true; }

            bool isForcingFlush() const { return m_bForceFlush; }

            UpdateInstance const & getUpdate()  const { return m_aUpdate; }
            RequestOptions const & getOptions() const { return m_aOptions; }
        };

        inline ComponentRequest getComponentRequest(UpdateRequest const & _aUR)
        { return ComponentRequest(_aUR.getUpdate().root.getModuleName(), _aUR.getOptions()); }
// ---------------------------------------------------------------------------
    } // namespace
// ---------------------------------------------------------------------------
} // namespace

#endif
