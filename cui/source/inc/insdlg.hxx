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
#ifndef _SVX_INSDLG_HXX
#define _SVX_INSDLG_HXX

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
#include <svtools/svmedit.hxx>  // MultiLineEdit
#include <comphelper/embeddedobjectcontainer.hxx>

class INetURLObject;

class InsertObjectDialog_Impl : public ModalDialog
{
protected:
    com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > m_xObj;
    const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& m_xStorage;
    comphelper::EmbeddedObjectContainer aCnt;

    InsertObjectDialog_Impl( Window * pParent, const ResId & rResId, const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage );
public:
    com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > GetObject()
                        { return m_xObj; }
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > GetIconIfIconified( ::rtl::OUString* pGraphicMediaType );
    virtual sal_Bool IsCreateNew() const;
};

class SvInsertOleDlg : public InsertObjectDialog_Impl
{
    RadioButton aRbNewObject;
    RadioButton aRbObjectFromfile;
    FixedLine aGbObject;
    ListBox aLbObjecttype;
    Edit aEdFilepath;
    PushButton aBtnFilepath;
    CheckBox aCbFilelink;
    OKButton aOKButton1;
    CancelButton aCancelButton1;
    HelpButton aHelpButton1;
    String aStrFile;
    String      _aOldStr;
    const SvObjectServerList* m_pServers;

    ::com::sun::star::uno::Sequence< sal_Int8 > m_aIconMetaFile;
    ::rtl::OUString m_aIconMediaType;

    DECL_LINK(          DoubleClickHdl, void* );
    DECL_LINK(BrowseHdl, void *);
    DECL_LINK(RadioHdl, void *);
    void                SelectDefault();
    ListBox&            GetObjectTypes()
                        { return aLbObjecttype; }
    String              GetFilePath() const { return aEdFilepath.GetText(); }
    sal_Bool                IsLinked() const    { return aCbFilelink.IsChecked(); }
    sal_Bool                IsCreateNew() const { return aRbNewObject.IsChecked(); }

public:
                        SvInsertOleDlg( Window* pParent,
                            const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage,
                            const SvObjectServerList* pServers = NULL );
    virtual short       Execute();

    /// get replacement for the iconified embedded object and the mediatype of the replacement
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > GetIconIfIconified( ::rtl::OUString* pGraphicMediaType );
};

class SvInsertPlugInDialog : public InsertObjectDialog_Impl
{
private:
    FixedLine aGbFileurl;
    Edit aEdFileurl;
    PushButton aBtnFileurl;
    FixedLine aGbPluginsOptions;
    MultiLineEdit aEdPluginsOptions;
    OKButton aOKButton1;
    CancelButton aCancelButton1;
    HelpButton aHelpButton1;
    INetURLObject*      m_pURL;
    String              m_aCommands;

    DECL_LINK(BrowseHdl, void *);
    String              GetPlugInFile() const { return aEdFileurl.GetText(); }
    String              GetPlugInOptions() const { return aEdPluginsOptions.GetText(); }

public:
                        SvInsertPlugInDialog( Window* pParent,
                            const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage );

                        ~SvInsertPlugInDialog();

    virtual short       Execute();
};

class SfxInsertFloatingFrameDialog : public InsertObjectDialog_Impl
{
private:
    FixedText               aFTName;
    Edit                    aEDName;
    FixedText               aFTURL;
    Edit                    aEDURL;
    PushButton              aBTOpen;

    FixedLine               aFLScrolling;
    RadioButton             aRBScrollingOn;
    RadioButton             aRBScrollingOff;
    RadioButton             aRBScrollingAuto;


    FixedLine               aFLSepLeft;
    FixedLine               aFLFrameBorder;
    RadioButton             aRBFrameBorderOn;
    RadioButton             aRBFrameBorderOff;

    FixedLine               aFLSepRight;
    FixedLine               aFLMargin;
    FixedText               aFTMarginWidth;
    NumericField            aNMMarginWidth;
    CheckBox                aCBMarginWidthDefault;
    FixedText               aFTMarginHeight;
    NumericField            aNMMarginHeight;
    CheckBox                aCBMarginHeightDefault;

    OKButton aOKButton1;
    CancelButton aCancelButton1;
    HelpButton aHelpButton1;

    DECL_STATIC_LINK(       SfxInsertFloatingFrameDialog, OpenHdl, PushButton* );
    DECL_STATIC_LINK(       SfxInsertFloatingFrameDialog, CheckHdl, CheckBox* );

public:
                            SfxInsertFloatingFrameDialog( Window *pParent,
                                const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage );
                            SfxInsertFloatingFrameDialog( Window* pParent,
                                const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >& xObj );
    virtual short           Execute();
};

#endif // _SVX_INSDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
