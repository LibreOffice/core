/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: iderdll.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _IDERDLL_HXX
#define _IDERDLL_HXX

class BasicIDEShell;
class BasicIDEData;

class BasicIDEDLL
{
    friend class BasicIDEShell;

    BasicIDEShell*  pShell;
    BasicIDEData*   pExtraData;

public:
                    BasicIDEDLL();
                    ~BasicIDEDLL();

    BasicIDEShell*  GetShell() const { return pShell; }
    BasicIDEData*   GetExtraData();
    static void     Init();
    static void     Exit();
    static BasicIDEDLL* GetDLL();
};

#define IDE_DLL()   BasicIDEDLL::GetDLL()

#endif //_IDERDLL_HXX
