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
#ifndef _DBAUI_DLGSIZE_HXX
#define _DBAUI_DLGSIZE_HXX

#include <vcl/dialog.hxx>

#include <vcl/field.hxx>

#include <vcl/button.hxx>

#include <vcl/fixed.hxx>

//.........................................................................
namespace dbaui
{
//.........................................................................

    class DlgSize : public ModalDialog
    {
    private:
        sal_Int32       m_nPrevValue, m_nStandard;
        void            SetValue( sal_Int32 nVal );

    protected:
        DECL_LINK( CbClickHdl, Button * );

        FixedText       aFT_VALUE;
        MetricField     aMF_VALUE;
        CheckBox        aCB_STANDARD;
        OKButton        aPB_OK;
        CancelButton    aPB_CANCEL;
        HelpButton      aPB_HELP;

    public:
        DlgSize( Window * pParent, sal_Int32 nVal, sal_Bool bRow, sal_Int32 _nAlternativeStandard = -1 );
        ~DlgSize();

        sal_Int32 GetValue();
    };
//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_DLGSIZE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
