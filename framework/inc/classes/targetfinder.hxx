/*************************************************************************
 *
 *  $RCSfile: targetfinder.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2000-10-23 13:55:34 $
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

#ifndef __FRAMEWORK_CLASSES_TARGETFINDER_HXX_
#define __FRAMEWORK_CLASSES_TARGETFINDER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAMES_HPP_
#include <com/sun/star/frame/XFrames.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

#define OUSTRING            ::rtl::OUString
#define REFERENCE           ::com::sun::star::uno::Reference
#define XFRAME              ::com::sun::star::frame::XFrame
#define XFRAMES             ::com::sun::star::frame::XFrames

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          We need some informations about our caller, With these enum he can specify his frame type ...
                    (Frame/Task/PlugInFrame/Desktop ...)
*//*-*************************************************************************************************************/

enum IMPL_EFrameType
{
    eFRAME          ,
    eTASK           ,
    ePLUGINFRAME    ,
    eDESKTOP
};

/*-************************************************************************************************************//**
    @short          These values specify special target names which must handled.
*//*-*************************************************************************************************************/

#define SPECIALTARGET_BLANK                                     DECLARE_ASCII("_blank"      )
#define SPECIALTARGET_SELF                                      DECLARE_ASCII("_self"       )
#define SPECIALTARGET_PARENT                                    DECLARE_ASCII("_parent"     )
#define SPECIALTARGET_TOP                                       DECLARE_ASCII("_top"        )
/* not supported in moment!
#define SPECIALTARGET_DOCUMENT                                  DECLARE_ASCII("_document"   )
#define SPECIALTARGET_BEAMER                                    DECLARE_ASCII("_beamer"     )
#define SPECIALTARGET_EXPLORER                                  DECLARE_ASCII("_explorer"   )
#define SPECIALTARGET_PARTWINDOW                                DECLARE_ASCII("_partwindow" )
*/

/*-************************************************************************************************************//**
    @short          valid result values to classify targeting
*//*-*************************************************************************************************************/

enum IMPL_ETargetClass
{
    eUNKNOWN    ,   /// given parameter are invalid - there is no chance to find these target!
    eCREATE     ,   /// a new target must create
    eSELF       ,   /// you are the target himself
    ePARENT     ,   /// your direct parent is the target!
    eUP         ,   /// search target at parents only
    eDOWN       ,   /// search target at childrens only
    eSIBLINGS   ,   /// search target at parents and his childrens ... but not at your children!
    eALL            /// react first for eCHILDRENS and then for eSIBLINGS! (protect your code against recursive calls from bottom or top!)
};

/*-************************************************************************************************************//**
    @short          implement helper to implement code for targeting only one time!
    @descr          We need checking of a target name in combination with given search flags at differnt places.
                    These helper analyze the parameter and recommend the direction for searching.
                    We can use a method to classify search direction and another one to change our search parameter
                    to make it right. You can use these - but you don't must do it!

    @implements     -
    @base           -

    @ATTENTION      This class is'nt threadsafe!

    @devstatus      ready to use
*//*-*************************************************************************************************************/

class TargetFinder
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      standard ctor/dtor
            @descr      We do nothing here.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

                 TargetFinder();
        virtual ~TargetFinder();

        //---------------------------------------------------------------------------------------------------------
        //  interface
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      get a recommendation for searching right target
            @descr      Our caller search for a target which match with given parameter.
                        Give him a direction to find the right one.

            @seealso    -

            @param      "xOwner";           We need a reference to our caller to get some special informations about his environment
            @param      "sTargetName";      This is the search parameter to find right frame by name or special value!
            @param      "nSearchFlags";     These are optional parameter to regulate search direction.
            @return     An enum value to classify the direction for searching.

            @onerror    eUNKNOWN is returned.
        *//*-*****************************************************************************************************/

        static IMPL_ETargetClass classify(  const   REFERENCE< XFRAME >&    xOwner          ,
                                            const   OUSTRING&               sTargetName     ,
                                                       sal_Int32                nSearchFlags    );

        /*-****************************************************************************************************//**
            @short      implement default search at children ...
            @descr      You CAN use these implementation or write your own code!
                        With these method we support a search for a target at your children.
                        We search direct children first and subframes of these direct one then.

            @ATTENTION  We don't accept inpossible calling parameter - like special target names!
                        We search for realy named targets only.

            @seealso    -

            @param      "xChildFrameAccess";    Access to container with child frames of our caller
            @param      "sTargetName";          This is the search parameter to find right frame by name or special value!
            @return     A reference to an existing frame or null if search failed.

            @onerror    A null reference is returned.
        *//*-*****************************************************************************************************/

        static REFERENCE< XFRAME > helpDownSearch(  const   REFERENCE< XFRAMES >&   xChildFrameAccess   ,
                                                    const   OUSTRING&               sTargetName         );

        //---------------------------------------------------------------------------------------------------------
        //  debug and test methods
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      debug-methods to check incoming parameter of some other mehods of this class
            @descr      The follow methods are used to check parameters for other methods
                        of this class. The return value is used directly for an ASSERT(...).
                        This mechanism is active in debug version only!

            @seealso    FRAMEWORK_ASSERT in implementation!

            @param      references to checking variables
            @return     sal_False on invalid parameter
            @return     sal_True otherwise

            @onerror    -
        *//*-*****************************************************************************************************/

        #ifdef ENABLE_ASSERTIONS

        private:

            static sal_Bool impldbg_checkParameter_classify         (   const   REFERENCE< XFRAME >&    xOwner              ,
                                                                        const   OUSTRING&               sTargetName         ,
                                                                                   sal_Int32                nSearchFlags        );
            static sal_Bool impldbg_checkParameter_helpDownSearch   (   const   REFERENCE< XFRAMES >&   xChildFrameAccess   ,
                                                                        const   OUSTRING&               sTargetName         );

        #endif  // #ifdef ENABLE_ASSERTIONS

};      //  class TargetFinder

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_CLASSES_TARGETFINDER_HXX_
