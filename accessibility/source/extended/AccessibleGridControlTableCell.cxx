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

#include <extended/AccessibleGridControlTableCell.hxx>
#include <vcl/accessibletable.hxx>
#include <tools/gen.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

namespace accessibility
{
    namespace
    {
        // FIXME this is a copy'n'paste from
        // source/extended/AccessibleBrowseBoxTableCell.cxx, get rid of that...
        /// @throws css::lang::IndexOutOfBoundsException
        void checkIndex_Impl( sal_Int32 _nIndex, const OUString& _sText )
        {
            if ( _nIndex >= _sText.getLength() )
                throw css::lang::IndexOutOfBoundsException();
        }
    }
    using namespace ::com::sun::star::lang;
    using namespace utl;
    using namespace comphelper;
    using namespace ::com::sun::star::uno;
    using ::com::sun::star::accessibility::XAccessible;
    using namespace ::com::sun::star::accessibility;
    using namespace ::vcl;
    using namespace ::vcl::table;


    // = AccessibleGridControlCell


    AccessibleGridControlCell::AccessibleGridControlCell(
            const css::uno::Reference< css::accessibility::XAccessible >& _rxParent, ::vcl::table::IAccessibleTable& _rTable,
            sal_Int32 _nRowPos, sal_uInt16 _nColPos, ::vcl::table::AccessibleTableControlObjType _eType )
        :AccessibleGridControlBase( _rxParent, _rTable, _eType )
        ,m_nRowPos( _nRowPos )
        ,m_nColPos( _nColPos )
    {
        // set accessible name here, because for that we need the position of the cell
        // and so the base class isn't capable of doing this
        OUString aAccName;
        if(_eType == TCTYPE_TABLECELL)
            aAccName = _rTable.GetAccessibleObjectName( TCTYPE_TABLECELL, _nRowPos, _nColPos );
        else if(_eType == TCTYPE_ROWHEADERCELL)
            aAccName = _rTable.GetAccessibleObjectName( TCTYPE_ROWHEADERCELL, _nRowPos, 0 );
        else if(_eType == TCTYPE_COLUMNHEADERCELL)
            aAccName = _rTable.GetAccessibleObjectName( TCTYPE_COLUMNHEADERCELL, 0, _nRowPos );
        implSetName( aAccName );
    }

    void SAL_CALL AccessibleGridControlCell::grabFocus()
    {
        SolarMutexGuard aSolarGuard;

        m_aTable.GoToCell( m_nColPos, m_nRowPos );
    }

    // implementation of a table cell
    OUString AccessibleGridControlTableCell::implGetText()
    {
        ensureIsAlive();
        return m_aTable.GetAccessibleCellText( getRowPos(),  getColumnPos() );
    }

    css::lang::Locale AccessibleGridControlTableCell::implGetLocale()
    {
        ensureIsAlive();
        return m_aTable.GetAccessible()->getAccessibleContext()->getLocale();
    }

    void AccessibleGridControlTableCell::implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex )
    {
        nStartIndex = 0;
        nEndIndex = 0;
    }

    AccessibleGridControlTableCell::AccessibleGridControlTableCell(const css::uno::Reference<XAccessible >& _rxParent,
                                ::vcl::table::IAccessibleTable& _rTable,
                                sal_Int32 _nRowPos,
                                sal_uInt16 _nColPos)
        :AccessibleGridControlCell( _rxParent, _rTable, _nRowPos, _nColPos, TCTYPE_TABLECELL )
    {
    }

    // XInterface

    /** Queries for a new interface. */
    css::uno::Any SAL_CALL AccessibleGridControlTableCell::queryInterface(
            const css::uno::Type& rType )
    {
        Any aRet = AccessibleGridControlCell::queryInterface(rType);
        if ( !aRet.hasValue() )
            aRet = AccessibleTextHelper_BASE::queryInterface(rType);
        return aRet;
    }

    /** Acquires the object (calls acquire() on base class). */
    void SAL_CALL AccessibleGridControlTableCell::acquire() throw ()
    {
        AccessibleGridControlCell::acquire();
    }

    /** Releases the object (calls release() on base class). */
    void SAL_CALL AccessibleGridControlTableCell::release() throw ()
    {
        AccessibleGridControlCell::release();
    }

    css::awt::Rectangle SAL_CALL AccessibleGridControlTableCell::getCharacterBounds( sal_Int32 nIndex )
    {
        SolarMutexGuard aSolarGuard;

        ensureIsAlive();
        if ( !implIsValidIndex( nIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        return AWTRectangle( m_aTable.GetFieldCharacterBounds( getRowPos(), getColumnPos(), nIndex ) );
    }

    sal_Int32 SAL_CALL AccessibleGridControlTableCell::getIndexAtPoint( const css::awt::Point& _aPoint )
    {
        SolarMutexGuard aSolarGuard;

        ensureIsAlive();

        return m_aTable.GetFieldIndexAtPoint( getRowPos(), getColumnPos(), VCLPoint( _aPoint ) );
    }

    /** @return
            The name of this class.
    */
    OUString SAL_CALL AccessibleGridControlTableCell::getImplementationName()
    {
        return OUString( "com.sun.star.accessibility.AccessibleGridControlTableCell" );
    }

    /** @return  The count of visible children. */
    sal_Int32 SAL_CALL AccessibleGridControlTableCell::getAccessibleChildCount()
    {
        return 0;
    }

    /** @return  The css::accessibility::XAccessible interface of the specified child. */
    css::uno::Reference< css::accessibility::XAccessible > SAL_CALL AccessibleGridControlTableCell::getAccessibleChild( sal_Int32 )
    {
        throw css::lang::IndexOutOfBoundsException();
    }

    /** Creates a new AccessibleStateSetHelper and fills it with states of the
        current object.
        @return
            A filled AccessibleStateSetHelper.
    */
    ::utl::AccessibleStateSetHelper* AccessibleGridControlTableCell::implCreateStateSetHelper()
    {
        ::utl::AccessibleStateSetHelper* pStateSetHelper = new ::utl::AccessibleStateSetHelper;

        if( isAlive() )
        {
            // SHOWING done with mxParent
            if( implIsShowing() )
                pStateSetHelper->AddState( AccessibleStateType::SHOWING );

            m_aTable.FillAccessibleStateSetForCell( *pStateSetHelper, getRowPos(), static_cast< sal_uInt16 >( getColumnPos() ) );
        }
        else
            pStateSetHelper->AddState( AccessibleStateType::DEFUNC );

        return pStateSetHelper;
    }


    // css::accessibility::XAccessible

    /** @return  The css::accessibility::XAccessibleContext interface of this object. */
    css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL AccessibleGridControlTableCell::getAccessibleContext()
    {
        SolarMutexGuard g;

        ensureIsAlive();
        return this;
    }

    // css::accessibility::XAccessibleContext

    sal_Int32 SAL_CALL AccessibleGridControlTableCell::getAccessibleIndexInParent()
    {
        SolarMutexGuard aSolarGuard;

        ensureIsAlive();

        return ( getRowPos() * m_aTable.GetColumnCount() ) + getColumnPos();
    }

    sal_Int32 SAL_CALL AccessibleGridControlTableCell::getCaretPosition(  )
    {
        return -1;
    }
    sal_Bool SAL_CALL AccessibleGridControlTableCell::setCaretPosition ( sal_Int32 nIndex )
    {
        SolarMutexGuard aSolarGuard;

        if ( !implIsValidRange( nIndex, nIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        return false;
    }
    sal_Unicode SAL_CALL AccessibleGridControlTableCell::getCharacter( sal_Int32 nIndex )
    {
        SolarMutexGuard aSolarGuard;

        return OCommonAccessibleText::implGetCharacter( implGetText(), nIndex );
    }
    css::uno::Sequence< css::beans::PropertyValue > SAL_CALL AccessibleGridControlTableCell::getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& )
    {
        SolarMutexGuard aSolarGuard;

        OUString sText( implGetText() );

        if ( !implIsValidIndex( nIndex, sText.getLength() ) )
            throw IndexOutOfBoundsException();

        return css::uno::Sequence< css::beans::PropertyValue >();
    }
    sal_Int32 SAL_CALL AccessibleGridControlTableCell::getCharacterCount(  )
    {
        SolarMutexGuard aSolarGuard;

        return implGetText().getLength();
    }

    OUString SAL_CALL AccessibleGridControlTableCell::getSelectedText(  )
    {
        SolarMutexGuard aSolarGuard;

        return OUString();
    }
    sal_Int32 SAL_CALL AccessibleGridControlTableCell::getSelectionStart(  )
    {
        SolarMutexGuard aSolarGuard;

        return 0;
    }
    sal_Int32 SAL_CALL AccessibleGridControlTableCell::getSelectionEnd(  )
    {
        SolarMutexGuard aSolarGuard;

        return 0;
    }
    sal_Bool SAL_CALL AccessibleGridControlTableCell::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    {
        SolarMutexGuard aSolarGuard;

        if ( !implIsValidRange( nStartIndex, nEndIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        return false;
    }
    OUString SAL_CALL AccessibleGridControlTableCell::getText(  )
    {
        SolarMutexGuard aSolarGuard;

        return implGetText(  );
    }
    OUString SAL_CALL AccessibleGridControlTableCell::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    {
        SolarMutexGuard aSolarGuard;

        return OCommonAccessibleText::implGetTextRange( implGetText(), nStartIndex, nEndIndex );
    }
    css::accessibility::TextSegment SAL_CALL AccessibleGridControlTableCell::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType )
    {
        SolarMutexGuard aSolarGuard;

        return OCommonAccessibleText::getTextAtIndex( nIndex ,aTextType);
    }
    css::accessibility::TextSegment SAL_CALL AccessibleGridControlTableCell::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType )
    {
        SolarMutexGuard aSolarGuard;

        return OCommonAccessibleText::getTextBeforeIndex( nIndex ,aTextType);
    }
    css::accessibility::TextSegment SAL_CALL AccessibleGridControlTableCell::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType )
    {
        SolarMutexGuard aSolarGuard;

        return OCommonAccessibleText::getTextBehindIndex( nIndex ,aTextType);
    }
    sal_Bool SAL_CALL AccessibleGridControlTableCell::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    {
        SolarMutexGuard aSolarGuard;

        OUString sText = implGetText();
        checkIndex_Impl( nStartIndex, sText );
        checkIndex_Impl( nEndIndex, sText );

        //!!! don't know how to put a string into the clipboard
        return false;
    }

    tools::Rectangle AccessibleGridControlTableCell::implGetBoundingBox()
    {
        vcl::Window* pParent = m_aTable.GetAccessibleParentWindow();
        DBG_ASSERT( pParent, "implGetBoundingBox - missing parent window" );
        tools::Rectangle aGridRect = m_aTable.GetWindowExtentsRelative( pParent );
        sal_Int32 nIndex = getAccessibleIndexInParent();
        tools::Rectangle aCellRect = m_aTable.calcCellRect(nIndex%m_aTable.GetColumnCount(), nIndex/m_aTable.GetColumnCount());
        long nX = aGridRect.Left() + aCellRect.Left();
        long nY = aGridRect.Top() + aCellRect.Top();
        tools::Rectangle aCell( Point( nX, nY ), aCellRect.GetSize());
        return aCell;
    }

    tools::Rectangle AccessibleGridControlTableCell::implGetBoundingBoxOnScreen()
    {
        tools::Rectangle aGridRect = m_aTable.GetWindowExtentsRelative( nullptr );
        sal_Int32 nIndex = getAccessibleIndexInParent();
        tools::Rectangle aCellRect = m_aTable.calcCellRect(nIndex%m_aTable.GetColumnCount(), nIndex/m_aTable.GetColumnCount());
        long nX = aGridRect.Left() + aCellRect.Left();
        long nY = aGridRect.Top() + aCellRect.Top();
        tools::Rectangle aCell( Point( nX, nY ), aCellRect.GetSize());
        return aCell;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
