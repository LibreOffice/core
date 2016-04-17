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


#include "accessibility/extended/AccessibleBrowseBoxTableCell.hxx"
#include <svtools/accessibletableprovider.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>

namespace accessibility
{
    namespace
    {
        void checkIndex_Impl( sal_Int32 _nIndex, const OUString& _sText ) throw (css::lang::IndexOutOfBoundsException)
        {
            if ( _nIndex >= _sText.getLength() )
                throw css::lang::IndexOutOfBoundsException();
        }

        sal_Int32 getIndex_Impl( sal_Int32 _nRow, sal_uInt16 _nColumn, sal_uInt16 _nColumnCount )
        {
            return _nRow * _nColumnCount + _nColumn;
        }
    }
    using namespace ::com::sun::star::lang;
    using namespace utl;
    using namespace comphelper;
    using namespace ::com::sun::star::uno;
    using ::com::sun::star::accessibility::XAccessible;
    using namespace ::com::sun::star::accessibility;
    using namespace ::svt;


    // implementation of a table cell
    OUString AccessibleBrowseBoxTableCell::implGetText()
    {
        ensureIsAlive();
        return mpBrowseBox->GetAccessibleCellText( getRowPos(), static_cast< sal_uInt16 >( getColumnPos() ) );
    }

    css::lang::Locale AccessibleBrowseBoxTableCell::implGetLocale()
    {
        ensureIsAlive();
        return mpBrowseBox->GetAccessible()->getAccessibleContext()->getLocale();
    }

    void AccessibleBrowseBoxTableCell::implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex )
    {
        nStartIndex = 0;
        nEndIndex = 0;
    }

    AccessibleBrowseBoxTableCell::AccessibleBrowseBoxTableCell(const Reference<XAccessible >& _rxParent,
                                IAccessibleTableProvider& _rBrowseBox,
                                const css::uno::Reference< css::awt::XWindow >& _xFocusWindow,
                                sal_Int32 _nRowPos,
                                sal_uInt16 _nColPos,
                                sal_Int32 _nOffset )
        :AccessibleBrowseBoxCell( _rxParent, _rBrowseBox, _xFocusWindow, _nRowPos, _nColPos )
    {
        m_nOffset = ( OFFSET_DEFAULT == _nOffset ) ? (sal_Int32)BBINDEX_FIRSTCONTROL : _nOffset;
        sal_Int32 nIndex = getIndex_Impl( _nRowPos, _nColPos, _rBrowseBox.GetColumnCount() );
       setAccessibleName( _rBrowseBox.GetAccessibleObjectName( BBTYPE_TABLECELL, nIndex ) );
       setAccessibleDescription( _rBrowseBox.GetAccessibleObjectDescription( BBTYPE_TABLECELL, nIndex ) );
        // Need to register as event listener
        Reference< XComponent > xComponent(_rxParent, UNO_QUERY);
        if( xComponent.is() )
            xComponent->addEventListener(static_cast< XEventListener *> (this));
    }

    void AccessibleBrowseBoxTableCell::nameChanged( const OUString& rNewName, const OUString& rOldName )
    {
        implSetName( rNewName );
        Any aOldValue, aNewValue;
        aOldValue <<= rOldName;
        aNewValue <<= rNewName;
        commitEvent( AccessibleEventId::NAME_CHANGED, aNewValue, aOldValue );
    }

    // XInterface -------------------------------------------------------------

    /** Queries for a new interface. */
    css::uno::Any SAL_CALL AccessibleBrowseBoxTableCell::queryInterface( const css::uno::Type& rType )
        throw ( css::uno::RuntimeException, std::exception )
    {
        Any aRet = AccessibleBrowseBoxCell::queryInterface(rType);
        if ( !aRet.hasValue() )
            aRet = AccessibleTextHelper_BASE::queryInterface(rType);
        return aRet;
    }

    /** Aquires the object (calls acquire() on base class). */
    void SAL_CALL AccessibleBrowseBoxTableCell::acquire() throw ()
    {
        AccessibleBrowseBoxCell::acquire();
    }

    /** Releases the object (calls release() on base class). */
    void SAL_CALL AccessibleBrowseBoxTableCell::release() throw ()
    {
        AccessibleBrowseBoxCell::release();
    }

    css::awt::Rectangle SAL_CALL AccessibleBrowseBoxTableCell::getCharacterBounds( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );

        ensureIsAlive();
        if ( !implIsValidIndex( nIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        css::awt::Rectangle aRect;

        if ( mpBrowseBox )
        {
            aRect = AWTRectangle( mpBrowseBox->GetFieldCharacterBounds( getRowPos(), getColumnPos(), nIndex ) );
        }

        return aRect;
    }

    sal_Int32 SAL_CALL AccessibleBrowseBoxTableCell::getIndexAtPoint( const css::awt::Point& _aPoint ) throw (RuntimeException, std::exception)
    {
        //! TODO CTL bidi
        // OSL_FAIL("Need to be done by base class!");
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        ensureIsAlive();

        return mpBrowseBox->GetFieldIndexAtPoint( getRowPos(), getColumnPos(), VCLPoint( _aPoint ) );
    }

    /** @return
            The name of this class.
    */
    OUString SAL_CALL AccessibleBrowseBoxTableCell::getImplementationName()
        throw ( css::uno::RuntimeException, std::exception )
    {
        return OUString( "com.sun.star.comp.svtools.AccessibleBrowseBoxTableCell" );
    }

    /** @return  The count of visible children. */
    sal_Int32 SAL_CALL AccessibleBrowseBoxTableCell::getAccessibleChildCount()
        throw ( css::uno::RuntimeException, std::exception )
    {
        return 0;
    }

    /** @return  The XAccessible interface of the specified child. */
    css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        AccessibleBrowseBoxTableCell::getAccessibleChild( sal_Int32 )
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
    ::utl::AccessibleStateSetHelper* AccessibleBrowseBoxTableCell::implCreateStateSetHelper()
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );

        ::utl::AccessibleStateSetHelper* pStateSetHelper = new ::utl::AccessibleStateSetHelper;

        if( isAlive() )
        {
            // SHOWING done with mxParent
            if( implIsShowing() )
                pStateSetHelper->AddState( AccessibleStateType::SHOWING );

            mpBrowseBox->FillAccessibleStateSetForCell( *pStateSetHelper, getRowPos(), static_cast< sal_uInt16 >( getColumnPos() ) );
        }
        else
            pStateSetHelper->AddState( AccessibleStateType::DEFUNC );

        return pStateSetHelper;
    }


    // XAccessible ------------------------------------------------------------

    /** @return  The XAccessibleContext interface of this object. */
    Reference< XAccessibleContext > SAL_CALL AccessibleBrowseBoxTableCell::getAccessibleContext() throw ( RuntimeException, std::exception )
    {
        ensureIsAlive();
        return this;
    }

    // XAccessibleContext -----------------------------------------------------

    sal_Int32 SAL_CALL AccessibleBrowseBoxTableCell::getAccessibleIndexInParent()
            throw ( css::uno::RuntimeException, std::exception )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        ensureIsAlive();

        return /*BBINDEX_FIRSTCONTROL*/ m_nOffset + ( getRowPos() * mpBrowseBox->GetColumnCount() ) + getColumnPos();
    }

    sal_Int32 SAL_CALL AccessibleBrowseBoxTableCell::getCaretPosition(  ) throw (css::uno::RuntimeException, std::exception)
    {
        return -1;
    }
    sal_Bool SAL_CALL AccessibleBrowseBoxTableCell::setCaretPosition ( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );

        if ( !implIsValidRange( nIndex, nIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        return false;
    }
    sal_Unicode SAL_CALL AccessibleBrowseBoxTableCell::getCharacter( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        return OCommonAccessibleText::getCharacter( nIndex );
    }
    css::uno::Sequence< css::beans::PropertyValue > SAL_CALL AccessibleBrowseBoxTableCell::getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );

        OUString sText( implGetText() );

        if ( !implIsValidIndex( nIndex, sText.getLength() ) )
            throw IndexOutOfBoundsException();

        return css::uno::Sequence< css::beans::PropertyValue >();
    }
    sal_Int32 SAL_CALL AccessibleBrowseBoxTableCell::getCharacterCount(  ) throw (css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        return OCommonAccessibleText::getCharacterCount(  );
    }

    OUString SAL_CALL AccessibleBrowseBoxTableCell::getSelectedText(  ) throw (css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        return OCommonAccessibleText::getSelectedText(  );
    }
    sal_Int32 SAL_CALL AccessibleBrowseBoxTableCell::getSelectionStart(  ) throw (css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        return OCommonAccessibleText::getSelectionStart(  );
    }
    sal_Int32 SAL_CALL AccessibleBrowseBoxTableCell::getSelectionEnd(  ) throw (css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        return OCommonAccessibleText::getSelectionEnd(  );
    }
    sal_Bool SAL_CALL AccessibleBrowseBoxTableCell::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        if ( !implIsValidRange( nStartIndex, nEndIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        return false;
    }
    OUString SAL_CALL AccessibleBrowseBoxTableCell::getText(  ) throw (css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        return OCommonAccessibleText::getText(  );
    }
    OUString SAL_CALL AccessibleBrowseBoxTableCell::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        return OCommonAccessibleText::getTextRange( nStartIndex, nEndIndex );
    }
    css::accessibility::TextSegment SAL_CALL AccessibleBrowseBoxTableCell::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        return OCommonAccessibleText::getTextAtIndex( nIndex ,aTextType);
    }
    css::accessibility::TextSegment SAL_CALL AccessibleBrowseBoxTableCell::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        return OCommonAccessibleText::getTextBeforeIndex( nIndex ,aTextType);
    }
    css::accessibility::TextSegment SAL_CALL AccessibleBrowseBoxTableCell::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        return OCommonAccessibleText::getTextBehindIndex( nIndex ,aTextType);
    }
    sal_Bool SAL_CALL AccessibleBrowseBoxTableCell::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        OUString sText = implGetText();
        checkIndex_Impl( nStartIndex, sText );
        checkIndex_Impl( nEndIndex, sText );

        //!!! don't know how to put a string into the clipboard
        return false;
    }
    void AccessibleBrowseBoxTableCell::disposing( const EventObject& _rSource ) throw (RuntimeException, std::exception)
    {
            if ( _rSource.Source == mxParent )
            {
                    dispose();
            }
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
