/*************************************************************************
 *
 *  $RCSfile: parametrizedattributedispatcher.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-07 16:12:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef FORMS_SOURCE_RICHTEXT_PARAMETRIZEDATTRIBUTEDISPATCHER_HXX
#include "parametrizedattributedispatcher.hxx"
#endif

#ifndef _MyEDITVIEW_HXX
#include <svx/editview.hxx>
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#ifndef _SFX_SFXUNO_HXX
#include <sfx2/sfxuno.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

//........................................................................
namespace frm
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::beans;

    //====================================================================
    //= OParametrizedAttributeDispatcher
    //====================================================================
    //--------------------------------------------------------------------
    OParametrizedAttributeDispatcher::OParametrizedAttributeDispatcher( EditView& _rView, AttributeId _nAttributeId, const URL& _rURL, IMultiAttributeDispatcher* _pMasterDispatcher )
        :OAttributeDispatcher( _rView, _nAttributeId, _rURL, _pMasterDispatcher )
    {
    }

    //--------------------------------------------------------------------
    OParametrizedAttributeDispatcher::~OParametrizedAttributeDispatcher()
    {
        acquire();
        dispose();
    }

    //--------------------------------------------------------------------
    namespace
    {
        static SfxSlotId lcl_normalizeLatinScriptSlotId( SfxSlotId _nSlotId )
        {
            switch ( _nSlotId )
            {
            case SID_ATTR_CHAR_LATIN_FONT:      return SID_ATTR_CHAR_FONT;
            case SID_ATTR_CHAR_LATIN_LANGUAGE:  return SID_ATTR_CHAR_LANGUAGE;
            case SID_ATTR_CHAR_LATIN_POSTURE:   return SID_ATTR_CHAR_POSTURE;
            case SID_ATTR_CHAR_LATIN_WEIGHT:    return SID_ATTR_CHAR_WEIGHT;
            case SID_ATTR_CHAR_LATIN_FONTHEIGHT:return SID_ATTR_CHAR_FONTHEIGHT;
            }
            return _nSlotId;
        }
    }

    //--------------------------------------------------------------------
    void OParametrizedAttributeDispatcher::fillFeatureEventFromAttributeState( FeatureStateEvent& _rEvent, const AttributeState& _rState ) const
    {
        OSL_ENSURE( getEditView(), "OParametrizedAttributeDispatcher::notifyState: already disposed!" );
        if ( !getEditView() )
            return;

        SfxItemSet aEmptySet( const_cast< EditView* >( getEditView() )->GetEmptyItemSet() );
        Sequence< PropertyValue > aUnoStateDescription;
        if ( _rState.getItem() )
        {
            aEmptySet.Put( *_rState.getItem() );
            SfxSlotId nSlotId = aEmptySet.GetPool()->GetSlotId( _rState.getItem()->Which() );
            TransformItems( nSlotId, aEmptySet, aUnoStateDescription );
            _rEvent.State <<= aUnoStateDescription;
        }
        else
            OAttributeDispatcher::fillFeatureEventFromAttributeState( _rEvent, _rState );
    }

    //--------------------------------------------------------------------
    const SfxPoolItem* OParametrizedAttributeDispatcher::convertDispatchArgsToItem( const Sequence< PropertyValue >& _rArguments )
    {
        // get the real slot id. This may differ from our attribute id: for instance, both
        // SID_ATTR_CHAR_HEIGHT and SID_ATTR_CHAR_LATIN_HEIGHT are mapped to the same which id
        SfxSlotId nSlotId = lcl_normalizeLatinScriptSlotId( (SfxSlotId)m_nAttributeId );

        SfxAllItemSet aParameterSet( getEditView()->GetEmptyItemSet() );
        TransformParameters( nSlotId, _rArguments, aParameterSet );

        const SfxPoolItem* pArgument = NULL;
        if ( aParameterSet.Count() )
        {
            OSL_ENSURE( aParameterSet.Count() == 1, "OParametrizedAttributeDispatcher::convertDispatchArgsToItem: Arguments which form more than 1 item? How this?" );
            const SfxItemPool& rPool = *aParameterSet.GetPool();
            WhichId nAttributeWhich = aParameterSet.GetPool()->GetWhich( nSlotId );
            pArgument = aParameterSet.GetItem( nAttributeWhich );
            OSL_ENSURE( pArgument, "OParametrizedAttributeDispatcher::convertDispatchArgsToItem: suspicious: there were arguments, but they're not for my slot!" );
        }

        return pArgument;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OParametrizedAttributeDispatcher::dispatch( const URL& _rURL, const Sequence< PropertyValue >& _rArguments ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        OSL_ENSURE( _rURL.Complete == getFeatureURL().Complete, "OParametrizedAttributeDispatcher::dispatch: invalid URL!" );
        if ( m_pMasterDispatcher )
        {
            const SfxPoolItem* pConvertedArgument = convertDispatchArgsToItem( _rArguments );
            m_pMasterDispatcher->executeAttribute( m_nAttributeId, pConvertedArgument );
        }
    }

//........................................................................
}   // namespace frm
//........................................................................
