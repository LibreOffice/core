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

#include <sal/macros.h>
#include <sal/log.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>

#include <ooo/vba/XVBAToOOEventDescGen.hpp>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/theIntrospection.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>

#include <com/sun/star/frame/XModel.hpp>

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

#include <com/sun/star/script/XScriptListener.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/evtmethodhelper.hxx>

#include <list>
#include <unordered_map>

using namespace ::com::sun::star;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::uno;
using namespace ::ooo::vba;

// Some constants
static const char DELIM[] = "::";
static const sal_Int32 DELIMLEN = strlen(DELIM);

static bool isKeyEventOk( awt::KeyEvent& evt, const Sequence< Any >& params )
{
    return ( params.getLength() > 0 ) && ( params[ 0 ] >>= evt );
}

static bool isMouseEventOk( awt::MouseEvent& evt, const Sequence< Any >& params )
{
    return ( params.getLength() > 0 ) && ( params[ 0 ] >>= evt );
}

static Sequence< Any > ooMouseEvtToVBADblClick( const Sequence< Any >& params )
{
    awt::MouseEvent evt;

    if ( !( isMouseEventOk(evt, params)) ||
        (evt.ClickCount != 2) )
        return Sequence< Any >();
    // give back orig params, this will signal that the event is good
    return params;
}

static Sequence< Any > ooMouseEvtToVBAMouseEvt( const Sequence< Any >& params )
{
    Sequence< Any > translatedParams;
    awt::MouseEvent evt;

    if ( !isMouseEventOk(evt, params) )
        return Sequence< Any >();

    translatedParams.realloc(4);

    // Buttons
    translatedParams[ 0 ] <<= evt.Buttons;
    // Shift
    translatedParams[ 1 ] <<= evt.Modifiers;
    // X
    translatedParams[ 2 ] <<= evt.X;
    // Y
    translatedParams[ 3 ] <<= evt.Y;
    return translatedParams;
}

static Sequence< Any > ooKeyPressedToVBAKeyPressed( const Sequence< Any >& params )
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

static Sequence< Any > ooKeyPressedToVBAKeyUpDown( const Sequence< Any >& params )
{
    Sequence< Any > translatedParams;
    awt::KeyEvent evt;

    if ( !isKeyEventOk( evt, params ) )
        return Sequence< Any >();

    translatedParams.realloc(2);

    Reference< msforms::XReturnInteger> xKeyCode = new ReturnInteger(  evt.KeyCode );
    sal_Int8 shift = sal::static_int_cast<sal_Int8>( evt.Modifiers );

    // #TODO check whether values from OOO conform to values generated from vba
    translatedParams[0] <<= xKeyCode;
    translatedParams[1] <<= shift;
    return  translatedParams;
}

typedef Sequence< Any > (*Translator)(const Sequence< Any >&);

//expand the "TranslateInfo" struct to support more kinds of events
struct TranslateInfo
{
    OUString sVBAName; //vba event name
    Translator toVBA;       //the method to convert OO event parameters to VBA event parameters
    bool (*ApproveRule)(const ScriptEvent& evt, void const * pPara); //this method is used to determine which types of controls should execute the event
    void const *pPara;            //Parameters for the above approve method
};


typedef std::unordered_map<
    OUString,
    std::list< TranslateInfo > > EventInfoHash;


struct TranslatePropMap
{
    OUString const sEventInfo;   //OO event name
    TranslateInfo const aTransInfo;
};

static bool ApproveAll(const ScriptEvent& evt, void const * pPara); //allow all types of controls to execute the event
static bool ApproveType(const ScriptEvent& evt, void const * pPara); //certain types of controls should execute the event, those types are given by pPara
static bool DenyType(const ScriptEvent& evt, void const * pPara);    //certain types of controls should not execute the event, those types are given by pPara
static bool DenyMouseDrag(const ScriptEvent& evt, void const * pPara); //used for VBA MouseMove event when "Shift" key is pressed

struct TypeList
{
    uno::Type const * pTypeList;
    int const nListLength;
};

Type const typeXFixedText = cppu::UnoType<awt::XFixedText>::get();
Type const typeXTextComponent = cppu::UnoType<awt::XTextComponent>::get();
Type const typeXComboBox = cppu::UnoType<awt::XComboBox>::get();
Type const typeXRadioButton = cppu::UnoType<awt::XRadioButton>::get();
Type const typeXListBox = cppu::UnoType<awt::XListBox>::get();


TypeList const fixedTextList = {&typeXFixedText, 1};
TypeList const textCompList = {&typeXTextComponent, 1};
TypeList const radioButtonList = {&typeXRadioButton, 1};
TypeList const comboBoxList = {&typeXComboBox, 1};
TypeList const listBoxList = {&typeXListBox, 1};

//this array stores the OO event to VBA event translation info
static TranslatePropMap aTranslatePropMap_Impl[] =
{
    { OUString("actionPerformed"), { OUString("_Change"), nullptr, DenyType, static_cast<void const *>(&radioButtonList) } },
    // actionPerformed ooo event
    { OUString("actionPerformed"), { OUString("_Click"), nullptr, ApproveAll, nullptr } },
    { OUString("itemStateChanged"), { OUString("_Change"), nullptr, ApproveType, static_cast<void const *>(&radioButtonList) } },
    // itemStateChanged ooo event
    { OUString("itemStateChanged"), { OUString("_Click"), nullptr, ApproveType, static_cast<void const *>(&comboBoxList) } },

    { OUString("itemStateChanged"), { OUString("_Click"), nullptr, ApproveType, static_cast<void const *>(&listBoxList) } },
    // changed ooo event
    { OUString("changed"), { OUString("_Change"), nullptr, ApproveAll, nullptr } },

    // focusGained ooo event
    { OUString("focusGained"), { OUString("_GotFocus"), nullptr, ApproveAll, nullptr } },

    // focusLost ooo event
    { OUString("focusLost"), { OUString("_LostFocus"), nullptr, ApproveAll, nullptr } },
    { OUString("focusLost"), { OUString("_Exit"), nullptr, ApproveType, static_cast<void const *>(&textCompList) } }, // support VBA TextBox_Exit event

    // adjustmentValueChanged ooo event
    { OUString("adjustmentValueChanged"), { OUString("_Scroll"), nullptr, ApproveAll, nullptr } },
    { OUString("adjustmentValueChanged"), { OUString("_Change"), nullptr, ApproveAll, nullptr } },

    // textChanged ooo event
    { OUString("textChanged"), { OUString("_Change"), nullptr, ApproveAll, nullptr } },

    // keyReleased ooo event
    { OUString("keyReleased"), { OUString("_KeyUp"), ooKeyPressedToVBAKeyUpDown, ApproveAll, nullptr } },

    // mouseReleased ooo event
    { OUString("mouseReleased"), { OUString("_Click"), ooMouseEvtToVBAMouseEvt, ApproveType, static_cast<void const *>(&fixedTextList) } },
    { OUString("mouseReleased"), { OUString("_MouseUp"), ooMouseEvtToVBAMouseEvt, ApproveAll, nullptr } },

    // mousePressed ooo event
    { OUString("mousePressed"), { OUString("_MouseDown"), ooMouseEvtToVBAMouseEvt, ApproveAll, nullptr } },
    { OUString("mousePressed"), { OUString("_DblClick"), ooMouseEvtToVBADblClick, ApproveAll, nullptr } },

    // mouseMoved ooo event
    { OUString("mouseMoved"), { OUString("_MouseMove"), ooMouseEvtToVBAMouseEvt, ApproveAll, nullptr } },
    { OUString("mouseDragged"), { OUString("_MouseMove"), ooMouseEvtToVBAMouseEvt, DenyMouseDrag, nullptr } },

    // keyPressed ooo event
    { OUString("keyPressed"), { OUString("_KeyDown"), ooKeyPressedToVBAKeyUpDown, ApproveAll, nullptr } },
    { OUString("keyPressed"), { OUString("_KeyPress"), ooKeyPressedToVBAKeyPressed, ApproveAll, nullptr } }
};

static EventInfoHash& getEventTransInfo()
{
    static EventInfoHash eventTransInfo = [&]()
    {
        EventInfoHash tmp;
        OUString sEventInfo;
        TranslatePropMap* pTransProp = aTranslatePropMap_Impl;
        int nCount = SAL_N_ELEMENTS(aTranslatePropMap_Impl);

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
            tmp[sEventInfo] = infoList;
        }
        return tmp;
    }();
    return eventTransInfo;
}


// Helper class

class ScriptEventHelper
{
public:
    explicit ScriptEventHelper( const Reference< XInterface >& xControl );
    explicit ScriptEventHelper( const OUString& sCntrlServiceName );
    ~ScriptEventHelper();
    Sequence< ScriptEventDescriptor > createEvents( const OUString& sCodeName );
    Sequence< OUString > getEventListeners();
private:
    Reference< XComponentContext > m_xCtx;
    Reference< XInterface > m_xControl;
    bool const m_bDispose;
};

static bool
eventMethodToDescriptor( const OUString& rEventMethod, ScriptEventDescriptor& evtDesc, const OUString& sCodeName )
{
    // format of ControlListener is TypeName::methodname e.g.
    // "com.sun.star.awt.XActionListener::actionPerformed" or
    // "XActionListener::actionPerformed

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

    // Only create an ScriptEventDescriptor for an event we can translate
    // or emulate
    if ( !sMethodName.isEmpty()
         && !sTypeName.isEmpty()
         && ( infos.find( sMethodName ) != infos.end() ) )
    {
        // just fill in CodeName, when the event fires the other
    // info is gathered from the event source to determine what
    // event handler we try to call
        evtDesc.ScriptCode = sCodeName;
        evtDesc.ListenerType = sTypeName;
        evtDesc.EventMethod = sMethodName;

        // set this it VBAInterop, ensures that it doesn't
        // get persisted or shown in property editors
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
    // dispose control ( and remove any associated event registrations )
    if ( m_bDispose )
    {
        try
        {
            uno::Reference< lang::XComponent > xComp( m_xControl, uno::UNO_QUERY_THROW );
            xComp->dispose();
        }
        // destructor can't throw
        catch( uno::Exception& )
        {
        }
    }
}

Sequence< OUString >
ScriptEventHelper::getEventListeners()
{
    std::list< OUString > eventMethods;

    Reference< beans::XIntrospection > xIntrospection = beans::theIntrospection::get( m_xCtx );

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

    return comphelper::containerToSequence(eventMethods);
}

Sequence< ScriptEventDescriptor >
ScriptEventHelper::createEvents( const OUString& sCodeName )
{
    Sequence< OUString > aControlListeners = getEventListeners();
    sal_Int32 nLength = aControlListeners.getLength();

    Sequence< ScriptEventDescriptor > aDest( nLength );
    sal_Int32 nEvts = 0;
    for ( OUString const & i : aControlListeners)
    {
        // from getListeners eventName is of form
        // "com.sun.star.awt.XActionListener::actionPerformed"
        // we need to strip "com.sun.star.awt." from that for form
        // controls
        ScriptEventDescriptor evtDesc;
        if ( eventMethodToDescriptor( i, evtDesc, sCodeName ) )
        {
            sal_Int32 dIndex = nEvts;
            ++nEvts;
            if ( nEvts > aDest.getLength() )
                aDest.realloc( nEvts );// should never happen
            aDest[ dIndex ] = evtDesc;
        }
    }
    aDest.realloc( nEvts );

    return aDest;
}


typedef ::cppu::WeakImplHelper< container::XNameContainer > NameContainer_BASE;

class ReadOnlyEventsNameContainer : public NameContainer_BASE
{
public:
    ReadOnlyEventsNameContainer( const Sequence< OUString >& eventMethods, const OUString& sCodeName );
    // XNameContainer

    virtual void SAL_CALL insertByName( const OUString&, const Any& ) override
    {
        throw RuntimeException("ReadOnly container" );

    }
    virtual void SAL_CALL removeByName( const OUString& ) override
    {
        throw RuntimeException("ReadOnly container" );
    }

    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString&, const Any& ) override
    {
        throw RuntimeException("ReadOnly container" );

    }

    // XNameAccess
    virtual Any SAL_CALL getByName( const OUString& aName ) override;
    virtual Sequence< OUString > SAL_CALL getElementNames(  ) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // XElementAccess
    virtual Type SAL_CALL getElementType(  ) override
    { return cppu::UnoType<OUString>::get(); }
    virtual sal_Bool SAL_CALL hasElements(  ) override
    { return !m_hEvents.empty(); }
private:

typedef std::unordered_map< OUString, Any > EventSupplierHash;

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
ReadOnlyEventsNameContainer::getByName( const OUString& aName ){
    EventSupplierHash::const_iterator it = m_hEvents.find( aName );
    if ( it == m_hEvents.end() )
        throw container::NoSuchElementException();
    return it->second;
}

Sequence< OUString > SAL_CALL
ReadOnlyEventsNameContainer::getElementNames(  )
{
    return comphelper::mapKeysToSequence(m_hEvents);
}

sal_Bool SAL_CALL
ReadOnlyEventsNameContainer::hasByName( const OUString& aName )
{
    EventSupplierHash::const_iterator it = m_hEvents.find( aName );
    if ( it == m_hEvents.end() )
        return false;
    return true;
}

class ReadOnlyEventsSupplier : public ::cppu::WeakImplHelper< XScriptEventsSupplier >
{
public:
    ReadOnlyEventsSupplier( const Sequence< OUString >& eventMethods, const OUString& sCodeName )
    { m_xNameContainer = new ReadOnlyEventsNameContainer( eventMethods, sCodeName ); }

    // XScriptEventSupplier
    virtual Reference< container::XNameContainer > SAL_CALL getEvents(  ) override { return m_xNameContainer; }
private:
    Reference< container::XNameContainer > m_xNameContainer;
};

typedef ::cppu::WeakImplHelper< XScriptListener, util::XCloseListener, lang::XInitialization, css::lang::XServiceInfo > EventListener_BASE;

#define EVENTLSTNR_PROPERTY_ID_MODEL         1
#define EVENTLSTNR_PROPERTY_MODEL            "Model"

class EventListener : public EventListener_BASE
    ,public ::comphelper::OMutexAndBroadcastHelper
    ,public ::comphelper::OPropertyContainer
    ,public ::comphelper::OPropertyArrayUsageHelper< EventListener >
{

public:
    EventListener();
    // XEventListener
    virtual void SAL_CALL disposing(const lang::EventObject& Source) override;
    using cppu::OPropertySetHelper::disposing;

    // XScriptListener
    virtual void SAL_CALL firing(const ScriptEvent& evt) override;
    virtual Any SAL_CALL approveFiring(const ScriptEvent& evt) override;
    // XCloseListener
    virtual void SAL_CALL queryClosing( const lang::EventObject& Source, sal_Bool GetsOwnership ) override;
    virtual void SAL_CALL notifyClosing( const lang::EventObject& Source ) override;
    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) override;
    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()
    virtual void SAL_CALL setFastPropertyValue( sal_Int32 nHandle, const css::uno::Any& rValue ) override
    {
        if ( nHandle == EVENTLSTNR_PROPERTY_ID_MODEL )
        {
            uno::Reference< frame::XModel > xModel( rValue, uno::UNO_QUERY );
            if( xModel != m_xModel)
            {
                // Remove the listener from the old XCloseBroadcaster.
                uno::Reference< util::XCloseBroadcaster > xCloseBroadcaster( m_xModel, uno::UNO_QUERY );
                if (xCloseBroadcaster.is())
                {
                    xCloseBroadcaster->removeCloseListener( this );
                }
                // Add the listener into the new XCloseBroadcaster.
                xCloseBroadcaster.set( xModel, uno::UNO_QUERY );
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

    OUString SAL_CALL getImplementationName() override
    {
        return OUString( "ooo.vba.EventListener"  );
    }

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        const OUString strName( getImplementationName() );
        return Sequence< OUString >( &strName, 1 );
    }

protected:
    // OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper(  ) override;

    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper(  ) const override;

private:
    void setShellFromModel();
    /// @throws RuntimeException
    void firing_Impl( const  ScriptEvent& evt, Any *pSyncRet );

    Reference< frame::XModel > m_xModel;
    bool m_bDocClosed;
    SfxObjectShell* mpShell;
};

EventListener::EventListener() :
OPropertyContainer(GetBroadcastHelper()), m_bDocClosed(false), mpShell( nullptr )
{
    registerProperty( EVENTLSTNR_PROPERTY_MODEL, EVENTLSTNR_PROPERTY_ID_MODEL,
        beans::PropertyAttribute::TRANSIENT, &m_xModel, cppu::UnoType<decltype(m_xModel)>::get() );
}

void
EventListener::setShellFromModel()
{
    // reset mpShell
    mpShell = nullptr;
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
}

//XEventListener
void
EventListener::disposing(const lang::EventObject&)
{
}

//XScriptListener

void SAL_CALL
EventListener::firing(const ScriptEvent& evt)
{
    firing_Impl( evt, nullptr );
}

Any SAL_CALL
EventListener::approveFiring(const ScriptEvent& evt)
{
    Any ret;
    firing_Impl( evt, &ret );
    return ret;
}

// XCloseListener
void SAL_CALL
EventListener::queryClosing( const lang::EventObject& /*Source*/, sal_Bool /*GetsOwnership*/ )
{
    //Nothing to do
}

void SAL_CALL
EventListener::notifyClosing( const lang::EventObject& /*Source*/ )
{
    m_bDocClosed = true;
    uno::Reference< util::XCloseBroadcaster > xCloseBroadcaster( m_xModel, uno::UNO_QUERY );
    if (xCloseBroadcaster.is())
    {
        xCloseBroadcaster->removeCloseListener( this );
    }
}

// XInitialization
void SAL_CALL
EventListener::initialize( const Sequence< Any >& aArguments )
{
    if ( aArguments.getLength() == 1 )
        aArguments[0] >>= m_xModel;
    SAL_INFO(
        "scripting",
        "args " << aArguments.getLength() << " m_xModel " << m_xModel.is());
}

// XInterface

IMPLEMENT_FORWARD_XINTERFACE2( EventListener, EventListener_BASE, OPropertyContainer )

// XTypeProvider

IMPLEMENT_FORWARD_XTYPEPROVIDER2( EventListener, EventListener_BASE, OPropertyContainer )

// OPropertySetHelper

::cppu::IPropertyArrayHelper&
EventListener::getInfoHelper(  )
{
    return *getArrayHelper();
}

// OPropertyArrayUsageHelper

::cppu::IPropertyArrayHelper*
EventListener::createArrayHelper(  ) const
{
    Sequence< beans::Property > aProps;
    describeProperties( aProps );
    return new ::cppu::OPropertyArrayHelper( aProps );
}

// XPropertySet
Reference< beans::XPropertySetInfo >
EventListener::getPropertySetInfo(  )
{
    Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}


//decide if the control should execute the event
bool ApproveAll(SAL_UNUSED_PARAMETER const ScriptEvent&, SAL_UNUSED_PARAMETER void const * )
{
    return true;
}

//for the given control type in evt.Arguments[0], look for if it appears in the type list in pPara
static bool FindControl(const ScriptEvent& evt, void const * pPara)
{
    lang::EventObject aEvent;
    evt.Arguments[ 0 ] >>= aEvent;
    uno::Reference< uno::XInterface > xInterface( aEvent.Source, uno::UNO_QUERY );

    TypeList const * pTypeListInfo = static_cast<TypeList const *>(pPara);
    Type const * pType = pTypeListInfo->pTypeList;
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

//if the given control type in evt.Arguments[0] appears in the type list in pPara, then approve the execution
bool ApproveType(const ScriptEvent& evt, void const * pPara)
{
    return FindControl(evt, pPara);
}

//if the given control type in evt.Arguments[0] appears in the type list in pPara, then deny the execution
bool DenyType(const ScriptEvent& evt, void const * pPara)
{
    return !FindControl(evt, pPara);
}

//when mouse is moving, either the mouse button is pressed or some key is pressed can trigger the OO mouseDragged event,
//the former should be denied, and the latter allowed, only by doing so can the VBA MouseMove event when the "Shift" key is
//pressed can be correctly triggered
bool DenyMouseDrag(const ScriptEvent& evt, SAL_UNUSED_PARAMETER void const * )
{
    awt::MouseEvent aEvent;
    evt.Arguments[ 0 ] >>= aEvent;
    return aEvent.Buttons == 0;
}


// EventListener

void
EventListener::firing_Impl(const ScriptEvent& evt, Any* pRet )
{
    // let default handlers deal with non vba stuff
    if ( evt.ScriptType != "VBAInterop" )
        return;
    lang::EventObject aEvent;
    evt.Arguments[ 0 ] >>= aEvent;
    OUString sName = "UserForm";

    uno::Reference< awt::XDialog > xDlg( aEvent.Source, uno::UNO_QUERY );
    if ( !xDlg.is() )
    {
        // evt.Source is
        // a) Dialog
        // b) xShapeControl ( from api (sheet control) )
        // c) eventmanager ( I guess )
        // d) vba control ( from api also )
        uno::Reference< drawing::XControlShape > xCntrlShape( evt.Source, uno::UNO_QUERY );
        uno::Reference< awt::XControl > xControl( aEvent.Source, uno::UNO_QUERY );
        if ( xCntrlShape.is() )
        {
            // for sheet controls ( that fire from the api ) we don't
            // have the real control ( that's only available from the view )
            // api code creates just a control instance that is transferred
            // via aEvent.Arguments[ 0 ] that control though has no
            // info like name etc.
            uno::Reference< container::XNamed > xName( xCntrlShape->getControl(), uno::UNO_QUERY_THROW );
            sName = xName->getName();
        }
        else
        {
            // Userform control ( fired from the api or from event manager )
            uno::Reference< beans::XPropertySet > xProps;
            xProps.set( xControl->getModel(), uno::UNO_QUERY_THROW );
            xProps->getPropertyValue("Name") >>= sName;
        }
    }
    //dumpEvent( evt );
    EventInfoHash& infos = getEventTransInfo();
    EventInfoHash::const_iterator eventInfo_it = infos.find( evt.MethodName );
    EventInfoHash::const_iterator it_end = infos.end();
    if ( eventInfo_it == it_end )
    {
        SAL_WARN("scripting", "Bogus event for " << evt.ScriptType );
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
        BasicManager* pBasicManager = mpShell->GetBasicManager();
        OUString sProject;
        OUString sScriptCode( evt.ScriptCode );
        // dialogs pass their own library, presence of Dot determines that
        if ( sScriptCode.indexOf( '.' ) == -1 )
        {
            //'Project' is a better default but I want to force failures
            //OUString sMacroLoc("Project");
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
        OUString sMacroLoc = sProject + "." + sScriptCode + ".";

        for (const auto& rTxInfo : eventInfo_it->second)
        {
            // If the document is closed, we should not execute macro.
            if (m_bDocClosed)
            {
                break;
            }

            OUString sTemp = sName.concat( rTxInfo.sVBAName );
            // see if we have a match for the handlerextension
            // where ScriptCode is methodname_handlerextension
            OUString sToResolve = sMacroLoc.concat( sTemp );

            ooo::vba::MacroResolvedInfo aMacroResolvedInfo = ooo::vba::resolveVBAMacro( mpShell, sToResolve );
            if ( aMacroResolvedInfo.mbFound )
            {

                if (! rTxInfo.ApproveRule(evt, rTxInfo.pPara) )
                {
                    continue;
                }

                // !! translate arguments & emulate events where necessary
                Sequence< Any > aArguments;
                if  ( rTxInfo.toVBA )
                {
                    aArguments = rTxInfo.toVBA( evt.Arguments );
                }
                else
                {
                    aArguments = evt.Arguments;
                }
                if ( aArguments.getLength() )
                {
                    // call basic event handlers for event

                    // create script url
                    OUString url = aMacroResolvedInfo.msResolvedMacro;
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
                        SAL_WARN("scripting", "event script raised " << e );
                    }
               }
           }
       }
    }
}

class VBAToOOEventDescGen : public ::cppu::WeakImplHelper< XVBAToOOEventDescGen, css::lang::XServiceInfo >
{
public:
    VBAToOOEventDescGen();

    // XVBAToOOEventDescGen
    virtual Sequence< ScriptEventDescriptor > SAL_CALL getEventDescriptions( const OUString& sCtrlServiceName, const OUString& sCodeName ) override;
    virtual Reference< XScriptEventsSupplier > SAL_CALL getEventSupplier( const Reference< XInterface >& xControl,  const OUString& sCodeName ) override;

    OUString SAL_CALL getImplementationName() override
    {
        return OUString( "ooo.vba.VBAToOOEventDesc"  );
    }

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        const OUString strName( getImplementationName() );
        return Sequence< OUString >( &strName, 1 );
    }

};

VBAToOOEventDescGen::VBAToOOEventDescGen() {}

Sequence< ScriptEventDescriptor > SAL_CALL
VBAToOOEventDescGen::getEventDescriptions( const OUString& sCntrlServiceName, const OUString& sCodeName )
{
    ScriptEventHelper evntHelper( sCntrlServiceName );
    return evntHelper.createEvents( sCodeName );
}

Reference< XScriptEventsSupplier > SAL_CALL
VBAToOOEventDescGen::getEventSupplier( const Reference< XInterface >& xControl, const OUString& sCodeName  )
{
    ScriptEventHelper evntHelper( xControl );
    Reference< XScriptEventsSupplier > xSupplier =
        new ReadOnlyEventsSupplier(
            evntHelper.getEventListeners(), sCodeName ) ;
    return xSupplier;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
ooo_vba_EventListener_get_implementation(css::uno::XComponentContext*,
                                         css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new EventListener);
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
ooo_vba_VBAToOOEventDesc_get_implementation(css::uno::XComponentContext*,
                                            css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new VBAToOOEventDescGen);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
