/*************************************************************************
 *
 *  $RCSfile: targetfinder.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: as $ $Date: 2001-03-09 14:42:25 $
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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_TARGETFINDER_HXX_
#include <classes/targetfinder.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

using namespace ::rtl                       ;
using namespace ::com::sun::star::uno       ;
using namespace ::com::sun::star::frame     ;

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  non exported declarations
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
TargetFinder::TargetFinder()
{
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
TargetFinder::~TargetFinder()
{
}

//*****************************************************************************************************************
//  interface
//*****************************************************************************************************************
ETargetClass TargetFinder::classify(            EFrameType      eFrameType          ,
                                        const   OUString&       sTargetName         ,
                                                   sal_Int32        nSearchFlags        ,
                                                sal_Bool&       bCreationAllowed    ,
                                                sal_Bool        bChildrenExist      ,
                                        const   OUString&       sFrameName          ,
                                                sal_Bool        bParentExist        ,
                                        const   OUString&       sParentName         )
{
    // Check incoming parameter
    LOG_ASSERT2( implcp_classify( eFrameType, sTargetName, nSearchFlags, bCreationAllowed, bChildrenExist, sFrameName, bParentExist, sParentName ), "TargetFinder::classify()", "Invalid parameter detected!" )

    // Initialize start values.
    ETargetClass    eResult             =   E_UNKNOWN                                                               ;   // default result of method
                    bCreationAllowed    =   (( nSearchFlags & FrameSearchFlag::CREATE ) == FrameSearchFlag::CREATE );   // if search failed we must caller allow to create new task/frame

    // Use some helper methods for different classes of tree nodes to get the result.
    switch( eFrameType )
    {
        case E_DESKTOP      :   eResult = impl_classifyForDesktop( bChildrenExist, sTargetName, nSearchFlags );
                                break;
        case E_PLUGINFRAME  :   eResult = impl_classifyForPlugInFrame( bParentExist, bChildrenExist, sFrameName, sTargetName, nSearchFlags );
                                break;
        case E_TASK         :   eResult = impl_classifyForTask( bParentExist, bChildrenExist, sFrameName, sTargetName, nSearchFlags );
                                break;
        case E_FRAME        :   eResult = impl_classifyForFrame( bParentExist, bChildrenExist, sFrameName, sParentName, sTargetName, nSearchFlags );
                                break;
    }

    // It doesnt matter if CREATE flag is set or not ...
    // If follow results are returned by our helper methods - the result will be clear!
    // In these cases we dont can allow (or must!) creation of new frames/tasks...
    if  (
            (   eResult ==  E_UNKNOWN       )   ||
            (   eResult ==  E_CREATETASK    )   ||
            (   eResult ==  E_SELF          )   ||
            (   eResult ==  E_PARENT        )   ||
            (   eResult ==  E_BEAMER        )
        )
    {
        bCreationAllowed = sal_False;
    }

    return eResult;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
ETargetClass TargetFinder::impl_classifyForDesktop(         sal_Bool    bChildrenExist  ,
                                                    const   OUSTRING&   sTargetName     ,
                                                            sal_Int32   nSearchFlags    )
{
    ETargetClass eResult = E_UNKNOWN;

    //*************************************************************************************************************
    //  I)      Handle special target names.
    //          Make an exclusiv search: if() else if() ...
    //
    //  I.I)    Look for "_blank"
    //          Only the desktop can create new tasks.
    //*************************************************************************************************************
    if( sTargetName == SPECIALTARGET_BLANK )
    {
        eResult = E_CREATETASK;
    }
    else
    {
        //*********************************************************************************************************
        //  II)     Special target names was handled ...
        //          Now look for right flags.
        //          Combine search results: if(); if() ...
        //
        //  II.I)   Special and exclusiv mode for search at our desktop!
        //          Normaly TASKS flag is used to restrict upper searches inside current task tree!
        //          All searches stop at a top frame if these flag isnt set.
        //          For down search it doesnt matter ...
        //          but I think we can use it to search at all direct(!) childrens of our desktop.
        //          These can be useful to create new tasks by name if it not already exist.
        //          =>  These flag cant combinde with CHILDREN or SIBLINGS or somethings else.
        //              We ignore such constructs. If you combine it with the CREATE flag - a new task will created
        //              if no existing one can be found.
        //*********************************************************************************************************
        if  (
                ( nSearchFlags & FrameSearchFlag::TASKS )   &&
                (
                    !( nSearchFlags & FrameSearchFlag::CHILDREN )   &&
                    !( nSearchFlags & FrameSearchFlag::SIBLINGS )   &&
                    !( nSearchFlags & FrameSearchFlag::PARENT   )   &&
                    !( nSearchFlags & FrameSearchFlag::SELF     )
                )
            )
        {
            eResult = E_TASKS;
        }
        else
        {
            //*****************************************************************************************************
            //  II.I)   Look for CHILDREN.
            //          Ignore flag if no childrens exist!
            //*****************************************************************************************************
            if  (
                    ( nSearchFlags      &   FrameSearchFlag::CHILDREN   )   &&
                    ( bChildrenExist    ==  sal_True                    )
                )
            {
                eResult = E_DEEP_DOWN;
            }

            //*****************************************************************************************************
            //  II.II)  Look for SIBLINGS.
            //          These change a deep to a flat search!
            //          Otherwise ... flag can be ignored - because the desktop has no siblings!
            //*****************************************************************************************************
            if( nSearchFlags & FrameSearchFlag::SIBLINGS )
            {
                switch( eResult )
                {
                    case E_DEEP_DOWN    :   eResult = E_FLAT_DOWN;
                                            break;
                }
            }
        }
    }

    //*************************************************************************************************************
    //  possible results:
    //      E_UNKNOWN
    //      E_CREATETASK
    //      E_TASKS
    //      E_DEEP_DOWN
    //      E_FLAT_DOWN
    //*************************************************************************************************************
    return eResult;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
ETargetClass TargetFinder::impl_classifyForPlugInFrame  (           sal_Bool    bParentExist    ,
                                                                    sal_Bool    bChildrenExist  ,
                                                            const   OUString&   sFrameName      ,
                                                            const   OUString&   sTargetName     ,
                                                                       sal_Int32    nSearchFlags    )
{
    // At the moment a PlugInFrame is a special task ... but we can use the same search algorithm!
    return impl_classifyForTask( bParentExist, bChildrenExist, sFrameName, sTargetName, nSearchFlags );
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
ETargetClass TargetFinder::impl_classifyForTask(            sal_Bool    bParentExist    ,
                                                            sal_Bool    bChildrenExist  ,
                                                    const   OUString&   sFrameName      ,
                                                    const   OUString&   sTargetName     ,
                                                               sal_Int32    nSearchFlags    )
{
    ETargetClass    eResult     =   E_UNKNOWN                                                               ;
    sal_Bool        bLeaveTask  =   (( nSearchFlags & FrameSearchFlag::TASKS  ) == FrameSearchFlag::TASKS  );   // we must know if we can search outside current task

    //*************************************************************************************************************
    //  I)      Handle special target names.
    //          Make an exclusiv search: if() else if() ...
    //
    //  I.I)    Look for "_blank"
    //          Only the desktop can create new tasks. Forward search to parent!
    //*************************************************************************************************************
    if( sTargetName == SPECIALTARGET_BLANK )
    {
        if( bParentExist == sal_True )
        {
            eResult = E_FORWARD_UP;
        }
    }
    else

    //*************************************************************************************************************
    //  I.II)   Look for "_self"
    //          Handle "" in the same way!
    //*************************************************************************************************************
    if  (
            (   sTargetName             ==  SPECIALTARGET_SELF  )   ||
            (   sTargetName.getLength() <   1                   )
        )
    {
        eResult = E_SELF;
    }
    else

    //*************************************************************************************************************
    //  I.III)  Look for "_top"
    //          A task is top everytime!
    //*************************************************************************************************************
    if( sTargetName == SPECIALTARGET_TOP )
    {
        eResult = E_SELF;
    }
    else

    //*************************************************************************************************************
    //  I.IV)   Look for "_beamer"
    //*************************************************************************************************************
    if( sTargetName == SPECIALTARGET_BEAMER )
    {
        eResult = E_BEAMER;
    }
    else

    {
        //*********************************************************************************************************
        //  II)     Special target names was handled ...
        //          Now look for right flags.
        //          Combine search results: if(); if() ...
        //
        //  II.I)   Look for SELF.
        //          Use given frame name to do that. It couldn't be empty(!) - because this was handled in step I.II).
        //*********************************************************************************************************
        if  (
                ( nSearchFlags  &   FrameSearchFlag::SELF   )   &&
                ( sTargetName   ==  sFrameName              )
            )
        {
            eResult = E_SELF;
        }

        //*********************************************************************************************************
        //  II.II)  Look for PARENT.
        //          Is allowed on tasks if outside search of it is allowed!
        //          Don't check name of parent here - otherwise we return the desktop as result ...
        //*********************************************************************************************************
        if  (
                ( eResult       !=  E_SELF                  )   &&
                ( nSearchFlags  &   FrameSearchFlag::PARENT )   &&
                ( bParentExist  ==  sal_True                )   &&
                ( bLeaveTask    ==  sal_True                )
            )
        {
            eResult = E_FORWARD_UP;
        }

        //*********************************************************************************************************
        //  II.II)  Look for CHILDREN.
        //          Ignore flag if no childrens exist!
        //*********************************************************************************************************
        if  (
                ( eResult           !=  E_SELF                      )   &&
                ( nSearchFlags      &   FrameSearchFlag::CHILDREN   )   &&
                ( bChildrenExist    ==  sal_True                    )
            )
        {
            switch( eResult )
            {
                case E_UNKNOWN      :   eResult = E_DEEP_DOWN;
                                        break;
                case E_FORWARD_UP   :   eResult = E_DEEP_BOTH;
                                        break;
            }
        }

        //*********************************************************************************************************
        //  II.III) Look for SIBLINGS.
        //          These change a deep to a flat search!
        //*********************************************************************************************************
        if  (
                ( eResult       !=  E_SELF                      )   &&
                   ( nSearchFlags   &   FrameSearchFlag::SIBLINGS   )
            )
        {
            switch( eResult )
            {
                case E_DEEP_DOWN    :   eResult = E_FLAT_DOWN;
                                        break;
                case E_DEEP_BOTH    :   eResult = E_FLAT_BOTH;
                                        break;
            }
        }
    }

    //*************************************************************************************************************
    //  possible results:
    //      E_UNKNOWN
    //      E_SELF
    //      E_BEAMER
    //      E_FORWARD_UP
    //      E_DEEP_DOWN
    //      E_DEEP_BOTH
    //      E_FLAT_DOWN
    //      E_FLAT_BOTH
    //*************************************************************************************************************
    return eResult;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
ETargetClass TargetFinder::impl_classifyForFrame(           sal_Bool    bParentExist    ,
                                                            sal_Bool    bChildrenExist  ,
                                                    const   OUString&   sFrameName      ,
                                                    const   OUString&   sParentName     ,
                                                    const   OUString&   sTargetName     ,
                                                            sal_Int32   nSearchFlags    )
{
    ETargetClass eResult = E_UNKNOWN;

    //*************************************************************************************************************
    //  I)      Handle special target names.
    //          Make an exclusiv search: if() else if() ...
    //
    //  I.I)    Look for "_blank"
    //          Only the desktop can create new tasks. Forward search to parent!
    //*************************************************************************************************************
    if( sTargetName == SPECIALTARGET_BLANK )
    {
        if( bParentExist == sal_True )
        {
            eResult = E_FORWARD_UP;
        }
    }
    else

    //*************************************************************************************************************
    //  I.II)   Look for "_self"
    //          Handle "" in the same way!
    //*************************************************************************************************************
    if  (
            (   sTargetName             ==  SPECIALTARGET_SELF  )   ||
            (   sTargetName.getLength() <   1                   )
        )
    {
        eResult = E_SELF;
    }
    else

    //*************************************************************************************************************
    //  I.III)  Look for "_top"
    //          A frame without a parent is top - otherwhise it's one of his parents!
    //*************************************************************************************************************
    if( sTargetName == SPECIALTARGET_TOP )
    {
        if( bParentExist == sal_False )
        {
            eResult = E_SELF;
        }
        else
        {
            eResult = E_FORWARD_UP;
        }
    }
    else

    //*************************************************************************************************************
    //  I.IV)   Look for "_parent"
    //          Ignore it if no parent exist!
    //*************************************************************************************************************
    if( sTargetName == SPECIALTARGET_PARENT )
    {
        if( bParentExist == sal_True )
        {
            eResult = E_PARENT;
        }
    }
    else

    //*************************************************************************************************************
    //  I.V)    Look for "_beamer"
    //          Only a task can handle or create the beamer!
    //*************************************************************************************************************
    if  (
            ( sTargetName   ==  SPECIALTARGET_BEAMER    )   &&
            ( bParentExist  ==  sal_True                )
        )
    {
        eResult = E_FORWARD_UP;
    }
    else

    {
        //*********************************************************************************************************
        //  II)     Special target names was handled ...
        //          Now look for right flags.
        //          Combine search results: if(); if() ...
        //
        //  II.I)   Look for SELF.
        //          Use given frame name to do that. It couldn't be empty(!) - because this was handled in step I.II).
        //*********************************************************************************************************
        if  (
                ( nSearchFlags  &   FrameSearchFlag::SELF   )   &&
                ( sTargetName   ==  sFrameName              )
            )
        {
            eResult = E_SELF;
        }

        //*********************************************************************************************************
        //  II.II)  Look for PARENT.
        //          Ignore flag if no parent exist! Check his name here to break search erlier!
        //          Ignore flag if we are a top frame and search outside current task isnt allowed.
        //*********************************************************************************************************
        if  (
                ( eResult       !=  E_SELF                  )   &&
                ( nSearchFlags  &   FrameSearchFlag::PARENT )   &&
                ( bParentExist  ==  sal_True                )
            )
        {
            if( sParentName == sTargetName )
            {
                eResult = E_PARENT;
            }
            else
            {
                eResult = E_FORWARD_UP;
            }
        }

        //*********************************************************************************************************
        //  II.III) Look for CHILDREN.
        //          Ignore flag if no childrens exist! Combine it with already set decisions!
        //*********************************************************************************************************
        if  (
                ( eResult           !=  E_SELF                      )   &&
                ( eResult           !=  E_PARENT                    )   &&
                ( nSearchFlags      &   FrameSearchFlag::CHILDREN   )   &&
                ( bChildrenExist    ==  sal_True                    )
            )
        {
            switch( eResult )
            {
                case E_UNKNOWN      :   eResult = E_DEEP_DOWN;
                                           break;
                case E_FORWARD_UP   :   eResult = E_DEEP_BOTH;
                                         break;
            }
        }

        //*********************************************************************************************************
        //  II.IV)  Look for SIBLINGS.
        //          These change a deep to a flat search!
        //*********************************************************************************************************
        if  (
                ( eResult       !=  E_SELF                      )   &&
                ( eResult       !=  E_PARENT                    )   &&
                   ( nSearchFlags   &   FrameSearchFlag::SIBLINGS   )
            )
        {
            switch( eResult )
            {
                case E_DEEP_DOWN    :   eResult = E_FLAT_DOWN;
                                        break;
                case E_DEEP_BOTH    :   eResult = E_FLAT_BOTH;
                                        break;
            }
        }
    }

    //*************************************************************************************************************
    //  possible results:
    //      E_UNKNOWN
    //      E_SELF
    //      E_PARENT
    //      E_FORWARD_UP
    //      E_DEEP_DOWN
    //      E_DEEP_BOTH
    //      E_FLAT_DOWN
    //      E_FLAT_BOTH
    //*************************************************************************************************************
    return eResult;
}

}       //  namespace framework
