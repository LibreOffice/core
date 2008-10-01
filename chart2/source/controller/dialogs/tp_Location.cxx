/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tp_Location.cxx,v $
 * $Revision: 1.4.44.1 $
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
#include "precompiled_chart2.hxx"

#include "tp_Location.hxx"
#include "tp_Location.hrc"
#include "ResId.hxx"
#include "Strings.hrc"
#include "NoWarningThisInCTOR.hxx"

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

#ifdef LOCATION_PAGE

LocationTabPage::LocationTabPage( svt::OWizardMachine* pParent
        , const uno::Reference< XChartDocument >& xChartModel
        , const uno::Reference< uno::XComponentContext >& xContext )
        : OWizardPage( pParent, SchResId(TP_LOCATION) )
        , m_aFL_Table( this, SchResId( FL_TABLE ) )
        , m_aFT_Table( this, SchResId( FT_TABLE ) )
        , m_aLB_Table( this, SchResId( LB_TABLE ) )
        , m_xChartModel( xChartModel )
        , m_xCC( xContext )
{
    FreeResource();
    this->SetText( String( SchResId( STR_PAGE_CHART_LOCATION ) ) );
}

LocationTabPage::~LocationTabPage()
{
}

bool LocationTabPage::canAdvance() const
{
    return false;
}
#endif
//.............................................................................
} //namespace chart
//.............................................................................
