/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SVX_RELFLD_HXX
#define INCLUDED_SVX_RELFLD_HXX

#include <tools/fldunit.hxx>
#include <vcl/field.hxx>
#include <svx/svxdllapi.h>


// class SvxRelativeField ------------------------------------------------

class SVX_DLLPUBLIC SvxRelativeField : public MetricField
{
private:
    sal_uInt16          nRelMin;
    sal_uInt16          nRelMax;
    sal_uInt16          nRelStep;
    bool                bRelativeMode;
    bool                bRelative;
    bool                bNegativeEnabled;

protected:
    void            Modify() override;

public:
    SvxRelativeField(vcl::Window* pParent, WinBits nBits, FieldUnit eUnit);

    void            EnableRelativeMode( sal_uInt16 nMin = 50, sal_uInt16 nMax = 150,
                                        sal_uInt16 nStep = 5 );
    void            SetRelative( bool bRelative );
    bool            IsRelative() const { return bRelative; }
    void            EnableNegativeMode() {bNegativeEnabled = true;}
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
