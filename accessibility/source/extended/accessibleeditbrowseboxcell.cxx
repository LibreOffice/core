/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_accessibility.hxx"
#include <accessibility/extended/accessibleeditbrowseboxcell.hxx>
#include <svtools/editbrowsebox.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>


// .................................................................................
namespace accessibility
{
// .................................................................................

    using namespace com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::awt;
    using namespace ::comphelper;
    using namespace ::svt;

    DBG_NAME( acc_EditBrowseBoxTableCell )
    // -----------------------------------------------------------------------------
    EditBrowseBoxTableCell::EditBrowseBoxTableCell(
                const com::sun::star::uno::Reference< XAccessible >& _rxParent,
                const com::sun::star::uno::Reference< XAccessible >& _rxOwningAccessible,
                const com::sun::star::uno::Reference< XAccessibleContext >& _xControlChild,
                IAccessibleTableProvider& _rBrowseBox,
                const Reference< XWindow >& _xFocusWindow,
                sal_Int32 _nRowPos,
                sal_uInt16 _nColPos)
        :AccessibleBrowseBoxCell( _rxParent, _rBrowseBox, _xFocusWindow, _nRowPos, _nColPos )
        ,OAccessibleContextWrapperHelper( ::comphelper::getProcessServiceFactory(), rBHelper, _xControlChild, _rxOwningAccessible, _rxParent )
    {
        DBG_CTOR( acc_EditBrowseBoxTableCell, NULL );

        aggregateProxy( m_refCount, *this );
    }

    // -----------------------------------------------------------------------------
    EditBrowseBoxTableCell::~EditBrowseBoxTableCell()
    {
        if ( !rBHelper.bDisposed )
        {
            acquire();  // to prevent duplicate dtor calls
            dispose();
        }

        DBG_DTOR( acc_EditBrowseBoxTableCell, NULL );
    }

    // -----------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL EditBrowseBoxTableCell::getImplementationName() throw ( ::com::sun::star::uno::RuntimeException )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.svtools.TableCellProxy" ) );
    }

    // -----------------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2( EditBrowseBoxTableCell, AccessibleBrowseBoxCell, OAccessibleContextWrapperHelper )

    // -----------------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( EditBrowseBoxTableCell, AccessibleBrowseBoxCell, OAccessibleContextWrapperHelper )

    // -----------------------------------------------------------------------------
    void EditBrowseBoxTableCell::notifyTranslatedEvent( const AccessibleEventObject& _rEvent ) throw (RuntimeException)
    {
        commitEvent( _rEvent.EventId, _rEvent.NewValue, _rEvent.OldValue );
    }

    // XAccessibleComponent
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL EditBrowseBoxTableCell::getForeground(  ) throw (RuntimeException)
    {
        SolarMethodGuard aGuard( *this );
        Reference< XAccessibleComponent > xAccComp( m_xInnerContext, UNO_QUERY );
        if ( xAccComp.is() )
            return xAccComp->getForeground();
        return 0;
    }

    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL EditBrowseBoxTableCell::getBackground(  ) throw (RuntimeException)
    {
        SolarMethodGuard aGuard( *this );
        Reference< XAccessibleComponent > xAccComp( m_xInnerContext, UNO_QUERY );
        if ( xAccComp.is() )
            return xAccComp->getBackground();
        return 0;
    }

    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL EditBrowseBoxTableCell::getAccessibleParent(  ) throw (RuntimeException)
    {
        return m_xParentAccessible;
    }

    // -----------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL EditBrowseBoxTableCell::getAccessibleDescription() throw ( RuntimeException )
    {
        SolarMethodGuard aGuard( *this );
        return m_xInnerContext->getAccessibleDescription();
    }

    // -----------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL EditBrowseBoxTableCell::getAccessibleName() throw ( RuntimeException )
    {
        SolarMethodGuard aGuard( *this );

        // TODO: localize this!
// IAccessible2 implementation, 2009
        //String sName = mpBrowseBox->GetColumnDescription( getColumnPos( ) );

        //if ( 0 == sName.Len() )
        //{
        //    sName = String::CreateFromAscii( "Column " );
            String  sName = String::CreateFromAscii( "Column " );
            sName += String::CreateFromInt32( getColumnPos( ) - 1 );
        //}
        sName += String::CreateFromAscii( ", Row " );
        sName += String::CreateFromInt32( getRowPos( ) );

        return ::rtl::OUString( sName );
    }

    // -----------------------------------------------------------------------------
    Reference< XAccessibleRelationSet > SAL_CALL EditBrowseBoxTableCell::getAccessibleRelationSet() throw ( RuntimeException )
    {
        SolarMethodGuard aGuard( *this );
        return OAccessibleContextWrapperHelper::getAccessibleRelationSet( );
    }

    // -----------------------------------------------------------------------------
    Reference<XAccessibleStateSet > SAL_CALL EditBrowseBoxTableCell::getAccessibleStateSet() throw ( RuntimeException )
    {
        SolarMethodGuard aGuard( *this );
        return m_xInnerContext->getAccessibleStateSet();
            // TODO: shouldn't we add an ACTIVE here? Isn't the EditBrowseBoxTableCell always ACTIVE?
    }

    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL EditBrowseBoxTableCell::getAccessibleChildCount(  ) throw (RuntimeException)
    {
        SolarMethodGuard aGuard( *this );
        return OAccessibleContextWrapperHelper::getAccessibleChildCount();
    }

    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL EditBrowseBoxTableCell::getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, RuntimeException)
    {
        SolarMethodGuard aGuard( *this );
        return OAccessibleContextWrapperHelper::getAccessibleChild( i );
    }

    // -----------------------------------------------------------------------------
    sal_Int16 SAL_CALL EditBrowseBoxTableCell::getAccessibleRole() throw ( RuntimeException )
    {
        SolarMethodGuard aGuard( *this );
        return m_xInnerContext->getAccessibleRole( );
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL EditBrowseBoxTableCell::dispose() throw( RuntimeException )
    {
        // simply disambiguate. Note that the OComponentHelper base in AccessibleBrowseBoxCell
        // will call our "disposing()", which will call "dispose()" on the OAccessibleContextWrapperHelper
        // so there is no need to do this here.
        AccessibleBrowseBoxCell::dispose();
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL EditBrowseBoxTableCell::disposing( const EventObject& _rSource ) throw (RuntimeException)
    {
        AccessibleBrowseBoxCell::disposing( _rSource );
        OAccessibleContextWrapperHelper::disposing( _rSource );
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL EditBrowseBoxTableCell::disposing()
    {
        SolarMethodGuard aGuard( *this, false );
        OAccessibleContextWrapperHelper::dispose();
            // TODO: do we need to dispose our inner object? The base class does this, but is it a good idea?
        AccessibleBrowseBoxCell::disposing();
    }
    // =============================================================================
    // = EditBrowseBoxTableCell
    // =============================================================================
    DBG_NAME( EditBrowseBoxTableCellAccess )
    // -----------------------------------------------------------------------------
    EditBrowseBoxTableCellAccess::EditBrowseBoxTableCellAccess(
            const Reference< XAccessible >& _rxParent, const Reference< XAccessible > _rxControlAccessible,
            const Reference< XWindow >& _rxFocusWindow,
            IAccessibleTableProvider& _rBrowseBox, sal_Int32 _nRowPos, sal_uInt16 _nColPos )
        :EditBrowseBoxTableCellAccess_Base( m_aMutex )
        ,m_xParent( _rxParent )
        ,m_xControlAccessible( _rxControlAccessible )
        ,m_xFocusWindow( _rxFocusWindow )
        ,m_pBrowseBox( &_rBrowseBox )
        ,m_nRowPos( _nRowPos )
        ,m_nColPos( _nColPos )
    {
        DBG_CTOR( EditBrowseBoxTableCellAccess, NULL );
    }
    // -----------------------------------------------------------------------------
    EditBrowseBoxTableCellAccess::~EditBrowseBoxTableCellAccess( )
    {
        DBG_DTOR( EditBrowseBoxTableCellAccess, NULL );
    }
    //--------------------------------------------------------------------
    Reference< XAccessibleContext > SAL_CALL EditBrowseBoxTableCellAccess::getAccessibleContext(  ) throw (RuntimeException)
    {
        if ( !m_pBrowseBox || !m_xControlAccessible.is() )
            throw DisposedException();
            Reference< XAccessibleContext > xMyContext( m_aContext );
            if ( !xMyContext.is() )
            {
            Reference< XAccessibleContext > xInnerContext = m_xControlAccessible->getAccessibleContext();
            Reference< XAccessible > xMe( this );

                xMyContext = new EditBrowseBoxTableCell( m_xParent, xMe, xInnerContext, *m_pBrowseBox, m_xFocusWindow, m_nRowPos, m_nColPos );
                m_aContext = xMyContext;
            }
        return xMyContext;
    }
    //--------------------------------------------------------------------
    void SAL_CALL EditBrowseBoxTableCellAccess::disposing()
    {
        // dispose our context, if it still alive
        Reference< XComponent > xMyContext( (Reference< XAccessibleContext >)m_aContext, UNO_QUERY );
        if ( xMyContext.is() )
        {
            try
            {
                xMyContext->dispose();
            }
            catch( const Exception& e )
            {
                (void)e;
                OSL_ENSURE( false, "EditBrowseBoxTableCellAccess::disposing: caught an exception while disposing the context!" );
            }
        }

        m_pBrowseBox = NULL;
        m_xControlAccessible.clear();
        m_aContext = Reference< XAccessibleContext >( );
        // NO dispose of the inner object there: it is the XAccessible of an window, and disposing
        // it would delete the respective VCL window
    }
// .................................................................................
} // namespace accessibility
// .................................................................................
