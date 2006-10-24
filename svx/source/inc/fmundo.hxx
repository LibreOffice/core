/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmundo.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2006-10-24 15:13:06 $
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

#ifndef _SVX_FMUNDO_HXX
#define _SVX_FMUNDO_HXX

#ifndef _SVDUNDO_HXX
#include "svdundo.hxx"
#endif
#ifndef _SVDOUNO_HXX
#include "svdouno.hxx"
#endif
#ifndef SVX_FMSCRIPTINGENV_HXX
#include "fmscriptingenv.hxx"
#endif


/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_SCRIPTEVENT_HPP_
#include <com/sun/star/script/ScriptEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_SCRIPTEVENTDESCRIPTOR_HPP_
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_CONTAINEREVENT_HPP_
#include <com/sun/star/container/ContainerEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
/** === end UNO includes === **/

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif



#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif

class FmFormModel;
class FmFormObj;
class SdrObject;
class FmXFormView;

FORWARD_DECLARE_INTERFACE(awt,XControl)
FORWARD_DECLARE_INTERFACE(awt,XControlContainer)
//FORWARD_DECLARE_INTERFACE(uno,Reference)

//==================================================================
// FmUndoPropertyAction
//==================================================================
class FmUndoPropertyAction: public SdrUndoAction
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xObj;
    ::rtl::OUString         aPropertyName;
    ::com::sun::star::uno::Any          aNewValue;
    ::com::sun::star::uno::Any          aOldValue;

public:
    FmUndoPropertyAction(FmFormModel& rMod, const ::com::sun::star::beans::PropertyChangeEvent& evt);

    virtual void Undo();
    virtual void Redo();

    virtual String          GetComment() const;

};

//==================================================================
// FmUndoContainerAction
//==================================================================
class FmUndoContainerAction: public SdrUndoAction
{
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >
                    m_xContainer;   // container which the action applies to
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                    m_xElement;     // object not owned by the action
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                    m_xOwnElement;  // object owned by the action
    sal_Int32       m_nIndex;       // index of the object within it's container
    ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >
                    m_aEvents;      // events of the object

public:
    enum Action
    {
        Inserted = 1,
        Removed  = 2
    };

private:
    Action              m_eAction;

public:
    FmUndoContainerAction(FmFormModel& rMod,
                          Action _eAction,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >& xCont,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xElem,
                          sal_Int32 nIdx = -1);
    ~FmUndoContainerAction();

    virtual void Undo();
    virtual void Redo();

protected:
    void    implReInsert( ) SAL_THROW( ( ::com::sun::star::uno::Exception ) );
    void    implReRemove( ) SAL_THROW( ( ::com::sun::star::uno::Exception ) );
};

//==================================================================
// FmUndoModelReplaceAction
//==================================================================
class FmUndoModelReplaceAction : public SdrUndoAction
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel> m_xReplaced;
    SdrUnoObj*          m_pObject;

public:
    FmUndoModelReplaceAction(FmFormModel& rMod, SdrUnoObj* pObject, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel>& xReplaced);
    ~FmUndoModelReplaceAction();

    virtual void Undo();
    virtual void Redo() { Undo(); }

    virtual String          GetComment() const;
};

//========================================================================
class SVX_DLLPRIVATE FmXUndoEnvironment
    : public ::cppu::WeakImplHelper3<   ::com::sun::star::beans::XPropertyChangeListener
                                    ,   ::com::sun::star::container::XContainerListener
                                    ,   ::com::sun::star::util::XModifyListener
                                    >
    , public SfxListener
                           //   public ::cppu::OWeakObject
{
    FmFormModel& rModel;

    void*                                   m_pPropertySetCache;
    ::svxform::PFormScriptingEnvironment    m_pScriptingEnv;
    oslInterlockedCount                     m_Locks;
    ::osl::Mutex                            m_aMutex;
    sal_Bool                                bReadOnly;
    bool                                    m_bDisposed;

public:
    FmXUndoEnvironment(FmFormModel& _rModel);
    ~FmXUndoEnvironment();

    // UNO Anbindung
    //  SMART_UNO_DECLARATION(FmXUndoEnvironment, ::cppu::OWeakObject);
    //  virtual sal_Bool queryInterface(UsrUik, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>&);
    //  virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass>>    getIdlClasses(void);

    void Lock() { osl_incrementInterlockedCount( &m_Locks ); }
    void UnLock() { osl_decrementInterlockedCount( &m_Locks ); }
    sal_Bool IsLocked() const { return m_Locks != 0; }

    // access control
    struct Accessor { friend class FmFormModel; private: Accessor() { } };

    // addition and removal of form collections
    void AddForms( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>& rForms );
    void RemoveForms( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>& rForms );

    // readonly-ness
    void SetReadOnly( sal_Bool bRead, const Accessor& ) { bReadOnly = bRead; }
    sal_Bool IsReadOnly() const {return bReadOnly;}

protected:
    // XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException);

    // XContainerListener
    virtual void SAL_CALL elementInserted(const ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementReplaced(const ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementRemoved(const ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException);

    // XModifyListener
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

    void ModeChanged();
    void dispose();

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

private:
    void AddElement(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& Element);
    void RemoveElement(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& Element);
    void TogglePropertyListening(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& Element);

    void    implSetModified();

    void    switchListening( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >& _rxContainer, bool _bStartListening ) SAL_THROW(());
    void    switchListening( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxObject, bool _bStartListening ) SAL_THROW(());

public:
    // Methoden zur Zuordnung von Controls zu Forms,
    // werden von der Seite und der UndoUmgebung genutzt
    void Inserted(SdrObject* pObj);
    void Removed(SdrObject* pObj);

    void Inserted(FmFormObj* pObj);
    void Removed(FmFormObj* pObj);
};


#endif  //_SVX_FMUNDO_HXX

