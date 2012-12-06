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

#ifndef STATUS_WORD_COUNT_DISPATCH_HXX
#define STATUS_WORD_COUNT_DISPATCH_HXX

#include "BaseDispatch.hxx"

#include <com/sun/star/awt/XRequestCallback.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>

namespace framework
{
    namespace statusbar_controller_wordcount
    {
        class WordCountDispatch : public BaseDispatch
        {
            public:
                static com::sun::star::uno::Reference< com::sun::star::frame::XDispatch >
                Create(
                    const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > &rxContext,
                    const com::sun::star::uno::Reference< com::sun::star::frame::XFrame > &rxFrame,
                    const rtl::OUString &rModuleIdentifier );
                static bool SupportsURL( const com::sun::star::util::URL &aURL, const rtl::OUString &rModuleIdentifier );

                virtual void SAL_CALL disposing();

                // com::sun::star::util::XModifyListener Methods
                virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject &aEvent ) throw ( ::com::sun::star::uno::RuntimeException );

                // com::sun::star::view::XSelectionChangeListener Methods
                virtual void SAL_CALL selectionChanged( const ::com::sun::star::lang::EventObject &aEvent ) throw ( ::com::sun::star::uno::RuntimeException );

                // com::sun::star::lang::XEventListener Methods
                virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject &aEvent ) throw ( ::com::sun::star::uno::RuntimeException );


            protected:
                virtual com::sun::star::frame::FeatureStateEvent GetState( const com::sun::star::util::URL &rURL );
                virtual rtl::OUString GetCommand()
                {
                    return m_aCommand;
                }
                virtual void ExecuteCommand( const com::sun::star::util::URL &aURL, const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > &lArguments );

            private:
                WordCountDispatch( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > &rxContext,
                                   const com::sun::star::uno::Reference< com::sun::star::frame::XFrame > &rxFrame,
                                   const rtl::OUString &rModuleIdentifier );
                ~WordCountDispatch();

                void ShowWordCountDialog() const;
                sal_Int32 CountWords( const rtl::OUString &sText );

                rtl::OUString m_aCommand;
                bool m_bIsModified;
                bool m_bInGetStatus;
                com::sun::star::uno::Reference< com::sun::star::i18n::XBreakIterator >     m_xBreakIterator;
                com::sun::star::uno::Reference< com::sun::star::awt::XRequestCallback >    m_xRequestCallback;
                com::sun::star::uno::Reference< com::sun::star::util::XModifiable >        m_xModifiable;
                com::sun::star::uno::Reference< com::sun::star::view::XSelectionSupplier > m_xSelectionSupplier;
        };
    }
}

#endif
