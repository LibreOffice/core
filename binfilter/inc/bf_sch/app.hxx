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

#ifndef _SCH_APP_HXX
#define _SCH_APP_HXX

// Die folgenden includes gehoeren eigentlich in app.cxx

#include "schdll.hxx"

class SdClipboard;
class SdBasic;
class BasicIDE;

#ifndef _SCH_GLOB_HXX
#include <glob.hxx>
#endif

#ifndef _SFXNEW_HXX //autogen
#include <bf_sfx2/new.hxx>
#endif

#ifndef _SVXERR_HXX //autogen
#include <bf_svx/svxerr.hxx>
#endif

#ifndef _SFXAPP_HXX //autogen
#include <bf_sfx2/app.hxx>
#endif
namespace binfilter {

/*************************************************************************
|*
|* Klasse fuer Applikationsobjekt
|*
\************************************************************************/

class SchApp: public SfxApplication
{
protected :
    virtual void Init();
    virtual void Exit();
    virtual void OpenClients();

    SvxErrorHandler *pSvxErrorHandler;

public:
    SchApp();
    virtual ~SchApp();

    virtual void Main();
};



} //namespace binfilter
#endif		// _SCH_APP_HXX

