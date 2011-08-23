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

#ifndef _E3D_PNTOBJ3D_HXX
#define _E3D_PNTOBJ3D_HXX

#include <bf_svx/obj3d.hxx>
namespace binfilter {

/*************************************************************************
|*
|* Klasse fuer einzelne dreidimensionale Punkte
|*
\************************************************************************/

class E3dPointObj : public E3dObject
{
 protected:
    Vector3D	aPosition;
    Vector3D	aTransPos;

    FASTBOOL	bTransPosValid	: 1;

    virtual void SetTransformChanged();

 public:
    TYPEINFO();
    E3dPointObj(const Vector3D& rPos);
    E3dPointObj();


    virtual void SetPosition(const Vector3D& rNewPos);
    const Vector3D& GetPosition() const { return aPosition; }
    const Vector3D& GetTransPosition();

    virtual void WriteData(SvStream& rOut) const;
    virtual void ReadData(const SdrObjIOHeader& rHead, SvStream& rIn);

};

}//end of namespace binfilter
#endif			// _E3D_PNTOBJ3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
