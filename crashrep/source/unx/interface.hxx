/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: interface.hxx,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include <X11/Xlib.h>

#include <hash_map>
#include <vector>
#include <string>

class WizardDialog;

// returns success, on true application will quit
bool save_crash_report( const std::string& rFileName, const std::hash_map< std::string, std::string >& rSettings );
// returns success, on true application will quit
bool send_crash_report( WizardDialog *pDialog, const std::hash_map< std::string, std::string >& rSettings );
// must return a valid UTF8 string containing the message that will be sent
std::string crash_get_details( const std::hash_map< std::string, std::string >& rSettings );

class StringResource
{
public:
    static void init( int argc, char** argv );
    static const char* get( const char* pKey );
};

class WizardPage
{
protected:
    std::string        m_aWizardTitle;
    GtkWidget*         m_pPageContents;
    WizardDialog*      m_pDialog;
public:
    WizardPage( WizardDialog* pDialog ) : m_pDialog( pDialog ) {}
    virtual ~WizardPage();

    const char*       getTitle() const { return m_aWizardTitle.c_str(); }
    GtkWidget*         getContents() const { return m_pPageContents; }

    virtual void update() = 0;
};

class WizardDialog
{
    std::vector< WizardPage* >      m_aPages;
    int                            m_nCurrentPage;

    GtkWidget*                      m_pTopLevel;
    GtkWidget*                      m_pVBox;
    GtkWidget*                      m_pViewPort;
    GtkWidget*                      m_pWizardTitle;
    GtkWidget*                      m_pPageArea;
    GtkWidget*                      m_pSeparator;
    GtkWidget*                      m_pButtonBox;
    GtkWidget*                      m_pBackButton;
    GtkWidget*                      m_pNextButton;
    GtkWidget*                      m_pSendButton;
    GtkWidget*                      m_pCancelButton;

    GtkWidget*                      m_pStatusDialog;

    std::hash_map< std::string, std::string > m_aSettings;

    static gint button_clicked( GtkWidget* pButton, WizardDialog* pThis );

    void nextPage();
    void lastPage();
public:
    WizardDialog();
    ~WizardDialog();

    // takes ownership of page
    void insertPage( WizardPage* pPage );

    void show( bool bShow = true );
    void show_messagebox( const std::string& rMessage );
    gint show_sendingstatus( bool bInProgress );
    void hide_sendingstatus();

    std::hash_map< std::string, std::string >& getSettings() { return m_aSettings; }
    GtkWindow* getTopLevel() const { return GTK_WINDOW(m_pTopLevel); }
};

class MainPage : public WizardPage
{
    GtkWidget*        m_pInfo;
    GtkWidget*        m_pEditLabel;
    GtkWidget*        m_pEdit;
    GtkWidget*        m_pEntryLabel;
    GtkWidget*        m_pEntryVBox;
    GtkWidget*        m_pScrolledEntry;
    GtkWidget*        m_pEntry;
    GtkWidget*        m_pHBox;
    GtkWidget*        m_pRightColumn;
    GtkWidget*        m_pLeftColumn;
    GtkWidget*        m_pDetails;
    GtkWidget*        m_pSave;
    GtkWidget*        m_pCheck;
    GtkWidget*        m_pOptions;
    GtkWidget*        m_pAddressLabel;
    GtkWidget*        m_pAddress;

    static gint button_clicked( GtkWidget* pButton, MainPage* pThis );
    static gint button_toggled( GtkWidget* pButton, MainPage* pThis );

public:
    MainPage( WizardDialog* );
    virtual ~MainPage();

    virtual void update();
};

class WelcomePage : public WizardPage
{
public:
    WelcomePage( WizardDialog* );
    virtual ~WelcomePage();

    virtual void update();
};

class OptionsDialog
{
        GtkWidget* m_pDialog;
        GtkWidget* m_pPage;
        GtkWidget* m_pLeftColumn;
        GtkWidget* m_pFrame;
        GtkWidget* m_pDirect;
        GtkWidget* m_pManual;
        GtkWidget* m_pServerLabel;
        GtkWidget* m_pServer;
        GtkWidget* m_pColon;
        GtkWidget* m_pPortLabel;
        GtkWidget* m_pPort;
        GtkWidget* m_pNote;
        GtkWidget* m_pOkButton;
        GtkWidget* m_pCancelButton;
        GtkWidget* m_pButtonBox;
        GtkWidget* m_pVBox;
        GtkWidget* m_pHBox;
        GtkWidget* m_pVBoxServer;
        GtkWidget* m_pVBoxPort;

        static gint button_toggled( GtkWidget* pButton, OptionsDialog* pThis );

public:
        OptionsDialog( GtkWindow* pParent,
                       std::hash_map< std::string, std::string >& rSettings );
        virtual ~OptionsDialog();

        std::string getUseProxy();
        std::string getServer();
        std::string getPort();

};
