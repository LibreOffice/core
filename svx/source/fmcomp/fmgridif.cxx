/*************************************************************************
 *
 *  $RCSfile: fmgridif.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-11 12:32:27 $
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
#pragma hdrstop

#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
#endif

#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCOMPONENT_HPP_
#include <com/sun/star/form/XFormComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif
#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif

#ifndef _SVX_FMPROP_HXX
#include "fmprop.hxx"
#endif

#ifndef _SVX_FMSERVS_HXX
#include "fmservs.hxx"
#endif

#ifndef _SVX_FMGRIDIF_HXX
#include "fmgridif.hxx"
#endif

#ifndef _SVX_FMGRIDCL_HXX
#include "fmgridcl.hxx"
#endif

#ifndef _SVX_GRIDCELL_HXX
#include "gridcell.hxx"
#endif

#ifndef _SVX_FMURL_HXX
#include "fmurl.hxx"
#endif

#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif

#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/helper/vclunohelper.hxx>
#endif

#ifndef _UTL_CONTAINER_HXX_
#include <unotools/container.hxx>
#endif
#ifndef _UNOTOOLS_ENUMHELPER_HXX_
#include <unotools/enumhelper.hxx>
#endif
#ifndef _UTL_PROPERTY_HXX_
#include <unotools/property.hxx>
#endif
#ifndef _UTL_TYPES_HXX_
#include <unotools/types.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif

#ifndef _FM_IMPLEMENTATION_IDS_HXX_
#include "fmimplids.hxx"
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif

//------------------------------------------------------------------
::com::sun::star::awt::FontDescriptor ImplCreateFontDescriptor( const Font& rFont )
{
    ::com::sun::star::awt::FontDescriptor aFD;
    aFD.Name = rFont.GetName();
    aFD.StyleName = rFont.GetStyleName();
    aFD.Height = rFont.GetSize().Height();
    aFD.Width = rFont.GetSize().Width();
    aFD.Family = rFont.GetFamily();
    aFD.CharSet = rFont.GetCharSet();
    aFD.Pitch = rFont.GetPitch();
    aFD.CharacterWidth = VCLUnoHelper::ConvertFontWidth( rFont.GetWidthType() );
    aFD.Weight= VCLUnoHelper::ConvertFontWeight( rFont.GetWeight() );
    aFD.Slant = (::com::sun::star::awt::FontSlant)rFont.GetItalic();
    aFD.Underline = rFont.GetUnderline();
    aFD.Strikeout = rFont.GetStrikeout();
    aFD.Orientation = rFont.GetOrientation();
    aFD.Kerning = rFont.IsKerning();
    aFD.WordLineMode = rFont.IsWordLineMode();
    aFD.Type = 0;   // ??? => Nur an Metric...
    return aFD;
}

//------------------------------------------------------------------
Font ImplCreateFont( const ::com::sun::star::awt::FontDescriptor& rDescr )
{
    Font aFont;
    aFont.SetName( rDescr.Name );
    aFont.SetStyleName( rDescr.StyleName );
    aFont.SetSize( Size( rDescr.Width, rDescr.Height ) );
    aFont.SetFamily( (FontFamily)rDescr.Family );
    aFont.SetCharSet( (CharSet)rDescr.CharSet );
    aFont.SetPitch( (FontPitch)rDescr.Pitch );
    aFont.SetWidthType( VCLUnoHelper::ConvertFontWidth( rDescr.CharacterWidth ) );
    aFont.SetWeight( VCLUnoHelper::ConvertFontWeight( rDescr.Weight ) );
    aFont.SetItalic( (FontItalic)rDescr.Slant );
    aFont.SetUnderline( (FontUnderline)rDescr.Underline );
    aFont.SetStrikeout( (FontStrikeout)rDescr.Strikeout );
    aFont.SetOrientation( rDescr.Orientation );
    aFont.SetKerning( rDescr.Kerning );
    aFont.SetWordLineMode( rDescr.WordLineMode );
    return aFont;
}

//------------------------------------------------------------------
FmXModifyMultiplexer::FmXModifyMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex )
                    :OWeakSubObject( rSource )
                    ,OInterfaceContainerHelper(rMutex)
{
}

//------------------------------------------------------------------
::com::sun::star::uno::Any  SAL_CALL FmXModifyMultiplexer::queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aReturn;
    aReturn = ::cppu::queryInterface(_rType,
        static_cast< ::com::sun::star::util::XModifyListener*>(this),
        static_cast< ::com::sun::star::lang::XEventListener*>(this)
    );

    if (!aReturn.hasValue())
        aReturn = OWeakSubObject::queryInterface( _rType );

    return aReturn;
}

//------------------------------------------------------------------
void FmXModifyMultiplexer::disposing(const ::com::sun::star::lang::EventObject& ) throw( ::com::sun::star::uno::RuntimeException )
{
}

//------------------------------------------------------------------
void FmXModifyMultiplexer::modified(const ::com::sun::star::lang::EventObject& e)
{
    ::com::sun::star::lang::EventObject aMulti( e);
    aMulti.Source = &m_rParent;
    NOTIFY_LISTENERS((*this), ::com::sun::star::util::XModifyListener, modified, aMulti);
}

//------------------------------------------------------------------
FmXUpdateMultiplexer::FmXUpdateMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex )
                    :OWeakSubObject( rSource )
                    ,OInterfaceContainerHelper(rMutex)
{
}

//------------------------------------------------------------------
::com::sun::star::uno::Any  SAL_CALL FmXUpdateMultiplexer::queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aReturn;
    aReturn = ::cppu::queryInterface(_rType,
        static_cast< ::com::sun::star::form::XUpdateListener*>(this),
        static_cast< ::com::sun::star::lang::XEventListener*>(this)
    );

    if (!aReturn.hasValue())
        aReturn = OWeakSubObject::queryInterface( _rType );

    return aReturn;
}

//------------------------------------------------------------------
void FmXUpdateMultiplexer::disposing(const ::com::sun::star::lang::EventObject& ) throw( ::com::sun::star::uno::RuntimeException )
{
}

//------------------------------------------------------------------
sal_Bool FmXUpdateMultiplexer::approveUpdate(const ::com::sun::star::lang::EventObject &e)
{
    ::com::sun::star::lang::EventObject aMulti( e );
    aMulti.Source = &m_rParent;

    sal_Bool bResult = sal_True;
    if (getLength())                                            \
    {                                                                   \
        ::cppu::OInterfaceIteratorHelper aIter(*this);              \
        while (bResult && aIter.hasMoreElements())                                  \
            bResult = reinterpret_cast< ::com::sun::star::form::XUpdateListener*>(aIter.next())->approveUpdate(aMulti);     \
    }

    return bResult;
}

//------------------------------------------------------------------
void FmXUpdateMultiplexer::updated(const ::com::sun::star::lang::EventObject &e)
{
    ::com::sun::star::lang::EventObject aMulti( e );
    aMulti.Source = &m_rParent;
    NOTIFY_LISTENERS((*this), ::com::sun::star::form::XUpdateListener, updated, aMulti);
}


//------------------------------------------------------------------
FmXContainerMultiplexer::FmXContainerMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex )
                        :OWeakSubObject( rSource )
                        ,OInterfaceContainerHelper(rMutex)
{
}

//------------------------------------------------------------------
::com::sun::star::uno::Any  SAL_CALL FmXContainerMultiplexer::queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aReturn;
    aReturn = ::cppu::queryInterface(_rType,
        static_cast< ::com::sun::star::container::XContainerListener*>(this),
        static_cast< ::com::sun::star::lang::XEventListener*>(this)
    );

    if (!aReturn.hasValue())
        aReturn = OWeakSubObject::queryInterface( _rType );

    return aReturn;
}

//------------------------------------------------------------------
void FmXContainerMultiplexer::disposing(const ::com::sun::star::lang::EventObject& ) throw( ::com::sun::star::uno::RuntimeException )
{
}
//------------------------------------------------------------------
void FmXContainerMultiplexer::elementInserted(const ::com::sun::star::container::ContainerEvent& e)
{
    ::com::sun::star::container::ContainerEvent aMulti( e );
    aMulti.Source = &m_rParent;
    NOTIFY_LISTENERS((*this), ::com::sun::star::container::XContainerListener, elementInserted, aMulti);
}

//------------------------------------------------------------------
void FmXContainerMultiplexer::elementRemoved(const ::com::sun::star::container::ContainerEvent& e)
{
    ::com::sun::star::container::ContainerEvent aMulti( e );
    aMulti.Source = &m_rParent;
    NOTIFY_LISTENERS((*this), ::com::sun::star::container::XContainerListener, elementRemoved, aMulti);
}


//------------------------------------------------------------------
void FmXContainerMultiplexer::elementReplaced(const ::com::sun::star::container::ContainerEvent& e)
{
    ::com::sun::star::container::ContainerEvent aMulti( e );
    aMulti.Source = &m_rParent;
    NOTIFY_LISTENERS((*this), ::com::sun::star::container::XContainerListener, elementReplaced, aMulti);
}

//==================================================================
//= FmXGridControl
//==================================================================

//------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL FmXGridControl_NewInstance_Impl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory)
{
    return *(new FmXGridControl(_rxFactory));
}

//------------------------------------------------------------------------------
FmXGridControl::FmXGridControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
               :m_aModifyListeners(*this, GetMutex())
               ,m_aUpdateListeners(*this, GetMutex())
               ,m_aContainerListeners(*this, GetMutex())
               ,m_nPeerCreationLevel(0)
               ,m_bInDraw(sal_False)
               ,m_xServiceFactory(_rxFactory)
{
}

//------------------------------------------------------------------------------
FmXGridControl::~FmXGridControl()
{
}

//------------------------------------------------------------------
::com::sun::star::uno::Any  SAL_CALL FmXGridControl::queryAggregation(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aReturn = ::cppu::queryInterface(_rType,
        static_cast< ::com::sun::star::form::XBoundComponent*>(this),
        static_cast< ::com::sun::star::form::XUpdateBroadcaster*>(this),
        static_cast< ::com::sun::star::form::XGrid*>(this),
        static_cast< ::com::sun::star::util::XModifyBroadcaster*>(this),
        static_cast< ::com::sun::star::form::XGridFieldDataSupplier*>(this),
        static_cast< ::com::sun::star::container::XIndexAccess*>(this),
        static_cast< ::com::sun::star::container::XElementAccess*>(static_cast< ::com::sun::star::container::XIndexAccess*>(this)),
        static_cast< ::com::sun::star::container::XEnumerationAccess*>(this),
        static_cast< ::com::sun::star::util::XModeSelector*>(this),
        static_cast< ::com::sun::star::container::XContainer*>(this),
        static_cast< ::com::sun::star::frame::XDispatchProvider*>(this),
        static_cast< ::com::sun::star::frame::XDispatchProviderInterception*>(this)
    );

    if (!aReturn.hasValue())
        aReturn = UnoControl::queryAggregation( _rType );
    return aReturn;
}

//------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> SAL_CALL FmXGridControl::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> aTypes = UnoControl::getTypes();

    sal_Int32 nLen = aTypes.getLength();
    aTypes.realloc(nLen + 10);
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::form::XBoundComponent>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::form::XGrid>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyBroadcaster>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::form::XGridFieldDataSupplier>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeSelector>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterception>*>(NULL));
    DBG_ASSERT(nLen == aTypes.getLength(), "FmXGridControl::getTypes : forgot to adjust realloc ?");

    return aTypes;
}

//------------------------------------------------------------------
::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL FmXGridControl::getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return form::OImplementationIds::getImplementationId(getTypes());
}

// ::com::sun::star::lang::XServiceInfo
//------------------------------------------------------------------------------
sal_Bool SAL_CALL FmXGridControl::supportsService(const ::rtl::OUString& ServiceName) throw()
{
    ::utl::StringSequence aSupported = getSupportedServiceNames();
    const ::rtl::OUString * pArray = aSupported.getConstArray();
    for( sal_Int32 i = 0; i < aSupported.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL FmXGridControl::getImplementationName() throw()
{
    return ::rtl::OUString::createFromAscii("com.sun.star.form.FmXGridControl");
}

//------------------------------------------------------------------------------
::utl::StringSequence SAL_CALL FmXGridControl::getSupportedServiceNames() throw()
{
    static ::rtl::OUString aServName = FM_SUN_CONTROL_GRIDCONTROL;
//  static ::rtl::OUString aServName(FM_SUN_CONTROL_GRIDCONTROL);
    return ::utl::StringSequence(&aServName, 1);
}

//------------------------------------------------------------------------------
void SAL_CALL FmXGridControl::dispose() throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::lang::EventObject aEvt;
    aEvt.Source = static_cast< ::cppu::OWeakObject* >(this);
    m_aModifyListeners.disposeAndClear(aEvt);
    m_aUpdateListeners.disposeAndClear(aEvt);
    m_aContainerListeners.disposeAndClear(aEvt);

    UnoControl::dispose();
}

//------------------------------------------------------------------------------
::rtl::OUString FmXGridControl::GetComponentServiceName()
{
    ::rtl::OUString aName = ::rtl::OUString::createFromAscii("DBGrid");
    return aName;
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL FmXGridControl::setModel(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& rModel)
{
    if (!UnoControl::setModel(rModel))
        return sal_False;

    ::com::sun::star::uno::Reference< ::com::sun::star::form::XGridPeer >  xGridPeer(mxPeer, ::com::sun::star::uno::UNO_QUERY);
    if (xGridPeer.is())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xCols(mxModel, ::com::sun::star::uno::UNO_QUERY);
        xGridPeer->setColumns(xCols);
    }
    return sal_True;
}

//------------------------------------------------------------------------------
FmXGridPeer* FmXGridControl::imp_CreatePeer(Window* pParent)
{
    FmXGridPeer* pReturn = new FmXGridPeer(m_xServiceFactory);

    // translate properties into WinBits
    WinBits nStyle = WB_TABSTOP;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xModelSet(getModel(), ::com::sun::star::uno::UNO_QUERY);
    if (xModelSet.is())
    {
        try
        {
            if (::utl::getINT16(xModelSet->getPropertyValue(FM_PROP_BORDER)))
                nStyle |= WB_BORDER;
        }
        catch(...)
        {
        }
    }

    pReturn->Create(pParent, nStyle);
    return pReturn;
}

//------------------------------------------------------------------------------
void SAL_CALL FmXGridControl::createPeer(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& rToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& rParentPeer) throw( ::com::sun::star::uno::RuntimeException )
{
    if (mxPeer.is())
        return;

    Window* pParentWin = NULL;
    if (rParentPeer.is())
    {
        VCLXWindow* pParent = VCLXWindow::GetImplementation(rParentPeer);
        if (pParent)
            pParentWin = pParent->GetWindow();
    }

    FmXGridPeer* pPeer = imp_CreatePeer(pParentWin);
    DBG_ASSERT(pPeer != NULL, "FmXGridControl::createPeer : imp_CreatePeer didn't return a peer !");
    mxPeer = pPeer;

    // lesen der properties aus dem model
    ++m_nPeerCreationLevel;
    updateFromModel();

    // folgendes unschoene Szenario : updateFromModel fuehrt zu einem propertiesChanged am Control,
    // das stellt fest, dass sich eine 'kritische' Property geaendert hat (zum Beispiel "Border") und
    // legt daraufhin eine neue Peer an, was wieder hier im createPeer landet, wir legen also eine
    // zweite FmXGridPeer an und initialisieren die. Dann kommen wir in der ersten Inkarnation aus
    // dem updsateFromModel raus und arbeiten dort weiter mit dem pPeer, das jetzt eigentlich schon
    // veraltet ist (da ja in der zweiten Inkarnation eine andere Peer angelegt wurde).
    // Deswegen also der Aufwand mit dem PeerCreationLevel, das stellt sicher, dass wir die in dem
    // tiefsten Level angelegte Peer wirklich verwenden, sie aber erst im top-level
    // initialisieren.
    if (--m_nPeerCreationLevel == 0)
    {
        DBG_ASSERT(mxPeer.is(), "FmXGridControl::createPeer : something went wrong ... no top level peer !");
        pPeer = FmXGridPeer::getImplementation(mxPeer);

        ::com::sun::star::awt::Rectangle rArea = getPosSize();
        if (!rArea.Width && !rArea.Height)
            pPeer->setPosSize(rArea.X, rArea.Y, rArea.Width, rArea.Height, ::com::sun::star::awt::PosSize::POSSIZE);

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xColumns(getModel(), ::com::sun::star::uno::UNO_QUERY);
        if (xColumns.is())
            pPeer->setColumns(xColumns);

        if (maComponentInfos.bVisible)
            pPeer->setVisible(sal_True);

        if (!maComponentInfos.bEnable)
            pPeer->setEnable(sal_False);

        if (maWindowListeners.getLength())
            pPeer->addWindowListener( &maWindowListeners );

        if (maFocusListeners.getLength())
            pPeer->addFocusListener( &maFocusListeners );

        if (maKeyListeners.getLength())
            pPeer->addKeyListener( &maKeyListeners );

        if (maMouseListeners.getLength())
            pPeer->addMouseListener( &maMouseListeners );

        if (maMouseMotionListeners.getLength())
            pPeer->addMouseMotionListener( &maMouseMotionListeners );

        if (maPaintListeners.getLength())
            pPeer->addPaintListener( &maPaintListeners );

        if (m_aModifyListeners.getLength())
            pPeer->addModifyListener( &m_aModifyListeners );

        if (m_aUpdateListeners.getLength())
            pPeer->addUpdateListener( &m_aUpdateListeners );

        if (m_aContainerListeners.getLength())
            pPeer->addContainerListener( &m_aContainerListeners );

        // forward the design mode
        sal_Bool bForceAlivePeer = m_bInDraw && !maComponentInfos.bVisible;
            // (we force a alive-mode peer if we're in "draw", cause in this case the peer will be used for drawing in
            // foreign devices. We secure this with the visibility check as an living peer is assumed to be noncritical
            // only if invisible)
        ::com::sun::star::uno::Any aOldCursorBookmark;
        if (!mbDesignMode || bForceAlivePeer)
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xComp(getModel(), ::com::sun::star::uno::UNO_QUERY);
            if (xComp.is())
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >  xForm(xComp->getParent(), ::com::sun::star::uno::UNO_QUERY);
                // is the form alive?
                // we can see that if the form contains columns
                ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier >  xColumnsSupplier(xForm, ::com::sun::star::uno::UNO_QUERY);
                if (xColumnsSupplier.is())
                {
                    if (::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > (xColumnsSupplier->getColumns(),::com::sun::star::uno::UNO_QUERY)->getCount())
                    {
                        // we get only a new bookmark if the resultset is not forwardonly
                        if (::utl::getINT32(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > (xForm, ::com::sun::star::uno::UNO_QUERY)->getPropertyValue(FM_PROP_RESULTSET_TYPE)) != ::com::sun::star::sdbc::ResultSetType::FORWARD_ONLY)
                        {
                            // as the FmGridControl touches the data source it is connected to we have to remember the current
                            // cursor position (and restore afterwards)
                            aOldCursorBookmark = ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XRowLocate > (xForm, ::com::sun::star::uno::UNO_QUERY)->getBookmark();
                        }
                    }
                }
                pPeer->setRowSet(xForm);
            }
        }
        pPeer->setDesignMode(mbDesignMode && !bForceAlivePeer);

        if (aOldCursorBookmark.hasValue())
        {   // we have a valid bookmark, so we have to restore the cursor's position
            ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xComp(getModel(), ::com::sun::star::uno::UNO_QUERY);
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XRowLocate >  xLocate(xComp->getParent(), ::com::sun::star::uno::UNO_QUERY);
            xLocate->moveToBookmark(aOldCursorBookmark);
        }

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XView >  xPeerView(mxPeer, ::com::sun::star::uno::UNO_QUERY);
        xPeerView->setZoom( maComponentInfos.nZoomX, maComponentInfos.nZoomY );
        xPeerView->setGraphics( mxGraphics );
    }
}

//------------------------------------------------------------------------------
void FmXGridControl::addModifyListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& l)
{
    m_aModifyListeners.addInterface( l );
    if( mxPeer.is() && m_aModifyListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyBroadcaster >  xGrid(mxPeer, ::com::sun::star::uno::UNO_QUERY);
        xGrid->addModifyListener( &m_aModifyListeners);
    }
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Sequence< sal_Bool > SAL_CALL FmXGridControl::queryFieldDataType( const ::com::sun::star::uno::Type& xType ) throw(::com::sun::star::uno::RuntimeException)
{
    if (mxPeer.is())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XGridFieldDataSupplier >  xPeerSupplier(mxPeer, ::com::sun::star::uno::UNO_QUERY);
        if (xPeerSupplier.is())
            return xPeerSupplier->queryFieldDataType(xType);
    }

    return ::com::sun::star::uno::Sequence<sal_Bool>();
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL FmXGridControl::queryFieldData( sal_Int32 nRow, const ::com::sun::star::uno::Type& xType ) throw(::com::sun::star::uno::RuntimeException)
{
    if (mxPeer.is())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XGridFieldDataSupplier >  xPeerSupplier(mxPeer, ::com::sun::star::uno::UNO_QUERY);
        if (xPeerSupplier.is())
            return xPeerSupplier->queryFieldData(nRow, xType);
    }

    return ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>();
}

//------------------------------------------------------------------------------
void SAL_CALL FmXGridControl::removeModifyListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& l)
{
    if( mxPeer.is() && m_aModifyListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyBroadcaster >  xGrid(mxPeer, ::com::sun::star::uno::UNO_QUERY);
        xGrid->removeModifyListener( &m_aModifyListeners);
    }
    m_aModifyListeners.removeInterface( l );
}

//------------------------------------------------------------------------------
void SAL_CALL FmXGridControl::draw( long x, long y )
{
    m_bInDraw = sal_True;
    UnoControl::draw(x, y);
    m_bInDraw = sal_False;
}

//------------------------------------------------------------------------------
void SAL_CALL FmXGridControl::setDesignMode(sal_Bool bOn)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XRowSetSupplier >  xGrid(mxPeer, ::com::sun::star::uno::UNO_QUERY);

    if (xGrid.is() && (bOn != mbDesignMode || (!bOn && !xGrid->getRowSet().is())))
    {
        if (bOn)
        {
            xGrid->setRowSet(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > ());
        }
        else
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xComp(getModel(), ::com::sun::star::uno::UNO_QUERY);
            if (xComp.is())
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >  xForm(xComp->getParent(), ::com::sun::star::uno::UNO_QUERY);
                xGrid->setRowSet(xForm);
            }
        }

        mbDesignMode = bOn;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclWindowPeer >  xVclWindowPeer(mxPeer, ::com::sun::star::uno::UNO_QUERY);
        if (xVclWindowPeer.is())
            xVclWindowPeer->setDesignMode(bOn);
    }
    mbDesignMode = bOn;
}

// ::com::sun::star::form::XBoundComponent
//------------------------------------------------------------------------------
void SAL_CALL FmXGridControl::addUpdateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XUpdateListener >& l)
{
    m_aUpdateListeners.addInterface( l );
    if( mxPeer.is() && m_aUpdateListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XBoundComponent >  xBound(mxPeer, ::com::sun::star::uno::UNO_QUERY);
        xBound->addUpdateListener( &m_aUpdateListeners);
    }
}

//------------------------------------------------------------------------------
void SAL_CALL FmXGridControl::removeUpdateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XUpdateListener >& l)
{
    if( mxPeer.is() && m_aUpdateListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XBoundComponent >  xBound(mxPeer, ::com::sun::star::uno::UNO_QUERY);
        xBound->removeUpdateListener( &m_aUpdateListeners);
    }
    m_aUpdateListeners.removeInterface( l );
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL FmXGridControl::commit()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XBoundComponent >  xBound(mxPeer, ::com::sun::star::uno::UNO_QUERY);
    if (xBound.is())
        return xBound->commit();
    else
        return sal_True;
}

// ::com::sun::star::container::XContainer
//------------------------------------------------------------------------------
void SAL_CALL FmXGridControl::addContainerListener(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& l) throw( ::com::sun::star::uno::RuntimeException )
{
    m_aContainerListeners.addInterface( l );
    if( mxPeer.is() && m_aContainerListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >  xContainer(mxPeer, ::com::sun::star::uno::UNO_QUERY);
        xContainer->addContainerListener( &m_aContainerListeners);
    }
}

//------------------------------------------------------------------------------
void SAL_CALL FmXGridControl::removeContainerListener(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& l) throw( ::com::sun::star::uno::RuntimeException )
{
    if( mxPeer.is() && m_aContainerListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >  xContainer(mxPeer, ::com::sun::star::uno::UNO_QUERY);
        xContainer->removeContainerListener( &m_aContainerListeners);
    }
    m_aContainerListeners.removeInterface( l );
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  SAL_CALL FmXGridControl::queryDispatch(const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >  xPeerProvider(mxPeer, ::com::sun::star::uno::UNO_QUERY);
    if (xPeerProvider.is())
        return xPeerProvider->queryDispatch(aURL, aTargetFrameName, nSearchFlags);
    else
        return ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > ();
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL FmXGridControl::queryDispatches(const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor>& aDescripts) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >  xPeerProvider(mxPeer, ::com::sun::star::uno::UNO_QUERY);
    if (xPeerProvider.is())
        return xPeerProvider->queryDispatches(aDescripts);
    else
        return ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > >();
}

//------------------------------------------------------------------------------
void SAL_CALL FmXGridControl::registerDispatchProviderInterceptor(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& _xInterceptor) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterception >  xPeerInterception(mxPeer, ::com::sun::star::uno::UNO_QUERY);
    if (xPeerInterception.is())
        xPeerInterception->registerDispatchProviderInterceptor(_xInterceptor);
}

//------------------------------------------------------------------------------
void SAL_CALL FmXGridControl::releaseDispatchProviderInterceptor(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& _xInterceptor) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterception >  xPeerInterception(mxPeer, ::com::sun::star::uno::UNO_QUERY);
    if (xPeerInterception.is())
        xPeerInterception->releaseDispatchProviderInterceptor(_xInterceptor);
}

//------------------------------------------------------------------------------
sal_Int16 SAL_CALL FmXGridControl::getCurrentColumnPosition()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XGrid >  xGrid(mxPeer, ::com::sun::star::uno::UNO_QUERY);
    return xGrid.is() ? xGrid->getCurrentColumnPosition() : -1;
}

//------------------------------------------------------------------------------
void SAL_CALL FmXGridControl::setCurrentColumnPosition(sal_Int16 nPos)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XGrid >  xGrid(mxPeer, ::com::sun::star::uno::UNO_QUERY);
    if (xGrid.is())
        xGrid->setCurrentColumnPosition(nPos);
}

// ::com::sun::star::container::XElementAccess
//------------------------------------------------------------------------------
sal_Bool SAL_CALL FmXGridControl::hasElements() throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XElementAccess >  xPeer(mxPeer, ::com::sun::star::uno::UNO_QUERY);
    return xPeer.is() ? xPeer->hasElements() : 0;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Type SAL_CALL FmXGridControl::getElementType(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent >*)NULL);
}

// ::com::sun::star::container::XEnumerationAccess
//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL FmXGridControl::createEnumeration() throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess >  xPeer(mxPeer, ::com::sun::star::uno::UNO_QUERY);
    if (xPeer.is())
        return xPeer->createEnumeration();
    else
        return new ::utl::OEnumerationByIndex(this);
}

// ::com::sun::star::container::XIndexAccess
//------------------------------------------------------------------------------
sal_Int32 SAL_CALL FmXGridControl::getCount() throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xPeer(mxPeer, ::com::sun::star::uno::UNO_QUERY);
    return xPeer.is() ? xPeer->getCount() : 0;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Any SAL_CALL FmXGridControl::getByIndex(sal_Int32 _nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xPeer(mxPeer, ::com::sun::star::uno::UNO_QUERY);
    if (!xPeer.is())
        throw ::com::sun::star::lang::IndexOutOfBoundsException();

    return xPeer->getByIndex(_nIndex);
}

// ::com::sun::star::util::XModeSelector
//------------------------------------------------------------------------------
void SAL_CALL FmXGridControl::setMode(const ::rtl::OUString& Mode) throw( ::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeSelector >  xPeer(mxPeer, ::com::sun::star::uno::UNO_QUERY);
    if (!xPeer.is())
        throw ::com::sun::star::lang::NoSupportException();

    xPeer->setMode(Mode);
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL FmXGridControl::getMode() throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeSelector >  xPeer(mxPeer, ::com::sun::star::uno::UNO_QUERY);
    return xPeer.is() ? xPeer->getMode() : ::rtl::OUString();
}

//------------------------------------------------------------------------------
::utl::StringSequence SAL_CALL FmXGridControl::getSupportedModes() throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeSelector >  xPeer(mxPeer, ::com::sun::star::uno::UNO_QUERY);
    return xPeer.is() ? xPeer->getSupportedModes() : ::utl::StringSequence();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL FmXGridControl::supportsMode(const ::rtl::OUString& Mode) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeSelector >  xPeer(mxPeer, ::com::sun::star::uno::UNO_QUERY);
    return xPeer.is() ? xPeer->supportsMode(Mode) : sal_False;
}

/*************************************************************************/
//------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  FmXGridPeer_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory)
{
    FmXGridPeer* pNewObject = new FmXGridPeer(_rxFactory);
    pNewObject->Create(NULL, WB_TABSTOP);
    return *pNewObject;
}

//------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> SAL_CALL FmXGridPeer::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> aTypes = VCLXWindow::getTypes();

    sal_Int32 nLen = aTypes.getLength();
    aTypes.realloc(nLen + 19);
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::form::XBoundComponent>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::form::XGridPeer>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::form::XGrid>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XRowSetSupplier>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSetListener>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyBroadcaster>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadListener>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::form::XGridFieldDataSupplier>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeSelector>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterception>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast< ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener>*>(NULL));
    DBG_ASSERT(nLen == aTypes.getLength(), "FmXGridPeer::getTypes : forgot to adjust realloc ?");

    return aTypes;
}

//------------------------------------------------------------------
::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL FmXGridPeer::getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return form::OImplementationIds::getImplementationId(getTypes());
}

//------------------------------------------------------------------
::com::sun::star::uno::Any  SAL_CALL FmXGridPeer::queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aReturn = ::cppu::queryInterface(_rType,
        static_cast< ::com::sun::star::form::XGrid*>(this),
        static_cast< ::com::sun::star::util::XModifyBroadcaster*>(this),
        static_cast< ::com::sun::star::container::XContainerListener*>(this),
        static_cast< ::com::sun::star::form::XLoadListener*>(this),
        static_cast< ::com::sun::star::form::XBoundComponent*>(this),
        static_cast< ::com::sun::star::form::XUpdateBroadcaster*>(this),
        static_cast< ::com::sun::star::beans::XPropertyChangeListener*>(this),
        static_cast< ::com::sun::star::lang::XEventListener*>(static_cast< ::com::sun::star::beans::XPropertyChangeListener*>(this)),
        static_cast< ::com::sun::star::sdb::XRowSetSupplier*>(this),
        static_cast< ::com::sun::star::sdbc::XRowSetListener*>(this),
        static_cast< ::com::sun::star::view::XSelectionChangeListener*>(this),
        static_cast< ::com::sun::star::form::XGridFieldDataSupplier*>(this)
    );

    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
        static_cast< ::com::sun::star::container::XElementAccess*>(static_cast< ::com::sun::star::container::XIndexAccess*>(this)),
        static_cast< ::com::sun::star::container::XIndexAccess*>(this),
        static_cast< ::com::sun::star::container::XEnumerationAccess*>(this),
        static_cast< ::com::sun::star::util::XModeSelector*>(this),
        static_cast< ::com::sun::star::frame::XDispatchProvider*>(this),
        static_cast< ::com::sun::star::frame::XDispatchProviderInterception*>(this),
        static_cast< ::com::sun::star::form::XResetListener*>(this)
    );

    if (!aReturn.hasValue())
        aReturn = VCLXWindow::queryInterface( _rType );

    return aReturn;
}

//------------------------------------------------------------------------------
rtl::OUString _fModeName = DATA_MODE;
FmXGridPeer::FmXGridPeer(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
            :m_aModifyListeners(m_aMutex)
            ,m_aUpdateListeners(m_aMutex)
            ,m_aContainerListeners(m_aMutex)
            ,m_nCursorListening(0)
//          ,m_aMode(rtl::OUString(DATA_MODE))
            ,m_aMode( _fModeName )
            ,m_pStateCache(NULL)
            ,m_pDispatchers(NULL)
            ,m_bInterceptingDispatch(sal_False)
            ,m_xServiceFactory(_rxFactory)
{
    // nach diesem Constructor muss Create gerufen werden !
}

//------------------------------------------------------------------------------
FmGridControl* FmXGridPeer::imp_CreateControl(Window* pParent, WinBits nStyle)
{
    return new FmGridControl(m_xServiceFactory, pParent, this, nStyle);
}

//------------------------------------------------------------------------------
void FmXGridPeer::Create(Window* pParent, WinBits nStyle)
{
    FmGridControl* pWin = imp_CreateControl(pParent, nStyle);
    DBG_ASSERT(pWin != NULL, "FmXGridPeer::Create : imp_CreateControl didn't return a control !");

    pWin->SetStateProvider(LINK(this, FmXGridPeer, OnQueryGridSlotState));
    pWin->SetSlotExecutor(LINK(this, FmXGridPeer, OnExecuteGridSlot));

    // Init muß immer aufgerufen werden
    pWin->Init();
    pWin->SetComponentInterface(this);

    getSupportedURLs();
}

//------------------------------------------------------------------------------
FmXGridPeer::~FmXGridPeer()
{
    setRowSet(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > ());
    setColumns(::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > ());
}

//------------------------------------------------------------------------------
const ::com::sun::star::uno::Sequence< sal_Int8 >&  FmXGridPeer::getUnoTunnelImplementationId() throw()
{
    static ::com::sun::star::uno::Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static ::com::sun::star::uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

//------------------------------------------------------------------------------
FmXGridPeer* FmXGridPeer::getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxIFace ) throw()
{
    FmXGridPeer* pReturn = NULL;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel >  xTunnel(_rxIFace, ::com::sun::star::uno::UNO_QUERY);
    if (xTunnel.is())
        pReturn = reinterpret_cast<FmXGridPeer*>(xTunnel->getSomething(getUnoTunnelImplementationId()));

    return pReturn;
}

//------------------------------------------------------------------------------
sal_Int64 SAL_CALL FmXGridPeer::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& _rIdentifier ) throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int64 nReturn(0);

    if  (   (_rIdentifier.getLength() == 16)
        &&  (0 == rtl_compareMemory( getUnoTunnelImplementationId().getConstArray(), _rIdentifier.getConstArray(), 16 ))
        )
    {
        nReturn = reinterpret_cast<sal_Int64>(this);
    }
    else
        nReturn = VCLXWindow::getSomething(_rIdentifier);

    return nReturn;
}

// ::com::sun::star::lang::XEventListener
//------------------------------------------------------------------------------
void FmXGridPeer::disposing(const ::com::sun::star::lang::EventObject& e) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::lang::EventObject aEvt(static_cast< ::cppu::OWeakObject* >(this));
    m_aUpdateListeners.disposeAndClear(aEvt);
    m_aModifyListeners.disposeAndClear(aEvt);
    m_aContainerListeners.disposeAndClear(aEvt);

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xCols(e.Source, ::com::sun::star::uno::UNO_QUERY);
    if (xCols.is())
        setColumns(::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > ());

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >  xCursor(e.Source, ::com::sun::star::uno::UNO_QUERY);
    if (xCursor.is())
    {
        m_xCursor = NULL;
        setRowSet(m_xCursor);
    }

    if (m_pDispatchers)
    {
        const ::com::sun::star::uno::Sequence< ::com::sun::star::util::URL>& aSupportedURLs = getSupportedURLs();
        const ::com::sun::star::util::URL* pSupportedURLs = aSupportedURLs.getConstArray();
        sal_Bool bDisconnect = sal_False;
        for (sal_uInt16 i=0; i<aSupportedURLs.getLength() && !bDisconnect; ++i, ++pSupportedURLs)
        {
            if (m_pDispatchers[i] == e.Source)
            {
                m_pDispatchers[i]->removeStatusListener((::com::sun::star::frame::XStatusListener*)this, *pSupportedURLs);
                m_pDispatchers[i] = NULL;
                m_pStateCache[i] = 0;
            }
        }
    }
    // VCLXComponent::disposing(e);
}

//------------------------------------------------------------------------------
void FmXGridPeer::addModifyListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& l)
{
    m_aModifyListeners.addInterface( l );
}

//------------------------------------------------------------------------------
void FmXGridPeer::removeModifyListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& l)
{
    m_aModifyListeners.removeInterface( l );
}

//------------------------------------------------------------------------------
#define LAST_KNOWN_TYPE     ::com::sun::star::form::FormComponentType::PATTERNFIELD
::com::sun::star::uno::Sequence< sal_Bool > SAL_CALL FmXGridPeer::queryFieldDataType( const ::com::sun::star::uno::Type& xType ) throw(::com::sun::star::uno::RuntimeException)
{
    // eine 'Konvertierungstabelle'
    static sal_Bool bCanConvert[LAST_KNOWN_TYPE][4] =
    {
        { sal_False, sal_False, sal_False, sal_False }, //  ::com::sun::star::form::FormComponentType::CONTROL
        { sal_False, sal_False, sal_False, sal_False }, //  ::com::sun::star::form::FormComponentType::COMMANDBUTTON
        { sal_False, sal_False, sal_False, sal_False }, //  ::com::sun::star::form::FormComponentType::RADIOBUTTON
        { sal_False, sal_False, sal_False, sal_False }, //  ::com::sun::star::form::FormComponentType::IMAGEBUTTON
        { sal_False, sal_False, sal_False, sal_True  }, //  ::com::sun::star::form::FormComponentType::CHECKBOX
        { sal_False, sal_False, sal_False, sal_False }, //  ::com::sun::star::form::FormComponentType::LISTBOX
        { sal_False, sal_False, sal_False, sal_False }, //  ::com::sun::star::form::FormComponentType::COMBOBOX
        { sal_False, sal_False, sal_False, sal_False }, //  ::com::sun::star::form::FormComponentType::GROUPBOX
        { sal_True , sal_False, sal_False, sal_False }, //  ::com::sun::star::form::FormComponentType::TEXTFIELD
        { sal_False, sal_False, sal_False, sal_False }, //  ::com::sun::star::form::FormComponentType::FIXEDTEXT
        { sal_False, sal_False, sal_False, sal_False }, //  ::com::sun::star::form::FormComponentType::GRIDCONTROL
        { sal_False, sal_False, sal_False, sal_False }, //  ::com::sun::star::form::FormComponentType::FILECONTROL
        { sal_False, sal_False, sal_False, sal_False }, //  ::com::sun::star::form::FormComponentType::HIDDENCONTROL
        { sal_False, sal_False, sal_False, sal_False }, //  ::com::sun::star::form::FormComponentType::IMAGECONTROL
        { sal_True , sal_True , sal_True , sal_False }, //  ::com::sun::star::form::FormComponentType::DATEFIELD
        { sal_True , sal_True , sal_False, sal_False }, //  ::com::sun::star::form::FormComponentType::TIMEFIELD
        { sal_True , sal_True , sal_False, sal_False }, //  ::com::sun::star::form::FormComponentType::NUMERICFIELD
        { sal_True , sal_True , sal_False, sal_False }, //  ::com::sun::star::form::FormComponentType::CURRENCYFIELD
        { sal_True , sal_False, sal_False, sal_False }  //  ::com::sun::star::form::FormComponentType::PATTERNFIELD
    };


    sal_Int16 nMapColumn = -1;
    switch (xType.getTypeClass())
    {
        case ::com::sun::star::uno::TypeClass_STRING            : nMapColumn = 0; break;
        case ::com::sun::star::uno::TypeClass_FLOAT:
        case ::com::sun::star::uno::TypeClass_DOUBLE            : nMapColumn = 1; break;
        case ::com::sun::star::uno::TypeClass_SHORT:
        case ::com::sun::star::uno::TypeClass_LONG:
        case ::com::sun::star::uno::TypeClass_UNSIGNED_LONG:
        case ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT    : nMapColumn = 2; break;
        case ::com::sun::star::uno::TypeClass_BOOLEAN           : nMapColumn = 3; break;
    }

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xColumns = getColumns();

    FmGridControl* pGrid = (FmGridControl*) GetWindow();
    sal_Int32 nColumns = pGrid->GetViewColCount();

    DbGridColumns aColumns = pGrid->GetColumns();

    ::com::sun::star::uno::Sequence<sal_Bool> aReturnSequence(nColumns);
    sal_Bool* pReturnArray = aReturnSequence.getArray();

    sal_Bool bRequestedAsAny = (xType.getTypeClass() == ::com::sun::star::uno::TypeClass_ANY);

    DbGridColumn* pCol;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >  xFieldContent;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xCurrentColumn;
    for (sal_Int32 i=0; i<nColumns; ++i)
    {
        if (bRequestedAsAny)
        {
            pReturnArray[i] = sal_True;
            continue;
        }

        pReturnArray[i] = sal_False;

        sal_uInt16 nModelPos = pGrid->GetModelColumnPos(pGrid->GetColumnIdFromViewPos(i));
        DBG_ASSERT(nModelPos != (sal_uInt16)-1, "FmXGridPeer::queryFieldDataType : no model pos !");

        pCol = aColumns.GetObject(nModelPos);
        const DbGridRowRef xRow = pGrid->GetSeekRow();
        xFieldContent = (xRow.Is() && xRow->HasField(pCol->GetFieldPos())) ? (const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >&)xRow->GetField(pCol->GetFieldPos()) : ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn > ();
        if (!xFieldContent.is())
            // can't supply anything without a field content
            // FS - 07.12.99 - 54391
            continue;

        xCurrentColumn;
        ::cppu::extractInterface(xCurrentColumn, xColumns->getByIndex(nModelPos));
        if (!::utl::hasProperty(FM_PROP_CLASSID, xCurrentColumn))
            continue;

        sal_Int16 nClassId;
        xCurrentColumn->getPropertyValue(FM_PROP_CLASSID) >>= nClassId;
        if (nClassId>LAST_KNOWN_TYPE)
            continue;
        DBG_ASSERT(nClassId>0, "FmXGridPeer::queryFieldDataType : somebody changed the definition of the ::com::sun::star::form::FormComponentType enum !");

        if (nMapColumn != -1)
            pReturnArray[i] = bCanConvert[nClassId-1][nMapColumn];
    }

    return aReturnSequence;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL FmXGridPeer::queryFieldData( sal_Int32 nRow, const ::com::sun::star::uno::Type& xType ) throw(::com::sun::star::uno::RuntimeException)
{
    FmGridControl* pGrid = (FmGridControl*) GetWindow();
    DBG_ASSERT(pGrid && pGrid->IsOpen(), "FmXGridPeer::queryFieldData : have no valid grid window !");
    if (!pGrid || !pGrid->IsOpen())
        return ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>();

    // das Control zur angegebenen Row fahren
    if (!pGrid->SeekRow(nRow))
    {
        throw ::com::sun::star::lang::IllegalArgumentException();
    }

    // don't use GetCurrentRow as this isn't affected by the above SeekRow
    // FS - 30.09.99 - 68644
    DbGridRowRef aRow = pGrid->GetSeekRow();
    DBG_ASSERT(aRow.Is(), "FmXGridPeer::queryFieldData : invalid current Row !");

    // die Columns des Controls brauche ich fuer GetFieldText
    DbGridColumns aColumns = pGrid->GetColumns();

    // und durch alle Spalten durch
    sal_Int32 nColumnCount = pGrid->GetViewColCount();

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any> aReturnSequence(nColumnCount);
    ::com::sun::star::uno::Any* pReturnArray = aReturnSequence.getArray();

    sal_Bool bRequestedAsAny = (xType.getTypeClass() == ::com::sun::star::uno::TypeClass_ANY);
    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >  xFieldContent;
    DbGridColumn* pCol;
    for (sal_Int32 i=0; i < nColumnCount; ++i)
    {
        sal_uInt16 nModelPos = pGrid->GetModelColumnPos(pGrid->GetColumnIdFromViewPos(i));
        DBG_ASSERT(nModelPos != (sal_uInt16)-1, "FmXGridPeer::queryFieldData : invalid model pos !");

        // don't use GetCurrentFieldValue to determine the field content as this isn't affected by the above SeekRow
        // FS - 30.09.99 - 68644
        pCol = aColumns.GetObject(nModelPos);
        const DbGridRowRef xRow = pGrid->GetSeekRow();
        xFieldContent = (xRow.Is() && xRow->HasField(pCol->GetFieldPos())) ? (const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >&)xRow->GetField(pCol->GetFieldPos()) : ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn > ();

        if (xFieldContent.is())
        {
            if (bRequestedAsAny)
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xFieldSet(xFieldContent, ::com::sun::star::uno::UNO_QUERY);
                pReturnArray[i] = xFieldSet->getPropertyValue(FM_PROP_VALUE);
            }
            else
            {
                switch (xType.getTypeClass())
                {
                    // Strings werden direkt ueber das GetFieldText abgehandelt
                    case ::com::sun::star::uno::TypeClass_STRING            :
                    {
                        String sText = aColumns.GetObject(nModelPos)->GetCellText(aRow, pGrid->getNumberFormatter());
                        pReturnArray[i] <<= ::rtl::OUString(sText);
                    }
                    break;
                    // alles andere wird an der DatabaseVariant erfragt
                    case ::com::sun::star::uno::TypeClass_FLOAT         : pReturnArray[i] <<= xFieldContent->getFloat(); break;
                    case ::com::sun::star::uno::TypeClass_DOUBLE        : pReturnArray[i] <<= xFieldContent->getDouble(); break;
                    case ::com::sun::star::uno::TypeClass_SHORT         : pReturnArray[i] <<= (sal_Int16)xFieldContent->getShort(); break;
                    case ::com::sun::star::uno::TypeClass_LONG          : pReturnArray[i] <<= (sal_Int32)xFieldContent->getLong(); break;
                    case ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT: pReturnArray[i] <<= (sal_uInt16)xFieldContent->getShort(); break;
                    case ::com::sun::star::uno::TypeClass_UNSIGNED_LONG : pReturnArray[i] <<= (sal_uInt32)xFieldContent->getLong(); break;
                    case ::com::sun::star::uno::TypeClass_BOOLEAN       : ::utl::setBOOL(pReturnArray[i],xFieldContent->getBoolean()); break;
                    default:
                    {
                        throw ::com::sun::star::lang::IllegalArgumentException();
                    }
                }
            }
        }
    }
    return aReturnSequence;
}

//------------------------------------------------------------------------------
void FmXGridPeer::CellModified()
{
    ::com::sun::star::lang::EventObject aEvt;
    aEvt.Source = static_cast< ::cppu::OWeakObject* >(this);
    NOTIFY_LISTENERS(m_aModifyListeners, ::com::sun::star::util::XModifyListener, modified, aEvt);
}

// ::com::sun::star::beans::XPropertyChangeListener
//------------------------------------------------------------------------------
void FmXGridPeer::propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt)
{
    FmGridControl* pGrid = (FmGridControl*) GetWindow();
    if (!pGrid)
        return;

    // DatenbankEvent
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >  xCursor(evt.Source, ::com::sun::star::uno::UNO_QUERY);
    if (evt.PropertyName == FM_PROP_VALUE || m_xCursor == evt.Source)
        pGrid->propertyChange(evt);
    else if (pGrid && m_xColumns.is() && m_xColumns->hasElements())
    {
        // zunaechst raussuchen welche Column sich geaendert hat
        ::utl::InterfaceRef xCurrent;
        for (sal_Int32 i = 0; i < m_xColumns->getCount(); i++)
        {
            ::cppu::extractInterface(xCurrent, m_xColumns->getByIndex(i));
            if (evt.Source == xCurrent)
                break;
        }

        if (i >= m_xColumns->getCount())
            // this is valid because we are listening at the cursor, too (RecordCount, -status, edit mode)
            return;

        sal_uInt16 nId = pGrid->GetColumnIdFromModelPos(i);
        sal_Bool bInvalidateColumn = sal_False;

        if (evt.PropertyName == FM_PROP_LABEL)
        {
            String aName = ::utl::getString(evt.NewValue);
            if (aName != pGrid->GetColumnTitle(nId))
                pGrid->SetColumnTitle(nId, aName);
        }
        else if (evt.PropertyName == FM_PROP_WIDTH)
        {
            sal_Int32 nWidth = 0;
            if (evt.NewValue.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID)
                nWidth = pGrid->GetDefaultColumnWidth(pGrid->GetColumnTitle(nId));
                // GetDefaultColumnWidth already considerd the zoom factor
            else
            {
                sal_Int32 nTest;
                if (evt.NewValue >>= nTest)
                {
                    nWidth = pGrid->LogicToPixel(Point(nTest,0),MAP_10TH_MM).X();
                    // take the zoom factor into account
                    nWidth = pGrid->CalcZoom(nWidth);
                }
            }
            if (nWidth != (sal_Int32(pGrid->GetColumnWidth(nId))))
            {
                if (pGrid->IsEditing())
                {
                    pGrid->DeactivateCell();
                    pGrid->ActivateCell();
                }
                pGrid->SetColumnWidth(nId, nWidth);
            }
        }
        else if (evt.PropertyName == FM_PROP_HIDDEN)
        {
            DBG_ASSERT(evt.NewValue.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_BOOLEAN,
                "FmXGridPeer::propertyChange : the property 'hidden' should be of type boolean !");
            if (::utl::getBOOL(evt.NewValue))
                pGrid->HideColumn(nId);
            else
                pGrid->ShowColumn(nId);
        }
        else if (evt.PropertyName == FM_PROP_ALIGN)
        {
            // it design mode it doesn't matter
            if (!isDesignMode())
            {
                FmGridControl* pGrid = (FmGridControl*) GetWindow();
                DbGridColumn* pCol = pGrid->GetColumns().GetObject(i);

                pCol->SetAlignmentFromModel(-1);
                bInvalidateColumn = sal_True;
            }
        }
        else if (evt.PropertyName == FM_PROP_FORMATKEY)
        {
            if (!isDesignMode())
                bInvalidateColumn = sal_True;
        }

        // need to invalidate the affected column ?
        if (bInvalidateColumn)
        {
            FmGridControl* pGrid = (FmGridControl*) GetWindow();

            sal_Bool bWasEditing = pGrid->IsEditing();
            if (bWasEditing)
                pGrid->DeactivateCell();

            Rectangle aColRect = pGrid->GetFieldRect(nId);
            aColRect.Top() = 0;
            aColRect.Bottom() = pGrid->GetSizePixel().Height();
            pGrid->Invalidate(aColRect);

            if (bWasEditing)
                pGrid->ActivateCell();
        }
    }
}

// ::com::sun::star::form::XBoundComponent
//------------------------------------------------------------------------------
void FmXGridPeer::addUpdateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XUpdateListener >& l)
{
    m_aUpdateListeners.addInterface(l);
}

//------------------------------------------------------------------------------
void FmXGridPeer::removeUpdateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XUpdateListener >& l)
{
    m_aUpdateListeners.removeInterface(l);
}

//------------------------------------------------------------------------------
sal_Bool FmXGridPeer::commit()
{
    FmGridControl* pGrid = (FmGridControl*) GetWindow();
    if (!m_xCursor.is() || !pGrid)
        return sal_True;

    ::com::sun::star::lang::EventObject aEvt(static_cast< ::cppu::OWeakObject* >(this));
    ::cppu::OInterfaceIteratorHelper aIter(m_aUpdateListeners);
    sal_Bool bCancel = sal_False;
    while (aIter.hasMoreElements() && !bCancel)
        if (!reinterpret_cast< ::com::sun::star::form::XUpdateListener*>(aIter.next())->approveUpdate(aEvt))
            bCancel = sal_True;

    if (!bCancel)
        bCancel = !pGrid->commit();

    if (!bCancel)
    {
        NOTIFY_LISTENERS(m_aUpdateListeners, ::com::sun::star::form::XUpdateListener, updated, aEvt);
    }
    return !bCancel;
}


//------------------------------------------------------------------------------
void FmXGridPeer::cursorMoved(const ::com::sun::star::lang::EventObject& _rEvent) throw( ::com::sun::star::uno::RuntimeException )
{
    FmGridControl* pGrid = (FmGridControl*) GetWindow();
    // we are not interested in move to insert row only in the resetted event
    // which is fired after positioning an the insert row
    if (pGrid && pGrid->IsOpen() && !::utl::getBOOL(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > (_rEvent.Source, ::com::sun::star::uno::UNO_QUERY)->getPropertyValue(FM_PROP_ISNEW)))
        pGrid->positioned(_rEvent);
}

//------------------------------------------------------------------------------
void FmXGridPeer::rowChanged(const ::com::sun::star::lang::EventObject& _rEvent) throw( ::com::sun::star::uno::RuntimeException )
{
    FmGridControl* pGrid = (FmGridControl*) GetWindow();
    if (pGrid && pGrid->IsOpen())
    {
        if (m_xCursor->rowUpdated() && !pGrid->IsCurrentAppending())
            pGrid->RowModified(pGrid->GetCurrentPos());
        else if (m_xCursor->rowInserted())
            pGrid->inserted(_rEvent);
    }
}

//------------------------------------------------------------------------------
void FmXGridPeer::rowSetChanged(const ::com::sun::star::lang::EventObject& event) throw( ::com::sun::star::uno::RuntimeException )
{
    // not interested in ...
    // (our parent is a form which means we get a loaded or reloaded after this rowSetChanged)
}

// ::com::sun::star::form::XLoadListener
//------------------------------------------------------------------------------
void FmXGridPeer::loaded(const ::com::sun::star::lang::EventObject& rEvent)
{
    updateGrid(m_xCursor);
}

//------------------------------------------------------------------------------
void FmXGridPeer::unloaded(const ::com::sun::star::lang::EventObject& rEvent)
{
    updateGrid( ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > (NULL) );
}

//------------------------------------------------------------------------------
void FmXGridPeer::reloading(const ::com::sun::star::lang::EventObject& aEvent) throw( ::com::sun::star::uno::RuntimeException )
{
    // empty the grid
    updateGrid( ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > (NULL) );
}

//------------------------------------------------------------------------------
void FmXGridPeer::unloading(const ::com::sun::star::lang::EventObject& aEvent) throw( ::com::sun::star::uno::RuntimeException )
{
    // empty the grid
    updateGrid( ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > (NULL) );
}

//------------------------------------------------------------------------------
void FmXGridPeer::reloaded(const ::com::sun::star::lang::EventObject& aEvent) throw( ::com::sun::star::uno::RuntimeException )
{
    updateGrid(m_xCursor);
}

// ::com::sun::star::form::XGridPeer
//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  FmXGridPeer::getColumns() throw( ::com::sun::star::uno::RuntimeException )
{
    return m_xColumns;
}

//------------------------------------------------------------------------------
void FmXGridPeer::addColumnListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xCol)
{
    static ::rtl::OUString aPropsListenedTo[] =
    {
        FM_PROP_LABEL, FM_PROP_WIDTH, FM_PROP_HIDDEN, FM_PROP_ALIGN, FM_PROP_FORMATKEY
    };

    // as not all properties have to be supported by all columns we have to check this
    // before adding a listener
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  xInfo = xCol->getPropertySetInfo();
    for (sal_uInt16 i=0; i<sizeof(aPropsListenedTo)/sizeof(aPropsListenedTo[0]); ++i)
        if (xInfo->hasPropertyByName(aPropsListenedTo[i]))
            xCol->addPropertyChangeListener(aPropsListenedTo[i], this);
}

//------------------------------------------------------------------------------
void FmXGridPeer::removeColumnListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xCol)
{
    // the same props as in addColumnListeners ... linux has problems with global static UStrings, so
    // we have to do it this way ....
    static ::rtl::OUString aPropsListenedTo[] =
    {
        FM_PROP_LABEL, FM_PROP_WIDTH, FM_PROP_HIDDEN, FM_PROP_ALIGN, FM_PROP_FORMATKEY
    };

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  xInfo = xCol->getPropertySetInfo();
    for (sal_uInt16 i=0; i<sizeof(aPropsListenedTo)/sizeof(aPropsListenedTo[0]); ++i)
        if (xInfo->hasPropertyByName(aPropsListenedTo[i]))
            xCol->removePropertyChangeListener(aPropsListenedTo[i], this);
}

//------------------------------------------------------------------------------
void FmXGridPeer::setColumns(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >& Columns) throw( ::com::sun::star::uno::RuntimeException )
{
    Window* pWin = GetWindow();

    if (m_xColumns.is())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xCol;
        for (sal_Int32 i = 0; i < m_xColumns->getCount(); i++)
        {
            ::cppu::extractInterface(xCol, m_xColumns->getByIndex(i));
            removeColumnListeners(xCol);
        }
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >  xContainer(m_xColumns, ::com::sun::star::uno::UNO_QUERY);
        xContainer->removeContainerListener(this);

        ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionSupplier >  xSelSupplier(m_xColumns, ::com::sun::star::uno::UNO_QUERY);
        xSelSupplier->removeSelectionChangeListener(this);

        ::com::sun::star::uno::Reference< ::com::sun::star::form::XReset >  xColumnReset(m_xColumns, ::com::sun::star::uno::UNO_QUERY);
        if (xColumnReset.is())
            xColumnReset->removeResetListener((::com::sun::star::form::XResetListener*)this);
    }
    if (Columns.is())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >  xContainer(Columns, ::com::sun::star::uno::UNO_QUERY);
        xContainer->addContainerListener(this);

        ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionSupplier >  xSelSupplier(Columns, ::com::sun::star::uno::UNO_QUERY);
        xSelSupplier->addSelectionChangeListener(this);

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xCol;
        for (sal_Int32 i = 0; i < Columns->getCount(); i++)
        {
            ::cppu::extractInterface(xCol, Columns->getByIndex(i));
            addColumnListeners(xCol);
        }

        ::com::sun::star::uno::Reference< ::com::sun::star::form::XReset >  xColumnReset(Columns, ::com::sun::star::uno::UNO_QUERY);
        if (xColumnReset.is())
            xColumnReset->addResetListener((::com::sun::star::form::XResetListener*)this);
    }
    m_xColumns = Columns;
    if (pWin)
    {
        ((FmGridControl*) pWin)->InitColumnsByModels(m_xColumns);

        if (m_xColumns.is())
        {
            ::com::sun::star::lang::EventObject aEvt(m_xColumns);
            selectionChanged(aEvt);
        }
    }
}

//------------------------------------------------------------------------------
void FmXGridPeer::setDesignMode(sal_Bool bOn)
{
    if (bOn != isDesignMode())
    {
        Window* pWin = GetWindow();
        if (pWin)
            ((FmGridControl*) pWin)->SetDesignMode(bOn);
    }

    if (bOn)
        DisConnectFromDispatcher();
    else
        UpdateDispatches(); // will connect if not already connected and just update else
}

//------------------------------------------------------------------------------
sal_Bool FmXGridPeer::isDesignMode()
{
    Window* pWin = GetWindow();
    if (pWin)
        return ((FmGridControl*) pWin)->IsDesignMode();
    else
        return sal_False;
}

//------------------------------------------------------------------------------
void FmXGridPeer::elementInserted(const ::com::sun::star::container::ContainerEvent& evt)
{
    FmGridControl* pGrid = (FmGridControl*) GetWindow();
    // Handle Column beruecksichtigen
    if (!pGrid || !m_xColumns.is() || pGrid->IsInColumnMove() || m_xColumns->getCount() == ((sal_Int32)pGrid->GetModelColCount()))
        return;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xSet;
    ::cppu::extractInterface(xSet, evt.Element);
    addColumnListeners(xSet);

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xNewColumn(xSet);
    String aName = ::utl::getString(xNewColumn->getPropertyValue(FM_PROP_LABEL));
    ::com::sun::star::uno::Any aWidth = xNewColumn->getPropertyValue(FM_PROP_WIDTH);
    sal_Int32 nWidth = 0;
    if (aWidth >>= nWidth)
        nWidth = pGrid->LogicToPixel(Point(nWidth,0),MAP_10TH_MM).X();

    pGrid->AppendColumn(aName, nWidth, (sal_Int16)::utl::getINT32(evt.Accessor));

    // jetzt die Spalte setzen
    DbGridColumn* pCol = pGrid->GetColumns().GetObject(::utl::getINT32(evt.Accessor));
    pCol->setModel(xNewColumn);

    ::com::sun::star::uno::Any aHidden = xNewColumn->getPropertyValue(FM_PROP_HIDDEN);
    if (::utl::getBOOL(aHidden))
        pGrid->HideColumn(pCol->GetId());
}

//------------------------------------------------------------------------------
void FmXGridPeer::elementReplaced(const ::com::sun::star::container::ContainerEvent& evt)
{
    FmGridControl* pGrid = (FmGridControl*) GetWindow();

    // Handle Column beruecksichtigen
    if (!pGrid || !m_xColumns.is() || pGrid->IsInColumnMove())
        return;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xNewColumn;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xOldColumn;
    ::cppu::extractInterface(xNewColumn, evt.Element);
    ::cppu::extractInterface(xOldColumn, evt.ReplacedElement);

    pGrid->RemoveColumn(pGrid->GetColumnIdFromModelPos(::utl::getINT32(evt.Accessor)));
    removeColumnListeners(xOldColumn);

    String aName = ::utl::getString(xNewColumn->getPropertyValue(FM_PROP_LABEL));
    ::com::sun::star::uno::Any aWidth = xNewColumn->getPropertyValue(FM_PROP_WIDTH);
    sal_Int32 nWidth = 0;
    if (aWidth >>= nWidth)
        nWidth = pGrid->LogicToPixel(Point(nWidth,0),MAP_10TH_MM).X();
    pGrid->AppendColumn(aName, nWidth, (sal_Int16)::utl::getINT32(evt.Accessor));

    addColumnListeners(xNewColumn);
}

//------------------------------------------------------------------------------
void FmXGridPeer::elementRemoved(const ::com::sun::star::container::ContainerEvent& evt)
{
    FmGridControl* pGrid    = (FmGridControl*) GetWindow();

    // Handle Column beruecksichtigen
    if (!pGrid || !m_xColumns.is() || pGrid->IsInColumnMove() || m_xColumns->getCount() == ((sal_Int32)pGrid->GetModelColCount()))
        return;

    pGrid->RemoveColumn(pGrid->GetColumnIdFromModelPos(::utl::getINT32(evt.Accessor)));

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xOldColumn;
    ::cppu::extractInterface(xOldColumn, evt.Element);
    removeColumnListeners(xOldColumn);
}

//------------------------------------------------------------------------------
void FmXGridPeer::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw( ::com::sun::star::uno::RuntimeException )
{
    sal_uInt16 nId = FmPropertyInfoService::getPropertyId(PropertyName);
    FmGridControl* pGrid = (FmGridControl*) GetWindow();
    sal_Bool bVoid = !Value.hasValue();
    switch (nId)
    {
        case FM_ATTR_HELPURL:
        {
            String sHelpURL(::utl::getString(Value));
            String sPattern;
            sPattern.AssignAscii("HID:");
            if (sHelpURL.Equals(sPattern, 0, sPattern.Len()))
            {
                String sID = sHelpURL.Copy(sPattern.Len());
                pGrid->SetHelpId(sID.ToInt32());
            }
        }
        break;
        case FM_ATTR_DISPLAYSYNCHRON:
            pGrid->setDisplaySynchron(::utl::getBOOL(Value));
            break;
        case FM_ATTR_CURSORCOLOR:
            if (bVoid)
                pGrid->SetCursorColor(COL_TRANSPARENT);
            else
                pGrid->SetCursorColor(Color(::utl::getINT32(Value)));
            if (isDesignMode())
                pGrid->Invalidate();
            break;
        case FM_ATTR_ALWAYSSHOWCURSOR:
            pGrid->EnablePermanentCursor(::utl::getBOOL(Value));
            if (isDesignMode())
                pGrid->Invalidate();
            break;
        case FM_ATTR_FONT:
        {
            if ( bVoid )
                pGrid->SetControlFont( Font() );
            else
            {
                ::com::sun::star::awt::FontDescriptor aFont;
                if (Value >>= aFont)
                {
                    if (::utl::operator==(aFont, ::utl::getDefaultFont()))  // ist das der Default
                        pGrid->SetControlFont( Font() );
                    else
                        pGrid->SetControlFont( ImplCreateFont( aFont ) );

                    // if our row-height property is void (which means "calculate it font-dependent") we have
                    // to adjust the control's row height
                    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xModelSet(getColumns(), ::com::sun::star::uno::UNO_QUERY);
                    if (xModelSet.is() && ::utl::hasProperty(FM_PROP_ROWHEIGHT, xModelSet))
                    {
                        ::com::sun::star::uno::Any aHeight = xModelSet->getPropertyValue(FM_PROP_ROWHEIGHT);
                        if (!aHeight.hasValue())
                            pGrid->SetDataRowHeight(0);
                    }

                }
            }
        }
        break;
        case FM_ATTR_BACKGROUNDCOLOR:
            if ( bVoid )
            {
                pGrid->SetControlBackground();
            }
            else
            {
                Color aColor( ::utl::getINT32(Value) );
                pGrid->SetBackground( aColor );
                pGrid->SetControlBackground( aColor );
            }
        break;
        case FM_ATTR_TEXTCOLOR:
            if ( bVoid )
            {
                pGrid->SetControlForeground();
            }
            else
            {
                Color aColor( ::utl::getINT32(Value) );
                pGrid->SetTextColor( aColor );
                pGrid->SetControlForeground( aColor );
            }
        break;
        case FM_ATTR_ROWHEIGHT:
        {
            sal_Int32 nLogHeight(0);
            if (Value >>= nLogHeight)
            {
                sal_Int32 nHeight = pGrid->LogicToPixel(Point(0,nLogHeight),MAP_10TH_MM).Y();
                // take the zoom factor into account
                nHeight = pGrid->CalcZoom(nHeight);
                pGrid->SetDataRowHeight(nHeight);
            }
            else if (bVoid)
                pGrid->SetDataRowHeight(0);
        }   break;
        case FM_ATTR_HASNAVIGATION:
        {
            if (Value.getValueType() == ::getBooleanCppuType())
                pGrid->EnableNavigationBar(*(sal_Bool*)Value.getValue());
        }   break;
        case FM_ATTR_RECORDMARKER:
        {
            if (Value.getValueType() == ::getBooleanCppuType())
                pGrid->EnableHandle(*(sal_Bool*)Value.getValue());
        }   break;
        case FM_ATTR_ENABLED:
        {
            if (Value.getValueType() == ::getBooleanCppuType())
            {
                // Im DesignModus nur das Datenfenster disablen
                // Sonst kann das Control nicht mehr konfiguriert werden
                if (isDesignMode())
                    pGrid->GetDataWindow().Enable(*(sal_Bool*)Value.getValue());
                else
                    pGrid->Enable(*(sal_Bool*)Value.getValue());
            }
        }   break;
        default:
            VCLXWindow::setProperty( PropertyName, Value );
    }
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Any FmXGridPeer::getProperty( const ::rtl::OUString& PropertyName ) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Any aProp;
    if (GetWindow())
    {
        FmGridControl* pGrid = (FmGridControl*) GetWindow();
        Window* pDataWindow  = &pGrid->GetDataWindow();
        sal_uInt16 nId = FmPropertyInfoService::getPropertyId(PropertyName);
        switch (nId)
        {
            case FM_ATTR_FONT:
            {
                Font aFont = pDataWindow->GetControlFont();
                aProp <<= ImplCreateFontDescriptor( aFont );
            }   break;
            case FM_ATTR_TEXTCOLOR:
                aProp <<= (sal_Int32)pDataWindow->GetControlForeground().GetColor();
                break;
            case FM_ATTR_BACKGROUNDCOLOR:
                aProp <<= (sal_Int32)pDataWindow->GetControlBackground().GetColor();
                break;
            case FM_ATTR_ROWHEIGHT:
            {
                sal_Int32 nPixelHeight = pGrid->GetDataRowHeight();
                // take the zoom factor into account
                nPixelHeight = pGrid->CalcReverseZoom(nPixelHeight);
                aProp <<= (sal_Int32)pGrid->PixelToLogic(Point(0,nPixelHeight),MAP_10TH_MM).Y();
            }   break;
            case FM_ATTR_HASNAVIGATION:
            {
                sal_Bool bHasNavBar = pGrid->HasNavigationBar();
                aProp <<= (sal_Bool)bHasNavBar;
            }   break;
            case FM_ATTR_RECORDMARKER:
            {
                sal_Bool bHasHandle = pGrid->HasHandle();
                aProp <<= (sal_Bool)bHasHandle;
            }   break;
            case FM_ATTR_ENABLED:
            {
                aProp <<= (sal_Bool)pDataWindow->IsEnabled();
            }   break;
            default:
                aProp = VCLXWindow::getProperty( PropertyName );
        }
    }
    return aProp;
}

//------------------------------------------------------------------------------
void FmXGridPeer::dispose() throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::lang::EventObject aEvt;
    aEvt.Source = static_cast< ::cppu::OWeakObject* >(this);
    m_aModifyListeners.disposeAndClear(aEvt);
    m_aUpdateListeners.disposeAndClear(aEvt);
    m_aContainerListeners.disposeAndClear(aEvt);
    VCLXWindow::dispose();

    DisConnectFromDispatcher();
    setRowSet(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > ());
}

// ::com::sun::star::container::XContainer
//------------------------------------------------------------------------------
void FmXGridPeer::addContainerListener(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& l) throw( ::com::sun::star::uno::RuntimeException )
{
    m_aContainerListeners.addInterface( l );
}
//------------------------------------------------------------------------------
void FmXGridPeer::removeContainerListener(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& l) throw( ::com::sun::star::uno::RuntimeException )
{
    m_aContainerListeners.removeInterface( l );
}

// ::com::sun::star::data::XDatabaseCursorSupplier
//------------------------------------------------------------------------------
void FmXGridPeer::startCursorListening()
{
    if (!m_nCursorListening)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >  xRowSet(m_xCursor, ::com::sun::star::uno::UNO_QUERY);
        if (xRowSet.is())
            xRowSet->addRowSetListener(this);

        ::com::sun::star::uno::Reference< ::com::sun::star::form::XReset >  xReset(m_xCursor, ::com::sun::star::uno::UNO_QUERY);
        if (xReset.is())
            xReset->addResetListener(this);

        // alle Listener anmelden
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xSet(m_xCursor, ::com::sun::star::uno::UNO_QUERY);
        if (xSet.is())
        {
            xSet->addPropertyChangeListener(FM_PROP_ISMODIFIED, this);
            xSet->addPropertyChangeListener(FM_PROP_ROWCOUNT, this);
        }
    }
    m_nCursorListening++;
}

//------------------------------------------------------------------------------
void FmXGridPeer::stopCursorListening()
{
    if (!--m_nCursorListening)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >  xRowSet(m_xCursor, ::com::sun::star::uno::UNO_QUERY);
        if (xRowSet.is())
            xRowSet->removeRowSetListener(this);

        ::com::sun::star::uno::Reference< ::com::sun::star::form::XReset >  xReset(m_xCursor, ::com::sun::star::uno::UNO_QUERY);
        if (xReset.is())
            xReset->removeResetListener(this);

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xSet(m_xCursor, ::com::sun::star::uno::UNO_QUERY);
        if (xSet.is())
        {
            xSet->removePropertyChangeListener(FM_PROP_ISMODIFIED, this);
            xSet->removePropertyChangeListener(FM_PROP_ROWCOUNT, this);
        }
    }
}

//------------------------------------------------------------------------------
void FmXGridPeer::updateGrid(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& _rxCursor)
{
    FmGridControl* pGrid = (FmGridControl*)GetWindow();
    if (pGrid)
        pGrid->setDataSource(_rxCursor);
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >  FmXGridPeer::getRowSet() throw( ::com::sun::star::uno::RuntimeException )
{
    return m_xCursor;
}

//------------------------------------------------------------------------------
void FmXGridPeer::setRowSet(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& _rDatabaseCursor) throw( ::com::sun::star::uno::RuntimeException )
{
    FmGridControl* pGrid = (FmGridControl*) GetWindow();
    if (!pGrid || !m_xColumns.is() || !m_xColumns->getCount())
        return;
    // alle Listener abmelden
    if (m_xCursor.is())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable >  xLoadable(m_xCursor, ::com::sun::star::uno::UNO_QUERY);
        // only if the form is loaded we set the rowset
        if (xLoadable.is())
        {
            stopCursorListening();
            xLoadable->removeLoadListener(this);
        }
    }

    m_xCursor = _rDatabaseCursor;

    if (pGrid)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable >  xLoadable(m_xCursor, ::com::sun::star::uno::UNO_QUERY);
        // only if the form is loaded we set the rowset
        if (xLoadable.is() && xLoadable->isLoaded())
            pGrid->setDataSource(m_xCursor);
        else
            pGrid->setDataSource(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > ());

        if (xLoadable.is())
        {
            startCursorListening();
            xLoadable->addLoadListener(this);
        }
    }
}

//------------------------------------------------------------------------------
sal_Int16 FmXGridPeer::getCurrentColumnPosition()
{
    FmGridControl* pGrid = (FmGridControl*) GetWindow();
    return pGrid ? pGrid->GetViewColumnPos(pGrid->GetCurColumnId()) : -1;
}

//------------------------------------------------------------------------------
void FmXGridPeer::setCurrentColumnPosition(sal_Int16 nPos)
{
    FmGridControl* pGrid = (FmGridControl*) GetWindow();
    if (pGrid)
        pGrid->GoToColumnId(pGrid->GetColumnIdFromViewPos(nPos));
}

//------------------------------------------------------------------------------
void FmXGridPeer::selectionChanged(const ::com::sun::star::lang::EventObject& evt)
{
    FmGridControl* pGrid = (FmGridControl*) GetWindow();
    if (pGrid)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionSupplier >  xSelSupplier(evt.Source, ::com::sun::star::uno::UNO_QUERY);
        ::com::sun::star::uno::Any aSelection = xSelSupplier->getSelection();
        DBG_ASSERT(aSelection.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_INTERFACE, "FmXGridPeer::selectionChanged : invalid selection !");
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xSelection;
        ::cppu::extractInterface(xSelection, aSelection);
        if (xSelection.is())
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xCol;
            for (sal_uInt32 i = 0; i < m_xColumns->getCount(); i++)
            {
                ::cppu::extractInterface(xCol, m_xColumns->getByIndex(i));
                if (xCol == xSelection)
                {
                    pGrid->markColumn(pGrid->GetColumnIdFromModelPos(i));
                    break;
                }
            }
        }
        else
            pGrid->markColumn(USHRT_MAX);
    }
}

// ::com::sun::star::container::XElementAccess
//------------------------------------------------------------------------------
sal_Bool FmXGridPeer::hasElements() throw( ::com::sun::star::uno::RuntimeException )
{
    return getCount() != 0;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Type SAL_CALL FmXGridPeer::getElementType(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::getCppuType((::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> *)NULL);
}

// ::com::sun::star::container::XEnumerationAccess
//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  FmXGridPeer::createEnumeration() throw( ::com::sun::star::uno::RuntimeException )
{
    return new ::utl::OEnumerationByIndex(this);
}

// ::com::sun::star::container::XIndexAccess
//------------------------------------------------------------------------------
sal_Int32 FmXGridPeer::getCount() throw( ::com::sun::star::uno::RuntimeException )
{
    FmGridControl* pGrid = (FmGridControl*) GetWindow();
    if (pGrid)
        return pGrid->GetViewColCount();
    else
        return 0;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Any FmXGridPeer::getByIndex(sal_Int32 _nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException )
{
    FmGridControl* pGrid = (FmGridControl*) GetWindow();
    if (_nIndex < 0 ||
        _nIndex >= getCount() || !pGrid)
        throw ::com::sun::star::lang::IndexOutOfBoundsException();

    ::com::sun::star::uno::Any aElement;
    // get the columnid
    sal_uInt16 nId = pGrid->GetColumnIdFromViewPos(_nIndex);
    // get the list position
    sal_uInt16 nPos = pGrid->GetModelColumnPos(nId);

    DbGridColumn* pCol = pGrid->GetColumns().GetObject(nPos);
//  DBG_ASSERT(pCol && pCol->GetCell(), "FmXGridPeer::getByIndex(): Invalid cell");
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  xControl(pCol->GetCell());
    aElement <<= xControl;

    return aElement;
}

// ::com::sun::star::util::XModeSelector
//------------------------------------------------------------------------------
void FmXGridPeer::setMode(const ::rtl::OUString& Mode) throw( ::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException )
{
    if (!supportsMode(Mode))
        throw ::com::sun::star::lang::NoSupportException();

    if (Mode == m_aMode)
        return;

    m_aMode = Mode;

    FmGridControl* pGrid = (FmGridControl*) GetWindow();
    if (Mode == FILTER_MODE)
        pGrid->SetFilterMode(sal_True);
    else
    {
        pGrid->SetFilterMode(sal_False);
        pGrid->setDataSource(m_xCursor);
    }
}

//------------------------------------------------------------------------------
::rtl::OUString FmXGridPeer::getMode() throw( ::com::sun::star::uno::RuntimeException )
{
    return m_aMode;
}

//------------------------------------------------------------------------------
::utl::StringSequence FmXGridPeer::getSupportedModes() throw( ::com::sun::star::uno::RuntimeException )
{
    static ::utl::StringSequence aModes;
    if (!aModes.getLength())
    {
        aModes.realloc(2);
        ::rtl::OUString* pModes = aModes.getArray();
        pModes[0] = DATA_MODE;
        pModes[1] = FILTER_MODE;
    }
    return aModes;
}

//------------------------------------------------------------------------------
sal_Bool FmXGridPeer::supportsMode(const ::rtl::OUString& Mode) throw( ::com::sun::star::uno::RuntimeException )
{
    ::utl::StringSequence aModes(getSupportedModes());
    const ::rtl::OUString* pModes = aModes.getConstArray();
    for (sal_Int32 i = aModes.getLength(); i > 0; )
    {
        if (pModes[--i] == Mode)
            return sal_True;
    }
    return sal_False;
}

//------------------------------------------------------------------------------
void FmXGridPeer::columnVisible(DbGridColumn* pColumn)
{
    FmGridControl* pGrid = (FmGridControl*) GetWindow();

    sal_Int32 _nIndex = pGrid->GetModelColumnPos(pColumn->GetId());
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  xControl(pColumn->GetCell());
    ::com::sun::star::container::ContainerEvent aEvt;
    aEvt.Source   = (::com::sun::star::container::XContainer*)this;
    aEvt.Accessor <<= _nIndex;
    aEvt.Element  <<= xControl;

    NOTIFY_LISTENERS(m_aContainerListeners, ::com::sun::star::container::XContainerListener, elementInserted, aEvt);
}

//------------------------------------------------------------------------------
void FmXGridPeer::columnHidden(DbGridColumn* pColumn)
{
    FmGridControl* pGrid = (FmGridControl*) GetWindow();

    sal_Int32 _nIndex = pGrid->GetModelColumnPos(pColumn->GetId());
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  xControl(pColumn->GetCell());
    ::com::sun::star::container::ContainerEvent aEvt;
    aEvt.Source   = (::com::sun::star::container::XContainer*)this;
    aEvt.Accessor <<= _nIndex;
    aEvt.Element  <<= xControl;

    NOTIFY_LISTENERS(m_aContainerListeners, ::com::sun::star::container::XContainerListener, elementRemoved, aEvt);
}

//------------------------------------------------------------------------------
void FmXGridPeer::draw( long x, long y )
{
    FmGridControl* pGrid = (FmGridControl*) GetWindow();
    sal_Int32 nOldFlags = pGrid->GetBrowserFlags();
    pGrid->SetBrowserFlags(nOldFlags | DBBF_NOROWPICTURE);

    VCLXWindow::draw(x, y);

    pGrid->SetBrowserFlags(nOldFlags);
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  FmXGridPeer::queryDispatch(const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  xResult;

    // first ask our interceptor chain
    if (m_xFirstDispatchInterceptor.is() && !m_bInterceptingDispatch)
    {
        m_bInterceptingDispatch = sal_True;
            // safety against recursion : as we are master of the first chain element and slave of the last one we would
            // have an infinite loop without this if no dispatcher can fullfill the rewuest)
        xResult = m_xFirstDispatchInterceptor->queryDispatch(aURL, aTargetFrameName, nSearchFlags);
        m_bInterceptingDispatch = sal_False;
    }

    // then ask ourself : we don't have any dispatches
    return xResult;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > FmXGridPeer::queryDispatches(const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor>& aDescripts) throw( ::com::sun::star::uno::RuntimeException )
{
    if (m_xFirstDispatchInterceptor.is())
        return m_xFirstDispatchInterceptor->queryDispatches(aDescripts);

    // then ask ourself : we don't have any dispatches
    return ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > >();
}

//------------------------------------------------------------------------------
void FmXGridPeer::registerDispatchProviderInterceptor(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& _xInterceptor) throw( ::com::sun::star::uno::RuntimeException )
{
    if (_xInterceptor.is())
    {
        if (m_xFirstDispatchInterceptor.is())
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > xFirstProvider(m_xFirstDispatchInterceptor, ::com::sun::star::uno::UNO_QUERY);
            // there is already an interceptor; the new one will become its master
            _xInterceptor->setSlaveDispatchProvider(xFirstProvider);
            m_xFirstDispatchInterceptor->setMasterDispatchProvider(xFirstProvider);
        }
        else
        {
            // it is the first interceptor; set ourself as slave
            _xInterceptor->setSlaveDispatchProvider((::com::sun::star::frame::XDispatchProvider*)this);
        }

        // we are the master of the chain's first interceptor
        m_xFirstDispatchInterceptor = _xInterceptor;
        m_xFirstDispatchInterceptor->setMasterDispatchProvider((::com::sun::star::frame::XDispatchProvider*)this);

        // we have a new interceptor and we're alive ?
        if (!isDesignMode())
            // -> check for new dispatchers
            UpdateDispatches();
    }
}

//------------------------------------------------------------------------------
void FmXGridPeer::releaseDispatchProviderInterceptor(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& _xInterceptor) throw( ::com::sun::star::uno::RuntimeException )
{
    if (!_xInterceptor.is())
        return;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >  xChainWalk(m_xFirstDispatchInterceptor);

    if (m_xFirstDispatchInterceptor == _xInterceptor)
    {   // our chain will have a new first element
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >  xSlave(m_xFirstDispatchInterceptor->getSlaveDispatchProvider(), ::com::sun::star::uno::UNO_QUERY);
        m_xFirstDispatchInterceptor = xSlave;
    }
    // do this before removing the interceptor from the chain as we won't know it's slave afterwards)

    while (xChainWalk.is())
    {
        // walk along the chain of interceptors and look for the interceptor that has to be removed
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >  xSlave(xChainWalk->getSlaveDispatchProvider(), ::com::sun::star::uno::UNO_QUERY);

        if (xChainWalk == _xInterceptor)
        {
            // old master may be an interceptor too
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >  xMaster(xChainWalk->getMasterDispatchProvider(), ::com::sun::star::uno::UNO_QUERY);

            // unchain the interceptor that has to be removed
            xChainWalk->setSlaveDispatchProvider(::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > ());
            xChainWalk->setMasterDispatchProvider(::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > ());

            // reconnect the chain
            if (xMaster.is())
            {
                if (xSlave.is())
                    xMaster->setSlaveDispatchProvider(::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >::query(xSlave));
                else
                    // it's the first interceptor of the chain, set ourself as slave
                    xMaster->setSlaveDispatchProvider((::com::sun::star::frame::XDispatchProvider*)this);
            }
            else
            {
                // the chain's first element was removed, set ourself as new master of the second one
                if (xSlave.is())
                    xSlave->setMasterDispatchProvider((::com::sun::star::frame::XDispatchProvider*)this);
            }
        }

        xChainWalk = xSlave;
    }
    // our interceptor chain has changed and we're alive ?
    if (!isDesignMode())
        // -> check the dispatchers
        UpdateDispatches();
}

//------------------------------------------------------------------------------
void FmXGridPeer::statusChanged(const ::com::sun::star::frame::FeatureStateEvent& Event) throw( ::com::sun::star::uno::RuntimeException )
{
    DBG_ASSERT(m_pStateCache, "FmXGridPeer::statusChanged : invalid call !");
    DBG_ASSERT(m_pDispatchers, "FmXGridPeer::statusChanged : invalid call !");

    ::com::sun::star::uno::Sequence< ::com::sun::star::util::URL>& aUrls = getSupportedURLs();
    const ::com::sun::star::util::URL* pUrls = aUrls.getConstArray();

    ::com::sun::star::uno::Sequence<sal_uInt16> aSlots = getSupportedGridSlots();
    const sal_uInt16* pSlots = aSlots.getConstArray();

    for (sal_uInt16 i=0; i<aUrls.getLength(); ++i, ++pUrls, ++pSlots)
    {
        if (pUrls->Main == Event.FeatureURL.Main)
        {
            DBG_ASSERT(m_pDispatchers[i] == Event.Source, "FmXGridPeer::statusChanged : the event source is a little bit suspect !");
            m_pStateCache[i] = Event.IsEnabled;
            FmGridControl* pGrid = (FmGridControl*) GetWindow();
            if (*pSlots != SID_FM_RECORD_UNDO)
                pGrid->GetNavigationBar().InvalidateState(*pSlots);
            break;
        }
    }
    DBG_ASSERT(i<aUrls.getLength(), "FmXGridPeer::statusChanged : got a call for an unknown url !");
}

//------------------------------------------------------------------------------
sal_Bool FmXGridPeer::approveReset(const ::com::sun::star::lang::EventObject& rEvent) throw( ::com::sun::star::uno::RuntimeException )
{
    return sal_True;
}

//------------------------------------------------------------------------------
void FmXGridPeer::resetted(const ::com::sun::star::lang::EventObject& rEvent) throw( ::com::sun::star::uno::RuntimeException )
{
    if (m_xColumns == rEvent.Source)
    {   // my model was reset -> refresh the grid content
        FmGridControl* pGrid = (FmGridControl*)GetWindow();
        if (!pGrid)
            return;
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        pGrid->resetCurrentRow();
    }
    // if the cursor fired a reset event we seem to be on the insert row
    else if (m_xCursor == rEvent.Source)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        FmGridControl* pGrid = (FmGridControl*) GetWindow();
        if (pGrid && pGrid->IsOpen())
            pGrid->positioned(rEvent);
    }
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Sequence<sal_uInt16>& FmXGridPeer::getSupportedGridSlots()
{
    static ::com::sun::star::uno::Sequence<sal_uInt16> aSupported;
    if (aSupported.getLength() == 0)
    {
        sal_uInt16 nSupported[] = {
            DbGridControl::NavigationBar::RECORD_FIRST,
            DbGridControl::NavigationBar::RECORD_PREV,
            DbGridControl::NavigationBar::RECORD_NEXT,
            DbGridControl::NavigationBar::RECORD_LAST,
            DbGridControl::NavigationBar::RECORD_NEW,
            SID_FM_RECORD_UNDO
        };
        aSupported.realloc(sizeof(nSupported)/sizeof(nSupported[0]));
        sal_uInt16* pSupported = aSupported.getArray();
        for (sal_uInt16 i=0; i<aSupported.getLength(); ++i, ++pSupported)
            *pSupported = nSupported[i];
    }
    return aSupported;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::util::URL>& FmXGridPeer::getSupportedURLs()
{
    static ::com::sun::star::uno::Sequence< ::com::sun::star::util::URL> aSupported;
    if (aSupported.getLength() == 0)
    {
        static ::rtl::OUString sSupported[] = {
            FMURL_RECORD_MOVEFIRST,
            FMURL_RECORD_MOVEPREV,
            FMURL_RECORD_MOVENEXT,
            FMURL_RECORD_MOVELAST,
            FMURL_RECORD_MOVETONEW,
            FMURL_RECORD_UNDO
        };
        aSupported.realloc(sizeof(sSupported)/sizeof(sSupported[0]));
        ::com::sun::star::util::URL* pSupported = aSupported.getArray();
        for (sal_uInt16 i=0; i<aSupported.getLength(); ++i, ++pSupported)
            pSupported->Complete = sSupported[i];

        // let an ::com::sun::star::util::URL-transformer normalize the URLs
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >  xTransformer(
            ::utl::getProcessServiceFactory()->createInstance(
                ::rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer")),
            ::com::sun::star::uno::UNO_QUERY);
        pSupported = aSupported.getArray();
        if (xTransformer.is())
        {
            for (i=0; i<aSupported.getLength(); ++i)
                xTransformer->parseStrict(pSupported[i]);
        }
    }

    return aSupported;
}

//------------------------------------------------------------------------------
void FmXGridPeer::UpdateDispatches()
{
    if (!m_pStateCache)
    {   // we don't have any dispatchers yet -> do the initial connect
        ConnectToDispatcher();
        return;
    }

    sal_uInt16 nDispatchersGot = 0;
    const ::com::sun::star::uno::Sequence< ::com::sun::star::util::URL>& aSupportedURLs = getSupportedURLs();
    const ::com::sun::star::util::URL* pSupportedURLs = aSupportedURLs.getConstArray();
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  xNewDispatch;
    for (sal_uInt16 i=0; i<aSupportedURLs.getLength(); ++i, ++pSupportedURLs)
    {
        xNewDispatch = queryDispatch(*pSupportedURLs, rtl::OUString(), 0);
        if (xNewDispatch != m_pDispatchers[i])
        {
            if (m_pDispatchers[i].is())
                m_pDispatchers[i]->removeStatusListener((::com::sun::star::frame::XStatusListener*)this, *pSupportedURLs);
            m_pDispatchers[i] = xNewDispatch;
            if (m_pDispatchers[i].is())
                m_pDispatchers[i]->addStatusListener((::com::sun::star::frame::XStatusListener*)this, *pSupportedURLs);
        }
        if (m_pDispatchers[i].is())
            ++nDispatchersGot;
    }

    if (!nDispatchersGot)
    {
        delete[] m_pStateCache;
        delete[] m_pDispatchers;
        m_pStateCache = NULL;
        m_pDispatchers = NULL;
    }
}

//------------------------------------------------------------------------------
void FmXGridPeer::ConnectToDispatcher()
{
    DBG_ASSERT((m_pStateCache != NULL) == (m_pDispatchers != NULL), "FmXGridPeer::ConnectToDispatcher : inconsistent !");
    if (m_pStateCache)
    {   // already connected -> just do an update
        UpdateDispatches();
        return;
    }

    const ::com::sun::star::uno::Sequence< ::com::sun::star::util::URL>& aSupportedURLs = getSupportedURLs();

    // _before_ adding the status listeners (as the add should result in a statusChanged-call) !
    m_pStateCache = new sal_Bool[aSupportedURLs.getLength()];
    m_pDispatchers = new ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > [aSupportedURLs.getLength()];

    sal_uInt16 nDispatchersGot = 0;
    const ::com::sun::star::util::URL* pSupportedURLs = aSupportedURLs.getConstArray();
    for (sal_uInt16 i=0; i<aSupportedURLs.getLength(); ++i, ++pSupportedURLs)
    {
        m_pStateCache[i] = 0;
        m_pDispatchers[i] = queryDispatch(*pSupportedURLs, rtl::OUString(), 0);
        if (m_pDispatchers[i].is())
        {
            m_pDispatchers[i]->addStatusListener((::com::sun::star::frame::XStatusListener*)this, *pSupportedURLs);
            ++nDispatchersGot;
        }
    }

    if (!nDispatchersGot)
    {
        delete[] m_pStateCache;
        delete[] m_pDispatchers;
        m_pStateCache = NULL;
        m_pDispatchers = NULL;
    }
}

//------------------------------------------------------------------------------
void FmXGridPeer::DisConnectFromDispatcher()
{
    if (!m_pStateCache || !m_pDispatchers)
        return;
    // we're not connected

    const ::com::sun::star::uno::Sequence< ::com::sun::star::util::URL>& aSupportedURLs = getSupportedURLs();
    const ::com::sun::star::util::URL* pSupportedURLs = aSupportedURLs.getConstArray();
    for (sal_uInt16 i=0; i<aSupportedURLs.getLength(); ++i, ++pSupportedURLs)
    {
        if (m_pDispatchers[i].is())
            m_pDispatchers[i]->removeStatusListener((::com::sun::star::frame::XStatusListener*)this, *pSupportedURLs);
    }

    delete[] m_pStateCache;
    delete[] m_pDispatchers;
    m_pStateCache = NULL;
    m_pDispatchers = NULL;
}

//------------------------------------------------------------------------------
IMPL_LINK(FmXGridPeer, OnQueryGridSlotState, void*, pSlot)
{
    if (!m_pStateCache)
        return -1;  // unspecified

    sal_uInt16 nSlot = (sal_uInt16)(sal_uInt32)pSlot;

    // search the given slot with our supported sequence
    ::com::sun::star::uno::Sequence<sal_uInt16>& aSupported = getSupportedGridSlots();
    const sal_uInt16* pSlots = aSupported.getConstArray();
    for (sal_uInt16 i=0; i<aSupported.getLength(); ++i)
        if (pSlots[i] == nSlot)
            if (!m_pDispatchers[i].is())
                return -1;  // nothing known about this slot
            else
                return m_pStateCache[i];

    return  -1;
}

//------------------------------------------------------------------------------
IMPL_LINK(FmXGridPeer, OnExecuteGridSlot, void*, pSlot)
{
    if (!m_pDispatchers)
        return 0;   // not handled

    ::com::sun::star::uno::Sequence< ::com::sun::star::util::URL>& aUrls = getSupportedURLs();
    const ::com::sun::star::util::URL* pUrls = aUrls.getConstArray();

    ::com::sun::star::uno::Sequence<sal_uInt16> aSlots = getSupportedGridSlots();
    const sal_uInt16* pSlots = aSlots.getConstArray();

    DBG_ASSERT(aSlots.getLength() == aUrls.getLength(), "FmXGridPeer::OnExecuteGridSlot : inconstent data returned by getSupportedURLs/getSupportedGridSlots !");

    sal_uInt16 nSlot = (sal_uInt16)(sal_uInt32)pSlot;
    for (sal_uInt16 i=0; i<aSlots.getLength(); ++i, ++pUrls, ++pSlots)
    {
        if (*pSlots == nSlot)
        {
            if (m_pDispatchers[i].is())
            {
                m_pDispatchers[i]->dispatch(*pUrls, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>());
                return 1;   // handled
            }
        }
    }

    return 0;   // not handled
}

