/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CUI_SOURCE_OPTIONS_PERSONALIZATION_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_PERSONALIZATION_HXX

#include <sfx2/tabdlg.hxx>
#include <salhelper/thread.hxx>
#include <rtl/ref.hxx>
#include <vcl/prgsbar.hxx>

#include <vector>

class FixedText;
class SearchAndParseThread;

class SvxPersonalizationTabPage : public SfxTabPage
{
    using SfxTabPage::DeactivatePage;

private:
    VclPtr<RadioButton> m_pNoPersona;                  ///< Just the default look, without any bitmap
    VclPtr<RadioButton> m_pDefaultPersona;             ///< Use the built-in bitmap
    VclPtr<RadioButton> m_pOwnPersona;                 ///< Use the user-defined bitmap
    VclPtr<PushButton> m_pSelectPersona;               ///< Let the user select in the 'own' case
    VclPtr<PushButton> m_vDefaultPersonaImages[3];     ///< Buttons to show the default persona images
    VclPtr<PushButton> m_pExtensionPersonaPreview;     ///< Buttons to show the last 3 personas installed via extensions
    VclPtr<ListBox> m_pPersonaList;                    ///< The ListBox to show the list of installed personas
    OUString m_aPersonaSettings;                ///< Header and footer images + color to be set in the settings.
    VclPtr<FixedText> m_pExtensionLabel;               ///< The "select persona installed via extensions" label

    std::vector<OUString> m_vDefaultPersonaSettings;
    std::vector<OUString> m_vExtensionPersonaSettings;

public:
    ::rtl::Reference< SearchAndParseThread > m_rApplyThread;
    SvxPersonalizationTabPage( vcl::Window *pParent, const SfxItemSet &rSet );
    virtual ~SvxPersonalizationTabPage();
    virtual void dispose() SAL_OVERRIDE;

    static VclPtr<SfxTabPage> Create( vcl::Window *pParent, const SfxItemSet *rSet );

    /// Apply the settings ([OK] button).
    virtual bool FillItemSet( SfxItemSet *rSet ) SAL_OVERRIDE;

    /// Reset to default settings ([Revert] button).
    virtual void Reset( const SfxItemSet *rSet ) SAL_OVERRIDE;

    void SetPersonaSettings( const OUString& );

    void LoadDefaultImages();
    void LoadExtensionThemes();

private:
    /// Handle the Persona selection
    DECL_LINK_TYPED( SelectPersona, Button*, void );

    /// When 'own' is chosen, but the Persona is not chosen yet.
    DECL_LINK_TYPED( ForceSelect, Button*, void );

    /// Handle the default Persona selection
    DECL_LINK_TYPED( DefaultPersona, Button*, void );

    /// Handle the Personas installed through extensions selection
    DECL_LINK_TYPED( SelectInstalledPersona, ListBox&, void );
};

/** Dialog that will allow the user to choose a Persona to use.

So far there is no better possibility than just to paste the URL from
https://addons.mozilla.org/firefox/themes ...
*/
class SelectPersonaDialog : public ModalDialog
{
private:
    VclPtr<Edit> m_pEdit;                          ///< The input line for the search term
    VclPtr<PushButton> m_pSearchButton;            ///< The search button
    VclPtr<FixedText> m_pProgressLabel;            ///< The label for showing progress of search
    VclPtr<PushButton> m_vResultList[9];           ///< List of buttons to show search results
    VclPtr<PushButton> m_vSearchSuggestions[5];    ///< List of buttons for the search suggestions
    VclPtr<PushButton> m_pOkButton;                ///< The OK button
    VclPtr<PushButton> m_pCancelButton;            ///< The Cancel button

    std::vector<OUString> m_vPersonaSettings;
    OUString m_aSelectedPersona;
    OUString m_aAppliedPersona;

public:
    explicit SelectPersonaDialog( vcl::Window *pParent );
    virtual ~SelectPersonaDialog();
    virtual void dispose() SAL_OVERRIDE;
    ::rtl::Reference< SearchAndParseThread > m_rSearchThread;

    OUString GetSelectedPersona() const;
    void SetProgress( OUString& );
    void SetImages( Image, sal_Int32 );
    void AddPersonaSetting( OUString& );
    void ClearSearchResults();
    void SetAppliedPersonaSetting( OUString& );
    OUString GetAppliedPersonaSetting() const;

private:
    /// Handle the Search button
    DECL_LINK_TYPED( SearchPersonas, Button*, void );
    DECL_LINK_TYPED( SelectPersona, Button*, void );
    DECL_LINK_TYPED( ActionOK, Button*, void );
    DECL_LINK_TYPED( ActionCancel, Button*, void );
};

class SearchAndParseThread: public salhelper::Thread
{
private:

    VclPtr<SelectPersonaDialog> m_pPersonaDialog;
    OUString m_aURL;
    bool m_bExecute;

    virtual ~SearchAndParseThread();
    virtual void execute() SAL_OVERRIDE;
    void getPreviewFile( const OUString&, OUString *, OUString * );

public:

    SearchAndParseThread( SelectPersonaDialog* pDialog,
                          const OUString& rURL );

    void StopExecution() { m_bExecute = false; }
};

#endif // INCLUDED_CUI_SOURCE_OPTIONS_PERSONALIZATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
