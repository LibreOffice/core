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
    com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > m_xObj;
    const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& m_xStorage;
    comphelper::EmbeddedObjectContainer aCnt;

    InsertObjectDialog_Impl(vcl::Window * pParent, const OString& rID,
        const OUString& rUIXMLDescription,
        const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage);
public:
    com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > GetObject()
                        { return m_xObj; }
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > GetIconIfIconified( OUString* pGraphicMediaType );
    virtual bool IsCreateNew() const;
};

class SvInsertOleDlg : public InsertObjectDialog_Impl
{
    RadioButton* m_pRbNewObject;
    RadioButton* m_pRbObjectFromfile;
    VclFrame* m_pObjectTypeFrame;
    ListBox* m_pLbObjecttype;
    VclFrame* m_pFileFrame;
    Edit* m_pEdFilepath;
    PushButton* m_pBtnFilepath;
    CheckBox* m_pCbFilelink;
    const SvObjectServerList* m_pServers;

    ::com::sun::star::uno::Sequence< sal_Int8 > m_aIconMetaFile;
    OUString m_aIconMediaType;

    DECL_LINK(DoubleClickHdl, void*);
    DECL_LINK(BrowseHdl, void *);
    DECL_LINK(RadioHdl, void *);
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
        const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage,
        const SvObjectServerList* pServers = NULL );
    virtual short Execute() SAL_OVERRIDE;

    /// get replacement for the iconified embedded object and the mediatype of the replacement
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > GetIconIfIconified( OUString* pGraphicMediaType ) SAL_OVERRIDE;
};

class SvInsertPlugInDialog : public InsertObjectDialog_Impl
{
private:
    Edit* m_pEdFileurl;
    PushButton* m_pBtnFileurl;
    VclMultiLineEdit* m_pEdPluginsOptions;
    INetURLObject* m_pURL;
    OUString m_aCommands;

    DECL_LINK(BrowseHdl, void *);
    OUString GetPlugInFile() const { return m_pEdFileurl->GetText(); }
    OUString GetPlugInOptions() const { return m_pEdPluginsOptions->GetText(); }

public:
    SvInsertPlugInDialog(vcl::Window* pParent,
        const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage);
    virtual ~SvInsertPlugInDialog();
    virtual short Execute() SAL_OVERRIDE;
};

class SfxInsertFloatingFrameDialog : public InsertObjectDialog_Impl
{
private:
    Edit* m_pEDName;
    Edit* m_pEDURL;
    PushButton* m_pBTOpen;

    RadioButton* m_pRBScrollingOn;
    RadioButton* m_pRBScrollingOff;
    RadioButton* m_pRBScrollingAuto;

    RadioButton* m_pRBFrameBorderOn;
    RadioButton* m_pRBFrameBorderOff;

    FixedText* m_pFTMarginWidth;
    NumericField* m_pNMMarginWidth;
    CheckBox* m_pCBMarginWidthDefault;
    FixedText* m_pFTMarginHeight;
    NumericField* m_pNMMarginHeight;
    CheckBox* m_pCBMarginHeightDefault;

    DECL_STATIC_LINK(SfxInsertFloatingFrameDialog, OpenHdl, PushButton* );
    DECL_STATIC_LINK(SfxInsertFloatingFrameDialog, CheckHdl, CheckBox* );

    void Init();

public:
    SfxInsertFloatingFrameDialog(vcl::Window *pParent,
        const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage );
    SfxInsertFloatingFrameDialog( vcl::Window* pParent,
        const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >& xObj );
    virtual short Execute() SAL_OVERRIDE;
};

#endif // INCLUDED_CUI_SOURCE_INC_INSDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
