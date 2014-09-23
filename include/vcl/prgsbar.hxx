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

#ifndef INCLUDED_VCL_PRGSBAR_HXX
#define INCLUDED_VCL_PRGSBAR_HXX

#include <vcl/dllapi.h>
#include <vcl/window.hxx>

/*************************************************************************
 *
 * class ProgressBar
 *
 * this class is used to display the ProgressBar
 *
 * -----------------------------------------------------------------------
 *
 * WinBits
 *
 * WB_BORDER           border around the window
 * WB_3DLOOK           3D representation
 *
 * -----------------------------------------------------------------------
 *
 * Methodes
 *
 * Use SetValue() to set a percentage between 0 and 100. A value larger
 * than 100 will cause the last rectangle to start flashing
 *
 ************************************************************************/


// - WinBits -


#define WB_STDPROGRESSBAR       WB_BORDER


// - ProgressBar -


class VCL_DLLPUBLIC ProgressBar : public vcl::Window
{
private:
    Point               maPos;
    long                mnPrgsWidth;
    long                mnPrgsHeight;
    sal_uInt16              mnPercent;
    sal_uInt16              mnPercentCount;
    bool                mbCalcNew;

    using Window::ImplInit;
    SAL_DLLPRIVATE void             ImplInit();
    SAL_DLLPRIVATE void             ImplInitSettings( bool bFont, bool bForeground, bool bBackground );
    SAL_DLLPRIVATE void             ImplDrawProgress( sal_uInt16 nOldPerc, sal_uInt16 nNewPerc );

public:
                        ProgressBar( vcl::Window* pParent, WinBits nWinBits = WB_STDPROGRESSBAR );
                        virtual ~ProgressBar();

    virtual void        Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void        Resize() SAL_OVERRIDE;
    virtual void        StateChanged( StateChangedType nStateChange ) SAL_OVERRIDE;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
    virtual Size        GetOptimalSize() const SAL_OVERRIDE;

    void                SetValue( sal_uInt16 nNewPercent );
    sal_uInt16          GetValue() const { return mnPercent; }
};

#endif // INCLUDED_VCL_PRGSBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
