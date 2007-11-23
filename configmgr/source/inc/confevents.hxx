/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: confevents.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:16:56 $
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

#ifndef CONFIGMGR_API_EVENTS_HXX_
#define CONFIGMGR_API_EVENTS_HXX_

#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

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



