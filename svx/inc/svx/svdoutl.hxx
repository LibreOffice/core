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

#include <editeng/outliner.hxx>
#include "svx/svxdllapi.h"
#include <svx/svdobj.hxx>

class SdrTextObj;
class SdrPaintInfoRec;
class SdrPage;

class SVX_DLLPUBLIC SdrOutliner : public Outliner
{
protected:
    SdrObjectWeakRef mpTextObj;
    const SdrPage* mpVisualizedPage;

public:
    SdrOutliner( SfxItemPool* pItemPool, sal_uInt16 nMode );
    virtual ~SdrOutliner() ;

    void  SetTextObj( const SdrTextObj* pObj );
    void SetTextObjNoInit( const SdrTextObj* pObj );
    const SdrTextObj* GetTextObj() const;

    void setVisualizedPage(const SdrPage* pPage) { if(pPage != mpVisualizedPage) mpVisualizedPage = pPage; }
    const SdrPage* getVisualizedPage() const { return mpVisualizedPage; }

    virtual String  CalcFieldValue(const SvxFieldItem& rField, sal_uInt16 nPara, sal_uInt16 nPos, Color*& rpTxtColor, Color*& rpFldColor);
};

#endif //_SVDOUTL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
