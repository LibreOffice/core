/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MNSRunnable.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:26:45 $
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

#ifndef _CONNECTIVITY_MAB_MOZAB_RUNNABLE_HXX_
#define _CONNECTIVITY_MAB_MOZAB_RUNNABLE_HXX_
#ifndef _CONNECTIVITY_MAB_NS_DECLARES_HXX_
#include "MNSDeclares.hxx"
#endif
#ifndef _THREAD_HXX_
#include <osl/thread.hxx>
#endif

#include <MNSInclude.hxx>

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_MOZILLA_XCODEPROXY_HDL_
#include <com/sun/star/mozilla/XCodeProxy.hpp>
#endif


using namespace com::sun::star::uno;
using namespace com::sun::star::mozilla;

namespace connectivity
{
    namespace mozab
    {
    class MNSRunnable : public nsIRunnable
        {
        public:
            nsIRunnable * ProxiedObject();
            MNSRunnable();
            virtual ~MNSRunnable();
            NS_DECL_ISUPPORTS
            NS_DECL_NSIRUNNABLE

        public:
            sal_Int32 StartProxy(const ::com::sun::star::uno::Reference< ::com::sun::star::mozilla::XCodeProxy >& aCode); //Call this to start proxy

        private:
            nsIRunnable* _ProxiedObject;
            Reference<XCodeProxy>  xCode;
#if OSL_DEBUG_LEVEL > 0
            oslThreadIdentifier m_oThreadID;
#endif
        };
    }
}
#endif //_CONNECTIVITY_MAB_MOZABHELPER_HXX_
