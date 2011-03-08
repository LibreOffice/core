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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#include "dlgsize.hrc"
#include "dlgsize.hxx"
#include "dbu_dlg.hrc"
#include "moduledbu.hxx"

//.........................................................................
namespace dbaui
{
//.........................................................................


#define DEF_ROW_HEIGHT  45
#define DEF_COL_WIDTH   227

DBG_NAME(DlgSize)
//==================================================================
DlgSize::DlgSize( Window* pParent, sal_Int32 nVal, sal_Bool bRow, sal_Int32 _nAlternativeStandard )
        :ModalDialog( pParent, ModuleRes(bRow ? DLG_ROWHEIGHT : DLG_COLWIDTH))
        ,m_nPrevValue(nVal)
        ,m_nStandard(bRow ? DEF_ROW_HEIGHT : DEF_COL_WIDTH)
        ,aFT_VALUE(this,    ModuleRes( FT_VALUE))
        ,aMF_VALUE(this,    ModuleRes( MF_VALUE))
        ,aCB_STANDARD(this, ModuleRes(CB_STANDARD))
        ,aPB_OK(this,       ModuleRes(PB_OK))
        ,aPB_CANCEL(this,   ModuleRes(PB_CANCEL))
        ,aPB_HELP(this,     ModuleRes(PB_HELP))
{
    DBG_CTOR(DlgSize,NULL);

    if ( _nAlternativeStandard > 0 )
        m_nStandard = _nAlternativeStandard;
    aCB_STANDARD.SetClickHdl(LINK(this,DlgSize,CbClickHdl));

    aMF_VALUE.EnableEmptyFieldValue(sal_True);
    sal_Bool bDefault = -1 == nVal;
    aCB_STANDARD.Check(bDefault);
    if (bDefault)
    {
        SetValue(m_nStandard);
        m_nPrevValue = m_nStandard;
    }
    LINK(this,DlgSize,CbClickHdl).Call(&aCB_STANDARD);

    FreeResource();
}

//------------------------------------------------------------------------------
DlgSize::~DlgSize()
{

    DBG_DTOR(DlgSize,NULL);
}

//------------------------------------------------------------------------------
void DlgSize::SetValue( sal_Int32 nVal )
{
    aMF_VALUE.SetValue(nVal, FUNIT_CM );
}

//------------------------------------------------------------------------------
sal_Int32 DlgSize::GetValue()
{
    if (aCB_STANDARD.IsChecked())
        return -1;
    return (sal_Int32)aMF_VALUE.GetValue( FUNIT_CM );
}

//------------------------------------------------------------------------------
IMPL_LINK( DlgSize, CbClickHdl, Button *, pButton )
{

    if( pButton == &aCB_STANDARD )
    {
        aMF_VALUE.Enable(!aCB_STANDARD.IsChecked());
        if (aCB_STANDARD.IsChecked())
        {
            m_nPrevValue = static_cast<sal_Int32>(aMF_VALUE.GetValue(FUNIT_CM));
                // don't use getValue as this will use aCB_STANDARD.to determine if we're standard
            aMF_VALUE.SetEmptyFieldValue();
        }
        else
        {
            SetValue( m_nPrevValue );
        }
    }
    return 0;
}
// -----------------------------------------------------------------------------
//.........................................................................
}   // namespace dbaui
//.........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
