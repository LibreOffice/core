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

#include <comphelper/processfactory.hxx>
#include <svl/lngmisc.hxx>
#include <svtools/popupmenucontrollerbase.hxx>
#include <unotools/lingucfg.hxx>
#include <vcl/image.hxx>
#include <vcl/menu.hxx>

#include <com/sun/star/frame/theUICommandDescription.hpp>
#include <com/sun/star/linguistic2/LinguServiceManager.hpp>

class ThesaurusMenuController : public svt::PopupMenuControllerBase
{
public:
    explicit ThesaurusMenuController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~ThesaurusMenuController();

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) throw ( css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw ( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw ( css::uno::RuntimeException, std::exception ) override;

private:
    virtual void impl_setPopupMenu() override;
    void getMeanings( std::vector< OUString >& rSynonyms, const OUString& rWord, const css::lang::Locale& rLocale, size_t nMaxSynonms );
    OUString getThesImplName( const css::lang::Locale& rLocale ) const;
    css::uno::Reference< css::linguistic2::XLinguServiceManager2 > m_xLinguServiceManager;
    css::uno::Reference< css::linguistic2::XThesaurus > m_xThesaurus;
    OUString m_aLastWord;
};

namespace {

OUString RetrieveLabelFromCommand( const OUString& rCmdURL, const OUString& rModuleName )
{
    if ( rCmdURL.isEmpty() || rModuleName.isEmpty() )
        return OUString();

    css::uno::Any a;
    css::uno::Sequence< css::beans::PropertyValue > aPropSeq;
    try
    {
        css::uno::Reference< css::container::XNameAccess > const xNameAccess(
            css::frame::theUICommandDescription::get( comphelper::getProcessComponentContext() ), css::uno::UNO_QUERY_THROW );
        a = xNameAccess->getByName( rModuleName );
        css::uno::Reference< css::container::XNameAccess > xUICommandLabels;
        a >>= xUICommandLabels;
        a = xUICommandLabels->getByName( rCmdURL );
        a >>= aPropSeq;
    }
    catch ( const css::uno::Exception& )
    {
        SAL_WARN( "fwk.uielement", "Failed to get label for command " << rCmdURL );
    }

    OUString aLabel;
    for ( const auto& aProp : aPropSeq )
    {
        if ( aProp.Name == "Label" )
        {
            aProp.Value >>= aLabel;
        }
        else if ( aProp.Name == "PopupLabel" )
        {
            OUString aStr;
            if ( ( aProp.Value >>= aStr ) && !aStr.isEmpty() )
                return aStr;
        }
    }
    return aLabel;
}

}

ThesaurusMenuController::ThesaurusMenuController( const css::uno::Reference< css::uno::XComponentContext >& rxContext ) :
    svt::PopupMenuControllerBase( rxContext ),
    m_xLinguServiceManager( css::linguistic2::LinguServiceManager::create( rxContext ) ),
    m_xThesaurus( m_xLinguServiceManager->getThesaurus() )
{
}

ThesaurusMenuController::~ThesaurusMenuController()
{
}

void ThesaurusMenuController::statusChanged( const css::frame::FeatureStateEvent& rEvent )
    throw ( css::uno::RuntimeException, std::exception )
{
    rEvent.State >>= m_aLastWord;
    m_xPopupMenu->clear();
    if ( rEvent.IsEnabled )
        impl_setPopupMenu();
}

void ThesaurusMenuController::impl_setPopupMenu()
{
    OUString aText = m_aLastWord.getToken(0, '#');
    OUString aIsoLang = m_aLastWord.getToken(1, '#');
    if ( aText.isEmpty() || aIsoLang.isEmpty() )
        return;

    std::vector< OUString > aSynonyms;
    css::lang::Locale aLocale = LanguageTag::convertToLocale( aIsoLang );
    getMeanings( aSynonyms, aText, aLocale, 7 /*max number of synonyms to retrieve*/ );

    VCLXMenu* pAwtMenu = VCLXMenu::GetImplementation( m_xPopupMenu );
    Menu* pVCLMenu = pAwtMenu->GetMenu();
    pVCLMenu->SetMenuFlags( MenuFlags::NoAutoMnemonics );
    if ( aSynonyms.size() > 0 )
    {
        SvtLinguConfig aCfg;
        Image aImage;
        OUString aThesImplName( getThesImplName( aLocale ) );
        OUString aSynonymsImageUrl( aCfg.GetSynonymsContextImage( aThesImplName ) );
        if ( !aThesImplName.isEmpty() && !aSynonymsImageUrl.isEmpty() )
            aImage = Image( aSynonymsImageUrl );

        for ( const auto& aSynonym : aSynonyms )
        {
            const sal_uInt16 nId = pVCLMenu->GetItemCount() + 1;
            OUString aItemText( linguistic::GetThesaurusReplaceText( aSynonym ) );
            pVCLMenu->InsertItem( nId, aItemText );
            pVCLMenu->SetItemCommand( nId, ".uno:ThesaurusFromContext?WordReplace:string=" + aItemText );

            if ( !aSynonymsImageUrl.isEmpty() )
                pVCLMenu->SetItemImage( nId, aImage );
        }

        pVCLMenu->InsertSeparator();
        OUString aThesaurusDialogCmd( ".uno:ThesaurusDialog" );
        pVCLMenu->InsertItem( 100, RetrieveLabelFromCommand( aThesaurusDialogCmd, m_aModuleName ) );
        pVCLMenu->SetItemCommand( 100, aThesaurusDialogCmd );
    }
}

void ThesaurusMenuController::getMeanings( std::vector< OUString >& rSynonyms, const OUString& rWord,
                                           const css::lang::Locale& rLocale, size_t nMaxSynonms )
{
    rSynonyms.clear();
    if ( m_xThesaurus.is() && m_xThesaurus->hasLocale( rLocale ) && !rWord.isEmpty() && nMaxSynonms > 0 )
    {
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
    throw ( css::uno::RuntimeException, std::exception )
{
    return OUString( "com.sun.star.comp.framework.ThesaurusMenuController" );
}

css::uno::Sequence< OUString > ThesaurusMenuController::getSupportedServiceNames()
    throw ( css::uno::RuntimeException, std::exception )
{
    css::uno::Sequence< OUString > aRet( 1 );
    aRet[0] = "com.sun.star.frame.PopupMenuController";
    return aRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_ThesaurusMenuController_get_implementation(
    css::uno::XComponentContext* xContext,
    css::uno::Sequence< css::uno::Any > const & )
{
    return cppu::acquire( new ThesaurusMenuController( xContext ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
