/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: status.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2007-06-19 14:38:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _BASICSTATUS_HXX
#define _BASICSTATUS_HXX

#ifndef _TASKBAR_HXX
#include <svtools/taskbar.hxx>
#endif
#ifndef _SV_STATUS_HXX
#include <vcl/status.hxx>
#endif

#define ST_MESSAGE          1
#define ST_LINE             2
#define ST_PROF             3

class BasicFrame;

class StatusLine : public TaskBar
{
protected:
    BasicFrame* pFrame;
    DECL_LINK( ActivateTask, TaskToolBox* );

public:
    StatusLine( BasicFrame* );
    void Message( const String& );              // Text anzeigen
    void Pos( const String& s );                    // Textposition anzeigen
    void SetProfileName( const String& s );     // Current Profile
    void LoadTaskToolBox();
};

#endif
