/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: itemholderbase.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-11 13:51:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems,   //  Inc.
 *    901 San Antonio Road,   //  Palo Alto,   //  CA 94303,   //  USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1,   //  as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,   //
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not,   //  write to the Free Software
 *    Foundation,   //  Inc.,   //  59 Temple Place,   //  Suite 330,   //  Boston,   //
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_SVTOOLS_ITEMHOLDERBASE_HXX_
#define INCLUDED_SVTOOLS_ITEMHOLDERBASE_HXX_

//-----------------------------------------------
// includes

#include <vector>

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

//-----------------------------------------------
// definitions

struct ItemHolderMutexBase
{
    ::osl::Mutex m_aLock;
};

enum EItem
{
    E_ACCELCFG                      ,   //
    E_ACCESSIBILITYOPTIONS          ,   // 2
    E_ADDXMLTOSTORAGEOPTIONS        ,   //
    E_APEARCFG                      ,   // 2

    E_CJKOPTIONS                    ,   // 2
    E_CMDOPTIONS                    ,   //
    E_COLORCFG                      ,   // 2
    E_COMPATIBILITY                 ,   //
    E_CTLOPTIONS                    ,   // 2

    E_DEFAULTOPTIONS                ,   //
    E_DYNAMICMENUOPTIONS            ,   //

    E_EVENTCFG                      ,   //
    E_EXTENDEDSECURITYOPTIONS       ,   //

    E_FLTRCFG                       ,   //
    E_FONTOPTIONS                   ,   //
    E_FONTSUBSTCONFIG               ,   // 2

    E_HELPOPTIONS                   ,   // 2
    E_HISTORYOPTIONS                ,   //

    E_INETOPTIONS                   ,   //
    E_INTERNALOPTIONS               ,   //

    E_JAVAOPTIONS                   ,   //

    E_LANGUAGEOPTIONS               ,   // 2
    E_LINGUCFG                      ,   //
    E_LOCALISATIONOPTIONS           ,   //

    E_MENUOPTIONS                   ,   //
    E_MISCCFG                       ,   // 2
    E_MISCOPTIONS                   ,   //
    E_MODULEOPTIONS                 ,   //

    E_OPTIONSDLGOPTIONS             ,   //
    E_OPTIONS3D                     ,   //

    E_PATHOPTIONS                   ,   //
    E_PRINTOPTIONS                  ,   // 2
    E_PRINTFILEOPTIONS              ,   // 2
    E_PRINTWARNINGOPTIONS           ,   //

    E_REGOPTIONS                    ,   //

    E_SAVEOPTIONS                   ,   //
    E_SEARCHOPT                     ,   //
    E_SECURITYOPTIONS               ,   //
    E_SOURCEVIEWCONFIG              ,   //
    E_STARTOPTIONS                  ,   //
    E_SYSLOCALEOPTIONS              ,   // 2

    E_UNDOOPTIONS                   ,   // 2
    E_USEROPTIONS                   ,   // 2

    E_VIEWOPTIONS_DIALOG            ,   //
    E_VIEWOPTIONS_TABDIALOG         ,   //
    E_VIEWOPTIONS_TABPAGE           ,   //
    E_VIEWOPTIONS_WINDOW            ,   //

    E_WORKINGSETOPTIONS             ,   //

    E_XMLACCELCFG                       //
};

struct TItemInfo
{
    TItemInfo()
        : pItem(0)
    {}

    void* pItem;
    EItem eItem;
};

typedef ::std::vector< TItemInfo > TItems;

#endif // INCLUDED_SVTOOLS_ITEMHOLDERBASE_HXX_
