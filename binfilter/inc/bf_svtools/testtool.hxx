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
#ifndef SVTOOLS_TESTTOOL_HXX
#define SVTOOLS_TESTTOOL_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

class Application;
class SvStream;

namespace binfilter
{

class StatementFlow;
class CommunicationManager;
class CommunicationLink;
#if OSL_DEBUG_LEVEL > 1
class EditWindow;
#endif
class ImplRC;

class RemoteControl
{
    friend class StatementFlow;

    BOOL         m_bIdleInserted;
#if OSL_DEBUG_LEVEL > 1
    EditWindow *m_pDbgWin;
#endif
    ImplRC* pImplRC;

public:
    RemoteControl();
    ~RemoteControl();
    BOOL QueCommands( ULONG nServiceId, SvStream *pIn );
    SvStream* GetReturnStream();

    DECL_LINK( IdleHdl,   Application* );
    DECL_LINK( CommandHdl, Application* );

    DECL_LINK( QueCommandsEvent, CommunicationLink* );
    ULONG nStoredServiceId;
    SvStream *pStoredStream;

    void ExecuteURL( String &aURL );

protected:
    CommunicationManager *pServiceMgr;
    SvStream *pRetStream;
};

}

#endif // SVTOOLS_TESTTOOL_HXX
