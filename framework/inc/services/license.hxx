/*************************************************************************
 *
 *  $RCSfile: license.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-10 13:19:27 $
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

#ifndef __FRAMEWORK_SERVICES_LICENSE_HXX_
#define __FRAMEWORK_SERVICES_LICENSE_HXX_

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XSERVICEINFO_HXX_
#include <macros/xserviceinfo.hxx>
#endif

#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

#ifndef __FRAMEWORK_PROPERTIES_H_
#include <properties.h>
#endif

#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

#ifndef __FRAMEWORK_UIELEMENT_MENUBARMANAGER_HXX_
#include <uielement/menubarmanager.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XJOB_HPP_
#include <com/sun/star/task/XJob.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_CLOSEVETOEXCEPTION_HPP_
#include <com/sun/star/util/CloseVetoException.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

namespace framework
{
    class License : public  css::lang::XTypeProvider            ,
                    public  css::lang::XServiceInfo             ,
                    public  css::task::XJob                     ,
                    public  css::util::XCloseable               ,
                    // base classes
                    // Order is neccessary for right initialization!
                    private ThreadHelpBase                      , // Struct for right initalization of mutex member! Must be first of baseclasses.
                    public  ::cppu::OWeakObject                   // => XWeak, XInterface
    {
    private:
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xFactory;
        sal_Bool m_bTerminate;
    public:
        License( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rSMGR );
        virtual ~License();

        /** declaration of XInterface, XTypeProvider, XServiceInfo */
        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        DECL_STATIC_LINK( License, Terminate, void* );

        /*
        XJob...
        any execute([in] sequence< ::com::sun::star::beans::NamedValue > Arguments )
            raises(
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::Exception );
        */
        virtual css::uno::Any SAL_CALL execute(const css::uno::Sequence<css::beans::NamedValue>& args)
            throw( css::lang::IllegalArgumentException, css::uno::Exception);

        /*
        XCLoseable
        void close( [in] boolean DeliverOwnership ) raises(CloseVetoException );
        */
        virtual void SAL_CALL close(sal_Bool bDeliverOwnership) throw (css::util::CloseVetoException);

        /*
        XCloseVroadcaster
        [oneway] void addCloseListener([in] XCloseListenerListener );
        [oneway] void removeCloseListener([in] XCloseListenerListener );
        */
        virtual void SAL_CALL addCloseListener(const css::uno::Reference< css::util::XCloseListener >& aListener) throw (css::uno::RuntimeException);
        virtual void SAL_CALL removeCloseListener(const css::uno::Reference< css::util::XCloseListener >& aListener) throw (css::uno::RuntimeException);

};

} // namespace framework

#endif // __FRAMEWORK_SERVICES_LAYOUTMANAGER_HXX_
