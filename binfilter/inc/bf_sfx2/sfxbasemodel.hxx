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

#ifndef _SFX_SFXBASEMODEL_HXX_
#define _SFX_SFXBASEMODEL_HXX_

//________________________________________________________________________________________________________
//	include of other projects
//________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif





#ifndef  _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif

#ifndef  _COM_SUN_STAR_DOCUMENT_XEVENTBROADCASTER_HPP_
#include <com/sun/star/document/XEventBroadcaster.hpp>
#endif


#ifndef  _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP_
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif



#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif


#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif


#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif




#ifndef _COM_SUN_STAR_VIEW_XPRINTABLE_HPP_
#include <com/sun/star/view/XPrintable.hpp>
#endif

#ifndef _COM_SUN_STAR_VIEW_XPRINTJOBBROADCASTER_HPP_
#include <com/sun/star/view/XPrintJobBroadcaster.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XLOADABLE_HPP_
#include <com/sun/star/frame/XLoadable.hpp>
#endif



#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif




#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif


#ifndef _COM_SUN_STAR_SCRIPT_XSTARBASICACCESS_HPP_
#include <com/sun/star/script/XStarBasicAccess.hpp>
#endif


#ifndef	_LINK_HXX_
#include <tools/link.hxx>
#endif

#include <com/sun/star/document/XViewDataSupplier.hpp>

//________________________________________________________________________________________________________
//	include of my own project
//________________________________________________________________________________________________________

#ifndef _SFXLSTNER_HXX
#include <bf_svtools/lstner.hxx>
#endif
#include <tools/debug.hxx> //for strip
namespace binfilter {
//________________________________________________________________________________________________________
//	defines
//________________________________________________________________________________________________________

// Some defines to write better code :-)
#define	XCHILD					::com::sun::star::container::XChild
#define	XNAMECONTAINER			::com::sun::star::container::XNameContainer
#define	XCONTROLLER				::com::sun::star::frame::XController
#define	XEVENTLISTENER			::com::sun::star::lang::XEventListener
#define	XINTERFACE				::com::sun::star::uno::XInterface
#define XCOMPONENT              ::com::sun::star::lang::XComponent
#define	XMODEL					::com::sun::star::frame::XModel
#define	XMODIFIABLE				::com::sun::star::util::XModifiable
#define	XMODIFIABLE				::com::sun::star::util::XModifiable
#define XMODIFYBROADCASTER      ::com::sun::star::util::XModifyBroadcaster
#define	XMODIFYLISTENER			::com::sun::star::util::XModifyListener
#define XCLOSEABLE              ::com::sun::star::util::XCloseable
#define XCLOSEBROADCASTER       ::com::sun::star::util::XCloseBroadcaster
#define XCLOSELISTENER          ::com::sun::star::util::XCloseListener
#define	XPRINTABLE				::com::sun::star::view::XPrintable
#define	XPRINTJOBBROADCASTER	::com::sun::star::view::XPrintJobBroadcaster
#define	XSTORABLE				::com::sun::star::frame::XStorable
#define	XLOADABLE				::com::sun::star::frame::XLoadable
#define	XTYPEPROVIDER			::com::sun::star::lang::XTypeProvider
#define	XSTARBASICACCESS		::com::sun::star::script::XStarBasicAccess
#define	NOSUCHELEMENTEXCEPTION	::com::sun::star::container::NoSuchElementException
#define	ELEMENTEXISTEXCEPTION	::com::sun::star::container::ElementExistException
#define XNAMEREPLACE			::com::sun::star::container::XNameReplace

#define	DOCEVENTOBJECT			::com::sun::star::document::EventObject
#define	XDOCEVENTLISTENER		::com::sun::star::document::XEventListener
#define	XDOCUMENTINFO			::com::sun::star::document::XDocumentInfo
#define	XDOCUMENTINFOSUPPLIER	::com::sun::star::document::XDocumentInfoSupplier
#define	XEVENTBROADCASTER		::com::sun::star::document::XEventBroadcaster
#define	XEVENTSSUPPLIER			::com::sun::star::document::XEventsSupplier

#define	NOSUPPORTEXCEPTION		::com::sun::star::lang::NoSupportException
#define	RUNTIMEEXCEPTION		::com::sun::star::uno::RuntimeException
#define CLOSEVETOEXCEPTION      ::com::sun::star::util::CloseVetoException

#define	ANY						::com::sun::star::uno::Any
#define	EVENTOBJECT				::com::sun::star::lang::EventObject
#define	PROPERTYVALUE			::com::sun::star::beans::PropertyValue
#define	REFERENCE				::com::sun::star::uno::Reference
#define	SEQUENCE				::com::sun::star::uno::Sequence
#define	MUTEX					::osl::Mutex
#define	OUSTRING				::rtl::OUString
#define	OWEAKOBJECT				::cppu::OWeakObject
#define	UNOTYPE					::com::sun::star::uno::Type
#define XVIEWDATASUPPLIER       ::com::sun::star::document::XViewDataSupplier
#define XINDEXACCESS		  	::com::sun::star::container::XIndexAccess
#define XTRANSFERABLE		  	::com::sun::star::datatransfer::XTransferable
#define DATAFLAVOR				::com::sun::star::datatransfer::DataFlavor

//________________________________________________________________________________________________________
//	forwards
//________________________________________________________________________________________________________
class	SfxPrinter;
class	SfxViewShell;
class	SfxObjectShell						;
class	SfxEventHint;
struct	IMPL_SfxBaseModel_DataContainer		;	// impl. struct to hold member of class SfxBaseModel

//________________________________________________________________________________________________________
//	class declarations
//________________________________________________________________________________________________________

struct IMPL_SfxBaseModel_MutexContainer
{
    MUTEX m_aMutex ;
} ;

/**_______________________________________________________________________________________________________
    @short		-

    @descr		-

    @implements	XChild
                XComponent
                XDocumentInfoSupplier
                XEventListener
                XModel
                XModifiable
                XPrintable
                XStorable
                ::document::XEventBroadcaster
                ::document::XEventsSupplier
                XCloseable
                XCloseBroadcaster

    @base		IMPL_MutexContainer
                 SfxListener
*/

class SfxBaseModel	:	public XTYPEPROVIDER
                    ,	public XCHILD
                    ,	public XDOCUMENTINFOSUPPLIER
                    ,	public XEVENTBROADCASTER
                    ,	public XEVENTLISTENER
                    ,	public XEVENTSSUPPLIER
                    ,	public XMODEL
                    ,	public XMODIFIABLE
                    ,	public XPRINTABLE
                    ,	public XPRINTJOBBROADCASTER
                    ,	public XSTORABLE
                    ,	public XLOADABLE
                    ,	public XSTARBASICACCESS
                    ,   public XVIEWDATASUPPLIER
                    ,   public XCLOSEABLE           // => XCLOSEBROADCASTER
                    ,   public XTRANSFERABLE
                    ,	public IMPL_SfxBaseModel_MutexContainer
                    ,	public SfxListener
                    ,	public OWEAKOBJECT
{

//________________________________________________________________________________________________________
//	public methods
//________________________________________________________________________________________________________

public:

    //____________________________________________________________________________________________________
    //	constructor/destructor
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    SfxBaseModel( SfxObjectShell *pObjektShell = NULL ) ;

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual ~SfxBaseModel() ;

    //____________________________________________________________________________________________________
    //	XInterface
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short		give answer, if interface is supported
        @descr		The interfaces are searched by type.

        @seealso	XInterface

        @param      "rType" is the type of searched interface.

        @return		Any		information about found interface

        @onerror	A RuntimeException is thrown.
    */

    virtual ANY SAL_CALL queryInterface( const UNOTYPE& rType ) throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short		increment refcount
        @descr		-

        @seealso	XInterface
        @seealso	release()

        @param		-

        @return		-

        @onerror	A RuntimeException is thrown.
    */

    virtual void SAL_CALL acquire() throw() ;

    /**___________________________________________________________________________________________________
        @short		decrement refcount
        @descr		-

        @seealso	XInterface
        @seealso	acquire()

        @param		-

        @return		-

        @onerror	A RuntimeException is thrown.
    */

    virtual void SAL_CALL release() throw() ;

    //____________________________________________________________________________________________________
    //	XTypeProvider
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short		get information about supported interfaces
        @descr      -

        @seealso	XTypeProvider

        @param      -

        @return		Sequence of types of all supported interfaces

        @onerror	A RuntimeException is thrown.
    */

    virtual SEQUENCE< UNOTYPE > SAL_CALL getTypes() throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short		get implementation id
        @descr      This ID is neccessary for UNO-caching. If there no ID, cache is disabled.
                    Another way, cache is enabled.

        @seealso	XTypeProvider

        @param		-

        @return		ID as Sequence of byte

        @onerror	A RuntimeException is thrown.
    */

    virtual SEQUENCE< sal_Int8 > SAL_CALL getImplementationId() throw( RUNTIMEEXCEPTION ) ;

    //____________________________________________________________________________________________________
    //	XStarBasicAccess
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @seealso	XStarBasicAccess
    */
    virtual REFERENCE< XNAMECONTAINER > SAL_CALL getLibraryContainer() throw( RUNTIMEEXCEPTION );

    /**___________________________________________________________________________________________________
        @seealso	XStarBasicAccess
    */
    virtual void SAL_CALL createLibrary( const OUSTRING& LibName, const OUSTRING& Password,
        const OUSTRING& ExternalSourceURL, const OUSTRING& LinkTargetURL )
            throw(ELEMENTEXISTEXCEPTION, RUNTIMEEXCEPTION);

    /**___________________________________________________________________________________________________
        @seealso	XStarBasicAccess
    */
    virtual void SAL_CALL addModule( const OUSTRING& LibraryName, const OUSTRING& ModuleName,
        const OUSTRING& Language, const OUSTRING& Source )
            throw( NOSUCHELEMENTEXCEPTION, RUNTIMEEXCEPTION);

    /**___________________________________________________________________________________________________
        @seealso	XStarBasicAccess
    */
    virtual void SAL_CALL addDialog( const OUSTRING& LibraryName, const OUSTRING& DialogName,
        const ::com::sun::star::uno::Sequence< sal_Int8 >& Data )
            throw(NOSUCHELEMENTEXCEPTION, RUNTIMEEXCEPTION);

    //____________________________________________________________________________________________________
    //	XChild
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual REFERENCE< XINTERFACE > SAL_CALL getParent() throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual void SAL_CALL setParent( const REFERENCE< XINTERFACE >& xParent ) throw(	NOSUPPORTEXCEPTION	,
                                                                                        RUNTIMEEXCEPTION	) ;

    //____________________________________________________________________________________________________
    //	XComponent
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual void SAL_CALL addEventListener(const REFERENCE< XEVENTLISTENER >& aListener) throw (::com::sun::star::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual void SAL_CALL removeEventListener(const REFERENCE< XEVENTLISTENER >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //	XDocumentInfoSupplier
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual REFERENCE< XDOCUMENTINFO > SAL_CALL getDocumentInfo() throw (::com::sun::star::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //	XEventListener
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual void SAL_CALL disposing( const EVENTOBJECT& aEvent ) throw (::com::sun::star::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //	XModel
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual sal_Bool SAL_CALL attachResource(	const	OUSTRING&					sURL	,
                                                const	SEQUENCE< PROPERTYVALUE >&	aArgs	)
        throw (::com::sun::star::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual OUSTRING SAL_CALL getURL() throw (::com::sun::star::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual SEQUENCE< PROPERTYVALUE > SAL_CALL getArgs() throw (::com::sun::star::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual void SAL_CALL connectController( const REFERENCE< XCONTROLLER >& xController ) throw (::com::sun::star::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual void SAL_CALL disconnectController( const REFERENCE< XCONTROLLER >& xController ) throw (::com::sun::star::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual void SAL_CALL lockControllers() throw (::com::sun::star::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual void SAL_CALL unlockControllers() throw (::com::sun::star::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual sal_Bool SAL_CALL hasControllersLocked() throw (::com::sun::star::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual REFERENCE< XCONTROLLER > SAL_CALL getCurrentController() throw (::com::sun::star::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual void SAL_CALL setCurrentController( const REFERENCE< XCONTROLLER >& xController )
        throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual REFERENCE< XINTERFACE > SAL_CALL getCurrentSelection() throw (::com::sun::star::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //	XModifiable
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual sal_Bool SAL_CALL isModified() throw (::com::sun::star::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual void SAL_CALL setModified( sal_Bool bModified )
        throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual void SAL_CALL addModifyListener( const REFERENCE< XMODIFYLISTENER >& xListener ) throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual void SAL_CALL removeModifyListener(const REFERENCE< XMODIFYLISTENER > & xListener) throw( RUNTIMEEXCEPTION ) ;

    //____________________________________________________________________________________________________
    //  XCloseable
    //____________________________________________________________________________________________________

    virtual void SAL_CALL close( sal_Bool bDeliverOwnership ) throw (CLOSEVETOEXCEPTION, RUNTIMEEXCEPTION);

    //____________________________________________________________________________________________________
    //  XCloseBroadcaster
    //____________________________________________________________________________________________________

    virtual void SAL_CALL addCloseListener( const REFERENCE< XCLOSELISTENER >& xListener ) throw (RUNTIMEEXCEPTION);
    virtual void SAL_CALL removeCloseListener( const REFERENCE< XCLOSELISTENER >& xListener ) throw (RUNTIMEEXCEPTION);

    //____________________________________________________________________________________________________
    //	XPrintJobBroadcaster
    //____________________________________________________________________________________________________

    virtual void SAL_CALL addPrintJobListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XPrintJobListener >& xListener )throw (::com::sun::star::uno::RuntimeException) {DBG_BF_ASSERT(0, "STRIP");}//STRIP001 virtual void SAL_CALL addPrintJobListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XPrintJobListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePrintJobListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XPrintJobListener >& xListener ) throw (::com::sun::star::uno::RuntimeException) {DBG_BF_ASSERT(0, "STRIP");}//STRIP001 virtual void SAL_CALL removePrintJobListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XPrintJobListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //	XPrintable
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual SEQUENCE< PROPERTYVALUE > SAL_CALL getPrinter() throw (::com::sun::star::uno::RuntimeException) {DBG_BF_ASSERT(0, "STRIP");SEQUENCE< PROPERTYVALUE > aFlavorSeq( 0 );return aFlavorSeq;}//STRIP001 virtual SEQUENCE< PROPERTYVALUE > SAL_CALL getPrinter() throw (::com::sun::star::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual void SAL_CALL setPrinter( const SEQUENCE< PROPERTYVALUE >& seqPrinter )
    throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException) {DBG_BF_ASSERT(0, "STRIP");}//STRIP001 throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual void SAL_CALL print( const SEQUENCE< PROPERTYVALUE >& seqOptions )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //	XStorable
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual sal_Bool SAL_CALL hasLocation() throw (::com::sun::star::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual OUSTRING SAL_CALL getLocation() throw (::com::sun::star::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual sal_Bool SAL_CALL isReadonly() throw (::com::sun::star::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual void SAL_CALL store() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual void SAL_CALL storeAsURL(	const	OUSTRING&					sURL			,
                                        const	SEQUENCE< PROPERTYVALUE >&	seqArguments	)
        throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException) ;

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual void SAL_CALL storeToURL(	const	OUSTRING&					sURL			,
                                        const	SEQUENCE< PROPERTYVALUE >&	seqArguments	)
        throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //	XLoadable
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual void SAL_CALL initNew() 
        throw (::com::sun::star::frame::DoubleInitializationException, 
               ::com::sun::star::io::IOException, 
               ::com::sun::star::uno::RuntimeException, 
               ::com::sun::star::uno::Exception); // #dochnoetig# {DBG_BF_ASSERT(0, "STRIP");}//STRIP001 ::com::sun::star::uno::Exception);

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual void SAL_CALL load(	const	SEQUENCE< PROPERTYVALUE >&	seqArguments )
        throw (::com::sun::star::frame::DoubleInitializationException, 
               ::com::sun::star::io::IOException, 
               ::com::sun::star::uno::RuntimeException, 
                ::com::sun::star::uno::Exception); // #dochnoetig# {DBG_BF_ASSERT(0, "STRIP");}//STRIP001 ::com::sun::star::uno::Exception);

    //____________________________________________________________________________________________________
    //	XTransferable
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual ANY SAL_CALL getTransferData( const DATAFLAVOR& aFlavor )
        throw (::com::sun::star::datatransfer::UnsupportedFlavorException, 
               ::com::sun::star::io::IOException, 
               ::com::sun::star::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual SEQUENCE< DATAFLAVOR > SAL_CALL getTransferDataFlavors() 
    throw (::com::sun::star::uno::RuntimeException);

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual sal_Bool SAL_CALL isDataFlavorSupported( const DATAFLAVOR& aFlavor )
        throw (::com::sun::star::uno::RuntimeException) {DBG_BF_ASSERT(0, "STRIP");return false;}//STRIP001 throw (::com::sun::star::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //	XEventsSupplier
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-	offers a list of event handlers which are be bound to events of
                        this object.
        @seealso	-

        @param		-

        @return		-	an <type>Events</type> object.

        @onerror	-
    */

    virtual REFERENCE< XNAMEREPLACE > SAL_CALL getEvents() throw( RUNTIMEEXCEPTION );

    //____________________________________________________________________________________________________
    //	XEventBroadcaster
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-	registers the given <type>XEventListener</type>.

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual void SAL_CALL addEventListener( const REFERENCE< XDOCEVENTLISTENER >& xListener ) throw( RUNTIMEEXCEPTION );

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-	unregisters the given <type>XEventListener</type>.

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    virtual void SAL_CALL removeEventListener( const REFERENCE< XDOCEVENTLISTENER >& xListener ) throw( RUNTIMEEXCEPTION );

    //____________________________________________________________________________________________________
    //	SfxListener
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    void Notify(			SfxBroadcaster&	aBC		,
                    const	SfxHint&		aHint	) ;

    //____________________________________________________________________________________________________
    //	public IMPL?
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    void changing() ;

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    SfxObjectShell* GetObjectShell() const ;

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    sal_Bool impl_isDisposed() const ;

    ::com::sun::star::uno::Reference < ::com::sun::star::container::XIndexAccess > SAL_CALL getViewData() throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL setViewData( const ::com::sun::star::uno::Reference < ::com::sun::star::container::XIndexAccess >& aData ) throw (::com::sun::star::uno::RuntimeException);

    /** calls all XEventListeners */
    void notifyEvent( const ::com::sun::star::document::EventObject& aEvent ) const;

    /** returns true if someone added a XEventListener to this XEventBroadcaster */
    sal_Bool hasEventListeners() const;

protected:
    virtual void impl_setPrinter(const SEQUENCE< PROPERTYVALUE >& rPrinter,SfxPrinter*& pPrinter,sal_uInt16& nChangeFlags,SfxViewShell*& pViewSh){DBG_BF_ASSERT(0, "STRIP");}//STRIP001 virtual void impl_setPrinter(const SEQUENCE< PROPERTYVALUE >& rPrinter,SfxPrinter*& pPrinter,sal_uInt16& nChangeFlags,SfxViewShell*& pViewSh);
//________________________________________________________________________________________________________
//	private methods
//________________________________________________________________________________________________________

private:

    /**___________________________________________________________________________________________________
        @short		-
        @descr		-

        @seealso	-

        @param		-

        @return		-

        @onerror	-
    */

    void impl_store(			SfxObjectShell*				pObjectShell	,
                        const	OUSTRING&					sURL			,
                        const	SEQUENCE< PROPERTYVALUE >&	seqArguments	,
                                sal_Bool					bSaveTo			) ;
    void	postEvent_Impl( const SfxEventHint& rHint );
    String	getEventName_Impl( long nID );

//________________________________________________________________________________________________________
//	private variables and methods
//________________________________________________________________________________________________________

private:

    IMPL_SfxBaseModel_DataContainer*	m_pData	;

    DECL_LINK( LoadDone_Impl, void* );

} ;	// class SfxBaseModel

}//end of namespace binfilter
#endif // _SFX_SFXBASEMODEL_HXX_
