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

#ifndef _RSCLST_HXX
#define _RSCLST_HXX

#include <rscall.h>

class REResourceList;

class REResourceList
{
protected:
    REResourceList*  pParent;
    RscId            aRscId;    //Id und Name des Resourceobjektes
    ByteString       aClassName;
    sal_Bool             bVisible;

public:
                     REResourceList();
                     REResourceList( REResourceList * pParentList,
                                     ByteString& rClassName,
                                     const RscId & rResourceID,
                                     sal_Bool   bVisible = sal_False );
                     ~REResourceList();

    REResourceList*  GetParent()     { return pParent; }
    ByteString       GetObjName()    { return aRscId.GetName(); }
    ByteString       GetClassName()  { return aClassName; }
    RscId            GetRscId()      { return aRscId; }
    void             SetRscId( const RscId & rId ){ aRscId = rId; }

    void             SetVisible( sal_Bool bVis )
                                     { bVisible = bVis; }
    sal_Bool             IsVisible()     { return bVisible; }
};

#endif // _RSCLST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
