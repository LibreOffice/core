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


#ifndef FORMS_SOURCE_RICHTEXT_ATTRIBUTEDISPATCHER_HXX
#define FORMS_SOURCE_RICHTEXT_ATTRIBUTEDISPATCHER_HXX

#include "featuredispatcher.hxx"
#include "rtattributes.hxx"
#include "textattributelistener.hxx"

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

