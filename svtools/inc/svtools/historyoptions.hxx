/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: historyoptions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:21:01 $
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
#ifndef INCLUDED_SVTOOLS_HISTORYOPTIONS_HXX
#define INCLUDED_SVTOOLS_HISTORYOPTIONS_HXX

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

#define HISTORY_PROPERTYNAME_URL            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URL"       ))
#define HISTORY_PROPERTYNAME_FILTER         ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Filter"    ))
#define HISTORY_PROPERTYNAME_TITLE          ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Title"     ))
#define HISTORY_PROPERTYNAME_PASSWORD       ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Password"  ))

/*-************************************************************************************************************//**
    @descr          You can use these enum values to specify right history if you call ouer interface methods.
*//*-*************************************************************************************************************/

enum EHistoryType
{
    ePICKLIST       = 0,
    eHISTORY        = 1,
    eHELPBOOKMARKS  = 2
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

class SvtHistoryOptions_Impl;

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          collect informations about history features
    @descr          -

    @implements     -
    @base           -

    @devstatus      ready to use
*//*-*************************************************************************************************************/

class SVL_DLLPUBLIC SvtHistoryOptions: public svt::detail::Options
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

         SvtHistoryOptions();
        virtual ~SvtHistoryOptions();

        //---------------------------------------------------------------------------------------------------------
        //  interface
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      interface methods to get and set value of config key "org.openoffice.Office.Common/History/..."
            @descr      key "PickList"  : The last used documents displayed in the file menu.
                        key "History"   : The last opened documents general.
        *//*-*****************************************************************************************************/

        /*-****************************************************************************************************//**
            @short      set/get max size of specified history
            @descr      Call this methods to get information about max. size of specified list.
                        These value lay down the max count of items in these history. If a new one
                        is add to it the oldest one is deleted automaticly!

            @seealso    -

            @param      "eHistory" select right history.
            @param      "nSize" is the new max size of specified list. If new size smaller then the old one
                        some oldest entries will be destroyed automaticly!
            @return     Current max size of specified list.

            @onerror    -
        *//*-*****************************************************************************************************/

        sal_uInt32  GetSize( EHistoryType eHistory                      ) const ;
        void        SetSize( EHistoryType eHistory, sal_uInt32 nSize    )       ;

        /*-****************************************************************************************************//**
            @short      clear complete sepcified list
            @descr      Call this methods to clear the whole list. After that GetItemCount() will return 0 ...
                        but GetSize() will return the old value!

            @seealso    -

            @param      "eHistory" select right history.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void Clear( EHistoryType eHistory );

        /*-****************************************************************************************************//**
            @short      return complete sepcified list
            @descr      If you will show the whole list call this method to get it completly.

            @seealso    -

            @param      "eHistory" select right history.
            @return     A list of history items is returned.

            @onerror    -
        *//*-*****************************************************************************************************/

        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > GetList( EHistoryType eHistory ) const ;

        /*-****************************************************************************************************//**
            @short      append a new item to specified list
            @descr      You can append items to a list only - removing isn't allowed for a special item!
                        The oldest entry is deleted automaticly if max size arrived or you can call Clear() ...
                        It exist two different overload methods to do this.
                        One for user which have an complete history item and another one for uncompletly data sets!

            @seealso    method SetSize()
            @seealso    method Clear()

            @param      "eHistory" select right history.
            @param      "sURL" URL to save in history
            @param      "sFilter" filter name to save in history
            @param      "sTitle" document title to save in history
            @param      "sPassword" password to save in history
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void AppendItem(            EHistoryType        eHistory    ,
                            const   ::rtl::OUString&    sURL        ,
                            const   ::rtl::OUString&    sFilter     ,
                            const   ::rtl::OUString&    sTitle      ,
                            const   ::rtl::OUString&    sPassword   );

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      return a reference to a static mutex
            @descr      Make these class threadsafe.

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

        static SvtHistoryOptions_Impl*  m_pDataContainer    ;   /// impl. data container as dynamic pointer for smaller memory requirements!
        static sal_Int32                m_nRefCount         ;   /// internal ref count mechanism

};      // class SvtHistoryOptions

#endif  // #ifndef INCLUDED_SVTOOLS_HISTORYOPTIONS_HXX
