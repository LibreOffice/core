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


#include <extended/AccessibleBrowseBoxHeaderCell.hxx>
#include <vcl/accessibletableprovider.hxx>
#include <extended/AccessibleBrowseBox.hxx>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

namespace accessibility
{
    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::uno;

AccessibleBrowseBoxHeaderCell::AccessibleBrowseBoxHeaderCell(sal_Int32 _nColumnRowId,
                                  const Reference< XAccessible >& rxParent,
                                  vcl::IAccessibleTableProvider& rBrowseBox,
                                  const css::uno::Reference< css::awt::XWindow >& _xFocusWindow,
                                  vcl::AccessibleBrowseBoxObjType eObjType)
: BrowseBoxAccessibleElement(rxParent,
                          rBrowseBox,
                          _xFocusWindow,
                          eObjType,
                          rBrowseBox.GetAccessibleObjectName( eObjType ,_nColumnRowId),
                          rBrowseBox.GetAccessibleObjectDescription( eObjType ,_nColumnRowId))
, m_nColumnRowId(_nColumnRowId)
{
}
/** Creates a new AccessibleStateSetHelper and fills it with states of the
    current object.
    @return
        A filled AccessibleStateSetHelper.
*/
::utl::AccessibleStateSetHelper* AccessibleBrowseBoxHeaderCell::implCreateStateSetHelper()
{
    SolarMethodGuard aGuard( getMutex() );

    ::utl::AccessibleStateSetHelper*
        pStateSetHelper = new ::utl::AccessibleStateSetHelper;

    if( isAlive() )
    {
        // SHOWING done with mxParent
        if( implIsShowing() )
            pStateSetHelper->AddState( AccessibleStateType::SHOWING );

        mpBrowseBox->FillAccessibleStateSet( *pStateSetHelper, getType() );
        pStateSetHelper->AddState( AccessibleStateType::VISIBLE );
        pStateSetHelper->AddState( AccessibleStateType::FOCUSABLE );
        pStateSetHelper->AddState( AccessibleStateType::TRANSIENT );
        pStateSetHelper->AddState( AccessibleStateType::SELECTABLE );

        bool bSelected = isRowBarCell() ? mpBrowseBox->IsRowSelected(m_nColumnRowId) : mpBrowseBox->IsColumnSelected(m_nColumnRowId);
        if ( bSelected )
            pStateSetHelper->AddState( AccessibleStateType::SELECTED );
    }
    else
        pStateSetHelper->AddState( AccessibleStateType::DEFUNC );

    return pStateSetHelper;
}

/** @return
        The count of visible children.
*/
sal_Int32 SAL_CALL AccessibleBrowseBoxHeaderCell::getAccessibleChildCount()
{
    return 0;
}


/** @return
        The XAccessible interface of the specified child.
*/
Reference<XAccessible > SAL_CALL AccessibleBrowseBoxHeaderCell::getAccessibleChild( sal_Int32 )
{
    throw IndexOutOfBoundsException();
}


/** Grabs the focus to the column header. */
void SAL_CALL AccessibleBrowseBoxHeaderCell::grabFocus()
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    if ( isRowBarCell() )
        mpBrowseBox->SelectRow(m_nColumnRowId);
    else
        mpBrowseBox->SelectColumn(static_cast<sal_uInt16>(m_nColumnRowId)); //!!!
}

/** @return
        The name of this class.
*/
OUString SAL_CALL AccessibleBrowseBoxHeaderCell::getImplementationName()
{
    return OUString( "com.sun.star.comp.svtools.AccessibleBrowseBoxHeaderCell" );
}

namespace
{
    tools::Rectangle getRectangle(vcl::IAccessibleTableProvider* _pBrowseBox,sal_Int32 _nRowColIndex, bool _bOnScreen,bool _bRowBar)
    {
        sal_Int32 nRow  = 0;
        sal_uInt16 nCol =  static_cast<sal_uInt16>(_nRowColIndex);
        if ( _bRowBar )
        {
            nRow = _nRowColIndex + 1;
            nCol = 0;
        }

        tools::Rectangle aRet(_pBrowseBox->GetFieldRectPixelAbs( nRow , nCol, true, _bOnScreen));
        return tools::Rectangle(aRet.TopLeft() - Point(0,aRet.GetHeight()),aRet.GetSize());
    }
}

tools::Rectangle AccessibleBrowseBoxHeaderCell::implGetBoundingBox()
{
    return getRectangle(mpBrowseBox,m_nColumnRowId,false,isRowBarCell());
}


tools::Rectangle AccessibleBrowseBoxHeaderCell::implGetBoundingBoxOnScreen()
{
    return getRectangle(mpBrowseBox,m_nColumnRowId,true,isRowBarCell());
}

sal_Int32 SAL_CALL AccessibleBrowseBoxHeaderCell::getAccessibleIndexInParent()
{
    ::osl::MutexGuard aGuard( getMutex() );
    ensureIsAlive();
    sal_Int32 nIndex = m_nColumnRowId;
    if ( mpBrowseBox->HasRowHeader() )
        --nIndex;
    return nIndex;
}

} // namespace accessibility


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
