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
#ifndef INCLUDED_SVX_SOURCE_INC_FMVWIMP_HXX
#define INCLUDED_SVX_SOURCE_INC_FMVWIMP_HXX

#include <sal/config.h>

#include <map>

#include "svx/svdmark.hxx"
#include "fmdocumentclassification.hxx"

#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/form/runtime/XFormController.hpp>
#include <com/sun/star/form/runtime/XFormControllerContext.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/ContainerEvent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/sdb/SQLErrorEvent.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <comphelper/stl_types.hxx>
#include <tools/link.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
#include <rtl/ref.hxx>
#include <vcl/vclptr.hxx>

class SdrPageWindow;

class SdrObject;
class FmFormObj;
class FmFormModel;
class FmFormView;
class FmFormShell;
namespace vcl { class Window; }
class OutputDevice;
class SdrUnoObj;
struct ImplSVEvent;

namespace com { namespace sun { namespace star {
    namespace awt {
        class XControl;
        class XWindow;
    }
    namespace beans {
        class XPropertySet;
    }
    namespace util {
        class XNumberFormats;
    }
}}}

class FmXFormView;

namespace svx {
    class ODataAccessDescriptor;
    struct OXFormsDescriptor;
}


// FormViewPageWindowAdapter

typedef ::cppu::WeakImplHelper <   css::container::XIndexAccess
                                ,   css::form::runtime::XFormControllerContext
                                >   FormViewPageWindowAdapter_Base;

class FormViewPageWindowAdapter : public FormViewPageWindowAdapter_Base
{
    friend class FmXFormView;

    ::std::vector< css::uno::Reference< css::form::runtime::XFormController > >   m_aControllerList;
    css::uno::Reference< css::awt::XControlContainer >                            m_xControlContainer;
    css::uno::Reference<css::uno::XComponentContext>                              m_xContext;
    FmXFormView*                m_pViewImpl;
    VclPtr<vcl::Window>         m_pWindow;

protected:
    virtual ~FormViewPageWindowAdapter();

public:
    FormViewPageWindowAdapter(  const css::uno::Reference<css::uno::XComponentContext>& _rContext,
        const SdrPageWindow&, FmXFormView* pView);
        //const SdrPageViewWinRec*, FmXFormView* pView);

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 Index) throw(css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XFormControllerContext
    virtual void SAL_CALL makeVisible( const css::uno::Reference< css::awt::XControl >& Control ) throw (css::uno::RuntimeException, std::exception) override;

    const ::std::vector< css::uno::Reference< css::form::runtime::XFormController > >& GetList() {return m_aControllerList;}

protected:
    css::uno::Reference< css::form::runtime::XFormController >  getController( const css::uno::Reference< css::form::XForm >& xForm ) const;
    void setController(
            const css::uno::Reference< css::form::XForm >& xForm,
            const css::uno::Reference< css::form::runtime::XFormController >& _rxParentController );
    const css::uno::Reference< css::awt::XControlContainer >&  getControlContainer() const { return m_xControlContainer; }
    void updateTabOrder( const css::uno::Reference< css::form::XForm >& _rxForm );
    void dispose();
    vcl::Window* getWindow() const {return m_pWindow;}
};

typedef ::rtl::Reference< FormViewPageWindowAdapter >   PFormViewPageWindowAdapter;
typedef ::std::vector< PFormViewPageWindowAdapter >     PageWindowAdapterList;
typedef ::std::set  <   css::uno::Reference< css::form::XForm >
                    ,   ::comphelper::OInterfaceCompare< css::form::XForm >
                    >   SetOfForms;
typedef ::std::map  <   css::uno::Reference< css::awt::XControlContainer >
                    ,   SetOfForms
                    ,   ::comphelper::OInterfaceCompare< css::awt::XControlContainer >
                    >   MapControlContainerToSetOfForms;
class SdrModel;

class FmXFormView : public ::cppu::WeakImplHelper<
                            css::form::XFormControllerListener,
                            css::awt::XFocusListener,
                            css::container::XContainerListener>
{
    friend class FmFormView;
    friend class FmFormShell;
    friend class FmXFormShell;
    friend class FormViewPageWindowAdapter;
    class ObjectRemoveListener;
    friend class ObjectRemoveListener;

    css::uno::Reference< css::awt::XWindow>                   m_xWindow;
    css::uno::Reference< css::beans::XPropertySet >           m_xLastCreatedControlModel;

    FmFormObj*      m_pMarkedGrid;
    FmFormView*     m_pView;
    ImplSVEvent *   m_nActivationEvent;
    ImplSVEvent *   m_nErrorMessageEvent;   // event for an asynchronous error message. See also m_aAsyncError
    ImplSVEvent *   m_nAutoFocusEvent;      // event for asynchronously setting the focus to a control
    ImplSVEvent *   m_nControlWizardEvent;  // event for asynchronously setting the focus to a control

    css::sdb::SQLErrorEvent
                    m_aAsyncError;          // error event which is to be displayed asyn. See m_nErrorMessageEvent.

    PageWindowAdapterList
                    m_aPageWindowAdapters;  // to be filled in alive mode only
    MapControlContainerToSetOfForms
                    m_aNeedTabOrderUpdate;

    // Liste der markierten Object, dient zur Restauration beim Umschalten von Alive in DesignMode
    SdrMarkList             m_aMark;
    ObjectRemoveListener*   m_pWatchStoredList;

    bool            m_bFirstActivation;
    bool            m_isTabOrderUpdateSuspended;

    FmFormShell* GetFormShell() const;

    void removeGridWindowListening();

protected:
    FmXFormView( FmFormView* _pView );
    virtual ~FmXFormView();

    void    saveMarkList();
    void    restoreMarkList( SdrMarkList& _rRestoredMarkList );
    void    stopMarkListWatching();
    void    startMarkListWatching();

    void    notifyViewDying( );
        // notifies this impl class that the anti-impl instance (m_pView) is going to die

public:
    // UNO Anbindung

// css::lang::XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw(css::uno::RuntimeException, std::exception) override;

// css::container::XContainerListener
    virtual void SAL_CALL elementInserted(const  css::container::ContainerEvent& rEvent) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementReplaced(const  css::container::ContainerEvent& rEvent) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementRemoved(const  css::container::ContainerEvent& rEvent) throw(css::uno::RuntimeException, std::exception) override;

// css::form::XFormControllerListener
    virtual void SAL_CALL formActivated(const css::lang::EventObject& rEvent) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL formDeactivated(const css::lang::EventObject& rEvent) throw(css::uno::RuntimeException, std::exception) override;

    // XFocusListener
    virtual void SAL_CALL focusGained( const css::awt::FocusEvent& e ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL focusLost( const css::awt::FocusEvent& e ) throw (css::uno::RuntimeException, std::exception) override;

    FmFormView* getView() const {return m_pView;}
    PFormViewPageWindowAdapter  findWindow( const css::uno::Reference< css::awt::XControlContainer >& _rxCC ) const;

    css::uno::Reference< css::form::runtime::XFormController >
            getFormController( const css::uno::Reference< css::form::XForm >& _rxForm, const OutputDevice& _rDevice ) const;

    // activation handling
    inline  bool        hasEverBeenActivated( ) const { return !m_bFirstActivation; }
    inline  void        setHasBeenActivated( ) { m_bFirstActivation = false; }

            void        onFirstViewActivation( const FmFormModel* _pDocModel );

    /** suspends the calls to activateTabOrder, which normally happen whenever for any ControlContainer of the view,
        new controls are inserted. Cannot be nested, i.e. you need to call resumeTabOrderUpdate before calling
        suspendTabOrderUpdate, again.
    */
    void    suspendTabOrderUpdate();

    /** resumes calls to activateTabOrder, and also does all pending calls which were collected since the last
        suspendTabOrderUpdate call.
    */
    void    resumeTabOrderUpdate();

    void    onCreatedFormObject( FmFormObj& _rFormObject );

    void    breakCreateFormObject();

    static bool
            isFocusable( const css::uno::Reference< css::awt::XControl >& i_rControl );

private:
    //void addWindow(const SdrPageViewWinRec*);
    void addWindow(const SdrPageWindow&);
    void removeWindow( const css::uno::Reference< css::awt::XControlContainer >& _rxCC );
    void Activate(bool bSync = false);
    void Deactivate(bool bDeactivateController = true);

    SdrObject*  implCreateFieldControl( const svx::ODataAccessDescriptor& _rColumnDescriptor );
    SdrObject*  implCreateXFormsControl( const svx::OXFormsDescriptor &_rDesc );

    static bool createControlLabelPair(
        OutputDevice& _rOutDev,
        sal_Int32 _nXOffsetMM,
        sal_Int32 _nYOffsetMM,
        const css::uno::Reference< css::beans::XPropertySet >& _rxField,
        const css::uno::Reference< css::util::XNumberFormats >& _rxNumberFormats,
        sal_uInt16 _nControlObjectID,
        const OUString& _rFieldPostfix,
        sal_uInt32 _nInventor,
        sal_uInt16 _nLabelObjectID,
        SdrPage* _pLabelPage,
        SdrPage* _pControlPage,
        SdrModel* _pModel,
        SdrUnoObj*& _rpLabel,
        SdrUnoObj*& _rpControl
    );

    bool    createControlLabelPair(
        OutputDevice& _rOutDev,
        sal_Int32 _nXOffsetMM,
        sal_Int32 _nYOffsetMM,
        const css::uno::Reference< css::beans::XPropertySet >& _rxField,
        const css::uno::Reference< css::util::XNumberFormats >& _rxNumberFormats,
        sal_uInt16 _nControlObjectID,
        const OUString& _rFieldPostfix,
        SdrUnoObj*& _rpLabel,
        SdrUnoObj*& _rpControl,
        const css::uno::Reference< css::sdbc::XDataSource >& _rxDataSource,
        const OUString& _rDataSourceName,
        const OUString& _rCommand,
        const sal_Int32 _nCommandType
    );

    void ObjectRemovedInAliveMode(const SdrObject* pObject);

    // asynchronously displays an error message. See also OnDelayedErrorMessage.
    void    displayAsyncErrorMessage( const css::sdb::SQLErrorEvent& _rEvent );

    // cancels all pending async events
    void cancelEvents();

    /// the auto focus to the first (in terms of the tab order) control
    void AutoFocus();
    DECL_LINK_TYPED( OnActivate, void*, void );
    DECL_LINK_TYPED( OnAutoFocus, void*, void );
    DECL_LINK_TYPED( OnDelayedErrorMessage, void*, void );
    DECL_LINK_TYPED( OnStartControlWizard, void*, void );

private:
    ::svxform::DocumentType impl_getDocumentType() const;
};


#endif // INCLUDED_SVX_SOURCE_INC_FMVWIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
