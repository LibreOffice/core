/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: windowstateconfiguration.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 11:03:55 $
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

#ifndef __FRAMEWORK_UICONFIGURATION_WINDOWSTATECONFIGURATION_HXX_
#define __FRAMEWORK_UICONFIGURATION_WINDOWSTATECONFIGURATION_HXX_

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>
#include <list>
#include <hash_map>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
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

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <com/sun/star/frame/XModuleManager.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

namespace framework
{

#define WINDOWSTATE_PROPERTY_LOCKED         "Locked"
#define WINDOWSTATE_PROPERTY_DOCKED         "Docked"
#define WINDOWSTATE_PROPERTY_VISIBLE        "Visible"
#define WINDOWSTATE_PROPERTY_DOCKINGAREA    "DockingArea"
#define WINDOWSTATE_PROPERTY_DOCKPOS        "DockPos"
#define WINDOWSTATE_PROPERTY_DOCKSIZE       "DockSize"
#define WINDOWSTATE_PROPERTY_POS            "Pos"
#define WINDOWSTATE_PROPERTY_SIZE           "Size"
#define WINDOWSTATE_PROPERTY_UINAME         "UIName"
#define WINDOWSTATE_PROPERTY_INTERNALSTATE  "InternalState"
#define WINDOWSTATE_PROPERTY_STYLE          "Style"
#define WINDOWSTATE_PROPERTY_CONTEXT        "ContextSensitive"
#define WINDOWSTATE_PROPERTY_HIDEFROMENU    "HideFromToolbarMenu"
#define WINDOWSTATE_PROPERTY_NOCLOSE        "NoClose"
#define WINDOWSTATE_PROPERTY_SOFTCLOSE      "SoftClose"
#define WINDOWSTATE_PROPERTY_CONTEXTACTIVE  "ContextActive"

class ConfigurationAccess_Windows;
class WindowStateConfiguration :  public com::sun::star::lang::XTypeProvider    ,
                                  public com::sun::star::lang::XServiceInfo     ,
                                  public com::sun::star::container::XNameAccess ,
                                  private ThreadHelpBase                        ,   // Struct for right initalization of mutex member! Must be first of baseclasses.
                                  public ::cppu::OWeakObject
{
    public:
        WindowStateConfiguration( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
        virtual ~WindowStateConfiguration();

        //  XInterface, XTypeProvider, XServiceInfo
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        // XNameAccess
        virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
            throw ( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
            throw (::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
            throw (::com::sun::star::uno::RuntimeException);

        // XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
            throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL hasElements()
            throw (::com::sun::star::uno::RuntimeException);

        typedef ::std::hash_map< ::rtl::OUString,
                                 ::rtl::OUString,
                                 OUStringHashCode,
                                 ::std::equal_to< ::rtl::OUString > > ModuleToWindowStateFileMap;

        typedef ::std::hash_map< ::rtl::OUString,
                                 ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >,
                                 OUStringHashCode,
                                 ::std::equal_to< ::rtl::OUString > > ModuleToWindowStateConfigHashMap;

    private:
        sal_Bool                                                                            m_bConfigRead;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xServiceManager;
        ModuleToWindowStateFileMap                                                          m_aModuleToFileHashMap;
        ModuleToWindowStateConfigHashMap                                                    m_aModuleToWindowStateHashMap;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModuleManager > m_xModuleManager;
};

} // namespace framework

#endif // __FRAMEWORK_UIELEMENT_WINDOWSTATECONFIGURATION_HXX_
