/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: targetfinder.cxx,v $
 * $Revision: 1.13 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <classes/targetfinder.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XFrame.hpp>

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

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  non exported declarations
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short      create new target info with valid values
    @descr      If you wish to call TargetFinder::classify...() methods you must give him
                a lot of informations about currently set environment of a frame.
                But ... sometimes you have all informations ... and sometimes you need a helper
                to get it. These ctor implementations do both things for you.
                You should call it with ALL or with NOTHING.
                He use your values (at ALL) or try to get this informations by himself (at NOTHING).
                After that you can use this filled target info to call you classify...().

    @seealso    method TargetFinder::classify...()

    @param      "xFrame"    , reference to frame which should be detected  [MUST    ]
    @param      "sTarget"   , name of searched target                      [MUST    ]
    @param      "nFlags"    , search flags                                 [MUST    ]
    @param      "eType"     , type of frame                                [OPTIONAL]
    @param      "bChildrens", Is children search possible?                 [OPTIONAL]
    @param      "bParent"   , Is parent search possible?                   [OPTIONAL]
    @param      "sFrame"    , name of frame for self-search                [OPTIONAL]
    @param      "sParent"   , parent name for parent-search                [OPTIONAL]
    @return     -

    @onerror    No error should occure ... if incoming parameters are correct :-)
    @threadsafe No
*//*-*************************************************************************************************************/
TargetInfo::TargetInfo( const ::rtl::OUString&                           sTarget     ,
                              sal_Int32                                  nFlags      ,
                              EFrameType                                 eType       ,
                              sal_Bool                                   bChildrens  ,
                              sal_Bool                                   bParent     ,
                        const ::rtl::OUString&                           sFrame      ,
                        const ::rtl::OUString&                           sParent     )
{
    // Check incoming parameter.
    LOG_ASSERT2( implcp_ctor( sTarget, nFlags, eType, bChildrens, bParent, sFrame, sParent ), "TargetInfo::TargetInfo( 2 )", "Invalid parameter detected!" )

    // Set given values on internal member.
    sTargetName      = sTarget                            ;
    nSearchFlags     = nFlags                             ;
    eFrameType       = eType                              ;
    bChildrenExist   = bChildrens                         ;
    bParentExist     = bParent                            ;
    sFrameName       = sFrame                             ;
    sParentName      = sParent                            ;
    bCreationAllowed = impl_getCreateFlag( nSearchFlags ) ;
}

//*****************************************************************************************************************
TargetInfo::TargetInfo( const css::uno::Reference< css::frame::XFrame >& xFrame  ,
                        const ::rtl::OUString&                           sTarget ,
                              sal_Int32                                  nFlags  )
{
    // Check incoming parameter.
    LOG_ASSERT2( implcp_ctor( xFrame, sTarget, nFlags ), "TargetInfo::TargetInfo( 1 )", "Invalid parameter detected!" )

    // Set default values!
    // So we must reset it to valid values only.
    bChildrenExist = sal_False        ;
    bParentExist   = sal_False        ;
    sFrameName     = ::rtl::OUString();
    sParentName    = ::rtl::OUString();

    // Take given values into internal variables.
    sTargetName  = sTarget;
    nSearchFlags = nFlags ;

    // Try to analyze environment of given frame to set all other member!
    eFrameType = getFrameType( xFrame );
    switch( eFrameType )
    {
        case E_DESKTOP      :   break; // Nothing to do .. because: Desktop has no parent, no name ... Use default values!
                                       // But - values for children info is set later ...
        case E_TASK         :   {
                                    css::uno::Reference< css::frame::XFrame > xParent( xFrame->getCreator(), css::uno::UNO_QUERY );
                                    bParentExist = xParent.is();
                                    // Desktop has no name! Don't ask parent!
                                    sFrameName   = xFrame->getName();
                                }
                                break;
        case E_FRAME        :   {
                                    css::uno::Reference< css::frame::XFrame > xParent( xFrame->getCreator(), css::uno::UNO_QUERY );
                                    bParentExist = xParent.is();
                                    if( xParent.is() == sal_True )
                                    {
                                        sParentName = xParent->getName();
                                    }
                                    sFrameName = xFrame->getName();
                                }
                                break;
        case E_UNKNOWNFRAME :   break;
        default             :   break;
    }

    css::uno::Reference< css::frame::XFramesSupplier > xSupplier  ( xFrame, css::uno::UNO_QUERY );
    css::uno::Reference< css::frame::XFrames >         xChildrens ;
    if( xSupplier.is() == sal_True )
    {
        xChildrens = xSupplier->getFrames();
        if( xChildrens.is() == sal_True )
        {
            bChildrenExist = xChildrens->hasElements();
        }
    }

    bCreationAllowed = impl_getCreateFlag( nSearchFlags );
}

/*-************************************************************************************************************//**
    @short      get an enum, which represent type of given frame
    @descr      This value can be used by calling following classify...() methods.
                It regulate searching of targets.
                Why we don't detect this type internal?
                Mostly this helper is called from a frame or task implmentation directly and detection will be superflous.
                But sometimes this information isn't available ... so we should support a generaly implementation
                of this search algorithm :-)

    @seealso    method classify...()

    @param      "xFrame", reference to frame which should be detected
    @return     An enum value to classify the frame type.

    @onerror    No error should occure ... if incoming parameters are correct :-)
    @threadsafe No
*//*-*************************************************************************************************************/
EFrameType TargetInfo::getFrameType( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    // Check incoming parameter.
    LOG_ASSERT2( implcp_getFrameType( xFrame ), "TargetFinder::getFrameType()", "Invalid parameter detected!" )

    // Try to cast it to right interfaces to get informations about right frame type.
    css::uno::Reference< css::frame::XDesktop >           xDesktopCheck( xFrame, css::uno::UNO_QUERY );
    css::uno::Reference< css::frame::XFrame >             xFrameCheck  ( xFrame, css::uno::UNO_QUERY );

    EFrameType eType = E_UNKNOWNFRAME;

    if (xDesktopCheck.is())
        eType=E_DESKTOP;
    else
    if (xFrameCheck.is())
    {
        eType=E_FRAME;
        // But may it's a special task frame.
        // Such frames have the desktop as direct parent!
        css::uno::Reference< ::com::sun::star::frame::XDesktop > xDesktopParentCheck( xFrame->getCreator(), css::uno::UNO_QUERY );
        if (xDesktopParentCheck.is())
            eType=E_TASK;
    }

    LOG_ASSERT2( eType==E_UNKNOWNFRAME, "TargetFinder::getFrameType()", "Unknown frame implementation detected!" )

    return eType;
}

/*-************************************************************************************************************//**
    @short      get state of create flag
    @descr      These flag allow user to create a new frame if no one could be found!

    @attention  Ignore it, if it is the only setted flag.
                Why? Otherwise creation of frames by using same name without search for already existing frames
                will be possible! This case produce more then one frame by using same name ... and it's not clear, which frame would be found
                at further searches! It's better to ignore this case and disable creation.

    @seealso    ctor

    @param      "nSearchFlags", collection of setted flags
    @return     true , for setted flag
                false, if CREATE flag the only one or isn't set

    @onerror    We return false.
*//*-*************************************************************************************************************/
sal_Bool TargetInfo::impl_getCreateFlag( sal_Int32 aSearchFlags )
{
    bCreationAllowed = sal_False;
    if( aSearchFlags != css::frame::FrameSearchFlag::CREATE )
    {
        bCreationAllowed = (( aSearchFlags & css::frame::FrameSearchFlag::CREATE ) == css::frame::FrameSearchFlag::CREATE );
    }
    return bCreationAllowed;
}

/*-************************************************************************************************************//**
    @short      get a recommendation for searching right target
    @descr      Our caller search for a target which match with given parameter.
                Give him a direction to find the right one.
                These method never create or return a tree node! Thats your job!
                We say: go up, go down or give you the permission to create new frame if search will fail!

    @attention  XFrame::findFrame() and XDispatchProvider::queryDispatch() should understand
                target names and searchflags ... but not in the same way. findFrame() can create
                a new frame, if it is missing - queryDispatch() couldn't do that! Only the combination with a successfuly
                dispatch() call should do that!
                Another reason - there exist some "virtual" targets ... like "_helpagent", "_menubar".
                These targets couldn't be handled by findFrame(); but by a queryDispatch().
                Thats why we implement different classify methods for different mechanism!

    @seealso    method XFrame::findFrame()
    @seealso    method XDispatcher::queryDispatch()
    @seealso    struct TargetInfo

    @param      "aInfo", information about search environment
    @return     An enum value to classify the direction for searching.

    @onerror    E_UNKNOWN is returned.
*//*-*************************************************************************************************************/
ETargetClass TargetFinder::classifyFindFrame( TargetInfo& aInfo )
{
    // Check incoming parameter
    LOG_ASSERT2( implcp_classifyFindFrame( aInfo ), "TargetFinder::classifyFindFrame()", "Invalid parameter detected!" )

    ETargetClass eResult = E_UNKNOWN;

    // Use some helper methods for different classes of tree nodes to get the result.
    switch( aInfo.eFrameType )
    {
        case E_DESKTOP      :   eResult = impl_classifyForDesktop_findFrame( aInfo.bChildrenExist, aInfo.sTargetName, aInfo.nSearchFlags );
                                break;
        case E_TASK         :   eResult = impl_classifyForTask_findFrame( aInfo.bParentExist, aInfo.bChildrenExist, aInfo.sFrameName, aInfo.sTargetName, aInfo.nSearchFlags );
                                break;
        case E_FRAME        :   eResult = impl_classifyForFrame_findFrame( aInfo.bParentExist, aInfo.bChildrenExist, aInfo.sFrameName, aInfo.sParentName, aInfo.sTargetName, aInfo.nSearchFlags );
                                break;
        default             :   break;
    }

    // It doesnt matter if CREATE flag is set or not ...
    // If follow results are returned by our helper methods - the result will be clear!
    // In these cases we dont can allow (or must!) creation of new frames/tasks...
    LOG_ASSERT2( eResult==E_MENUBAR || eResult==E_HELPAGENT, "TargetFinder::classifyFindFrame()", "Invalid search result found!")
    if  (
            (   eResult ==  E_CREATETASK    )   ||
            (   eResult ==  E_SELF          )   ||
            (   eResult ==  E_PARENT        )   ||
            (   eResult ==  E_BEAMER        )
        )
    {
        aInfo.bCreationAllowed = sal_False;
    }

    return eResult;
}

//*****************************************************************************************************************
ETargetClass TargetFinder::classifyQueryDispatch( TargetInfo& aInfo )
{
    // Check incoming parameter
    LOG_ASSERT2( implcp_classifyQueryDispatch( aInfo ), "TargetFinder::classifyQueryDispatch()", "Invalid parameter detected!" )

    ETargetClass eResult = E_UNKNOWN;

    //*************************************************************************************************************
    // I ) handle "_menubar"
    // II) handle "_helpagent"
    //      DESKTOP             =>  Could not be handled by our desktop ...
    //                              because: Its a supported property of a task ... but which one should be used, if call comes from the top?!
    //      TASK                =>  Supported! return SELF
    //      FRAME               =>  Not supported ... but search for tasks ... if flags allow this search.
    //*************************************************************************************************************
    if( aInfo.sTargetName == SPECIALTARGET_MENUBAR )
    {
        switch( aInfo.eFrameType )
        {
            case E_TASK         :   eResult = E_MENUBAR;
                                    break;
            case E_FRAME        :   if( aInfo.bParentExist == sal_True )
                                    {
                                        eResult = E_FORWARD_UP;
                                    }
                                    break;
            default             :   break;
        }
    }
    else
    if( aInfo.sTargetName == SPECIALTARGET_HELPAGENT )
    {
        switch( aInfo.eFrameType )
        {
            case E_TASK         :   eResult = E_HELPAGENT;
                                    break;
            case E_FRAME        :   if( aInfo.bParentExist == sal_True )
                                    {
                                        eResult = E_FORWARD_UP;
                                    }
                                    break;
            default             :   break;
        }
    }
    //*************************************************************************************************************
    // III ) handle "_blank"
    //          DESKTOP     =>  Only the desktop can create new tasks ... he has a special dispatch helper to do that!
    //                          return CREATETASK
    //          TASK
    //          FRAME       =>  They couldn't create any new task => They must forward it to the desktop dispatch helper!
    //                          return FORWARD_UP
    //*************************************************************************************************************
    else
    if( aInfo.sTargetName == SPECIALTARGET_BLANK )
    {
        switch( aInfo.eFrameType )
        {
            case E_DESKTOP      :   eResult = E_CREATETASK;
                                    break;
            case E_TASK         :
            case E_FRAME        :   if( aInfo.bParentExist == sal_True )
                                    {
                                        eResult = E_FORWARD_UP;
                                    }
                                    break;
            default             :   break;
        }
    }
    else
    if( aInfo.sTargetName == SPECIALTARGET_DEFAULT )
    {
        switch( aInfo.eFrameType )
        {
            case E_DESKTOP      :   eResult = E_DEFAULT;
                                    break;
            case E_TASK         :
            case E_FRAME        :   if( aInfo.bParentExist == sal_True )
                                    {
                                        eResult = E_FORWARD_UP;
                                    }
                                    break;
            default             :   break;
        }
    }
    //*************************************************************************************************************
    // IV)  handle "", "_self"
    //      These case is clear. It's queals to "_self". Search flags could be ignored - other special target names
    //      couldn't occure - a real name couldn't be resolved realy! Frames with empty names exist more then ones ...
    //      Return E_SELF for desktop too!! Some URLs could be dispatched on desktop too ... e.g. slot, uno, macro URLs.
    //      Desktop must check other URLs - to prevent himself against wrong laoding of documents in it ... e.g. file...!
    //*************************************************************************************************************
    else
    if(
        ( aInfo.sTargetName.getLength() <  1                  ) ||
        ( aInfo.sTargetName             == SPECIALTARGET_SELF )
      )
    {
        eResult = E_SELF;
    }
    //*************************************************************************************************************
    // V)   There exist no other special targets or flag combinations ...
    //      I think we can use helper for normal findFrame() classify here!
    //*************************************************************************************************************
    else
    {
        switch( aInfo.eFrameType )
        {
            case E_DESKTOP      :   eResult = impl_classifyForDesktop_findFrame( aInfo.bChildrenExist, aInfo.sTargetName, aInfo.nSearchFlags );
                                    break;
            case E_TASK         :   eResult = impl_classifyForTask_findFrame( aInfo.bParentExist, aInfo.bChildrenExist, aInfo.sFrameName, aInfo.sTargetName, aInfo.nSearchFlags );
                                    break;
            case E_FRAME        :   eResult = impl_classifyForFrame_findFrame( aInfo.bParentExist, aInfo.bChildrenExist, aInfo.sFrameName, aInfo.sParentName, aInfo.sTargetName, aInfo.nSearchFlags );
                                    break;
            default             :   break;
        }
    }

    // It doesnt matter if CREATE flag is set or not ...
    // If follow results are returned by our helper methods - the result will be clear!
    // In these cases we dont can allow (or must!) creation of new frames/tasks...
    if  (
            (   eResult ==  E_CREATETASK    )   ||
            (   eResult ==  E_DEFAULT       )   ||
            (   eResult ==  E_SELF          )   ||
            (   eResult ==  E_PARENT        )   ||
            (   eResult ==  E_BEAMER        )   ||
            (   eResult ==  E_MENUBAR       )   ||
            (   eResult ==  E_HELPAGENT     )
        )
    {
        aInfo.bCreationAllowed = sal_False;
    }

    return eResult;
}

/*-************************************************************************************************************//**
    @short      helper methods for classify...()
    @descr      Every tree node (desktop, frame, task ...) has another preference shares to search a target.
                With these helper methods we differ between these search algorithm!

    @seealso    method classify...()

    @param      "bParentExist"      set if a parent exist for caller tree node
    @param      "bChildrenExist"    set if some children exist for caller tree node
    @param      "sFrameName"        name of current tree node (used for SELF flag to break search earlier!)
    @param      "sParentName"       name of current tree node (used for PARENT flag to break search earlier!)
    @param      "sTargetName"       name of searched target tree node
    @param      "nSearchFlags"      flags to regulate search in tree
    @param      "bTopFrame"         used to break upper searches at a top frame if search outside current task isnt allowed!
    @return     A decision about valid search direction.

    @onerror    E_UNKNOWN is returned.
*//*-*************************************************************************************************************/
ETargetClass TargetFinder::impl_classifyForDesktop_findFrame(       sal_Bool         bChildrenExist  ,
                                                              const ::rtl::OUString& sTargetName     ,
                                                                    sal_Int32        nSearchFlags    )
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
                ( nSearchFlags & css::frame::FrameSearchFlag::TASKS )   &&
                (
                    !( nSearchFlags & css::frame::FrameSearchFlag::CHILDREN )   &&
                    !( nSearchFlags & css::frame::FrameSearchFlag::SIBLINGS )   &&
                    !( nSearchFlags & css::frame::FrameSearchFlag::PARENT   )   &&
                    !( nSearchFlags & css::frame::FrameSearchFlag::SELF     )
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
                    ( nSearchFlags      &   css::frame::FrameSearchFlag::CHILDREN   )   &&
                    ( bChildrenExist    ==  sal_True                                )
                )
            {
                eResult = E_DEEP_DOWN;
            }

            //*****************************************************************************************************
            //  II.II)  Look for SIBLINGS.
            //          These change a deep to a flat search!
            //          Otherwise ... flag can be ignored - because the desktop has no siblings!
            //*****************************************************************************************************
            if( nSearchFlags & css::frame::FrameSearchFlag::SIBLINGS )
            {
                switch( eResult )
                {
                    case E_DEEP_DOWN    :   eResult = E_FLAT_DOWN;
                                            break;
                              default           :     break;
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
ETargetClass TargetFinder::impl_classifyForTask_findFrame(        sal_Bool          bParentExist    ,
                                                                  sal_Bool          bChildrenExist  ,
                                                            const ::rtl::OUString&  sFrameName      ,
                                                            const ::rtl::OUString&  sTargetName     ,
                                                                  sal_Int32         nSearchFlags    )
{
    ETargetClass    eResult     =   E_UNKNOWN                                                                                       ;
    sal_Bool        bLeaveTask  =   (( nSearchFlags & css::frame::FrameSearchFlag::TASKS  ) == css::frame::FrameSearchFlag::TASKS  );   // we must know if we can search outside current task

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
                ( nSearchFlags  &   css::frame::FrameSearchFlag::SELF   )   &&
                ( sTargetName   ==  sFrameName                          )
            )
        {
            eResult = E_SELF;
        }

        //*********************************************************************************************************
        //  II.III) Look for PARENT.
        //          Is allowed on tasks if outside search of it is allowed!
        //          Don't check name of parent here - otherwise we return the desktop as result ...
        //*********************************************************************************************************
        if  (
                ( eResult       !=  E_SELF                              )   &&
                ( nSearchFlags  &   css::frame::FrameSearchFlag::PARENT )   &&
                ( bParentExist  ==  sal_True                            )   &&
                ( bLeaveTask    ==  sal_True                            )
            )
        {
            eResult = E_FORWARD_UP;
        }

        //*********************************************************************************************************
        //  II.II)  Look for CHILDREN.
        //          Ignore flag if no childrens exist!
        //*********************************************************************************************************
        if  (
                ( eResult           !=  E_SELF                                  )   &&
                ( nSearchFlags      &   css::frame::FrameSearchFlag::CHILDREN   )   &&
                ( bChildrenExist    ==  sal_True                                )
            )
        {
            switch( eResult )
            {
                case E_UNKNOWN    : eResult = E_DEEP_DOWN;
                                            break;
                case E_FORWARD_UP   : eResult = E_DEEP_BOTH;
                                            break;
                        default:            break;
            }
        }

        //*********************************************************************************************************
        //  II.III) Look for SIBLINGS.
        //          These change a deep to a flat search!
        //*********************************************************************************************************
        if  (
                ( eResult       !=  E_SELF                                  )   &&
                ( nSearchFlags  &   css::frame::FrameSearchFlag::SIBLINGS   )
            )
        {
            switch( eResult )
            {
                case E_DEEP_DOWN    :   eResult = E_FLAT_DOWN;
                                break;
                case E_DEEP_BOTH    :   eResult = E_FLAT_BOTH;
                                break;
                        default:                break;
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
ETargetClass TargetFinder::impl_classifyForFrame_findFrame(         sal_Bool           bParentExist    ,
                                                                    sal_Bool           bChildrenExist  ,
                                                            const   ::rtl::OUString&   sFrameName      ,
                                                            const   ::rtl::OUString&   sParentName     ,
                                                            const   ::rtl::OUString&   sTargetName     ,
                                                                    sal_Int32          nSearchFlags    )
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
                ( nSearchFlags  &   css::frame::FrameSearchFlag::SELF   )   &&
                ( sTargetName   ==  sFrameName                          )
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
                ( eResult       !=  E_SELF                              )   &&
                ( nSearchFlags  &   css::frame::FrameSearchFlag::PARENT )   &&
                ( bParentExist  ==  sal_True                            )
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
                ( eResult           !=  E_SELF                                  )   &&
                ( eResult           !=  E_PARENT                                )   &&
                ( nSearchFlags      &   css::frame::FrameSearchFlag::CHILDREN   )   &&
                ( bChildrenExist    ==  sal_True                                )
            )
        {
            switch( eResult )
            {
                case E_UNKNOWN    : eResult = E_DEEP_DOWN;
                                                break;
                case E_FORWARD_UP   :   eResult = E_DEEP_BOTH;
                                                break;
                        default           :     break;
            }
        }

        //*********************************************************************************************************
        //  II.IV)  Look for SIBLINGS.
        //          These change a deep to a flat search!
        //*********************************************************************************************************
        if  (
                ( eResult       !=  E_SELF                                  )   &&
                ( eResult       !=  E_PARENT                                )   &&
                ( nSearchFlags  &   css::frame::FrameSearchFlag::SIBLINGS   )
            )
        {
            switch( eResult )
            {
                case E_DEEP_DOWN    :   eResult = E_FLAT_DOWN;
                                break;
                case E_DEEP_BOTH    :   eResult = E_FLAT_BOTH;
                                                break;
                        default           :     break;
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

//*****************************************************************************************************************
sal_Bool TargetInfo::implcp_ctor( const ::rtl::OUString& sTarget    ,
                                        sal_Int32        /*nFlags*/ ,
                                        EFrameType       eType      ,
                                        sal_Bool         bChildrens ,
                                        sal_Bool         bParent    ,
                                  const ::rtl::OUString& sFrame     ,
                                  const ::rtl::OUString& sParent    )
{
    return(
            ( &sTarget       == NULL            )   ||
            ( eType          == E_UNKNOWNFRAME  )   ||
            (
                ( bChildrens != sal_True        )   &&
                ( bChildrens != sal_False       )
            )                                       ||
            (
                ( bParent    != sal_True        )   &&
                ( bParent    != sal_False       )
            )                                       ||
            ( &sFrame        == NULL            )   ||
            ( &sParent       == NULL            )
          );
}

//*****************************************************************************************************************
// Attention: - empty "sTarget" is allowed! => equal to "_self"
//            - there exist no test for flags!
sal_Bool TargetInfo::implcp_ctor( const css::uno::Reference< css::frame::XFrame >& xFrame  ,
                                  const ::rtl::OUString&                           sTarget ,
                                        sal_Int32                                /*nFlags*/  )
{
    return(
            ( &xFrame     == NULL      )    ||
            ( xFrame.is() == sal_False )    ||
            ( &sTarget    == NULL      )
          );
}

//*****************************************************************************************************************
sal_Bool TargetInfo::implcp_getFrameType( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    return(
            ( &xFrame     == NULL      )  ||
            ( xFrame.is() == sal_False )
          );
}

//*****************************************************************************************************************
sal_Bool TargetFinder::implcp_classifyFindFrame( const TargetInfo& aInfo )
{
    return( &aInfo == NULL );
}

//*****************************************************************************************************************
sal_Bool TargetFinder::implcp_classifyQueryDispatch( const TargetInfo& aInfo )
{
    return( &aInfo == NULL );
}

#endif  //  #ifdef ENABLE_ASSERTIONS

}       //  namespace framework
