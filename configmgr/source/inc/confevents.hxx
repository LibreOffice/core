/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: confevents.hxx,v $
 * $Revision: 1.10 $
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

#ifndef CONFIGMGR_API_EVENTS_HXX_
#define CONFIGMGR_API_EVENTS_HXX_

#include "utility.hxx"
#include <rtl/ref.hxx>

namespace rtl { class OUString; }

namespace configmgr
{
    class Change;
    struct TreeChangeList;
    class RequestOptions;

    namespace configuration { class AbsolutePath; }
    using configuration::AbsolutePath;

    struct IConfigBroadcaster;
    struct IConfigListener : public virtual configmgr::SimpleReferenceObject
    {
        virtual void disposing(IConfigBroadcaster* pSource) = 0;
    };
    struct INodeListener : IConfigListener
    {
        virtual void nodeChanged(Change const& aChange, AbsolutePath const& aPath, IConfigBroadcaster* pSource) = 0;
        virtual void nodeDeleted(AbsolutePath const& aPath, IConfigBroadcaster* pSource) = 0;
    };
    typedef rtl::Reference<INodeListener> INodeListenerRef;

    struct IConfigBroadcaster
    {
    protected:
        IConfigBroadcaster() {}
        virtual ~IConfigBroadcaster() {}
    public:
        virtual void addListener(AbsolutePath const& aPath, const RequestOptions& _aOptions, INodeListenerRef const& pListener) = 0;
        virtual void removeListener(const RequestOptions& _aOptions, INodeListenerRef const& pListener) = 0;

    };

    class ConfigChangeBroadcastHelper; // broadcasts changes for a given set of options
    class ConfigChangeBroadcaster : public IConfigBroadcaster
    {
    public:
        ConfigChangeBroadcaster();
        virtual ~ConfigChangeBroadcaster();

        virtual void addListener(AbsolutePath const& aName, const RequestOptions& _aOptions, INodeListenerRef const& pListener);
        virtual void removeListener(const RequestOptions& _aOptions, INodeListenerRef const& pListener);

    protected:
        virtual void fireChanges(TreeChangeList const& _aChanges, sal_Bool _bError);
    protected:
        virtual ConfigChangeBroadcastHelper* getBroadcastHelper(const RequestOptions& _aOptions, bool bCreate) = 0;
        ConfigChangeBroadcastHelper* newBroadcastHelper(); // needed to implement the preceding
        void disposeBroadcastHelper(ConfigChangeBroadcastHelper* pHelper); // needed to discard the preceding
    };

} // namespace

#endif // CONFIGMGR_API_EVENTS_HXX_



