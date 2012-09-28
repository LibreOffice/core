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
        static void checkIndex_Impl( sal_Int32 _nIndex, const OUString& _sText ) throw (::com::sun::star::lang::IndexOutOfBoundsException)
        {
            if ( _nIndex >= _sText.getLength() )
                throw ::com::sun::star::lang::IndexOutOfBoundsException();
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


    // =============================================================================
    // = AccessibleGridControlCell
    // =============================================================================
    // -----------------------------------------------------------------------------
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

    // -----------------------------------------------------------------------------
    AccessibleGridControlCell::~AccessibleGridControlCell()
    {
    }

    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleGridControlCell::grabFocus() throw ( RuntimeException )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        m_aTable.GoToCell( m_nColPos, m_nRowPos );
    }
    //// -----------------------------------------------------------------------------
    // implementation of a table cell
    OUString AccessibleGridControlTableCell::implGetText()
    {
        ensureIsAlive();
        return m_aTable.GetAccessibleCellText( getRowPos(),  getColumnPos() );
    }

    ::com::sun::star::lang::Locale AccessibleGridControlTableCell::implGetLocale()
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
    ::com::sun::star::uno::Any SAL_CALL AccessibleGridControlTableCell::queryInterface(
            const ::com::sun::star::uno::Type& rType )
        throw ( ::com::sun::star::uno::RuntimeException )
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

    ::com::sun::star::awt::Rectangle SAL_CALL AccessibleGridControlTableCell::getCharacterBounds( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );

        ensureIsAlive();
        if ( !implIsValidIndex( nIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        ::com::sun::star::awt::Rectangle aRect;

        if ( &m_aTable )
            aRect = AWTRectangle( m_aTable.GetFieldCharacterBounds( getRowPos(), getColumnPos(), nIndex ) );
        return aRect;
    }

    sal_Int32 SAL_CALL AccessibleGridControlTableCell::getIndexAtPoint( const ::com::sun::star::awt::Point& _aPoint ) throw (RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        ensureIsAlive();

        return m_aTable.GetFieldIndexAtPoint( getRowPos(), getColumnPos(), VCLPoint( _aPoint ) );
    }

    /** @return
            The name of this class.
    */
    OUString SAL_CALL AccessibleGridControlTableCell::getImplementationName()
        throw ( ::com::sun::star::uno::RuntimeException )
    {
        return OUString( "com.sun.star.accessibility.AccessibleGridControlTableCell" );
    }

    /** @return  The count of visible children. */
    sal_Int32 SAL_CALL AccessibleGridControlTableCell::getAccessibleChildCount()
        throw ( ::com::sun::star::uno::RuntimeException )
    {
        return 0;
    }

    /** @return  The XAccessible interface of the specified child. */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
        AccessibleGridControlTableCell::getAccessibleChild( sal_Int32 )
            throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                    ::com::sun::star::uno::RuntimeException )
    {
        throw ::com::sun::star::lang::IndexOutOfBoundsException();
    }

    /** Creates a new AccessibleStateSetHelper and fills it with states of the
        current object.
        @return
            A filled AccessibleStateSetHelper.
    */
    ::utl::AccessibleStateSetHelper* AccessibleGridControlTableCell::implCreateStateSetHelper()
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );

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
    Reference< XAccessibleContext > SAL_CALL AccessibleGridControlTableCell::getAccessibleContext() throw ( RuntimeException )
    {
        ensureIsAlive();
        return this;
    }

    // XAccessibleContext -----------------------------------------------------

    sal_Int32 SAL_CALL AccessibleGridControlTableCell::getAccessibleIndexInParent()
            throw ( ::com::sun::star::uno::RuntimeException )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        ensureIsAlive();

        return ( getRowPos() * m_aTable.GetColumnCount() ) + getColumnPos();
    }

    sal_Int32 SAL_CALL AccessibleGridControlTableCell::getCaretPosition(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        return -1;
    }
    sal_Bool SAL_CALL AccessibleGridControlTableCell::setCaretPosition ( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );

        if ( !implIsValidRange( nIndex, nIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        return sal_False;
    }
    sal_Unicode SAL_CALL AccessibleGridControlTableCell::getCharacter( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        return OCommonAccessibleText::getCharacter( nIndex );
    }
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL AccessibleGridControlTableCell::getCharacterAttributes( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< OUString >& ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );

        OUString sText( implGetText() );

        if ( !implIsValidIndex( nIndex, sText.getLength() ) )
            throw IndexOutOfBoundsException();

        return ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >();
    }
    sal_Int32 SAL_CALL AccessibleGridControlTableCell::getCharacterCount(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        return OCommonAccessibleText::getCharacterCount(  );
    }

    OUString SAL_CALL AccessibleGridControlTableCell::getSelectedText(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        return OCommonAccessibleText::getSelectedText(  );
    }
    sal_Int32 SAL_CALL AccessibleGridControlTableCell::getSelectionStart(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        return OCommonAccessibleText::getSelectionStart(  );
    }
    sal_Int32 SAL_CALL AccessibleGridControlTableCell::getSelectionEnd(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        return OCommonAccessibleText::getSelectionEnd(  );
    }
    sal_Bool SAL_CALL AccessibleGridControlTableCell::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        if ( !implIsValidRange( nStartIndex, nEndIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        return sal_False;
    }
    OUString SAL_CALL AccessibleGridControlTableCell::getText(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        return OCommonAccessibleText::getText(  );
    }
    OUString SAL_CALL AccessibleGridControlTableCell::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        return OCommonAccessibleText::getTextRange( nStartIndex, nEndIndex );
    }
    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleGridControlTableCell::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        return OCommonAccessibleText::getTextAtIndex( nIndex ,aTextType);
    }
    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleGridControlTableCell::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        return OCommonAccessibleText::getTextBeforeIndex( nIndex ,aTextType);
    }
    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleGridControlTableCell::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        return OCommonAccessibleText::getTextBehindIndex( nIndex ,aTextType);
    }
    sal_Bool SAL_CALL AccessibleGridControlTableCell::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        OUString sText = implGetText();
        checkIndex_Impl( nStartIndex, sText );
        checkIndex_Impl( nEndIndex, sText );

        //!!! don't know how to put a string into the clipboard
        return sal_False;
    }

    Rectangle AccessibleGridControlTableCell::implGetBoundingBox()
    {
        Window* pParent = m_aTable.GetAccessibleParentWindow();
        DBG_ASSERT( pParent, "implGetBoundingBox - missing parent window" );
        Rectangle aGridRect = m_aTable.GetWindowExtentsRelative( pParent );
        sal_Int32 nIndex = getAccessibleIndexInParent();
        Rectangle aCellRect = m_aTable.calcCellRect(nIndex%m_aTable.GetColumnCount(), nIndex/m_aTable.GetColumnCount());
        long nX = aGridRect.Left() + aCellRect.Left();
        long nY = aGridRect.Top() + aCellRect.Top();
        Rectangle aCell( Point( nX, nY ), aCellRect.GetSize());
        return aCell;
    }
    // -----------------------------------------------------------------------------
    Rectangle AccessibleGridControlTableCell::implGetBoundingBoxOnScreen()
    {
        Rectangle aGridRect = m_aTable.GetWindowExtentsRelative( NULL );
        sal_Int32 nIndex = getAccessibleIndexInParent();
        Rectangle aCellRect = m_aTable.calcCellRect(nIndex%m_aTable.GetColumnCount(), nIndex/m_aTable.GetColumnCount());
        long nX = aGridRect.Left() + aCellRect.Left();
        long nY = aGridRect.Top() + aCellRect.Top();
        Rectangle aCell( Point( nX, nY ), aCellRect.GetSize());
        return aCell;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
