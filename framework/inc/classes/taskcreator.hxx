/*************************************************************************
 *
 *  $RCSfile: taskcreator.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: as $ $Date: 2001-08-10 11:54:21 $
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

#ifndef __FRAMEWORK_CLASSES_TASKCREATOR_HXX_
#define __FRAMEWORK_CLASSES_TASKCREATOR_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_TARGETFINDER_HXX_
#include <classes/targetfinder.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

struct TaskInfo
{
    //-------------------------------------------------------------------------------------------------------------
    public:
        //---------------------------------------------------------------------------------------------------------
        // Create new info structure and fill it with valid values.
        inline TaskInfo( const css::uno::Reference< css::lang::XMultiServiceFactory >& xNewFactory ,
                         const css::uno::Reference< css::frame::XFrame >&              xNewParent  ,
                         const ::rtl::OUString&                                        sNewName    ,
                               sal_Bool                                                bNewVisible )
        {
            xFactory    = xNewFactory                                                                          ;
            xParent     = css::uno::Reference< css::frame::XFramesSupplier >( xNewParent, css::uno::UNO_QUERY );
            sTaskName   = impl_filterNames( sNewName )                                                         ;
            bVisible    = bNewVisible                                                                          ;
        }

        //---------------------------------------------------------------------------------------------------------
        // Don't forget to release references and memory!
        inline ~TaskInfo()
        {
            xFactory    = css::uno::Reference< css::lang::XMultiServiceFactory >();
            xParent     = css::uno::Reference< css::frame::XFramesSupplier >()    ;
            sTaskName   = ::rtl::OUString()                                       ;
            bVisible    = sal_False                                               ;
        }

    private:
        //---------------------------------------------------------------------------------------------------------
        // Filter special names which can't be a valid frame name!
        // Attention: "_beamer" is a valid name - because:
        //  It exist one beamer for one task tree only.
        //  If he exist, we can find it - otherwhise he will be created by our task-frame!
        inline ::rtl::OUString impl_filterNames( const ::rtl::OUString& sName )
        {
            ::rtl::OUString sFiltered( sName );
            if(
                ( sName == SPECIALTARGET_BLANK      )   ||
                ( sName == SPECIALTARGET_SELF       )   ||
                ( sName == SPECIALTARGET_PARENT     )   ||
                ( sName == SPECIALTARGET_TOP        )   ||
                ( sName == SPECIALTARGET_MENUBAR    )   ||
                ( sName == SPECIALTARGET_HELPAGENT  )
              )
            {
                sFiltered = ::rtl::OUString();
            }
            return sFiltered;
        }

    //-------------------------------------------------------------------------------------------------------------
    public:
        css::uno::Reference< css::lang::XMultiServiceFactory >  xFactory    ;
        css::uno::Reference< css::frame::XFramesSupplier >      xParent     ;
        ::rtl::OUString                                         sTaskName   ;
        sal_Bool                                                bVisible    ;
};

/*-************************************************************************************************************//**
    @short          a helper to create new tasks or plugin frames for "_blank" or FrameSearchFlag::CREATE at desktop
    @descr          There are different places to create new tasks/plugin frames. Its not easy to service this code!
                    Thats the reason for this helper. He capsulate asynchronous/synchronous creation by calling
                    a simple interface.

    @implements     -
    @base           -

    @devstatus      ready to use
    @threadsafe     no
*//*-*************************************************************************************************************/
class TaskCreator
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------
                 TaskCreator() {};
        virtual ~TaskCreator() {};

        static css::uno::Reference< css::frame::XFrame > createSystemTask ( const TaskInfo& aInfo );
        static css::uno::Reference< css::frame::XFrame > createBrowserTask( const TaskInfo& aInfo );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------
    protected:

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------
    private:

    //-------------------------------------------------------------------------------------------------------------
    //  debug methods
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------
    #ifdef ENABLE_ASSERTIONS
    private:
        static sal_Bool implcp_createSystemTask ( const TaskInfo& aInfo );
        static sal_Bool implcp_createBrowserTask( const TaskInfo& aInfo );
    #endif  //  #ifdef ENABLE_ASSERTIONS

    //-------------------------------------------------------------------------------------------------------------
    //  private variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------
    private:

};      //  class TaskCreator

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_CLASSES_TASKCREATOR_HXX_
