/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dynamicmenuoptions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:16:35 $
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
#ifndef INCLUDED_SVTOOLS_DYNAMICMENUOPTIONS_HXX
#define INCLUDED_SVTOOLS_DYNAMICMENUOPTIONS_HXX

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef INCLUDED_SVTOOLS_OPTIONS_HXX
#include <svtools/options.hxx>
#endif

//_________________________________________________________________________________________________________________
//  types, enums, ...
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @descr          The method GetList() returns a list of property values.
                    Use follow defines to seperate values by names.
*//*-*************************************************************************************************************/
#define DYNAMICMENU_PROPERTYNAME_URL                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URL"             ))
#define DYNAMICMENU_PROPERTYNAME_TITLE                  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Title"           ))
#define DYNAMICMENU_PROPERTYNAME_IMAGEIDENTIFIER        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ImageIdentifier" ))
#define DYNAMICMENU_PROPERTYNAME_TARGETNAME             ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TargetName"      ))

/*-************************************************************************************************************//**
    @descr          You can use these enum values to specify right menu if you call our interface methods.
*//*-*************************************************************************************************************/
enum EDynamicMenuType
{
    E_NEWMENU       =   0,
    E_WIZARDMENU    =   1,
    E_HELPBOOKMARKS =   2
};
//_________________________________________________________________________________________________________________
//  forward declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          forward declaration to our private date container implementation
    @descr          We use these class as internal member to support small memory requirements.
                    You can create the container if it is neccessary. The class which use these mechanism
                    is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/

class SvtDynamicMenuOptions_Impl;

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          collect informations about dynamic menus
    @descr          Make it possible to configure dynamic menu structures of menus like "new" or "wizard".

    @implements     -
    @base           -

    @devstatus      ready to use
*//*-*************************************************************************************************************/

class SVL_DLLPUBLIC SvtDynamicMenuOptions: public svt::detail::Options
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

         SvtDynamicMenuOptions();
        virtual ~SvtDynamicMenuOptions();

        //---------------------------------------------------------------------------------------------------------
        //  interface
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      clear complete sepcified list
            @descr      Call this methods to clear the whole list.
                        To fill it again use AppendItem().

            @seealso    -

            @param      "eMenu" select right menu to clear.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void Clear( EDynamicMenuType eMenu );

        /*-****************************************************************************************************//**
            @short      return complete specified list
            @descr      Call it to get all entries of an dynamic menu.
                        We return a list of all nodes with his names and properties.

            @seealso    -

            @param      "eMenu" select right menu.
            @return     A list of menu items is returned.

            @onerror    We return an empty list.
        *//*-*****************************************************************************************************/

        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > GetMenu( EDynamicMenuType eMenu ) const;

        /*-****************************************************************************************************//**
            @short      append a new item to specified menu
            @descr      You can append items to a menu only - removing isn't allowed for a special item!
                        We support a nothing or all mechanism only! Clear all or append something ...

            @seealso    method Clear()

            @param      "eMenu"             select right menu.
            @param      "sURL"              URL for dispatch
            @param      "sTitle"            label of menu entry
            @param      "sImageIdentifier"  icon identifier
            @param      "sTargetName"       target for dispatch
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void AppendItem(            EDynamicMenuType    eMenu            ,
                            const   ::rtl::OUString&    sURL             ,
                            const   ::rtl::OUString&    sTitle           ,
                            const   ::rtl::OUString&    sImageIdentifier ,
                            const   ::rtl::OUString&    sTargetName      );

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

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

        SVL_DLLPRIVATE static ::osl::Mutex& GetOwnStaticMutex();

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

        static SvtDynamicMenuOptions_Impl* m_pDataContainer    ;   /// impl. data container as dynamic pointer for smaller memory requirements!
        static sal_Int32             m_nRefCount         ;   /// internal ref count mechanism

};      // class SvtDynamicMenuOptions

#endif  // #ifndef INCLUDED_SVTOOLS_DYNAMICMENUOPTIONS_HXX
