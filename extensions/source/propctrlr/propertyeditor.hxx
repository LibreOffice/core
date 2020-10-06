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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYEDITOR_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYEDITOR_HXX

#include "browserpage.hxx"
#include "pcrcommon.hxx"

#include <com/sun/star/inspection/XPropertyControl.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <vcl/weld.hxx>
#include <map>

namespace pcr
{
    class IPropertyLineListener;
    class IPropertyControlObserver;
    class OBrowserPage;
    struct OLineDescriptor;
    class OBrowserListBox;

    //= OPropertyEditor
    class OPropertyEditor final
    {
    private:
        typedef std::map< OUString, sal_uInt16 >   MapStringToPageId;
        struct PropertyPage
        {
            sal_uInt16 nPos;
            OUString sLabel;
            std::unique_ptr<OBrowserPage> xPage;
            PropertyPage();
            PropertyPage(sal_uInt16 nPagePos, const OUString& rLabel, std::unique_ptr<OBrowserPage> pPage);
        };

        std::unique_ptr<weld::Container> m_xContainer;
        std::unique_ptr<weld::Notebook> m_xTabControl;
        // controls initially have this parent before they are moved
        std::unique_ptr<weld::Container> m_xControlHoldingParent;
        css::uno::Reference<css::uno::XComponentContext> m_xContext;
        IPropertyLineListener*      m_pListener;
        IPropertyControlObserver*   m_pObserver;
        sal_uInt16                  m_nNextId;
        Link<LinkParamNone*,void>   m_aPageActivationHandler;
        bool                        m_bHasHelpSection;

        MapStringToPageId           m_aPropertyPageIds;
        std::map<sal_uInt16, PropertyPage> m_aShownPages;
        std::map<sal_uInt16, PropertyPage> m_aHiddenPages;

    public:
        explicit OPropertyEditor(const css::uno::Reference<css::uno::XComponentContext>& rContext, weld::Builder& rBuilder);
        ~OPropertyEditor();

        void                        SetLineListener( IPropertyLineListener* );
        void                        SetControlObserver( IPropertyControlObserver* );

        void                        EnableHelpSection( bool _bEnable );
        bool                        HasHelpSection() const { return m_bHasHelpSection; }
        void                        SetHelpText( const OUString& _rHelpText );

        void                        SetHelpId( const OString& sHelpId );
        sal_uInt16                  AppendPage( const OUString& r, const OString& _rHelpId );
        void                        SetPage( sal_uInt16 );
        void                        RemovePage(sal_uInt16 nID);
        sal_uInt16                  GetCurPage() const;
        void                        ClearAll();

        void                        SetPropertyValue(const OUString& _rEntryName, const css::uno::Any& _rValue, bool _bUnknownValue );
        sal_uInt16                  GetPropertyPos(const OUString& rEntryName ) const;
        css::uno::Reference< css::inspection::XPropertyControl >
                                    GetPropertyControl( const OUString& rEntryName );
        void                        EnablePropertyLine( const OUString& _rEntryName, bool _bEnable );
        void                        EnablePropertyControls( const OUString& _rEntryName, sal_Int16 _nControls, bool _bEnable );

        void                        ShowPropertyPage( sal_uInt16 _nPageId, bool _bShow );

        void                        InsertEntry( const OLineDescriptor&, sal_uInt16 _nPageId, sal_uInt16 nPos = EDITOR_LIST_APPEND );
        void                        RemoveEntry( const OUString& _rName );
        void                        ChangeEntry( const OLineDescriptor& );

        void                        setPageActivationHandler(const Link<LinkParamNone*,void>& _rHdl) { m_aPageActivationHandler = _rHdl; }

        Size                        get_preferred_size() const;

        weld::Container*            getWidget() const { return m_xTabControl.get(); }

        void                        Show() { m_xTabControl->show(); }
        void                        Hide() { m_xTabControl->hide(); }
        void                        GrabFocus() { m_xTabControl->grab_focus(); }

        void                        CommitModified();

    private:
        OBrowserPage* getPage( sal_uInt16 _rPageId );
        const OBrowserPage* getPage( sal_uInt16 _rPageId ) const;

        OBrowserPage* getPage( const OUString& _rPropertyName );
        const OBrowserPage* getPage( const OUString& _rPropertyName ) const;

        typedef void (OPropertyEditor::*PageOperation)( OBrowserPage&, const void* );
        void    forEachPage( PageOperation _pOperation );

        void    setPageLineListener( OBrowserPage& _rPage, const void* );
        void    setPageControlObserver( OBrowserPage& _rPage, const void* );
        void    enableHelpSection( OBrowserPage& _rPage, const void* );
        static void setHelpSectionText( OBrowserPage& _rPage, const void* _pPointerToOUString );

        DECL_LINK(OnPageDeactivate, const OString&, bool);
        DECL_LINK(OnPageActivate, const OString&, void);
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYEDITOR_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
