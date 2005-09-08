/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: attributedispatcher.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:04:20 $
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
#ifndef FORMS_SOURCE_RICHTEXT_ATTRIBUTEDISPATCHER_HXX
#define FORMS_SOURCE_RICHTEXT_ATTRIBUTEDISPATCHER_HXX

#ifndef FORMS_SOURCE_RICHTEXT_FEATUREDISPATCHER_HXX
#include "featuredispatcher.hxx"
#endif
#ifndef FORMS_SOURCE_RICHTEXT_RTATTRIBUTES_HXX
#include "rtattributes.hxx"
#endif
#ifndef FORMS_SOURCE_RICHTEXT_TEXTATTRIBUTELISTENER_HXX
#include "textattributelistener.hxx"
#endif

//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= OAttributeDispatcher
    //====================================================================
    class OAttributeDispatcher  :public ORichTextFeatureDispatcher
                                ,public ITextAttributeListener
    {
    protected:
        IMultiAttributeDispatcher*          m_pMasterDispatcher;
        AttributeId                         m_nAttributeId;

    public:
        /** ctor
            @param _nAttributeId
                the id of the attribute which this instance is responsible for
            @param _rURL
                the URL of the feature which this instance is responsible for
            @param _pMasterDispatcher
                the dispatcher which can execute the given attribute
            @param _pConverter
                an instance which is able to convert between SfxPoolItems and XDispatch-Parameters
                If not <NULL/>, the parametrized version of IMultiAttributeDispatcher::executeAttribute
                will be used.
        */
        OAttributeDispatcher(
            EditView&                           _rView,
            AttributeId                         _nAttributeId,
            const ::com::sun::star::util::URL&  _rURL,
            IMultiAttributeDispatcher*          _pMasterDispatcher
        );

        /// notifies a new attribute state
        void    notifyAttributeState( const AttributeState& _rState ) SAL_THROW (());

    protected:
        ~OAttributeDispatcher( );

    protected:
        // XDispatch
        virtual void SAL_CALL dispatch( const ::com::sun::star::util::URL& URL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments ) throw (::com::sun::star::uno::RuntimeException);

        // ITextAttributeListener
        virtual void    onAttributeStateChanged( AttributeId _nAttributeId, const AttributeState& _rState );

        // ORichTextFeatureDispatcher
        virtual void    disposing( ::osl::ClearableMutexGuard& _rClearBeforeNotify );

        // ORichTextFeatureDispatcher
        virtual ::com::sun::star::frame::FeatureStateEvent  buildStatusEvent() const;

        // own overridables
        virtual void    fillFeatureEventFromAttributeState( ::com::sun::star::frame::FeatureStateEvent& _rEvent, const AttributeState& _rState ) const;

    protected:
        /// notifies our feature state to one particular listener
        void    notifyState(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& _rxControl,
                    const AttributeState& _rState
                ) SAL_THROW (());
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_SOURCE_RICHTEXT_ATTRIBUTEDISPATCHER_HXX

