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

#include <i18nlangtag/languagetag.hxx>
#include <svl/lngmisc.hxx>
#include <svtools/popupmenucontrollerbase.hxx>
#include <unotools/lingucfg.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/image.hxx>
#include <vcl/menu.hxx>
#include <sal/log.hxx>

#include <com/sun/star/linguistic2/LinguServiceManager.hpp>

namespace {

class ThesaurusMenuController : public svt::PopupMenuControllerBase
{
public:
    explicit ThesaurusMenuController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

private:
    void fillPopupMenu();
    void getMeanings( std::vector< OUString >& rSynonyms, const OUString& rWord, const css::lang::Locale& rLocale, size_t nMaxSynonms );
    OUString getThesImplName( const css::lang::Locale& rLocale ) const;
    css::uno::Reference< css::linguistic2::XLinguServiceManager2 > m_xLinguServiceManager;
    css::uno::Reference< css::linguistic2::XThesaurus > m_xThesaurus;
    OUString m_aLastWord;
};

}

ThesaurusMenuController::ThesaurusMenuController( const css::uno::Reference< css::uno::XComponentContext >& rxContext ) :
    svt::PopupMenuControllerBase( rxContext ),
    m_xLinguServiceManager( css::linguistic2::LinguServiceManager::create( rxContext ) ),
    m_xThesaurus( m_xLinguServiceManager->getThesaurus() )
{
}

void ThesaurusMenuController::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    rEvent.State >>= m_aLastWord;
    m_xPopupMenu->clear();
    if ( rEvent.IsEnabled )
        fillPopupMenu();
}

void ThesaurusMenuController::fillPopupMenu()
{
    sal_Int32 nIdx{ 0 };
    OUString aText = m_aLastWord.getToken(0, '#', nIdx);
    OUString aIsoLang = m_aLastWord.getToken(0, '#', nIdx);
    if ( aText.isEmpty() || aIsoLang.isEmpty() )
        return;

    std::vector< OUString > aSynonyms;
    css::lang::Locale aLocale = LanguageTag::convertToLocale( aIsoLang );
    getMeanings( aSynonyms, aText, aLocale, 7 /*max number of synonyms to retrieve*/ );

    m_xPopupMenu->enableAutoMnemonics(false);
    if ( aSynonyms.empty() )
        return;

    SvtLinguConfig aCfg;
    Image aImage;
    OUString aThesImplName( getThesImplName( aLocale ) );
    OUString aSynonymsImageUrl( aCfg.GetSynonymsContextImage( aThesImplName ) );
    if ( !aThesImplName.isEmpty() && !aSynonymsImageUrl.isEmpty() )
        aImage = Image( aSynonymsImageUrl );

    VCLXMenu* pAwtMenu = comphelper::getFromUnoTunnel<VCLXMenu>( m_xPopupMenu );
    Menu* pVCLMenu = pAwtMenu->GetMenu();

    sal_uInt16 nId = 1;
    for ( const auto& aSynonym : aSynonyms )
    {
        OUString aItemText( linguistic::GetThesaurusReplaceText( aSynonym ) );
        pVCLMenu->InsertItem( nId, aItemText );
        pVCLMenu->SetItemCommand( nId, ".uno:ThesaurusFromContext?WordReplace:string=" + aItemText );

        if ( !aSynonymsImageUrl.isEmpty() )
            pVCLMenu->SetItemImage( nId, aImage );
        nId++;
    }

    pVCLMenu->InsertSeparator();
    OUString aThesaurusDialogCmd( ".uno:ThesaurusDialog" );
    auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(aThesaurusDialogCmd, m_aModuleName);
    pVCLMenu->InsertItem( nId, vcl::CommandInfoProvider::GetPopupLabelForCommand(aProperties) );
    pVCLMenu->SetItemCommand( nId, aThesaurusDialogCmd );
}

void ThesaurusMenuController::getMeanings( std::vector< OUString >& rSynonyms, const OUString& rWord,
                                           const css::lang::Locale& rLocale, size_t nMaxSynonms )
{
    rSynonyms.clear();
    if ( !(m_xThesaurus.is() && m_xThesaurus->hasLocale( rLocale ) && !rWord.isEmpty() && nMaxSynonms > 0) )
        return;

    try
    {
        const css::uno::Sequence< css::uno::Reference< css::linguistic2::XMeaning > > aMeaningSeq(
            m_xThesaurus->queryMeanings( rWord, rLocale, css::uno::Sequence< css::beans::PropertyValue >() ) );

        for ( const auto& xMeaning : aMeaningSeq )
        {
            const css::uno::Sequence< OUString > aSynonymSeq( xMeaning->querySynonyms() );
            for ( const auto& aSynonym : aSynonymSeq )
            {
                rSynonyms.push_back( aSynonym );
                if ( rSynonyms.size() == nMaxSynonms )
                    return;
            }
        }
    }
    catch ( const css::uno::Exception& )
    {
        SAL_WARN( "fwk.uielement", "Failed to get synonyms" );
    }
}

OUString ThesaurusMenuController::getThesImplName( const css::lang::Locale& rLocale ) const
{
    css::uno::Sequence< OUString > aServiceNames =
        m_xLinguServiceManager->getConfiguredServices( "com.sun.star.linguistic2.Thesaurus", rLocale );
    SAL_WARN_IF( aServiceNames.getLength() > 1, "fwk.uielement", "Only one thesaurus is allowed per locale, but found more!" );
    if ( aServiceNames.getLength() == 1 )
        return aServiceNames[0];

    return OUString();
}

OUString ThesaurusMenuController::getImplementationName()
{
    return "com.sun.star.comp.framework.ThesaurusMenuController";
}

css::uno::Sequence< OUString > ThesaurusMenuController::getSupportedServiceNames()
{
    return { "com.sun.star.frame.PopupMenuController" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_ThesaurusMenuController_get_implementation(
    css::uno::XComponentContext* xContext,
    css::uno::Sequence< css::uno::Any > const & )
{
    return cppu::acquire( new ThesaurusMenuController( xContext ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
