/*************************************************************************
 *
 *  $RCSfile: dlgsize.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2002-05-24 12:45:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBAUI_DLGSIZE_HRC
#include "dlgsize.hrc"
#endif
#ifndef _DBAUI_DLGSIZE_HXX
#include "dlgsize.hxx"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................


#define DEF_ROW_HEIGHT  45
#define DEF_COL_WIDTH   227

//==================================================================
DlgSize::DlgSize( Window* pParent, sal_Int32 nVal, sal_Bool bRow, sal_Int32 _nAlternativeStandard )
        :ModalDialog( pParent, ModuleRes(bRow ? DLG_ROWHEIGHT : DLG_COLWIDTH))
        ,aFT_VALUE(this,    ResId( FT_VALUE))
        ,aMF_VALUE(this,    ResId( MF_VALUE))
        ,aCB_STANDARD(this, ResId(CB_STANDARD))
        ,aPB_OK(this,       ResId(PB_OK))
        ,aPB_CANCEL(this,   ResId(PB_CANCEL))
        ,aPB_HELP(this,     ResId(PB_HELP))
        ,m_nPrevValue(nVal)
        ,m_nStandard(bRow ? DEF_ROW_HEIGHT : DEF_COL_WIDTH)
{
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
            m_nPrevValue = aMF_VALUE.GetValue(FUNIT_CM);
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


