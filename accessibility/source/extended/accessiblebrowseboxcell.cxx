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

#include "accessibility/extended/accessiblebrowseboxcell.hxx"
#include <svtools/accessibletableprovider.hxx>

// .................................................................................
namespace accessibility
{
// .................................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::accessibility;
    using namespace ::svt;

    // =============================================================================
    // = AccessibleBrowseBoxCell
    // =============================================================================
    AccessibleBrowseBoxCell::AccessibleBrowseBoxCell(
            const Reference< XAccessible >& _rxParent, IAccessibleTableProvider& _rBrowseBox,
            const Reference< XWindow >& _xFocusWindow,
            sal_Int32 _nRowPos, sal_uInt16 _nColPos, AccessibleBrowseBoxObjType _eType )
        :AccessibleBrowseBoxBase( _rxParent, _rBrowseBox, _xFocusWindow, _eType )
        ,m_nRowPos( _nRowPos )
        ,m_nColPos( _nColPos )
    {
        // set accessible name here, because for that we need the position of the cell
        // and so the base class isn't capable of doing this
        sal_Int32 nPos = _nRowPos * _rBrowseBox.GetColumnCount() + _nColPos;
        OUString aAccName = _rBrowseBox.GetAccessibleObjectName( BBTYPE_TABLECELL, nPos );
        implSetName( aAccName );
    }

    // -----------------------------------------------------------------------------
    AccessibleBrowseBoxCell::~AccessibleBrowseBoxCell()
    {
    }

    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleBrowseBoxCell::grabFocus() throw ( RuntimeException )
    {
        SolarMethodGuard aGuard( *this );
        mpBrowseBox->GoToCell( m_nRowPos, m_nColPos );
    }
    // -----------------------------------------------------------------------------
    ::Rectangle AccessibleBrowseBoxCell::implGetBoundingBox()
    {
        return mpBrowseBox->GetFieldRectPixelAbs( m_nRowPos, m_nColPos, sal_False, sal_False );
    }

    // -----------------------------------------------------------------------------
    ::Rectangle AccessibleBrowseBoxCell::implGetBoundingBoxOnScreen()
    {
        return mpBrowseBox->GetFieldRectPixelAbs( m_nRowPos, m_nColPos, sal_False );
    }

// .................................................................................
}   // namespace accessibility
// .................................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
