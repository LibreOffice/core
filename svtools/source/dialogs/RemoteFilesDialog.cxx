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

#define FILTER_ALL "*.*"

class FileViewContainer : public vcl::Window
{
    private:
    VclPtr<SvtFileView> m_pFileView;

    public:
    FileViewContainer(vcl::Window *pParent)
        : Window(pParent)
        , m_pFileView(NULL)
    {
    }

    virtual ~FileViewContainer()
    {
        disposeOnce();
    }

    virtual void dispose() SAL_OVERRIDE
    {
        m_pFileView.clear();
        vcl::Window::dispose();
    }

    void init(SvtFileView* pFileView)
    {
        m_pFileView = pFileView;
    }

    virtual void Resize() SAL_OVERRIDE
    {
        Window::Resize();

        if(!m_pFileView)
            return;

        Size aSize = GetSizePixel();
        m_pFileView->SetSizePixel( aSize );
    }
};

RemoteFilesDialog::RemoteFilesDialog(vcl::Window* pParent, WinBits nBits)
    : ModalDialog(pParent, "RemoteFilesDialog", "svt/ui/remotefilesdialog.ui")
    , m_context(comphelper::getProcessComponentContext())
    , m_pFileView(NULL)
    , m_pContainer(NULL)
{
    get(m_pOpen_btn, "open");
    get(m_pSave_btn, "save");
    get(m_pCancel_btn, "cancel");
    get(m_pAddService_btn, "add_service_btn");
    get(m_pServices_lb, "services_lb");
    get(m_pPath_ed, "path");
    get(m_pFilter_lb, "filter_lb");
    get(m_pName_ed, "name_ed");

    m_eMode = (nBits & WB_SAVEAS) ? REMOTEDLG_MODE_SAVE : REMOTEDLG_MODE_OPEN;
    m_eType = (nBits & WB_PATH) ? REMOTEDLG_TYPE_PATHDLG : REMOTEDLG_TYPE_FILEDLG;
    m_bMultiselection = (nBits & WB_MULTISELECTION) ? true : false;
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

    m_pContainer = VclPtr<FileViewContainer>::Create( get<vcl::Window>("container") );

    m_pContainer->set_hexpand(true);
    m_pContainer->set_vexpand(true);

    m_pFileView = VclPtr<SvtFileView>::Create( m_pContainer, WB_BORDER,
                                       REMOTEDLG_TYPE_PATHDLG == m_eType,
                                       m_bMultiselection );

    m_pFileView->Show();
    m_pFileView->EnableAutoResize();
    m_pFileView->SetDoubleClickHdl( LINK( this, RemoteFilesDialog, DoubleClickHdl ) );
    m_pFileView->SetSelectHdl( LINK( this, RemoteFilesDialog, SelectHdl ) );

    m_pContainer->init(m_pFileView);
    m_pContainer->Show();

    m_pAddService_btn->SetMenuMode(MENUBUTTON_MENUMODE_TIMED);
    m_pAddService_btn->SetClickHdl( LINK( this, RemoteFilesDialog, AddServiceHdl ) );
    m_pAddService_btn->SetSelectHdl( LINK( this, RemoteFilesDialog, EditServiceMenuHdl ) );

    FillServicesListbox();

    m_pServices_lb->SetSelectHdl( LINK( this, RemoteFilesDialog, SelectServiceHdl ) );

    m_pFilter_lb->InsertEntry(FILTER_ALL);
    m_pFilter_lb->SelectEntryPos(0);
}

RemoteFilesDialog::~RemoteFilesDialog()
{
    disposeOnce();
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

void RemoteFilesDialog::Resize()
{
    ModalDialog::Resize();

    if(m_pFileView && m_pContainer)
    {
        Size aSize = m_pContainer->GetSizePixel();
        m_pFileView->SetSizePixel(aSize);
    }
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

int RemoteFilesDialog::GetSelectedServicePos()
{
    int nSelected = m_pServices_lb->GetSelectEntryPos();
    int nPos = 0;
    int i = -1;

    if(m_aServices.size() == 0)
        return -1;

    while(nPos < (int)m_aServices.size())
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

OUString RemoteFilesDialog::getCurrentFilter()
{
    OUString sFilter;

    sFilter = m_pFilter_lb->GetSelectEntry();

    return sFilter;
}

void RemoteFilesDialog::OpenURL( OUString sURL )
{
    if(m_pFileView)
    {
        OUStringList BlackList;
        FileViewResult eResult = eFailure;
        OUString sFilter = getCurrentFilter();

        m_pFileView->EndInplaceEditing( false );
        m_pPath_ed->SetText( sURL );
        eResult = m_pFileView->Initialize( sURL, sFilter, NULL, BlackList );
    }
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

IMPL_LINK_NOARG ( RemoteFilesDialog, SelectServiceHdl )
{
    int nPos = GetSelectedServicePos();

    if(nPos > 0)
    {
        OUString sURL = m_aServices[nPos]->GetUrl();

        OpenURL( sURL );
    }

    return 1;
}

IMPL_LINK_TYPED ( RemoteFilesDialog, EditServiceMenuHdl, MenuButton *, pButton, void )
{
    OString sIdent(pButton->GetCurItemIdent());
    if(sIdent == "edit_service"  && m_pServices_lb->GetEntryCount() > 0)
    {
        unsigned int nSelected = m_pServices_lb->GetSelectEntryPos();
        int nPos = GetSelectedServicePos();

        if(nPos > 0)
        {
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
    }
    else if(sIdent == "delete_service"  && m_pServices_lb->GetEntryCount() > 0)
    {
        unsigned int nSelected = m_pServices_lb->GetSelectEntryPos();
        int nPos = GetSelectedServicePos();

        if(nPos > 0)
        {
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
}

IMPL_LINK_NOARG ( RemoteFilesDialog, DoubleClickHdl )
{
    OUString sURL = m_pFileView->GetCurrentURL();

    OpenURL( sURL );

    return 1;
}

IMPL_LINK_NOARG ( RemoteFilesDialog, SelectHdl )
{
    SvTreeListEntry* pEntry = m_pFileView->FirstSelected();
    SvtContentEntry* pData = static_cast<SvtContentEntry*>(pEntry->GetUserData());

    INetURLObject aURL(pData->maURL);
    m_pName_ed->SetText(aURL.GetLastName());

    return 1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
