/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: specialdispatchers.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 00:01:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"

#ifndef FORMS_SOURCE_RICHTEXT_SPECIALDISPATCHERS_HXX
#include "specialdispatchers.hxx"
#endif

#ifndef _MyEDITENG_HXX
#include <svx/editeng.hxx>
#endif
#ifndef _MyEDITVIEW_HXX
#include <svx/editview.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#define ITEMID_SCRIPTSPACE          SID_ATTR_PARA_SCRIPTSPACE
#ifndef _SVX_SRIPTSPACEITEM_HXX
#include <svx/scriptspaceitem.hxx>
#endif

//........................................................................
namespace frm
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::beans;

    //====================================================================
    //= OSelectAllDispatcher
    //====================================================================
    //--------------------------------------------------------------------
    OSelectAllDispatcher::OSelectAllDispatcher( EditView& _rView, const URL&  _rURL )
        :ORichTextFeatureDispatcher( _rView, _rURL )
    {
    }

    //--------------------------------------------------------------------
    OSelectAllDispatcher::~OSelectAllDispatcher( )
    {
        if ( !isDisposed() )
        {
            acquire();
            dispose();
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL OSelectAllDispatcher::dispatch( const URL& _rURL, const Sequence< PropertyValue >& /*_rArguments*/ ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        OSL_ENSURE( _rURL.Complete == getFeatureURL().Complete, "OSelectAllDispatcher::dispatch: invalid URL!" );
        (void)_rURL;

        checkDisposed();

        EditEngine* pEngine = getEditView() ? getEditView()->GetEditEngine() : NULL;
        OSL_ENSURE( pEngine, "OSelectAllDispatcher::dispatch: no edit engine - but not yet disposed?" );
        if ( !pEngine )
            return;

        USHORT nParagraphs = pEngine->GetParagraphCount();
        if ( nParagraphs )
        {
            USHORT nLastParaNumber = nParagraphs - 1;
            xub_StrLen nParaLen = pEngine->GetTextLen( nLastParaNumber );
            getEditView()->SetSelection( ESelection( 0, 0, nLastParaNumber, nParaLen ) );
        }
    }

    //--------------------------------------------------------------------
    FeatureStateEvent OSelectAllDispatcher::buildStatusEvent() const
    {
        FeatureStateEvent aEvent( ORichTextFeatureDispatcher::buildStatusEvent() );
        aEvent.IsEnabled = sal_True;
        return aEvent;
    }

    //====================================================================
    //= OParagraphDirectionDispatcher
    //====================================================================
    //--------------------------------------------------------------------
    OParagraphDirectionDispatcher::OParagraphDirectionDispatcher( EditView& _rView, AttributeId _nAttributeId, const URL& _rURL,
            IMultiAttributeDispatcher* _pMasterDispatcher )
        :OAttributeDispatcher( _rView, _nAttributeId, _rURL, _pMasterDispatcher )
    {
    }

    //--------------------------------------------------------------------
    FeatureStateEvent OParagraphDirectionDispatcher::buildStatusEvent() const
    {
        FeatureStateEvent aEvent( OAttributeDispatcher::buildStatusEvent() );

        EditEngine* pEngine = getEditView() ? getEditView()->GetEditEngine() : NULL;
        OSL_ENSURE( pEngine, "OParagraphDirectionDispatcher::dispatch: no edit engine - but not yet disposed?" );
        if ( pEngine && pEngine->IsVertical() )
            aEvent.IsEnabled = sal_False;

        return aEvent;
    }

    //====================================================================
    //= OTextDirectionDispatcher
    //====================================================================
    //--------------------------------------------------------------------
    OTextDirectionDispatcher::OTextDirectionDispatcher( EditView& _rView, const URL& _rURL )
        :ORichTextFeatureDispatcher( _rView, _rURL )
    {
    }

    //--------------------------------------------------------------------
    void SAL_CALL OTextDirectionDispatcher::dispatch( const URL& _rURL, const Sequence< PropertyValue >& /*_rArguments*/ ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        OSL_ENSURE( _rURL.Complete == getFeatureURL().Complete, "OTextDirectionDispatcher::dispatch: invalid URL!" );
        (void)_rURL;

        checkDisposed();

        EditEngine* pEngine = getEditView() ? getEditView()->GetEditEngine() : NULL;
        OSL_ENSURE( pEngine, "OTextDirectionDispatcher::dispatch: no edit engine - but not yet disposed?" );
        if ( !pEngine )
            return;

        pEngine->SetVertical( !pEngine->IsVertical() );
    }

    //--------------------------------------------------------------------
    FeatureStateEvent OTextDirectionDispatcher::buildStatusEvent() const
    {
        FeatureStateEvent aEvent( ORichTextFeatureDispatcher::buildStatusEvent() );

        EditEngine* pEngine = getEditView() ? getEditView()->GetEditEngine() : NULL;
        OSL_ENSURE( pEngine, "OTextDirectionDispatcher::dispatch: no edit engine - but not yet disposed?" );

        aEvent.IsEnabled = sal_True;
        aEvent.State <<= (sal_Bool)( pEngine && pEngine->IsVertical() );

        return aEvent;
    }

    //====================================================================
    //= OAsianFontLayoutDispatcher
    //====================================================================
    //--------------------------------------------------------------------
    OAsianFontLayoutDispatcher::OAsianFontLayoutDispatcher( EditView& _rView, AttributeId _nAttributeId, const URL& _rURL, IMultiAttributeDispatcher* _pMasterDispatcher )
        :OParametrizedAttributeDispatcher( _rView, _nAttributeId, _rURL, _pMasterDispatcher )
    {
    }

    //--------------------------------------------------------------------
    const SfxPoolItem* OAsianFontLayoutDispatcher::convertDispatchArgsToItem( const Sequence< PropertyValue >& _rArguments )
    {
        // look for the "Enable" parameter
        const PropertyValue* pLookup = _rArguments.getConstArray();
        const PropertyValue* pLookupEnd = _rArguments.getConstArray() + _rArguments.getLength();
        while ( pLookup != pLookupEnd )
        {
            if ( pLookup->Name.equalsAscii( "Enable" ) )
                break;
            ++pLookup;
        }
        if ( pLookup != pLookupEnd )
        {
            sal_Bool bEnable = sal_True;
            OSL_VERIFY( pLookup->Value >>= bEnable );
            if ( m_nAttributeId == SID_ATTR_PARA_SCRIPTSPACE )
                return new SvxScriptSpaceItem( bEnable, (WhichId)m_nAttributeId );
            return new SfxBoolItem( (WhichId)m_nAttributeId, bEnable );
        }

        OSL_ENSURE( sal_False, "OAsianFontLayoutDispatcher::convertDispatchArgsToItem: did not find the one and only argument!" );
        return NULL;
    }

//........................................................................
}   // namespace frm
//........................................................................

