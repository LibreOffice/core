/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <sal/macros.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>

#include <ooo/vba/XVBAToOOEventDescGen.hpp>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/Introspection.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>

#include <com/sun/star/frame/XModel.hpp>

#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/script/provider/XScriptProviderSupplier.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>

#include <com/sun/star/container/XNamed.hpp>

#include <com/sun/star/drawing/XControlShape.hpp>

#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/awt/XFixedText.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/awt/XComboBox.hpp>
#include <com/sun/star/awt/XRadioButton.hpp>
#include <com/sun/star/awt/XListBox.hpp>

#include <sfx2/objsh.hxx>
#include <basic/sbstar.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbx.hxx>
#include <filter/msfilter/msvbahelper.hxx>
#include <vbahelper/vbareturntypes.hxx>


#include <comphelper/anytostring.hxx>

#include <com/sun/star/script/XScriptListener.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase2.hxx>
#include <comphelper/evtmethodhelper.hxx>

#include <list>
#include <boost/unordered_map.hpp>
#define ASYNC 0




#if ASYNC
#include <vcl/svapp.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::uno;
using namespace ::ooo::vba;


const static OUString DELIM("::");
const static sal_Int32 DELIMLEN = DELIM.getLength();

bool isKeyEventOk( awt::KeyEvent& evt, const Sequence< Any >& params )
{
    if ( !( params.getLength() > 0 ) ||
        !( params[ 0 ] >>= evt ) )
        return false;
    return true;
}

bool isMouseEventOk( awt::MouseEvent& evt, const Sequence< Any >& params )
{
    if ( !( params.getLength() > 0 ) ||
        !( params[ 0 ] >>= evt ) )
        return false;
    return true;
}

Sequence< Any > ooMouseEvtToVBADblClick( const Sequence< Any >& params )
{
    awt::MouseEvent evt;

    if ( !( isMouseEventOk(evt, params)) ||
        (evt.ClickCount != 2) )
        return Sequence< Any >();
    
    return params;
}

Sequence< Any > ooMouseEvtToVBAMouseEvt( const Sequence< Any >& params )
{
    Sequence< Any > translatedParams;
    awt::MouseEvent evt;

    if ( !isMouseEventOk(evt, params) )
        return Sequence< Any >();

    translatedParams.realloc(4);

    
    translatedParams[ 0 ] <<= evt.Buttons;
    
    translatedParams[ 1 ] <<= evt.Modifiers;
    
    translatedParams[ 2 ] <<= evt.X;
    
    translatedParams[ 3 ] <<= evt.Y;
    return translatedParams;
}

Sequence< Any > ooKeyPressedToVBAKeyPressed( const Sequence< Any >& params )
{
    Sequence< Any > translatedParams;
    awt::KeyEvent evt;

    if ( !isKeyEventOk( evt, params ) )
        return Sequence< Any >();

    translatedParams.realloc(1);

    Reference< msforms::XReturnInteger> xKeyCode = new ReturnInteger(  sal_Int32( evt.KeyCode ) );
    translatedParams[0] <<= xKeyCode;
    return  translatedParams;
}

Sequence< Any > ooKeyPressedToVBAKeyUpDown( const Sequence< Any >& params )
{
    Sequence< Any > translatedParams;
    awt::KeyEvent evt;

    if ( !isKeyEventOk( evt, params ) )
        return Sequence< Any >();

    translatedParams.realloc(2);

    Reference< msforms::XReturnInteger> xKeyCode = new ReturnInteger(  evt.KeyCode );
    sal_Int8 shift = sal::static_int_cast<sal_Int8>( evt.Modifiers );

    
    translatedParams[0] <<= xKeyCode;
    translatedParams[1] <<= shift;
    return  translatedParams;
}

typedef Sequence< Any > (*Translator)(const Sequence< Any >&);


struct TranslateInfo
{
    OUString sVBAName; 
    Translator toVBA;       
    bool (*ApproveRule)(const ScriptEvent& evt, void* pPara); 
    void *pPara;            
};


typedef boost::unordered_map< OUString,
std::list< TranslateInfo >,
OUStringHash,
::std::equal_to< OUString > > EventInfoHash;


struct TranslatePropMap
{
    OUString sEventInfo;   
    TranslateInfo aTransInfo;
};

bool ApproveAll(const ScriptEvent& evt, void* pPara); 
bool ApproveType(const ScriptEvent& evt, void* pPara); 
bool DenyType(const ScriptEvent& evt, void* pPara);    
bool DenyMouseDrag(const ScriptEvent& evt, void* pPara); 

struct TypeList
{
    uno::Type* pTypeList;
    int nListLength;
};

Type typeXFixedText = cppu::UnoType<awt::XFixedText>::get();
Type typeXTextComponent = cppu::UnoType<awt::XTextComponent>::get();
Type typeXComboBox = cppu::UnoType<awt::XComboBox>::get();
Type typeXRadioButton = cppu::UnoType<awt::XRadioButton>::get();
Type typeXListBox = cppu::UnoType<awt::XListBox>::get();


TypeList fixedTextList = {&typeXFixedText, 1};
TypeList textCompList = {&typeXTextComponent, 1};
TypeList radioButtonList = {&typeXRadioButton, 1};
TypeList comboBoxList = {&typeXComboBox, 1};
TypeList listBoxList = {&typeXListBox, 1};


static TranslatePropMap aTranslatePropMap_Impl[] =
{
    { OUString("actionPerformed"), { OUString("_Change"), NULL, DenyType, (void*)(&radioButtonList) } },
    
    { OUString("actionPerformed"), { OUString("_Click"), NULL, ApproveAll, NULL } },
    { OUString("itemStateChanged"), { OUString("_Change"), NULL, ApproveType, (void*)(&radioButtonList) } },
    
    { OUString("itemStateChanged"), { OUString("_Click"), NULL, ApproveType, (void*)(&comboBoxList) } },

    { OUString("itemStateChanged"), { OUString("_Click"), NULL, ApproveType, (void*)(&listBoxList) } },
    
    { OUString("changed"), { OUString("_Change"), NULL, ApproveAll, NULL } },

    
    { OUString("focusGained"), { OUString("_GotFocus"), NULL, ApproveAll, NULL } },

    
    { OUString("focusLost"), { OUString("_LostFocus"), NULL, ApproveAll, NULL } },
    { OUString("focusLost"), { OUString("_Exit"), NULL, ApproveType, (void*)(&textCompList) } }, 

    
    { OUString("adjustmentValueChanged"), { OUString("_Scroll"), NULL, ApproveAll, NULL } },
    { OUString("adjustmentValueChanged"), { OUString("_Change"), NULL, ApproveAll, NULL } },

    
    { OUString("textChanged"), { OUString("_Change"), NULL, ApproveAll, NULL } },

    
    { OUString("keyReleased"), { OUString("_KeyUp"), ooKeyPressedToVBAKeyUpDown, ApproveAll, NULL } },

    
    { OUString("mouseReleased"), { OUString("_Click"), ooMouseEvtToVBAMouseEvt, ApproveType, (void*)(&fixedTextList) } },
    { OUString("mouseReleased"), { OUString("_MouseUp"), ooMouseEvtToVBAMouseEvt, ApproveAll, NULL } },

    
    { OUString("mousePressed"), { OUString("_MouseDown"), ooMouseEvtToVBAMouseEvt, ApproveAll, NULL } },
    { OUString("mousePressed"), { OUString("_DblClick"), ooMouseEvtToVBADblClick, ApproveAll, NULL } },

    
    { OUString("mouseMoved"), { OUString("_MouseMove"), ooMouseEvtToVBAMouseEvt, ApproveAll, NULL } },
    { OUString("mouseDragged"), { OUString("_MouseMove"), ooMouseEvtToVBAMouseEvt, DenyMouseDrag, NULL } },

    
    { OUString("keyPressed"), { OUString("_KeyDown"), ooKeyPressedToVBAKeyUpDown, ApproveAll, NULL } },
    { OUString("keyPressed"), { OUString("_KeyPress"), ooKeyPressedToVBAKeyPressed, ApproveAll, NULL } }
};

EventInfoHash& getEventTransInfo()
{
    static bool initialised = false;
    static EventInfoHash eventTransInfo;
    if ( !initialised )
    {
        OUString sEventInfo;
        TranslatePropMap* pTransProp = aTranslatePropMap_Impl;
        int nCount = sizeof(aTranslatePropMap_Impl) / sizeof(aTranslatePropMap_Impl[0]);

        int i = 0;
        while (i < nCount)
        {
            sEventInfo = pTransProp->sEventInfo;
            std::list< TranslateInfo > infoList;
            do
            {
                infoList.push_back( pTransProp->aTransInfo );
                pTransProp++;
                i++;
            }while(i < nCount && sEventInfo == pTransProp->sEventInfo);
            eventTransInfo[sEventInfo] = infoList;
        }
        initialised = true;
    }
    return eventTransInfo;
}




class ScriptEventHelper
{
public:
    ScriptEventHelper( const Reference< XInterface >& xControl );
    ScriptEventHelper( const OUString& sCntrlServiceName );
    ~ScriptEventHelper();
    Sequence< ScriptEventDescriptor > createEvents( const OUString& sCodeName );
    Sequence< OUString > getEventListeners();
private:
    Reference< XComponentContext > m_xCtx;
    Reference< XInterface > m_xControl;
    bool m_bDispose;
};

bool
eventMethodToDescriptor( const OUString& rEventMethod, ScriptEventDescriptor& evtDesc, const OUString& sCodeName )
{
    
    
    

    OUString sMethodName;
    OUString sTypeName;
    sal_Int32 nDelimPos = rEventMethod.indexOf( DELIM );
    if ( nDelimPos == -1 )
    {
        return false;
    }
    sMethodName = rEventMethod.copy( nDelimPos + DELIMLEN );
    sTypeName = rEventMethod.copy( 0, nDelimPos );

    EventInfoHash& infos = getEventTransInfo();

    
    
    if ( !sMethodName.isEmpty()
         && !sTypeName.isEmpty()
         && ( infos.find( sMethodName ) != infos.end() ) )
    {
        
    
    
        evtDesc.ScriptCode = sCodeName;
        evtDesc.ListenerType = sTypeName;
        evtDesc.EventMethod = sMethodName;

        
        
        evtDesc.ScriptType = "VBAInterop";
        return true;
    }
    return false;

}

ScriptEventHelper::ScriptEventHelper( const Reference< XInterface >& xControl ) :
    m_xCtx( comphelper::getProcessComponentContext() ),
    m_xControl( xControl ),
    m_bDispose( false )
{}

ScriptEventHelper::ScriptEventHelper( const OUString& sCntrlServiceName ) :
    m_xCtx( comphelper::getProcessComponentContext() ),
    m_bDispose( true )
{
   m_xControl.set( m_xCtx->getServiceManager()->createInstanceWithContext( sCntrlServiceName, m_xCtx ), uno::UNO_QUERY );
}

ScriptEventHelper::~ScriptEventHelper()
{
    
    if ( m_bDispose )
    {
        try
        {
            uno::Reference< lang::XComponent > xComp( m_xControl, uno::UNO_QUERY_THROW );
            xComp->dispose();
        }
        
        catch( uno::Exception& )
        {
        }
    }
}

Sequence< OUString >
ScriptEventHelper::getEventListeners()
{
    std::list< OUString > eventMethods;

    Reference< beans::XIntrospection > xIntrospection = beans::Introspection::create( m_xCtx );

    Reference< beans::XIntrospectionAccess > xIntrospectionAccess =
        xIntrospection->inspect( makeAny( m_xControl ) );
    Sequence< Type > aControlListeners =
        xIntrospectionAccess->getSupportedListeners();
    sal_Int32 nLength = aControlListeners.getLength();
    for ( sal_Int32 i = 0; i< nLength; ++i )
    {
        Type& listType = aControlListeners[ i ];
        OUString sFullTypeName = listType.getTypeName();
        Sequence< OUString > sMeths =
            comphelper::getEventMethodsForType( listType );
        sal_Int32 sMethLen = sMeths.getLength();
        for ( sal_Int32 j=0 ; j < sMethLen; ++j )
        {
            OUString sEventMethod = sFullTypeName;
            sEventMethod += DELIM;
            sEventMethod += sMeths[ j ];
            eventMethods.push_back( sEventMethod );
        }
    }

    Sequence< OUString > sEventMethodNames( eventMethods.size() );
    std::list< OUString >::const_iterator it = eventMethods.begin();
    OUString* pDest = sEventMethodNames.getArray();

    for ( ; it != eventMethods.end(); ++it, ++pDest )
        *pDest = *it;

    return sEventMethodNames;
}

Sequence< ScriptEventDescriptor >
ScriptEventHelper::createEvents( const OUString& sCodeName )
{
    Sequence< OUString > aControlListeners = getEventListeners();
    OUString* pSrc = aControlListeners.getArray();
    sal_Int32 nLength = aControlListeners.getLength();

    Sequence< ScriptEventDescriptor > aDest( nLength );
    sal_Int32 nEvts = 0;
    for ( sal_Int32 i = 0; i< nLength; ++i, ++pSrc )
    {
        
        
        
        
        ScriptEventDescriptor evtDesc;
        if ( eventMethodToDescriptor( *pSrc, evtDesc, sCodeName ) )
        {
            sal_Int32 dIndex = nEvts;
            ++nEvts;
            if ( nEvts > aDest.getLength() )
                aDest.realloc( nEvts );
            aDest[ dIndex ] = evtDesc;
        }
    }
    aDest.realloc( nEvts );

    return aDest;
}


typedef ::cppu::WeakImplHelper1< container::XNameContainer > NameContainer_BASE;

class ReadOnlyEventsNameContainer : public NameContainer_BASE
{
public:
    ReadOnlyEventsNameContainer( const Sequence< OUString >& eventMethods, const OUString& sCodeName );
    

    virtual void SAL_CALL insertByName( const OUString&, const Any& ) throw (lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, RuntimeException)
    {
        throw RuntimeException("ReadOnly container", Reference< XInterface >() );

    }
    virtual void SAL_CALL removeByName( const OUString& ) throw (::com::sun::star::container::NoSuchElementException, lang::WrappedTargetException, RuntimeException)
    {
        throw RuntimeException("ReadOnly container", Reference< XInterface >() );
    }

    
    virtual void SAL_CALL replaceByName( const OUString&, const Any& ) throw (lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, RuntimeException)
    {
        throw RuntimeException("ReadOnly container", Reference< XInterface >() );

    }

    
    virtual Any SAL_CALL getByName( const OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, RuntimeException);
    virtual Sequence< OUString > SAL_CALL getElementNames(  ) throw (RuntimeException);
    virtual ::sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (RuntimeException);

    
    virtual Type SAL_CALL getElementType(  ) throw (RuntimeException)
    { return getCppuType(static_cast< const OUString * >(0) ); }
    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (RuntimeException)
    { return ( ( m_hEvents.size() > 0 ? sal_True : sal_False ) ); }
private:

typedef boost::unordered_map< OUString, Any, OUStringHash,
::std::equal_to< OUString > > EventSupplierHash;

    EventSupplierHash m_hEvents;
};

ReadOnlyEventsNameContainer::ReadOnlyEventsNameContainer( const Sequence< OUString >& eventMethods, const OUString& sCodeName )
{
    const OUString* pSrc = eventMethods.getConstArray();
    sal_Int32 nLen = eventMethods.getLength();
    for ( sal_Int32 index = 0; index < nLen; ++index, ++pSrc )
    {
        Any aDesc;
        ScriptEventDescriptor evtDesc;
        if (  eventMethodToDescriptor( *pSrc, evtDesc, sCodeName ) )
        {
            aDesc <<= evtDesc;
            m_hEvents[ *pSrc ] = aDesc;
        }
    }
}

Any SAL_CALL
ReadOnlyEventsNameContainer::getByName( const OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, RuntimeException){
    EventSupplierHash::const_iterator it = m_hEvents.find( aName );
    if ( it == m_hEvents.end() )
        throw container::NoSuchElementException();
    return it->second;
}

Sequence< OUString > SAL_CALL
ReadOnlyEventsNameContainer::getElementNames(  ) throw (RuntimeException)
{
    Sequence< OUString > names(m_hEvents.size());
    OUString* pDest = names.getArray();
    EventSupplierHash::const_iterator it = m_hEvents.begin();
    EventSupplierHash::const_iterator it_end = m_hEvents.end();
    for ( sal_Int32 index = 0; it != it_end; ++index, ++pDest, ++it )
        *pDest = it->first;
    return names;
}

sal_Bool SAL_CALL
ReadOnlyEventsNameContainer::hasByName( const OUString& aName ) throw (RuntimeException)
{
    EventSupplierHash::const_iterator it = m_hEvents.find( aName );
    if ( it == m_hEvents.end() )
        return sal_False;
    return sal_True;
}

typedef ::cppu::WeakImplHelper1< XScriptEventsSupplier > EventsSupplier_BASE;

class ReadOnlyEventsSupplier : public EventsSupplier_BASE
{
public:
    ReadOnlyEventsSupplier( const Sequence< OUString >& eventMethods, const OUString& sCodeName )
    { m_xNameContainer = new ReadOnlyEventsNameContainer( eventMethods, sCodeName ); }

    
    virtual Reference< container::XNameContainer > SAL_CALL getEvents(  ) throw (RuntimeException){ return m_xNameContainer; }
private:
    Reference< container::XNameContainer > m_xNameContainer;
};

typedef ::cppu::WeakImplHelper3< XScriptListener, util::XCloseListener, lang::XInitialization > EventListener_BASE;

#define EVENTLSTNR_PROPERTY_ID_MODEL         1
#define EVENTLSTNR_PROPERTY_MODEL            OUString( "Model"  )

class EventListener : public EventListener_BASE
    ,public ::comphelper::OMutexAndBroadcastHelper
    ,public ::comphelper::OPropertyContainer
    ,public ::comphelper::OPropertyArrayUsageHelper< EventListener >

{

public:
    EventListener( const Reference< XComponentContext >& rxContext );
    
    virtual void SAL_CALL disposing(const lang::EventObject& Source) throw( RuntimeException );
    using cppu::OPropertySetHelper::disposing;

    
    virtual void SAL_CALL firing(const ScriptEvent& evt) throw(RuntimeException);
    virtual Any SAL_CALL approveFiring(const ScriptEvent& evt) throw(reflection::InvocationTargetException, RuntimeException);
    
    virtual void SAL_CALL queryClosing( const lang::EventObject& Source, ::sal_Bool GetsOwnership ) throw (util::CloseVetoException, uno::RuntimeException);
    virtual void SAL_CALL notifyClosing( const lang::EventObject& Source ) throw (uno::RuntimeException);
    
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (::com::sun::star::uno::RuntimeException);
    
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException);
    
    DECLARE_XINTERFACE()

    
    DECLARE_XTYPEPROVIDER()
    virtual void SAL_CALL setFastPropertyValue( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
    {
        if ( nHandle == EVENTLSTNR_PROPERTY_ID_MODEL )
        {
            uno::Reference< frame::XModel > xModel( rValue, uno::UNO_QUERY );
            if( xModel != m_xModel)
            {
                
                uno::Reference< util::XCloseBroadcaster > xCloseBroadcaster( m_xModel, uno::UNO_QUERY );
                if (xCloseBroadcaster.is())
                {
                    xCloseBroadcaster->removeCloseListener( this );
                }
                
                xCloseBroadcaster = uno::Reference< util::XCloseBroadcaster >( xModel, uno::UNO_QUERY );
                if (xCloseBroadcaster.is())
                {
                    xCloseBroadcaster->addCloseListener( this );
                }
            }
        }
        OPropertyContainer::setFastPropertyValue( nHandle, rValue );
    if ( nHandle == EVENTLSTNR_PROPERTY_ID_MODEL )
            setShellFromModel();
    }

protected:
    
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper(  );

    
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper(  ) const;

private:
#if ASYNC
    DECL_LINK( OnAsyncScriptEvent, ScriptEvent* );
#endif
    void setShellFromModel();
    void firing_Impl( const  ScriptEvent& evt, Any *pSyncRet=NULL ) throw( RuntimeException );

    Reference< XComponentContext > m_xContext;
    Reference< frame::XModel > m_xModel;
    bool m_bDocClosed;
    SfxObjectShell* mpShell;
    OUString msProject;
};

EventListener::EventListener( const Reference< XComponentContext >& rxContext ) :
OPropertyContainer(GetBroadcastHelper()), m_xContext( rxContext ), m_bDocClosed(false), mpShell( 0 )
{
    registerProperty( EVENTLSTNR_PROPERTY_MODEL, EVENTLSTNR_PROPERTY_ID_MODEL,
        beans::PropertyAttribute::TRANSIENT, &m_xModel, ::getCppuType( &m_xModel ) );
    msProject = "Standard";
}

void
EventListener::setShellFromModel()
{
    
    mpShell = 0;
    SfxObjectShell* pShell = SfxObjectShell::GetFirst();
    while ( m_xModel.is() && pShell )
    {
        if ( pShell->GetModel() == m_xModel )
        {
            mpShell = pShell;
            break;
        }
        pShell = SfxObjectShell::GetNext( *pShell );
    }
    
    try
    {
        uno::Reference< beans::XPropertySet > xProps( m_xModel, UNO_QUERY_THROW );
        uno::Reference< script::vba::XVBACompatibility > xVBAMode( xProps->getPropertyValue("BasicLibraries"), uno::UNO_QUERY_THROW );
        msProject = xVBAMode->getProjectName();
    }
    catch ( uno::Exception& ) {}
}


void
EventListener::disposing(const lang::EventObject&)  throw( RuntimeException )
{
}



void SAL_CALL
EventListener::firing(const ScriptEvent& evt) throw(RuntimeException)
{
#if ASYNC
    
    
    acquire();
    Application::PostUserEvent( LINK( this, EventListener, OnAsyncScriptEvent ), new ScriptEvent( evt ) );
#else
    firing_Impl( evt );
#endif
}

#if ASYNC
IMPL_LINK( EventListener, OnAsyncScriptEvent, ScriptEvent*, _pEvent )
{
    if ( !_pEvent )
        return 1L;

    {
        
        
        

        
        
        firing_Impl( *_pEvent, NULL );
    }

    delete _pEvent;
    
    release();
    return 0L;
 }
#endif

Any SAL_CALL
EventListener::approveFiring(const ScriptEvent& evt) throw(reflection::InvocationTargetException, RuntimeException)
{
    Any ret;
    firing_Impl( evt, &ret );
    return ret;
}


void SAL_CALL
EventListener::queryClosing( const lang::EventObject& /*Source*/, ::sal_Bool /*GetsOwnership*/ ) throw (util::CloseVetoException, uno::RuntimeException)
{
    
}

void SAL_CALL
EventListener::notifyClosing( const lang::EventObject& /*Source*/ ) throw (uno::RuntimeException)
{
    m_bDocClosed = true;
    uno::Reference< util::XCloseBroadcaster > xCloseBroadcaster( m_xModel, uno::UNO_QUERY );
    if (xCloseBroadcaster.is())
    {
        xCloseBroadcaster->removeCloseListener( this );
    }
}


void SAL_CALL
EventListener::initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException)
{
    if ( aArguments.getLength() == 1 )
        aArguments[0] >>= m_xModel;
    OSL_TRACE("EventListener::initialize() args %d m_xModel %d", aArguments.getLength(), m_xModel.is() );
}



IMPLEMENT_FORWARD_XINTERFACE2( EventListener, EventListener_BASE, OPropertyContainer )



IMPLEMENT_FORWARD_XTYPEPROVIDER2( EventListener, EventListener_BASE, OPropertyContainer )



::cppu::IPropertyArrayHelper&
EventListener::getInfoHelper(  )
{
    return *getArrayHelper();
}



::cppu::IPropertyArrayHelper*
EventListener::createArrayHelper(  ) const
{
    Sequence< beans::Property > aProps;
    describeProperties( aProps );
    return new ::cppu::OPropertyArrayHelper( aProps );
}


Reference< beans::XPropertySetInfo >
EventListener::getPropertySetInfo(  ) throw (RuntimeException)
{
    Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}



bool ApproveAll(const ScriptEvent&, void* )
{
    return true;
}


bool FindControl(const ScriptEvent& evt, void* pPara)
{
    lang::EventObject aEvent;
    evt.Arguments[ 0 ] >>= aEvent;
    uno::Reference< uno::XInterface > xInterface( aEvent.Source, uno::UNO_QUERY );

    TypeList* pTypeListInfo = static_cast<TypeList*>(pPara);
    Type* pType = pTypeListInfo->pTypeList;
    int nLen = pTypeListInfo->nListLength;

    for (int i = 0; i < nLen; i++)
    {
        if ( xInterface->queryInterface( *pType ).hasValue() )
        {
            return true;
        }
        pType++;
    }

    return false;
}


bool ApproveType(const ScriptEvent& evt, void* pPara)
{
    return FindControl(evt, pPara);
}


bool DenyType(const ScriptEvent& evt, void* pPara)
{
    return !FindControl(evt, pPara);
}




bool DenyMouseDrag(const ScriptEvent& evt, void* )
{
    awt::MouseEvent aEvent;
    evt.Arguments[ 0 ] >>= aEvent;
    if (aEvent.Buttons == 0 )
    {
        return true;
    }
    else
    {
        return false;
    }
}




void
EventListener::firing_Impl(const ScriptEvent& evt, Any* pRet ) throw(RuntimeException)
{
    OSL_TRACE("EventListener::firing_Impl( FAKE VBA_EVENTS )");
    static const OUString vbaInterOp =
        OUString("VBAInterop");

    
    if ( !evt.ScriptType.equals( vbaInterOp ) )
        return;
    lang::EventObject aEvent;
    evt.Arguments[ 0 ] >>= aEvent;
    OSL_TRACE("evt.MethodName is  %s", OUStringToOString( evt.MethodName, RTL_TEXTENCODING_UTF8 ).getStr() );
    OSL_TRACE("Argument[0] is  %s", OUStringToOString( comphelper::anyToString( evt.Arguments[0] ), RTL_TEXTENCODING_UTF8 ).getStr() );
    OSL_TRACE("Getting Control");
    OUString sName = "UserForm";
    OSL_TRACE("Getting Name");

    uno::Reference< awt::XDialog > xDlg( aEvent.Source, uno::UNO_QUERY );
    if ( !xDlg.is() )
    {
        OSL_TRACE("Getting Control");
        
        
        
        
        
        uno::Reference< drawing::XControlShape > xCntrlShape( evt.Source, uno::UNO_QUERY );
        uno::Reference< awt::XControl > xControl( aEvent.Source, uno::UNO_QUERY );
        if ( xCntrlShape.is() )
        {
            
            
            
            
            
            OSL_TRACE("Got control shape");
            uno::Reference< container::XNamed > xName( xCntrlShape->getControl(), uno::UNO_QUERY_THROW );
            OSL_TRACE("Got xnamed ");
            sName = xName->getName();
        }
        else
        {
            
            uno::Reference< beans::XPropertySet > xProps;
            OSL_TRACE("Getting properties");
            xProps.set( xControl->getModel(), uno::UNO_QUERY_THROW );
            xProps->getPropertyValue("Name") >>= sName;
        }
    }
    
    EventInfoHash& infos = getEventTransInfo();
    EventInfoHash::const_iterator eventInfo_it = infos.find( evt.MethodName );
    EventInfoHash::const_iterator it_end = infos.end();
    if ( eventInfo_it == it_end )
    {
        OSL_TRACE("Bogus event for %s",
            OUStringToOString( evt.ScriptType, RTL_TEXTENCODING_UTF8 ).getStr() );
        return;
    }

    uno::Reference< script::provider::XScriptProviderSupplier > xSPS( m_xModel, uno::UNO_QUERY );
    uno::Reference< script::provider::XScriptProvider > xScriptProvider;
    if ( xSPS.is() )
    {
        xScriptProvider =  xSPS->getScriptProvider();
    }
    if ( xScriptProvider.is() && mpShell )
    {
        std::list< TranslateInfo >::const_iterator txInfo =
            eventInfo_it->second.begin();
        std::list< TranslateInfo >::const_iterator txInfo_end = eventInfo_it->second.end();

        BasicManager* pBasicManager = mpShell->GetBasicManager();
        OUString sProject;
        OUString sScriptCode( evt.ScriptCode );
        
        if ( sScriptCode.indexOf( '.' ) == -1 )
        {
            
            
            sProject = "Standard";

            if (!pBasicManager->GetName().isEmpty())
            {
                sProject =  pBasicManager->GetName();
            }
        }
        else
        {
            sal_Int32 nIndex = sScriptCode.indexOf( '.' );
            sProject = sScriptCode.copy( 0, nIndex );
            sScriptCode = sScriptCode.copy( nIndex + 1 );
        }
        OUString sMacroLoc = sProject;
        sMacroLoc = sMacroLoc.concat(  OUString(".") );
        sMacroLoc = sMacroLoc.concat( sScriptCode ).concat( OUString(".") );

        OSL_TRACE("sMacroLoc is %s", OUStringToOString( sMacroLoc, RTL_TEXTENCODING_UTF8 ).getStr() );
        for ( ; txInfo != txInfo_end; ++txInfo )
        {
            
            if (m_bDocClosed)
            {
                break;
            }

            OUString sTemp = sName.concat( (*txInfo).sVBAName );
            
            
            OUString sToResolve = sMacroLoc.concat( sTemp );

            OSL_TRACE("*** trying to invoke %s ",
                OUStringToOString( sToResolve, RTL_TEXTENCODING_UTF8 ).getStr() );
            ooo::vba::MacroResolvedInfo aMacroResolvedInfo = ooo::vba::resolveVBAMacro( mpShell, sToResolve );
            if ( aMacroResolvedInfo.mbFound )
            {

                if (! txInfo->ApproveRule(evt, txInfo->pPara) )
                {
                    continue;
                }

                
                Sequence< Any > aArguments;
                if  ( (*txInfo).toVBA )
                {
                    aArguments = (*txInfo).toVBA( evt.Arguments );
                }
                else
                {
                    aArguments = evt.Arguments;
                }
                if ( aArguments.getLength() )
                {
                    

                    
                    OUString url = aMacroResolvedInfo.msResolvedMacro;

                    OSL_TRACE("resolved script = %s",
                        OUStringToOString( url,
                            RTL_TEXTENCODING_UTF8 ).getStr() );
                    try
                    {
                        uno::Any aDummyCaller = uno::makeAny( OUString("Error") );
                        if ( pRet )
                        {
                            ooo::vba::executeMacro( mpShell, url, aArguments, *pRet, aDummyCaller );
                        }
                        else
                        {
                            uno::Any aRet;
                            ooo::vba::executeMacro( mpShell, url, aArguments, aRet, aDummyCaller );
                        }
                    }
                    catch ( uno::Exception& e )
                    {
                        OSL_TRACE("event script raised %s", OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
                    }
               }
           }
       }
    }
}

typedef ::cppu::WeakImplHelper1< XVBAToOOEventDescGen > VBAToOOEventDescGen_BASE;


class VBAToOOEventDescGen : public VBAToOOEventDescGen_BASE
{
public:
    VBAToOOEventDescGen( const Reference< XComponentContext >& rxContext );

    
    virtual Sequence< ScriptEventDescriptor > SAL_CALL getEventDescriptions( const OUString& sCtrlServiceName, const OUString& sCodeName ) throw (RuntimeException);
    virtual Reference< XScriptEventsSupplier > SAL_CALL getEventSupplier( const Reference< XInterface >& xControl,  const OUString& sCodeName ) throw (::com::sun::star::uno::RuntimeException);
private:
    Reference< XComponentContext > m_xContext;

};

VBAToOOEventDescGen::VBAToOOEventDescGen( const Reference< XComponentContext >& rxContext ):m_xContext( rxContext ) {}

Sequence< ScriptEventDescriptor > SAL_CALL
VBAToOOEventDescGen::getEventDescriptions( const OUString& sCntrlServiceName, const OUString& sCodeName ) throw (RuntimeException)
{
    ScriptEventHelper evntHelper( sCntrlServiceName );
    return evntHelper.createEvents( sCodeName );
}

Reference< XScriptEventsSupplier > SAL_CALL
VBAToOOEventDescGen::getEventSupplier( const Reference< XInterface >& xControl, const OUString& sCodeName  ) throw (::com::sun::star::uno::RuntimeException)
{
    ScriptEventHelper evntHelper( xControl );
    Reference< XScriptEventsSupplier > xSupplier =
        new ReadOnlyEventsSupplier(
            evntHelper.getEventListeners(), sCodeName ) ;
    return xSupplier;
}



namespace evtlstner
{
    OUString SAL_CALL getImplementationName()
    {
        static OUString* pImplName = 0;
        if ( !pImplName )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pImplName )
            {
                static OUString aImplName( "ooo.vba.EventListener"  );
                pImplName = &aImplName;
            }
        }
        return *pImplName;
    }

    uno::Reference< XInterface > SAL_CALL create(
    Reference< XComponentContext > const & xContext )
    SAL_THROW(())
    {
        return static_cast< lang::XTypeProvider * >( new EventListener( xContext ) );
    }

    Sequence< OUString > SAL_CALL getSupportedServiceNames()
    {
        const OUString strName( ::evtlstner::getImplementationName() );
        return Sequence< OUString >( &strName, 1 );
    }
}
namespace ooevtdescgen
{
    OUString SAL_CALL getImplementationName()
    {
        static OUString* pImplName = 0;
        if ( !pImplName )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pImplName )
            {
                static OUString aImplName( "ooo.vba.VBAToOOEventDesc"  );
                pImplName = &aImplName;
            }
        }
        return *pImplName;
    }

    uno::Reference< XInterface > SAL_CALL create(
        Reference< XComponentContext > const & xContext )
        SAL_THROW(())
    {
        return static_cast< lang::XTypeProvider * >( new VBAToOOEventDescGen( xContext ) );
    }

    Sequence< OUString > SAL_CALL getSupportedServiceNames()
    {
        const OUString strName( ::ooevtdescgen::getImplementationName() );
        return Sequence< OUString >( &strName, 1 );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
