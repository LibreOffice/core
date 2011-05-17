/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright IBM Corporation 2009, 2010.
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

#include "vbaquerytable.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "sfx2/lnkbase.hxx"
#include "sfx2/linkmgr.hxx"
#include "arealink.hxx"
#include "vbarange.hxx"

using namespace com::sun::star;


ScVbaQueryTable::ScVbaQueryTable(const css::uno::Reference< ov::XHelperInterface >& /*xParent*/,
                                 const css::uno::Reference< css::uno::XComponentContext > & /*xContext*/,
                                 ScDocument *pDocument ,
                                 ScVbaRange *pParent
                                 )
//:QueryTable_Base(xParent, xContext)
{
    m_pDocument = pDocument;
    m_pParent = pParent;
}

ScVbaQueryTable::~ScVbaQueryTable()
{

}

::sal_Bool SAL_CALL
ScVbaQueryTable::Refresh( const ::com::sun::star::uno::Any& /*aBackgroundQuery*/ ) throw (::com::sun::star::uno::RuntimeException)
{

    //Get parent Info
    sal_Int32 nRow = m_pParent->getRow();
    sal_Int32 nClm = m_pParent->getColumn();
    sal_Int16 nTab = m_pParent->getWorksheet()->getIndex() - 1; //The vba index begin from 1.
    ScAddress crrRngAddr(nClm, nRow, nTab);

    //Get link info
    sfx2::LinkManager *pLinkMng = m_pDocument->GetLinkManager();
    const ::sfx2::SvBaseLinks &rLinks = pLinkMng->GetLinks();
    sal_uInt16 nCount = rLinks.Count();

    for (sal_uInt16 i=0; i<nCount; i++)
    {
        ::sfx2::SvBaseLink* pBase = *rLinks[i];
        if (pBase->ISA(ScAreaLink))
        {

            ScAreaLink *pAreaLink = (ScAreaLink *)pBase;
            const ScRange &destRange = pAreaLink->GetDestArea();
            if (destRange.In(crrRngAddr))
            {
                pBase->Update();
            }
        }
    }

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
