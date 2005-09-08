/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: parametrizedattributedispatcher.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:06:23 $
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

#ifndef FORMS_SOURCE_RICHTEXT_PARAMETRIZEDATTRIBUTEDISPATCHER_HXX
#define FORMS_SOURCE_RICHTEXT_PARAMETRIZEDATTRIBUTEDISPATCHER_HXX

#ifndef FORMS_SOURCE_RICHTEXT_ATTRIBUTEDISPATCHER_HXX
#include "attributedispatcher.hxx"
#endif

class SfxPoolItem;
//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= OParametrizedAttributeDispatcher
    //====================================================================
    class OParametrizedAttributeDispatcher  :public OAttributeDispatcher
    {
    public:
        OParametrizedAttributeDispatcher(
            EditView&                           _rView,
            AttributeId                         _nAttributeId,
            const ::com::sun::star::util::URL&  _rURL,
            IMultiAttributeDispatcher*          _pMasterDispatcher
        );

    protected:
        ~OParametrizedAttributeDispatcher();

        // XDispatch
        virtual void SAL_CALL dispatch( const ::com::sun::star::util::URL& URL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments ) throw (::com::sun::star::uno::RuntimeException);

        // OAttributeDispatcher
        virtual void    fillFeatureEventFromAttributeState( ::com::sun::star::frame::FeatureStateEvent& _rEvent, const AttributeState& _rState ) const;

    protected:
        // own overridables
        /** convert the arguments as got in a XDispatch::dispatch call into an SfxPoolItem, which can
            be used with a IMultiAttributeDispatcher::executeAttribute
        */
        virtual const SfxPoolItem* convertDispatchArgsToItem(
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rArguments );
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_SOURCE_RICHTEXT_PARAMETRIZEDATTRIBUTEDISPATCHER_HXX

