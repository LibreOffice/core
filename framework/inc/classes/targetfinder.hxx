/*************************************************************************
 *
 *  $RCSfile: targetfinder.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: as $ $Date: 2001-03-09 14:42:23 $
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

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          We need some informations about our caller, With these enum he can specify his frame type ...
                    (Frame/Task/PlugInFrame/Desktop ...)
*//*-*************************************************************************************************************/

enum EFrameType
{
    E_DESKTOP       ,
    E_PLUGINFRAME   ,
    E_TASK          ,
    E_FRAME
};

/*-************************************************************************************************************//**
    @short          These values specify special target names which must handled.
*//*-*************************************************************************************************************/

#define SPECIALTARGET_BLANK                                     DECLARE_ASCII("_blank"      )
#define SPECIALTARGET_SELF                                      DECLARE_ASCII("_self"       )
#define SPECIALTARGET_PARENT                                    DECLARE_ASCII("_parent"     )
#define SPECIALTARGET_TOP                                       DECLARE_ASCII("_top"        )
#define SPECIALTARGET_BEAMER                                    DECLARE_ASCII("_beamer"     )
/* not supported yet!
#define SPECIALTARGET_DOCUMENT                                  DECLARE_ASCII("_document"   )
#define SPECIALTARGET_EXPLORER                                  DECLARE_ASCII("_explorer"   )
#define SPECIALTARGET_PARTWINDOW                                DECLARE_ASCII("_partwindow" )
*/

/*-************************************************************************************************************//**
    @short          valid result values to classify targeting
*//*-*************************************************************************************************************/

enum ETargetClass
{
    E_UNKNOWN       ,   /// occure if you call us without valid flag combinations!
    E_CREATETASK    ,   /// create new task (supported by desktop only!)
    E_SELF          ,   /// you are the target himself
    E_PARENT        ,   /// your parent is the target
    E_BEAMER        ,   /// an existing beamer is the target (create new one if it not already exist!)
    E_TASKS         ,   /// special (but exclusiv) search for tasks only (supported at desktop only - but can combined with CREATE!)
    E_FORWARD_UP    ,   /// forward call to your parent
    E_DEEP_DOWN     ,   /// search at your children (search children of direct children before another direcht children!)
    E_FLAT_DOWN     ,   /// search at your children (search at all direct children first;  children of direcht children then!)
    E_DEEP_BOTH     ,   /// combination of E_DEEP_DOWN and E_FORWARD_UP ( search down first!)
    E_FLAT_BOTH         /// combination of E_FLAT_DOWN and E_FORWARD_UP ( search down first!)
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
                        These method never create or return a tree node! Thats your job!
                        We say: go up, go down or give you the permission to create new frame if search will fail!

            @seealso    -

            @param      "eFrameType"        Give us your node type (desktop, task ,frame) Its neccessary to select right search algorithm.
            @param      "sTargetName"       This is the search parameter to find right frame by name or special value!
            @param      "nSearchFlags"      This value is an optional parameter to regulate search direction if no special target name was given.
            @param      "bCreationAllowed"  We set it TRUE if flag TASKS is set. You must search for given target, but could create a new tree node if search will fail!
            @param      "bChildrenExist"    Say us - if some children exist. Otherwise down search is ignored!
            @param      "bParentExist"      Say us - if a parent exist. Otherwise upper search is ignored!
            @param      "sFrameName"        If SELF flag is set we can break search earlier if this name is the target!
            @param      "sParentName"       If PARENT flag is set we can break search earlier if this name is the target!
            @return     An enum value to classify the direction for searching.

            @onerror    E_UNKNOWN is returned.
        *//*-*****************************************************************************************************/

        static ETargetClass classify(           EFrameType      eFrameType                          ,
                                        const   OUSTRING&       sTargetName                         ,
                                                   sal_Int32        nSearchFlags                        ,
                                                sal_Bool&       bCreationAllowed                    ,
                                                sal_Bool        bChildrenExist                      ,
                                        const   OUSTRING&       sFrameName          =   OUSTRING()  ,
                                                sal_Bool        bParentExist        =   sal_False   ,
                                        const   OUSTRING&       sParentName         =   OUSTRING()  );

        //---------------------------------------------------------------------------------------------------------
        //  private methods
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      helper methods for classify()
            @descr      Every tree node (desktop, frame, task ...) has another preference shares to search a target.
                        With these helper methods we differ between these search algorithm!

            @seealso    method classify()

            @param      "bParentExist"      set if a parent exist for caller tree node
            @param      "bChildrenExist"    set if some children exist for caller tree node
            @param      "sFrameName"        name of current tree node (used for SELF flag to break search earlier!)
            @param      "sParentName"       name of current tree node (used for PARENT flag to break search earlier!)
            @param      "sTargetName"       name of searched target tree node
            @param      "nSearchFlags"      flags to regulate search in tree
            @param      "bTopFrame"         used to break upper searches at a top frame if search outside current task isnt allowed!

            @return     A reference to an existing frame or null if search failed.

            @onerror    A null reference is returned.
        *//*-*****************************************************************************************************/

        static ETargetClass impl_classifyForDesktop     (           sal_Bool    bChildrenExist      ,
                                                            const   OUSTRING&   sTargetName         ,
                                                                       sal_Int32    nSearchFlags        );

        static ETargetClass impl_classifyForPlugInFrame (           sal_Bool    bParentExist        ,
                                                                    sal_Bool    bChildrenExist      ,
                                                            const   OUSTRING&   sFrameName          ,
                                                            const   OUSTRING&   sTargetName         ,
                                                                       sal_Int32    nSearchFlags        );

        static ETargetClass impl_classifyForTask        (           sal_Bool    bParentExist        ,
                                                                    sal_Bool    bChildrenExist      ,
                                                            const   OUSTRING&   sFrameName          ,
                                                            const   OUSTRING&   sTargetName         ,
                                                                       sal_Int32    nSearchFlags        );

        static ETargetClass impl_classifyForFrame       (           sal_Bool    bParentExist        ,
                                                                    sal_Bool    bChildrenExist      ,
                                                            const   OUSTRING&   sFrameName          ,
                                                            const   OUSTRING&   sParentName         ,
                                                            const   OUSTRING&   sTargetName         ,
                                                                       sal_Int32    nSearchFlags        );

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

        //*********************************************************************************************************
        // - check invalid references, misused booleans, wrong flags or for an unknown frame type
        // - value of bCreationAllowed will set by classify() - existing value isn't important
        // - empty strings are allowed
        static inline sal_Bool implcp_classify(         EFrameType  eFrameType          ,
                                                const   OUSTRING&   sTargetName         ,
                                                        sal_Int32   nSearchFlags        ,
                                                        sal_Bool&   bCreationAllowed    ,
                                                        sal_Bool    bChildrenExist      ,
                                                const   OUSTRING&   sFrameName          ,
                                                        sal_Bool    bParentExist        ,
                                                const   OUSTRING&   sParentName         )
        {
            return  (
                        ( &sTargetName      ==  NULL            )   ||
                        ( &sFrameName       ==  NULL            )   ||
                        ( &sParentName      ==  NULL            )   ||
                        ( &bCreationAllowed ==  NULL            )   ||
                        ( nSearchFlags      <   0               )   ||
                        (
                            ( bChildrenExist!=  sal_False       )   &&
                            ( bChildrenExist!=  sal_True        )
                        )                                           ||
                        (
                            ( bParentExist  !=  sal_False       )   &&
                            ( bParentExist  !=  sal_True        )
                        )                                           ||
                        (
                            ( eFrameType    !=  E_DESKTOP       )   &&
                            ( eFrameType    !=  E_PLUGINFRAME   )   &&
                            ( eFrameType    !=  E_TASK          )   &&
                            ( eFrameType    !=  E_FRAME         )
                        )
                    );
        }

        #endif  // #ifdef ENABLE_ASSERTIONS

};      //  class TargetFinder

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_CLASSES_TARGETFINDER_HXX_
