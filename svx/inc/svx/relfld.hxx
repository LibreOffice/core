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
#ifndef _SVX_RELFLD_HXX
#define _SVX_RELFLD_HXX

#include <vcl/field.hxx>
#include "svx/svxdllapi.h"


// class SvxRelativeField ------------------------------------------------

class SVX_DLLPUBLIC SvxRelativeField : public MetricField
{
private:
    sal_uInt16          nRelMin;
    sal_uInt16          nRelMax;
    sal_uInt16          nRelStep;
    sal_Bool            bRelativeMode;
    sal_Bool            bRelative;
    sal_Bool            bNegativeEnabled;

protected:
    void            Modify();

public:
    SvxRelativeField( Window* pParent, const ResId& rResId );

    void            EnableRelativeMode( sal_uInt16 nMin = 50, sal_uInt16 nMax = 150,
                                        sal_uInt16 nStep = 5 );
    sal_Bool            IsRelativeMode() const { return bRelativeMode; }
    void            SetRelative( sal_Bool bRelative = sal_False );
    sal_Bool            IsRelative() const { return bRelative; }
    void            EnableNegativeMode() {bNegativeEnabled = sal_True;}
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
