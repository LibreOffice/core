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

#ifndef _SVDOUTL_HXX
#define _SVDOUTL_HXX

#include <bf_svx/outliner.hxx>
namespace binfilter {

class SdrTextObj;

class SdrOutliner : public Outliner
{
protected:
    const SdrTextObj* pTextObj;

public:
//    TYPEINFO();

    SdrOutliner( SfxItemPool* pItemPool, USHORT nMode );
    virtual ~SdrOutliner() ;

    void  SetTextObj( const SdrTextObj* pObj );
    void SetTextObjNoInit( const SdrTextObj* pObj );

    const SdrTextObj* GetTextObj() const { return pTextObj; }

    virtual String  CalcFieldValue(const SvxFieldItem& rField, USHORT nPara, USHORT nPos, Color*& rpTxtColor, Color*& rpFldColor);
};

}//end of namespace binfilter
#endif //_SVDOUTL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
