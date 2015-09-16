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
#ifndef INCLUDED_CUI_SOURCE_INC_INSDLG_HXX
#define INCLUDED_CUI_SOURCE_INC_INSDLG_HXX

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XStorage.hpp>

#include <svtools/insdlg.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/svmedit.hxx>
#include <comphelper/embeddedobjectcontainer.hxx>

class VclFrame;

class INetURLObject;

class InsertObjectDialog_Impl : public ModalDialog
{
protected:
    css::uno::Reference < css::embed::XEmbeddedObject > m_xObj;
    const css::uno::Reference < css::embed::XStorage > m_xStorage;
    comphelper::EmbeddedObjectContainer aCnt;

    InsertObjectDialog_Impl(vcl::Window * pParent, const OUString& rID,
        const OUString& rUIXMLDescription,
        const css::uno::Reference < css::embed::XStorage >& xStorage);
public:
    css::uno::Reference < css::embed::XEmbeddedObject > GetObject()
                        { return m_xObj; }
    virtual css::uno::Reference< css::io::XInputStream > GetIconIfIconified( OUString* pGraphicMediaType );
    virtual bool IsCreateNew() const;
};

class SvInsertOleDlg : public InsertObjectDialog_Impl
{
    VclPtr<RadioButton> m_pRbNewObject;
    VclPtr<RadioButton> m_pRbObjectFromfile;
    VclPtr<VclFrame> m_pObjectTypeFrame;
    VclPtr<ListBox> m_pLbObjecttype;
    VclPtr<VclFrame> m_pFileFrame;
    VclPtr<Edit> m_pEdFilepath;
    VclPtr<PushButton> m_pBtnFilepath;
    VclPtr<CheckBox> m_pCbFilelink;
    const SvObjectServerList* m_pServers;

    css::uno::Sequence< sal_Int8 > m_aIconMetaFile;
    OUString m_aIconMediaType;

    DECL_LINK_TYPED(DoubleClickHdl, ListBox&, void);
    DECL_LINK_TYPED(BrowseHdl, Button*, void);
    DECL_LINK_TYPED(RadioHdl, Button*, void);
    void SelectDefault();
    ListBox& GetObjectTypes()
        { return *m_pLbObjecttype; }
    OUString GetFilePath() const
        { return m_pEdFilepath->GetText(); }
    bool IsLinked() const
        { return m_pCbFilelink->IsChecked(); }
    bool IsCreateNew() const SAL_OVERRIDE
        { return m_pRbNewObject->IsChecked(); }

public:
    SvInsertOleDlg( vcl::Window* pParent,
        const css::uno::Reference < css::embed::XStorage >& xStorage,
        const SvObjectServerList* pServers = NULL );
    virtual ~SvInsertOleDlg();
    virtual void dispose() SAL_OVERRIDE;
    virtual short Execute() SAL_OVERRIDE;

    /// get replacement for the iconified embedded object and the mediatype of the replacement
    css::uno::Reference< css::io::XInputStream > GetIconIfIconified( OUString* pGraphicMediaType ) SAL_OVERRIDE;
};

class SvInsertPlugInDialog : public InsertObjectDialog_Impl
{
private:
    VclPtr<Edit> m_pEdFileurl;
    VclPtr<PushButton> m_pBtnFileurl;
    VclPtr<VclMultiLineEdit> m_pEdPluginsOptions;
    INetURLObject* m_pURL;
    OUString m_aCommands;

    DECL_LINK_TYPED(BrowseHdl, Button*, void);
    OUString GetPlugInFile() const { return m_pEdFileurl->GetText(); }
    OUString GetPlugInOptions() const { return m_pEdPluginsOptions->GetText(); }

public:
    SvInsertPlugInDialog(vcl::Window* pParent,
        const css::uno::Reference < css::embed::XStorage >& xStorage);
    virtual ~SvInsertPlugInDialog();
    virtual void dispose() SAL_OVERRIDE;
    virtual short Execute() SAL_OVERRIDE;
};

class SfxInsertFloatingFrameDialog : public InsertObjectDialog_Impl
{
private:
    VclPtr<Edit> m_pEDName;
    VclPtr<Edit> m_pEDURL;
    VclPtr<PushButton> m_pBTOpen;

    VclPtr<RadioButton> m_pRBScrollingOn;
    VclPtr<RadioButton> m_pRBScrollingOff;
    VclPtr<RadioButton> m_pRBScrollingAuto;

    VclPtr<RadioButton> m_pRBFrameBorderOn;
    VclPtr<RadioButton> m_pRBFrameBorderOff;

    VclPtr<FixedText> m_pFTMarginWidth;
    VclPtr<NumericField> m_pNMMarginWidth;
    VclPtr<CheckBox> m_pCBMarginWidthDefault;
    VclPtr<FixedText> m_pFTMarginHeight;
    VclPtr<NumericField> m_pNMMarginHeight;
    VclPtr<CheckBox> m_pCBMarginHeightDefault;

    DECL_LINK_TYPED(OpenHdl, Button*, void );
    DECL_LINK_TYPED(CheckHdl, Button*, void );

    void Init();

public:
    SfxInsertFloatingFrameDialog(vcl::Window *pParent,
        const css::uno::Reference < css::embed::XStorage >& xStorage );
    SfxInsertFloatingFrameDialog( vcl::Window* pParent,
        const css::uno::Reference < css::embed::XEmbeddedObject >& xObj );
    virtual ~SfxInsertFloatingFrameDialog();
    virtual void dispose() SAL_OVERRIDE;
    virtual short Execute() SAL_OVERRIDE;
};

#endif // INCLUDED_CUI_SOURCE_INC_INSDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
