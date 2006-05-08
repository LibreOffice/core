/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: addonsoptions.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-05-08 15:16:23 $
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
#ifndef __FRAMEWORK_CLASSES_ADDONSOPTIONS_HXX_
#define __FRAMEWORK_CLASSES_ADDONSOPTIONS_HXX_

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif

#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

//_________________________________________________________________________________________________________________
//  types, enums, ...
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @descr          The method GetAddonsMenu() returns a list of property values.
                    Use follow defines to seperate values by names.
*//*-*************************************************************************************************************/
#define ADDONSMENUITEM_PROPERTYNAME_URL                 ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URL"               ))
#define ADDONSMENUITEM_PROPERTYNAME_TITLE               ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Title"             ))
#define ADDONSMENUITEM_PROPERTYNAME_TARGET              ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Target"            ))
#define ADDONSMENUITEM_PROPERTYNAME_IMAGEIDENTIFIER     ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ImageIdentifier"   ))
#define ADDONSMENUITEM_PROPERTYNAME_CONTEXT             ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Context"           ))
#define ADDONSMENUITEM_PROPERTYNAME_SUBMENU             ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Submenu"           ))
#define ADDONSMENUITEM_PROPERTYNAME_CONTROLTYPE         ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ControlType"       ))
#define ADDONSMENUITEM_PROPERTYNAME_WIDTH               ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Width"             ))

#define ADDONSPOPUPMENU_URL_PREFIX_STR                  "private:menu/Addon"

#define ADDONSPOPUPMENU_URL_PREFIX                      ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( ADDONSPOPUPMENU_URL_PREFIX_STR ))

namespace framework
{

//_________________________________________________________________________________________________________________
//  forward declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          forward declaration to our private date container implementation
    @descr          We use these class as internal member to support small memory requirements.
                    You can create the container if it is neccessary. The class which use these mechanism
                    is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/

class AddonsOptions_Impl;

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          collect informations about menu features
    @descr          -

    @implements     -
    @base           -

    @devstatus      ready to use
*//*-*************************************************************************************************************/

class AddonsOptions
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:
        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      standard constructor and destructor
            @descr      This will initialize an instance with default values.
                        We implement these class with a refcount mechanism! Every instance of this class increase it
                        at create and decrease it at delete time - but all instances use the same data container!
                        He is implemented as a static member ...

            @seealso    member m_nRefCount
            @seealso    member m_pDataContainer

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

         AddonsOptions();
        ~AddonsOptions();

        //---------------------------------------------------------------------------------------------------------
        //  interface
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      clears completely the addons menu
            @descr      Call this methods to clear the addons menu
                        To fill it again use AppendItem().

            @seealso    -

            @param      "eMenu" select right menu to clear.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void Clear();

        /*-****************************************************************************************************//**
            @short      returns if an addons menu is available
            @descr      Call to retrieve if a addons menu is available


            @return     sal_True if there is a menu otherwise sal_False
        *//*-*****************************************************************************************************/

        sal_Bool    HasAddonsMenu() const;

        /*-****************************************************************************************************//**
            @short      returns if an addons help menu is available
            @descr      Call to retrieve if a addons menu is available


            @return     sal_True if there is a menu otherwise sal_False
        *//*-*****************************************************************************************************/

        sal_Bool    HasAddonsHelpMenu() const;

        /*-****************************************************************************************************//**
            @short      returns number of addons toolbars
            @descr      Call to retrieve the number of addons toolbars


            @return     number of addons toolbars
        *//*-*****************************************************************************************************/
        sal_Int32   GetAddonsToolBarCount() const ;

        /*-****************************************************************************************************//**
            @short      returns the  complete addons menu
            @descr      Call it to get all entries of the addon menu.
                        We return a list of all nodes with his names and properties.

            @seealso    -

            @return     A list of menu items is returned.

            @onerror    We return an empty list.
        *//*-*****************************************************************************************************/

        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& GetAddonsMenu() const;

        /*-****************************************************************************************************//**
            @short      Gets the menu bar part of all addon components registered
            @descr      -

            @seealso    -

            @return     A complete

            @onerror    We return sal_False
        *//*-*****************************************************************************************************/

        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& GetAddonsMenuBarPart() const;

        /*-****************************************************************************************************//**
            @short      Gets a toolbar part of an single addon
            @descr      -

            @seealso    -

            @return     A complete

            @onerror    We return sal_False
        *//*-*****************************************************************************************************/

        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& GetAddonsToolBarPart( sal_uInt32 nIndex ) const;

        /*-****************************************************************************************************//**
            @short      Gets a unique toolbar resource name of an single addon
            @descr      -

            @seealso    -

            @return     A complete

            @onerror    We return sal_False
        *//*-*****************************************************************************************************/

        const ::rtl::OUString GetAddonsToolbarResourceName( sal_uInt32 nIndex ) const;

        /*-****************************************************************************************************//**
            @short      Gets the Add-On help menu part of all addon components registered
            @descr      -

            @seealso    -

            @return     A complete

            @onerror    We return sal_False
        *//*-*****************************************************************************************************/
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& GetAddonsHelpMenu() const;

        /*-****************************************************************************************************//**
            @short      Retrieve an image for a command URL which is defined inside the addon menu configuration
            @descr      Call it to retrieve an image for a command URL which is defined inside the addon menu configuration

            @seealso    -

            @return     An image which was defined in the configuration for the menu item. The image can be empty
                        no bitmap was defined for the request image properties.

            @onerror    An empty image
        *//*-*****************************************************************************************************/

        Image GetImageFromURL( const rtl::OUString& aURL, sal_Bool bBig, sal_Bool bHiContrast, sal_Bool bNoScale ) const;

        Image GetImageFromURL( const rtl::OUString& aURL, sal_Bool bBig, sal_Bool bHiContrast ) const;
    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      return a reference to a static mutex
            @descr      These class is partially threadsafe (for de-/initialization only).
                        All access methods are'nt safe!
                        We create a static mutex only for one ime and use at different times.

            @seealso    -

            @param      -
            @return     A reference to a static mutex member.

            @onerror    -
        *//*-*****************************************************************************************************/

        static ::osl::Mutex& GetOwnStaticMutex();

        /*-****************************************************************************************************//**
            @short      return a reference to a static mutex
            @descr      These class is partially threadsafe (for de-/initialization only).
                        All access methods are'nt safe!
                        We create a static mutex only for one ime and use at different times.

            @seealso    -

            @param      -
            @return     A reference to a static mutex member.

            @onerror    -
        *//*-*****************************************************************************************************/
        DECL_STATIC_LINK( AddonsOptions, Notify, void* );

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*Attention

            Don't initialize these static member in these header!
            a) Double dfined symbols will be detected ...
            b) and unresolved externals exist at linking time.
            Do it in your source only.
         */

        static AddonsOptions_Impl*  m_pDataContainer    ;   /// impl. data container as dynamic pointer for smaller memory requirements!
        static sal_Int32            m_nRefCount         ;   /// internal ref count mechanism

};      // class SvtMenuOptions

}

#endif  // #ifndef __FRAMEWORK_CLASSES_ADDONSOPTIONS_HXX_
