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

    m_pAddService_btn->SetClickHdl( LINK( this, RemoteFilesDialog, AddServiceHdl) );

    fillServicesListbox();
}

void RemoteFilesDialog::fillServicesListbox()
{
    m_pServices_lb->Clear();

    // Load from user settings
    Sequence< OUString > placesUrlsList(officecfg::Office::Common::Misc::FilePickerPlacesUrls::get(m_context));
    Sequence< OUString > placesNamesList(officecfg::Office::Common::Misc::FilePickerPlacesNames::get(m_context));

    // TODO: filter - only online services
    if(placesUrlsList.getLength() > 0 && placesNamesList.getLength() > 0)
    {
        for(sal_Int32 nPlace = 0; nPlace < placesUrlsList.getLength() && nPlace < placesNamesList.getLength(); ++nPlace)
        {
            ServicePtr pService(new Place(placesNamesList[nPlace], placesUrlsList[nPlace], true));
            m_aServices.push_back(pService);

            m_pServices_lb->InsertEntry(placesNamesList[nPlace]);
        }

        m_pServices_lb->SelectEntryPos(0);
    }
    else
    {
        m_pServices_lb->Enable(false);
    }
}

IMPL_LINK_NOARG ( RemoteFilesDialog, AddServiceHdl )
{
    ScopedVclPtrInstance< PlaceEditDialog > aDlg(this);
    short aRetCode = aDlg->Execute();

    switch (aRetCode) {
        case RET_OK :
        {
            ServicePtr newService = aDlg->GetPlace();
            m_aServices.push_back(newService);
            m_pServices_lb->Enable(true);
            m_pServices_lb->InsertEntry(newService->GetName());
            m_pServices_lb->SelectEntryPos(m_pServices_lb->GetEntryCount() - 1);

            // save services

            Sequence< OUString > placesUrlsList(m_aServices.size());
            Sequence< OUString > placesNamesList(m_aServices.size());

            int i = 0;
            for(std::vector<ServicePtr>::const_iterator it = m_aServices.begin(); it != m_aServices.end(); ++it)
            {
                placesUrlsList[i] = (*it)->GetUrl();
                placesNamesList[i] = (*it)->GetName();
                i++;
            }

            std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create(m_context));
            officecfg::Office::Common::Misc::FilePickerPlacesUrls::set(placesUrlsList, batch);
            officecfg::Office::Common::Misc::FilePickerPlacesNames::set(placesNamesList, batch);
            batch->commit();
      break;
        }
        case RET_CANCEL :
        default :
            // Do Nothing
            break;
    };

    return 1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
