/*************************************************************************
 *
 *  $RCSfile: eventatt.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 17:00:17 $
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

//#include <stl_queue.h>

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif


#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHER_HPP_
#include <com/sun/star/script/XEventAttacher.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XALLLISTENER_HPP_
#include <com/sun/star/script/XAllListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XSCRIPTEVENTSSUPPLIER_HPP_
#include <com/sun/star/script/XScriptEventsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XSCRIPTEVENTSATTACHER_HPP_
#include <com/sun/star/script/XScriptEventsAttacher.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_SCRIPTEVENTDESCRIPTOR_HPP_
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLMODEL_HPP_
#include <com/sun/star/awt/XControlModel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XDIALOG_HPP_
#include <com/sun/star/awt/XDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif

#include <drafts/com/sun/star/script/provider/XScriptProviderSupplier.hpp>
#include <drafts/com/sun/star/script/provider/XScriptProvider.hpp>

#include <com/sun/star/frame/XModel.hpp>

//==================================================================================================

#include <xmlscript/dynload.hxx>
#include <svtools/sbx.hxx>
#include <sbunoobj.hxx>
#include <sbstar.hxx>
#include <sbmeth.hxx>
#include <runtime.hxx>
#include <sbintern.hxx>


#include <cppuhelper/implbase1.hxx>
using namespace ::drafts::com::sun::star::script;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

typedef ::cppu::WeakImplHelper1< ::com::sun::star::script::XScriptEventsAttacher > ScriptEventsAttacherHelper;


// Attaches dialogs
class DialogEventAttacher : public ScriptEventsAttacherHelper
{
    friend class DialogAllListener_Impl;

    ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacher > mxEventAttacher;
    ::osl::Mutex maMutex;

public:
    DialogEventAttacher( void ) {}

    // Methods
    virtual void SAL_CALL attachEvents( const ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >& Objects,
        const ::com::sun::star::uno::Reference< ::com::sun::star::script::XScriptListener >& xListener,
        const ::com::sun::star::uno::Any& Helper )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::beans::IntrospectionException,
                  ::com::sun::star::script::CannotCreateAdapterException,
                  ::com::sun::star::lang::ServiceNotRegisteredException,
                  ::com::sun::star::uno::RuntimeException);
};



//===================================================================

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::reflection;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::cppu;
using namespace ::osl;
using namespace ::rtl;



//===================================================================
void unoToSbxValue( SbxVariable* pVar, const Any& aValue );
Any sbxToUnoValue( SbxVariable* pVar );


Reference< frame::XModel >  getModelFromBasic( SbxObject* pBasic )
{
    Reference< frame::XModel > xModel;

    SbxObject* basicChosen = pBasic;

    if ( basicChosen == NULL)
    {
        OSL_TRACE("getModelFromBasic() StarBASIC* is NULL" );
        return xModel;
    }
    SbxObject* p = pBasic;
    SbxObject* pParent = p->GetParent();
    SbxObject* pParentParent = pParent ? pParent->GetParent() : NULL;

    if( pParentParent )
    {
        basicChosen = pParentParent;
    }
    else if( pParent )
    {
        basicChosen = pParent;
    }


    Any aModel;
    SbxVariable *pCompVar = basicChosen->Find(  UniString(RTL_CONSTASCII_USTRINGPARAM("ThisComponent")), SbxCLASS_OBJECT );

    if ( pCompVar )
    {
         aModel = sbxToUnoValue( pCompVar );
         if ( sal_False == ( aModel >>= xModel ) ||
              !xModel.is() )
         {
             OSL_TRACE("Failed to extract model from thisComponent ");
             return xModel;
         }
         else
         {
             OSL_TRACE("Have model ThisComponent points to url %s",
                 ::rtl::OUStringToOString( xModel->getURL(),
                     RTL_TEXTENCODING_ASCII_US ).pData->buffer );

         }
    }
    else
    {
        OSL_TRACE("Failed to get ThisComponent");
    }
    return xModel;
}

void SFURL_firing_impl( const ScriptEvent& aScriptEvent, Any* pRet, const Reference< frame::XModel >& xModel )
{
        OSL_TRACE("SFURL_firing_impl() processing script url %s",
            ::rtl::OUStringToOString( aScriptEvent.ScriptCode,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        try
        {
            Reference< provider::XScriptProviderSupplier > xSPS =
                Reference< provider::XScriptProviderSupplier >
                    ( xModel, UNO_QUERY );

            if ( !xSPS.is() )
            {
                OSL_TRACE("SFURL_firing_impl(): Failed to get XScriptProvider");
                return;
            }

            Reference< provider::XScriptProvider > xScriptProvider =
                xSPS->getScriptProvider();

            if ( !xScriptProvider.is() )
            {
                OSL_TRACE("SFURL_firing_impl() Failed to create msp");
                return;
            }
            Sequence< Any > inArgs( 0 );
            Sequence< Any > outArgs( 0 );
            Sequence< sal_Int16 > outIndex;

            // get Arguments for script
            inArgs = aScriptEvent.Arguments;

            Reference< provider::XScript > xScript = xScriptProvider->getScript( aScriptEvent.ScriptCode );

            if ( !xScript.is() )
            {
                OSL_TRACE("SFURL_firing_impl() Failed to obtain XScript");
                return;
            }

            Any result = xScript->invoke( inArgs, outIndex, outArgs );
            if ( pRet )
            {
                *pRet = result;
            }
        }
        catch ( RuntimeException& re )
        {
            OSL_TRACE("SFURL_firing_impl() Caught RuntimeException reason %s.",
                ::rtl::OUStringToOString( re.Message,
                    RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        }
        catch ( Exception& e )
        {
            OSL_TRACE("SFURL_firing_impl() Caught Exception reason %s.",
                ::rtl::OUStringToOString( e.Message,
                    RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        }

}


class BasicScriptListener_Impl : public WeakImplHelper1< XScriptListener >
{
    StarBASICRef maBasicRef;
        Reference< frame::XModel > m_xModel;

    virtual void firing_impl(const ScriptEvent& aScriptEvent, Any* pRet);

public:
    BasicScriptListener_Impl( StarBASIC* pBasic, const Reference< frame::XModel >& xModel )
        : maBasicRef( pBasic ), m_xModel( xModel ) {}

    // Methoden von XAllListener
    virtual void SAL_CALL firing(const ScriptEvent& aScriptEvent)
        throw( RuntimeException );
    virtual Any SAL_CALL approveFiring(const ScriptEvent& aScriptEvent)
        throw( InvocationTargetException, RuntimeException );

    // Methoden von XEventListener
    virtual void SAL_CALL disposing(const EventObject& Source)
        throw( RuntimeException );
};

// Methods XAllListener
void BasicScriptListener_Impl::firing( const ScriptEvent& aScriptEvent ) throw ( RuntimeException )
{
    firing_impl( aScriptEvent, NULL );
}

Any BasicScriptListener_Impl::approveFiring( const ScriptEvent& aScriptEvent )
    throw ( InvocationTargetException, RuntimeException )
{
    Any aRetAny;
    firing_impl( aScriptEvent, &aRetAny );
    return aRetAny;
}

// Methods XEventListener
void BasicScriptListener_Impl::disposing(const EventObject& ) throw ( RuntimeException )
{
    // TODO: ???
    //NAMESPACE_VOS(OGuard) guard( Application::GetSolarMutex() );
    //xSbxObj.Clear();
}


void BasicScriptListener_Impl::firing_impl( const ScriptEvent& aScriptEvent, Any* pRet )
{
    //Guard< Mutex > aGuard( Mutex::getGlobalMutex() );
    //{
    if( aScriptEvent.ScriptType.compareToAscii( "StarBasic" ) == 0 )
    {
        // Full qualified name?
        String aMacro( aScriptEvent.ScriptCode );
        String aLibName;
        String aLocation;
        if( aMacro.GetTokenCount( '.' ) == 3 )
        {
            sal_uInt16 nLast = 0;
            OUString aFullLibName = aMacro.GetToken( 0, '.', nLast );

            sal_Int32 nIndex = aFullLibName.indexOf( (sal_Unicode)':' );
            if (nIndex >= 0)
            {
                aLocation = aFullLibName.copy( 0, nIndex );
                aLibName = aFullLibName.copy( nIndex + 1 );
            }

            String aModul = aMacro.GetToken( 0, '.', nLast );
            aMacro.Erase( 0, nLast );
        }

        SbxObject* p = maBasicRef;
        SbxObject* pParent = p->GetParent();
        SbxObject* pParentParent = pParent ? pParent->GetParent() : NULL;

        StarBASICRef xAppStandardBasic;
        StarBASICRef xDocStandardBasic;
        if( pParentParent )
        {
            // Own basic must be document library
            xAppStandardBasic = (StarBASIC*)pParentParent;
            xDocStandardBasic = (StarBASIC*)pParent;
        }
        else if( pParent )
        {
            String aName = p->GetName();
            if( aName.EqualsAscii("Standard") )
            {
                // Own basic is doc standard lib
                xDocStandardBasic = (StarBASIC*)p;
            }
            xAppStandardBasic = (StarBASIC*)pParent;
        }
        else
        {
            xAppStandardBasic = (StarBASIC*)p;
        }

        sal_Bool bSearchLib = true;
        StarBASICRef xLibSearchBasic;
        if( aLocation.EqualsAscii("application") )
            xLibSearchBasic = xAppStandardBasic;
        else if( aLocation.EqualsAscii("document") )
            xLibSearchBasic = xDocStandardBasic;
        else
            bSearchLib = false;

        SbxVariable* pMethVar = NULL;
        // Be still tolerant and make default search if no search basic exists
        if( bSearchLib && xLibSearchBasic.Is() )
        {
            StarBASICRef xLibBasic;
            sal_Int16 nCount = xLibSearchBasic->GetObjects()->Count();
            for( sal_Int16 nObj = -1; nObj < nCount ; nObj++ )
            {
                StarBASIC* pBasic;
                if( nObj == -1 )
                {
                    pBasic = (StarBASIC*)xLibSearchBasic;
                }
                else
                {
                    SbxVariable* pVar = xLibSearchBasic->GetObjects()->Get( nObj );
                    pBasic = PTR_CAST(StarBASIC,pVar);
                }
                if( pBasic )
                {
                    String aName = pBasic->GetName();
                    if( aName == aLibName )
                    {
                        // Search only in the lib, not automatically in application basic
                        USHORT nFlags = pBasic->GetFlags();
                        pBasic->ResetFlag( SBX_GBLSEARCH );
                        pMethVar = pBasic->Find( aMacro, SbxCLASS_DONTCARE );
                        pBasic->SetFlags( nFlags );
                        break;
                    }
                }
            }
        }

        // Default: Be tolerant and search everywhere
        if( (!pMethVar || !pMethVar->ISA(SbMethod)) && maBasicRef.Is() )
            pMethVar = maBasicRef->FindQualified( aMacro, SbxCLASS_DONTCARE );

        SbMethod* pMeth = PTR_CAST(SbMethod,pMethVar);
        if( !pMeth )
            return;

        // Setup parameters
        SbxArrayRef xArray;
        String aTmp;
        sal_Int32 nCnt = aScriptEvent.Arguments.getLength();
        if( nCnt )
        {
            xArray = new SbxArray;
            const Any *pArgs = aScriptEvent.Arguments.getConstArray();
            for( sal_Int32 i = 0; i < nCnt; i++ )
            {
                SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
                unoToSbxValue( (SbxVariable*)xVar, pArgs[i] );
                xArray->Put( xVar, i+1 );
            }
        }

        // Call method
        SbxVariableRef xValue = pRet ? new SbxVariable : 0;
        if( xArray.Is() )
            pMeth->SetParameters( xArray );
        ErrCode nErr = pMeth->Call( xValue );
        if( pRet )
            *pRet = sbxToUnoValue( xValue );
        pMeth->SetParameters( NULL );
    }
        else // scripting framework script
        {
            //callBasic via scripting framework
            SFURL_firing_impl( aScriptEvent, pRet, m_xModel );

        }
}


//===================================================================

// Function to map from NameContainer to sequence needed
// to call XScriptEventsAttacher::attachEvents
//void SAL_CALL attachDialogEvents( StarBASIC* pBasic,
void SAL_CALL attachDialogEvents( StarBASIC* pBasic, const Reference< frame::XModel >& xModel,
    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& xDialogControl )
    //const ::com::sun::star::uno::Reference< ::com::sun::star::script::XScriptListener >& xListener )
{
    static Reference< XScriptEventsAttacher > xEventsAttacher;
    Guard< Mutex > aGuard( Mutex::getGlobalMutex() );
    {
        if( !xEventsAttacher.is() )
            xEventsAttacher = new DialogEventAttacher();
    }

    if( !xDialogControl.is() )
        return;

    Reference< XControlContainer > xControlContainer( xDialogControl, UNO_QUERY );
    if( !xControlContainer.is() )
        return;
    Sequence< Reference< XControl > > aControls = xControlContainer->getControls();
    const Reference< XControl >* pControls = aControls.getConstArray();
    sal_Int32 nControlCount = aControls.getLength();
    sal_Int32 nObjectCount = nControlCount + 1;

    Sequence< Reference< XInterface > > aObjectSeq( nObjectCount );
    Reference< XInterface >* pObjects = aObjectSeq.getArray();
    for( sal_Int32 i = 0 ; i < nControlCount ; i++ )
    {
        Reference< XInterface > xIface( pControls[i], UNO_QUERY );
        pObjects[i] = xIface;
    }

    // Also take the DialogControl itself into the sequence
    Reference< XInterface > xDialogIface( xDialogControl, UNO_QUERY );
    pObjects[ nControlCount ] = xDialogIface;

    Reference< XScriptListener > xScriptListener = new BasicScriptListener_Impl( pBasic, xModel );
    Any Helper;
    xEventsAttacher->attachEvents( aObjectSeq, xScriptListener, Helper );
}


//===================================================================

class DialogAllListener_Impl : public WeakImplHelper1< XAllListener >
{
    Reference< XScriptListener >    mxScriptListener;
    OUString                        maScriptType;
    OUString                        maScriptCode;
    Mutex                           maMutex;

    virtual void firing_impl(const AllEventObject& Event, Any* pRet);

public:
    DialogAllListener_Impl( const Reference< XScriptListener >& xListener,
        const OUString &rScriptType, const OUString & rScriptCode );

    // Methoden von XAllListener
    virtual void SAL_CALL firing(const AllEventObject& Event)
        throw( RuntimeException );
    virtual Any SAL_CALL approveFiring(const AllEventObject& Event)
        throw( InvocationTargetException, RuntimeException );

    // Methoden von XEventListener
    virtual void SAL_CALL disposing(const EventObject& Source)
        throw( RuntimeException );
};

//========================================================================

DialogAllListener_Impl::DialogAllListener_Impl( const Reference< XScriptListener >& xListener,
    const OUString &rScriptType, const OUString & rScriptCode )
        : mxScriptListener( xListener )
        , maScriptType( rScriptType )
        , maScriptCode( rScriptCode )
{
}

// Methods XAllListener
void DialogAllListener_Impl::firing( const AllEventObject& Event ) throw ( RuntimeException )
{
    firing_impl( Event, NULL );
}

Any DialogAllListener_Impl::approveFiring( const AllEventObject& Event )
    throw ( InvocationTargetException, RuntimeException )
{
    Any aRetAny;
    firing_impl( Event, &aRetAny );
    return aRetAny;
}

// Methods XEventListener
void DialogAllListener_Impl::disposing(const EventObject& ) throw ( RuntimeException )
{
    // TODO: ???
    //NAMESPACE_VOS(OGuard) guard( Application::GetSolarMutex() );
    //xSbxObj.Clear();
}

void DialogAllListener_Impl::firing_impl( const AllEventObject& Event, Any* pRet )
{
    Guard< Mutex > aGuard( maMutex );

    ScriptEvent aScriptEvent;
    aScriptEvent.Source         = (OWeakObject *)this;  // get correct XInterface
    aScriptEvent.ListenerType   = Event.ListenerType;
    aScriptEvent.MethodName     = Event.MethodName;
    aScriptEvent.Arguments      = Event.Arguments;
    aScriptEvent.Helper         = Event.Helper;
    aScriptEvent.Arguments      = Event.Arguments;
    aScriptEvent.ScriptType     = maScriptType;
    aScriptEvent.ScriptCode     = maScriptCode;

    if( pRet )
        *pRet = mxScriptListener->approveFiring( aScriptEvent );
    else
        mxScriptListener->firing( aScriptEvent );
}


//===================================================================



void SAL_CALL DialogEventAttacher::attachEvents
(
    const Sequence< Reference< XInterface > >& Objects,
    const Reference< XScriptListener >& xListener,
    const Any& Helper
)
    throw(  IllegalArgumentException,
            IntrospectionException,
            CannotCreateAdapterException,
            ServiceNotRegisteredException,
            RuntimeException )
{
    // Get EventAttacher and Introspection (Introspection???)
    {
        Guard< Mutex > aGuard( maMutex );
        if( !mxEventAttacher.is() )
        {
            // AllListenerAdapterService holen
            Reference< XMultiServiceFactory > xSMgr( comphelper::getProcessServiceFactory() );
            if( !xSMgr.is() )
                throw RuntimeException();

            Reference< XInterface > xIFace( xSMgr->createInstance(
                OUString::createFromAscii("com.sun.star.script.EventAttacher") ) );
            if ( xIFace.is() )
            {
                mxEventAttacher = Reference< XEventAttacher >::query( xIFace );
            }
            if( !mxEventAttacher.is() )
                throw ServiceNotRegisteredException();
        }
    }

    // Go over all objects
    const Reference< XInterface >* pObjects = Objects.getConstArray();
    sal_Int32 i, nObjCount = Objects.getLength();
    for( i = 0 ; i < nObjCount ; i++ )
    {
        // We know that we have to do with instances of XControl
        // Otherwise this is not the right implementation for
        // XScriptEventsAttacher and we have to give up
        Reference< XControl > xControl( pObjects[ i ], UNO_QUERY );
        if( !xControl.is() )
            throw IllegalArgumentException();

        // Get XEventsSupplier from ControlModel
        Reference< XControlModel > xControlModel = xControl->getModel();
        Reference< XScriptEventsSupplier > xEventsSupplier( xControlModel, UNO_QUERY );
        if( xEventsSupplier.is() )
        {
            Reference< XNameContainer > xEventCont = xEventsSupplier->getEvents();
            Sequence< OUString > aNames = xEventCont->getElementNames();
            const OUString* pNames = aNames.getConstArray();
            sal_Int32 j, nNameCount = aNames.getLength();

            for( j = 0 ; j < nNameCount ; j++ )
            {
                ScriptEventDescriptor aDesc;

                Any aElement = xEventCont->getByName( pNames[ j ] );
                aElement >>= aDesc;
                Reference< XAllListener > xAllListener =
                    new DialogAllListener_Impl( xListener, aDesc.ScriptType, aDesc.ScriptCode );

                // Try first to attach event to the ControlModel
                sal_Bool bSuccess = sal_False;
                try
                {
                    Reference< XEventListener > xListener = mxEventAttacher->
                        attachSingleEventListener( xControlModel, xAllListener, Helper,
                        aDesc.ListenerType, aDesc.AddListenerParam, aDesc.EventMethod );

                    if( xListener.is() )
                        bSuccess = sal_True;
                }
                catch( IllegalArgumentException& )
                {}
                catch( IntrospectionException& )
                {}
                catch( CannotCreateAdapterException& )
                {}
                catch( ServiceNotRegisteredException& )
                {}
                //{
                    //throw IntrospectionException();
                //}

                try
                {
                // If we had no success, try to attach to the Control
                if( !bSuccess )
                {
                    Reference< XEventListener > xListener;  // Do we need that?!?
                    xListener = mxEventAttacher->attachSingleEventListener
                        ( xControl, xAllListener, Helper, aDesc.ListenerType,
                          aDesc.AddListenerParam, aDesc.EventMethod );
                }
                }
                catch( IllegalArgumentException& )
                {}
                catch( IntrospectionException& )
                {}
                catch( CannotCreateAdapterException& )
                {}
                catch( ServiceNotRegisteredException& )
                {}
            }
        }
    }

}

void RTL_Impl_CreateUnoDialog( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite )
{
    static ::xmlscript::XML_script * s_xmlscript = 0;
    if (0 == s_xmlscript)
        s_xmlscript = ::xmlscript::getXmlScript();

    Reference< XMultiServiceFactory > xMSF( comphelper::getProcessServiceFactory() );
    if( !xMSF.is() )
        return;

    // We need at least 1 parameter
    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // Get dialog
    SbxBaseRef pObj = (SbxBase*)rPar.Get( 1 )->GetObject();
    if( !(pObj && pObj->ISA(SbUnoObject)) )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    Any aAnyISP = ((SbUnoObject*)(SbxBase*)pObj)->getUnoAny();
    TypeClass eType = aAnyISP.getValueType().getTypeClass();

    if( eType != TypeClass_INTERFACE )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // Create new uno dialog
    Reference< XNameContainer > xDialogModel( xMSF->createInstance
        ( OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialogModel" ) ) ),
            UNO_QUERY );
    if( !xDialogModel.is() )
        return;

    Reference< XInputStreamProvider > xISP;
    aAnyISP >>= xISP;
    if( !xISP.is() )
        return;

    Reference< XComponentContext > xContext;
    Reference< XPropertySet > xProps( xMSF, UNO_QUERY );
    OSL_ASSERT( xProps.is() );
    OSL_VERIFY( xProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext")) ) >>= xContext );

    // Import the DialogModel
    Reference< XInputStream > xInput( xISP->createInputStream() );
    s_xmlscript->importDialogModel( xInput, xDialogModel, xContext );

    // Add dialog model to dispose vector
    Reference< XComponent > xDlgComponent( xDialogModel, UNO_QUERY );
    pINST->getComponentVector().push_back( xDlgComponent );

    // Create a "living" Dialog
    Reference< XControl > xDlg( xMSF->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialog" ) ) ), UNO_QUERY );
    Reference< XControlModel > xDlgMod( xDialogModel, UNO_QUERY );
    xDlg->setModel( xDlgMod );
    Reference< XWindow > xW( xDlg, UNO_QUERY );
    xW->setVisible( sal_False );
    Reference< XToolkit > xToolkit( xMSF->createInstance(
        OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.ExtToolkit" ) ) ), UNO_QUERY );
    xDlg->createPeer( xToolkit, NULL );
        StarBASIC* pStartedBasic = pINST->GetBasic();
        // need ThisCompoent from calling script

        OSL_TRACE("About to try get a hold of ThisComponent");
        Reference< frame::XModel > xModel = getModelFromBasic( pStartedBasic ) ;
    attachDialogEvents( pStartedBasic, xModel, xDlg );

    // Return dialog
    Any aRetVal;
    aRetVal <<= xDlg;
    SbxVariableRef refVar = rPar.Get(0);
    unoToSbxValue( (SbxVariable*)refVar, aRetVal );
}


//===================================================================
