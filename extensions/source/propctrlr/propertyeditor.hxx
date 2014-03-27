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

#ifndef _EXTENSIONS_PROPCTRLR_PROPERTYEDITOR_HXX_
#define _EXTENSIONS_PROPCTRLR_PROPERTYEDITOR_HXX_

#include "pcrcommon.hxx"

#include <com/sun/star/inspection/XPropertyControl.hpp>
#include <vcl/tabctrl.hxx>
#include <boost/mem_fn.hpp>
#include <map>


namespace pcr
{


    class IPropertyLineListener;
    class IPropertyControlObserver;
    class OBrowserPage;
    struct OLineDescriptor;
    class OBrowserListBox;


    //= OPropertyEditor

    class OPropertyEditor : public Control
    {
    private:
        typedef ::std::map< OUString, sal_uInt16 >   MapStringToPageId;
        struct HiddenPage
        {
            sal_uInt16  nPos;
            TabPage*    pPage;
            HiddenPage() : nPos( 0 ), pPage( NULL ) { }
            HiddenPage( sal_uInt16 _nPos, TabPage* _pPage ) : nPos( _nPos ), pPage( _pPage ) { }
        };

    private:
        TabControl                  m_aTabControl;
        IPropertyLineListener*      m_pListener;
        IPropertyControlObserver*   m_pObserver;
        sal_uInt16                  m_nNextId;
        Link                        m_aPageActivationHandler;
        bool                        m_bHasHelpSection;
        sal_Int32                   m_nMinHelpLines;
        sal_Int32                   m_nMaxHelpLines;

        MapStringToPageId                       m_aPropertyPageIds;
        ::std::map< sal_uInt16, HiddenPage >    m_aHiddenPages;

    protected:
        void                        Resize() SAL_OVERRIDE;
        void                        GetFocus() SAL_OVERRIDE;

    public:
                                    OPropertyEditor (Window* pParent, WinBits nWinStyle = WB_DIALOGCONTROL);

                                    ~OPropertyEditor();

        void                        EnableUpdate();
        void                        DisableUpdate();

        void                        SetLineListener( IPropertyLineListener* );
        void                        SetControlObserver( IPropertyControlObserver* );

        void                        EnableHelpSection( bool _bEnable );
        bool                        HasHelpSection() const;
        void                        SetHelpText( const OUString& _rHelpText );
        void                        SetHelpLineLimites( sal_Int32 _nMinLines, sal_Int32 _nMaxLines );

        void                        SetHelpId( const OString& sHelpId );
        sal_uInt16                  AppendPage( const OUString& r, const OString& _rHelpId );
        void                        SetPage( sal_uInt16 );
        void                        RemovePage(sal_uInt16 nID);
        sal_uInt16                  GetCurPage();
        void                        ClearAll();

        void                        SetPropertyValue(const OUString& _rEntryName, const ::com::sun::star::uno::Any& _rValue, bool _bUnknownValue );
        ::com::sun::star::uno::Any  GetPropertyValue(const OUString& rEntryName ) const;
        sal_uInt16                  GetPropertyPos(const OUString& rEntryName ) const;
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >
                                    GetPropertyControl( const OUString& rEntryName );
        void                        EnablePropertyLine( const OUString& _rEntryName, bool _bEnable );
        void                        EnablePropertyControls( const OUString& _rEntryName, sal_Int16 _nControls, bool _bEnable );

        void                        ShowPropertyPage( sal_uInt16 _nPageId, bool _bShow );

        sal_uInt16                  InsertEntry( const OLineDescriptor&, sal_uInt16 _nPageId, sal_uInt16 nPos = EDITOR_LIST_APPEND );
        void                        RemoveEntry( const OUString& _rName );
        void                        ChangeEntry( const OLineDescriptor& );

        void    setPageActivationHandler(const Link& _rHdl) { m_aPageActivationHandler = _rHdl; }
        Link    getPageActivationHandler() const { return m_aPageActivationHandler; }

        // #95343# -------------------------------
        sal_Int32 getMinimumWidth();
        sal_Int32 getMinimumHeight();

        void                        CommitModified();

    protected:
        using Window::SetHelpText;
        using Window::Update;

    private:
        OBrowserPage* getPage( sal_uInt16& _rPageId );
        const OBrowserPage* getPage( sal_uInt16& _rPageId ) const;

        OBrowserPage* getPage( const OUString& _rPropertyName );
        const OBrowserPage* getPage( const OUString& _rPropertyName ) const;

        void Update(const ::std::mem_fun_t<void,OBrowserListBox>& _aUpdateFunction);

        typedef void (OPropertyEditor::*PageOperation)( OBrowserPage&, const void* );
        void    forEachPage( PageOperation _pOperation, const void* _pArgument = NULL );

        void    setPageLineListener( OBrowserPage& _rPage, const void* );
        void    setPageControlObserver( OBrowserPage& _rPage, const void* );
        void    enableHelpSection( OBrowserPage& _rPage, const void* );
        void    setHelpSectionText( OBrowserPage& _rPage, const void* _pPointerToOUString );
        void    setHelpLineLimits( OBrowserPage& _rPage, const void* );

    protected:
        DECL_LINK(OnPageDeactivate, void *);
        DECL_LINK(OnPageActivate, void *);
    };


} // namespace pcr


#endif // _EXTENSIONS_PROPCTRLR_PROPERTYEDITOR_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
