/*************************************************************************
 *
 *  $RCSfile: showcols.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-28 16:56:41 $
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

#ifndef _SVX_SHOWCOLS_HXX
#include "showcols.hxx"
#endif

#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SVX_DIALMGR_HXX //autogen
#include "dialmgr.hxx"
#endif

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#include "fmstatic.hxx" //CHINA001

IMPLEMENT_CONSTASCII_USTRING(CUIFM_PROP_HIDDEN, "Hidden");
IMPLEMENT_CONSTASCII_USTRING(CUIFM_PROP_LABEL,"Label");
//==========================================================================
//  FmShowColsDialog
//==========================================================================
DBG_NAME(FmShowColsDialog);
//--------------------------------------------------------------------------
FmShowColsDialog::FmShowColsDialog(Window* pParent)
    :ModalDialog(pParent, SVX_RES(RID_SVX_DLG_SHOWGRIDCOLUMNS))
    ,m_aList(this, ResId(1))
    ,m_aLabel(this, ResId(1))
    ,m_aOK(this, ResId(1))
    ,m_aCancel(this, ResId(1))
{
    DBG_CTOR(FmShowColsDialog,NULL);
    m_aList.EnableMultiSelection(sal_True);
    m_aOK.SetClickHdl( LINK( this, FmShowColsDialog, OnClickedOk ) );

    FreeResource();
}

//--------------------------------------------------------------------------
FmShowColsDialog::~FmShowColsDialog()
{
    DBG_DTOR(FmShowColsDialog,NULL);
}

//--------------------------------------------------------------------------
IMPL_LINK( FmShowColsDialog, OnClickedOk, Button*, pButton )
{
    DBG_ASSERT(m_xColumns.is(), "FmShowColsDialog::OnClickedOk : you should call SetColumns before executing the dialog !");
    if (m_xColumns.is())
    {
        ::com::sun::star::uno::Any aCol;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xCol;
        for (sal_uInt16 i=0; i<m_aList.GetSelectEntryCount(); ++i)
        {
            m_xColumns->getByIndex(reinterpret_cast<sal_Int64>(m_aList.GetEntryData(m_aList.GetSelectEntryPos(i)))) >>= xCol;
            if (xCol.is())
            {
                try
                {
                    //CHINA001 xCol->setPropertyValue(::svxform::FM_PROP_HIDDEN, ::cppu::bool2any(sal_False));
                    xCol->setPropertyValue(CUIFM_PROP_HIDDEN, ::cppu::bool2any(sal_False));
                }
                catch(...)
                {
                    DBG_ERROR("FmShowColsDialog::OnClickedOk Exception occured!");
                }
            }
        }
    }

    EndDialog(RET_OK);
    return 0L;
}

//--------------------------------------------------------------------------
void FmShowColsDialog::SetColumns(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer>& xCols)
{
    DBG_ASSERT(xCols.is(), "FmShowColsDialog::SetColumns : invalid columns !");
    if (!xCols.is())
        return;
    m_xColumns = xCols.get();

    m_aList.Clear();

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>  xCurCol;
    String sCurName;
    for (sal_uInt16 i=0; i<xCols->getCount(); ++i)
    {
        sCurName.Erase();
        ::cppu::extractInterface(xCurCol, xCols->getByIndex(i));
        sal_Bool bIsHidden = sal_False;
        try
        {
            //CHINA001 ::com::sun::star::uno::Any aHidden = xCurCol->getPropertyValue(::svxform::FM_PROP_HIDDEN);
            ::com::sun::star::uno::Any aHidden = xCurCol->getPropertyValue(CUIFM_PROP_HIDDEN);
            bIsHidden = ::comphelper::getBOOL(aHidden);

            ::rtl::OUString sName;
            //CHINA001 xCurCol->getPropertyValue(::svxform::FM_PROP_LABEL) >>= sName;

            xCurCol->getPropertyValue(CUIFM_PROP_LABEL) >>= sName;
            sCurName = (const sal_Unicode*)sName;
        }
        catch(...)
        {
            DBG_ERROR("FmShowColsDialog::SetColumns Exception occured!");
        }

        // if the col is hidden, put it into the list
        if (bIsHidden)
            m_aList.SetEntryData( m_aList.InsertEntry(sCurName), reinterpret_cast<void*>((sal_Int64)i) );
    }
}

