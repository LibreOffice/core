/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: interface.cxx,v $
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
#include <interface.hxx>
#include <iostream.h>

using namespace std;

/*
 *  WizardPage
 */
WizardPage::~WizardPage()
{
}

/*
 *  WizardDialog
 */

WizardDialog::WizardDialog()
{
    m_pStatusDialog = NULL;
    m_pTopLevel = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_title( GTK_WINDOW(m_pTopLevel), StringResource::get( "%WELCOME_CAPTION%" ) );

    m_pVBox = gtk_vbox_new( FALSE, 5 );
    gtk_widget_show( m_pVBox );
    gtk_container_add( GTK_CONTAINER(m_pTopLevel), m_pVBox );

    m_pViewPort = gtk_viewport_new( NULL, NULL );
    gtk_widget_show( m_pViewPort );

    // set wizard title style
    gtk_widget_ensure_style( m_pViewPort );
    GtkStyle* pStyle = gtk_style_copy( gtk_widget_get_style( m_pViewPort ) );
    for( int i = 0; i < 5; i++ )
    {
        pStyle->bg[i] = pStyle->white;
        pStyle->text[i] = pStyle->black;
        pStyle->bg_gc[i] = pStyle->white_gc;
        pStyle->text_gc[i] = pStyle->black_gc;
    }
    gint nFontSize = pango_font_description_get_size( pStyle->font_desc );
    nFontSize = nFontSize * 3 / 2;
    pango_font_description_set_size( pStyle->font_desc, nFontSize );
    gtk_widget_set_style( m_pViewPort, pStyle );
    gtk_viewport_set_shadow_type( GTK_VIEWPORT(m_pViewPort), GTK_SHADOW_NONE );
    gtk_box_pack_start( GTK_BOX(m_pVBox), m_pViewPort, FALSE, FALSE, 0 );

    m_pWizardTitle = gtk_label_new( "Wizard" ); // to be replaced by resp. page titles
    gtk_widget_show( m_pWizardTitle );
    gtk_label_set_justify( GTK_LABEL(m_pWizardTitle), GTK_JUSTIFY_LEFT );
    gtk_misc_set_alignment( GTK_MISC(m_pWizardTitle), 0, 1 );
    gtk_misc_set_padding( GTK_MISC(m_pWizardTitle), 10, 10 );
    gtk_widget_set_style( m_pWizardTitle, pStyle );
    gtk_container_add( GTK_CONTAINER(m_pViewPort), m_pWizardTitle );

    // prepare the area for the single pages
    m_pPageArea = gtk_vbox_new( FALSE, 1);
    gtk_widget_show( m_pPageArea );
    gtk_box_pack_start( GTK_BOX(m_pVBox), m_pPageArea, TRUE, TRUE, 0 );

    m_pSeparator = gtk_hseparator_new();
    gtk_widget_show( m_pSeparator );
    gtk_box_pack_start( GTK_BOX(m_pVBox), m_pSeparator, FALSE, FALSE, 0 );

    m_pButtonBox = gtk_hbutton_box_new();
    gtk_widget_show( m_pButtonBox );
    gtk_box_pack_start( GTK_BOX(m_pVBox), m_pButtonBox, FALSE, FALSE, 0 );
    gtk_button_box_set_layout( GTK_BUTTON_BOX(m_pButtonBox), GTK_BUTTONBOX_END );
    gtk_button_box_set_spacing( GTK_BUTTON_BOX(m_pButtonBox), 0 );

    m_pBackButton = gtk_button_new_with_mnemonic( StringResource::get( "%BACK_BUTTON%" ) );
    gtk_widget_show( m_pBackButton );
    gtk_container_add( GTK_CONTAINER(m_pButtonBox), m_pBackButton );
    gtk_container_set_border_width( GTK_CONTAINER(m_pBackButton), 5 );
    GTK_WIDGET_SET_FLAGS( m_pBackButton, GTK_CAN_DEFAULT );

    m_pNextButton = gtk_button_new_with_mnemonic( StringResource::get( "%NEXT_BUTTON%" ) );
    gtk_widget_show( m_pNextButton );
    gtk_container_add( GTK_CONTAINER(m_pButtonBox), m_pNextButton );
    gtk_container_set_border_width( GTK_CONTAINER(m_pNextButton), 5 );
    GTK_WIDGET_SET_FLAGS( m_pNextButton, GTK_CAN_DEFAULT );

    m_pSendButton = gtk_button_new_with_mnemonic( StringResource::get( "%SEND_BUTTON%" ) );
    gtk_widget_show( m_pSendButton );
    gtk_container_add( GTK_CONTAINER(m_pButtonBox), m_pSendButton );
    gtk_container_set_border_width( GTK_CONTAINER(m_pSendButton), 5 );
    GTK_WIDGET_SET_FLAGS( m_pSendButton, GTK_CAN_DEFAULT );

    m_pCancelButton = gtk_button_new_with_mnemonic( StringResource::get( "%DONOT_SEND_BUTTON%" ) );
    gtk_widget_show( m_pCancelButton );
    gtk_container_add( GTK_CONTAINER(m_pButtonBox), m_pCancelButton );
    gtk_container_set_border_width( GTK_CONTAINER(m_pCancelButton), 5 );
    GTK_WIDGET_SET_FLAGS( m_pCancelButton, GTK_CAN_DEFAULT );

    gtk_signal_connect( GTK_OBJECT(m_pTopLevel), "delete-event", G_CALLBACK(gtk_main_quit), NULL );
    gtk_signal_connect( GTK_OBJECT(m_pCancelButton), "clicked", G_CALLBACK(gtk_main_quit), NULL );
    gtk_signal_connect( GTK_OBJECT(m_pNextButton), "clicked", G_CALLBACK(button_clicked), this );
    gtk_signal_connect( GTK_OBJECT(m_pBackButton), "clicked", G_CALLBACK(button_clicked), this );
    gtk_signal_connect( GTK_OBJECT(m_pSendButton), "clicked", G_CALLBACK(button_clicked), this );
    gtk_widget_set_sensitive( m_pSendButton, FALSE );

    m_nCurrentPage = -1;
}

WizardDialog::~WizardDialog()
{
    int nPages = m_aPages.size();
    while( nPages-- )
        delete m_aPages[nPages];
}

void WizardDialog::show_messagebox( const std::string& rMessage )
{
    GtkWidget* messagebox = NULL;
    GtkMessageType eType = GTK_MESSAGE_ERROR;

    messagebox = gtk_message_dialog_new( NULL,
                                      (GtkDialogFlags)0,
                                      eType,
                                      GTK_BUTTONS_OK,
                                      rMessage.c_str(),
                                      NULL
                                      );

    gtk_dialog_run( GTK_DIALOG(messagebox) );
    gtk_widget_destroy( GTK_WIDGET(messagebox) );
}



// bInProgress: true=sending, false=finished
gint WizardDialog::show_sendingstatus( bool bInProgress )
{
    m_pStatusDialog = gtk_dialog_new_with_buttons( StringResource::get( "%SENDING_REPORT_HEADER%" ),
                                                      getTopLevel(),
                                                      GTK_DIALOG_MODAL,
                                                      bInProgress ? GTK_STOCK_CANCEL : GTK_STOCK_OK,
                                                      bInProgress ? GTK_RESPONSE_REJECT : GTK_RESPONSE_OK,
                                                      NULL );

    gtk_window_set_default_size( GTK_WINDOW(m_pStatusDialog), 350, 130 );

    GtkWidget *pLabel = gtk_label_new( bInProgress ? StringResource::get( "%SENDING_REPORT_STATUS%" ) :
                                              StringResource::get( "%SENDING_REPORT_STATUS_FINISHED%" ) );
    gtk_widget_show( pLabel );
    gtk_label_set_justify( GTK_LABEL(pLabel), GTK_JUSTIFY_CENTER);
    gtk_misc_set_alignment( GTK_MISC(pLabel), 0, 0 );

    gtk_container_add( GTK_CONTAINER(GTK_DIALOG(m_pStatusDialog)->vbox), pLabel );

    gint ret = gtk_dialog_run( GTK_DIALOG(m_pStatusDialog) );
    gtk_widget_destroy( m_pStatusDialog );
    m_pStatusDialog = NULL;

    return ret;
}

void WizardDialog::hide_sendingstatus()
{
    if( m_pStatusDialog )
    {
        gtk_dialog_response( GTK_DIALOG(m_pStatusDialog), GTK_RESPONSE_OK );

        XEvent  event;

        memset( &event, 0, sizeof(event) );

        event.xexpose.type = Expose;
        event.xexpose.display = GDK_DISPLAY();
        event.xexpose.window = GDK_WINDOW_XWINDOW( m_pStatusDialog->window );
        event.xexpose.width = event.xexpose.height = 10;

        XSendEvent(
            GDK_DISPLAY(),
            GDK_WINDOW_XWINDOW( m_pStatusDialog->window ),
            True,
            ExposureMask,
            &event );

        XFlush( GDK_DISPLAY() );
    }
}


gint WizardDialog::button_clicked( GtkWidget* pButton, WizardDialog* pThis )
{
    if( pButton == pThis->m_pNextButton )
        pThis->nextPage();
    else if( pButton == pThis->m_pBackButton )
        pThis->lastPage();
    else if( pButton == pThis->m_pSendButton )
    {
        if( pThis->m_nCurrentPage != -1 )
            pThis->m_aPages[pThis->m_nCurrentPage]->update();

        if( send_crash_report( pThis, pThis->getSettings() ) )
            gtk_main_quit();
    }

    return 0;
}

void WizardDialog::insertPage( WizardPage* pPage )
{
    m_aPages.push_back( pPage );
    if( m_nCurrentPage == -1 )
    {
        m_nCurrentPage = 0;
        gtk_label_set_text( GTK_LABEL(m_pWizardTitle), pPage->getTitle() );
        gtk_box_pack_start( GTK_BOX(m_pPageArea), pPage->getContents(), TRUE, TRUE, 0 );
        gtk_widget_set_sensitive( m_pNextButton, FALSE );
        gtk_widget_set_sensitive( m_pBackButton, FALSE );
    }
    else
    {
        gtk_widget_set_sensitive( m_pNextButton, TRUE );
    }
}

void WizardDialog::nextPage()
{
    if( m_aPages.empty() || m_nCurrentPage >= (int)m_aPages.size()-1 )
        return;

    m_aPages[m_nCurrentPage]->update();

    gtk_container_remove( GTK_CONTAINER(m_pPageArea), m_aPages[m_nCurrentPage]->getContents() );
    m_nCurrentPage++;
    gtk_label_set_text( GTK_LABEL(m_pWizardTitle), m_aPages[m_nCurrentPage]->getTitle() );
    gtk_box_pack_start( GTK_BOX(m_pPageArea), m_aPages[m_nCurrentPage]->getContents(), TRUE, TRUE, 0 );

    if( m_nCurrentPage == (int)m_aPages.size()-1 )
    {
        gtk_widget_set_sensitive( m_pNextButton, FALSE );
        gtk_widget_set_sensitive( m_pSendButton, TRUE );
    }
    if( m_aPages.size() > 1 )
        gtk_widget_set_sensitive( m_pBackButton, TRUE );
}

void WizardDialog::lastPage()
{
    if( m_aPages.empty() || m_nCurrentPage <= 0 )
        return;

    m_aPages[m_nCurrentPage]->update();

    gtk_container_remove( GTK_CONTAINER(m_pPageArea), m_aPages[m_nCurrentPage]->getContents() );
    m_nCurrentPage--;
    gtk_label_set_text( GTK_LABEL(m_pWizardTitle), m_aPages[m_nCurrentPage]->getTitle() );
    gtk_box_pack_start( GTK_BOX(m_pPageArea), m_aPages[m_nCurrentPage]->getContents(), TRUE, TRUE, 0 );

    if( m_nCurrentPage == 0 )
        gtk_widget_set_sensitive( m_pBackButton, FALSE );
    if( m_aPages.size() > 1 )
        gtk_widget_set_sensitive( m_pNextButton, TRUE );
}

void WizardDialog::show( bool bShow )
{
    if( bShow )
        gtk_widget_show( m_pTopLevel );
    else
        gtk_widget_hide( m_pTopLevel );
}

/*
 *  MainPage
 */

MainPage::MainPage( WizardDialog* pParent ) : WizardPage( pParent )
{
    hash_map< string, string >& rSettings = m_pDialog->getSettings();
    m_aWizardTitle = StringResource::get( "%REPORT_HEADER%" );

    m_pPageContents = gtk_vbox_new( FALSE, 0 );
    gtk_widget_show( m_pPageContents );

    m_pInfo = gtk_label_new( StringResource::get( "%REPORT_BODY%" ) );
    gtk_widget_show( m_pInfo );
    gtk_label_set_line_wrap( GTK_LABEL(m_pInfo), TRUE );
    gtk_label_set_justify( GTK_LABEL(m_pInfo), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment( GTK_MISC(m_pInfo), 0, 1 );
    gtk_misc_set_padding( GTK_MISC(m_pInfo ), 5, 5);
    gtk_box_pack_start( GTK_BOX(m_pPageContents), m_pInfo, FALSE, FALSE, 0 );

    m_pHBox = gtk_hbox_new( FALSE, 0 );
    gtk_widget_show( m_pHBox );
    gtk_box_pack_start( GTK_BOX(m_pPageContents), m_pHBox, TRUE, TRUE, 0 );

    m_pLeftColumn = gtk_vbox_new( FALSE, 5 );
    gtk_widget_show( m_pLeftColumn );
    gtk_container_set_border_width( GTK_CONTAINER(m_pLeftColumn), 5 );
    gtk_box_pack_start( GTK_BOX(m_pHBox), m_pLeftColumn, TRUE, TRUE, 0 );

    m_pRightColumn = gtk_vbutton_box_new();
    gtk_widget_show( m_pRightColumn );
    gtk_button_box_set_layout( GTK_BUTTON_BOX(m_pRightColumn), GTK_BUTTONBOX_END );
    gtk_box_pack_start( GTK_BOX(m_pHBox), m_pRightColumn, FALSE, FALSE, 0 );

    m_pEditLabel = gtk_label_new_with_mnemonic( StringResource::get( "%ENTER_TITLE%" ) );
    gtk_widget_show( m_pEditLabel );
    gtk_label_set_justify( GTK_LABEL(m_pEditLabel), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment( GTK_MISC(m_pEditLabel), 0, 1 );
    gtk_box_pack_start( GTK_BOX(m_pLeftColumn), m_pEditLabel, FALSE, FALSE, 0 );

    m_pEdit = gtk_entry_new();
    gtk_widget_show( m_pEdit );
    gtk_box_pack_start( GTK_BOX(m_pLeftColumn), m_pEdit, FALSE, FALSE, 0 );

    gtk_label_set_mnemonic_widget( GTK_LABEL(m_pEditLabel), m_pEdit );

    hash_map<string, string>::iterator aIter;
    aIter = rSettings.find( "TITLE" );
    if( aIter != rSettings.end() )
        gtk_entry_set_text( GTK_ENTRY(m_pEdit), aIter->second.c_str() );

    m_pEntryVBox = gtk_vbox_new( FALSE, 5 );
    gtk_widget_show( m_pEntryVBox );
    gtk_box_pack_start( GTK_BOX(m_pLeftColumn), m_pEntryVBox, TRUE, TRUE, 0 );

    m_pEntryLabel = gtk_label_new_with_mnemonic( StringResource::get( "%ENTER_DESCRIPTION%" ) );
    gtk_widget_show( m_pEntryLabel );
    gtk_label_set_justify( GTK_LABEL(m_pEntryLabel), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment( GTK_MISC(m_pEntryLabel), 0, 1 );
    gtk_box_pack_start( GTK_BOX(m_pEntryVBox), m_pEntryLabel, FALSE, FALSE, 0 );

    m_pScrolledEntry = gtk_scrolled_window_new( NULL, NULL );
    gtk_widget_show( m_pScrolledEntry );
    gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW(m_pScrolledEntry), GTK_SHADOW_IN );
    gtk_box_pack_start( GTK_BOX(m_pEntryVBox), m_pScrolledEntry, TRUE, TRUE, 0 );

    m_pEntry = gtk_text_view_new();
    gtk_widget_show( m_pEntry );
    gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW(m_pEntry), GTK_WRAP_WORD );
    gtk_container_add( GTK_CONTAINER(m_pScrolledEntry), m_pEntry );

    aIter = rSettings.find( "DESCRIPTION" );
    if( aIter != rSettings.end() )
    {
        GtkTextBuffer* pBuffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(m_pEntry) );
        gtk_text_buffer_set_text( pBuffer, aIter->second.c_str(), -1 );
    }


    gtk_label_set_mnemonic_widget( GTK_LABEL(m_pEntryLabel), m_pEntry );

    m_pDetails = gtk_button_new_with_mnemonic( StringResource::get( "%SHOW_REPORT_BUTTON%" ) );
    gtk_widget_show(m_pDetails);
    gtk_container_set_border_width( GTK_CONTAINER(m_pDetails), 5 );
    gtk_container_add( GTK_CONTAINER(m_pRightColumn), m_pDetails );

    m_pOptions = gtk_button_new_with_mnemonic( StringResource::get( "%OPTIONS_BUTTON%" ) );
    gtk_widget_show(m_pOptions);
    gtk_container_set_border_width( GTK_CONTAINER(m_pOptions), 5 );
    gtk_container_add( GTK_CONTAINER(m_pRightColumn), m_pOptions );

    // check env var for save button
    const char *szUserType = getenv( "STAROFFICE_USERTYPE" );
    if( szUserType && *szUserType )
    {
        m_pSave = gtk_button_new_with_mnemonic( StringResource::get( "%SAVE_REPORT_BUTTON%" ) );
        gtk_widget_show(m_pSave);
        gtk_container_set_border_width( GTK_CONTAINER(m_pSave), 5 );
        gtk_container_add( GTK_CONTAINER(m_pRightColumn), m_pSave );
    }
    else
        m_pSave = NULL;


    m_pCheck = gtk_check_button_new_with_mnemonic( StringResource::get( "%ALLOW_CONTACT%" ) );
    gtk_widget_show( m_pCheck );
    gtk_container_set_border_width( GTK_CONTAINER(m_pCheck), 5 );
    //gtk_box_pack_start( GTK_BOX(m_pPageContents), m_pCheck, FALSE, FALSE, 5 );
    gtk_box_pack_start( GTK_BOX(m_pLeftColumn), m_pCheck, FALSE, FALSE, 5 );

    aIter = rSettings.find( "CONTACT" );
    if( aIter != rSettings.end() )
    {
        const char *str = aIter->second.c_str();
        if( str && !strcasecmp(str, "true") )
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(m_pCheck), TRUE );
    }

    m_pAddressLabel = gtk_label_new_with_mnemonic( StringResource::get( "%ENTER_EMAIL%" ) );
    gtk_widget_show( m_pAddressLabel );
    gtk_label_set_justify( GTK_LABEL(m_pAddressLabel), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment( GTK_MISC(m_pAddressLabel), 0, 1 );
    gtk_box_pack_start( GTK_BOX(m_pLeftColumn), m_pAddressLabel, FALSE, FALSE, 5 );

    m_pAddress = gtk_entry_new();
    gtk_widget_show( m_pAddress );
    gtk_box_pack_start( GTK_BOX(m_pLeftColumn), m_pAddress, FALSE, FALSE, 5 );

    aIter = rSettings.find( "EMAIL" );
    if( aIter != rSettings.end() )
        gtk_entry_set_text( GTK_ENTRY(m_pAddress), aIter->second.c_str() );

    gtk_signal_connect( GTK_OBJECT(m_pDetails), "clicked", G_CALLBACK(button_clicked), this );
    gtk_signal_connect( GTK_OBJECT(m_pOptions), "clicked", G_CALLBACK(button_clicked), this );
    if(m_pSave) // optional
        gtk_signal_connect( GTK_OBJECT(m_pSave), "clicked", G_CALLBACK(button_clicked), this );

    gtk_signal_connect( GTK_OBJECT(m_pCheck), "toggled", G_CALLBACK(button_toggled), this );
    button_toggled( m_pCheck, this );

    g_object_ref( G_OBJECT(m_pPageContents) );
}

MainPage::~MainPage()
{
    g_object_unref( G_OBJECT(m_pPageContents) );
}

void MainPage::update()
{
    hash_map< string, string >& rSettings = m_pDialog->getSettings();

    GtkTextIter start, end;
    GtkTextBuffer* pTextBuffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(m_pEntry) );
    gtk_text_buffer_get_bounds( pTextBuffer, &start, &end );
    rSettings[ "DESCRIPTION" ] = gtk_text_buffer_get_text( pTextBuffer, &start, &end, 1 );
    rSettings[ "TITLE" ] = gtk_entry_get_text( GTK_ENTRY(m_pEdit) );
    rSettings[ "CONTACT" ] = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(m_pCheck) ) ? "true" : "false";
    rSettings[ "EMAIL" ] = gtk_entry_get_text( GTK_ENTRY(m_pAddress) );
}

gint MainPage::button_toggled( GtkWidget* pButton, MainPage* pThis )
{
    if ( GTK_TOGGLE_BUTTON (pThis->m_pCheck)->active )
    {
        gtk_widget_set_sensitive( pThis->m_pAddressLabel, TRUE);
        gtk_widget_set_sensitive( pThis->m_pAddress, TRUE);
    }
    else
    {
        gtk_widget_set_sensitive( pThis->m_pAddressLabel, FALSE);
        gtk_widget_set_sensitive( pThis->m_pAddress, FALSE);
    }
    return 0;
}

gint MainPage::button_clicked( GtkWidget* pButton, MainPage* pThis )
{
    if( pButton == pThis->m_pSave )
    {
        GtkWidget* pFile = gtk_file_selection_new( StringResource::get( "%SAVE_REPORT_TITLE%" ) );
        gint nRet = gtk_dialog_run( GTK_DIALOG(pFile) );
        if( nRet == GTK_RESPONSE_OK )
        {
            string aFile = gtk_file_selection_get_filename( GTK_FILE_SELECTION(pFile) );
            pThis->update();
            if( save_crash_report( aFile, pThis->m_pDialog->getSettings() ) )
            {
                // gtk_main_quit();
            }
        }
        gtk_widget_destroy( pFile );
    }
    else if( pButton == pThis->m_pDetails )
    {
        pThis->update();

        GtkWidget* pDialog = gtk_dialog_new_with_buttons( StringResource::get( "%REPORT_CAPTION%" ),
                                                          pThis->m_pDialog->getTopLevel(),
                                                          GTK_DIALOG_MODAL,
                                                          GTK_STOCK_OK,
                                                          GTK_RESPONSE_OK,
                                                          NULL );

        gtk_window_set_default_size( GTK_WINDOW(pDialog), 500, 300 );
        GtkWidget* pScroll = gtk_scrolled_window_new( NULL, NULL );
        gtk_widget_show( pScroll );
        gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW(pScroll), GTK_SHADOW_IN );
        gtk_container_add( GTK_CONTAINER(GTK_DIALOG(pDialog)->vbox), pScroll );

        string aText = crash_get_details( pThis->m_pDialog->getSettings() );
        GtkWidget* pView = gtk_text_view_new();
        gtk_widget_show( pView );
        gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW(pView), GTK_WRAP_WORD );
        gtk_text_view_set_editable( GTK_TEXT_VIEW(pView), FALSE );
        GtkTextBuffer* pBuffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(pView) );
        gtk_text_buffer_set_text( pBuffer, aText.data(), aText.size() );
        gtk_container_add( GTK_CONTAINER(pScroll), pView );

        gtk_dialog_run( GTK_DIALOG(pDialog) );
        gtk_widget_destroy( pDialog );
    }
    else if( pButton == pThis->m_pOptions )
    {
        OptionsDialog aOptions( pThis->m_pDialog->getTopLevel(), pThis->m_pDialog->getSettings() );
        //pThis->m_pDialog->show_sendingstatus( true );
        //pThis->m_pDialog->show_sendingstatus( false );
        //sleep(5);
        //pThis->m_pDialog->hide_sendingstatus();
    }

    return 0;
}

/*
 * OptionsDialog (Proxy-Settings)
 */

OptionsDialog::OptionsDialog( GtkWindow *pParent ,hash_map< string, string >& rSettings )
{
    m_pDialog = gtk_dialog_new_with_buttons( StringResource::get( "%OPTIONS_TITLE%" ),
                                               pParent,
                                               GTK_DIALOG_MODAL,
                                               GTK_STOCK_OK, GTK_RESPONSE_OK,
                                               GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
                                               NULL );

    gtk_window_set_default_size( GTK_WINDOW(m_pDialog), 500, 300 );

    m_pPage = gtk_vbox_new( FALSE, 0 );
    m_pLeftColumn = gtk_vbox_new( FALSE, 5 );

    gtk_widget_show( m_pPage );
    gtk_widget_show( m_pLeftColumn );

    gtk_container_add( GTK_CONTAINER(GTK_DIALOG(m_pDialog)->vbox), m_pPage );

    gtk_container_set_border_width( GTK_CONTAINER(m_pLeftColumn), 5 );
    gtk_box_pack_start( GTK_BOX(m_pPage), m_pLeftColumn, FALSE, FALSE, 5 );

    // frame for proxy settings
    m_pFrame = gtk_frame_new(StringResource::get( "%PROXY_SETTINGS_HEADER%" ));
    gtk_frame_set_shadow_type( GTK_FRAME(m_pFrame), GTK_SHADOW_ETCHED_IN );
    gtk_widget_show(m_pFrame);
    gtk_box_pack_start(GTK_BOX (m_pLeftColumn), m_pFrame, TRUE, TRUE, 0);

    m_pVBox = gtk_vbox_new( FALSE, 0 );
    gtk_widget_show( m_pVBox );
    gtk_container_add( GTK_CONTAINER( m_pFrame ), m_pVBox );

    // the radio buttons
    m_pDirect = gtk_radio_button_new_with_mnemonic( NULL,
            StringResource::get( "%PROXY_SETTINGS_DIRECT%" ) );
    gtk_widget_show(m_pDirect);
    gtk_box_pack_start(GTK_BOX (m_pVBox), m_pDirect, FALSE, FALSE, 0);

    m_pManual = gtk_radio_button_new_with_mnemonic( gtk_radio_button_group( GTK_RADIO_BUTTON(m_pDirect) ),
            StringResource::get( "%PROXY_SETTINGS_MANUAL%" ) );
    gtk_widget_show(m_pManual);
    gtk_box_pack_start(GTK_BOX (m_pVBox), m_pManual, FALSE, FALSE, 0);

    hash_map<string, string>::iterator aIter;
    const char *str = NULL;
    aIter = rSettings.find( "USEPROXY" );
    if( aIter != rSettings.end() )
        str = aIter->second.c_str();
    if( str && !strcasecmp(str, "true") )
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(m_pManual), TRUE );
    else
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(m_pDirect), TRUE );

    // the server and port edit controls
    m_pHBox = gtk_hbox_new( FALSE, 0 );
    gtk_widget_show( m_pHBox );
    gtk_box_pack_start(GTK_BOX (m_pVBox), m_pHBox, FALSE, FALSE, 10);

    m_pVBoxServer = gtk_vbox_new( FALSE, 5 );
    gtk_widget_show( m_pVBoxServer );
    gtk_box_pack_start(GTK_BOX (m_pHBox), m_pVBoxServer, TRUE, TRUE, 10);

    m_pVBoxPort = gtk_vbox_new( FALSE, 5 );
    gtk_widget_show( m_pVBoxPort );
    gtk_box_pack_start(GTK_BOX (m_pHBox), m_pVBoxPort, FALSE, FALSE, 10);

    m_pServerLabel = gtk_label_new_with_mnemonic( StringResource::get( "%PROXY_SETTINGS_ADDRESS%" ) );
    gtk_widget_show( m_pServerLabel );
    gtk_label_set_justify( GTK_LABEL(m_pServerLabel), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment( GTK_MISC(m_pServerLabel), 0, 1 );
    gtk_box_pack_start( GTK_BOX(m_pVBoxServer), m_pServerLabel, FALSE, FALSE, 0 );

    m_pServer = gtk_entry_new();
    gtk_widget_show( m_pServer );
    gtk_box_pack_start( GTK_BOX(m_pVBoxServer), m_pServer, FALSE, FALSE, 0 );
    gtk_label_set_mnemonic_widget( GTK_LABEL(m_pServerLabel), m_pServer );
    aIter = rSettings.find( "SERVER" );
    if( aIter != rSettings.end() )
        gtk_entry_set_text( GTK_ENTRY(m_pServer), aIter->second.c_str() );

    m_pPortLabel = gtk_label_new_with_mnemonic( StringResource::get( "%PROXY_SETTINGS_PORT%" ) );
    gtk_widget_show( m_pPortLabel );
    gtk_label_set_justify( GTK_LABEL(m_pPortLabel), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment( GTK_MISC(m_pPortLabel), 0, 1 );
    gtk_box_pack_start( GTK_BOX(m_pVBoxPort), m_pPortLabel, FALSE, FALSE, 0 );

    m_pPort = gtk_entry_new();
    gtk_widget_show( m_pPort );
    gtk_box_pack_start( GTK_BOX(m_pVBoxPort), m_pPort, FALSE, FALSE, 0 );
    gtk_label_set_mnemonic_widget( GTK_LABEL(m_pPortLabel), m_pPort );
    aIter = rSettings.find( "PORT" );
    if( aIter != rSettings.end() )
        gtk_entry_set_text( GTK_ENTRY(m_pPort), aIter->second.c_str() );

    // help text
    m_pNote = gtk_label_new( StringResource::get( "%PROXY_SETTINGS_DESCRIPTION%" ) );
    gtk_widget_show( m_pNote );
    gtk_label_set_justify( GTK_LABEL(m_pNote), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment( GTK_MISC(m_pNote), 0, 1 );
    gtk_box_pack_start(GTK_BOX (m_pLeftColumn), m_pNote, FALSE, FALSE, 5);
    gtk_label_set_line_wrap (GTK_LABEL (m_pNote), TRUE);


    gtk_signal_connect( GTK_OBJECT(m_pDirect), "toggled", G_CALLBACK(button_toggled), this );
    gtk_signal_connect( GTK_OBJECT(m_pManual), "toggled", G_CALLBACK(button_toggled), this );

    button_toggled( m_pDirect, this );

    if( gtk_dialog_run( GTK_DIALOG(m_pDialog) ) == GTK_RESPONSE_OK )
    {
        rSettings[ "SERVER" ] = getServer();
        rSettings[ "PORT" ]   = getPort();
        rSettings[ "USEPROXY" ] = getUseProxy();
    }
    g_object_ref( G_OBJECT(m_pDialog) );
}

OptionsDialog::~OptionsDialog()
{
    gtk_widget_destroy( m_pDialog );
    g_object_unref( G_OBJECT(m_pDialog) );
}

gint OptionsDialog::button_toggled( GtkWidget* pButton, OptionsDialog* pThis )
{
    if ( GTK_TOGGLE_BUTTON (pThis->m_pManual)->active )
    {
        gtk_widget_set_sensitive( pThis->m_pServerLabel, TRUE);
        gtk_widget_set_sensitive( pThis->m_pServer, TRUE);
        gtk_widget_set_sensitive( pThis->m_pPortLabel, TRUE);
        gtk_widget_set_sensitive( pThis->m_pPort, TRUE);
    }
    else
    {
        gtk_widget_set_sensitive( pThis->m_pServerLabel, FALSE);
        gtk_widget_set_sensitive( pThis->m_pServer, FALSE);
        gtk_widget_set_sensitive( pThis->m_pPortLabel, FALSE);
        gtk_widget_set_sensitive( pThis->m_pPort, FALSE);
    }

    return 0;
}

string OptionsDialog::getUseProxy()
{
    return gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(m_pDirect) ) ? "false" : "true";
}
string OptionsDialog::getServer()
{
    return gtk_entry_get_text( GTK_ENTRY(m_pServer) );
}
string OptionsDialog::getPort()
{
    return gtk_entry_get_text( GTK_ENTRY(m_pPort) );
}

/*
 *  WelcomePage
 */
WelcomePage::WelcomePage( WizardDialog* pParent ) : WizardPage( pParent )
{
    m_aWizardTitle = StringResource::get( "%WELCOME_HEADER%" );

    string aBody = StringResource::get( "%WELCOME_BODY1%" );
    aBody += StringResource::get( "%WELCOME_BODY2%" );
    aBody += StringResource::get( "%WELCOME_BODY3%" );
    aBody += "\n\n";
    aBody += StringResource::get( "%PRIVACY_URL%" );
    m_pPageContents = gtk_label_new( aBody.c_str() );
    gtk_widget_show( m_pPageContents );
    gtk_label_set_line_wrap( GTK_LABEL(m_pPageContents), TRUE );
    gtk_label_set_justify( GTK_LABEL(m_pPageContents), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment( GTK_MISC(m_pPageContents), 0, 1 );
    gtk_misc_set_padding( GTK_MISC(m_pPageContents ), 5, 5);

    g_object_ref( G_OBJECT(m_pPageContents) );
}

WelcomePage::~WelcomePage()
{
    g_object_unref( G_OBJECT(m_pPageContents) );
}

void WelcomePage::update()
{
}
