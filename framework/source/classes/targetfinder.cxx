/*************************************************************************
 *
 *  $RCSfile: targetfinder.cxx,v $
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

#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XTASK_HPP_
#include <com/sun/star/frame/XTask.hpp>
#endif

#ifndef _COM_SUN_STAR_MOZILLA_XPLUGININSTANCE_HPP_
#include <com/sun/star/mozilla/XPluginInstance.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XELEMENTACCESS_HPP_
#include <com/sun/star/container/XElementAccess.hpp>
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
using namespace ::com::sun::star::mozilla   ;
using namespace ::com::sun::star::container ;

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
IMPL_ETargetClass TargetFinder::classify(   const   Reference< XFrame >&    xOwner          ,
                                            const   OUString&               sTargetName     ,
                                                       sal_Int32                nSearchFlags    )
{
    /*Attention

         The desktop is a special object - the root of our frame tree ...
        but he can't handle components!
        I think he could'nt be a realy supported target any time!!!
        Never return it as SELF or PARENT or TOP ...

    */

    // Safe impossible cases.
    // These method is not defined for all incoming parameter!
    LOG_ASSERT( impldbg_checkParameter_classify( xOwner, sTargetName, nSearchFlags ), "TargetFinder::classify()\nInvalid parameter detected!\n" )

    // Set return value if method failed.
    IMPL_ETargetClass eResult = eUNKNOWN;

    // Get some special informations about our caller.
    // We need his name, his type ... and something else.
    IMPL_EFrameType eFrameType      ;
    sal_Bool        bParentExist    ;
    sal_Bool        bChildrenExist  ;
    OUString        sFrameName      ;

    // Children can exist for every frame implementation.
    Reference< XFramesSupplier > xSupplier( xOwner                  , UNO_QUERY );
    Reference< XElementAccess >  xAccess  ( xSupplier->getFrames()  , UNO_QUERY );
    bChildrenExist = xAccess->hasElements();

    // But all other informations are optional or defaults!
    if( Reference< XDesktop >( xOwner, UNO_QUERY ).is() ==  sal_True )
    {
        // a)   Our desktop is a special implementation!
        //      He has no parent and no name.
        eFrameType      =   eDESKTOP    ;
        bParentExist    =   sal_False   ;
        sFrameName      =   OUString()  ;
    }
    else
    {
        // b)   All other implementations has a parent and it's own name.
        //      We set frame type to default eFRAME ...
        eFrameType      =   eFRAME                      ;
        bParentExist    =   xOwner->getCreator().is()   ;
        sFrameName      =   xOwner->getName()           ;
        if( Reference< XTask >( xOwner, UNO_QUERY ).is() ==  sal_True )
        {
            // c)   ... but it can be that our caller is a task ...
            eFrameType = eTASK;
        }
        else
        if( Reference< XPluginInstance >( xOwner, UNO_QUERY ).is() ==  sal_True )
        {
            // d)   ... or a plug-in frame!
            eFrameType = ePLUGINFRAME;
        }
    }

    //*************************************************************************************************************
    //  1)  Look for "_blank"
    //      These is the most used case and must be fast!
    //      FrameSearchFlag::CREATE can be used at least if no other parameter match the given one!!!
    //      Return "eUP" for all normaly frames/tasks/plugins ... These implementations don't must known something about
    //      creation(!) ... The desktop only can do it - and we return right recommendation for it.
    //*************************************************************************************************************
    if( sTargetName == SPECIALTARGET_BLANK )
    {
        if( eFrameType == eDESKTOP )
        {
            eResult = eCREATE;
        }
        else
        {
            eResult = eUP;
        }
    }
    else

    //*************************************************************************************************************
    //  2)  Look for "_self", "". Its the same like "_self"!
    //*************************************************************************************************************
    if  (
            ( sTargetName               ==  SPECIALTARGET_SELF  )   ||  //  "_self"
            ( sTargetName.getLength()   <   1                   )       //  ""
        )
    {
        eResult = eSELF;
    }
    else

    //*************************************************************************************************************
    //  3)  Look for "_top".
    //      We must do it before "_parent" because it can a be combination of existing parent - frame type and ...
    //*************************************************************************************************************
    if( sTargetName == SPECIALTARGET_TOP )
    {
        switch( eFrameType )
        {
            // a) A normal frame without a parent is TOP and must handle it by himself.
            case eFRAME         :   {
                                        if( bParentExist == sal_False )
                                        {
                                            eResult = eSELF;
                                        }
                                    }
                                    break;

            // b) A task is TOP every time and must handle it by himself.
            // c) A plugin frame ... too.
            case eTASK          :
            case ePLUGINFRAME   :   {
                                        eResult = eSELF;
                                    }
                                    break;
        }
    }
    else

    //*************************************************************************************************************
    //  4)  Look for "_parent". We must handle these as DIRECT parent only. It's not a flag to search at parents ...
    //
    //      Attention: If a parent exist we return ePARENT as recommendation ...
    //      but don't do it if frame type different from eFRAME(!) ...
    //      because; otherwise the parent is the desktop automaticly!!!
    //      (see ATTENTION a beginning of these function for further informations)
    //*************************************************************************************************************
    if  (
            ( sTargetName   ==  SPECIALTARGET_PARENT    )   &&
            ( bParentExist  ==  sal_True                )   &&
            ( eFrameType    ==  eFRAME                  )
        )
    {
        eResult = ePARENT;
    }
    else

    //*************************************************************************************************************
    //  ATTENTION!
    //  We have searched for special targets only before ... and it was an exclusive search.
    //  [ if() else if() else ... ]
    //  But now we must search for any named frames and use search flags to do that in different combinations!
    //  Look for any untested flag before if no result exist at that time!
    //  [ if_no_result(); if_no_result(); return result ]
    //*************************************************************************************************************
    {
        //*****************************************************************************************************
        //  5)  Look for SELF. Check right name.
        //      We don't must look for an empty target name "" (!)
        //      because we have already done it in 2).
        //      Dont handle SELF for desktop!
        //*****************************************************************************************************
        if  (
                ( eFrameType    !=  eDESKTOP                )   &&
                ( nSearchFlags  &   FrameSearchFlag::SELF   )   &&
                ( sTargetName   ==  sFrameName              )
            )
        {
            eResult = eSELF;
        }

        //*****************************************************************************************************
        //  6)  Look for PARENT.
        //      You can do it for our desktop ... because he mst support search on his children!
        //      Our implementation will protect us against SELF/_self/"" on the desktop ...
        //*****************************************************************************************************
        if  (
                ( eResult       ==  eUNKNOWN                )   &&
                ( nSearchFlags  &   FrameSearchFlag::PARENT )   &&
                ( bParentExist  ==  sal_True                )   &&
                ( eFrameType    !=  eDESKTOP                )
            )
        {
            eResult = eUP;
        }

        //*************************************************************************************************************
        //  7)  Look for CHILDREN.
        //  Attention:  In 6) we set return value to eUP ... but other flags can combined with these one!
        //              zB CHILDREN
        //              In these case we must correct our result to eALL, I think!
        //*************************************************************************************************************
        if  (
                ( nSearchFlags      &   FrameSearchFlag::CHILDREN   )   &&
                ( bChildrenExist    ==  sal_True                    )
            )
        {
            switch( eResult )
            {
                case eUNKNOWN   :   {
                                        eResult = eDOWN;
                                    }
                                    break;
                case eUP        :   {
                                        eResult = eALL;
                                    }
                                    break;
            }
        }

        //*************************************************************************************************************
        //  8)  Search for SIBLINGS.
        //      We must check for existing parents because we can search our siblings as children of our parent only!
        //*************************************************************************************************************
        if  (
                ( eResult       ==  eUNKNOWN                    )   &&
                ( nSearchFlags  &   FrameSearchFlag::SIBLINGS   )   &&
                ( bParentExist  ==  sal_True                    )   &&
                ( eFrameType    !=  eDESKTOP                    )
            )
        {
            eResult = eSIBLINGS;
        }

        //*************************************************************************************************************
        //  9)  Search for TASKS.
        //      If CREATE is set we must forward call to desktop. He is the only one, who can do that!
        //*************************************************************************************************************
        if  (
                ( eResult       ==  eUNKNOWN                )   &&
                ( nSearchFlags  &   FrameSearchFlag::TASKS  )
            )
        {
            if( nSearchFlags & FrameSearchFlag::CREATE )
            {
                switch( eFrameType )
                {
                    case eTASK          :
                    case ePLUGINFRAME   :
                    case eFRAME         :   {
                                                eResult = eUP;
                                            }
                                            break;

                    case eDESKTOP       :   {
                                                eResult = eCREATE;
                                            }
                                            break;
                }
            }
            else
            {
                switch( eFrameType )
                {
                    case eTASK          :
                    case ePLUGINFRAME   :   {
                                                eResult = eSELF;
                                            }
                                            break;

                    case eFRAME         :   {
                                                eResult = eUP;
                                            }
                                            break;

                    case eDESKTOP       :   {
                                                eResult = eDOWN;
                                            }
                                            break;
                }
            }
        }
    }

    // Return result of operation.
    return eResult;
}

//*****************************************************************************************************************
//  interface
//*****************************************************************************************************************
Reference< XFrame > TargetFinder::helpDownSearch(   const   Reference< XFrames >&   xChildFrameAccess   ,
                                                    const   OUString&               sTargetName         )
{
    // Safe impossible cases.
    // We don't accept all incoming parameter!
    LOG_ASSERT( impldbg_checkParameter_helpDownSearch( xChildFrameAccess, sTargetName ), "TargetFinder::helpDownSearch()\nInvalid parameter detected!\n" )

    // Set default return value if method failed.
    Reference< XFrame > xResult;

    // Get a collection of all childs of our owner frame,
    // and search given target name in these list.
    Sequence< Reference< XFrame > > seqChilds   = xChildFrameAccess->queryFrames( FrameSearchFlag::CHILDREN );
    sal_uInt32                      nCount      = seqChilds.getLength();
    sal_uInt32                      nPosition   = 0;
    for( nPosition=0; nPosition<nCount; ++nPosition )
    {
        if( seqChilds[nPosition]->getName() == sTargetName )
        {
            xResult = seqChilds[nPosition];
            break;
        }
    }

    return xResult;
}

//_________________________________________________________________________________________________________________
//  debug methods
//_________________________________________________________________________________________________________________

/*-----------------------------------------------------------------------------------------------------------------
    The follow methods check parameter for other functions. If a parameter or his value is non valid,
    we return "sal_False". (else sal_True) This mechanism is used to throw an ASSERT!

    ATTENTION

        If you miss a test for one of this parameters, contact the autor or add it himself !(?)
        But ... look for right testing! See using of this methods!
-----------------------------------------------------------------------------------------------------------------*/

#ifdef ENABLE_ASSERTIONS

//*****************************************************************************************************************
// Check for valid pointer only in the moment - I think to control all combinations of flags is'nt a good idea ...
// The target name can be empty but we must look for valid enum values.
sal_Bool TargetFinder::impldbg_checkParameter_classify( const   Reference< XFrame >&    xOwner          ,
                                                        const   OUString&               sTargetName     ,
                                                                sal_Int32               nSearchFlags    )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xOwner       ==  NULL        )   ||
            ( xOwner.is()   ==  sal_False   )   ||
            ( &sTargetName  ==  NULL        )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
// We can't work with invalid references and don't accept special target names!
// We search for realy named frames only.
sal_Bool TargetFinder::impldbg_checkParameter_helpDownSearch(   const   Reference< XFrames >&   xChildFrameAccess   ,
                                                                 const  OUSTRING&               sTargetName         )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xChildFrameAccess    ==  NULL                    )   ||
            ( xChildFrameAccess.is()==  sal_False               )   ||
            ( &sTargetName          ==  NULL                    )   ||
            ( sTargetName           ==  SPECIALTARGET_BLANK     )   ||
            ( sTargetName           ==  SPECIALTARGET_SELF      )   ||
            ( sTargetName           ==  SPECIALTARGET_PARENT    )   ||
            ( sTargetName           ==  SPECIALTARGET_TOP       )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

#endif  // #ifdef ENABLE_ASSERTIONS

}       //  namespace framework
