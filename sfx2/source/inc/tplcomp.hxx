/*************************************************************************
 *
 *  $RCSfile: tplcomp.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:35 $
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

#ifndef _TPLCOMP_HXX
#define _TPLCOMP_HXX

#include <osl/mutex.hxx>
//ASDBG #ifndef _USR_UNO_HXX
//ASDBG #include <usr/uno.hxx>
//ASDBG #endif
//ASDBG #ifndef _USR_LSTCOMP_HXX
//ASDBG #include <usr/iterhlp.hxx>
//ASDBG #endif
//ASDBG #ifndef _USR_SERVICES_HXX
//ASDBG #include <usr/services.hxx>
//ASDBG #endif
#ifndef _DTRANS_HXX
#include <so3/dtrans.hxx>
#endif
#ifndef _COM_SUN_STAR_CHAOS_XCOMMANDEXECUTOR_HPP_
#include <com/sun/star/chaos/XCommandExecutor.hpp>
#endif
#ifndef _COM_SUN_STAR_CHAOS_XSTATUSCALLBACK_HPP_
#include <com/sun/star/chaos/XStatusCallback.hpp>
#endif
#ifndef _COM_SUN_STAR_CHAOS_UNKNOWNCOMMANDEXCEPTION_HPP_
#include <com/sun/star/chaos/UnknownCommandException.hpp>
#endif
#ifndef _COM_SUN_STAR_CHAOS_STATUSCALLBACKEVENT_HPP_
#include <com/sun/star/chaos/StatusCallbackEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_CHAOS_XURLCONTENT_HPP_
#include <com/sun/star/chaos/XURLContent.hpp>
#endif
#ifndef _COM_SUN_STAR_CHAOS_XEXPLOREREXCHANGE_HPP_
#include <com/sun/star/chaos/XExplorerExchange.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HPP_
#include <com/sun/star/beans/PropertyValues.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XFASTPROPERTYSET_HPP_
#include <com/sun/star/beans/XFastPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XVETOABLECHANGELISTENER_HPP_
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATECHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyStateChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTIESCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCONTAINER_HPP_
#include <com/sun/star/beans/XPropertyContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATECHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyStateChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

class CntAnchor;

class TplAnchorListener;

// -----------------
// - TplComponent -
// -----------------

class TplComponent : public ::com::sun::star::chaos::XURLContent, public ::com::sun::star::chaos::XCommandExecutor,
                     public ::com::sun::star::beans::XPropertySet, public ::com::sun::star::chaos::XExplorerExchange,
                     public ::cppu::OWeakObject, public ::com::sun::star::lang::XServiceInfo
{
private:

    void                        HandleNewEvt( const ::rtl::OUString& rCommand,
                                              const ::com::sun::star::uno::Any& rAny );
    void                        CreatePopupMenu( ::com::sun::star::uno::Any& rAny ) const;

protected:

    ::rtl::OUString                     maURL;
    ::rtl::OUString                     maName;
    ::osl::Mutex         aMutex;
    ::cppu::OInterfaceContainerHelper maListeners;
    TplAnchorListener*          mpListener;
    short                       mnOpenMode;

public:

    SFX_DECL_XINTERFACE_XTYPEPROVIDER_XSERVICEINFO

                                TplComponent();
                                ~TplComponent();

    // interface  ::com::sun::star::ucb::XContent
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL     getParent() throw ( ::com::sun::star::uno::RuntimeException )
            { ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  aRef; return aRef; }
    virtual void        SAL_CALL                setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & Parent ) throw ( ::com::sun::star::uno::RuntimeException )
            {}

    // interface  ::com::sun::star::chaos::XURLContent
    virtual ::rtl::OUString   SAL_CALL              getURL()  throw ( ::com::sun::star::uno::RuntimeException )
            { return maURL; }
    virtual sal_Bool       SAL_CALL             initialize( const ::com::sun::star::uno::Reference< ::com::sun::star::chaos::XURLContent > & aParent, const ::rtl::OUString& aURL ) throw ( ::com::sun::star::uno::RuntimeException );

    // interface  ::com::sun::star::chaos::XCommandExecutor
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >  SAL_CALL     queryCommands()  throw ( ::com::sun::star::uno::RuntimeException );
    virtual void       SAL_CALL             cancel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & rCommandId ) throw ( ::com::sun::star::uno::RuntimeException )
            {}
    virtual void       SAL_CALL             execute( const ::rtl::OUString& rCommand, const ::com::sun::star::uno::Any& rArgs,
                                         const ::com::sun::star::uno::Reference< ::com::sun::star::chaos::XStatusCallback > & rCallback ) throw ( ::com::sun::star::uno::RuntimeException );

    // interface  ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  SAL_CALL  getPropertySetInfo()  throw ( ::com::sun::star::uno::RuntimeException );
    virtual void      SAL_CALL             setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any   SAL_CALL               getPropertyValue( const ::rtl::OUString& aPropertyName )  throw ( ::com::sun::star::uno::RuntimeException );
    virtual void      SAL_CALL              addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                                           const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual void      SAL_CALL              removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                                              const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual void      SAL_CALL              addVetoableChangeListener( const ::rtl::OUString& aPropertyName,
                                                           const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual void      SAL_CALL              removeVetoableChangeListener( const ::rtl::OUString& aPropertyName,
                                                              const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener ) throw ( ::com::sun::star::uno::RuntimeException );
    // interface  ::com::sun::star::chaos::XExplorerExchange
    virtual ::com::sun::star::uno::Any     SAL_CALL             createDataObject( sal_uInt16& nSourceOptions ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual sal_uInt16    SAL_CALL              insertDataObject( const ::com::sun::star::uno::Any& pSotDataObjectPtr,
                                                  sal_uInt16 nUserAction,
                                                  sal_uInt16 nSourceOptions ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool      SAL_CALL              queryInsertDataObject( const ::com::sun::star::uno::Any& pSotDataObjectPtr,
                                                       sal_uInt16 nUserAction,
                                                       sal_uInt16 nSourceOptions,
                                                       sal_uInt16& nResultingAction,
                                                       sal_uInt16& nDefaultAction ) throw ( ::com::sun::star::uno::RuntimeException );


    // -------------------------------------------------------------

    void                        CreateNewChild( CntAnchor* pAnchor,
                                                const String& rTitle );
    void                        RemoveChild( CntAnchor* pAnchor );
    void                        StopListening( CntAnchor* pAnchor );
    void                        HandleOpenEvt();
};

// ------------------------
// - TplContentsComponent -
// ------------------------

class TplContentsListener;

class TplContentsComponent : public TplComponent
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::chaos::XURLContent >                _xParent;
    TplContentsListener*        _pListener;
    SvDataObjectRef             _xObject;
    sal_uInt16                      _nLayout;

    void                        HandleOpenEvt( const ::com::sun::star::uno::Reference< ::com::sun::star::chaos::XStatusCallback > & rJob,
                                               const ::com::sun::star::chaos::StatusCallbackEvent& rEvt );
    void                        HandleNewEvt( const ::rtl::OUString& rCommand,
                                              const ::com::sun::star::uno::Any& rAny );
    void                        HandleInitEvt();

    void                        CreateFileName( String& rName,
                                                String& rDirURL );
    void                        CreateNewTpl( const String& rFilter,
                                              const String& rParentDir,
                                              const String& rFileName,
                                              const String& rDocName );
protected:

    void                        CreatePopupMenu( ::com::sun::star::uno::Any& rAny ) const;

public:

    SFX_DECL_XINTERFACE_XTYPEPROVIDER_XSERVICEINFO

                                TplContentsComponent();
                                ~TplContentsComponent();

    virtual sal_Bool   SAL_CALL             initialize( const ::com::sun::star::uno::Reference< ::com::sun::star::chaos::XURLContent > & aParent, const ::rtl::OUString& aURL ) throw ( ::com::sun::star::uno::RuntimeException );


    // interface  ::com::sun::star::chaos::XCommandExecutor
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >  SAL_CALL     queryCommands()  throw ( ::com::sun::star::uno::RuntimeException );
    virtual void         SAL_CALL           execute( const ::rtl::OUString& rCommand, const ::com::sun::star::uno::Any& rArgs,
                                         const ::com::sun::star::uno::Reference< ::com::sun::star::chaos::XStatusCallback > & rCallback ) throw ( ::com::sun::star::uno::RuntimeException );

    // interface  ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  SAL_CALL  getPropertySetInfo()  throw ( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any    SAL_CALL              getPropertyValue( const ::rtl::OUString& aPropertyName )  throw ( ::com::sun::star::uno::RuntimeException );
    virtual void       SAL_CALL             setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw ( ::com::sun::star::uno::RuntimeException );

    // interface  ::com::sun::star::chaos::XExplorerExchange
    virtual ::com::sun::star::uno::Any      SAL_CALL                createDataObject( sal_uInt16& nSourceOptions ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual sal_uInt16  SAL_CALL                    insertDataObject( const ::com::sun::star::uno::Any& pSotDataObjectPtr,
                                                  sal_uInt16 nUserAction,
                                                  sal_uInt16 nSourceOptions ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool    SAL_CALL                    queryInsertDataObject( const ::com::sun::star::uno::Any& pSotDataObjectPtr,
                                                       sal_uInt16 nUserAction,
                                                       sal_uInt16 nSourceOptions,
                                                       sal_uInt16& nResultingAction,
                                                       sal_uInt16& nDefaultAction ) throw ( ::com::sun::star::uno::RuntimeException );

    // -------------------------------------------------------------

    void                        CreateNewChild( CntAnchor* pAnchor );
    void                        HandleDeleteEvt();
    void                        SetURL( const ::rtl::OUString& rURL);
    void                        SetName( const ::rtl::OUString& rNewName );
};


#endif

