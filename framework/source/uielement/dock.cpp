void ToolBarManager::AddCustomizeMenuItems(ToolBox const * pToolBar)
{
    // No config menu entries if command ".uno:ConfigureDialog" is not enabled
    Reference< XDispatch > xDisp;
    css::util::URL aURL;
    if ( m_xFrame.is() )
    {
        Reference< XDispatchProvider > xProv( m_xFrame, UNO_QUERY );
        aURL.Complete = ".uno:ConfigureDialog";
        m_xURLTransformer->parseStrict( aURL );
        if ( xProv.is() )
            xDisp = xProv->queryDispatch( aURL, OUString(), 0 );

        if ( !xDisp.is() || IsPluginMode() )
            return;
    }

    // popup menu for quick customization
    bool bHideDisabledEntries = !SvtMenuOptions().IsEntryHidingEnabled();

    ::PopupMenu *pMenu = pToolBar->GetMenu();

    // copy all menu items 'Visible buttons, Customize toolbar, Dock toolbar,
    // Dock all Toolbars) from the loaded resource into the toolbar menu
    sal_uInt16 nGroupLen = pMenu->GetItemCount();
    if (nGroupLen)
        pMenu->InsertSeparator();

    VclPtr<PopupMenu> xVisibleItemsPopupMenu;

    if (MenuItemAllowed(MENUITEM_TOOLBAR_VISIBLEBUTTON))
    {
        pMenu->InsertItem(MENUITEM_TOOLBAR_VISIBLEBUTTON, FwkResId(STR_TOOLBAR_VISIBLE_BUTTONS));
        xVisibleItemsPopupMenu = VclPtr<PopupMenu>::Create();
        pMenu->SetPopupMenu(MENUITEM_TOOLBAR_VISIBLEBUTTON, xVisibleItemsPopupMenu);
    }

    if (MenuItemAllowed(MENUITEM_TOOLBAR_CUSTOMIZETOOLBAR) && m_pToolBar->IsCustomize())
    {
        pMenu->InsertItem(MENUITEM_TOOLBAR_CUSTOMIZETOOLBAR, FwkResId(STR_TOOLBAR_CUSTOMIZE_TOOLBAR));
        pMenu->SetItemCommand(MENUITEM_TOOLBAR_CUSTOMIZETOOLBAR, ".uno:ConfigureToolboxVisible");
    }

    if (nGroupLen != pMenu->GetItemCount())
    {
        pMenu->InsertSeparator();
        nGroupLen = pMenu->GetItemCount();
    }

    if (pToolBar->IsFloatingMode())
    {
        if (MenuItemAllowed(MENUITEM_TOOLBAR_DOCKTOOLBAR))
        {
            pMenu->InsertItem(MENUITEM_TOOLBAR_DOCKTOOLBAR, FwkResId(STR_TOOLBAR_DOCK_TOOLBAR));
            pMenu->SetAccelKey(MENUITEM_TOOLBAR_DOCKTOOLBAR, vcl::KeyCode(KEY_F10, true, true, false, false));
        }
    }
    else
    {
        if ( MenuItemAllowed(MENUITEM_TOOLBAR_UNDOCKTOOLBAR ) &&
           !( xDockable->isLocked() ))
        {
            pMenu->InsertItem(MENUITEM_TOOLBAR_UNDOCKTOOLBAR, FwkResId(STR_TOOLBAR_UNDOCK_TOOLBAR));
            pMenu->SetAccelKey(MENUITEM_TOOLBAR_UNDOCKTOOLBAR, vcl::KeyCode(KEY_F10, true, true, false, false));
        }
    }

    if (MenuItemAllowed(MENUITEM_TOOLBAR_DOCKALLTOOLBAR))
        pMenu->InsertItem(MENUITEM_TOOLBAR_DOCKALLTOOLBAR, FwkResId(STR_TOOLBAR_DOCK_ALL_TOOLBARS));

    if (nGroupLen != pMenu->GetItemCount())
    {
        pMenu->InsertSeparator();
        nGroupLen = pMenu->GetItemCount();
    }

    if (MenuItemAllowed(MENUITEM_TOOLBAR_LOCKTOOLBARPOSITION))
        pMenu->InsertItem(MENUITEM_TOOLBAR_LOCKTOOLBARPOSITION, FwkResId(STR_TOOLBAR_LOCK_TOOLBAR));

    if (MenuItemAllowed(MENUITEM_TOOLBAR_CLOSE))
        pMenu->InsertItem(MENUITEM_TOOLBAR_CLOSE, FwkResId(STR_TOOLBAR_CLOSE_TOOLBAR));

    if (m_pToolBar->IsCustomize())
    {
        bool    bIsFloating( false );

        DockingManager* pDockMgr = vcl::Window::GetDockingManager();
        if ( pDockMgr )
            bIsFloating = pDockMgr->IsFloating( m_pToolBar );

        if ( !bIsFloating )
        {
            pMenu->EnableItem(MENUITEM_TOOLBAR_DOCKALLTOOLBAR, false);
            Reference< XDockableWindow > xDockable( VCLUnoHelper::GetInterface( m_pToolBar ), UNO_QUERY );
            if( xDockable.is() )
                pMenu->CheckItem(MENUITEM_TOOLBAR_LOCKTOOLBARPOSITION, xDockable->isLocked());
        }
        else
            pMenu->EnableItem(MENUITEM_TOOLBAR_LOCKTOOLBARPOSITION, false);

        if (SvtMiscOptions().DisableUICustomization())
        {
            pMenu->EnableItem(MENUITEM_TOOLBAR_VISIBLEBUTTON, false);
            pMenu->EnableItem(MENUITEM_TOOLBAR_CUSTOMIZETOOLBAR, false);
            pMenu->EnableItem(MENUITEM_TOOLBAR_LOCKTOOLBARPOSITION, false);
        }

        // Disable menu item CLOSE if the toolbar has no closer
        if( !(pToolBar->GetFloatStyle() & WB_CLOSEABLE) )
            pMenu->EnableItem(MENUITEM_TOOLBAR_CLOSE, false);

        // Temporary stores a Command --> Url map to update contextual menu with the
        // correct icons. The popup icons are by default the same as those in the
        // toolbar. They are not correct for contextual popup menu.
        std::map< OUString, Image > commandToImage;

        if (xVisibleItemsPopupMenu)
        {
            // Go through all toolbar items and add them to the context menu
            for ( ToolBox::ImplToolItems::size_type nPos = 0; nPos < m_pToolBar->GetItemCount(); ++nPos )
            {
                if ( m_pToolBar->GetItemType(nPos) == ToolBoxItemType::BUTTON )
                {
                    sal_uInt16 nId = m_pToolBar->GetItemId(nPos);
                    OUString aCommandURL = m_pToolBar->GetItemCommand( nId );
                    xVisibleItemsPopupMenu->InsertItem( STARTID_CUSTOMIZE_POPUPMENU+nPos, m_pToolBar->GetItemText( nId ), MenuItemBits::CHECKABLE );
                    xVisibleItemsPopupMenu->CheckItem( STARTID_CUSTOMIZE_POPUPMENU+nPos, m_pToolBar->IsItemVisible( nId ) );
                    xVisibleItemsPopupMenu->SetItemCommand( STARTID_CUSTOMIZE_POPUPMENU+nPos, aCommandURL );
                    Image aImage(vcl::CommandInfoProvider::GetImageForCommand(aCommandURL, m_xFrame));
                    commandToImage[aCommandURL] = aImage;
                    xVisibleItemsPopupMenu->SetItemImage( STARTID_CUSTOMIZE_POPUPMENU+nPos, aImage );
                    vcl::KeyCode aKeyCodeShortCut = vcl::CommandInfoProvider::GetCommandKeyCodeShortcut( aCommandURL, m_xFrame );
                    xVisibleItemsPopupMenu->SetAccelKey( STARTID_CUSTOMIZE_POPUPMENU+nPos, aKeyCodeShortCut );
                }
                else
                {
                    xVisibleItemsPopupMenu->InsertSeparator();
                }
            }
        }

        // Now we go through all the contextual menu to update the icons
        // and accelerator key shortcuts
        std::map< OUString, Image >::iterator it;
        for ( sal_uInt16 nPos = 0; nPos < pMenu->GetItemCount(); ++nPos )
        {
            sal_uInt16 nId = pMenu->GetItemId( nPos );
            OUString cmdUrl = pMenu->GetItemCommand( nId );
            it = commandToImage.find( cmdUrl );
            if (it != commandToImage.end()) {
                pMenu->SetItemImage( nId, it->second );
            }
            vcl::KeyCode aKeyCodeShortCut = vcl::CommandInfoProvider::GetCommandKeyCodeShortcut( cmdUrl, m_xFrame );
            if ( aKeyCodeShortCut.GetFullCode() != 0 )
                pMenu->SetAccelKey( nId, aKeyCodeShortCut );
        }
    }

    // Set the title of the menu
    pMenu->SetText( pToolBar->GetText() );

    if ( bHideDisabledEntries )
        pMenu->RemoveDisabledEntries();
}