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


#ifndef APPLICAT_HXX
#define APPLICAT_HXX

class SvxErrorHandler;

namespace binfilter {

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

    virtual void		OpenClients();

    // initialization / deinitialization
    virtual void		Init();
    virtual void        Exit();

public:
    void Main();

    SmApplicat() :
        SfxApplication("iso")
    {
    }

};

#endif
} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
