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
    RadioButton *m_pNoPersona;                  ///< Just the default look, without any bitmap
    RadioButton *m_pDefaultPersona;             ///< Use the built-in bitmap
    RadioButton *m_pOwnPersona;                 ///< Use the user-defined bitmap
    PushButton *m_pSelectPersona;               ///< Let the user select in the 'own' case
    PushButton *m_vDefaultPersonaImages[3];     ///< Buttons to show the default persona images
    PushButton *m_pExtensionPersonaPreview;     ///< Buttons to show the last 3 personas installed via extensions
    ListBox* m_pPersonaList;                    ///< The ListBox to show the list of installed personas
    OUString m_aPersonaSettings;                ///< Header and footer images + color to be set in the settings.
    FixedText *m_pExtensionLabel;               ///< The "select persona installed via extensions" label

    std::vector<OUString> m_vDefaultPersonaSettings;
    std::vector<OUString> m_vExtensionPersonaSettings;

public:
    ::rtl::Reference< SearchAndParseThread > m_rApplyThread;
    SvxPersonalizationTabPage( Window *pParent, const SfxItemSet &rSet );
    virtual ~SvxPersonalizationTabPage();

    static SfxTabPage* Create( Window *pParent, const SfxItemSet *rSet );

    /// Apply the settings ([OK] button).
    virtual bool FillItemSet( SfxItemSet *rSet ) SAL_OVERRIDE;

    /// Reset to default settings ([Revert] button).
    virtual void Reset( const SfxItemSet *rSet ) SAL_OVERRIDE;

    void SetPersonaSettings( const OUString );

    void LoadDefaultImages();
    void LoadExtensionThemes();

private:
    /// Handle the Persona selection
    DECL_LINK( SelectPersona, PushButton* );

    /// When 'own' is chosen, but the Persona is not chosen yet.
    DECL_LINK( ForceSelect, RadioButton* );

    /// Handle the default Persona selection
    DECL_LINK( DefaultPersona, PushButton* );

    /// Handle the Personas installed through extensions selection
    DECL_LINK( SelectInstalledPersona, ListBox* );
};

/** Dialog that will allow the user to choose a Persona to use.

So far there is no better possibility than just to paste the URL from
https://addons.mozilla.org/firefox/themes ...
*/
class SelectPersonaDialog : public ModalDialog
{
private:
    Edit *m_pEdit;                          ///< The input line for the search term
    PushButton *m_pSearchButton;            ///< The search button
    FixedText *m_pProgressLabel;            ///< The label for showing progress of search
    PushButton *m_vResultList[9];           ///< List of buttons to show search results
    PushButton *m_vSearchSuggestions[5];    ///< List of buttons for the search suggestions
    PushButton *m_pOkButton;                ///< The OK button
    PushButton *m_pCancelButton;            ///< The Cancel button

    std::vector<OUString> m_vPersonaSettings;
    OUString m_aSelectedPersona;
    OUString m_aAppliedPersona;

public:
    SelectPersonaDialog( Window *pParent );
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
    DECL_LINK( SearchPersonas, PushButton* );
    DECL_LINK( SelectPersona, PushButton* );
    DECL_LINK( ActionOK, PushButton* );
    DECL_LINK( ActionCancel, PushButton* );
};

class SearchAndParseThread: public salhelper::Thread
{
private:

    SelectPersonaDialog *m_pPersonaDialog;
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
