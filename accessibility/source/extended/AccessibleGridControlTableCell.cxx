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


#include "accessibility/extended/AccessibleGridControlTableCell.hxx"
#include <svtools/accessibletable.hxx>
#include "accessibility/extended/AccessibleGridControl.hxx"
#include <tools/gen.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>

namespace accessibility
{
    namespace
    {
        // FIXME this is a copy'n'paste from
        // source/extended/AccessibleBrowseBoxTableCell.cxx, get rid of that...
        static void checkIndex_Impl( sal_Int32 _nIndex, const OUString& _sText ) throw (css::lang::IndexOutOfBoundsException)
        {
            if ( _nIndex >= _sText.getLength() )
                throw css::lang::IndexOutOfBoundsException();
        }
    }
    using namespace ::com::sun::star::lang;
    using namespace utl;
    using namespace comphelper;
    using ::accessibility::AccessibleGridControl;
    using namespace ::com::sun::star::uno;
    using ::com::sun::star::accessibility::XAccessible;
    using namespace ::com::sun::star::accessibility;
    using namespace ::svt;
    using namespace ::svt::table;



    // = AccessibleGridControlCell


    AccessibleGridControlCell::AccessibleGridControlCell(
            const Reference< XAccessible >& _rxParent, IAccessibleTable& _rTable,
            sal_Int32 _nRowPos, sal_uInt16 _nColPos, AccessibleTableControlObjType _eType )
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


    AccessibleGridControlCell::~AccessibleGridControlCell()
    {
    }


    void SAL_CALL AccessibleGridControlCell::grabFocus() throw ( RuntimeException, std::exception )
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

    AccessibleGridControlTableCell::AccessibleGridControlTableCell(const Reference<XAccessible >& _rxParent,
                                IAccessibleTable& _rTable,
                                sal_Int32 _nRowPos,
                                sal_uInt16 _nColPos,
                                AccessibleTableControlObjType  eObjType)
        :AccessibleGridControlCell( _rxParent, _rTable, _nRowPos, _nColPos, eObjType )
    {
    }

    // XInterface -------------------------------------------------------------

    /** Queries for a new interface. */
    css::uno::Any SAL_CALL AccessibleGridControlTableCell::queryInterface(
            const css::uno::Type& rType )
        throw ( css::uno::RuntimeException, std::exception )
    {
        Any aRet = AccessibleGridControlCell::queryInterface(rType);
        if ( !aRet.hasValue() )
            aRet = AccessibleTextHelper_BASE::queryInterface(rType);
        return aRet;
    }

    /** Aquires the object (calls acquire() on base class). */
    void SAL_CALL AccessibleGridControlTableCell::acquire() throw ()
    {
        AccessibleGridControlCell::acquire();
    }

    /** Releases the object (calls release() on base class). */
    void SAL_CALL AccessibleGridControlTableCell::release() throw ()
    {
        AccessibleGridControlCell::release();
    }

    css::awt::Rectangle SAL_CALL AccessibleGridControlTableCell::getCharacterBounds( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;

        ensureIsAlive();
        if ( !implIsValidIndex( nIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        return AWTRectangle( m_aTable.GetFieldCharacterBounds( getRowPos(), getColumnPos(), nIndex ) );
    }

    sal_Int32 SAL_CALL AccessibleGridControlTableCell::getIndexAtPoint( const css::awt::Point& _aPoint ) throw (RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;

        ensureIsAlive();

        return m_aTable.GetFieldIndexAtPoint( getRowPos(), getColumnPos(), VCLPoint( _aPoint ) );
    }

    /** @return
            The name of this class.
    */
    OUString SAL_CALL AccessibleGridControlTableCell::getImplementationName()
        throw ( css::uno::RuntimeException, std::exception )
    {
        return OUString( "com.sun.star.accessibility.AccessibleGridControlTableCell" );
    }

    /** @return  The count of visible children. */
    sal_Int32 SAL_CALL AccessibleGridControlTableCell::getAccessibleChildCount()
        throw ( css::uno::RuntimeException, std::exception )
    {
        return 0;
    }

    /** @return  The XAccessible interface of the specified child. */
    css::uno::Reference<
        css::accessibility::XAccessible > SAL_CALL
        AccessibleGridControlTableCell::getAccessibleChild( sal_Int32 )
            throw ( css::lang::IndexOutOfBoundsException,
                    css::uno::RuntimeException, std::exception )
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


    // XAccessible ------------------------------------------------------------

    /** @return  The XAccessibleContext interface of this object. */
    Reference< XAccessibleContext > SAL_CALL AccessibleGridControlTableCell::getAccessibleContext() throw ( RuntimeException, std::exception )
    {
        SolarMutexGuard g;

        ensureIsAlive();
        return this;
    }

    // XAccessibleContext -----------------------------------------------------

    sal_Int32 SAL_CALL AccessibleGridControlTableCell::getAccessibleIndexInParent()
            throw ( css::uno::RuntimeException, std::exception )
    {
        SolarMutexGuard aSolarGuard;

        ensureIsAlive();

        return ( getRowPos() * m_aTable.GetColumnCount() ) + getColumnPos();
    }

    sal_Int32 SAL_CALL AccessibleGridControlTableCell::getCaretPosition(  ) throw (css::uno::RuntimeException, std::exception)
    {
        return -1;
    }
    sal_Bool SAL_CALL AccessibleGridControlTableCell::setCaretPosition ( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;

        if ( !implIsValidRange( nIndex, nIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        return false;
    }
    sal_Unicode SAL_CALL AccessibleGridControlTableCell::getCharacter( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;

        return OCommonAccessibleText::getCharacter( nIndex );
    }
    css::uno::Sequence< css::beans::PropertyValue > SAL_CALL AccessibleGridControlTableCell::getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;

        OUString sText( implGetText() );

        if ( !implIsValidIndex( nIndex, sText.getLength() ) )
            throw IndexOutOfBoundsException();

        return css::uno::Sequence< css::beans::PropertyValue >();
    }
    sal_Int32 SAL_CALL AccessibleGridControlTableCell::getCharacterCount(  ) throw (css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;

        return OCommonAccessibleText::getCharacterCount(  );
    }

    OUString SAL_CALL AccessibleGridControlTableCell::getSelectedText(  ) throw (css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;

        return OCommonAccessibleText::getSelectedText(  );
    }
    sal_Int32 SAL_CALL AccessibleGridControlTableCell::getSelectionStart(  ) throw (css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;

        return OCommonAccessibleText::getSelectionStart(  );
    }
    sal_Int32 SAL_CALL AccessibleGridControlTableCell::getSelectionEnd(  ) throw (css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;

        return OCommonAccessibleText::getSelectionEnd(  );
    }
    sal_Bool SAL_CALL AccessibleGridControlTableCell::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;

        if ( !implIsValidRange( nStartIndex, nEndIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        return false;
    }
    OUString SAL_CALL AccessibleGridControlTableCell::getText(  ) throw (css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;

        return OCommonAccessibleText::getText(  );
    }
    OUString SAL_CALL AccessibleGridControlTableCell::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;

        return OCommonAccessibleText::getTextRange( nStartIndex, nEndIndex );
    }
    css::accessibility::TextSegment SAL_CALL AccessibleGridControlTableCell::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;

        return OCommonAccessibleText::getTextAtIndex( nIndex ,aTextType);
    }
    css::accessibility::TextSegment SAL_CALL AccessibleGridControlTableCell::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;

        return OCommonAccessibleText::getTextBeforeIndex( nIndex ,aTextType);
    }
    css::accessibility::TextSegment SAL_CALL AccessibleGridControlTableCell::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;

        return OCommonAccessibleText::getTextBehindIndex( nIndex ,aTextType);
    }
    sal_Bool SAL_CALL AccessibleGridControlTableCell::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;

        OUString sText = implGetText();
        checkIndex_Impl( nStartIndex, sText );
        checkIndex_Impl( nEndIndex, sText );

        //!!! don't know how to put a string into the clipboard
        return false;
    }

    Rectangle AccessibleGridControlTableCell::implGetBoundingBox()
    {
        vcl::Window* pParent = m_aTable.GetAccessibleParentWindow();
        DBG_ASSERT( pParent, "implGetBoundingBox - missing parent window" );
        Rectangle aGridRect = m_aTable.GetWindowExtentsRelative( pParent );
        sal_Int32 nIndex = getAccessibleIndexInParent();
        Rectangle aCellRect = m_aTable.calcCellRect(nIndex%m_aTable.GetColumnCount(), nIndex/m_aTable.GetColumnCount());
        long nX = aGridRect.Left() + aCellRect.Left();
        long nY = aGridRect.Top() + aCellRect.Top();
        Rectangle aCell( Point( nX, nY ), aCellRect.GetSize());
        return aCell;
    }

    Rectangle AccessibleGridControlTableCell::implGetBoundingBoxOnScreen()
    {
        Rectangle aGridRect = m_aTable.GetWindowExtentsRelative( nullptr );
        sal_Int32 nIndex = getAccessibleIndexInParent();
        Rectangle aCellRect = m_aTable.calcCellRect(nIndex%m_aTable.GetColumnCount(), nIndex/m_aTable.GetColumnCount());
        long nX = aGridRect.Left() + aCellRect.Left();
        long nY = aGridRect.Top() + aCellRect.Top();
        Rectangle aCell( Point( nX, nY ), aCellRect.GetSize());
        return aCell;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
