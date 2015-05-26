/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svtools/RemoteFilesDialog.hxx>

using namespace ::com::sun::star::uno;

RemoteFilesDialog::RemoteFilesDialog(vcl::Window* pParent, WinBits nBits)
    : ModalDialog(pParent, "RemoteFilesDialog", "svt/ui/remotefilesdialog.ui")
    , m_context(comphelper::getProcessComponentContext())
{
    get(m_pOpen_btn, "open");
    get(m_pSave_btn, "save");
    get(m_pCancel_btn, "cancel");
    get(m_pAddService_btn, "add_service_btn");
    get(m_pServices_lb, "services_lb");

    m_eMode = (nBits & WB_SAVEAS) ? REMOTEDLG_MODE_SAVE : REMOTEDLG_MODE_OPEN;
    m_bIsUpdated = false;

    if(m_eMode == REMOTEDLG_MODE_OPEN)
    {
        m_pSave_btn->Hide();
        m_pOpen_btn->Show();
    }
    else
    {
        m_pSave_btn->Show();
        m_pOpen_btn->Hide();
    }

    m_pAddService_btn->SetMenuMode(MENUBUTTON_MENUMODE_TIMED);
    m_pAddService_btn->SetClickHdl( LINK( this, RemoteFilesDialog, AddServiceHdl ) );
    m_pAddService_btn->SetSelectHdl( LINK( this, RemoteFilesDialog, EditServiceMenuHdl ) );

    FillServicesListbox();
}

void RemoteFilesDialog::dispose()
{
    if(m_bIsUpdated)
    {
        Sequence< OUString > placesUrlsList(m_aServices.size());
        Sequence< OUString > placesNamesList(m_aServices.size());

        int i = 0;
        for(std::vector<ServicePtr>::const_iterator it = m_aServices.begin(); it != m_aServices.end(); ++it)
        {
            placesUrlsList[i] = (*it)->GetUrl();
            placesNamesList[i] = (*it)->GetName();
            ++i;
        }

        std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create(m_context));
        officecfg::Office::Common::Misc::FilePickerPlacesUrls::set(placesUrlsList, batch);
        officecfg::Office::Common::Misc::FilePickerPlacesNames::set(placesNamesList, batch);
        batch->commit();
    }

    ModalDialog::dispose();
}

void RemoteFilesDialog::FillServicesListbox()
{
    m_pServices_lb->Clear();
    m_aServices.clear();

    // Load from user settings
    Sequence< OUString > placesUrlsList(officecfg::Office::Common::Misc::FilePickerPlacesUrls::get(m_context));
    Sequence< OUString > placesNamesList(officecfg::Office::Common::Misc::FilePickerPlacesNames::get(m_context));

    for(sal_Int32 nPlace = 0; nPlace < placesUrlsList.getLength() && nPlace < placesNamesList.getLength(); ++nPlace)
    {
        ServicePtr pService(new Place(placesNamesList[nPlace], placesUrlsList[nPlace], true));
        m_aServices.push_back(pService);

        // Add to the listbox only remote services, not local bookmarks
        if(!pService->IsLocal())
        {
            m_pServices_lb->InsertEntry(placesNamesList[nPlace]);
        }
    }

    if(m_pServices_lb->GetEntryCount() > 0)
        m_pServices_lb->SelectEntryPos(0);
    else
        m_pServices_lb->Enable(false);
}

unsigned int RemoteFilesDialog::GetSelectedServicePos()
{
    int nSelected = m_pServices_lb->GetSelectEntryPos();
    unsigned int nPos = 0;
    int i = -1;

    while(nPos < m_aServices.size())
    {
        while(m_aServices[nPos]->IsLocal())
            nPos++;
        i++;
        if(i == nSelected)
            break;
        nPos++;
    }

    return nPos;
}

IMPL_LINK_NOARG ( RemoteFilesDialog, AddServiceHdl )
{
    ScopedVclPtrInstance< PlaceEditDialog > aDlg(this);
    short aRetCode = aDlg->Execute();

    switch(aRetCode)
    {
        case RET_OK :
        {
            ServicePtr newService = aDlg->GetPlace();
            m_aServices.push_back(newService);
            m_pServices_lb->Enable(true);
            m_pServices_lb->InsertEntry(newService->GetName());
            m_pServices_lb->SelectEntryPos(m_pServices_lb->GetEntryCount() - 1);

            m_bIsUpdated = true;
      break;
        }
        case RET_CANCEL :
        default :
            // Do Nothing
            break;
    };

    return 1;
}

IMPL_LINK_TYPED ( RemoteFilesDialog, EditServiceMenuHdl, MenuButton *, pButton, void )
{
    OString sIdent(pButton->GetCurItemIdent());
    if(sIdent == "edit_service"  && m_pServices_lb->GetEntryCount() > 0)
    {
        unsigned int nSelected = m_pServices_lb->GetSelectEntryPos();
        unsigned int nPos = GetSelectedServicePos();

        ScopedVclPtrInstance< PlaceEditDialog > aDlg(this, m_aServices[nPos]);
        short aRetCode = aDlg->Execute();

        switch(aRetCode)
        {
            case RET_OK :
            {
                ServicePtr pEditedService = aDlg->GetPlace();

                m_aServices[nPos] = pEditedService;
                m_pServices_lb->RemoveEntry(nSelected);
                m_pServices_lb->InsertEntry(pEditedService->GetName(), nSelected);
                m_pServices_lb->SelectEntryPos(nSelected);

                m_bIsUpdated = true;
        break;
            }
            case RET_CANCEL :
            default :
                // Do Nothing
                break;
        };
    }
    else if(sIdent == "delete_service"  && m_pServices_lb->GetEntryCount() > 0)
    {
        unsigned int nSelected = m_pServices_lb->GetSelectEntryPos();
        unsigned int nPos = GetSelectedServicePos();

        // TODO: Confirm dialog

        m_aServices.erase(m_aServices.begin() + nPos);
        m_pServices_lb->RemoveEntry(nSelected);

        if(m_pServices_lb->GetEntryCount() > 0)
        {
            m_pServices_lb->SelectEntryPos(0);
        }
        else
        {
            m_pServices_lb->SetNoSelection();
            m_pServices_lb->Enable(false);
        }

        m_bIsUpdated = true;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
