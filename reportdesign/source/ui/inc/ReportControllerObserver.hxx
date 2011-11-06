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


#ifndef INCLUDED_REPORTCONTROLLEROBSERVER_HXX
#define INCLUDED_REPORTCONTROLLEROBSERVER_HXX

#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/report/XReportDefinition.hpp>
#include <cppuhelper/implbase3.hxx>

#include <memory>
#include <dllapi.h>
#include <vector>

// #include <svl/lstner.hxx>
#include <vcl/svapp.hxx>
#include <tools/link.hxx>

#include <FormattedFieldBeautifier.hxx>
#include <FixedTextColor.hxx>

namespace rptui
{
    class OReportController;
    class OXReportControllerObserverImpl;

    //========================================================================
    class /* REPORTDESIGN_DLLPUBLIC */ OXReportControllerObserver
        : public ::cppu::WeakImplHelper3<   ::com::sun::star::beans::XPropertyChangeListener
                                        ,   ::com::sun::star::container::XContainerListener
                                        ,   ::com::sun::star::util::XModifyListener
                                        >
        /*,public SfxListener*/
    {

        const ::std::auto_ptr<OXReportControllerObserverImpl> m_pImpl;

        FormattedFieldBeautifier m_aFormattedFieldBeautifier;
        FixedTextColor           m_aFixedTextColor;

        // do not allow copy
        OXReportControllerObserver(const OXReportControllerObserver&);
        OXReportControllerObserver& operator=(const OXReportControllerObserver&);
        virtual  ~OXReportControllerObserver(); // UNO Object must have private destructor!
    public:
        OXReportControllerObserver(const OReportController& _rController);

// protected:
        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );

        // XContainerListener
        virtual void SAL_CALL elementInserted(const ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced(const ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved(const ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException);

        // XModifyListener
        virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);


        void AddElement(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& Element);
        void RemoveElement(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& Element);

        void AddSection( const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection);
        void RemoveSection( const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection );

        // SfxListener
        // virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

        /**
           Create an object ob OUndoEnvLock locks the undo possibility
           As long as in the OUndoEnvLock scope, no undo is possible for manipulated object.
         */
        class OEnvLock
        {
            OXReportControllerObserver& m_rObserver;
        public:
            OEnvLock(OXReportControllerObserver& _rObserver): m_rObserver(_rObserver){m_rObserver.Lock();}
            ~OEnvLock(){ m_rObserver.UnLock(); }
        };

        void Lock();
        void UnLock();
        sal_Bool IsLocked() const;

        void Clear();
    private:

        void TogglePropertyListening(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& Element);
        void switchListening( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& _rxContainer, bool _bStartListening ) SAL_THROW(());
        void switchListening( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxObject, bool _bStartListening ) SAL_THROW(());

        // void ModeChanged(); // will be called from 'Notify'
        DECL_LINK(SettingsChanged, VclWindowEvent* );
    private:

        ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild> >::const_iterator getSection(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild>& _xContainer) const;

    };

} // namespace rptui

#endif /* INCLUDED_REPORTCONTROLLEROBSERVER_HXX */


