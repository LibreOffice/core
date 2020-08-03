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
#pragma once

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XStorage.hpp>

#include <svtools/insdlg.hxx>
#include <vcl/weld.hxx>
#include <comphelper/embeddedobjectcontainer.hxx>

class INetURLObject;

class InsertObjectDialog_Impl : public weld::GenericDialogController
{
protected:
    css::uno::Reference < css::embed::XEmbeddedObject > m_xObj;
    const css::uno::Reference < css::embed::XStorage > m_xStorage;
    comphelper::EmbeddedObjectContainer aCnt;

    InsertObjectDialog_Impl(weld::Window * pParent,
        const OUString& rUIXMLDescription, const OString& rID,
        const css::uno::Reference < css::embed::XStorage >& xStorage);
public:
    const css::uno::Reference<css::embed::XEmbeddedObject>& GetObject() const { return m_xObj; }
    virtual css::uno::Reference<css::io::XInputStream> GetIconIfIconified(OUString* pGraphicMediaType);
    void SetHelpId(const OString& rHelpId) { m_xDialog->set_help_id(rHelpId); }
    virtual bool IsCreateNew() const;
};

class SvInsertOleDlg : public InsertObjectDialog_Impl
{
    const SvObjectServerList* m_pServers;

    css::uno::Sequence< sal_Int8 > m_aIconMetaFile;
    OUString m_aIconMediaType;

    std::unique_ptr<weld::RadioButton> m_xRbNewObject;
    std::unique_ptr<weld::RadioButton> m_xRbObjectFromfile;
    std::unique_ptr<weld::Frame> m_xObjectTypeFrame;
    std::unique_ptr<weld::TreeView> m_xLbObjecttype;
    std::unique_ptr<weld::Frame> m_xFileFrame;
    std::unique_ptr<weld::Entry> m_xEdFilepath;
    std::unique_ptr<weld::Button> m_xBtnFilepath;
    std::unique_ptr<weld::CheckButton> m_xCbFilelink;
    std::unique_ptr<weld::CheckButton> m_xCbAsIcon;

    DECL_LINK(DoubleClickHdl, weld::TreeView&, bool);
    DECL_LINK(BrowseHdl, weld::Button&, void);
    DECL_LINK(RadioHdl, weld::Button&, void);
    bool IsCreateNew() const override { return m_xRbNewObject->get_active(); }

public:
    SvInsertOleDlg(weld::Window* pParent,
        const css::uno::Reference < css::embed::XStorage >& xStorage,
        const SvObjectServerList* pServers );
    virtual short run() override;

    /// get replacement for the iconified embedded object and the mediatype of the replacement
    css::uno::Reference< css::io::XInputStream > GetIconIfIconified( OUString* pGraphicMediaType ) override;
};

class SfxInsertFloatingFrameDialog : public InsertObjectDialog_Impl
{
private:
    std::unique_ptr<weld::Entry> m_xEDName;
    std::unique_ptr<weld::Entry> m_xEDURL;
    std::unique_ptr<weld::Button> m_xBTOpen;

    std::unique_ptr<weld::RadioButton> m_xRBScrollingOn;
    std::unique_ptr<weld::RadioButton> m_xRBScrollingOff;
    std::unique_ptr<weld::RadioButton> m_xRBScrollingAuto;

    std::unique_ptr<weld::RadioButton> m_xRBFrameBorderOn;
    std::unique_ptr<weld::RadioButton> m_xRBFrameBorderOff;

    std::unique_ptr<weld::Label> m_xFTMarginWidth;
    std::unique_ptr<weld::SpinButton> m_xNMMarginWidth;
    std::unique_ptr<weld::CheckButton> m_xCBMarginWidthDefault;
    std::unique_ptr<weld::Label> m_xFTMarginHeight;
    std::unique_ptr<weld::SpinButton> m_xNMMarginHeight;
    std::unique_ptr<weld::CheckButton> m_xCBMarginHeightDefault;

    DECL_LINK(OpenHdl, weld::Button&, void);
    DECL_LINK(CheckHdl, weld::Button&, void);

    void Init();

public:
    SfxInsertFloatingFrameDialog(weld::Window *pParent,
        const css::uno::Reference<css::embed::XStorage>& xStorage);
    SfxInsertFloatingFrameDialog(weld::Window* pParent,
        const css::uno::Reference<css::embed::XEmbeddedObject>& xObj);
    virtual short run() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
