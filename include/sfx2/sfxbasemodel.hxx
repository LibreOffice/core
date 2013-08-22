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

#ifndef _SFX_SFXBASEMODEL_HXX_
#define _SFX_SFXBASEMODEL_HXX_

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/frame/XModule.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/frame/XTitleChangeBroadcaster.hpp>
#include <com/sun/star/frame/XUntitledNumbers.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/frame/XController2.hpp>
#include <com/sun/star/document/XCmisDocument.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentRecovery.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>
#include <com/sun/star/rdf/XDocumentMetadataAccess.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/document/EventObject.hpp>
#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/NotInitializedException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/frame/DoubleInitializationException.hpp>
#include <com/sun/star/util/XModifiable2.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/view/XPrintable.hpp>
#include <com/sun/star/view/XPrintJobBroadcaster.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XStorable2.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/script/provider/XScriptProviderSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManager2.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/script/XStarBasicAccess.hpp>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>

#ifndef INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_32
#define INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_32
#define COMPHELPER_IMPLBASE_INTERFACE_NUMBER 32
#include <comphelper/implbase_var.hxx>
#endif

#include <svl/lstner.hxx>

class SfxMedium;
class   SfxPrinter;
class   SfxViewShell;
class   SfxObjectShell                      ;
class   SfxEventHint;
class   SfxViewFrame;
struct  IMPL_SfxBaseModel_DataContainer     ;   // impl. struct to hold member of class SfxBaseModel

namespace sfx { namespace intern {
    class ViewCreationGuard;
} }
//________________________________________________________________________________________________________
//  class declarations
//________________________________________________________________________________________________________

/**_______________________________________________________________________________________________________
    @short      -

    @descr      -

    @implements XChild
                XComponent
                document::XDocumentPropertiesSupplier
                rdf::XDocumentMetadataAccess
                XEventListener
                XModel
                XModifiable2
                XPrintable
                XStorable2
                document::XEventBroadcaster
                document::XEventsSupplier
                document::XEmbeddedScripts
                document::XScriptInvocationContext
                XCloseable
                XCloseBroadcaster

    @base       cppu::BaseMutex
                 SfxListener
*/

typedef ::comphelper::WeakImplHelper32  <   css::container::XChild
                                        ,   css::document::XDocumentPropertiesSupplier
                                        ,   css::document::XCmisDocument
                                        ,   css::rdf::XDocumentMetadataAccess
                                        ,   css::document::XDocumentRecovery
                                        ,   css::document::XUndoManagerSupplier
                                        ,   css::document::XEventBroadcaster
                                        ,   css::document::XDocumentEventBroadcaster
                                        ,   css::lang::XEventListener
                                        ,   css::document::XEventsSupplier
                                        ,   css::document::XEmbeddedScripts
                                        ,   css::document::XScriptInvocationContext
                                        ,   css::frame::XModel2
                                        ,   css::util::XModifiable2
                                        ,   css::view::XPrintable
                                        ,   css::view::XPrintJobBroadcaster
                                        ,   css::frame::XStorable2
                                        ,   css::frame::XLoadable
                                        ,   css::script::XStarBasicAccess
                                        ,   css::document::XViewDataSupplier
                                        ,   css::util::XCloseable           // => css::util::XCloseBroadcaster
                                        ,   css::datatransfer::XTransferable
                                        ,   css::document::XDocumentSubStorageSupplier
                                        ,   css::document::XStorageBasedDocument
                                        ,   css::script::provider::XScriptProviderSupplier
                                        ,   css::ui::XUIConfigurationManagerSupplier
                                        ,   css::embed::XVisualObject
                                        ,   css::lang::XUnoTunnel
                                        ,   css::frame::XModule
                                        ,   css::frame::XTitle
                                        ,   css::frame::XTitleChangeBroadcaster
                                        ,   css::frame::XUntitledNumbers
                                        >   SfxBaseModel_Base;

class SFX2_DLLPUBLIC SfxBaseModel   :   protected ::cppu::BaseMutex
                                    ,   public SfxBaseModel_Base
                                    ,   public SfxListener
{

//________________________________________________________________________________________________________
//  public methods
//________________________________________________________________________________________________________

public:

    //____________________________________________________________________________________________________
    //  constructor/destructor
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    SfxBaseModel( SfxObjectShell *pObjektShell = NULL ) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual ~SfxBaseModel() ;

    //____________________________________________________________________________________________________
    //  XInterface
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      give answer, if interface is supported
        @descr      The interfaces are searched by type.

        @seealso    XInterface

        @param      "rType" is the type of searched interface.

        @return     Any     information about found interface

        @onerror    A RuntimeException is thrown.
    */

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& rType ) throw( css::uno::RuntimeException ) ;

    /**___________________________________________________________________________________________________
        @short      increment refcount
        @descr      -

        @seealso    XInterface
        @seealso    release()

        @param      -

        @return     -

        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL acquire() throw() ;

    /**___________________________________________________________________________________________________
        @short      decrement refcount
        @descr      -

        @seealso    XInterface
        @seealso    acquire()

        @param      -

        @return     -

        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL release() throw() ;

    //____________________________________________________________________________________________________
    //  XTypeProvider
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      get information about supported interfaces
        @descr      -

        @seealso    XTypeProvider

        @param      -

        @return     Sequence of types of all supported interfaces

        @onerror    A RuntimeException is thrown.
    */

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() throw( css::uno::RuntimeException ) ;

    /**___________________________________________________________________________________________________
        @short      get implementation id
        @descr      This ID is neccessary for UNO-caching. If there no ID, cache is disabled.
                    Another way, cache is enabled.

        @seealso    XTypeProvider

        @param      -

        @return     ID as Sequence of byte

        @onerror    A RuntimeException is thrown.
    */

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw( css::uno::RuntimeException ) ;


    //____________________________________________________________________________________________________
    //  XStarBasicAccess
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @seealso    XStarBasicAccess
    */
    virtual css::uno::Reference< css::container::XNameContainer > SAL_CALL getLibraryContainer() throw( css::uno::RuntimeException );

    /**___________________________________________________________________________________________________
        @seealso    XStarBasicAccess
    */
    virtual void SAL_CALL createLibrary( const OUString& LibName, const OUString& Password,
        const OUString& ExternalSourceURL, const OUString& LinkTargetURL )
            throw(css::container::ElementExistException, css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @seealso    XStarBasicAccess
    */
    virtual void SAL_CALL addModule( const OUString& LibraryName, const OUString& ModuleName,
        const OUString& Language, const OUString& Source )
            throw( css::container::NoSuchElementException, css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @seealso    XStarBasicAccess
    */
    virtual void SAL_CALL addDialog( const OUString& LibraryName, const OUString& DialogName,
        const css::uno::Sequence< sal_Int8 >& Data )
            throw(css::container::NoSuchElementException, css::uno::RuntimeException);


    //____________________________________________________________________________________________________
    //  XChild
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent() throw( css::uno::RuntimeException ) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& xParent ) throw(css::lang::NoSupportException,
                                                                                        css::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //  XComponent
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL dispose() throw (css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addEventListener(const css::uno::Reference< css::lang::XEventListener >& aListener) throw (css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeEventListener(const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException);

    // XDocumentPropertiesSupplier
    virtual css::uno::Reference< css::document::XDocumentProperties >
        SAL_CALL getDocumentProperties()
        throw (css::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //  XEventListener
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw (css::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //  XModel
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual sal_Bool SAL_CALL attachResource(const OUString& sURL,
                                             const css::uno::Sequence< css::beans::PropertyValue >& aArgs)
        throw (css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual OUString SAL_CALL getURL() throw (css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getArgs() throw (css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL connectController( const css::uno::Reference< css::frame::XController >& xController ) throw (css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL disconnectController( const css::uno::Reference< css::frame::XController >& xController ) throw (css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL lockControllers() throw (css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL unlockControllers() throw (css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual sal_Bool SAL_CALL hasControllersLocked() throw (css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual css::uno::Reference< css::frame::XController > SAL_CALL getCurrentController() throw (css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setCurrentController( const css::uno::Reference< css::frame::XController >& xController )
        throw (css::container::NoSuchElementException, css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getCurrentSelection() throw (css::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //  XModel2
    //____________________________________________________________________________________________________
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL getControllers()
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableViewControllerNames()
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference< css::frame::XController2 > SAL_CALL createDefaultViewController(const css::uno::Reference< css::frame::XFrame >& Frame )
        throw (css::uno::RuntimeException         ,
               css::lang::IllegalArgumentException,
               css::uno::Exception                );

    virtual css::uno::Reference< css::frame::XController2 > SAL_CALL createViewController(const OUString&                                 ViewName       ,
                                                                                          const css::uno::Sequence< css::beans::PropertyValue >& Arguments      ,
                                                                                          const css::uno::Reference< css::frame::XFrame >&       Frame          )
        throw (css::uno::RuntimeException         ,
               css::lang::IllegalArgumentException,
               css::uno::Exception                );

    //____________________________________________________________________________________________________
    //  XModifiable2
    //____________________________________________________________________________________________________

    virtual ::sal_Bool SAL_CALL disableSetModified(  ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL enableSetModified(  ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isSetModifiedEnabled(  ) throw (css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual sal_Bool SAL_CALL isModified() throw (css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setModified( sal_Bool bModified )
        throw (css::beans::PropertyVetoException, css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addModifyListener( const css::uno::Reference< css::util::XModifyListener >& xListener ) throw( css::uno::RuntimeException ) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeModifyListener(const css::uno::Reference< css::util::XModifyListener > & xListener) throw( css::uno::RuntimeException ) ;

    //____________________________________________________________________________________________________
    //  XCloseable
    //____________________________________________________________________________________________________

    virtual void SAL_CALL close( sal_Bool bDeliverOwnership ) throw (css::util::CloseVetoException, css::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //  XCloseBroadcaster
    //____________________________________________________________________________________________________

    virtual void SAL_CALL addCloseListener( const css::uno::Reference< css::util::XCloseListener >& xListener ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL removeCloseListener( const css::uno::Reference< css::util::XCloseListener >& xListener ) throw (css::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //  XPrintJobBroadcaster
    //____________________________________________________________________________________________________

    virtual void SAL_CALL addPrintJobListener( const css::uno::Reference< css::view::XPrintJobListener >& xListener ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL removePrintJobListener( const css::uno::Reference< css::view::XPrintJobListener >& xListener ) throw (css::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //  XPrintable
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getPrinter() throw (css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setPrinter( const css::uno::Sequence< css::beans::PropertyValue >& seqPrinter )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);
    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL print( const css::uno::Sequence< css::beans::PropertyValue >& seqOptions )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //  XStorable2
    //____________________________________________________________________________________________________

    virtual void SAL_CALL storeSelf( const  css::uno::Sequence< css::beans::PropertyValue >&   seqArguments    )
        throw (css::lang::IllegalArgumentException, css::io::IOException, css::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //  XStorable
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual sal_Bool SAL_CALL hasLocation() throw (css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual OUString SAL_CALL getLocation() throw (css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual sal_Bool SAL_CALL isReadonly() throw (css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL store() throw (css::io::IOException, css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL storeAsURL(   const   OUString& sURL,
                                        const   css::uno::Sequence< css::beans::PropertyValue >&   seqArguments    )
        throw (css::io::IOException, css::uno::RuntimeException) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL storeToURL(   const   OUString& sURL,
                                        const   css::uno::Sequence< css::beans::PropertyValue >&   seqArguments    )
        throw (css::io::IOException, css::uno::RuntimeException);



    //____________________________________________________________________________________________________
    //  XLoadable
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL initNew()
        throw (css::frame::DoubleInitializationException,
               css::io::IOException,
               css::uno::RuntimeException,
               css::uno::Exception);

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL load( const   css::uno::Sequence< css::beans::PropertyValue >&   seqArguments )
        throw (css::frame::DoubleInitializationException,
               css::io::IOException,
               css::uno::RuntimeException,
               css::uno::Exception);

    //____________________________________________________________________________________________________
    //  XDocumentSubStorageSupplier
    //____________________________________________________________________________________________________

    virtual css::uno::Reference< css::embed::XStorage > SAL_CALL getDocumentSubStorage( const OUString& aStorageName, sal_Int32 nMode )
        throw ( css::uno::RuntimeException );

    virtual css::uno::Sequence< OUString > SAL_CALL getDocumentSubStoragesNames()
        throw ( css::io::IOException, css::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XStorageBasedDocument
    //____________________________________________________________________________________________________

    virtual void SAL_CALL loadFromStorage( const css::uno::Reference< css::embed::XStorage >& xStorage,
                                            const css::uno::Sequence< css::beans::PropertyValue >& aMediaDescriptor )
        throw ( css::lang::IllegalArgumentException,
                com::sun::star::frame::DoubleInitializationException,
                css::io::IOException,
                css::uno::Exception,
                css::uno::RuntimeException );

    virtual void SAL_CALL storeToStorage( const css::uno::Reference< css::embed::XStorage >& xStorage,
                                            const css::uno::Sequence< css::beans::PropertyValue >& aMediaDescriptor )
        throw ( css::lang::IllegalArgumentException,
                css::io::IOException,
                css::uno::Exception,
                css::uno::RuntimeException );

    virtual void SAL_CALL switchToStorage( const css::uno::Reference< css::embed::XStorage >& xStorage )
        throw ( css::lang::IllegalArgumentException,
                css::io::IOException,
                css::uno::Exception,
                css::uno::RuntimeException );

    virtual css::uno::Reference< css::embed::XStorage > SAL_CALL getDocumentStorage()
        throw ( css::io::IOException,
                css::uno::Exception,
                css::uno::RuntimeException );

    virtual void SAL_CALL addStorageChangeListener(
            const css::uno::Reference< css::document::XStorageChangeListener >& xListener )
        throw ( css::uno::RuntimeException );

    virtual void SAL_CALL removeStorageChangeListener(
            const css::uno::Reference< css::document::XStorageChangeListener >& xListener )
        throw ( css::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XVisualObject
    //____________________________________________________________________________________________________

    virtual void SAL_CALL setVisualAreaSize( sal_Int64 nAspect, const css::awt::Size& aSize )
        throw ( css::lang::IllegalArgumentException,
                css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException );

    virtual css::awt::Size SAL_CALL getVisualAreaSize( sal_Int64 nAspect )
        throw ( css::lang::IllegalArgumentException,
                css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException );

    virtual css::embed::VisualRepresentation SAL_CALL getPreferredVisualRepresentation( ::sal_Int64 nAspect )
        throw ( css::lang::IllegalArgumentException,
                css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL getMapUnit( sal_Int64 nAspect )
        throw ( css::uno::Exception,
                css::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XScriptProviderSupplier
    //____________________________________________________________________________________________________

    virtual css::uno::Reference< css::script::provider::XScriptProvider > SAL_CALL getScriptProvider() throw ( css::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XUIConfigurationManagerSupplier
    //____________________________________________________________________________________________________
    virtual css::uno::Reference< css::ui::XUIConfigurationManager > SAL_CALL getUIConfigurationManager() throw ( css::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XTransferable
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual css::uno::Any SAL_CALL getTransferData( const css::datatransfer::DataFlavor& aFlavor )
        throw (css::datatransfer::UnsupportedFlavorException,
               css::io::IOException,
               css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */


    virtual css::uno::Sequence< css::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors()
        throw (css::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual sal_Bool SAL_CALL isDataFlavorSupported( const css::datatransfer::DataFlavor& aFlavor )
        throw (css::uno::RuntimeException);


    //____________________________________________________________________________________________________
    //  XEventsSupplier
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -   offers a list of event handlers which are be bound to events of
                        this object.
        @seealso    -

        @param      -

        @return     -   an Events object.

        @onerror    -
    */

    virtual css::uno::Reference< css::container::XNameReplace > SAL_CALL getEvents() throw( css::uno::RuntimeException );


    //____________________________________________________________________________________________________
    //  XEmbeddedScripts
    //____________________________________________________________________________________________________

    virtual css::uno::Reference< css::script::XStorageBasedLibraryContainer > SAL_CALL getBasicLibraries() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::script::XStorageBasedLibraryContainer > SAL_CALL getDialogLibraries() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getAllowMacroExecution() throw (css::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //  XScriptInvocationContext
    //____________________________________________________________________________________________________

    virtual css::uno::Reference< css::document::XEmbeddedScripts > SAL_CALL getScriptContainer() throw (css::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //  XEventBroadcaster
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -   registers the given XEventListener.

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::document::XEventListener >& xListener ) throw( css::uno::RuntimeException );

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -   unregisters the given XEventListener.

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::document::XEventListener >& xListener ) throw( css::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XDocumentEventBroadcaster
    //____________________________________________________________________________________________________

    virtual void SAL_CALL addDocumentEventListener( const css::uno::Reference< css::document::XDocumentEventListener >& _Listener ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL removeDocumentEventListener( const css::uno::Reference< css::document::XDocumentEventListener >& _Listener ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL notifyDocumentEvent( const OUString& _EventName, const css::uno::Reference< css::frame::XController2 >& _ViewController, const css::uno::Any& _Supplement ) throw (css::lang::IllegalArgumentException, css::lang::NoSupportException, css::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //  XUnoTunnel
    //____________________________________________________________________________________________________

    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException);

    // css.frame.XModule
    virtual void SAL_CALL setIdentifier(const OUString& sIdentifier)
        throw (css::uno::RuntimeException);

    // css.frame.XModule
    virtual OUString SAL_CALL getIdentifier()
        throw (css::uno::RuntimeException);

    // css.frame.XTitle
    virtual OUString SAL_CALL getTitle()
        throw (css::uno::RuntimeException);

    // css.frame.XTitle
    virtual void SAL_CALL setTitle( const OUString& sTitle )
        throw (css::uno::RuntimeException);

    // css.frame.XTitleChangeBroadcaster
    virtual void SAL_CALL addTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener )
        throw (css::uno::RuntimeException);

    // css.frame.XTitleChangeBroadcaster
    virtual void SAL_CALL removeTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener )
        throw (css::uno::RuntimeException);

    // css.frame.XUntitledNumbers
    virtual ::sal_Int32 SAL_CALL leaseNumber( const css::uno::Reference< css::uno::XInterface >& xComponent )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException         );

    // css.frame.XUntitledNumbers
    virtual void SAL_CALL releaseNumber( ::sal_Int32 nNumber )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException         );

    // css.frame.XUntitledNumbers
    virtual void SAL_CALL releaseNumberForComponent( const css::uno::Reference< css::uno::XInterface >& xComponent )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException         );

    // css.frame.XUntitledNumbers
    virtual OUString SAL_CALL getUntitledPrefix()
        throw (css::uno::RuntimeException);

    // css.document.XDocumentRecovery
    virtual ::sal_Bool SAL_CALL wasModifiedSinceLastSave()
        throw ( css::uno::RuntimeException );
    virtual void SAL_CALL storeToRecoveryFile( const OUString& i_TargetLocation, const css::uno::Sequence< css::beans::PropertyValue >& i_MediaDescriptor )
        throw ( css::uno::RuntimeException,
                css::io::IOException,
                css::lang::WrappedTargetException );
    virtual void SAL_CALL recoverFromFile( const OUString& i_SourceLocation, const OUString& i_SalvagedFile, const css::uno::Sequence< css::beans::PropertyValue >& i_MediaDescriptor )
        throw ( css::uno::RuntimeException,
                css::io::IOException,
                css::lang::WrappedTargetException );

    // css.document.XUndoManagerSupplier
    virtual css::uno::Reference< css::document::XUndoManager > SAL_CALL getUndoManager(  ) throw (css::uno::RuntimeException);

    //____________________________________________________________________________________________________

    // css::rdf::XNode:
    virtual OUString SAL_CALL getStringValue()
        throw (css::uno::RuntimeException);

    // css::rdf::XURI:
    virtual OUString SAL_CALL getNamespace()
        throw (css::uno::RuntimeException);
    virtual OUString SAL_CALL getLocalName()
        throw (css::uno::RuntimeException);

    // css::rdf::XRepositorySupplier:
    virtual css::uno::Reference<
        css::rdf::XRepository > SAL_CALL getRDFRepository()
        throw (css::uno::RuntimeException);

    // css::rdf::XDocumentMetadataAccess:
    virtual css::uno::Reference<
                css::rdf::XMetadatable > SAL_CALL
        getElementByMetadataReference(
            const css::beans::StringPair & i_rReference)
        throw (css::uno::RuntimeException);
    virtual css::uno::Reference<
                css::rdf::XMetadatable > SAL_CALL
        getElementByURI(const css::uno::Reference<
            css::rdf::XURI > & i_xURI)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException);
    virtual css::uno::Sequence< css::uno::Reference<
            css::rdf::XURI > > SAL_CALL getMetadataGraphsWithType(
            const css::uno::Reference<
                css::rdf::XURI > & i_xType)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException);
    virtual css::uno::Reference<
                css::rdf::XURI> SAL_CALL
        addMetadataFile(const OUString & i_rFileName,
            const css::uno::Sequence<
                css::uno::Reference< css::rdf::XURI >
                > & i_rTypes)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException,
            css::container::ElementExistException);
    virtual css::uno::Reference<
                css::rdf::XURI> SAL_CALL
        importMetadataFile(::sal_Int16 i_Format,
            const css::uno::Reference<
                css::io::XInputStream > & i_xInStream,
            const OUString & i_rFileName,
            const css::uno::Reference<
                css::rdf::XURI > & i_xBaseURI,
            const css::uno::Sequence<
                css::uno::Reference< css::rdf::XURI >
                > & i_rTypes)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException,
            css::datatransfer::UnsupportedFlavorException,
            css::container::ElementExistException,
            css::rdf::ParseException,
            css::io::IOException);
    virtual void SAL_CALL removeMetadataFile(
            const css::uno::Reference<
                css::rdf::XURI > & i_xGraphName)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException,
            css::container::NoSuchElementException);
    virtual void SAL_CALL addContentOrStylesFile(
            const OUString & i_rFileName)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException,
            css::container::ElementExistException);
    virtual void SAL_CALL removeContentOrStylesFile(
            const OUString & i_rFileName)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException,
            css::container::NoSuchElementException);

    virtual void SAL_CALL loadMetadataFromStorage(
            const css::uno::Reference<
                css::embed::XStorage > & i_xStorage,
            const css::uno::Reference<
                css::rdf::XURI > & i_xBaseURI,
            const css::uno::Reference<
                css::task::XInteractionHandler> & i_xHandler)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException);
    virtual void SAL_CALL storeMetadataToStorage(
            const css::uno::Reference<
                css::embed::XStorage > & i_xStorage)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException);
    virtual void SAL_CALL loadMetadataFromMedium(
            const css::uno::Sequence<
                css::beans::PropertyValue > & i_rMedium)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException);
    virtual void SAL_CALL storeMetadataToMedium(
            const css::uno::Sequence<
                css::beans::PropertyValue > & i_rMedium)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException);

    // XCmisDocument

    virtual css::uno::Sequence< css::beans::PropertyValue >
            SAL_CALL getCmisPropertiesValues()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCmisPropertiesValues(
            const css::uno::Sequence<
                css::beans::PropertyValue >& _cmispropertiesvalues )
        throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< css::beans::PropertyValue >
            SAL_CALL getCmisPropertiesDisplayNames()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCmisPropertiesDisplayNames(
            const css::uno::Sequence<
                css::beans::PropertyValue >& _cmispropertiesdisplaynames )
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL checkOut(  ) throw ( css::uno::RuntimeException );
    virtual void SAL_CALL cancelCheckOut(  ) throw ( css::uno::RuntimeException );
    virtual void SAL_CALL checkIn( sal_Bool bIsMajor, const OUString & rMessage  )
        throw ( css::uno::RuntimeException );

    virtual sal_Bool SAL_CALL isVersionable( ) throw ( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL canCheckOut( ) throw ( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL canCancelCheckOut( ) throw ( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL canCheckIn( ) throw ( css::uno::RuntimeException );

    sal_Bool getBoolPropertyValue( const OUString& rName ) throw ( css::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  SfxListener
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    void Notify(            SfxBroadcaster& aBC     ,
                    const   SfxHint&        aHint   ) ;

    //____________________________________________________________________________________________________
    //  public IMPL?
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    void changing() ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    SfxObjectShell* GetObjectShell() const ;

    SAL_DLLPRIVATE sal_Bool impl_isDisposed() const ;
    sal_Bool IsInitialized() const;
    sal_Bool IsDisposed() const { return impl_isDisposed(); }
    void MethodEntryCheck( const bool i_mustBeInitialized ) const;
    ::osl::Mutex& getMutex() const { return m_aMutex; }

    css::uno::Reference < css::container::XIndexAccess > SAL_CALL getViewData() throw (css::uno::RuntimeException);
    void SAL_CALL setViewData( const css::uno::Reference < css::container::XIndexAccess >& aData ) throw (css::uno::RuntimeException);

    /** calls all XEventListeners */
    void notifyEvent( const css::document::EventObject& aEvent ) const;

    /** returns true if someone added a XEventListener to this XEventBroadcaster */
    sal_Bool hasEventListeners() const;

protected:

    /* returns a unique id for the model that is valid as long as the document
       is loaded. The id is not saved across document close/reload. */
    OUString getRuntimeUID() const;

    /* returns true if the document signatures are valid, otherwise false */
    sal_Bool hasValidSignatures() const;

//________________________________________________________________________________________________________
//  private methods
//________________________________________________________________________________________________________

private:

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    css::uno::Reference< css::ui::XUIConfigurationManager2 > getUIConfigurationManager2() throw ( css::uno::RuntimeException );
    bool impl_getPrintHelper();
    SAL_DLLPRIVATE void ListenForStorage_Impl( const css::uno::Reference< css::embed::XStorage >& xStorage );
    SAL_DLLPRIVATE OUString GetMediumFilterName_Impl();

    SAL_DLLPRIVATE void impl_store( const OUString& sURL,
                        const   css::uno::Sequence< css::beans::PropertyValue >&   seqArguments    ,
                                sal_Bool                    bSaveTo         ) ;

    SAL_DLLPRIVATE void postEvent_Impl( const OUString& aName, const css::uno::Reference< css::frame::XController2 >& xController = css::uno::Reference< css::frame::XController2 >() );

    SAL_DLLPRIVATE OUString getEventName_Impl( long nID );
       SAL_DLLPRIVATE bool QuerySaveSizeExceededModules( const com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler >& xHandler );

    SAL_DLLPRIVATE css::uno::Reference< css::frame::XTitle > impl_getTitleHelper ();
    SAL_DLLPRIVATE css::uno::Reference< css::frame::XUntitledNumbers > impl_getUntitledHelper ();

    SAL_DLLPRIVATE SfxViewFrame* FindOrCreateViewFrame_Impl(
                        const css::uno::Reference< css::frame::XFrame >& i_rFrame,
                        ::sfx::intern::ViewCreationGuard& i_rGuard
                    ) const;

    SAL_DLLPRIVATE void NotifyModifyListeners_Impl() const;

    SAL_DLLPRIVATE void loadCmisProperties();

    SAL_DLLPRIVATE void handleLoadError( sal_uInt32 nError, SfxMedium* pMedium );

//________________________________________________________________________________________________________
//  private variables and methods
//________________________________________________________________________________________________________

private:

    IMPL_SfxBaseModel_DataContainer*    m_pData ;
    // cannot be held in m_pData, since it needs to be accessed in non-threadsafe context
    const bool                          m_bSupportEmbeddedScripts;
    const bool                          m_bSupportDocRecovery;

} ; // class SfxBaseModel

/** base class for sub components of an SfxBaseModel, which share their ref count and lifetime with the SfxBaseModel
*/
class SFX2_DLLPUBLIC SfxModelSubComponent
{
public:
    /** checks whether the instance is alive, i.e. properly initialized, and not yet disposed
    */
    void    MethodEntryCheck()
    {
        m_rModel.MethodEntryCheck( true );
    }

    // called when the SfxBaseModel which the component is superordinate of is being disposed
    virtual void disposing();

protected:
    SfxModelSubComponent( SfxBaseModel& i_model )
        :m_rModel( i_model )
    {
    }
    virtual ~SfxModelSubComponent();

    // helpers for implementing XInterface - delegates ref counting to the SfxBaseModel
    void acquire()  {   m_rModel.acquire(); }
    void release()  {   m_rModel.release(); }

    bool isDisposed() const {   return m_rModel.IsDisposed();   }

protected:
    const SfxBaseModel& getBaseModel() const { return m_rModel; }
          SfxBaseModel& getBaseModel()       { return m_rModel; }

          ::osl::Mutex&  getMutex()          { return m_rModel.getMutex(); }

private:
    SfxBaseModel&   m_rModel;
};

class SFX2_DLLPUBLIC SfxModelGuard
{
public:
    enum AllowedModelState
    {
        // not yet initialized
        E_INITIALIZING,
        // fully alive, i.e. initialized, and not yet disposed
        E_FULLY_ALIVE
    };

    SfxModelGuard( SfxBaseModel& i_rModel, const AllowedModelState i_eState = E_FULLY_ALIVE )
        : m_aGuard()
    {
        i_rModel.MethodEntryCheck( i_eState != E_INITIALIZING );
    }
    SfxModelGuard( SfxModelSubComponent& i_rSubComponent )
        :m_aGuard()
    {
        i_rSubComponent.MethodEntryCheck();
    }
    ~SfxModelGuard()
    {
    }

    void reset()
    {
        m_aGuard.reset();
    }

    void clear()
    {
        m_aGuard.clear();
    }

private:
    SolarMutexResettableGuard  m_aGuard;
};

#endif // _SFX_SFXBASEMODEL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
