/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_FMVWIMP_HXX
#define _SVX_FMVWIMP_HXX

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif


#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLER_HPP_
#include <com/sun/star/form/XFormController.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFOCUSLISTENER_HPP_
#include <com/sun/star/awt/XFocusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLERROREVENT_HPP_
#include <com/sun/star/sdb/SQLErrorEvent.hpp>
#endif
#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif

#ifndef _SVDMARK_HXX
#include "svdmark.hxx"
#endif
FORWARD_DECLARE_INTERFACE(awt,XControl)
FORWARD_DECLARE_INTERFACE(awt,XWindow)
FORWARD_DECLARE_INTERFACE(beans,XPropertySet)
FORWARD_DECLARE_INTERFACE(util,XNumberFormats)
class Window;
class OutputDevice;
namespace binfilter {

class SdrPageViewWinRec;
class SdrPageView;
class SdrObject;
class FmFormObj;
class FmFormModel;
class FmFormShell;
class FmFormView;

//STRIP008 FORWARD_DECLARE_INTERFACE(awt,XControl)
//STRIP008 FORWARD_DECLARE_INTERFACE(awt,XWindow)
//STRIP008 FORWARD_DECLARE_INTERFACE(beans,XPropertySet)
//STRIP008 FORWARD_DECLARE_INTERFACE(util,XNumberFormats)

class FmXFormController;
class FmXFormView;

namespace svx {
    class ODataAccessDescriptor;
}

//==================================================================
// FmXPageViewWinRec
//==================================================================
class FmXPageViewWinRec : public ::cppu::WeakImplHelper1< ::com::sun::star::container::XIndexAccess>
{
    friend class FmXFormView;

    ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController > >	m_aControllerList;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >				m_xORB;
    FmXFormView*				m_pViewImpl;
    Window*						m_pWindow;

public:
    FmXPageViewWinRec(	const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&	_xORB,
                        const SdrPageViewWinRec*, FmXFormView* pView);
    ~FmXPageViewWinRec();

// UNO Anbindung
    
// ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createEnumeration() throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 _Index) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    const vector< ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController > >& GetList() {return m_aControllerList;}

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController >  getController( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& xForm );
    void setController(	const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& xForm,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >& xCC,
                        FmXFormController* pParent = NULL);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >  getControlContainer() const;
    void updateTabOrder( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& xControl,
                         const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >& xCC );
    void dispose();
    Window* getWindow() const {return m_pWindow;}
};

typedef vector<FmXPageViewWinRec*> FmWinRecList;
//==================================================================
// FmXFormView
//==================================================================
class FmXFormView :	public ::cppu::WeakImplHelper3<
                            ::com::sun::star::form::XFormControllerListener,
                            ::com::sun::star::awt::XFocusListener,
                            ::com::sun::star::container::XContainerListener>
{
    friend class FmFormView;
    friend class FmFormShell;
    friend class FmXFormShell;
    friend class FmXPageViewWinRec;
    class ObjectRemoveListener;
    friend class ObjectRemoveListener;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >	m_xORB;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>					m_xWindow;

    FmFormObj*		m_pMarkedGrid;
    FmFormView*		m_pView;
    sal_uInt32		m_nActivationEvent;
    sal_uInt32		m_nErrorMessageEvent;	// event for an asynchronous error message. See also m_aAsyncError
    sal_uInt32      m_nAutoFocusEvent;		// event for asynchronously setting the focus to a control

    ::com::sun::star::sdb::SQLErrorEvent
                    m_aAsyncError;			// error event which is to be displayed asyn. See m_nErrorMessageEvent.

    FmWinRecList	m_aWinList;				// to be filled in alive mode only

    // Liste der markierten Object, dient zur Restauration beim Umschalten von Alive in DesignMode
    SdrMarkList				m_aMark;
    ObjectRemoveListener*	m_pWatchStoredList;

    sal_Bool		m_bFirstActivation	: 1;



    FmFormShell* GetFormShell() const;

    void removeGridWindowListening();

protected:
    FmXFormView(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&	_xORB,
                FmFormView* _pView);
    ~FmXFormView();

    void	saveMarkList( sal_Bool _bSmartUnmark = sal_True );
    void	restoreMarkList( SdrMarkList& _rRestoredMarkList );
    void	stopMarkListWatching();
    void	startMarkListWatching();

    void	notifyViewDying( );
        // notifies this impl class that the anti-impl instance (m_pView) is going to die

public:
    // UNO Anbindung

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XContainerListener
    virtual void SAL_CALL elementInserted(const  ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementReplaced(const  ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementRemoved(const  ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::form::XFormControllerListener
    virtual void SAL_CALL formActivated(const ::com::sun::star::lang::EventObject& rEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL formDeactivated(const ::com::sun::star::lang::EventObject& rEvent) throw(::com::sun::star::uno::RuntimeException);

    // XFocusListener
    virtual void SAL_CALL focusGained( const ::com::sun::star::awt::FocusEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL focusLost( const ::com::sun::star::awt::FocusEvent& e ) throw (::com::sun::star::uno::RuntimeException);

    FmFormView* getView() const {return m_pView;}
    const FmWinRecList& getWindowList() const {return m_aWinList;}


    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getORB() { return m_xORB; }

    // activation handling
    inline	sal_Bool	hasEverBeenActivated( ) const { return !m_bFirstActivation; }
    inline	void		setHasBeenActivated( ) { m_bFirstActivation = sal_False; }

            void		onFirstViewActivation( const FmFormModel* _pDocModel );

private:
    FmWinRecList::iterator findWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >& rCC );
    void addWindow(const SdrPageViewWinRec*);
    void removeWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >& rCC );
    void Activate(sal_Bool bSync = sal_False);
    void Deactivate(BOOL bDeactivateController = TRUE);



    void ObjectRemovedInAliveMode(const SdrObject* pObject);

    // asynchronously displays an error message. See also OnDelayedErrorMessage.
    void	displayAsyncErrorMessage( const ::com::sun::star::sdb::SQLErrorEvent& _rEvent );

    // cancels all pending async events
    void cancelEvents();

    /// the the auto focus to the first (in terms of the tab order) control
    DECL_LINK( OnActivate, void* );
    DECL_LINK( OnAutoFocus, void* );
    DECL_LINK( OnDelayedErrorMessage, void* );
};



}//end of namespace binfilter
#endif // _SVX_FMVWIMP_HXX

