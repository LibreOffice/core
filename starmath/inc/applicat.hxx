/*************************************************************************
 *
 *  $RCSfile: applicat.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:57:24 $
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

    // factories
    virtual SfxFileDialog* CreateDocFileDialog(ULONG nBits,
                                        const SfxObjectFactory &rFact);
    virtual void        OpenClients();

    // initialization / deinitialization
#ifndef VCL
    virtual void        Init(int nArgs, char* pArgs[]);
#else
    virtual void        Init();
#endif
    virtual void        Exit();

public:
#ifndef VCL
    void Main(int nArgc, char* pArgv[]);
#else
    void Main();
#endif

    SmApplicat() :
        SfxApplication("iso")
    {
    }

};

#endif
#endif

