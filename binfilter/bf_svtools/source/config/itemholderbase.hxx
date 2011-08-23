/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef INCLUDED_SVTOOLS_ITEMHOLDERBASE_HXX_
#define INCLUDED_SVTOOLS_ITEMHOLDERBASE_HXX_

//-----------------------------------------------
// includes

#include <vector>

#include <osl/mutex.hxx>

namespace binfilter
{

class Options;

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

    Options * pItem;
    EItem eItem;
};

typedef ::std::vector< TItemInfo > TItems;

}

#endif // INCLUDED_SVTOOLS_ITEMHOLDERBASE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
