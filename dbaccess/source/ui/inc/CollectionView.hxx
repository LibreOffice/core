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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_COLLECTIONVIEW_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_COLLECTIONVIEW_HXX

#include <svtools/fileview.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace dbaui
{
    /* this class allows to browse through the collection of forms and reports
    */
    class OCollectionView : public ModalDialog
    {
        VclPtr<FixedText>      m_pFTCurrentPath;
        VclPtr<PushButton>     m_pNewFolder;
        VclPtr<PushButton>     m_pUp;
        VclPtr<SvtFileView>    m_pView;
        VclPtr<Edit>           m_pName;
        VclPtr<PushButton>     m_pPB_OK;
        css::uno::Reference< css::ucb::XContent>                  m_xContent;
        css::uno::Reference< css::uno::XComponentContext >        m_xContext;
        bool                   m_bCreateForm;

        DECL_LINK_TYPED(Up_Click, Button*, void);
        DECL_LINK_TYPED(NewFolder_Click, Button*, void);
        DECL_LINK_TYPED(Save_Click, Button*, void);
        DECL_LINK_TYPED(Dbl_Click_FileView, SvTreeListBox*, bool);

        /// sets the fixedtext to the right content
        void initCurrentPath();
    public:
        OCollectionView( vcl::Window * pParent
                        ,const css::uno::Reference< css::ucb::XContent>& _xContent
                        ,const OUString& _sDefaultName
                        ,const css::uno::Reference< css::uno::XComponentContext >& _rxContext);
        virtual ~OCollectionView();
        virtual void dispose() SAL_OVERRIDE;
        css::uno::Reference< css::ucb::XContent> getSelectedFolder() const { return m_xContent;}
        OUString getName() const;
    };
}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_COLLECTIONVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
