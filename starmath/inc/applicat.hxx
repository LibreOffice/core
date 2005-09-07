/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: applicat.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 14:57:06 $
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


#ifndef APPLICAT_HXX
#define APPLICAT_HXX

class SvxErrorHandler;

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif

/**************************************************************************/
/*
**
**  MACRO DEFINITION
**
**/

#define SMDLL   1

#define APPLICATIONNAME      "smath3"

/**************************************************************************/
/*
**
**  CLASS DEFINITION
**
**/

#ifdef WIN
#define RELEASE     "WIN304"
#endif

#ifdef PM2
#define RELEASE     "PM304"
#endif

#ifdef MAC
#define RELEASE     "MAC304"
#endif

#ifdef WNT
#define RELEASE     "WNT304"
#endif

#ifdef UNX
#define RELEASE     "UNX304"
#endif

#ifndef SMDLL
class SmResId : public ResId
{
public:
    SmResId(USHORT nId) :
        ResId(nId)
    {
    }

};

#endif

#ifndef _DLL_
class SmDLL;

class SmApplicat: public SfxApplication
{
protected:
    SvxErrorHandler     *pSvxErrorHandler;

    virtual void        OpenClients();

    // initialization / deinitialization
    virtual void        Init();
    virtual void        Exit();

public:
    void Main();

    SmApplicat() :
        SfxApplication("iso")
    {
    }

};

#endif
#endif

