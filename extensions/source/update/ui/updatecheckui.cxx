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


#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/document/XDocumentEventListener.hpp>
#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/theGlobalEventBroadcaster.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/resmgr.hxx>
#include <vcl/image.hxx>
#include <vcl/menubarupdateicon.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sfx2/strings.hrc>
#include <rtl/ustrbuf.hxx>

#include <bitmaps.hlst>

#define PROPERTY_TITLE          "BubbleHeading"
#define PROPERTY_TEXT           "BubbleText"
#define PROPERTY_IMAGE          "BubbleImageURL"
#define PROPERTY_SHOW_BUBBLE    "BubbleVisible"
#define PROPERTY_CLICK_HDL      "MenuClickHDL"
#define PROPERTY_SHOW_MENUICON  "MenuIconVisible"

using namespace ::com::sun::star;


namespace
{

class UpdateCheckUI : public ::cppu::WeakImplHelper
                        < lang::XServiceInfo, document::XDocumentEventListener, beans::XPropertySet >
{
    uno::Reference< uno::XComponentContext > m_xContext;
    uno::Reference< task::XJob > mrJob;
    OUString       maBubbleImageURL;
    MenuBarUpdateIconManager maBubbleManager;
    std::locale         maSfxLocale;

private:
                    DECL_LINK(ClickHdl, LinkParamNone*, void);

    Image           GetBubbleImage( OUString const &rURL );

public:
    explicit        UpdateCheckUI(const uno::Reference<uno::XComponentContext>&);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(OUString const & serviceName) override;
    virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XDocumentEventListener
    virtual void SAL_CALL documentEventOccured(const document::DocumentEvent& Event) override;
    virtual void SAL_CALL disposing(const lang::EventObject& Event) override;

    //XPropertySet
    virtual uno::Reference< beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue(const OUString& PropertyName, const uno::Any& aValue) override;
    virtual uno::Any SAL_CALL getPropertyValue(const OUString& PropertyName) override;
    virtual void SAL_CALL addPropertyChangeListener(const OUString& PropertyName,
                                                    const uno::Reference< beans::XPropertyChangeListener > & aListener) override;
    virtual void SAL_CALL removePropertyChangeListener(const OUString& PropertyName,
                                                       const uno::Reference< beans::XPropertyChangeListener > & aListener) override;
    virtual void SAL_CALL addVetoableChangeListener(const OUString& PropertyName,
                                                    const uno::Reference< beans::XVetoableChangeListener > & aListener) override;
    virtual void SAL_CALL removeVetoableChangeListener(const OUString& PropertyName,
                                                       const uno::Reference< beans::XVetoableChangeListener > & aListener) override;
};

UpdateCheckUI::UpdateCheckUI(const uno::Reference<uno::XComponentContext>& xContext)
    : m_xContext(xContext)
{
    maSfxLocale = Translate::Create("sfx");

    uno::Reference< document::XDocumentEventBroadcaster > xBroadcaster( frame::theGlobalEventBroadcaster::get(m_xContext) );
    xBroadcaster->addDocumentEventListener( this );

    SolarMutexGuard aGuard;

    maBubbleManager.SetBubbleImage(GetBubbleImage(maBubbleImageURL));
    maBubbleManager.SetClickHdl(LINK(this, UpdateCheckUI, ClickHdl));
}

OUString SAL_CALL
UpdateCheckUI::getImplementationName()
{
    return "vnd.sun.UpdateCheckUI";
}

uno::Sequence< OUString > SAL_CALL
UpdateCheckUI::getSupportedServiceNames()
{
    return { "com.sun.star.setup.UpdateCheckUI" };
}

sal_Bool SAL_CALL
UpdateCheckUI::supportsService( OUString const & serviceName )
{
    return cppu::supportsService(this, serviceName);
}

Image UpdateCheckUI::GetBubbleImage( OUString const &rURL )
{
    Image aImage;

    if ( !maBubbleImageURL.isEmpty() )
    {
        uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();

        if( !xContext.is() )
            throw uno::RuntimeException(
                "UpdateCheckUI: unable to obtain service manager from component context" );

        try
        {
            uno::Reference< graphic::XGraphicProvider > xGraphProvider(graphic::GraphicProvider::create(xContext));
            uno::Sequence< beans::PropertyValue > aMediaProps( 1 );
            aMediaProps[0].Name = "URL";
            aMediaProps[0].Value <<= rURL;

            uno::Reference< graphic::XGraphic > xGraphic = xGraphProvider->queryGraphic( aMediaProps );
            if ( xGraphic.is() )
            {
                aImage = Image( xGraphic );
            }
        }
        catch( const uno::Exception& )
        {
        }
    }

    if ( aImage.GetSizePixel().Width() == 0 )
        aImage = Image(StockImage::Yes, SV_RESID_BITMAP_INFOBOX);

    return aImage;
}

void SAL_CALL UpdateCheckUI::documentEventOccured(const document::DocumentEvent& rEvent)
{
    SolarMutexGuard aGuard;

    if( rEvent.EventName == "OnPrepareViewClosing" )
    {
        maBubbleManager.RemoveBubbleWindow(true);
    }
}

void SAL_CALL UpdateCheckUI::disposing(const lang::EventObject&)
{
}

uno::Reference< beans::XPropertySetInfo > UpdateCheckUI::getPropertySetInfo()
{
    return nullptr;
}

void UpdateCheckUI::setPropertyValue(const OUString& rPropertyName,
                                     const uno::Any& rValue)
{
    SolarMutexGuard aGuard;

    OUString aString;

    if( rPropertyName == PROPERTY_TITLE ) {
        rValue >>= aString;
        maBubbleManager.SetBubbleTitle(aString);
    }
    else if( rPropertyName == PROPERTY_TEXT ) {
        rValue >>= aString;
        maBubbleManager.SetBubbleText(aString);
    }
    else if( rPropertyName == PROPERTY_IMAGE ) {
        rValue >>= aString;
        if ( aString != maBubbleImageURL ) {
            maBubbleImageURL = aString;
            maBubbleManager.SetBubbleImage(GetBubbleImage(maBubbleImageURL));
        }
    }
    else if( rPropertyName == PROPERTY_SHOW_BUBBLE ) {
        bool bShowBubble= false;
        rValue >>= bShowBubble;
        maBubbleManager.SetShowBubble(bShowBubble);
    }
    else if( rPropertyName == PROPERTY_CLICK_HDL ) {
        uno::Reference< task::XJob > aJob;
        rValue >>= aJob;
        if ( !aJob.is() )
            throw lang::IllegalArgumentException();
        mrJob = aJob;
    }
    else if (rPropertyName == PROPERTY_SHOW_MENUICON ) {
        bool bShowMenuIcon = false;
        rValue >>= bShowMenuIcon;
        maBubbleManager.SetShowMenuIcon(bShowMenuIcon);
    }
    else
        throw beans::UnknownPropertyException(rPropertyName);
}

uno::Any UpdateCheckUI::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;

    uno::Any aRet;

    if( rPropertyName == PROPERTY_TITLE )
        aRet <<= maBubbleManager.GetBubbleTitle();
    else if( rPropertyName == PROPERTY_TEXT )
        aRet <<= maBubbleManager.GetBubbleText();
    else if( rPropertyName == PROPERTY_SHOW_BUBBLE )
        aRet <<= maBubbleManager.GetShowBubble();
    else if( rPropertyName == PROPERTY_IMAGE )
        aRet <<= maBubbleImageURL;
    else if( rPropertyName == PROPERTY_CLICK_HDL )
        aRet <<= mrJob;
    else if( rPropertyName == PROPERTY_SHOW_MENUICON )
        aRet <<= maBubbleManager.GetShowMenuIcon();
    else
        throw beans::UnknownPropertyException(rPropertyName);

    return aRet;
}


void UpdateCheckUI::addPropertyChangeListener( const OUString& /*aPropertyName*/,
                                               const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
{
    //no bound properties
}


void UpdateCheckUI::removePropertyChangeListener( const OUString& /*aPropertyName*/,
                                                  const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
{
    //no bound properties
}

void UpdateCheckUI::addVetoableChangeListener( const OUString& /*aPropertyName*/,
                                               const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
{
    //no vetoable properties
}

void UpdateCheckUI::removeVetoableChangeListener( const OUString& /*aPropertyName*/,
                                                  const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
{
    //no vetoable properties
}

IMPL_LINK_NOARG(UpdateCheckUI, ClickHdl, LinkParamNone*, void)
{
    SolarMutexGuard aGuard;

    if ( mrJob.is() )
    {
        try {
            uno::Sequence<beans::NamedValue> aEmpty;
            mrJob->execute( aEmpty );
        }
        catch(const uno::Exception&) {
            std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(nullptr,
                                                           VclMessageType::Warning, VclButtonsType::Ok,
                                                           Translate::get(STR_NO_WEBBROWSER_FOUND, maSfxLocale)));
            xErrorBox->run();
        }
    }
}

} // anonymous namespace


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
extensions_update_UpdateCheckUI_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    SolarMutexGuard aGuard;
    return cppu::acquire(new UpdateCheckUI(context));
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
