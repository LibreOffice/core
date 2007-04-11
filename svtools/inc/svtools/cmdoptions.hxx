/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cmdoptions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:13:30 $
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
#ifndef INCLUDED_SVTOOLS_CMDOPTIONS_HXX
#define INCLUDED_SVTOOLS_CMDOPTIONS_HXX

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

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
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
#define CMDOPTIONS_PROPERTYNAME_URL                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "CommandURL" ))

//_________________________________________________________________________________________________________________
//  forward declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          forward declaration to our private date container implementation
    @descr          We use these class as internal member to support small memory requirements.
                    You can create the container if it is neccessary. The class which use these mechanism
                    is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/

class SvtCommandOptions_Impl;

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

class SVL_DLLPUBLIC SvtCommandOptions: public svt::detail::Options
{
    friend class SvtCommandOptions_Impl;

    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        enum CmdOption
        {
            CMDOPTION_DISABLED,
            CMDOPTION_NONE
        };

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

         SvtCommandOptions();
        virtual ~SvtCommandOptions();

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

        void Clear( CmdOption eOption );

        /*-****************************************************************************************************//**
            @short      return complete specified list
            @descr      Call it to get all entries of an dynamic menu.
                        We return a list of all nodes with his names and properties.

            @seealso    -

            @param      "eOption" select the list to retrieve.
            @return     A list of command strings is returned.

            @onerror    We return an empty list.
        *//*-*****************************************************************************************************/

        sal_Bool HasEntries( CmdOption eOption ) const;

        /*-****************************************************************************************************//**
            @short      Lookup if a command URL is inside a given list
            @descr      Lookup if a command URL is inside a given lst

            @seealso    -

            @param      "eOption" select right command list
            @param      "aCommandURL" a command URL that is used for the look up
            @return     "sal_True" if the command is inside the list otherwise "sal_False"

            @onerror    -
        *//*-*****************************************************************************************************/

        sal_Bool Lookup( CmdOption eOption, const ::rtl::OUString& aCommandURL ) const;

        /*-****************************************************************************************************//**
            @short      return complete specified list
            @descr      Call it to get all entries of an dynamic menu.
                        We return a list of all nodes with his names and properties.

            @seealso    -

            @param      "eOption" select the list to retrieve.
            @return     A list of command strings is returned.

            @onerror    We return an empty list.
        *//*-*****************************************************************************************************/

        ::com::sun::star::uno::Sequence< ::rtl::OUString > GetList( CmdOption eOption ) const;

        /*-****************************************************************************************************//**
            @short      adds a new command to specified options list
            @descr      You can add a command to specified options list!

            @seealso    method Clear()

            @param      "eOption"           specifies the command list
            @param      "sURL"              URL for dispatch
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void AddCommand( CmdOption eOption, const ::rtl::OUString& sURL );

        /*-****************************************************************************************************//**
            @short      register an office frame, which must update its dispatches if
                        the underlying configuration was changed.

            @descr      To avoid using of "dead" frame objects or implementing
                        deregistration mechanism too, we use weak references to
                        the given frames.

            @param      "xFrame"            points to the frame, which wish to be
                                            notified, if configuration was changed.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void EstablisFrameCallback(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame);

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

        static SvtCommandOptions_Impl*  m_pDataContainer    ;   /// impl. data container as dynamic pointer for smaller memory requirements!
        static sal_Int32                m_nRefCount         ;   /// internal ref count mechanism

};      // class SvtCmdOptions

#endif  // #ifndef INCLUDED_SVTOOLS_CMDOPTIONS_HXX
